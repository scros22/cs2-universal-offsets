---
name: find-CCSPlayer_WeaponServices_SelectItem
description: |
  Find and identify the CCSPlayer_WeaponServices_SelectItem virtual function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the SelectItem function
  in CCSPlayer_WeaponServices vtable.
  Trigger: CCSPlayer_WeaponServices_SelectItem, SelectItem, weapon select, switch weapon
disable-model-invocation: true
---

# CCSPlayer_WeaponServices_SelectItem Function Location Workflow

## Overview

This workflow is used to locate the `CCSPlayer_WeaponServices_SelectItem` function in CS2 server binary files. This is a virtual function in the `CCSPlayer_WeaponServices` vtable responsible for switching the player's active weapon. It is called by bot AI (e.g., `CCSBot::SwitchToBestWeapon`) and other weapon management code.

## Location Steps

### 1. Search for Signature String

Use `find_regex` to search for the `Grenade.*THROW FAILED` string, which appears in a caller of this function:

```
mcp__ida-pro-mcp__find_regex(pattern="Grenade.*THROW FAILED")
```

Expected result: Find string address (e.g., `0x18155aaf8` for Windows, varies by version)

### 2. Find Cross-References to String

Use `xrefs_to` to find the function referencing this string:

```
mcp__ida-pro-mcp__xrefs_to(addrs="<string_addr>")
```

Expected result: Find a grenade think function (e.g., `sub_1802D0550`) that contains the string reference.

### 3. Decompile and Identify the Caller Function

Decompile the function found in step 2:

```
mcp__ida-pro-mcp__decompile(addr="<function_addr>")
```

Look for the following code pattern in the THROW FAILED branch:
```c
sub_XXXXXXX(a1, 1);  // CCSBot_SwitchToBestWeapon(bot, forceEquip)
```

### 4. Rename and Decompile CCSBot_SwitchToBestWeapon

the function called with `(a1, 1)` in the THROW FAILED branch is `CCSBot_SwitchToBestWeapon`

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<CCSBot_SwitchToBestWeapon_function_addr>", "name": "CCSBot_SwitchToBestWeapon"}})
```

Decompile `CCSBot_SwitchToBestWeapon`. Look for the weapon selection pattern:

```c
// Pattern: weaponServices->vtable[SelectItem](weaponServices, weapon, 0)
(*(void (__fastcall **)(...))(**(_QWORD **)(*(_QWORD *)(a1 + 24) + <WeaponServices_offset>LL) + <vtable_offset>LL))(
    *(_QWORD *)(*(_QWORD *)(a1 + 24) + <WeaponServices_offset>LL),
    weapon,
    0LL);
```

The function at the vtable offset used in this indirect call is `CCSPlayer_WeaponServices_SelectItem`.

### 5. Get VTable Info and Resolve the Target Function

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CCSPlayer_WeaponServices`.

If the skill returns an error, **STOP** and report to user.

Otherwise, use the vtable entries to calculate the index from the vtable offset observed in the disassembly:
- `vfunc_index = vtable_offset / 8` (for 64-bit binaries)

Read the function address from `vtable_entries[index]`.

### 6. Rename Function

Use `rename` to give the function a meaningful name:

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<function_addr>", "name": "CCSPlayer_WeaponServices_SelectItem"}})
```

### 7. Generate and Validate Unique Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 8. Write IDA analysis output as YAML beside the binary

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CCSPlayer_WeaponServices_SelectItem`
- `func_addr`: The function address from step 5
- `func_sig`: The validated signature from step 7

VTable parameters:
- `vtable_name`: `CCSPlayer_WeaponServices`
- `vfunc_offset`: The offset from step 5
- `vfunc_index`: The index from step 5

## Function Characteristics

The `CCSPlayer_WeaponServices_SelectItem` function:

- Is a virtual function in the `CCSPlayer_WeaponServices` vtable
- Signature: `bool SelectItem(CCSPlayer_WeaponServices *this, CBasePlayerWeapon *weapon, int switchReason)`
- Handles weapon switching logic including:
  - Checking if weapon switching is allowed (`sub_180B88A20`)
  - Getting current active weapon
  - If target weapon is already active: special handling for holster/deploy cycles
  - If target weapon is different: holster old weapon, deploy new weapon, handle old weapon cleanup
- Returns `1` on successful switch, `0` on failure

## VTable Information

- **VTable Name**: `CCSPlayer_WeaponServices`
- **VTable Mangled Name**:
  - Windows: `??_7CCSPlayer_WeaponServices@@6B@`
  - Linux: `_ZTV24CCSPlayer_WeaponServices`
- **VTable Offset**: `0xD0` (may change with game updates)
- **VTable Index**: `26` (may change with game updates)

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSPlayer_WeaponServices_SelectItem.windows.yaml`
- `libserver.so` / `libserver.so` → `CCSPlayer_WeaponServices_SelectItem.linux.yaml`

## Related Functions

- `CCSBot_SwitchToBestWeapon` - Bot AI function that calls SelectItem to equip best weapon
- `CCSPlayer_WeaponServices_Weapon_GetSlot` - Gets weapon in a specific slot
- Grenade think function - Contains THROW FAILED string, calls SwitchToBestWeapon on failure
