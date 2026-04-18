---
name: find-CNetworkMessages_GetNetworkSerializationContextData
description: |
  Find and identify the CNetworkMessages_GetNetworkSerializationContextData virtual function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 networksystem.dll or libnetworksystem.so to locate the getter vfunc
  by reusing the known CNetworkMessages_SetNetworkSerializationContextData slot and checking the adjacent CNetworkMessages vtable entry.
  Trigger: CNetworkMessages_GetNetworkSerializationContextData
disable-model-invocation: true
---

# Find CNetworkMessages_GetNetworkSerializationContextData

Locate `CNetworkMessages_GetNetworkSerializationContextData` vfunc in CS2 `networksystem.dll` or `libnetworksystem.so` using IDA Pro MCP tools.

## Method

### 1. Load CNetworkMessages_SetNetworkSerializationContextData from YAML

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CNetworkMessages_SetNetworkSerializationContextData`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vfunc_index`
- `vfunc_offset`

### 2. Load CNetworkMessages VTable from YAML

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CNetworkMessages`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vtable_numvfunc`
- `vtable_entries`

### 3. Resolve the Adjacent Getter Slot

Compute the candidate slot for `CNetworkMessages_GetNetworkSerializationContextData`:

- `target_vfunc_index = CNetworkMessages_SetNetworkSerializationContextData.vfunc_index + 1`
- `target_vfunc_offset = CNetworkMessages_SetNetworkSerializationContextData.vfunc_offset + 8`

Validate that `target_vfunc_index < vtable_numvfunc`, then read:

- `candidate_func_addr = CNetworkMessages_vtable[target_vfunc_index]`

This adjacent-slot rule is required because `GetNetworkSerializationContextData` immediately follows
`SetNetworkSerializationContextData` in the `CNetworkMessages` vtable for `networksystem.dll` / `libnetworksystem.so`.

### 4. Decompile the Getter Candidate

Decompile the resolved adjacent entry:

```text
mcp__ida-pro-mcp__decompile addr="<candidate_func_addr>"
```

Confirm the candidate matches the network-serialization-context getter pattern.

Expected behavior:

1. Takes `(this, key_name)` arguments
2. Uses an internal lock around the lookup
3. Looks up `key_name` from the internal symbol table owned by `CNetworkMessages`
4. Returns the resolved 16-bit context value, or `0xFFFF` / `-1` when not found

Windows example shape:

```c
__int64 __fastcall CNetworkMessages_GetNetworkSerializationContextData(..., __int64 key_name)
{
  AcquireSRWLockExclusive(...);
  CUtlSymbolTable::Find(..., key_name);
  ...
  ReleaseSRWLockExclusive(...);
  return ...;
}
```

Linux example shape:

```c
__int64 __fastcall CNetworkMessages_GetNetworkSerializationContextData(__int64 thisptr, __int64 key_name)
{
  ...
  sub_176D10(..., thisptr + ..., key_name);
  ...
  return ...;
}
```

The exact helper names and structure offsets may change across game updates. The identification rule is:

1. The function is the next vtable entry after `SetNetworkSerializationContextData`
2. It performs a locked lookup from the `CNetworkMessages` serialization-context symbol table
3. It returns the resolved small integer context value instead of writing data into the object

If all three conditions hold, the candidate is `CNetworkMessages_GetNetworkSerializationContextData`.

### 5. Generate Function Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` with `addr=<candidate_func_addr>` to generate a robust and unique `func_sig` for `CNetworkMessages_GetNetworkSerializationContextData`.

Use the returned validated `func_sig` in the next step.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CNetworkMessages_GetNetworkSerializationContextData`
- `func_addr`: `<candidate_func_addr>`
- `func_sig`: The validated signature from step 5
- `vfunc_sig`: `None`

VTable parameters:
- `vtable_name`: `CNetworkMessages`
- `vfunc_offset`: `<target_vfunc_offset>` in hex
- `vfunc_index`: `<target_vfunc_index>`

## Function Characteristics

- **Purpose**: Returns the network serialization context id associated with a key name from the `CNetworkMessages` symbol table
- **Binary**: `networksystem.dll` / `libnetworksystem.so`
- **Parameters**: `(this, key_name)`
- **Return value**: A small integer / symbol id for the serialization context, typically `0xFFFF` when the key is not found

## Discovery Strategy

1. Reuse the existing `CNetworkMessages_SetNetworkSerializationContextData` YAML to obtain the authoritative slot index
2. Reuse the existing `CNetworkMessages_vtable` YAML to resolve the adjacent vtable entry
3. Confirm the adjacent candidate is a locked symbol-table lookup getter for serialization-context data
4. Generate a stable `func_sig` from the resolved getter body

This is robust because:
- The vtable adjacency between setter and getter is explicit
- The getter has a distinctive locked symbol-table lookup structure on both Windows and Linux
- The final YAML stores both the function signature and the precise vtable metadata

## Output YAML Format

The output YAML filename depends on the platform:
- `networksystem.dll` -> `CNetworkMessages_GetNetworkSerializationContextData.windows.yaml`
- `libnetworksystem.so` -> `CNetworkMessages_GetNetworkSerializationContextData.linux.yaml`
