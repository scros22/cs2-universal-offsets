# gv_sig

## Overview
`gv_sig` is the relocation signature for global-variable discovery, anchored at a GV-access instruction.

## Field Context
- Related YAML fields: `gv_name`, `gv_va`, `gv_rva`, `gv_sig`, `gv_sig_va`, `gv_inst_offset`, `gv_inst_length`, `gv_inst_disp`.

## Generation Principle
- Canonical auto-generation is implemented by `preprocess_gen_gv_sig_via_mcp` in `ida_analyze_util.py`.
- Input anchor is known `gv_va`, optionally constrained by `gv_access_inst_va` / `gv_access_func_va`.
- Candidate GV-access instructions are discovered in priority order:
  1. explicit `gv_access_inst_va`
  2. instruction scan in `gv_access_func_va`
  3. fallback `DataRefsTo(gv_va)` code refs
- For each candidate instruction, RIP-relative displacement (`disp_i32`) is validated to resolve to `gv_va`.
- Signature is collected forward on instruction boundaries.
- Volatile bytes are wildcarded (`??`), including the GV displacement bytes themselves.
- Candidate acceptance requires:
  - exactly one `find_bytes(limit=2)` match
  - matched address equals candidate GV-access instruction address
- The shortest accepted candidate becomes `gv_sig`.
- Generator outputs `gv_sig_va`, `gv_inst_offset` (currently `0`), `gv_inst_length`, `gv_inst_disp`, and `gv_va/gv_rva`.

## Usage Method
- In `preprocess_gv_sig_via_mcp`, old `gv_sig` + `gv_inst_*` metadata are reused to relocate GV address in new binary.
- Flow:
  1. Load `gv_sig`, `gv_inst_offset`, `gv_inst_length`, `gv_inst_disp`.
  2. Unique-match `gv_sig` -> `gv_sig_va`.
  3. Compute instruction address: `inst_addr = gv_sig_va + gv_inst_offset`.
  4. Read instruction bytes (`gv_inst_length`) and parse displacement at `gv_inst_disp`.
  5. Recover GV address:
     `gv_va = inst_addr + gv_inst_length + disp_i32`.
  6. Recompute `gv_rva` and write updated GV YAML.

## Downstream Use
- Current dist gamedata updaters mainly consume function/offset/patch outputs.
- `gv_sig` is primarily used by analysis/preprocess relocation flow.

## Practical Notes
- Uniqueness must be instruction-level strict.
- Over-wildcarding hurts uniqueness; under-wildcarding hurts portability.
- `gv_inst_*` must stay consistent with RIP-relative disp32 model.
