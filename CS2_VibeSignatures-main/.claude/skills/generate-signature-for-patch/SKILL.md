---
name: generate-signature-for-patch
description: |
  Generate and validate unique byte signatures for instructions that need to be runtime-patched using IDA Pro MCP.
  Use this skill when you need a signature to locate a specific instruction for patching (e.g., force/skip a branch, NOP a call, change an immediate operand).
  Triggers: patch signature, signature for patch, patch instruction signature, nop signature, jump patch signature, skip branch signature, force branch signature
---

# Generate Signature for Patch

Generate a unique hex byte signature that locates an instruction to be patched at runtime, along with the replacement `patch_bytes`.

## Core Concept

For patch signatures, we signature the **instruction to be patched**. The signature uniquely identifies the location so a runtime patcher can find and overwrite the original bytes with `patch_bytes`.

Hard requirements:
1. The target instruction must be fully fixed (no wildcard bytes at all).
2. Instructions other than the target instruction may use wildcarding.
3. Signature length grows by complete instruction boundaries and stops at the shortest unique prefix.
4. `patch_bytes` are determined by the LLM based on the desired patch effect, **not** by the script.
5. `len(patch_bytes)` must equal `len(original_instruction_bytes)` (pad with `0x90` NOP if the replacement is shorter).

Strategy:
- **Forward-only expansion:** Expand only forward (after target instruction). The signature may extend beyond the current function boundary into CC padding or the next function. `patch_sig_disp` is always `0` — the signature always starts at the target instruction.

## Prerequisites

- Target instruction address (the instruction to be patched)
- Desired patch effect description (e.g., "skip if-branch", "NOP out function call", "change immediate value")
- IDA Pro MCP connection

## Method

### 1. Determine `patch_bytes` (LLM Step — before running the script)

Examine the target instruction and its context using IDA Pro MCP, then determine the appropriate `patch_bytes`.

Common patch patterns:
- **Skip conditional branch (near jcc `0F 8x rel32` → `jmp rel32`):** Replace first 6 bytes with `E9 <new_rel32> 90`. Compute `new_rel32` = original branch target − (patch_addr + 5).
- **Skip conditional branch (short jcc `7x rel8` → `jmp short`):** Replace `7x rel8` with `EB rel8`.
- **Force conditional branch to fall through:** NOP the entire jcc instruction (`90 90 ...`).
- **NOP a `call rel32`:** Replace `E8 xx xx xx xx` with `90 90 90 90 90`.
- **NOP a `call [reg+disp]`:** Replace all bytes with `90`.
- **Change immediate operand:** Modify the immediate bytes in-place.

### 2. Generate and Validate Signature (Single Step)

Use a single `py_eval` call that:
- Collects instruction bytes from the target instruction forward, tests uniqueness.
- Forward-only expansion (no backward expansion — `patch_sig_disp` is always `0`).
- Enforces **no wildcard on the target instruction**.
- Computes both VA and RVA for the target instruction.
- Outputs the shortest unique signature as `patch_sig` with metadata.

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi, ida_bytes, idautils, ida_ua, ida_segment, json

