---
name: find-CBasePlayerPawn_DropActivePlayerWeapon-AND-CCSPlayer_ItemServices_DropActivePlayerWeapon-AND-CCSPlayer_WeaponServices_DropWeapon
description: |
  Find and identify CCSPlayer_ItemServices_DropActivePlayerWeapon and CCSPlayer_ItemServices_RemoveWeapons functions in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the weapon drop call chain: CBasePlayerPawn_DropActivePlayerWeapon (vtable thunk) -> CCSPlayer_ItemServices_DropActivePlayerWeapon -> CCSPlayer_WeaponServices_DropWeapon. Located via "KilledNPC" string xref in the death handler function.
  Trigger: CBasePlayerPawn_DropActivePlayerWeapon, CCSPlayer_ItemServices_DropActivePlayerWeapon, CCSPlayer_WeaponServices_DropWeapon, DropActivePlayerWeapon, DropWeapon, KilledNPC
disable-model-invocation: true
---

# Find CBasePlayerPawn_DropActivePlayerWeapon, CCSPlayer_ItemServices_DropActivePlayerWeapon, and CCSPlayer_WeaponServices_DropWeapon

Locate the weapon drop call chain in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Overview

This skill traces the weapon drop call chain through three functions:

1. **CBasePlayerPawn_DropActivePlayerWeapon** — A small vtable thunk on `CBasePlayerPawn` that delegates to `CCSPlayer_ItemServices`
2. **CCSPlayer_ItemServices_DropActivePlayerWeapon** — The mid-level handler that gets the active weapon and calculates velocity
3. **CCSPlayer_WeaponServices_DropWeapon** — The actual weapon drop implementation on `CCSPlayer_WeaponServices`

The entry point is found via the `"KilledNPC"` string in the death handler function.

## Method

### 1. Search for "KilledNPC" String

Use `find_regex` to search for the `KilledNPC` string:

```
mcp__ida-pro-mcp__find_regex(pattern="KilledNPC")
```

Expected result: Find string address (e.g., `0x1816f97c8` for Windows, varies by version)

### 2. Find Cross-References to the String

Use `xrefs_to` to find the function referencing this string:

```
mcp__ida-pro-mcp__xrefs_to(addrs="<string_addr>")
```

Expected result: Find a data xref inside a death handler function (e.g., `sub_180BF18F0`)

### 3. Decompile the Death Handler Function

Decompile the function found in step 2 and locate the following code pattern:

Windows:
```c
  v17 = -1;
  v18 = *(_DWORD *)(*(_QWORD *)a2 + 76LL);
  if ( (v18 & 0x2000) != 0 )
    v17 = (v18 >> 8) & 1;
  (*(void (__fastcall **)(__int64, __int64 *, _QWORD, _QWORD))(*(_QWORD *)a1 + <OFFSET>LL))(a1, &v25, v17, 0LL);
```

Linux:
```c
  v17 = -1;
  v18 = *(_DWORD *)(*(_QWORD *)a2 + 76LL);
  if ( (v18 & 0x2000) != 0 )
    v17 = (v18 >> 8) & 1;
  (*(void (__fastcall **)(__int64, __int64 *, _QWORD, _QWORD))(*(_QWORD *)a1 + <OFFSET>LL))(a1, &v25, v17, 0LL);
```

The virtual call `*(_QWORD *)a1 + <OFFSET>` is `CBasePlayerPawn::DropActivePlayerWeapon`. Record the `<OFFSET>` value (e.g., 2656 on Windows).

### 4. Load CBasePlayerPawn VTable

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CBasePlayerPawn`.

If the skill returns an error, **STOP** and report to user.

Calculate vtable index: `vfunc_index = <OFFSET> / 8` (e.g., 2656 / 8 = 332)

Verify the vtable entry at that index points to a small thunk function (typically ~0x17 bytes).

### 5. Decompile and Verify CBasePlayerPawn_DropActivePlayerWeapon

Decompile the function at `vtable_entries[vfunc_index]`. It should be a small thunk like:

Windows:
```c
__int64 __fastcall sub_XXXXXXXX(__int64 a1)
{
  __int64 v1; // rcx
  __int64 result; // rax

  v1 = *(_QWORD *)(a1 + 0xB78);  // m_pItemServices offset (may change)
  if ( v1 )
    return (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v1 + 176LL))(v1);  // ItemServices vtable call
  return result;
}
```

Linux:
```c
__int64 __fastcall sub_XXXXXXXX(__int64 a1)
{
  __int64 v1;
  __int64 result;

  v1 = *(_QWORD *)(a1 + <m_pItemServices_offset>);
  if ( v1 )
    return (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v1 + <vtable_offset>LL))(v1);
  return result;
}
```

Record the ItemServices vtable offset from the inner call (e.g., 176 on Windows, may differ on Linux).

### 6. Rename CBasePlayerPawn_DropActivePlayerWeapon

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<thunk_addr>", "name": "CBasePlayerPawn_DropActivePlayerWeapon"}})
```

