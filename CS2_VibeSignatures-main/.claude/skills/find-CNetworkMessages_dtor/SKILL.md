---
name: find-CNetworkMessages_dtor
description: |
  Find and identify the CNetworkMessages destructor virtual function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 networksystem.dll or libnetworksystem.so to locate the CNetworkMessages_dtor vfunc
  by scanning the last three entries of the CNetworkMessages vtable.
  Trigger: CNetworkMessages_dtor
disable-model-invocation: true
---

# Find CNetworkMessages_dtor

Locate `CNetworkMessages_dtor` vfunc in CS2 `networksystem.dll` or `libnetworksystem.so` using IDA Pro MCP tools.

## Method

### 1. Load CNetworkMessages VTable from YAML

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CNetworkMessages`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vtable_numvfunc`
- `vtable_entries`

### 2. Identify Destructor Candidates

The destructor is located in the last three vtable entries. Check the entries at indices:

- `vtable_numvfunc - 3`
- `vtable_numvfunc - 2`
- `vtable_numvfunc - 1`

Read the function addresses from the vtable entries for these three slots.

### 3. Decompile and Identify the Destructor

Decompile all three candidate functions:

```text
mcp__ida-pro-mcp__decompile addr="<candidate_1_addr>"
mcp__ida-pro-mcp__decompile addr="<candidate_2_addr>"
mcp__ida-pro-mcp__decompile addr="<candidate_3_addr>"
```

#### Windows (`networksystem.dll`)

The destructor has this characteristic two-argument pattern:

```c
__int64 __fastcall CNetworkMessages_dtor(__int64 a1, char a2)
{
  CNetworkMessages_dtor2(a1);
  if ( (a2 & 1) != 0 )
    (*(void (__fastcall **)(_QWORD, __int64))(*g_pMemAlloc + 24LL))(g_pMemAlloc, a1);
  return a1;
}
```

Key identification rules for Windows:
1. Takes two parameters: `(__int64 a1, char a2)` (this + flags)
2. Calls another large destructor function (`CNetworkMessages_dtor2`) with just `a1`
3. Conditionally frees memory via `g_pMemAlloc` if `(a2 & 1) != 0`
4. Returns `a1`

The inner `CNetworkMessages_dtor2` function is a large function that:
- Writes `CNetworkMessages::vftable` back to `*this` as the first operation: `*(_QWORD *)a1 = &CNetworkMessages::vftable;`
- Contains a loop iterating 64 times destroying network message entries
- Calls multiple `CUtlSymbolTable::~CUtlSymbolTable` destructors
- Ends by setting `CConCommandMemberAccessor<CNetworkMessages>::vftable` on multiple member offsets

#### Linux (`libnetworksystem.so`)

The destructor has this characteristic single-argument pattern:

```c
__int64 __fastcall CNetworkMessages_dtor(__int64 a1)
{
  ...
  *(_QWORD *)a1 = CNetworkMessages_vtable;
  ...
}
```

Key identification rules for Linux:
1. Takes one parameter: `(__int64 a1)` (this only)
2. Writes `CNetworkMessages_vtable` (or `CNetworkMessages::vftable`) to `*(_QWORD *)a1` as the first substantive operation
3. Is a very large function (hundreds of lines) that performs extensive cleanup
4. Contains a loop destroying network message entries with `g_pMemAlloc` free calls
5. Ends by writing `CConCommandMemberAccessor` vtable pointers and conditionally calling unregister functions

### 4. Confirm the Destructor

The correct function among the three candidates is the one that matches the patterns above. Specifically:

- **Windows**: Look for the small wrapper that calls a large inner destructor and conditionally frees `this`
- **Linux**: Look for the very large function that starts by writing the vtable pointer to `*this`

If none of the three candidates match, **STOP** and report to user.

### 5. Generate Function Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` with `addr=<dtor_func_addr>` to generate a robust and unique `func_sig` for `CNetworkMessages_dtor`.

Use the returned validated `func_sig` in the next step.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CNetworkMessages_dtor`
- `func_addr`: `<dtor_func_addr>`
- `func_sig`: The validated signature from step 5
- `vfunc_sig`: `None`

VTable parameters:
- `vtable_name`: `CNetworkMessages`
- `vfunc_offset`: `<target_vfunc_offset>` in hex (computed as `vfunc_index * 8`)
- `vfunc_index`: `<target_vfunc_index>`

## Function Characteristics

- **Purpose**: Destructor for the `CNetworkMessages` singleton; tears down all registered network messages, symbol tables, and member structures
- **Binary**: `networksystem.dll` / `libnetworksystem.so`
- **Parameters (Windows)**: `(this, char flags)` where `flags & 1` controls whether to free the object's memory
- **Parameters (Linux)**: `(this)` only
- **Return value**: `this` pointer (Windows) / varies (Linux)

## Discovery Strategy

1. Load the existing `CNetworkMessages_vtable` YAML to get the full vtable
2. Scan the last three vtable entries (`numvfunc - 3` through `numvfunc - 1`) as destructor candidates
3. Decompile each candidate and match against the destructor patterns:
   - Windows: small wrapper calling inner dtor + conditional `g_pMemAlloc` free
   - Linux: large function starting with vtable pointer write-back
4. Generate a stable `func_sig` from the resolved destructor body

This is robust because:
- C++ destructors are always placed near the end of the vtable
- The destructor pattern (vtable write-back, extensive member cleanup) is highly distinctive
- Checking three candidates provides tolerance for vtable layout variations

## Output YAML Format

The output YAML filename depends on the platform:
- `networksystem.dll` -> `CNetworkMessages_dtor.windows.yaml`
- `libnetworksystem.so` -> `CNetworkMessages_dtor.linux.yaml`
