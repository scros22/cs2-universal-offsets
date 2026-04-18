---
name: find-CCSPlayer_WeaponServices_Weapon_GetSlot
description: Find and identify the CCSPlayer_WeaponServices_Weapon_GetSlot function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the Weapon_GetSlot function by searching for a known string in a caller function and tracing the call target.
disable-model-invocation: true
---

# Find CCSPlayer_WeaponServices_Weapon_GetSlot

Locate `CCSPlayer_WeaponServices_Weapon_GetSlot` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

This is a **non-virtual function** found indirectly through a caller that references a known debug string.

## Method

1. Search for the debug string in a known caller function:
   ```
   mcp__ida-pro-mcp__find_regex pattern="MoveToInitialEncounter.*Can't find battlefront"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function (the caller, NOT the target):
   ```
   mcp__ida-pro-mcp__decompile addr="<caller_function_addr>"
   ```

4. Identify the target function in the decompiled code:

   Look for a call pattern like:
   ```c
   sub_XXXXXXXX(*(_QWORD *)(*(_QWORD *)(a1 + 24) + <offset>), 0, -1);
   ```
   This call appears just **after** the `*(_QWORD *)(a1 + 25872) = *v19;` assignment in the else branch (when battlefront is found). The called function is `CCSPlayer_WeaponServices_Weapon_GetSlot`.

   The result is then checked: `v20 && *(_DWORD *)(*(_QWORD *)(v20 + 800) + 1088LL) == 5` to determine if the weapon is in slot 5.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<target_func_addr>", "name": "CCSPlayer_WeaponServices_Weapon_GetSlot"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CCSPlayer_WeaponServices_Weapon_GetSlot`
   - `func_addr`: The target function address from step 4
   - `func_sig`: The validated signature from step 6

## Signature Pattern

The function itself does NOT contain distinctive strings. It is identified by being called from a bot AI navigation function that references:
```
MoveToInitialEncounter: Can't find battlefront!
```

## Function Characteristics

- **Type**: Non-virtual function (not in a vtable)
- **Parameters**: `(this, slot_index, unknown)` where `this` is a CCSPlayer_WeaponServices pointer
- **Return**: Pointer to a weapon entity, or 0 if not found
- **Behavior**: Iterates over weapon slots and returns the weapon at the specified slot index

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` -> `CCSPlayer_WeaponServices_Weapon_GetSlot.windows.yaml`
- `libserver.so` -> `CCSPlayer_WeaponServices_Weapon_GetSlot.linux.yaml`
