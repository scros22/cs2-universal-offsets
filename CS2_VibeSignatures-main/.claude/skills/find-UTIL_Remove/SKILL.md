---
name: find-UTIL_Remove
description: Find and identify the UTIL_Remove function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the UTIL_Remove function by analyzing the Molotov extinguish pattern and identifying the finalization call after particle effects.
disable-model-invocation: true
---

# Find UTIL_Remove

Locate `UTIL_Remove` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the extinguish event string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="Molotov\.Extinguish"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Analyze the decompiled function to identify the extinguish-finalization pattern:

   Look for this sequence:
   - A call with "Molotov.Extinguish" string
   - A call to `sub_1576A10` with "particles/inferno_fx/extinguish_fire.vpcf" particle resource
   - Positional data from object pointer (e.g., `*(double *)(a1 + offset)`)
   - **Immediately after**: A function call taking only `this` (a1) as argument

   This final call is UTIL_Remove.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<util_remove_addr>", "name": "UTIL_Remove"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `UTIL_Remove`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## Signature Pattern

The function is identified by analyzing the extinguish-finalization pattern:

```c
sub_1857410(&v52, a1, "Molotov.Extinguish", 0, 1, &v67, 0.0);
sub_1576A10(
  "particles/inferno_fx/extinguish_fire.vpcf",
  0, 0, 0xFFFFFFFFLL, 0, 0,
  *(double *)(a1 + 5584),
  *(float *)(a1 + 5592),
  *(double *)&v58,
  *((float *)&v58 + 2));
sub_14FE9C0(a1);  // <- This is UTIL_Remove
```

## Function Characteristics

- **Parameters**: `(this)` where `this` is an entity pointer
- **Purpose**: Entity removal/cleanup function called after extinguish effects
- **Context**: Part of the Molotov/Incendiary grenade extinguish workflow
- **Call Pattern**: Takes only the entity pointer, no additional arguments
- **Position**: Called immediately after particle effect spawn, before function return

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `UTIL_Remove.windows.yaml`
- `libserver.so` → `UTIL_Remove.linux.yaml`
