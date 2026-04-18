# vfunc_sig

## Overview
`vfunc_sig` is an alternate virtual-function relocation signature used when `func_sig` is unavailable or unstable.

## Field Context
- Related YAML fields: `vfunc_sig`, `vtable_name`, `vfunc_index`, `vfunc_offset` (and occasionally `vfunc_inst_offset`).
- Typical role: anchor to a virtual-call pattern tied to a vtable slot.

## Generation Principle
- In this repository, `vfunc_sig` is typically produced by the skill `/generate-signature-for-vfuncoffset`.
- The anchor is a virtual-call instruction encoding vtable slot displacement (e.g., indirect call/jump via `[reg+imm]`).
- Signature growth is forward from the target instruction; default workflow documents `vfunc_sig_disp = 0`.
- Candidates are validated by uniqueness (`find_bytes`) and robustness.
- The shortest valid unique pattern is selected.
- Slot-related displacement bytes are intentionally preserved/represented so the signature remains slot-specific.

## Usage Method
- In `preprocess_func_sig_via_mcp`, `vfunc_sig` is a fallback path used when old YAML has no `func_sig`.
- Required metadata:
  - `vfunc_sig`
  - `vtable_name`
  - and `vfunc_index` or `vfunc_offset`
- Flow:
  1. Unique-match `vfunc_sig` in new binary.
  2. Load/generate target vtable YAML.
  3. Resolve function VA via `vtable_entries[vfunc_index]`.
  4. Query function info and emit function YAML with vfunc metadata.

## Downstream Use
- Dist gamedata modules mainly consume `vfunc_index` (offset metadata), not `vfunc_sig` directly.
- `vfunc_sig` is primarily used by analysis/preprocess relocation pipelines.

## Practical Notes
- Use when function-head signatures are weak but vtable slot identity is stable.
- Keep `vfunc_index` / `vfunc_offset` internally consistent (`offset = index * 8` in current 64-bit assumptions).
- Reject non-unique signatures.
