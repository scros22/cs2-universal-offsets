---
name: find-CEntityIOOutput_FireOutputInternal
description: Find and identify the CEntityIOOutput_FireOutputInternal function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the FireOutputInternal function by searching for the error message string "Couldn't find output named '%s' on entity '%s'.\n" and analyzing cross-references.
disable-model-invocation: true
---

# Find CEntityIOOutput_FireOutputInternal

Locate `CEntityIOOutput_FireOutputInternal` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the error message string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="Couldn't find output named '%s' on entity '%s'"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

   This function is a helper/wrapper. Look for its callers to find the actual FireOutputInternal function.

4. Get cross-references to the wrapper function:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<wrapper_func_addr>"
   ```

5. Decompile the calling functions and identify `CEntityIOOutput_FireOutputInternal`:
   ```
   mcp__ida-pro-mcp__decompile addr="<caller_func_addr>"
   ```

   Look for a function with this pattern:
   - Takes multiple parameters including entity pointers and a float delay value
   - Iterates through an entity IO output list at offset +8 of the first parameter
   - Calls sub-functions to queue/fire entity IO events
   - Has a loop that processes pending outputs

6. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": {"addr": "<function_addr>", "name": "CEntityIOOutput_FireOutputInternal"}}
   ```

7. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

8. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CEntityIOOutput_FireOutputInternal`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 7

## String Reference

The function is indirectly related to this error message:
```
Couldn't find output named '%s' on entity '%s'.\n
```

This string appears in a helper function that validates entity outputs. The caller of that helper is `CEntityIOOutput_FireOutputInternal`.

## Function Characteristics

- **Parameters**: `(this, activator, caller, value, delay, outputID, ...)`
  - `this`: CEntityIOOutput pointer
  - `activator`: Entity that triggered the output
  - `caller`: Entity that owns the output
  - `value`: Variant value to pass
  - `delay`: Float delay before firing
  - `outputID`: Output identifier
- **Purpose**: Fires an entity I/O output, processing all connected targets with optional delay
- **Behavior**:
  - Iterates through registered callbacks at `this+8`
  - Queues delayed outputs or fires immediately based on delay value
  - Handles output removal after firing if configured

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CEntityIOOutput_FireOutputInternal.windows.yaml`
- `libserver.so` → `CEntityIOOutput_FireOutputInternal.linux.yaml`
