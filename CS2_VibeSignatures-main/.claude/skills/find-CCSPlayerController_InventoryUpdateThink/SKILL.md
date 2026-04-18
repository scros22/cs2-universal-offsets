---
name: find-CCSPlayerController_InventoryUpdateThink
description: Find and identify the CCSPlayerController_InventoryUpdateThink wrapper function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the InventoryUpdateThink think function wrapper by searching for the think function name string in schema registration and tracing through the schema structure to find the wrapper function pointer.
disable-model-invocation: true
---

# Find CCSPlayerController_InventoryUpdateThink

Locate `CCSPlayerController_InventoryUpdateThink` (the think function wrapper) in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Background

Think functions in Source 2 are registered through a schema system. The wrapper function pointer is stored in a schema structure, not directly referenced by the string. This requires:
1. Finding the string
2. Locating the schema initialization function
3. Reading the structure to find the wrapper function pointer

## Method

### 1. Search for the think function name string

```
mcp__ida-pro-mcp__find_regex pattern="InventoryUpdateThink"
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
qword_XXXXXX = sub_1F007F0("CCSPlayerController", "InventoryUpdateThink");
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
```c
const char *__fastcall sub_XXXXXX(__int64 a1)
{
  return InventoryUpdateThink(*(_QWORD *)(a1 + 2744));
}
```

Where:
- `a1` is `CCSPlayerController*`
- Offset `2744` (0xAB8) retrieves a member pointer
- Calls the real `InventoryUpdateThink` function

### 6. Rename the wrapper function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<wrapper_addr>", "name": "CCSPlayerController_InventoryUpdateThink"}]}
```

### 7. Generate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature.

### 8. Write IDA analysis output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CCSPlayerController_InventoryUpdateThink`
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

- **Wrapper Size**: ~12 bytes (very small thunk function)
- **Pattern**: `mov rdi, [rdi+offset]; jmp RealFunction`
- **Member Offset**: 0xAB8 (2744 decimal) - may change between versions

## Assembly Pattern

```asm
48 8B BF B8 0A 00 00    mov  rdi, [rdi+0xAB8]   ; Get member pointer
E9 XX XX XX XX          jmp  InventoryUpdateThink ; Tail call
```

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSPlayerController_InventoryUpdateThink.windows.yaml`
- `libserver.so` / `libserver.so` → `CCSPlayerController_InventoryUpdateThink.linux.yaml`

## Notes

- This is NOT a virtual function, so no vtable information is needed
- The wrapper is a simple thunk that retrieves a member and tail-calls the real function
- The signature includes the member offset which may change if CCSPlayerController layout changes
