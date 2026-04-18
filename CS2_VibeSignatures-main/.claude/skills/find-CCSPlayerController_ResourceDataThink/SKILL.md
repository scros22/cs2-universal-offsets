---
name: find-CCSPlayerController_ResourceDataThink
description: Find and identify the CCSPlayerController_ResourceDataThink wrapper function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the ResourceDataThink think function wrapper by searching for the think function name string in schema registration and tracing through the schema structure to find the wrapper function pointer.
disable-model-invocation: true
---

# Find CCSPlayerController_ResourceDataThink

Locate `CCSPlayerController_ResourceDataThink` (the think function wrapper) in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Background

Think functions in Source 2 are registered through a schema system. The wrapper function pointer is stored in a schema structure, not directly referenced by the string. This requires:
1. Finding the string
2. Locating the schema initialization function
3. Reading the structure to find the wrapper function pointer

## Method

### 1. Search for the think function name string

```
mcp__ida-pro-mcp__find_regex pattern="ResourceDataThink"
```

### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

### 3. Decompile the referencing function (schema init)

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

Look for the pattern:
```c
qword_XXXXXX = sub_1F007F0("CCSPlayerController", "ResourceDataThink");
```

Note: `sub_1F007F0` is a string concatenation function, NOT the actual think function.

### 4. Find the wrapper function pointer in schema structure

The schema structure stores the wrapper function pointer at a fixed offset from the name string pointer. Read bytes around the name storage location:

```python
mcp__ida-pro-mcp__py_eval code="""
import ida_bytes

# Address where the name string pointer is stored (qword_XXXXXX from step 3)
name_storage_addr = <name_storage_addr>

# Read 128 bytes starting from offset -0x38 to find the wrapper pointer
# The wrapper pointer is typically at offset +0x28 from the name storage
raw_bytes = ida_bytes.get_bytes(name_storage_addr - 0x38, 128)

# Look for function pointers (addresses in .text segment range)
for i in range(0, len(raw_bytes) - 7, 8):
    ptr = int.from_bytes(raw_bytes[i:i+8], 'little')
    if 0x900000 < ptr < 0x2100000:  # Typical .text range for libserver.so
        print(f"Potential function pointer at offset {hex(i - 0x38)}: {hex(ptr)}")
"""
```

### 5. Verify and decompile the wrapper function

```
mcp__ida-pro-mcp__decompile addr="<wrapper_addr>"
```

The wrapper should look like:

Linux:
```c
__int64 __fastcall sub_XXXXXX(__int64 a1, double a2, double a3, double a4, double a5)
{
  float v5; // xmm0_4

  ++*(_DWORD *)(a1 + 3084);
  sub_YYYYYY(a1, a2, a3, a4, a5);
  v5 = sub_ZZZZZZ(*(_DWORD *)(*(_QWORD *)(a1 + 16) + 56LL));
  return sub_WWWWWW(a1, sub_XXXXXX, 0LL, 2578889LL, 0LL, v5 + 0.1);
}
```

Windows:
```c
double __fastcall sub_XXXXXX(__int64 a1)
{
  _DWORD *v2; // rax
  int v4; // [rsp+40h] [rbp+8h] BYREF
  char v5; // [rsp+48h] [rbp+10h] BYREF

  ++*(_DWORD *)(a1 + 2356);
  sub_YYYYYY();
  sub_ZZZZZZ(&v4, *(_DWORD *)(*(_QWORD *)(a1 + 16) + 56LL));
  sub_AAAAAA(&v5, &v4);
  return sub_WWWWWW(a1, (unsigned int)sub_XXXXXX, *v2, 2578889, 0LL);
}
```

Where:
- `a1` is `CCSPlayerController*`
- The function increments a counter at a member offset
- Calls a sub-function, then re-registers itself as a think function
- The magic constant `2578889` (0x275289) identifies this as `ResourceDataThink`

### 6. Rename the wrapper function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<wrapper_addr>", "name": "CCSPlayerController_ResourceDataThink"}]}
```

### 7. Generate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature.

### 8. Write IDA analysis output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CCSPlayerController_ResourceDataThink`
- `func_addr`: The wrapper function address from step 5
- `func_sig`: The validated signature from step 7

## Schema Structure Pattern

The CCSPlayerController schema registers multiple think functions:

| Think Function Name | Purpose |
|---------------------|---------|
| PlayerForceTeamThink | Force team assignment logic |
| ResetForceTeamThink | Reset force team state |
| ResourceDataThink | Resource data updates |
| InventoryUpdateThink | Inventory synchronization |

## Function Characteristics

- **Pattern**: Increments a counter, calls a sub-function, re-registers itself via think system
- **Magic Constant**: `2578889` (0x275289) — unique identifier for ResourceDataThink
- **Self-referencing**: The function passes its own address when re-registering the think callback

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSPlayerController_ResourceDataThink.windows.yaml`
- `libserver.so` / `libserver.so` → `CCSPlayerController_ResourceDataThink.linux.yaml`

## Notes

- This is NOT a virtual function, so no vtable information is needed
- Unlike InventoryUpdateThink (which is a simple thunk), ResourceDataThink contains actual logic
- The signature should capture the unique constant `2578889` (0x275289) for robustness
