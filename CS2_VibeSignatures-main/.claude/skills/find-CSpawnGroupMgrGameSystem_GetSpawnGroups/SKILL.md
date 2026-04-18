---
name: find-CSpawnGroupMgrGameSystem_GetSpawnGroups
description: Find and identify the CSpawnGroupMgrGameSystem_GetSpawnGroups function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the GetSpawnGroups function by searching for known format string references and analyzing cross-references.
disable-model-invocation: true
---

# Find CSpawnGroupMgrGameSystem_GetSpawnGroups

Locate `CSpawnGroupMgrGameSystem_GetSpawnGroups` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the format string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="%d spawn groups"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify `CSpawnGroupMgrGameSystem_GetSpawnGroups` by looking for this code pattern in the decompiled function:
   ```c
   sub_XXXXX(a1, &v_count);
   IsChannelEnabled = LoggingSystem_IsChannelEnabled(..., 2LL);
   v4 = v_count;
   if ( IsChannelEnabled )
     LoggingSystem_Log(..., 2LL, "%d spawn groups:\n", v_count);
   ```
   The `sub_XXXXX(a1, &v_count)` call immediately before the `"%d spawn groups:\n"` log is `CSpawnGroupMgrGameSystem_GetSpawnGroups`.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<GetSpawnGroups_addr>", "name": "CSpawnGroupMgrGameSystem_GetSpawnGroups"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CSpawnGroupMgrGameSystem_GetSpawnGroups`.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results for `CSpawnGroupMgrGameSystem_GetSpawnGroups`.

   Required parameters:
   - `func_name`: `CSpawnGroupMgrGameSystem_GetSpawnGroups`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 6

## Signature Pattern

The function is called right before a logging call with format string:
```
%d spawn groups:\n
```
Source file: `game\shared\spawngroupmgrgamesystem.cpp`

## Function Characteristics

- **Parameters**: `(this, &out_spawn_group_ids)` where `this` is CSpawnGroupMgrGameSystem pointer, `out_spawn_group_ids` is a pointer to an output structure receiving spawn group count and ID array
- **Purpose**: Retrieves the list of spawn group IDs managed by the spawn group manager game system
- **Not a virtual function**: This is a regular member function, no vtable lookup needed

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CSpawnGroupMgrGameSystem_GetSpawnGroups.windows.yaml`
- `libserver.so` → `CSpawnGroupMgrGameSystem_GetSpawnGroups.linux.yaml`
