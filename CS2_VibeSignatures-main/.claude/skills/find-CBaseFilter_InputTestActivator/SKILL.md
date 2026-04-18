---
name: find-CBaseFilter_InputTestActivator
description: Find and identify the CBaseFilter_InputTestActivator function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the InputTestActivator handler by searching for the "InputTestActivator" string and analyzing cross-references.
disable-model-invocation: true
---

# Find CBaseFilter_InputTestActivator

Locate `CBaseFilter_InputTestActivator` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="InputTestActivator"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify the CBaseFilter_InputTestActivator function:
   - In the decompiled code, look for the following pattern where entity input descriptors are being registered:
     ```c
     off_XXXXXXXX = "Negated";
     word_XXXXXXXXX = 1;
     off_XXXXXXXX = "InputTestActivator";
     off_XXXXXXXX = "TestActivator";
     off_XXXXXXXX = sub_XXXXXXXXXX; // This is CBaseFilter_InputTestActivator
     ```
   - The function pointer assigned right after the `"TestActivator"` string assignment is `CBaseFilter_InputTestActivator`.
   - Decompile that function pointer target to verify it is the correct function.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CBaseFilter_InputTestActivator"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CBaseFilter_InputTestActivator`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## String References

The function is located by finding the entity input descriptor registration that references:
- `"InputTestActivator"` - the input name
- `"TestActivator"` - the display name
- `"Negated"` - a nearby field in the same descriptor block

## Function Characteristics

- **Type**: Entity input handler for CBaseFilter
- **Parameters**: `(this, inputdata)` where `this` is CBaseFilter pointer, `inputdata` is the input event data
- **Purpose**: Handles the TestActivator input on filter entities, testing whether an activator passes the filter

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBaseFilter_InputTestActivator.windows.yaml`
- `libserver.so` → `CBaseFilter_InputTestActivator.linux.yaml`
