---
name: find-CBasePlayerPawn_GetEyeAngles
description: |
  Find and identify the CBasePlayerPawn_GetEyeAngles virtual function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the GetEyeAngles function on CBasePlayerPawn.
  Trigger: CBasePlayerPawn_GetEyeAngles, GetEyeAngles, eye angles
disable-model-invocation: true
---

# CBasePlayerPawn_GetEyeAngles Function Location Workflow

## Overview

Locate the `CBasePlayerPawn_GetEyeAngles` virtual function in CS2 server binary. This function returns the player pawn's eye angles (pitch/yaw/roll). It checks for a vehicle/observer entity first, and falls back to scene node interpolation or direct eye position retrieval.

## Location Steps

### 1. Get CBaseEntity and CBasePlayerPawn VTable

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CBaseEntity` and with `class_name=CBasePlayerPawn`.

If the skill returns an error, **STOP** and report to user.
Otherwise, extract `vtable_va`, `vtable_numvfunc` and `vtable_entries` for subsequent steps.

### 2. Search for Signature String

Use `find_regex` to search for the `BotPlaceCommand` error string:

```
mcp__ida-pro-mcp__find_regex(pattern="BotPlaceCommand.*could not find a bot to move")
```

Expected result: Find string address containing `"Error: BotPlaceCommand() could not find a bot to move to player's location.\n"`

### 3. Find Cross-References and Decompile

Use `xrefs_to` to find the function referencing the string:

```
mcp__ida-pro-mcp__xrefs_to(addrs="<string_addr>")
```

Decompile the referencing function with `decompile`. Look for this code pattern:

```c
if ( !v14
    || !(*(unsigned __int8 (__fastcall **)(__int64))(*(_QWORD *)v14 + 1344LL))(v14)
    || !(*(unsigned __int8 (__fastcall **)(__int64))(*(_QWORD *)v15 + 3344LL))(v15) )
{
    Msg("Error: BotPlaceCommand() could not find a human player to move a bot to.\n");
    return 0;
}
if ( !v7 )
{
    Msg("Error: BotPlaceCommand() could not find a bot to move to player's location.\n");
    return 0;
}
sub_XXXXXXXX(v15, (__int64)v44, 0LL, 0LL);  // <-- This is the wrapper calling GetEyeAngles
```

The function called right after both error checks (with 4 args: player, output_buf, 0, 0) is the `AngleVectors` wrapper.

### 4. Decompile the Wrapper Function

Decompile the wrapper function identified in step 3. It should look like:

```c
__int64 __fastcall sub_XXXXXXXX(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
  _BYTE v8[24];
  (*(void (__fastcall **)(__int64, _BYTE *))(*(_QWORD *)a1 + <OFFSET>LL))(a1, v8); // GetEyeAngles vtable call
  return sub_YYYYYYYY(v8, a2, a3, a4); // AngleVectors
}
```

Extract the vtable offset `<OFFSET>` from the virtual call `*(_QWORD *)a1 + <OFFSET>`. Calculate the vtable index: `index = <OFFSET> / 8`.

### 5. Resolve and Rename the Function

Using the vtable entries from step 1, look up `CBasePlayerPawn vtable_entries[index]` to resolve the actual function address of `CBasePlayerPawn_GetEyeAngles`.

Using the vtable entries from step 1, look up `CBaseEntity vtable_entries[index]` to resolve the actual function address of `CBaseEntity_GetEyeAngles`.

Rename them:

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<CBaseEntity_GetEyeAngles_function_addr>", "name": "CBaseEntity_GetEyeAngles"}})
```

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<CBasePlayerPawn_GetEyeAngles_function_addr>", "name": "CBasePlayerPawn_GetEyeAngles"}})
```

### 6. Generate and Validate Unique Signature for CBaseEntity_GetEyeAngles

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `CBaseEntity_GetEyeAngles`.

### 7. Generate and Validate Unique Signature for CBasePlayerPawn_GetEyeAngles

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CBasePlayerPawn_GetEyeAngles`.

### 8. Write IDA analysis output as YAML beside the binary

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results for `CBasePlayerPawn_GetEyeAngles` and `CBaseEntity_GetEyeAngles`.

For `CBaseEntity_GetEyeAngles`: 

  Required parameters:
  - `func_name`: `CBaseEntity_GetEyeAngles`
  - `func_addr`: The function address from step 5
  - `vfunc_sig`: CBaseEntity_GetEyeAngles's `<vfunc_sig>` from step 6

  VTable parameters:
  - `vtable_name`: `CBaseEntity`
  - `vfunc_offset`: `index * 8` (from step 4)
  - `vfunc_index`: The index from step 4

For `CBasePlayerPawn_GetEyeAngles`: 

  Required parameters:
  - `func_name`: `CBasePlayerPawn_GetEyeAngles`
  - `func_addr`: The function address from step 5
  - `func_sig`: The validated signature from step 8

  VTable parameters:
  - `vtable_name`: `CBasePlayerPawn`
  - `vfunc_offset`: `index * 8` (from step 4)
  - `vfunc_index`: The index from step 4

## Function Characteristics

The `CBasePlayerPawn_GetEyeAngles` function:
- Is a virtual function on `CBasePlayerPawn`
- Takes `(this, QAngle *out)` — writes eye angles to the output parameter
- Checks `this[373]` for a vehicle/observer entity; if present, recursively calls `GetEyeAngles` on that entity's view target
- Falls back to scene node interpolation or direct retrieval via vtable `+1504`

## VTable Information

- **VTable Name**: `CBasePlayerPawn`
- **VTable Mangled Name**:
  - Windows: `??_7CBasePlayerPawn@@6B@`
  - Linux: `_ZTV15CBasePlayerPawn`
- **VTable Offset**: `0x5D8` (may change with game updates)
- **VTable Index**: `187` (may change with game updates)

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBasePlayerPawn_GetEyeAngles.windows.yaml`, `CBaseEntity_GetEyeAngles.windows.yaml`
- `libserver.so` / `libserver.so` → `CBasePlayerPawn_GetEyeAngles.linux.yaml`, `CBaseEntity_GetEyeAngles.linux.yaml`
