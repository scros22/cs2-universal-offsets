---
name: find-CCSPlayer_ItemServices_GiveNamedItem
description: Find and identify the CCSPlayer_ItemServices_GiveNamedItem function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the GiveNamedItem wrapper function in CCSPlayer_ItemServices vtable by searching for the "GiveNamedItem: interpreting" string reference and analyzing vtable entries.
disable-model-invocation: true
---

# Find CCSPlayer_ItemServices_GiveNamedItem

Locate `CCSPlayer_ItemServices_GiveNamedItem` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

### 1. Search for the target string

Search for the debug string used by the underlying GiveNamedItem function:

```
mcp__ida-pro-mcp__find_regex pattern="GiveNamedItem: interpreting"
```

This will find: `"GiveNamedItem: interpreting '%s' as '%s'\n"`

### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

This identifies the main `GiveNamedItem` function that uses this string.

### 3. Read CCSPlayer_ItemServices vtable information from yaml

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CCSPlayer_ItemServices`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract these values for subsequent steps:
- `vtable_va`: The vtable start address (use as `<VTABLE_START>`)
- `vtable_numvfunc`: The valid vtable entry count (last valid index = count - 1)
- `vtable_entries`: An array of virtual functions starting from vtable[0]

### 4. Read vtable entries at indices 18-24

Get the vtable address from step 3, then read the virtual function pointers:

```
mcp__ida-pro-mcp__get_int queries=[
  {"addr": "<vtable_addr + 0xb0>", "ty": "u64le"},
  {"addr": "<vtable_addr + 0xb8>", "ty": "u64le"},
  {"addr": "<vtable_addr + 0xc0>", "ty": "u64le"},
  {"addr": "<vtable_addr + 0xc8>", "ty": "u64le"},
  {"addr": "<vtable_addr + 0xd0>", "ty": "u64le"},
  {"addr": "<vtable_addr + 0xd8>", "ty": "u64le"},
  {"addr": "<vtable_addr + 0xe0>", "ty": "u64le"}
]
```

### 5. Decompile each vtable function

For each function address retrieved in step 4:

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

### 6. Identify the target function

For EACH function from step 5, determine whether:
- **A)** The function directly references the target string from step 1, OR
- **B)** The function calls (directly or indirectly) the GiveNamedItem function from step 2

The target function will be the FIRST wrapper that directly calls `GiveNamedItem`. Look for decompiled code like:

```c
__int64 __fastcall sub_XXXXXX(__int64 a1, char *a2, double a3, float a4) //This is actually the vfunc "CCSPlayer_ItemServices::GiveNamedItem", or what we called "CCSPlayer_ItemServices_GiveNamedItem"
{
  return GiveNamedItem(a1, a2, 0, 0, 0, 0, a3, a4);//It calls a regular function "GiveNamedItem"
}
```

### 7. Rename the function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr_sub_XXXXXX>", "name": "CCSPlayer_ItemServices_GiveNamedItem"}]}
```

### 8. Find vtable offset and index

**ALWAYS** Use SKILL `/get-vtable-index` to get vtable offset and index for the function.

VTable class name: `CCSPlayer_WeaponServices`

### 9. Generate and validate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 10. Write IDA analysis output as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CCSPlayer_ItemServices_GiveNamedItem`
- `func_addr`: The function address from step 6
- `func_sig`: The validated signature from step 9

Vtable parameters:
- `vtable_name`: `CCSPlayer_ItemServices`
- `vfunc_offset`: The offset from step 8
- `vfunc_index`: The index from step 8

## Function Characteristics

- **Type**: Virtual function wrapper
- **Purpose**: Thin wrapper around the main `GiveNamedItem` function
- **Parameters**:
  - `this` - CCSPlayer_ItemServices pointer
  - `itemName` - Name of the item to give
  - Additional parameters for position/rotation (zeroed in wrapper)
- **Implementation**: Calls `GiveNamedItem` with zeroed optional parameters

## Signature Pattern

The target function is a simple wrapper that:
1. Clears registers (r9d, r8d, ecx, edx) to zero
2. Jumps directly to the main `GiveNamedItem` function

This pattern makes the function very short (typically 15 bytes) and requires including surrounding context (padding or next function prologue) for signature uniqueness.

## Vtable Information

### Windows (server.dll)
- **Vtable Name**: `CCSPlayer_ItemServices::\`vftable'`
- **Vtable Mangled Name**: `??_7CCSPlayer_ItemServices@@6B@`
- **Vtable Index**: 20 - May change with game updates
- **Vtable Offset**: 0xb0 - May change with game updates

### Linux (libserver.so)
- **Vtable Name**: `vtable for CCSPlayer_ItemServices`
- **Vtable Mangled Name**: `_ZTV22CCSPlayer_ItemServices`
- **Vtable Index**: 20 - May change with game updates
- **Vtable Offset**: 0xb0 (from virtual function table start, +0x10 from symbol start) - May change with game updates

**Important for Linux:** The first 16 bytes of the vtable symbol are RTTI metadata. The actual virtual function pointers start at `_ZTV22CCSPlayer_ItemServices + 0x10`.

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSPlayer_ItemServices_GiveNamedItem.windows.yaml`
- `libserver.so` → `CCSPlayer_ItemServices_GiveNamedItem.linux.yaml`

## Related Functions

Other vtable entries at indices 18-24 include:
- **Index 18**: Returns 0 (nullsub)
- **Index 19**: Destructor wrapper
- **Index 20**: `CCSPlayer_ItemServices_GiveNamedItem` ⭐ (This function)
- **Index 21**: `GiveNamedItem` with different parameters (returns bool)
- **Index 22**: Another `GiveNamedItem` wrapper
- **Index 23-24**: Other item service functions

Only indices 20, 21, and 22 directly call the main `GiveNamedItem` function.
