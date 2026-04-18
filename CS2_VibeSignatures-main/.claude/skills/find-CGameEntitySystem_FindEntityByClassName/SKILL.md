---
name: find-CGameEntitySystem_FindEntityByClassName
description: Find and identify the CGameEntitySystem_FindEntityByClassName function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the entity-by-classname lookup function by searching for known debug string references in a caller function and analyzing cross-references.
disable-model-invocation: true
---

# Find CGameEntitySystem_FindEntityByClassName

Locate `CGameEntitySystem_FindEntityByClassName` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the debug string used by a caller function:
   ```
   mcp__ida-pro-mcp__find_regex pattern="TRAIN: %s, Nearest track is %s"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. In the decompiled code, identify the call matching this pattern:
   ```c
   v4 = sub_XXXXXXXX(qword_YYYYYYYY, 0, (int)"path_track");
   // Called in a loop iterating all path_track entities
   // Pattern: called twice - once with 0 as second arg, then with previous result
   ```
   The function called with `(global_ptr, startEntity, "path_track")` is `CGameEntitySystem_FindEntityByClassName`.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CGameEntitySystem_FindEntityByClassName"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CGameEntitySystem_FindEntityByClassName`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## Signature Pattern

The function is identified indirectly. A caller function contains the debug string:
```
TRAIN: %s, Nearest track is %s\n
```

The caller iterates all `path_track` entities using `CGameEntitySystem_FindEntityByClassName` in a loop pattern:
```c
v4 = CGameEntitySystem_FindEntityByClassName(g_pGameEntitySystem, 0, "path_track");
do {
    // distance calculation logic
    v4 = CGameEntitySystem_FindEntityByClassName(g_pGameEntitySystem, v4, "path_track");
} while (v4);
```

## Function Characteristics

- **Parameters**: `(CGameEntitySystem* this, CEntityInstance* startEntity, const char* className)`
  - `this`: CGameEntitySystem global pointer (stored in a global variable)
  - `startEntity`: Previous entity to continue iteration from (0 for first call)
  - `className`: Entity class name string to search for
- **Return**: `CEntityInstance*` — next matching entity, or NULL if none found
- **Not a virtual function** — this is a regular member function

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CGameEntitySystem_FindEntityByClassName.windows.yaml`
- `libserver.so` → `CGameEntitySystem_FindEntityByClassName.linux.yaml`
