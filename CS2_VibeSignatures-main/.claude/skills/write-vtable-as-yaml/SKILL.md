---
name: write-vtable-as-yaml
description: Write vtable analysis results as YAML file beside the binary using IDA Pro MCP. Use this skill after locating a vtable to persist the results in a standardized YAML format.
---

# Write VTable as YAML

Persist vtable analysis results to a YAML file beside the binary using IDA Pro MCP.

## Prerequisites

Before using this skill, you should have:
1. Located the target vtable address
2. Identified the class name for the vtable

## Required Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `vtable_class` | Class name for the vtable | `CSource2Server` |
| `vtable_va` | Virtual address of the vtable | `0x182B8D9D8` |

## Optional Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `vtable_symbol` | The IDA symbol name for the vtable | "??_7CBaseEntity@@6B@" |

## Method

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi
import ida_bytes
import ida_name
import os
import yaml

# === REQUIRED: Replace these values ===
vtable_class = "<vtable_class>"     # e.g., "CBaseEntity"
vtable_va = <vtable_va>             # e.g., 0x182B8D9D8
# ======================================

# === OPTIONAL: Replace these values ===
vtable_symbol = "<vtable_symbol>"     # e.g., "??_7CBaseEntity@@6B@" or "_ZTV11CBaseEntity + 0x10" or "off_180XXXXXX"
# ======================================

input_file = idaapi.get_input_file_path()
dir_path = os.path.dirname(input_file)

if input_file.endswith('.dll'):
    platform = 'windows'
    image_base = idaapi.get_imagebase()
else:
    platform = 'linux'
    image_base = 0x0

vtable_rva = vtable_va - image_base

# Handle Linux vtables (skip RTTI metadata)
vtable_name = ida_name.get_name(vtable_va) or ""
if vtable_name.startswith("_ZTV"):
    vtable_va = vtable_va + 0x10
    vtable_rva = vtable_va - image_base

# Determine pointer size and count virtual functions
ptr_size = 8 if idaapi.inf_is_64bit() else 4
vtable_entries = []

for i in range(1000):
    if ptr_size == 8:
        ptr_value = ida_bytes.get_qword(vtable_va + i * ptr_size)
    else:
        ptr_value = ida_bytes.get_dword(vtable_va + i * ptr_size)

    if ptr_value == 0 or ptr_value == 0xFFFFFFFFFFFFFFFF:
        break

    func = idaapi.get_func(ptr_value)
    if func is None:
        flags = ida_bytes.get_full_flags(ptr_value)
        if not ida_bytes.is_code(flags):
            break

    vtable_entries.append(ptr_value)

count = len(vtable_entries)
vtable_size = count * ptr_size

# Build YAML data structure
yaml_data = {
    'vtable_class': vtable_class,
    'vtable_symbol': vtable_symbol,
    'vtable_va': hex(vtable_va),
    'vtable_rva': hex(vtable_rva),
    'vtable_size': hex(vtable_size),
    'vtable_numvfunc': count,
    'vtable_entries': {i: hex(entry) for i, entry in enumerate(vtable_entries)}
}

yaml_path = os.path.join(dir_path, f"{vtable_class}_vtable.{platform}.yaml")
with open(yaml_path, 'w', encoding='utf-8') as f:
    yaml.dump(yaml_data, f, default_flow_style=False, sort_keys=False, allow_unicode=True)
print(f"Written to: {yaml_path}")
"""
```

## Output File Naming Convention

The output YAML filename follows this pattern:
- `<vtable_class>_vtable.<platform>.yaml`

Examples:
- `server.dll` → `CSource2Server_vtable.windows.yaml`
- `libserver.so` / `libserver.so` → `CSource2Server_vtable.linux.yaml`

## Output YAML Format

`CSource2Server_vtable.windows.yaml` - Example for CSource2Server vtable on Windows:

```yaml
vtable_class: CSource2Server
vtable_symbol: off_180XXXXXX # Symbol in IDA to CSource2Server's vtable
vtable_va: 0x182B8D9D8       # Virtual address - changes with game updates
vtable_rva: 0x2B8D9D8        # Relative virtual address (VA - image base) - changes with game updates
vtable_size: 0x2D8           # VTable size in bytes - changes with game updates
vtable_numvfunc: 97          # Number of virtual functions - changes with game updates
vtable_entries:              # Every virtual functions starting from vtable[0]
  0: 0x180C87B20             # vtable[0] - changes with game updates
  1: 0x180C87FA0             # vtable[1] - changes with game updates
  2: 0x180C87FF0             # vtable[2] - changes with game updates
```

`CSource2Server_vtable.linux.yaml` - Example for CSource2Server vtable on linux:

```yaml
vtable_class: CSource2Server
vtable_symbol: _ZTV14CSource2Server + 0x10 # Symbol in IDA to CSource2Server's vtable
vtable_va: '0x2261dd8'       # Virtual address - changes with game updates
vtable_rva: '0x2261dd8'      # Relative virtual address (VA - image base) - changes with game updates
vtable_size: '0x310'         # VTable size in bytes - changes with game updates
vtable_numvfunc: 98          # Number of virtual functions - changes with game updates
vtable_entries:              # Every virtual functions starting from vtable[0]
  0: '0x16ea780'             # vtable[0] - changes with game updates
  1: '0x16e9b50'             # vtable[1] - changes with game updates
  2: '0x16e3270'             # vtable[2] - changes with game updates
```

## Platform Detection

The skill automatically detects the platform based on file extension:
- `.dll` → Windows (uses `idaapi.get_imagebase()` for image base)
- `.so` → Linux (uses `0x0` as image base, skips RTTI metadata for `_ZTV` prefixed vtables)

## Linux VTable Handling

For Linux binaries, vtables with `_ZTV` prefix (mangled vtable names) have RTTI metadata at the beginning:
- Offset 0x00: offset to top
- Offset 0x08: RTTI pointer
- Offset 0x10: First virtual function pointer

The skill automatically skips this metadata when counting virtual functions.

## Notes

- All values marked "changes with game updates" should be regenerated when analyzing new binary versions
- The YAML file is written to the same directory as the input binary
- vtable_size is automatically calculated as `vtable_numvfunc * pointer_size`
- vtable_rva is automatically calculated as `vtable_va - image_base`
