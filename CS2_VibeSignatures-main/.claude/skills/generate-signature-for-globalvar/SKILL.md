---
name: generate-signature-for-globalvar
description: |
  Generate and validate unique byte signatures for global variable using IDA Pro MCP. Use this skill when you need to create a pattern-scanning signature for a global variable that can reliably locate it across binary updates.
  Triggers: global variable signature, signature for global variable
---

# Generate Signature for Global Variable

Generate a unique hex byte signature that locates an **instruction accessing a global variable** using fully programmatic wildcard detection and validation — no manual byte analysis required.

## Core Concept

Since global variable addresses change between binary updates, we don't signature the GV itself. Instead, we:
1. Find an instruction that **references** the global variable (mov/lea/cmp/etc.)
2. Generate a signature to locate that **instruction**
3. At runtime, parse the instruction to **resolve** the actual GV address

### RIP-Relative Addressing (x86-64)

In x86-64, most global variable accesses use RIP-relative addressing:
```
GV_Address = Instruction_Address + Instruction_Length + RIP_Offset
```

Where:
- `Instruction_Address` = address found by pattern scan
- `Instruction_Length` = total bytes of the instruction (opcode + ModR/M + offset)
- `RIP_Offset` = signed 32-bit displacement (last 4 bytes of instruction)

## Prerequisites

- Global variable address. `qword_XXXXXX` for example.
- IDA Pro MCP connection

## Method

### 1. Generate and Validate Signature (Single Step)

Use a single `py_eval` call that:
- Discovers candidate instructions accessing the GV via `DataRefsTo`
- Verifies each candidate resolves to the target GV via RIP-relative displacement
- Collects instruction stream with auto-wildcarding for each candidate
- Tracks instruction boundaries so prefixes always cover complete instructions
- Progressively tests at each instruction boundary via binary search
- Outputs the shortest unique signature with full metadata

**Note**: If you already know the GV-accessing instruction address, set `target_inst = <inst_addr>`. If you know the containing function, set `target_func = <func_addr>`.

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi, ida_bytes, idautils, ida_ua, ida_segment, json

def main():
    target_gv = <gv_addr>
    target_inst = None       # Set to instruction address if known, e.g. 0x1804F3DF3
    target_func = None       # Set to function address to restrict search, e.g. 0x1804F3DA0
    min_sig_bytes = 8
    max_sig_bytes = 96
    max_instructions = 64
    max_candidates = 32

    # --- Binary search wrapper (IDA 9.0+ find_bytes -> older bin_search fallback) ---
    def raw_bin_search(ea, max_ea, data, mask, flags=0):
        if hasattr(ida_bytes, 'find_bytes'):
            return ida_bytes.find_bytes(data, ea, range_end=max_ea, mask=mask, flags=flags)
        return ida_bytes.bin_search(ea, max_ea, data, mask, len(data), flags)

    # --- Search bounds ---
    seg = ida_segment.get_segm_by_name(".text")
    if seg:
        search_start, search_end = seg.start_ea, seg.end_ea
    else:
        search_start, search_end = idaapi.cvar.inf.min_ea, idaapi.cvar.inf.max_ea

    def resolve_disp_off(insn_ea, insn, raw):
        cand_offsets = set()
        for op in insn.ops:
            if int(op.type) == int(idaapi.o_void):
                continue
            offb = int(getattr(op, 'offb', 0))
            offo = int(getattr(op, 'offo', 0))
            if offb > 0 and offb + 4 <= insn.size:
                cand_offsets.add(offb)
            if offo > 0 and offo + 4 <= insn.size:
                cand_offsets.add(offo)
        for off in sorted(cand_offsets):
            disp_i32 = int.from_bytes(raw[off:off + 4], 'little', signed=True)
            resolved = (insn_ea + insn.size + disp_i32) & 0xFFFFFFFFFFFFFFFF
            if resolved == target_gv:
                return off
        return None

    def collect_and_validate(inst_ea, disp_off):
        f = idaapi.get_func(inst_ea)
        if not f:
            return None
        limit_end = min(f.end_ea, inst_ea + max_sig_bytes)
        sig_tokens = []
        inst_boundaries = []
        cursor = inst_ea
        first_len = None
        while cursor < f.end_ea and cursor < limit_end and len(sig_tokens) < max_sig_bytes:
            insn = idautils.DecodeInstruction(cursor)
            if not insn or insn.size <= 0:
                break
            raw = ida_bytes.get_bytes(cursor, insn.size)
            if not raw:
                break
            wild = set()
            for op in insn.ops:
                ot = int(op.type)
                if ot == int(idaapi.o_void):
                    continue
                if ot in (int(idaapi.o_imm), int(idaapi.o_near), int(idaapi.o_far), int(idaapi.o_mem), int(idaapi.o_displ)):
                    offb = int(getattr(op, 'offb', 0))
                    if offb > 0 and offb < insn.size:
                        dsz = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))
                        if dsz <= 0:
                            dsz = insn.size - offb
                        for i in range(offb, min(insn.size, offb + dsz)):
                            wild.add(i)
                    offo = int(getattr(op, 'offo', 0))
                    if offo > 0 and offo < insn.size:
                        dsz2 = ida_ua.get_dtype_size(getattr(op, 'dtype', getattr(op, 'dtyp', 0)))
                        if dsz2 <= 0:
                            dsz2 = insn.size - offo
                        for i in range(offo, min(insn.size, offo + dsz2)):
                            wild.add(i)
            b0 = raw[0]
            if b0 in (0xE8, 0xE9, 0xEB):
                for i in range(1, insn.size):
                    wild.add(i)
            elif b0 == 0x0F and insn.size >= 2 and (raw[1] & 0xF0) == 0x80:
                for i in range(2, insn.size):
                    wild.add(i)
            elif 0x70 <= b0 <= 0x7F:
                for i in range(1, insn.size):
                    wild.add(i)
            if cursor == inst_ea:
                first_len = insn.size
                for i in range(disp_off, min(insn.size, disp_off + 4)):
                    wild.add(i)
            for idx in range(insn.size):
                sig_tokens.append("??" if idx in wild else f"{raw[idx]:02X}")
            inst_boundaries.append(len(sig_tokens))
            cursor += insn.size
        if not sig_tokens or first_len is None:
            return None
        for boundary in inst_boundaries:
            if boundary < min_sig_bytes:
                continue
            prefix_tokens = sig_tokens[:boundary]
            if all(t == "??" for t in prefix_tokens):
                continue
            data = bytes(0 if t == "??" else int(t, 16) for t in prefix_tokens)
            mask = bytes(0x00 if t == "??" else 0xFF for t in prefix_tokens)
            flags = ida_bytes.BIN_SEARCH_FORWARD | ida_bytes.BIN_SEARCH_NOBREAK
            matches = []
            ea = raw_bin_search(search_start, search_end, data, mask, flags)
            while ea != idaapi.BADADDR and len(matches) < 2:
                matches.append(ea)
                ea = raw_bin_search(ea + 1, search_end, data, mask, flags)
            if len(matches) == 1 and matches[0] == inst_ea:
                return {
                    "gv_sig": " ".join(prefix_tokens),
                    "sig_bytes": boundary,
                    "gv_sig_va": hex(inst_ea),
                    "gv_inst_length": first_len,
                    "gv_inst_disp": disp_off,
                }
        return None

    # --- Discover candidate GV-accessing instructions ---
    candidates_tried = 0
    best = None
    seen = set()

    def try_candidate(iea):
        nonlocal candidates_tried, best
        if iea in seen:
            return
        seen.add(iea)
        insn = idautils.DecodeInstruction(iea)
        if not insn or insn.size <= 0:
            return
        raw = ida_bytes.get_bytes(iea, insn.size)
        if not raw:
            return
        doff = resolve_disp_off(iea, insn, raw)
        if doff is None:
            return
        candidates_tried += 1
        result = collect_and_validate(iea, doff)
        if result is not None:
            if best is None or result["sig_bytes"] < best["sig_bytes"]:
                best = result

    if target_inst is not None:
        try_candidate(target_inst)
    elif target_func is not None:
        f = idaapi.get_func(target_func)
        if f:
            ea = f.start_ea
            while ea < f.end_ea and candidates_tried < max_candidates:
                fl = ida_bytes.get_full_flags(ea)
                if ida_bytes.is_code(fl):
                    try_candidate(ea)
                    if best is not None:
                        break
                nea = ida_bytes.next_head(ea, f.end_ea)
                if nea == idaapi.BADADDR or nea <= ea:
                    break
                ea = nea
    else:
        for ref in idautils.DataRefsTo(target_gv):
            if candidates_tried >= max_candidates:
                break
            fl = ida_bytes.get_full_flags(ref)
            if not ida_bytes.is_code(fl):
                continue
            try_candidate(ref)
            if best is not None:
                break

    if best:
        best["gv_va"] = hex(target_gv)
        best["gv_rva"] = hex(target_gv - idaapi.get_imagebase())
        best["gv_inst_offset"] = 0
        best["status"] = "success"
        print(json.dumps(best))
    else:
        print(json.dumps({
            "gv_va": hex(target_gv),
            "candidates_tried": candidates_tried,
            "error": "no unique gv-access signature found",
            "status": "failed"
        }))

