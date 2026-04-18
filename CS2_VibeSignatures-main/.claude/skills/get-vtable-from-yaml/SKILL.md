---
name: get-vtable-from-yaml
description: Load vtable information from a pre-generated YAML file. Use this skill when you need to get vtable address and size for a class before analyzing virtual functions. This skill checks for existing vtable YAML files and errors out if not found, ensuring the vtable analysis has been done first.
---

# Get VTable from YAML

Load vtable information from a pre-generated `{class_name}_vtable.{platform}.yaml` file beside the binary.

## Parameters

- `class_name`: The class name to look up (e.g., `CCSPlayerController`, `CCSPlayer_WeaponServices`, `CServerSideClient`)

## Method

### 1. Check and Load VTable YAML

Run the following code with the appropriate `class_name`:

```python
mcp__ida-pro-mcp__py_eval code="""
import idaapi
import os

class_name = "<CLASS_NAME>"  # Replace with actual class name

input_file = idaapi.get_input_file_path()
dir_path = os.path.dirname(input_file)
platform = 'windows' if input_file.endswith('.dll') else 'linux'

yaml_path = os.path.join(dir_path, f"{class_name}_vtable.{platform}.yaml")

if os.path.exists(yaml_path):
    with open(yaml_path, 'r', encoding='utf-8') as f:
        print(f.read())
    print(f"YAML_EXISTS: True")
else:
    print(f"ERROR: Required file {class_name}_vtable.{platform}.yaml not found.")
"""
```

### 2. Handle Result

**If YAML exists** (`YAML_EXISTS: True`), extract these values from the output:
- `vtable_va`: The vtable virtual address (use as `<VTABLE_START>`)
- `vtable_rva`: The vtable relative virtual address
- `vtable_size`: The vtable size in bytes
- `vtable_numvfunc`: The valid vtable entry count (last valid index = count - 1)
- `vtable_entries`: An array of virtual functions starting from vtable[0].

Example YAML content:
```yaml
vtable_class: CCSPlayerController
vtable_symbol: _ZTV19CCSPlayerController
vtable_va: 0x221fc80
vtable_rva: 0x221fc80
vtable_size: 0xd60
vtable_numvfunc: 428
vtable_entries:
  - 0x9b4bb0
  - 0x9b4bc0
  - 0x9b4bd0
```

**If YAML does NOT exist**, **ERROR OUT** and report to user:
```
ERROR: Required file {class_name}_vtable.{platform}.yaml not found.
Please run `/write-vtable-as-yaml` with class_name={class_name} first to generate the vtable YAML file.
```
Do NOT proceed with any remaining steps in the calling skill.

## Usage in Other Skills

When a skill needs vtable information, use this skill first:

```markdown
### 1. Get {ClassName} VTable Address

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name={ClassName}`.

If the skill returns an error, stop and report to user.
Otherwise, extract `vtable_va`, `vtable_numvfunc` and `vtable_entries` for subsequent steps.
```

## Expected Output Values

| Field | Description | Example |
|-------|-------------|---------|
| `vtable_class` | Class name | `CCSPlayerController` |
| `vtable_va` | Virtual address of vtable | `0x2114cd0` |
| `vtable_rva` | Relative virtual address | `0x2114cd0` |
| `vtable_size` | Size in bytes | `0xd60` |
| `vtable_numvfunc` | Number of virtual functions | `428` |
| `vtable_entries` | An array of virtual functions starting from vtable[0] | ... |
