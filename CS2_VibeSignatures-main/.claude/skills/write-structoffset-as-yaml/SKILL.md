---
name: write-structoffset-as-yaml
description: Write struct member offset analysis results as YAML file beside the binary using IDA Pro MCP. Use this skill after identifying a struct member offset and optionally generating a signature for it to persist the results in a standardized YAML format.
---

# Write Struct Offset as YAML

Persist a single struct member offset analysis result to a YAML file beside the binary using IDA Pro MCP.

## Prerequisites

Before using this skill, you should have:
1. Identified the struct name and member name
2. Determined the member offset (and optionally size)
3. Generated a unique signature using `/generate-signature-for-structoffset`

## Required Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `struct_name` | Name of the struct/class | `CBaseEntity` |
| `member_name` | Name of the struct member | `m_skeletonInstance` |
| `offset` | Hex offset of the member from struct start | `0x278` |

## Optional Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `size` | Size of the member in bytes (use `None` to omit) | `8` |
| `offset_sig` | Unique byte signature locating an instruction that contains the offset (use `None` to omit) | `8B 93 E0 04 00 00` |
| `offset_sig_disp` | Byte displacement from signature start to the target instruction. `0` or `None` means signature starts at the target instruction. Non-zero means backward expansion was used by `/generate-signature-for-structoffset`. (use `None` to omit) | `8` |

## Method

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi
import os
import yaml

# === REQUIRED: Replace these values ===
struct_name = "<struct_name>"           # e.g., "CBaseEntity"
member_name = "<member_name>"           # e.g., "m_skeletonInstance"
offset = <offset>                       # e.g., 0x278
# ======================================

# === OPTIONAL: Set to None to omit from output ===
size = <size>                           # e.g., 8 or None
offset_sig = <offset_sig>              # e.g., "8B 93 E0 04 00 00" or None
offset_sig_disp = <offset_sig_disp>    # e.g., 8 or None (0 also omitted)
# =================================================

# Get binary path and determine platform
input_file = idaapi.get_input_file_path()
dir_path = os.path.dirname(input_file)

if input_file.endswith('.dll'):
    platform = 'windows'
else:
    platform = 'linux'

# Build data dictionary conditionally
data = {}

data['struct_name'] = struct_name
data['member_name'] = member_name
data['offset'] = hex(offset)

if size is not None and size > 0:
    data['size'] = size

if offset_sig is not None:
    data['offset_sig'] = offset_sig

if offset_sig_disp is not None and offset_sig_disp > 0:
    data['offset_sig_disp'] = offset_sig_disp

yaml_path = os.path.join(dir_path, f"{struct_name}_{member_name}.{platform}.yaml")
with open(yaml_path, 'w', encoding='utf-8') as f:
    yaml.dump(data, f, default_flow_style=False, sort_keys=False, allow_unicode=True)
print(f"Written to: {yaml_path}")
"""
```

## Output File Naming Convention

The output YAML filename follows this pattern:
- `<struct_name>_<member_name>.<platform>.yaml`

Examples:
- `server.dll` → `CBaseEntity_m_skeletonInstance.windows.yaml`
- `libserver.so` / `libserver.so` → `CBaseEntity_m_skeletonInstance.linux.yaml`

## Output YAML Format

Full output (with `size`, `offset_sig`, and `offset_sig_disp` provided):
```yaml
struct_name: CBaseEntity
member_name: m_skeletonInstance
offset: 0x278
size: 8
offset_sig: FF 50 ?? 48 85 C0 74 ?? 48 8B 80 A0 03 00 00 48 83 C4 28 C3
offset_sig_disp: 8
```

Output without backward expansion (`offset_sig_disp` is 0 or omitted):
```yaml
struct_name: CBaseEntity
member_name: m_skeletonInstance
offset: 0x278
size: 8
offset_sig: 8B 93 78 02 00 00
```

Minimal output (with `size=None`, `offset_sig=None`):
```yaml
struct_name: CBaseEntity
member_name: m_skeletonInstance
offset: 0x278
```

Each field:
- `struct_name` - Name of the struct/class
- `member_name` - Name of the struct member
- `offset` - Hex offset from struct start
- `size` (optional) - Size in bytes
- `offset_sig` (optional) - Unique byte signature of an instruction containing the offset (e.g., `8B 93 E0 04 00 00` for `mov edx, [rbx+4E0h]`)
- `offset_sig_disp` (optional) - Byte displacement from signature start to the target instruction. Only present when non-zero (backward expansion was used). Runtime: scan for `offset_sig`, then add `offset_sig_disp` to get the target instruction address.

## Platform Detection

The skill automatically detects the platform based on file extension:
- `.dll` → Windows
- `.so` → Linux

## Example Usage

### With all parameters

```python
struct_name = "CBaseEntity"
member_name = "m_skeletonInstance"
offset = 0x278
size = 8
offset_sig = "8B 93 78 02 00 00"
offset_sig_disp = None
```

### With backward-expanded signature

```python
struct_name = "CSkeletonInstance"
member_name = "m_animationController"
offset = 0x3A0
size = 8
offset_sig = "FF 50 40 48 85 C0 74 0C 48 8B 80 A0 03 00 00 48 83 C4 28 C3"
offset_sig_disp = 8
```

### Without optional parameters

```python
struct_name = "CBaseEntity"
member_name = "m_skeletonInstance"
offset = 0x278
size = None
offset_sig = None
offset_sig_disp = None
```

### With only size

```python
struct_name = "CBaseEntity"
member_name = "m_iHealth"
offset = 0x408
size = 4
offset_sig = None
```

### With only signature

```python
struct_name = "CBaseEntity"
member_name = "m_nActualMoveType"
offset = 0x4E0
size = None
offset_sig = "8B 93 E0 04 00 00"
```

## Notes

- All offsets are written in hexadecimal format with lowercase `0x` prefix
- The YAML file is written to the same directory as the input binary
- When `size` is `None` or `0`, the `size` field is omitted from the output entirely
- When `offset_sig` is `None`, the `offset_sig` field is omitted from the output entirely
- When `offset_sig_disp` is `None` or `0`, the `offset_sig_disp` field is omitted from the output entirely (signature starts at the target instruction)
- `offset_sig` should be a signature generated by `/generate-signature-for-structoffset`
- `offset_sig_disp` is the byte displacement from signature start to the target instruction, only needed when backward expansion was used
