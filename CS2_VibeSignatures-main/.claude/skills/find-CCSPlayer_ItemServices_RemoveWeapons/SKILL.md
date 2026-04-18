---
name: find-CCSPlayer_ItemServices_RemoveWeapons
description: |
  Find and identify the CCSPlayer_ItemServices_RemoveWeapons function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the RemoveWeapons virtual function
  in the CCSPlayer_ItemServices vtable.
  Trigger: CCSPlayer_ItemServices_RemoveWeapons
disable-model-invocation: true
---

# CCSPlayer_ItemServices_RemoveWeapons Function Location Workflow

## Overview

This workflow locates the `CCSPlayer_ItemServices_RemoveWeapons` function in CS2 server binary files. This is a virtual function in the `CCSPlayer_ItemServices` vtable, called during the death animation state to strip all weapons from a player pawn.

The identification strategy is:
1. Find `CCSPlayerStateDeathAnim` vtable → decompile vtable[0] (`OnEnter`)
2. In `OnEnter`, the first virtual call targets `m_pItemServices->RemoveWeapons`
3. Cross-reference with `CCSPlayer_ItemServices` vtable to confirm the vtable index

## Location Steps

### 1. Get CCSPlayerStateDeathAnim VTable

- **ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CCSPlayerStateDeathAnim`.

  If the skill returns an error, **STOP** and report to user.

  Otherwise, extract `vtable_entries[0]` — this is `CCSPlayerStateDeathAnim::OnEnter`.

### 2. Decompile OnEnter and Identify the RemoveWeapons Call

Decompile `vtable_entries[0]`:

```
mcp__ida-pro-mcp__decompile(addr="<vtable_entries_0>")
```

In the decompiled output, look for the first virtual call pattern near the top of the function:

```c
(*(void (__fastcall **)(_QWORD, __int64))(**(_QWORD **)(*(_QWORD *)(a1 + 32) + <m_pItemServices_offset>) + <vfunc_byte_offset>))(...)
```

Key pattern:
- `a1 + 32` → the player pawn pointer
- `+ <m_pItemServices_offset>` → offset to `m_pItemServices` member (e.g., 2936 on Windows)
- `+ <vfunc_byte_offset>` → byte offset into `CCSPlayer_ItemServices` vtable

Calculate the vtable index: `vfunc_index = vfunc_byte_offset / 8` (64-bit pointers).

### 3. Get CCSPlayer_ItemServices VTable

- **ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CCSPlayer_ItemServices`.

  If the skill returns an error, **STOP** and report to user.

  Otherwise, use the `vfunc_index` from Step 2 to look up the function address in `vtable_entries`.

### 4. Rename Function

Use `rename` to give the function a meaningful name:

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<function_addr>", "name": "CCSPlayer_ItemServices_RemoveWeapons"}})
```

### 5. Generate and Validate Unique Signature

- **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CCSPlayer_ItemServices_RemoveWeapons`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 5

VTable parameters:
- `vtable_name`: `CCSPlayer_ItemServices`
- `vfunc_offset`: `vfunc_index * 8` (from step 2)
- `vfunc_index`: The index from step 2

## Function Characteristics

The `CCSPlayer_ItemServices_RemoveWeapons` function:
- Is called from `CCSPlayerStateDeathAnim::OnEnter` as the first virtual call
- Takes a boolean parameter (set to 1/true when called from death state)
- Strips all weapons from the player pawn's inventory

## VTable Information

- **VTable Name**: `CCSPlayer_ItemServices`
- **VTable Mangled Name**:
  - Windows: `??_7CCSPlayer_ItemServices@@6B@`
  - Linux: `_ZTV22CCSPlayer_ItemServices`
- **VTable Offset**: `0xB8` (may change with game updates)
- **VTable Index**: `23` (may change with game updates)

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSPlayer_ItemServices_RemoveWeapons.windows.yaml`
- `libserver.so` / `libserver.so` → `CCSPlayer_ItemServices_RemoveWeapons.linux.yaml`

## Related Functions

- `CCSPlayerStateDeathAnim::OnEnter` (vtable[0]) - Death state entry, calls RemoveWeapons
- `CCSPlayer_ItemServices::DropActivePlayerWeapon` - Drops the active weapon
