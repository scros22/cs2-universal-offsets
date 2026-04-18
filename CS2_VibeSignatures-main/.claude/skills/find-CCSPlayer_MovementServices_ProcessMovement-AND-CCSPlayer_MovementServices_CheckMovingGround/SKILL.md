---
name: find-CCSPlayer_MovementServices_ProcessMovement-AND-CCSPlayer_MovementServices_CheckMovingGround
description: |
  Find and identify CCSPlayer_MovementServices_ProcessMovement and CCSPlayer_MovementServices_CheckMovingGround functions in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the movement processing functions.
  Trigger: CCSPlayer_MovementServices_ProcessMovement, CCSPlayer_MovementServices_CheckMovingGround, ProcessMovement, CheckMovingGround, movement services, Force Down
disable-model-invocation: true
---

# CCSPlayer_MovementServices_ProcessMovement & CheckMovingGround Location Workflow

## Overview

Locate two related functions in CS2 server binary:
- `CCSPlayer_MovementServices_ProcessMovement` — Main movement tick function
- `CCSPlayer_MovementServices_CheckMovingGround` — Virtual function called within ProcessMovement

## Location Steps

### 1. Search for Signature String

Use `find_regex` to search for the `Force Down` debug string:

```
mcp__ida-pro-mcp__find_regex(pattern="\\[%s\\] Force Down: %s, Next: %s")
```

### 2. Find Cross-References to String

Use `xrefs_to` on the string address to find the referencing function (a ForceButtonDown-like function).

### 3. Locate ProcessMovement via Global Variable

Decompile the function from step 2. Near the top, find a global variable comparison pattern:

```c
v5 = qword_XXXXXXXX == v4;  // v4 = this->field_0x38
if ( qword_XXXXXXXX == v4 )
```

Use `xrefs_to` on that global variable. Look for a `mov` instruction that **writes** to it:

```asm
mov cs:qword_XXXXXXXX, rax
```

The function containing this write instruction is `CCSPlayer_MovementServices::ProcessMovement`.

### 4. Rename ProcessMovement

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<ProcessMovement_addr>", "name": "CCSPlayer_MovementServices::ProcessMovement"}})
```

### 5. Identify CheckMovingGround Virtual Call

Decompile ProcessMovement. Immediately after the global variable write, find the virtual call at vtable offset `0x140`:

```c
qword_XXXXXXXX = *(_QWORD *)(pMovementServices + 56);
(*(void (__fastcall **)(__int64))(*(_QWORD *)pMovementServices + 320LL))(pMovementServices);// This is CCSPlayer_MovementServices_CheckMovingGround, func_offset = 320LL, vfunc_index = 40
```

Note the instruction address of this `call qword ptr [rax+140h]` for signature generation.
5. 

### 6. Load CCSPlayer_MovementServices VTable, Resolve CheckMovingGround Function Address

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CCSPlayer_MovementServices`

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `vtable_va`, `vtable_numvfunc` and `vtable_entries` for subsequent steps.

`CCSPlayer_MovementServices vtable_entries[vfunc_index] = CCSPlayer_MovementServices_CheckMovingGround`

### 7. Rename CheckMovingGround

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<CheckMovingGround_addr>", "name": "CCSPlayer_MovementServices_CheckMovingGround"}})
```

### 8. Generate Signatures

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CCSPlayer_MovementServices_ProcessMovement`.

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `CCSPlayer_MovementServices_CheckMovingGround`, with `inst_addr` being the address of the `call qword ptr [rax+140h]` instruction inside ProcessMovement, and `vfunc_offset` = `0x140`.

### 9. Write YAML Output

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results for `CCSPlayer_MovementServices_ProcessMovement`.

Required parameters:
- `func_name`: `CCSPlayer_MovementServices_ProcessMovement`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 8

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results for `CCSPlayer_MovementServices_CheckMovingGround`.

Required parameters:
- `func_name`: `CCSPlayer_MovementServices_CheckMovingGround`
- `func_addr`: The function address from step 6
- `vfunc_sig`: The validated vfunc signature from step 8

VTable parameters:
- `vtable_name`: `CCSPlayer_MovementServices`
- `vfunc_offset`: `0x140`
- `vfunc_index`: `40` (0x140 / 8)

## Function Characteristics

### CCSPlayer_MovementServices_ProcessMovement

Main movement tick function. Key behaviors:
- Saves movement state, sets global pawn pointer from `this->field_0x38`
- Calls multiple virtual functions for movement processing
- Handles state rollback if needed
- Clears global pawn pointer on exit

### CCSPlayer_MovementServices_CheckMovingGround

Virtual function at vtable index 40 (offset 0x140). Called within ProcessMovement to check/handle ground movement state.

## VTable Information

- **VTable Name**: `CCSPlayer_MovementServices`
- **VTable Mangled Name**:
  - Windows: `??_7CCSPlayer_MovementServices@@6B@`
  - Linux: `_ZTV16CCSPlayer_MovementServices`
- **CheckMovingGround VTable Offset**: `0x140` (may change with game updates)
- **CheckMovingGround VTable Index**: `40` (may change with game updates)

## Output YAML Files

- `CCSPlayer_MovementServices_ProcessMovement.windows.yaml` / `.linux.yaml`
- `CCSPlayer_MovementServices_CheckMovingGround.windows.yaml` / `.linux.yaml`
