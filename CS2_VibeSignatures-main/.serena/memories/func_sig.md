# func_sig

## Overview
`func_sig` is the primary function relocation signature used in `{symbol}.{platform}.yaml`.

## Field Context
- Related YAML fields: `func_name`, `func_va`, `func_rva`, `func_size`, `func_sig`.
- Typical role: relocate the target function in a new binary version via unique byte-pattern match.

## Generation Principle
- Canonical auto-generation is implemented by `preprocess_gen_func_sig_via_mcp` in `ida_analyze_util.py`.
- Input is a known function start address (`func_va`).
- Bytes are collected from the function head instruction-by-instruction.
- Volatile bytes are wildcarded (`??`), including immediates/displacements and relative branch/call offsets.
- Candidate signatures grow only at instruction boundaries.
- Every candidate is checked by `find_bytes(limit=2)` for uniqueness.
- A candidate is accepted only if it uniquely matches and the match address equals the target function head.
- The shortest accepted candidate is emitted as `func_sig`.

## Usage Method
- In version-to-version preprocessing (`preprocess_func_sig_via_mcp`), `func_sig` is the primary relocation anchor.
- Flow:
  1. Run `find_bytes` with old `func_sig`.
  2. Require exactly one match.
  3. Query IDA function info at match address (`func_va`, `func_size`).
  4. Recompute `func_rva` and write new YAML.
- Important behavior: if old YAML already has `func_sig`, preprocessing chooses this path first and does not auto-fallback to `vfunc_sig` on failure.

## Downstream Use
- Dist gamedata modules mainly consume `func_sig` (converted to target pattern formats like CSS/VDF/Swiftly variants).

## Practical Notes
- Keep uniqueness strict (`==1` match); multi-hit signatures are invalid.
- Ensure signature anchors function head, not an interior address.
- Overly short signatures may pass today but become unstable in future updates.
