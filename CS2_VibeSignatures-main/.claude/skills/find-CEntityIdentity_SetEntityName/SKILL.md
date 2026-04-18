---
name: find-CEntityIdentity_SetEntityName
description: |
  Find and identify the CEntityIdentity_SetEntityName function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the SetEntityName function
  on CEntityIdentity, which sets the targetname of an entity.
  Triggers: CEntityIdentity_SetEntityName, SetEntityName, entity name, set entity name
disable-model-invocation: true
---

# CEntityIdentity_SetEntityName Function Location Workflow

## Overview

This workflow locates `CEntityIdentity_SetEntityName` in CS2 server binary files. This function sets the targetname (entity name) on a `CEntityIdentity` object. It is called from a V8 script binding wrapper that logs `"Entity"` / `"SetEntityName"`.

## Location Steps

### 1. Search for Signature Strings

Use `find_regex` to search for the `SetEntityName` string:

```
mcp__ida-pro-mcp__find_regex(pattern="SetEntityName")
```

Expected result: Find string addresses containing `"SetEntityName"`.

### 2. Find Cross-References

Use `xrefs_to` to find locations that reference the `"SetEntityName"` string:

```
mcp__ida-pro-mcp__xrefs_to(addrs="<string_addr>")
```

Expected result: Find the V8 binding wrapper function that references `"SetEntityName"` multiple times alongside `"Entity"`.

### 3. Decompile the V8 Binding Wrapper

Decompile the function found in step 2 and locate the following code pattern:

```c
if ( *(int *)(a1 + 16) >= 1 )
{
    v16 = 0LL;
    if ( (unsigned __int8)sub_XXXXX(v17) )
    {
      v14 = byte_XXXXX;
      if ( v16 )
        v14 = v16;
      sub_XXXXX(*(_QWORD *)(v13 + 16), (__int64)v14);  // <-- This is CEntityIdentity_SetEntityName
    }
    else if ( (unsigned __int8)LoggingSystem_IsChannelEnabled(...) )
    {
      LoggingSystem_Log(..., "Method %s.%s invoked with bad %s value. (parameter #%d)\n",
        "Entity", "SetEntityName", "name", 0);
    }
}
else if ( (unsigned __int8)LoggingSystem_IsChannelEnabled(...) )
{
    LoggingSystem_Log(..., "Method %s.%s requires %d argument(s), %s.",
      "Entity", "SetEntityName", 1, "(name: string)");
}
```

Key identification: `CEntityIdentity_SetEntityName` is the call `sub_XXXXX(*(_QWORD *)(v13 + 16), v14)` that takes **2 parameters** — the entity identity pointer (obtained via `*(_QWORD *)(v13 + 16)`) and the name string. It appears inside the success branch after the V8 argument validation, right before the bad-value error log that mentions `"name"` as parameter #0.

### 4. Rename Function

Use `rename` to give the function a meaningful name:

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<SetEntityName_func_addr>", "name": "CEntityIdentity_SetEntityName"}})
```

### 5. Generate and Validate Unique Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CEntityIdentity_SetEntityName`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 5

## Function Characteristics

The `CEntityIdentity_SetEntityName` function:

- Is **not** a virtual function (no vtable lookup needed)
- Takes 2 parameters: `(CEntityIdentity* identity, const char* name)`
- Is called from a V8 script binding wrapper that logs `"Entity"` / `"SetEntityName"`
- The first argument is the entity identity pointer accessed via `*(_QWORD *)(entity + 16)` (offset 0x10 from the entity instance)
- The wrapper also references the argument descriptor string `"(name: string)"`

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CEntityIdentity_SetEntityName.windows.yaml`
- `libserver.so` / `libserver.so` → `CEntityIdentity_SetEntityName.linux.yaml`