### 7. Load CCSPlayer_ItemServices VTable

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CCSPlayer_ItemServices`.

If the skill returns an error, **STOP** and report to user.

Calculate the ItemServices vtable index from the offset recorded in step 5: `item_vfunc_index = <inner_offset> / 8` (e.g., 176 / 8 = 22)

The function at `CCSPlayer_ItemServices_vtable_entries[item_vfunc_index]` is `CCSPlayer_ItemServices_DropActivePlayerWeapon`.

### 8. Rename CCSPlayer_ItemServices_DropActivePlayerWeapon

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<item_services_func_addr>", "name": "CCSPlayer_ItemServices_DropActivePlayerWeapon"}})
```

### 9. Decompile CCSPlayer_ItemServices_DropActivePlayerWeapon and Find DropWeapon

Decompile the function and look for the final virtual call to `m_pWeaponServices->DropWeapon`:

Windows:
```c
    return (*(__int64 (__fastcall **)(__int64, __int64, _QWORD, unsigned __int64 *))(*(_QWORD *)v4 + 192LL))(
             v4, v6, 0LL, v9);
```

Linux:
```c
    return (*(__int64 (__fastcall **)(__int64, __int64, _QWORD, __int64 *))(*(_QWORD *)v6 + 200LL))(v6, v8, 0LL, &v13);
```

Record the WeaponServices vtable offset (e.g., 192 on Windows, 200 on Linux).

### 10. Load CCSPlayer_WeaponServices VTable

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CCSPlayer_WeaponServices`.

If the skill returns an error, **STOP** and report to user.

Calculate the WeaponServices vtable index: `weapon_vfunc_index = <weapon_offset> / 8` (e.g., 192 / 8 = 24 on Windows, 200 / 8 = 25 on Linux)

The function at `CCSPlayer_WeaponServices_vtable_entries[weapon_vfunc_index]` is `CCSPlayer_WeaponServices_DropWeapon`.

### 11. Rename CCSPlayer_WeaponServices_DropWeapon

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<weapon_services_func_addr>", "name": "CCSPlayer_WeaponServices_DropWeapon"}})
```

### 12. Generate Signatures

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate robust signatures for all three functions.

For CBasePlayerPawn_DropActivePlayerWeapon:
```
/generate-signature-for-function addr=<CBasePlayerPawn_DropActivePlayerWeapon_addr>
```

For CCSPlayer_ItemServices_DropActivePlayerWeapon:
```
/generate-signature-for-function addr=<CCSPlayer_ItemServices_DropActivePlayerWeapon_addr>
```

For CCSPlayer_WeaponServices_DropWeapon:
```
/generate-signature-for-function addr=<CCSPlayer_WeaponServices_DropWeapon_addr>
```

### 13. Write Analysis Results as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write analysis results for all three functions:

#### For CBasePlayerPawn_DropActivePlayerWeapon:

Required parameters:
- `func_name`: `CBasePlayerPawn_DropActivePlayerWeapon`
- `func_addr`: The thunk function address from step 4
- `func_sig`: The validated signature from step 12

VTable parameters:
- `vtable_name`: `CBasePlayerPawn`
- `vfunc_offset`: The offset from step 3 (e.g., `0xA60`)
- `vfunc_index`: The index from step 4 (e.g., `332`)

#### For CCSPlayer_ItemServices_DropActivePlayerWeapon:

Required parameters:
- `func_name`: `CCSPlayer_ItemServices_DropActivePlayerWeapon`
- `func_addr`: The function address from step 7
- `func_sig`: The validated signature from step 12

VTable parameters:
- `vtable_name`: `CCSPlayer_ItemServices`
- `vfunc_offset`: The offset from step 5 (e.g., `0xB0`)
- `vfunc_index`: The index from step 7 (e.g., `22`)

#### For CCSPlayer_WeaponServices_DropWeapon:

Required parameters:
- `func_name`: `CCSPlayer_WeaponServices_DropWeapon`
- `func_addr`: The function address from step 10
- `func_sig`: The validated signature from step 12

VTable parameters:
- `vtable_name`: `CCSPlayer_WeaponServices`
- `vfunc_offset`: The offset from step 9 (e.g., `0xC0`)
- `vfunc_index`: The index from step 10 (e.g., `24`)

## Function Characteristics Summary

### CBasePlayerPawn_DropActivePlayerWeapon

