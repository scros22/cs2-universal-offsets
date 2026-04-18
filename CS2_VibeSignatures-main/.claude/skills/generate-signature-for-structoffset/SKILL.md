---
name: generate-signature-for-structoffset
description: |
  Generate and validate unique byte signatures for instructions containing a struct member offset using IDA Pro MCP.
  Use this skill when you need a signature for an instruction like mov [rcx+1A8h], eax or cmp dword ptr [rdi+0B0h], 0, where the struct offset must be explicitly fixed in the first instruction bytes.
  Triggers: struct offset signature, signature for struct offset, structure member offset signature, mov [reg+offset] signature, struct field signature
---

# Generate Signature for Struct Offset

Generate a unique hex byte signature that locates an instruction containing a specific struct member offset (for example: `mov [rcx+1A8h], eax`, `cmp dword ptr [rdi+0B0h], 0`).

## Core Concept

For struct-offset signatures, we signature the **instruction containing the struct offset**, not the function body itself.

Hard requirements:
1. The target instruction must be fully fixed (no wildcard bytes at all).
2. The displacement bytes carrying `struct_offset` in the target instruction must be explicitly included (not wildcarded).
3. Instructions other than the target instruction may use wildcarding.
4. Signature length grows by complete instruction boundaries and stops at the shortest unique prefix.

Strategy:
- **Forward-only expansion:** Expand only forward (after target instruction). The signature may extend beyond the current function boundary into CC padding or the next function. `offset_sig_disp` is always `0` — the signature always starts at the target instruction.

## Prerequisites

- Target instruction address (the instruction that contains the struct offset)
- Expected `struct_offset` value (e.g. `0x1A8`)
- IDA Pro MCP connection

## Method

### 1. Generate and Validate Signature (Single Step)

Use a single `py_eval` call that:
- Validates the input instruction contains the expected `struct_offset` displacement.
- Collects instruction bytes from the target instruction forward, tests uniqueness.
- Forward-only expansion (no backward expansion — `offset_sig_disp` is always `0`).
- Enforces **no wildcard on the target instruction**.
- Computes both VA and RVA for the target instruction.
- Outputs the shortest unique signature as `struct_sig` with metadata.

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi, ida_bytes, idautils, ida_ua, ida_segment, json

def main():
    target_inst = <inst_addr>
    target_struct_offset = <struct_offset>   # e.g. 0x1A8 from "mov [rcx+1A8h], eax"
    min_sig_bytes = 6
    max_sig_bytes = 96
    max_instructions = 64

    # --- Binary search wrapper (IDA 9.0+ find_bytes -> older bin_search fallback) ---
    def raw_bin_search(ea, max_ea, data, mask, flags=0):
        if hasattr(ida_bytes, 'find_bytes'):
            return ida_bytes.find_bytes(data, ea, range_end=max_ea, mask=mask, flags=flags)
        return ida_bytes.bin_search(ea, max_ea, data, mask, len(data), flags)

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

    def find_struct_disp_matches(insn, raw, expected):
        hits = []
        for op in insn.ops:
            ot = int(op.type)
            if ot == int(idaapi.o_void):
                continue
            if ot not in (int(idaapi.o_displ), int(idaapi.o_mem), int(idaapi.o_imm)):
                continue

            for attr in ("offb", "offo"):
                off = int(getattr(op, attr, 0))
                if off <= 0 or off >= insn.size:
                    continue

                sizes = []
                dsz = ida_ua.get_dtype_size(getattr(op, "dtype", getattr(op, "dtyp", 0)))
                if dsz > 0:
                    sizes.append(dsz)
                for s in (1, 2, 4, 8):
                    if s not in sizes:
                        sizes.append(s)

                for sz in sizes:
                    if off + sz > insn.size:
                        continue
                    unsigned_val = int.from_bytes(raw[off:off + sz], "little", signed=False)
                    signed_val = int.from_bytes(raw[off:off + sz], "little", signed=True)
                    expected_mod = expected & ((1 << (8 * sz)) - 1)
                    if unsigned_val == expected_mod or signed_val == expected:
                        hits.append((off, sz, unsigned_val, signed_val))

        uniq = []
        seen = set()
        for h in hits:
            key = (h[0], h[1])
            if key not in seen:
                seen.add(key)
                uniq.append(h)
        return uniq

    disp_hits = find_struct_disp_matches(insn0, raw0, target_struct_offset)
    if not disp_hits:
        print(json.dumps({
            "inst_va": hex(target_inst),
            "inst_bytes": " ".join(f"{b:02X}" for b in raw0),
            "struct_offset": hex(target_struct_offset),
            "error": "target instruction does not contain the expected struct offset",
            "status": "failed"
        }))
        return

    # Prefer the largest matching displacement size so we lock the full offset bytes.
    disp_hits.sort(key=lambda x: (x[1], -x[0]), reverse=True)
    disp_off, disp_size, _, _ = disp_hits[0]

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
            "struct_sig": phase1_sig,
            "sig_bytes": phase1_boundary,
            "struct_sig_va": hex(target_inst),
            "offset_sig_disp": 0,
            "struct_inst_length": target_inst_len,
            "struct_disp_offset": disp_off,
            "struct_disp_size": disp_size,
            "struct_offset": hex(target_struct_offset),
            "status": "success"
        }))
        return

    # Forward-only expansion exhausted without finding a unique signature.
    print(json.dumps({
        "struct_sig_va": hex(target_inst),
        "struct_offset": hex(target_struct_offset),
        "first_inst_bytes": " ".join(f"{b:02X}" for b in raw0),
        "total_fwd_tokens": len(fwd_tokens),
        "sig_full_fwd": " ".join(fwd_tokens),
        "error": "no unique signature found with forward-only expansion",
        "status": "failed"
    }))

main()
"""
```

**Result handling:**
- `status == "success"` -> Use `struct_sig` directly as final signature.
- `status == "failed"` -> See Step 2.

### 2. Iterate if Needed

If Step 1 returns `status: "failed"`:
1. Increase `max_sig_bytes` (e.g. from `96` to `192`) and re-run Step 1.
2. Increase `max_instructions` (e.g. from `64` to `128`) if function instructions are short.
3. If still not unique, use a different instruction that references the same struct offset and re-run.

### 3. Continue with Unfinished Tasks

If we are called by a task from a task list / parent SKILL, restore and continue with the unfinished tasks.

## Output Format

Required:
- `struct_sig`: Space-separated hex bytes with `??` for wildcards.

Recommended metadata:
- `struct_sig_va`: VA of signature start (always equals target instruction VA since `offset_sig_disp` is always `0`).
- `offset_sig_disp`: Always `0` — signature always starts at the target instruction.
- `struct_inst_length`: Length of the target instruction in bytes.
- `struct_disp_offset`: Byte position of the struct offset displacement within the **signature** (= displacement position within target instruction, since `offset_sig_disp` is always `0`).
- `struct_disp_size`: Displacement byte size.
- `struct_offset`: The expected struct offset used for validation.

### Example Output

```yaml
struct_sig: "C7 81 A8 01 00 00 01 00 00 00 48 8B ?? ?? ?? ?? 48 85 C0 74 ??"
struct_sig_va: 0x180123456
offset_sig_disp: 0
struct_inst_length: 10
struct_disp_offset: 2
struct_disp_size: 4
struct_offset: 0x1A8
```
