---
name: find-g_pGameRules-AND-CGameRules_FindPickerEntity
description: Find and identify the g_pGameRules and CCSGameRules_FindPickerEntity function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the FindPickerEntity function by searching for the "[%03d] Found: %s, firing" string and tracing its caller to identify g_pGameRules and the vfunc call pattern.
disable-model-invocation: true
---

# Find g_pGameRules and CGameRules_FindPickerEntity

Locate `g_pGameRules` (global variable) and `CGameRules_FindPickerEntity` (virtual function) in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

### 1. Search for String Reference

Search for the string `[%03d] Found: %s, firing` in the binary:

```
mcp__ida-pro-mcp__find_regex pattern="\[%03d\] Found: %s, firing"
```

This returns two matching strings:
- `[%03d] Found: %s, firing\n` — used in a single-entity firing path
- `[%03d] Found: %s, firing (%s)\n` — used in a loop-based name-lookup path

### 2. Find Cross-References to the First String

Get xrefs to the `[%03d] Found: %s, firing\n` string (the one WITHOUT `(%s)`):

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

This leads to a caller function (the entity I/O output firing function).

### 3. Decompile the Caller Function

```
mcp__ida-pro-mcp__decompile addr="<caller_func_addr>"
```

### 4. Identify g_pGameRules and the FindPickerEntity VFunc Call

In the decompiled output, look for this code pattern:

```c
v5 = (*(__int64 (__fastcall **)(__int64, __int64, _QWORD))(*(_QWORD *)qword_XXXXXXXXX + 200LL))(
       qword_XXXXXXXXX,
       v3,
       0LL);
```

Key identification points:
- `qword_XXXXXXXXX` is `g_pGameRules` — a global pointer to `CGameRules`
- `+200LL` (decimal 200 = `0xC8`) is the vtable offset for `FindPickerEntity`
- The call takes `(this, entity_handle, 0)` as parameters

### 5. Rename g_pGameRules

Rename the global variable if not already named:

```
mcp__ida-pro-mcp__rename batch={"data": {"old": "qword_XXXXXXXXX", "new": "g_pGameRules"}}
```

### 6. Get CGameRules VTable Address

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CGameRules`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `vtable_entries` and use the vtable offset from step 4 to calculate the index:
- vtable index = decimal_offset / 8 (e.g., 200 / 8 = index 25)
- Look up the function address from `vtable_entries[<index>]`

### 7. Decompile and Verify FindPickerEntity

Decompile the function at the vtable entry:

```
mcp__ida-pro-mcp__decompile addr="<vtable_entry_addr>"
```

Verify it contains:
- RTTI type checking for `CBaseEntity` and `CBasePlayerController` (via `__RTDynamicCast`)
- Calls to get player pawn from controller
- Entity lookup logic (position/distance based entity picking)

### 8. Rename the Function

```
mcp__ida-pro-mcp__rename batch={"func": {"addr": "<function_addr>", "name": "CGameRules_FindPickerEntity"}}
```

### 9. Generate and Validate Unique Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for CGameRules_FindPickerEntity.

### 10. Write IDA Analysis Output for CGameRules_FindPickerEntity as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CGameRules_FindPickerEntity`
- `func_addr`: The function address from step 7
- `func_sig`: The validated signature from step 9

VTable parameters:
- `vtable_name`: `CGameRules`
- `vfunc_offset`: `0xC8` (decimal 200)
- `vfunc_index`: Calculated in step 6

### 11. Generate and Validate Unique Signature for g_pGameRules

**ALWAYS** Use SKILL `/generate-signature-for-globalvar` to generate a robust and unique signature for `g_pGameRules`.

### 12. Write IDA Analysis Output for g_pGameRules as YAML

**ALWAYS** Use SKILL `/write-globalvar-as-yaml` to write the analysis results for `g_pGameRules`.

Required parameters:
- `gv_name`: `g_pGameRules`
- `gv_addr`: The global variable address from step 5
- `gv_sig`: The validated signature from step 11
- `gv_sig_va`: The virtual address that signature matches
- `gv_inst_offset`: Offset from signature start to GV-accessing instruction
- `gv_inst_length`: Length of the GV-accessing instruction
- `gv_inst_disp`: Displacement offset within the instruction

## Function Characteristics

- **Parameters**: `(this, player_entity, target)` where `this` is CGameRules pointer
- **Purpose**: Finds the entity a player is looking at (picker/crosshair trace)
- **Pattern**: Uses RTTI dynamic cast to check for CBasePlayerController, gets the controlled pawn, then performs entity lookup
- **Contains**: Type checking for CBasePlayerController, pawn retrieval, entity system queries

## Global Variable Characteristics

### g_pGameRules

- **Type**: Global pointer (`CGameRules*`)
- **Purpose**: Singleton pointer to the game rules object, used throughout the server for game rule queries
- **Access Pattern**: Typically accessed via `mov rcx, cs:g_pGameRules` before calling virtual methods on it
- **Related Class**: `CGameRules` (base class with vtable at `??_7CGameRules@@6B@`)

## Output YAML Format

The output YAML filenames depend on the platform:

For CGameRules_FindPickerEntity:
- `server.dll` → `CGameRules_FindPickerEntity.windows.yaml`
- `libserver.so` → `CGameRules_FindPickerEntity.linux.yaml`

For g_pGameRules:
- `server.dll` → `g_pGameRules.windows.yaml`
- `libserver.so` → `g_pGameRules.linux.yaml`
