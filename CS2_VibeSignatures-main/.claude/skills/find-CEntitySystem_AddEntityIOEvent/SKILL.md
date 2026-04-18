---
name: find-CEntitySystem_AddEntityIOEvent
description: Find and identify the CEntitySystem_AddEntityIOEvent function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the AddEntityIOEvent function by searching for the "_DisableUpdateTarget" string reference and analyzing cross-references to find the function called with "SetPosition" parameter.
disable-model-invocation: true
---

# Find CEntitySystem_AddEntityIOEvent

Locate `CEntitySystem_AddEntityIOEvent` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the input event string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="_DisableUpdateTarget"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing functions to find the pattern:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify the CEntitySystem_AddEntityIOEvent function:
   - Look for a function that contains calls to a function with "SetPosition" as a parameter
   - The pattern looks like:
     ```c
     sub_XXXXXX(qword_XXXXXX, a1, "_DisableUpdateTarget", a1, a1, 0, 0, 0LL, 0LL);
     // ... setup code ...
     sub_YYYYYYYY(qword_XXXXXX, a1, "SetPosition", a1, a1, &v27, v16, v20, 0LL, 0LL);
     ```
   - The function `sub_YYYYYYYY` called with "SetPosition" parameter is `CEntitySystem_AddEntityIOEvent`

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CEntitySystem_AddEntityIOEvent"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CEntitySystem_AddEntityIOEvent`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## String References

The function is identified by finding code that:
- Calls `CEntitySystem_FireEntityIOEvent` (or similar) with `"_DisableUpdateTarget"`
- Then calls `CEntitySystem_AddEntityIOEvent` multiple times with `"SetPosition"`
- Finally calls with `"_EnableUpdateTarget"`

## Function Characteristics

- **Parameters**:
  - `arg0`: Entity system pointer (global)
  - `arg1`: Target entity pointer
  - `arg2`: Input name (const char*, e.g., "SetPosition")
  - `arg3`: Activator entity
  - `arg4`: Caller entity
  - `arg5`: Value pointer
  - `arg6`: Delay (float as int)
  - `arg7`: Output ID
  - `arg8`: Reserved (0)
  - `arg9`: Reserved (0)

- **Purpose**: Adds an entity I/O event to be fired after a specified delay

## Related Functions

- `CEntitySystem_FireEntityIOEvent` - Fires entity I/O events immediately (called with "_DisableUpdateTarget", "_EnableUpdateTarget")
- `CEntitySystem_AddEntityIOEvent` - Queues entity I/O events with delay (called with "SetPosition")

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CEntitySystem_AddEntityIOEvent.windows.yaml`
- `libserver.so` → `CEntitySystem_AddEntityIOEvent.linux.yaml`
