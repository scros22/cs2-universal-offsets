# offset_sig

## Overview
`offset_sig` is the relocation signature used to recover struct-member offsets across versions.

## Field Context
- Related YAML fields: `struct_name`, `member_name`, `offset`, `size`, `offset_sig` (optional `offset_sig_disp`).

## Generation Principle
- In this repository, authoring usually follows the struct-offset signature workflow (`.claude/skills/generate-signature-for-structoffset/SKILL.md`).
- Signature is built near an instruction that encodes/uses the target member offset.
- Volatile bytes are wildcarded (`??`) to improve cross-version robustness.
- Observed outputs generally use forward anchoring (signature starts at target instruction, `offset_sig_disp` omitted/0).
- Acceptance goal: unique matching plus reliable offset re-derivation.

## Usage Method
- In `preprocess_struct_offset_sig_via_mcp`, old `offset_sig` is reused to recover the new `offset`.
- Flow:
  1. Load `struct_name`, `member_name`, `offset_sig`, optional `offset_sig_disp`, optional `size`.
  2. Unique-match `offset_sig` -> `sig_addr`.
  3. Compute instruction address: `inst_addr = sig_addr + offset_sig_disp` (default 0).
  4. Decode instruction and inspect operand positions (`offb/offo`) and candidate sizes.
  5. Extract displacement/immediate candidates; prefer candidates matching old offset when available.
  6. Emit new YAML with updated `offset`, carrying `offset_sig` and optional metadata.

## Practical Notes
- Multi-hit `offset_sig` is rejected.
- Non-zero `offset_sig_disp` means signature starts before target instruction.
- `offset` may change across versions; `offset_sig` should remain stable enough to re-derive it.
- Weak signatures (too short/too wildcarded) reduce long-term reliability.
