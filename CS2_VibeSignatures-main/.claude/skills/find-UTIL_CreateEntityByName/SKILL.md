---
name: find-UTIL_CreateEntityByName
description: Find and identify the UTIL_CreateEntityByName function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the entity creation utility function by searching for the "Attempted to spawn a not-spawnable entity classname" error string reference and analyzing cross-references to identify the semantic wrapper function.
disable-model-invocation: true
---

# Find UTIL_CreateEntityByName

Locate `UTIL_CreateEntityByName` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the error string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="Attempted to spawn a not-spawnable entity classname"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Identify the internal spawning function and get its callers:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<internal_spawn_function_addr>"
   ```

4. Decompile the two small wrapper functions to identify the semantic wrapper:
   ```
   mcp__ida-pro-mcp__decompile addr="<wrapper1_addr>"
   mcp__ida-pro-mcp__decompile addr="<wrapper2_addr>"
   ```

5. Identify the semantic wrapper by analyzing:
   - The pure forwarder passes through all caller parameters
   - The semantic wrapper **injects a hardcoded `-1` constant** into parameter position 2
   - This constant enforces specific spawn behavior (default mode/flag)

6. Rename the semantic wrapper function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<semantic_wrapper_addr>", "name": "UTIL_CreateEntityByName"}]}
   ```

7. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

8. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `UTIL_CreateEntityByName`
   - `func_addr`: The semantic wrapper address from step 6
   - `func_sig`: The validated signature from step 7

## Error String Pattern

The internal spawning function contains an error message:
```
Attempted to spawn a not-spawnable entity classname "%s"!\n
```

## Function Characteristics

- **Type**: Non-virtual utility function (wrapper)
- **Parameters**: `(classname, unknown_param)` where `classname` is the entity class name string
- **Purpose**: Simplified entity creation interface that enforces default spawn behavior
- **Implementation**: Wraps internal spawn function by injecting `-1` constant for default mode

## Identifying the Semantic Wrapper

Among the caller functions to the internal spawn function, identify the semantic wrapper by:

1. **Look for two small wrappers of similar size** (typically ~0x30 bytes each)
2. **Compare their parameter handling**:
   - Pure forwarder: Passes all caller parameters through
   - Semantic wrapper: **Injects constant `-1` into parameter position 2**
3. **The semantic wrapper is UTIL_CreateEntityByName** - it enforces specific spawn semantics

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `UTIL_CreateEntityByName.windows.yaml`
- `libserver.so` → `UTIL_CreateEntityByName.linux.yaml`