main()
"""
```

**Result handling:**
- `status == "success"` → Use `gv_sig` and metadata directly
- `status == "failed"` → See Step 2

### 2. Iterate if Needed

If Step 1 returns `status: "failed"`:
1. Increase `max_sig_bytes` (e.g., to 192) and re-run Step 1
2. Specify a different `target_func` to find more candidates
3. Re-run until unique

### 3. Continue with Unfinished Tasks

If we are called by a task from a task list / parent SKILL, restore and continue with the unfinished tasks.

## Output Format

Provide the following information for runtime GV resolution:

### Required Output Fields

1. **gv_sig**: Space-separated hex bytes with `??` for wildcards
1. **gv_sig_va**: The virtual address that the signature matches
2. **gv_inst_offset**: Always `0` (signature starts at the GV-accessing instruction)
3. **gv_inst_length**: Total length of the GV-accessing instruction (from output metadata)
4. **gv_inst_disp**: Position of the 4-byte RIP-relative offset within the instruction (from output metadata)

### Example Output

```yaml
gv_sig: "48 8B 1D ?? ?? ?? ?? 48 85 DB 0F 84 ?? ?? ?? ?? BD FF FF 00 00"
gv_sig_va: 0x1804f3df3     # The virtual address that the signature matches
gv_inst_offset: 0          # GV instruction starts at signature start
gv_inst_length: 7          # 48 8B 1D XX XX XX XX = 7 bytes
gv_inst_disp:   3          # Displacement offset start at position 3 (after 48 8B 1D)
```

### Runtime Resolution Formula

At runtime, after pattern scan finds the signature at address `scan_result`:

```cpp
// C++ example
uint8_t* inst_addr = scan_result + inst_offset;
int32_t rip_offset = *(int32_t*)(inst_addr + inst_disp);
void* gv_address = inst_addr + inst_length + rip_offset;
```

```python
# Python example
import struct
inst_addr = scan_result + inst_offset
rip_offset = struct.unpack('<i', memory[inst_addr + inst_disp : inst_addr + inst_disp + 4])[0]
gv_address = inst_addr + inst_length + rip_offset
```
