---
name: find-CGameEntitySystem_FindEntityByName
description: Find and identify the CGameEntitySystem_FindEntityByName function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the entity-by-name lookup function by searching for known string references in a caller function and analyzing cross-references.
disable-model-invocation: true
---

# Find CGameEntitySystem_FindEntityByName

Locate `CGameEntitySystem_FindEntityByName` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the string used by a caller function:
   ```
   mcp__ida-pro-mcp__find_regex pattern="commentary_semaphore"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile one of the referencing functions:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. In the decompiled code, identify the call matching this pattern:
   ```c
   // Pattern 1: early-return guard
   if ( sub_XXXXXXXX(qword_YYYYYYYY, 0, (unsigned int)"commentary_semaphore", 0, 0LL, 0LL, 0LL) )
       return;

   // Pattern 2: result assignment
   v8 = sub_XXXXXXXX(qword_YYYYYYYY, 0, (unsigned int)"commentary_semaphore", 0, 0LL, 0LL, 0LL);
   ```
   The function called with 7 arguments `(global_ptr, startEntity, name, searchingEntity, activator, caller, filter)` is `CGameEntitySystem_FindEntityByName`.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CGameEntitySystem_FindEntityByName"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CGameEntitySystem_FindEntityByName`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## Signature Pattern

The function is identified indirectly. Caller functions reference the string `"commentary_semaphore"` and pass it to `CGameEntitySystem_FindEntityByName` with 7 arguments.

The key distinguishing feature from `CGameEntitySystem_FindEntityByClassName` (3 args) is the 7-parameter signature:
```c
CGameEntitySystem_FindEntityByName(
    g_pGameEntitySystem,  // CGameEntitySystem* this
    0,                    // CEntityInstance* startEntity
    "commentary_semaphore", // const char* name
    0,                    // CEntityInstance* searchingEntity
    0LL,                  // CEntityInstance* activator
    0LL,                  // CEntityInstance* caller
    0LL                   // IEntityFindFilter* filter
);
```

## Function Characteristics

- **Parameters**: `(CGameEntitySystem* this, CEntityInstance* startEntity, const char* name, CEntityInstance* searchingEntity, CEntityInstance* activator, CEntityInstance* caller, IEntityFindFilter* filter)`
  - `this`: CGameEntitySystem global pointer (same global as FindEntityByClassName)
  - `startEntity`: Previous entity to continue iteration from (0 for first call)
  - `name`: Entity targetname string to search for
  - `searchingEntity`: Entity performing the search (can be NULL)
  - `activator`: Activator entity (can be NULL)
  - `caller`: Caller entity (can be NULL)
  - `filter`: Optional find filter interface (can be NULL)
- **Return**: `CEntityInstance*` — matching entity, or NULL if none found
- **Not a virtual function** — this is a regular member function

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CGameEntitySystem_FindEntityByName.windows.yaml`
- `libserver.so` → `CGameEntitySystem_FindEntityByName.linux.yaml`
