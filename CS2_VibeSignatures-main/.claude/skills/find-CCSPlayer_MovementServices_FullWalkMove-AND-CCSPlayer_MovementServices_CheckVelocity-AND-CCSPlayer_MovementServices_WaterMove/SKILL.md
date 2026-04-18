---
name: find-CCSPlayer_MovementServices_FullWalkMove-AND-CCSPlayer_MovementServices_CheckVelocity-AND-CCSPlayer_MovementServices_WaterMove
description: |
  Find and identify the CCSPlayer_MovementServices_FullWalkMove, CCSPlayer_MovementServices_CheckVelocity, and CCSPlayer_MovementServices_WaterMove functions in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the ground walk movement, velocity sanitization, and water movement functions.
  Trigger: FullWalkMove, CheckVelocity, WaterMove, FullWalkMovePreMove, NaN velocity, water current, ground movement
disable-model-invocation: true
---

# CCSPlayer_MovementServices_FullWalkMove & CheckVelocity & WaterMove Location Workflow

## Overview

Locate three related functions in CS2 server binary:
- `CCSPlayer_MovementServices_FullWalkMove` — Main ground walk movement logic
- `CCSPlayer_MovementServices_CheckVelocity` — NaN/bounds velocity and origin sanitizer
- `CCSPlayer_MovementServices_WaterMove` — Water current/drag pre-processing

All three are called sequentially in the ground-movement branch of ProcessMovement:
```c
CCSPlayer_MovementServices_WaterMove(a1, a2);
CCSPlayer_MovementServices_CheckVelocity(a1, a2, "FullWalkMovePreMove");
CCSPlayer_MovementServices_FullWalkMove(a1, a2);
```

## Location Steps

### 1. Search for Signature String

Use `find_regex` to search for the `FullWalkMovePreMove` string:

```
mcp__ida-pro-mcp__find_regex(pattern="FullWalkMovePreMove")
```

### 2. Find Cross-References to String

Use `xrefs_to` on the string address to find the referencing function (the parent ProcessMovement-like function):

```
mcp__ida-pro-mcp__xrefs_to(addrs="<string_addr>")
```

### 3. Decompile the Parent Function

Decompile the function that references the string:

```
mcp__ida-pro-mcp__decompile(addr="<parent_func_addr>")
```

In the decompiled output, locate the ground-movement branch (guarded by a flag check like `(*(_BYTE *)(*(_QWORD *)(a1 + 56) + 904LL) & 1) != 0`). Inside this branch, find three sequential calls:

```c
sub_XXXXXXXX(a1, a2);                          // WaterMove
sub_YYYYYYYY(a1, a2, "FullWalkMovePreMove");   // CheckVelocity
sub_ZZZZZZZZ(a1, a2);                          // FullWalkMove
```

- The function called with `"FullWalkMovePreMove"` as the third argument is `CCSPlayer_MovementServices_CheckVelocity`
- The function called immediately **before** CheckVelocity (no string arg) is `CCSPlayer_MovementServices_WaterMove`
- The function called immediately **after** CheckVelocity (no string arg) is `CCSPlayer_MovementServices_FullWalkMove`

### 4. Verify CheckVelocity

Decompile the CheckVelocity candidate. Confirm it contains DevMsg calls with format strings:
- `"CCSPlayer_MovementServices(%s):  %d/%s Got a NaN velocity on %s\n"`
- `"CCSPlayer_MovementServices(%s):  %d/%s Got a NaN origin on %s\n"`
- `"CCSPlayer_MovementServices(%s):  %d/%s Got a velocity too high (>%.2f) on %s\n"`
- `"CCSPlayer_MovementServices(%s):  %d/%s Got a velocity too low (<%.2f) on %s\n"`

### 5. Verify WaterMove

Decompile the WaterMove candidate. Confirm it:
- Checks water level at `a1+1312` against a threshold from a global config
- Reads velocity magnitude and applies drag/current force
- Scales down wish velocity proportionally
- Stores pre-move velocity into `a2+232/240`

### 6. Verify FullWalkMove

Decompile the FullWalkMove candidate. Confirm it:
- Sets a flag at `a1+705 = 1`
- Decomposes wish angles into forward/right vectors
- Zeroes vertical velocity (`a2+64 = 0`)
- Clamps velocity to max speed
- Performs collision traces and step-move logic
- Handles water movement case separately

### 7. Rename All Three Functions

```
mcp__ida-pro-mcp__rename(batch={"func": [
  {"addr": "<FullWalkMove_addr>", "name": "CCSPlayer_MovementServices_FullWalkMove"},
  {"addr": "<CheckVelocity_addr>", "name": "CCSPlayer_MovementServices_CheckVelocity"},
  {"addr": "<WaterMove_addr>", "name": "CCSPlayer_MovementServices_WaterMove"}
]})
```

### 8. Generate Signatures

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CCSPlayer_MovementServices_FullWalkMove`.

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CCSPlayer_MovementServices_CheckVelocity`.

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CCSPlayer_MovementServices_WaterMove`.

### 9. Write YAML Output

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results for `CCSPlayer_MovementServices_FullWalkMove`.

Required parameters:
- `func_name`: `CCSPlayer_MovementServices_FullWalkMove`
- `func_addr`: The function address from step 3/6
- `func_sig`: The validated signature from step 8

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results for `CCSPlayer_MovementServices_CheckVelocity`.

Required parameters:
- `func_name`: `CCSPlayer_MovementServices_CheckVelocity`
- `func_addr`: The function address from step 3/4
- `func_sig`: The validated signature from step 8

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results for `CCSPlayer_MovementServices_WaterMove`.

Required parameters:
- `func_name`: `CCSPlayer_MovementServices_WaterMove`
- `func_addr`: The function address from step 3/5
- `func_sig`: The validated signature from step 8

## Function Characteristics

### CCSPlayer_MovementServices_FullWalkMove

Main ground walk movement function. Key behaviors:
- Sets in-move flag at `this+705`
- Decomposes wish angles into forward/right vectors
- Computes wish direction from forward/side inputs
- Applies friction and acceleration
- Zeroes vertical velocity for ground movement
- Clamps velocity to max speed with half-step integration
- Performs collision traces via TryPlayerMove/StepMove
- Handles water movement case separately
- Parameters: `(this, CMoveData*)`

### CCSPlayer_MovementServices_CheckVelocity

Velocity and origin sanitization function. Key behaviors:
- Checks each component of velocity (a2+56/60/64) and origin (a2+200/204/208) for NaN values
- Zeroes out any NaN components
- Clamps velocity components to a max speed convar
- Logs diagnostic messages with the phase name string (3rd parameter)
- Parameters: `(this, CMoveData*, const char* phaseName)`

### CCSPlayer_MovementServices_WaterMove

Water current/drag pre-processing function. Key behaviors:
- Checks water level against threshold from movement config
- Reads current velocity magnitude
- Applies water drag force by scaling and subtracting from acceleration vector
- Scales down wish velocity proportionally
- Stores pre-move velocity state
- Parameters: `(this, CMoveData*)`

## Output YAML Files

- `CCSPlayer_MovementServices_FullWalkMove.windows.yaml` / `.linux.yaml`
- `CCSPlayer_MovementServices_CheckVelocity.windows.yaml` / `.linux.yaml`
- `CCSPlayer_MovementServices_WaterMove.windows.yaml` / `.linux.yaml`
