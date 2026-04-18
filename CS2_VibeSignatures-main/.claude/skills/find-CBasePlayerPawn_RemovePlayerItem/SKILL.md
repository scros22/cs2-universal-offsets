---
name: find-CBasePlayerPawn_RemovePlayerItem
description: Find and identify the CBasePlayerPawn_RemovePlayerItem function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the RemovePlayerItem function by searching for the "DestroyWeapon" string reference and analyzing cross-references.
disable-model-invocation: true
---

# Find CBasePlayerPawn_RemovePlayerItem

Locate `CBasePlayerPawn_RemovePlayerItem` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the debug string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="DestroyWeapon"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify the RemovePlayerItem function from the decompiled code. Look for code pattern like:
   ```c
   if ( *(int *)(a1 + 16) >= 1 )
   {
     v15 = *(_QWORD *)(a1 + 8);
     v18 = 0;
     if ( (unsigned __int8)sub_XXXXX(v19, v15, &v18)
       && (v16 = sub_XXXXX(v18, 0, &CBaseEntity_RTTI, &CCSWeaponBase_RTTI, 0)) != 0 )
     {
       sub_XXXXXXX(*(_QWORD *)(v14 + 2928), v16);  // <-- This is CBasePlayerPawn_RemovePlayerItem
     }
   }
   else if ( LoggingSystem_IsChannelEnabled(...) )
   {
     LoggingSystem_Log(..., "Method %s.%s invoked with bad %s value. (parameter #%d)\n",
                       "CSPlayerPawn", "DestroyWeapon", "target", 0);
   }
   ```

   The function is called with:
   - First argument: `*(_QWORD *)(pawn + 2928)` - an entity system pointer
   - Second argument: The weapon entity after RTTI casting to CCSWeaponBase

5. Get function info:
   ```
   mcp__ida-pro-mcp__lookup_funcs queries="<removeplayeritem_func_addr>"
   ```

6. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": {"addr": "<function_addr>", "name": "CBasePlayerPawn_RemovePlayerItem"}}
   ```

7. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

8. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CBasePlayerPawn_RemovePlayerItem`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 7

## Signature Pattern

The function is called when destroying a weapon from a player's inventory. The calling context includes:
- RTTI type checking for CBaseEntity and CCSWeaponBase
- Error logging with "CSPlayerPawn" and "DestroyWeapon" strings

## Function Characteristics

- **Parameters**: `(this, weapon)` where `this` is likely a weapon services or inventory pointer (from pawn + 2928), `weapon` is a CCSWeaponBase pointer
- **Purpose**: Removes a weapon/item from the player pawn's inventory
- **Called by**: Script/VScript DestroyWeapon method handler for CSPlayerPawn

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBasePlayerPawn_RemovePlayerItem.windows.yaml`
- `libserver.so` → `CBasePlayerPawn_RemovePlayerItem.linux.yaml`
