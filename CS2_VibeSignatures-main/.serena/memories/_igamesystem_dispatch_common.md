# _igamesystem_dispatch_common

## Overview
`ida_preprocessor_scripts/_igamesystem_dispatch_common.py` is the shared preprocess entry for IGameSystem dispatch-style skills. The design is deterministic: collect all dispatch calls first, then map targets by stable `vfunc_index`/`vfunc_offset` ordering.

## Current Design
### 1) Collect all dispatch entries
- `_build_dispatch_py_eval(...)` collects all candidates from the source function (or resolved internal wrapper).
- Windows path: scans `lea rdx, callback`, then parses callback `call/jmp [reg+disp]`.
- Linux path: scans `mov esi/rsi, odd_imm` + next `call`, computes `vfunc_off = imm - 1`.
- Only non-negative, 8-byte aligned offsets are accepted.

### 2) Strict count validation
- `expected_dispatch_count` controls the required total entry count.
- Validation requires `len(entries) == expected_dispatch_count`.
- If omitted:
  - without `dispatch_rank`: defaults to `target_count`
  - with `dispatch_rank`: defaults to `max(dispatch_rank) + 1`

### 3) Stable mapping with dispatch_rank
- `target_specs` fields:
  - `target_name` (required)
  - `rename_to` (optional)
  - `dispatch_rank` (optional)
- If `dispatch_rank` is used:
  - all specs must provide it
  - ranks must be unique and non-negative
  - entries are sorted by `(vfunc_index, vfunc_offset)`
  - each target picks by rank
- If `dispatch_rank` is not used:
  - mapping uses scan order for selected entries
  - `expected_dispatch_count` must equal `target_count`

## Public API Snapshot
`preprocess_igamesystem_dispatch_skill(...)` relevant args:
- `target_specs`
- `multi_order` (`scan` / `index`)
- `expected_dispatch_count=None`
- `debug=False`

## Behavior Notes
- Fail-fast on validation/extraction mismatch (`False` return).
- `multi_order == "index"` is honored for multi-target mapping.
- If `dispatch_rank` is present, stable index sorting is forced even with `multi_order="scan"`.
- Internal/callback renaming remains best-effort (non-fatal).

## Updated Callers (current)
### SpawnGroup series
- `find-IGameSystem_PreSpawnGroupLoad.py`
  - `dispatch_rank=0`
  - `EXPECTED_DISPATCH_COUNT=2`
- `find-IGameSystem_PostSpawnGroupLoad.py`
  - `dispatch_rank=1`
  - `EXPECTED_DISPATCH_COUNT=2`
- `find-IGameSystem_PostSpawnGroupUnload.py`
  - `dispatch_rank=1`
  - `EXPECTED_DISPATCH_COUNT=2`
- `find-IGameSystem_PreSpawnGroupUnload.py`
  - single-target default mapping

### ClientPreEntityThink case
- `find-IGameSystem_ClientPreEntityThink.py`
  - observed dispatch indices: `22 / 23 / 24`
  - `IGameSystem_ClientPreEntityThink` uses `dispatch_rank=0` (index 22)
  - `EXPECTED_DISPATCH_COUNT=3`

## Rationale
Mapping now relies on deterministic index ordering plus strict entry-count assertions, which is more stable under compiler/reordering differences.

### SpawnGroupPrecache / SpawnGroupUncache
- `find-IGameSystem_SpawnGroupPrecache.py`
  - source: `CSpawnGroupMgrGameSystem_SpawnGroupPrecache`
  - single-target default mapping (1 dispatch)
- `find-IGameSystem_SpawnGroupUncache.py`
  - source: `CSpawnGroupMgrGameSystem_SpawnGroupActuallyShutdown`
  - `dispatch_rank=0`
  - `EXPECTED_DISPATCH_COUNT=2`

## Files Involved
- `ida_preprocessor_scripts/_igamesystem_dispatch_common.py`
- `ida_preprocessor_scripts/find-IGameSystem_PreSpawnGroupLoad.py`
- `ida_preprocessor_scripts/find-IGameSystem_PostSpawnGroupLoad.py`
- `ida_preprocessor_scripts/find-IGameSystem_PostSpawnGroupUnload.py`
- `ida_preprocessor_scripts/find-IGameSystem_PreSpawnGroupUnload.py`
- `ida_preprocessor_scripts/find-IGameSystem_SpawnGroupPrecache.py`
- `ida_preprocessor_scripts/find-IGameSystem_SpawnGroupUncache.py`
- `ida_preprocessor_scripts/find-IGameSystem_ClientPreEntityThink.py`
