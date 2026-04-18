---
name: find-UTIL_PlayerSlotToPlayerController
description: Find and identify the UTIL_PlayerSlotToPlayerController function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the function by searching for the "game_playerleave" string reference and identifying its callee.
disable-model-invocation: true
---

# Find UTIL_PlayerSlotToPlayerController

Locate `UTIL_PlayerSlotToPlayerController` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="game_playerleave"
   ```

   Look for the exact string `"game_playerleave"`.

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

   The referencing function should match this pattern:
   ```cpp
   __int64 __fastcall sub_XXXXXXXX(__int64 a1, unsigned int a2)
   {
       result = sub_YYYYYYYY(a2);          // <-- This is UTIL_PlayerSlotToPlayerController
       if ( result )
       {
           result = sub_ZZZZZZZZ(result);
           v3 = result;
           if ( result )
           {
               result = sub_WWWWWWWW("game_playerleave", result, result, 3LL, 0);
               v5 = *(_QWORD *)(v3 + ...);
               if ( v5 )
               {
                   // virtual function call
               }
           }
       }
       return result;
   }
   ```

   **Key identification**: The first call in this function takes `a2` (unsigned int, a player slot index) and returns a pointer. That callee (`sub_YYYYYYYY`) is `UTIL_PlayerSlotToPlayerController`.

4. Get the address of the callee (the first call target that takes `a2`), then decompile it to verify:
   ```
   mcp__ida-pro-mcp__decompile addr="<callee_addr>"
   ```

5. Rename the callee function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<callee_addr>", "name": "UTIL_PlayerSlotToPlayerController"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `UTIL_PlayerSlotToPlayerController`
   - `func_addr`: The callee function address
   - `func_sig`: The validated signature with wildcards

## Function Characteristics

- **Parameters**:
  - `a1`: Player slot index (unsigned int)
- **Return**: CCSPlayerController pointer (or 0 if invalid)

## Key Behaviors

1. Takes a player slot index as input
2. Returns the corresponding CCSPlayerController pointer
3. Called as the first function in the `"game_playerleave"` event handler

## DLL Information

- **DLL**: `server.dll` (Windows) / `libserver.so` (Linux)

## Notes

- This is NOT a virtual function
- The target is NOT the function containing `"game_playerleave"`, but its **callee** that converts a player slot to a controller
- The signature may require wildcards for RIP-relative addresses

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `UTIL_PlayerSlotToPlayerController.windows.yaml`
- `libserver.so` → `UTIL_PlayerSlotToPlayerController.linux.yaml`
