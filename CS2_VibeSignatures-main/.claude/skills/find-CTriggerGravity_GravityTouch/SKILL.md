---
name: find-CTriggerGravity_GravityTouch
description: Find and identify the CTriggerGravity_GravityTouch function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the GravityTouch handler by searching for the "GravityTouch" string reference and analyzing the registration pattern.
disable-model-invocation: true
---

# Find CTriggerGravity_GravityTouch

Locate `CTriggerGravity_GravityTouch` in CS2 `server.dll` or `libserver.so` using IDA Pro MCP tools.

## Method

1. Search for the string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="GravityTouch"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify `CTriggerGravity_GravityTouch` from the registration pattern:

   On Windows:
   ```c
   qword_XXXXXXXX = sub_XXXXXXXX("CTriggerGravity", "GravityTouch");
   dword_XXXXXXXX = 0;
   qword_XXXXXXXX = (__int64)sub_YYYYYYYY; // This is CTriggerGravity_GravityTouch
   ```

   On Linux:
   ```c
   v1 = sub_XXXXXXXX("CTriggerGravity", "GravityTouch");
   qword_XXXXXXXX = 0LL;
   qword_XXXXXXXX = v1;
   qword_XXXXXXXX = (__int64)sub_YYYYYYYY; // This is CTriggerGravity_GravityTouch
   ```

   The function pointer assigned a few lines after the `("CTriggerGravity", "GravityTouch")` call is `CTriggerGravity_GravityTouch`.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<CTriggerGravity_GravityTouch_addr>", "name": "CTriggerGravity_GravityTouch"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CTriggerGravity_GravityTouch`.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results for `CTriggerGravity_GravityTouch`.

   Required parameters:
   - `func_name`: `CTriggerGravity_GravityTouch`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## Signature Pattern

The function is registered via a `("CTriggerGravity", "GravityTouch")` call in a static initializer. The function pointer is assigned to a global variable shortly after the registration call.

## Function Characteristics

- **Class**: `CTriggerGravity`
- **Prototype**: `void CTriggerGravity::GravityTouch(CBaseEntity* pOther)`
- **Parameters**:
  - `this`: Pointer to the CTriggerGravity instance
  - `pOther`: Pointer to the entity touching the gravity trigger
- **Behavior**:
  1. Validates the touching entity via vtable call (checks entity type)
  2. Reads the gravity scale value from the trigger entity
  3. Applies the gravity value to the touching entity

## DLL Information

- **DLL**: `server.dll` (Windows) / `libserver.so` (Linux)

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CTriggerGravity_GravityTouch.windows.yaml`
- `libserver.so` → `CTriggerGravity_GravityTouch.linux.yaml`
