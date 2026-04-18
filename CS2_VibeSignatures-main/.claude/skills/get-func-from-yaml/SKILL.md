---
name: get-func-from-yaml
description: Load function information from a pre-generated YAML file. Use this skill when you need function address, size, signature, and optional vtable metadata before downstream analysis. This skill checks for existing function YAML files and errors out if not found.
---

# Get Function from YAML

Load function information from a pre-generated `{func_name}.{platform}.yaml` file beside the binary.

## Parameters

- `func_name`: The function name to look up (e.g., `CBaseModelEntity_SetModel`, `CCSPlayerController_ChangeTeam`, `CEntityInstance_AcceptInput`)

## Method

### 1. Check and Load Function YAML

Run the following code with the appropriate `func_name`:

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi
import os

func_name = "<FUNC_NAME>"  # Replace with actual function name

input_file = idaapi.get_input_file_path()
dir_path = os.path.dirname(input_file)
platform = 'windows' if input_file.endswith('.dll') else 'linux'

yaml_path = os.path.join(dir_path, f"{func_name}.{platform}.yaml")

if os.path.exists(yaml_path):
    with open(yaml_path, 'r', encoding='utf-8') as f:
        print(f.read())
    print("FUNC_YAML_EXISTS: True")
else:
    print(f"ERROR: Required file {func_name}.{platform}.yaml not found.")
"""
```

### 2. Handle Result

**If YAML exists** (`FUNC_YAML_EXISTS: True`), extract available values from the output:
- `func_name`: Function name (if present)
- `func_va`: Function virtual address (if present)
- `func_rva`: Function relative virtual address (if present)
- `func_size`: Function size in bytes (if present)
- `func_sig`: Function signature bytes (if present)
- `vtable_name`: Related vtable class name (optional, virtual-function YAML only)
- `vfunc_offset`: Offset from vtable start (optional, virtual-function YAML only)
- `vfunc_index`: Index in vtable (optional, virtual-function YAML only)

Example YAML content:
```yaml
func_name: CCSPlayerController_Respawn
func_va: 0x180A8CA10
func_rva: 0xA8CA10
func_size: 0x3F
func_sig: 41 B8 80 00 00 00 48 8D 99 10 05 00 00
vtable_name: CCSPlayerController
vfunc_offset: 0x330
vfunc_index: 102
```

If some fields are missing, use only the fields that exist in YAML and **do not fabricate values**.

**If YAML does NOT exist**, **ERROR OUT** and report to user:
```
ERROR: Required file {func_name}.{platform}.yaml not found.
Please run `/write-func-as-yaml` with func_name={func_name} first.
For virtual functions, `/write-vfunc-as-yaml` is also acceptable.
```
Do NOT proceed with any remaining steps in the calling skill.

## Usage in Other Skills

When a skill needs function information, use this skill first:

```markdown
### 1. Get {FuncName} Function Info

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name={FuncName}`.

If the skill returns an error, stop and report to user.
Otherwise, extract `func_va`, `func_sig`, and other available fields for subsequent steps.
```

## Example Output

| Field | Description | Example |
|-------|-------------|---------|
| `func_name` | Function name | `CCSPlayerController_Respawn` |
| `func_va` | Virtual address of function (optional in some virtual-function YAML) | `0x180A8CA10` |
| `func_rva` | Relative virtual address (optional) | `0xA8CA10` |
| `func_size` | Function size in bytes (optional) | `0x3F` |
| `func_sig` | Byte signature (optional) | `41 B8 80 00 00 00 ...` |
| `vtable_name` | Related class name for virtual function (optional) | `CCSPlayerController` |
| `vfunc_offset` | Offset from vtable start (optional) | `0x330` |
| `vfunc_index` | Index in vtable (optional) | `102` |