def main():
    target_inst = <inst_addr>
    min_sig_bytes = 6
    max_sig_bytes = 96
    max_instructions = 64

    # --- Binary search wrapper (IDA 9.0+ find_bytes -> older bin_search fallback) ---
    def raw_bin_search(ea, max_ea, data, mask, flags=0):
        if hasattr(ida_bytes, 'find_bytes'):
            return ida_bytes.find_bytes(data, ea, range_end=max_ea, mask=mask, flags=flags)
        return ida_bytes.bin_search(ea, max_ea, data, mask, len(data), flags)

    image_base = idaapi.get_imagebase()

    f = idaapi.get_func(target_inst)
    if not f:
        print(json.dumps({
            "inst_va": hex(target_inst),
            "error": "target instruction is not inside a known function",
            "status": "failed"
        }))
        return

    insn0 = idautils.DecodeInstruction(target_inst)
    if not insn0 or insn0.size <= 0:
        print(json.dumps({
            "inst_va": hex(target_inst),
            "error": "failed to decode target instruction",
            "status": "failed"
        }))
        return

    raw0 = ida_bytes.get_bytes(target_inst, insn0.size)
    if not raw0:
        print(json.dumps({
            "inst_va": hex(target_inst),
            "error": "failed to read target instruction bytes",
            "status": "failed"
        }))
        return

    seg = ida_segment.get_segm_by_name(".text")
    if seg:
        search_start, search_end = seg.start_ea, seg.end_ea
    else:
        search_start, search_end = idaapi.cvar.inf.min_ea, idaapi.cvar.inf.max_ea

    # --- Helper: wildcard non-target instructions ---
    def wildcard_instruction(addr, insn_obj, raw_bytes):
        wild = set()
        for op in insn_obj.ops:
            ot = int(op.type)
            if ot == int(idaapi.o_void):
                continue
            if ot in (int(idaapi.o_imm), int(idaapi.o_near), int(idaapi.o_far), int(idaapi.o_mem), int(idaapi.o_displ)):
                offb = int(getattr(op, "offb", 0))
                if offb > 0 and offb < insn_obj.size:
                    dsz = ida_ua.get_dtype_size(getattr(op, "dtype", getattr(op, "dtyp", 0)))
                    if dsz <= 0:
                        dsz = insn_obj.size - offb
                    for i in range(offb, min(insn_obj.size, offb + dsz)):
                        wild.add(i)

                offo = int(getattr(op, "offo", 0))
                if offo > 0 and offo < insn_obj.size:
                    dsz2 = ida_ua.get_dtype_size(getattr(op, "dtype", getattr(op, "dtyp", 0)))
                    if dsz2 <= 0:
                        dsz2 = insn_obj.size - offo
                    for i in range(offo, min(insn_obj.size, offo + dsz2)):
                        wild.add(i)

        # Branch/call rel targets are volatile.
        b0 = raw_bytes[0]
        if b0 in (0xE8, 0xE9, 0xEB):
            for i in range(1, insn_obj.size):
                wild.add(i)
        elif b0 == 0x0F and insn_obj.size >= 2 and (raw_bytes[1] & 0xF0) == 0x80:
            for i in range(2, insn_obj.size):
                wild.add(i)
        elif 0x70 <= b0 <= 0x7F:
            for i in range(1, insn_obj.size):
                wild.add(i)

        tokens = []
        for idx in range(insn_obj.size):
            tokens.append("??" if idx in wild else f"{raw_bytes[idx]:02X}")
        return tokens

    # --- Helper: test uniqueness of a token list, expecting match at expected_addr ---
    def test_unique(tokens, expected_addr):
        if all(t == "??" for t in tokens):
            return False
        data = bytes(0 if t == "??" else int(t, 16) for t in tokens)
        mask = bytes(0x00 if t == "??" else 0xFF for t in tokens)
        flags = ida_bytes.BIN_SEARCH_FORWARD | ida_bytes.BIN_SEARCH_NOBREAK

        matches = []
        ea = raw_bin_search(search_start, search_end, data, mask, flags)
        while ea != idaapi.BADADDR and len(matches) < 2:
            matches.append(ea)
            ea = raw_bin_search(ea + 1, search_end, data, mask, flags)

        return len(matches) == 1 and matches[0] == expected_addr

    # ====================================================================
    # Forward-only expansion (signature starts at target_inst)
    # May extend beyond the current function into CC padding or next function.
    # ====================================================================
    limit_end = target_inst + max_sig_bytes
    fwd_tokens = []
    fwd_boundaries = []
    cursor = target_inst
    inst_count = 0
    target_inst_len = None

    while (
        cursor < search_end
        and cursor < limit_end
        and len(fwd_tokens) < max_sig_bytes
        and inst_count < max_instructions
    ):
        insn = idautils.DecodeInstruction(cursor)
        if not insn or insn.size <= 0:
            break
        raw = ida_bytes.get_bytes(cursor, insn.size)
        if not raw:
            break

        if cursor == target_inst:
            # Target instruction: fully fixed, no wildcards.
            target_inst_len = insn.size
            for idx in range(insn.size):
                if len(fwd_tokens) < max_sig_bytes:
                    fwd_tokens.append(f"{raw[idx]:02X}")
        else:
            toks = wildcard_instruction(cursor, insn, raw)
            for t in toks:
                if len(fwd_tokens) < max_sig_bytes:
                    fwd_tokens.append(t)

        fwd_boundaries.append(len(fwd_tokens))
        cursor += insn.size
        inst_count += 1

    if target_inst_len is None:
        print(json.dumps({
            "inst_va": hex(target_inst),
            "error": "no signature bytes collected",
            "status": "failed"
        }))
        return

    min_boundary = max(min_sig_bytes, target_inst_len)

    # Try expanding at each instruction boundary until unique
    phase1_sig = None
    phase1_boundary = 0
    for boundary in fwd_boundaries:
        if boundary < min_boundary:
            continue
        prefix = fwd_tokens[:boundary]
        if test_unique(prefix, target_inst):
            phase1_sig = " ".join(prefix)
            phase1_boundary = boundary
            break

    if phase1_sig:
        print(json.dumps({
            "patch_sig": phase1_sig,
            "sig_bytes": phase1_boundary,
            "patch_sig_va": hex(target_inst),
            "patch_sig_disp": 0,
            "patch_inst_length": target_inst_len,
            "patch_va": hex(target_inst),
            "patch_rva": hex(target_inst - image_base),
            "original_bytes": " ".join(f"{b:02X}" for b in raw0),
            "status": "success"
        }))
        return

    # Forward-only expansion exhausted without finding a unique signature.
    print(json.dumps({
        "patch_va": hex(target_inst),
        "patch_rva": hex(target_inst - image_base),
        "original_bytes": " ".join(f"{b:02X}" for b in raw0),
        "total_fwd_tokens": len(fwd_tokens),
        "sig_full_fwd": " ".join(fwd_tokens),
        "error": "no unique signature found with forward-only expansion",
        "status": "failed"
    }))

