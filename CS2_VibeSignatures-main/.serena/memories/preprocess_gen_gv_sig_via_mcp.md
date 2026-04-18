# preprocess_gen_gv_sig_via_mcp

## Overview
`preprocess_gen_gv_sig_via_mcp` is an async preprocessing function in `ida_analyze_util.py` for generating the shortest unique signature from a known global-variable address. It enforces that the signature match address must be the instruction accessing that global variable (`gv_inst_offset = 0`), and finally returns a field set that can be written directly into global-variable YAML.

## Responsibilities
- Parse and validate input parameters (`gv_va`, optional `gv_access_inst_va/gv_access_func_va`, length/candidate limits, extra wildcard offsets).
- Use MCP `py_eval` to collect candidate instructions that access the target GV and their expandable instruction streams.
- Incrementally grow signature prefix on instruction boundaries in Python, and validate via MCP `find_bytes` with "unique match + match address equals candidate GV access instruction address".
- Select the shortest usable signature among multiple candidates, and produce `gv_va/gv_rva/gv_sig/gv_sig_va/gv_inst_*`.

## Files Involved (no line numbers)
- ida_analyze_util.py

## Architecture
Overall flow is "two-phase generation + validation":
1. **IDA-side candidate discovery (`py_eval`)**
   - `_resolve_disp_off`: locate 4-byte displacement field from operand `offb/offo`, verify `inst_ea + insn.size + disp_i32 == target_gv`.
   - `_collect_sig_stream`: collect instruction stream forward from candidate instruction (bounded by `max_sig_bytes`, `max_instructions`), and mark wildcard bytes in each instruction:
     - Operand bytes (`o_imm/o_near/o_far/o_mem/o_displ`)
     - Jump/call displacement bytes (`E8/E9/EB`, `0F 8x`, `70-7F`)
     - Displacement field of first GV-access instruction (`disp_off..disp_off+4`)
   - Candidate source priority:
     - If `gv_access_inst_va` is specified: only try this address
     - Else if `gv_access_func_va` is specified: iterate code heads in that function and try each
     - Else: iterate code refs in `DataRefsTo(target_gv)`
2. **Python-side signature search (`find_bytes`)**
   - Flatten candidate instruction stream into tokens, and append absolute-offset wildcards using `extra_wildcard_offsets`.
   - Test prefixes only on complete-instruction boundaries (length >= `min_sig_bytes`, and cannot be all `??`).
   - For each prefix call `find_bytes(limit=2)`: must satisfy `n == 1`, and unique match address must equal current candidate `gv_inst_va`.
   - Pick the shortest signature across all candidates as `best`.
3. **Result packaging**
   - Return on success:
     - `gv_inst_offset` is fixed to `0`
     - `gv_inst_length/gv_inst_disp` come from first GV-access instruction
     - `gv_rva = gv_va - image_base`

```mermaid
flowchart TD
    A[Parameter parse/validation] --> B[py_eval: generate candidates]
    B --> C{candidate list not empty?}
    C -- No --> Z[Return None]
    C -- Yes --> D[Build tokens + wildcards per candidate]
    D --> E[Grow length by instruction boundaries]
    E --> F[find_bytes(limit=2) uniqueness check]
    F --> G{Unique and match==gv_inst_va?}
    G -- No --> E
    G -- Yes --> H[Update best (shortest)]
    H --> I{More candidates?}
    I -- Yes --> D
    I -- No --> J{best exists?}
    J -- No --> Z
    J -- Yes --> K[Return GV YAML fields]
```

## Dependencies
- Internal dependency: `parse_mcp_result` (parsing `py_eval/find_bytes` returns)
- MCP tools: `py_eval`, `find_bytes`
- IDA Python API (in `py_eval` script): `idaapi`, `ida_bytes`, `idautils`, `ida_ua`
- Standard library: `json`

## Notes
- This function requires `image_base` to support integer subtraction; return stage computes `gv_rva = gv_va - image_base`.
- It only recognizes/validates **4-byte displacement** GV access patterns (`disp_i32`), and does not cover other addressing encodings.
- `extra_wildcard_offsets` are absolute offsets relative to signature start; too many offsets can degrade signatures and make unique matching impossible.
- Uniqueness check requires not only a unique `find_bytes` result, but also exact match to candidate `gv_inst_va`, preventing "unique signature but wrong anchor".
- No direct callers are currently found in this repository; existing GV preprocessing flow mainly uses `preprocess_gv_sig_via_mcp` (reuse old signatures).

## Callers (optional)
- No direct callers in current repository (text search only matches the function definition itself)