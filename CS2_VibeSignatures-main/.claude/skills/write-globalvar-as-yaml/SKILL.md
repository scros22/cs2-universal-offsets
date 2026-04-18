---
name: write-globalvar-as-yaml
description: Write global variable analysis results as YAML file beside the binary using IDA Pro MCP. Use this skill after completing global variable identification and signature generation to persist the results in a standardized YAML format.
---

# Write Global Variable IDA Analysis Output as YAML

Persist global variable analysis results to a YAML file beside the binary using IDA Pro MCP.

## Prerequisites

Before using this skill, you should have:
1. Identified and renamed the target global variable
2. Generated a unique signature using `/generate-signature-for-globalvar`

## Required Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `gv_name` | Name of the global variable | `IGameSystem_InitAllSystems_pFirst` |
| `gv_addr` | Virtual address of the global variable | `0x180XXXXXX` |
| `gv_sig` | Unique byte signature (must start at the GV-referencing instruction) | `48 8B 1D ?? ?? ?? ?? 48 85 DB 0F 84 ?? ?? ?? ?? BD FF FF 00 00` |
| `gv_inst_length` | Length of the instruction in bytes | `7` |
| `gv_inst_disp` | Displacement offset within the instruction | `3` |
| `gv_sig_va` | Virtual address where the signature matches | `0x180XXXXXX` |

**Note:** `gv_inst_offset` is always 0 - the signature MUST start at the instruction that references the global variable.

## Method

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi
import os
import yaml

# === REQUIRED: Replace these values ===
gv_name = "<gv_name>"               # e.g., "IGameSystem_InitAllSystems_pFirst"
gv_addr = <gv_addr>                 # e.g., 0x180XXXXXX
gv_sig = "<gv_sig>"                 # e.g., "48 8B 1D ?? ?? ?? ?? 48 85 DB 0F 84 ?? ?? ?? ?? BD FF FF 00 00"
gv_sig_va = <gv_sig_va>             # e.g., 0x180XXXXXX (virtual address where signature matches)
gv_inst_length = <gv_inst_length>   # e.g., 7 (instruction length in bytes)
gv_inst_disp = <gv_inst_disp>       # e.g., 3 (displacement offset within instruction)
# ======================================

# Fixed value - signature must start at the GV-referencing instruction
gv_inst_offset = 0

# Get binary path and determine platform
input_file = idaapi.get_input_file_path()
dir_path = os.path.dirname(input_file)

if input_file.endswith('.dll'):
    platform = 'windows'
    image_base = idaapi.get_imagebase()
else:
    platform = 'linux'
    image_base = 0x0

gv_rva = gv_addr - image_base

data = {
    'gv_name': gv_name,
    'gv_va': hex(gv_addr),
    'gv_rva': hex(gv_rva),
    'gv_sig': gv_sig,
    'gv_sig_va': hex(gv_sig_va),
    'gv_inst_offset': gv_inst_offset,
    'gv_inst_length': gv_inst_length,
    'gv_inst_disp': gv_inst_disp,
}

yaml_path = os.path.join(dir_path, f"{gv_name}.{platform}.yaml")
with open(yaml_path, 'w', encoding='utf-8') as f:
    yaml.dump(data, f, default_flow_style=False, sort_keys=False)
print(f"Written to: {yaml_path}")
"""
```

## Output File Naming Convention

The output YAML filename follows this pattern:
- `<gv_name>.<platform>.yaml`

Examples:
- `server.dll` → `IGameSystem_InitAllSystems_pFirst.windows.yaml`
- `libserver.so` / `libserver.so` → `IGameSystem_InitAllSystems_pFirst.linux.yaml`

## Output YAML Format

```yaml
gv_name: IGameSystem_InitAllSystems_pFirst
gv_va: 0x180XXXXXX   # Global variable's virtual address - changes with game updates
gv_rva: 0xXXXXXX     # Relative virtual address (VA - image base) - changes with game updates
gv_sig: 41 B8 80 00 00 00 48 8D 99 10 05 00 00  # Unique byte signature (starts at GV-referencing instruction)
gv_sig_va: 0x180XXXXXX     # The virtual address that signature matches
gv_inst_offset: 0          # Always 0 - signature must start at the GV-referencing instruction
gv_inst_length: 7          # 48 8B 1D XX XX XX XX = 7 bytes
gv_inst_disp:   3          # Displacement offset start at position 3 (after 48 8B 1D)
```

## Platform Detection

The skill automatically detects the platform based on file extension:
- `.dll` → Windows (uses `idaapi.get_imagebase()` for image base)
- `.so` → Linux (uses `0x0` as image base)

## Notes

- All values marked "changes with game updates" should be regenerated when analyzing new binary versions
- The YAML file is written to the same directory as the input binary
- gv_rva is automatically calculated as `gv_va - image_base`
