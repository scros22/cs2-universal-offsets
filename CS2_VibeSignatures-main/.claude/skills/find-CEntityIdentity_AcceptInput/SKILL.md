---
name: find-CEntityIdentity_AcceptInput
description: |
  Find and identify the CEntityIdentity_AcceptInput function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the AcceptInput function
  on CEntityIdentity, which dispatches entity I/O inputs.
  Triggers: CEntityIdentity_AcceptInput, AcceptInput, entity input dispatch, entity IO accept
disable-model-invocation: true
---

# CEntityIdentity_AcceptInput Function Location Workflow

## Overview

This workflow locates `CEntityIdentity_AcceptInput` in CS2 server binary files. This function is the core entity I/O input dispatcher on `CEntityIdentity`, responsible for accepting and executing named inputs on entities. It is called after entity IO event listeners have been iterated.

## Location Steps

### 1. Search for Signature String

Use `find_regex` to search for the `Invalid target entity.` string:

```
mcp__ida-pro-mcp__find_regex(pattern="Invalid target entity\\.")
```

Expected result: Find string address (e.g., `0x18164ACA8` for Windows, varies by version)

### 2. Find Cross-References

Use `xrefs_to` to find locations that reference this string:

```
mcp__ida-pro-mcp__xrefs_to(addrs="<string_addr>")
```

Expected result: Find the containing function (e.g., `sub_1808092D0`) that references the string in an error path.

### 3. Decompile the Containing Function

Decompile the function found in step 2 and locate the following code pattern:

```c
for ( i = &v23[*(int *)(qword_XXXXX + 2968)]; v23 != i; ++v23 )
{
    v25 = &unk_XXXXX;
    if ( *(_QWORD *)(a3 + 72) )
        v25 = *(void **)(a3 + 72);
    (*(void (__fastcall **)(...))(*(_QWORD *)*v23 + 8LL))(*v23, v20, v25, v22, v22, &v30, 0, v34);
}
if ( *(_QWORD *)(a3 + 72) )
    v21 = *(void **)(a3 + 72);
v26 = (__int64 *)sub_XXXXX(&v35, v21);
v27 = *(_QWORD **)(v20 + 16);
a5 = *v26;
sub_XXXXX(v27, (void **)&a5, v22, v22, (__int64)&v30, 0, (__int64)v34, 0LL);  // <-- This is CEntityIdentity_AcceptInput
sub_XXXXX(v34, v28);
if ( (v32 & 1) != 0 )
    (*(void (__fastcall **)(_QWORD, __int64))(*g_pMemAlloc + 24LL))(g_pMemAlloc, v30);
```

Key identification: `CEntityIdentity_AcceptInput` is the call that takes **8 parameters** `(entity_identity, input_value_ptr, caller, caller, variant_ptr, 0, params, 0)` immediately after the entity IO listener iteration loop and after fetching `v27 = *(_QWORD **)(v20 + 16)`.

### 4. Rename Function

Use `rename` to give the function a meaningful name:

```
mcp__ida-pro-mcp__rename(batch={"func": {"addr": "<AcceptInput_func_addr>", "name": "CEntityIdentity_AcceptInput"}})
```

### 5. Generate and Validate Unique Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CEntityIdentity_AcceptInput`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 5

## Function Characteristics

The `CEntityIdentity_AcceptInput` function:

- Is **not** a virtual function (no vtable lookup needed)
- Takes 8 parameters: `(CEntityIdentity* identity, CVariant* value, CEntityInstance* activator, CEntityInstance* caller, CVariant* variant, int outputID, CEntityIOOutput* params, int64 unknown)`
- Is called from within the entity I/O dispatch function that also references `"Invalid target entity."` in its error path
- Located after a for-loop that iterates entity IO event listeners via `qword_XXXXX + 2968/2976`

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CEntityIdentity_AcceptInput.windows.yaml`
- `libserver.so` / `libserver.so` → `CEntityIdentity_AcceptInput.linux.yaml`