- **VTable Class**: `CBasePlayerPawn`
- **VTable Index**: 332 (may change with updates)
- **Size**: Very small (~0x17 bytes), a thunk/wrapper
- **Purpose**: Reads `m_pItemServices` from pawn and delegates to its `DropActivePlayerWeapon` virtual function
- **Key Pattern**: `mov rcx, [rcx+<m_pItemServices_offset>]; test rcx, rcx; jz short; mov rax, [rcx]; jmp qword ptr [rax+<offset>]`

### CCSPlayer_ItemServices_DropActivePlayerWeapon

- **VTable Class**: `CCSPlayer_ItemServices`
- **VTable Index**: 22 (may change with updates)
- **Parameters**: `(this, velocity_vector)`
- **Purpose**: Gets active weapon from WeaponServices, calculates throw velocity using eye angles, then calls WeaponServices::DropWeapon
- **Key Features**:
  - Reads `m_pWeaponServices` from pawn (offset ~0xB70)
  - Gets active weapon via helper function
  - Calculates velocity using SIMD math with a scalar from vtable[35] (offset 280) of an eye-angle object
  - Final call to `m_pWeaponServices->DropWeapon`

### CCSPlayer_WeaponServices_DropWeapon

- **VTable Class**: `CCSPlayer_WeaponServices`
- **VTable Index**: 24 (may change with updates)
- **Parameters**: `(this, weapon_entity, unk, velocity_vector)`
- **Purpose**: The actual weapon drop implementation that handles physics and entity state

## VTable Information

### CBasePlayerPawn
- **Mangled Name**:
  - Windows: `??_7CBasePlayerPawn@@6B@`
  - Linux: `_ZTV15CBasePlayerPawn`
- **DropActivePlayerWeapon Offset**: `0xA60` (may change with game updates)
- **DropActivePlayerWeapon Index**: `332` (may change with game updates)

### CCSPlayer_ItemServices
- **Mangled Name**:
  - Windows: `??_7CCSPlayer_ItemServices@@6B@`
  - Linux: `_ZTV24CCSPlayer_ItemServices`
- **DropActivePlayerWeapon Offset**: `0xB0` (may change with game updates)
- **DropActivePlayerWeapon Index**: `22` (may change with game updates)

### CCSPlayer_WeaponServices
- **Mangled Name**:
  - Windows: `??_7CCSPlayer_WeaponServices@@6B@`
  - Linux: `_ZTV25CCSPlayer_WeaponServices`
- **DropWeapon Offset**: `0xC0` (may change with game updates)
- **DropWeapon Index**: `24` (may change with game updates)

## Output YAML Files

This skill generates **three separate YAML files**:

**Platform-specific naming:**
- `server.dll` (Windows):
  - `CBasePlayerPawn_DropActivePlayerWeapon.windows.yaml`
  - `CCSPlayer_ItemServices_DropActivePlayerWeapon.windows.yaml`
  - `CCSPlayer_WeaponServices_DropWeapon.windows.yaml`
- `libserver.so` / `libserver.so` (Linux):
  - `CBasePlayerPawn_DropActivePlayerWeapon.linux.yaml`
  - `CCSPlayer_ItemServices_DropActivePlayerWeapon.linux.yaml`
  - `CCSPlayer_WeaponServices_DropWeapon.linux.yaml`

## Call Chain Diagram

```
Death Handler (contains "KilledNPC" string)
  |
  +-- CBasePlayerPawn::DropActivePlayerWeapon (vtable thunk)
        |
        +-- reads this->m_pItemServices
        |
        +-- CCSPlayer_ItemServices::DropActivePlayerWeapon
              |
              +-- gets active weapon from m_pWeaponServices
              +-- calculates throw velocity (SIMD)
              |
              +-- CCSPlayer_WeaponServices::DropWeapon
                    |
                    +-- actual weapon drop logic
```

## Troubleshooting

**If "KilledNPC" string not found:**
- Verify the binary is a CS2 server binary (server.dll or libserver.so)
- The string may have been changed in a game update

**If the death handler has different structure:**
- Look for the pattern: check `0x2000` flag, compute `(flags >> 8) & 1`, then virtual call with 4 args
- The virtual call offset may differ between versions

**If vtable YAML files are missing:**
- Run `/write-vtable-as-yaml` for each required class first:
  - `CBasePlayerPawn`
  - `CCSPlayer_ItemServices`
  - `CCSPlayer_WeaponServices`

**If CBasePlayerPawn thunk signature is not unique:**
- The thunk is very small (~23 bytes) and may not be unique with auto-wildcarding
- Keep struct offsets (e.g., `0xB78`) and vtable offsets (e.g., `0xB0`) as fixed bytes
- Only wildcard the short jump offset
