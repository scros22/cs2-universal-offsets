---
name: find-CCSPlayer_ItemServices_CanAcquire
description: Find and identify the CCSPlayer_ItemServices_CanAcquire function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the CanAcquire function that checks if a player's item services can acquire a specific item. This function validates item acquisition based on bot AI purchase logic and item availability.
disable-model-invocation: true
---

# Find CCSPlayer_ItemServices_CanAcquire

Locate `CCSPlayer_ItemServices_CanAcquire` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the unique error string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="\[AI BT\]: Unable to determine the cost of '%s'\. Moving on to the next bot\."
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify the target method call:

   Look for the following pattern in the decompiled code:
   ```c
   v39 = (*CALL)(*(object + 3712), item_handle, 1, 0);
   ```

   This call satisfies:
   - First argument is a services subobject at offset 3712 (offset can diffs between windows / linux and change after game updates)
   - Second argument is an item definition/handle
   - Third argument is a flag (value: 1) indicating execution mode
   - Fourth argument is a flag (value: 0)
   - Return value controls success/failure flow (checked with `if ( v39 )`)

   The function being called at this pattern is `CCSPlayer_ItemServices_CanAcquire`.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CCSPlayer_ItemServices_CanAcquire"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CCSPlayer_ItemServices_CanAcquire`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 6

   Note: This is NOT a virtual function, so no vtable parameters are needed.

## Signature Pattern

The function is identified through bot AI purchase logic that contains error logging:
```
[AI BT]: Unable to determine the cost of '%s'. Moving on to the next bot.
```

The calling context shows item validation and purchase flow where the function at offset 3712 from the player services object is invoked to check item acquisition eligibility.

## Function Characteristics

- **Parameters**: `(item_services, item_def, mode_flag, additional_flag)` where:
  - `item_services` is the CCSPlayer_ItemServices pointer
  - `item_def` is the item definition/handle
  - `mode_flag` controls execution mode (typically 1)
  - `additional_flag` is an additional parameter (typically 0)
- **Return value**: Boolean indicating whether the item can be acquired (non-zero = can acquire)
- **Offset**: Called via services subobject at offset **3712** from player object

## Code Context

The function is called within bot AI purchase logic:
```c
v37 = (_BYTE *)sub_19077C0(v64, *(unsigned __int16 *)(v68 + 16), 0, 0); // Get item
v38 = *(_QWORD *)(v28 + 24);  // Get player object pointer

if ( !v37 || !v37[104] || !v38 )  // Validate item and player
{
  // Error: player unable to purchase item
}

v39 = sub_1339CD0(*(_QWORD *)(v38 + 3712), v37, 1, 0); // CAN ACQUIRE CHECK

if ( v39 )  // Success: item can be acquired
{
  // Continue with purchase
}
else
{
  // Error: unable to determine cost, skip to next bot
  sub_97B5D0("[AI BT]: Unable to determine the cost of '%s'. Moving on to the next bot.");
}
```

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSPlayer_ItemServices_CanAcquire.windows.yaml`
- `libserver.so` → `CCSPlayer_ItemServices_CanAcquire.linux.yaml`

Note: No vtable information is included as this is not a virtual function.