main()
"""
```

**Result handling:**
- `status == "success"` -> Use `patch_sig` directly as final signature. Proceed to Step 3.
- `status == "failed"` -> See Step 4.

### 3. Verify `patch_bytes` (Optional but Recommended)

After generating the signature, verify the LLM-determined `patch_bytes` by applying them in IDA and checking the disassembly, then **restore** the original bytes.

**Step 3a: Apply patch and inspect**

Use `mcp__ida-pro-mcp__patch` to write `patch_bytes` at `patch_va`:

```
mcp__ida-pro-mcp__patch addr="<patch_va>" data="<patch_bytes hex>"
```

Then use `mcp__ida-pro-mcp__decompile` or `mcp__ida-pro-mcp__disasm` to verify the patch effect matches the desired behavior.

**Step 3b: Restore original bytes**

After verification, **always** restore the original bytes:

```
mcp__ida-pro-mcp__patch addr="<patch_va>" data="<original_bytes hex>"
```

If the patch effect does not match expectations, revise `patch_bytes` and repeat from Step 1.

### 4. Iterate if Needed

If Step 2 returns `status: "failed"`:
1. Increase `max_sig_bytes` (e.g. from `96` to `192`) and re-run Step 2.
2. Increase `max_instructions` (e.g. from `64` to `128`) if function instructions are short.
3. If still not unique, consider patching a different instruction that achieves the same effect and re-run.

### 5. Continue with Unfinished Tasks

If we are called by a task from a task list / parent SKILL, restore and continue with the unfinished tasks.

## Output Format

Required:
- `patch_sig`: Space-separated hex bytes with `??` for wildcards.
- `patch_bytes`: Space-separated hex bytes to write at the patch location.

Recommended metadata:
- `patch_sig_va`: VA of signature start (always equals `patch_va` since `patch_sig_disp` is always `0`).
- `patch_sig_disp`: Always `0` — signature always starts at the target instruction.
- `patch_inst_length`: Length of the target instruction in bytes.
- `patch_va`: VA of the instruction to be patched.
- `patch_rva`: RVA of the instruction to be patched (VA − image base).
- `original_bytes`: Original bytes of the target instruction (for restore/rollback).

### Example Output

Patch effect: skip conditional branch `jbe` → unconditional `jmp` (the `if` block becomes dead code).

```yaml
patch_sig: "0F 86 AF 00 00 00 0F 57 C0 0F 2E C2"
patch_va: 0x180A00E2F
patch_rva: 0xA00E2F
patch_sig_disp: 0
patch_inst_length: 6
original_bytes: "0F 86 AF 00 00 00"
patch_bytes: "E9 B0 00 00 00 90"
```

In this example:
- Original instruction: `jbe loc_180A00EE4` (6 bytes: `0F 86 AF 00 00 00`)
- Patch converts it to `jmp loc_180A00EE4` + `nop`: `E9 B0 00 00 00 90`
- `new_rel32` = `0x180A00EE4 − (0x180A00E2F + 5)` = `0xB0` → `B0 00 00 00`
- The branch now always jumps, making the if-block dead code
