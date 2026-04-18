---
name: create-preprocessor-scripts
description: |
  Create a new find-XXXX preprocessor Python script from scratch (no existing SKILL.md),
  add config.yaml skill and symbol entries. Covers xref-string-based and LLM_DECOMPILE-based
  discovery patterns. Use when a GitHub issue or user instruction specifies a new function to find.
disable-model-invocation: true
---

# Create Preprocessor Scripts from Scratch

Create an `ida_preprocessor_scripts/find-XXXX.py` preprocessor script and add the corresponding
`config.yaml` entries for a newly requested function, vtable, or struct member offset.

## When to Use

- A GitHub issue or user instruction requests adding support for finding a new function/symbol
- No existing `.claude/skills/find-XXXX/SKILL.md` needs conversion (for that, use `convert-finder-skill-to-preprocessor-scripts`)

## Inputs

The user or issue will provide some or all of:

| Field | Description | Example |
|-------|-------------|---------|
| **Function name(s)** | Target symbol(s) to find | `CPlayer_MovementServices_PlayWaterStepSound` |
| **Module** | Which DLL/SO the function lives in | `server`, `engine`, `networksystem`, `client` |
| **Category** | Symbol type | `func`, `vfunc`, `structmember`, `patch`, `vtable` |
| **xref_strings** | Debug strings for xref-based discovery | `"CT_Water.StepLeft"` |
| **Predecessor function** | Function to decompile for LLM_DECOMPILE patterns | `CBaseEntity_TakeDamageOld` |
| **VTable class** | Class owning the vtable (for vfuncs) | `CBasePlayerPawn` |
| **Desired YAML fields** | Which fields the output YAML needs | `func_name, func_sig, func_va, func_rva, func_size` |
| **Dependencies** | Input YAMLs this skill depends on | `CCSPlayer_MovementServices_vtable.{platform}.yaml` |
| **Aliases** | Alternative names for the symbol | `CPlayer_MovementServices::PlayWaterStepSound` |

## Overview

Six preprocessor patterns exist. The discovery method and target type determine which to use:

| Pattern | Discovery Method | Has FUNC_XREFS | Has LLM_DECOMPILE | Has INHERIT_VFUNCS | Has FUNC_VTABLE_RELATIONS | preprocess_skill has llm_config |
|---------|-----------------|-----------------|---------------------|--------------------|---------------------------|-------------------------------|
| **A** -- Regular function via xref strings | `find_regex` + `xrefs_to` on debug strings | Yes | No | No | No | No |
| **B** -- Virtual function via xref strings | Same as A, but function is in a vtable | Yes | No | No | Yes | No |
| **C** -- Virtual function via LLM_DECOMPILE | Decompile a known predecessor function, identify vfunc call offsets | No | Yes | No | Yes | Yes |
| **D** -- Regular function via LLM_DECOMPILE | Decompile a known predecessor function, identify direct call targets | No | Yes | No | No | Yes |
| **E** -- Struct member offset via LLM_DECOMPILE | Decompile a known predecessor function, identify struct field access offsets | No | Yes | No | No | Yes |
| **F** -- Virtual function via INHERIT_VFUNCS | Inherit vtable slot index from a known base-class vfunc, look up same slot in derived-class vtable | No | No | Yes | No | No |

Additionally, **struct member offsets** can be mixed into any pattern as a secondary target (see "Struct Member Mixin" section below).

---

## Step 1: Determine the Pattern

From the user's input, determine:

1. **Is the target a function, vfunc, or struct member offset?**
   - Has `xref_strings` + category `func` -> **Pattern A**
   - Has `xref_strings` + category `vfunc` -> **Pattern B**
   - Has predecessor function + category `vfunc` -> **Pattern C**
   - Has predecessor function + category `func` -> **Pattern D**
   - Has predecessor function + category `structmember` -> **Pattern E**
   - Has base vfunc name + category `vfunc` (derived-class override of known base vfunc) -> **Pattern F**

2. **Do xref strings differ between Windows and Linux?** If yes, use platform-specific `FUNC_XREFS_WINDOWS` / `FUNC_XREFS_LINUX` variant.

3. **Are there multiple functions?** If they share the same discovery method and starting point, put them in the same script with `-AND-` in the name. Otherwise, split into separate scripts.

---

## Step 2: Create the Preprocessor Script

Script location: `ida_preprocessor_scripts/find-{skill_name}.py`

The filename MUST match the `name` field in `config.yaml` skill entry.

### Pattern A -- Regular function via xref strings

Use when: function is non-virtual, discovered via debug string cross-references.

```python
#!/usr/bin/env python3
"""Preprocess script for find-{SKILL_NAME} skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "{FUNC_NAME}",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "{FUNC_NAME}",
        [
            "{XREF_STRING_1}",  # Debug string from user input
        ],
        [],   # xref_signatures_list -- byte patterns if needed, usually empty
        [],   # xref_funcs_list -- known caller function names if needed
        [],   # exclude_funcs_list -- function names to exclude from results
        [],   # exclude_strings_list -- strings to exclude
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "{FUNC_NAME}",
        [
            "func_name",
            "func_sig",
            "func_va",
            "func_rva",
            "func_size",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
):
    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        func_xrefs=FUNC_XREFS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

### Pattern B -- Virtual function via xref strings

Same as Pattern A, but adds `FUNC_VTABLE_RELATIONS` and vtable fields to `GENERATE_YAML_DESIRED_FIELDS`:

```python
#!/usr/bin/env python3
"""Preprocess script for find-{SKILL_NAME} skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "{FUNC_NAME}",
]

FUNC_XREFS = [
    # (func_name, xref_strings_list, xref_signatures_list, xref_funcs_list, exclude_funcs_list, exclude_strings_list)
    (
        "{FUNC_NAME}",
        [
            "{XREF_STRING_1}",
        ],
        [],
        [],
        [],
        [],
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("{FUNC_NAME}", "{VTABLE_CLASS}"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "{FUNC_NAME}",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
            "vtable_name",
            "vfunc_offset",
            "vfunc_index",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, debug=False,
):
    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        func_xrefs=FUNC_XREFS,
        func_vtable_relations=FUNC_VTABLE_RELATIONS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

### Platform-Specific Xref Strings (Patterns A & B variant)

When xref strings differ between Windows and Linux, split into two variables:

```python
FUNC_XREFS_WINDOWS = [
    (
        "{FUNC_NAME}",
        [
            "CSource2GameEntities::CheckTransmit",  # Full assertion string on Windows
        ],
        [], [], [], [],
    ),
]

FUNC_XREFS_LINUX = [
    (
        "{FUNC_NAME}",
        [
            "./gameinterface.cpp:30",  # Shorter path-based string on Linux
        ],
        [], [], [], [],
    ),
]
```

Then in `preprocess_skill`, use a ternary to select the right one:

```python
        func_xrefs=FUNC_XREFS_WINDOWS if platform == "windows" else FUNC_XREFS_LINUX,
```

### Pattern C -- Virtual function via LLM_DECOMPILE

Use when: function IS virtual (has vtable slot), discovered by decompiling a known predecessor function.

```python
#!/usr/bin/env python3
"""Preprocess script for find-{SKILL_NAME} skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "{FUNC_NAME_1}",
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    (
        "{FUNC_NAME_1}",
        "prompt/call_llm_decompile.md",
        "references/{MODULE}/{PREDECESSOR_FUNC}.{platform}.yaml",
    ),
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("{FUNC_NAME_1}", "{VTABLE_CLASS}"),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "{FUNC_NAME_1}",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, llm_config=None, debug=False,
):

    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        func_vtable_relations=FUNC_VTABLE_RELATIONS,
        llm_decompile_specs=LLM_DECOMPILE,
        llm_config=llm_config,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

### Pattern D -- Regular function via LLM_DECOMPILE

Use when: function is NOT virtual, discovered by decompiling a known predecessor function.

```python
#!/usr/bin/env python3
"""Preprocess script for find-{SKILL_NAME} skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "{FUNC_NAME}",
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    (
        "{FUNC_NAME}",
        "prompt/call_llm_decompile.md",
        "references/{MODULE}/{PREDECESSOR_FUNC}.{platform}.yaml",
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "{FUNC_NAME}",
        [
            "func_name",
            "func_sig",
            "func_va",
            "func_rva",
            "func_size",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, llm_config=None, debug=False,
):
    """Reuse previous gamever func_sig to locate target function(s) and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        func_names=TARGET_FUNCTION_NAMES,
        llm_decompile_specs=LLM_DECOMPILE,
        llm_config=llm_config,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

### Pattern E -- Struct member offset via LLM_DECOMPILE

Use when: target is a **struct member offset** (not a function), discovered by decompiling a known predecessor function.

```python
#!/usr/bin/env python3
"""Preprocess script for find-{SKILL_NAME} skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_STRUCT_MEMBER_NAMES = [
    "{STRUCT_MEMBER_NAME}",  # e.g. "CCheckTransmitInfo_m_nPlayerSlot"
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    (
        "{STRUCT_MEMBER_NAME}",
        "prompt/call_llm_decompile.md",
        "references/{MODULE}/{PREDECESSOR_FUNC}.{platform}.yaml",
    ),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "{STRUCT_MEMBER_NAME}",
        [
            "struct_name",
            "member_name",
            "offset",
            "size",
            "offset_sig",
            "offset_sig_disp",
        ],
    ),
]

async def preprocess_skill(
    session, skill_name, expected_outputs, old_yaml_map,
    new_binary_dir, platform, image_base, llm_config=None, debug=False,
):
    """Reuse previous gamever offset_sig to locate target struct offset and write YAML."""
    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        struct_member_names=TARGET_STRUCT_MEMBER_NAMES,
        llm_decompile_specs=LLM_DECOMPILE,
        llm_config=llm_config,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

**Key differences from Pattern D:**
- Uses `TARGET_STRUCT_MEMBER_NAMES` instead of `TARGET_FUNCTION_NAMES`
- Passes `struct_member_names=` instead of `func_names=` to `preprocess_common_skill`
- YAML fields are struct-specific: `struct_name, member_name, offset, size, offset_sig, offset_sig_disp`
- No `FUNC_VTABLE_RELATIONS`
- config.yaml symbol category is `structmember` (not `func` or `vfunc`)

### Pattern F -- Virtual function via INHERIT_VFUNCS

Use when: the target is a **derived-class override** of a known base-class virtual function. The base vfunc has already been found (by another script), and this script inherits its vtable slot index to look up the same slot in the derived class's vtable.

This is the simplest pattern -- no xref strings, no LLM decompilation needed. Just a vtable slot lookup.

```python
#!/usr/bin/env python3
"""Preprocess script for find-{SKILL_NAME} skill."""

from ida_analyze_util import preprocess_common_skill

INHERIT_VFUNCS = [
    # (target_func_name, inherit_vtable_class, base_vfunc_name, generate_func_sig)
    ("{DERIVED_FUNC_NAME}", "{DERIVED_VTABLE_CLASS}", "{BASE_VFUNC_NAME}", True),
]

GENERATE_YAML_DESIRED_FIELDS = [
    # (symbol_name, generate_yaml_fields)
    (
        "{DERIVED_FUNC_NAME}",
        [
            "func_name",
            "func_va",
            "func_rva",
            "func_size",
            "func_sig",
            "vtable_name",
            "vfunc_offset",
            "vfunc_index",
        ],
    ),
]

async def preprocess_skill(
    session,
    skill_name,
    expected_outputs,
    old_yaml_map,
    new_binary_dir,
    platform,
    image_base,
    debug=False,
):
    """Reuse old func_sig first; fallback to vtable index + generated signature when needed."""
    _ = skill_name

    return await preprocess_common_skill(
        session=session,
        expected_outputs=expected_outputs,
        old_yaml_map=old_yaml_map,
        new_binary_dir=new_binary_dir,
        platform=platform,
        image_base=image_base,
        inherit_vfuncs=INHERIT_VFUNCS,
        generate_yaml_desired_fields=GENERATE_YAML_DESIRED_FIELDS,
        debug=debug,
    )
```

**INHERIT_VFUNCS tuple fields:**
- `target_func_name` -- name for the derived-class function (e.g. `"CBaseEntity_Precache"`)
- `inherit_vtable_class` -- class whose vtable to look up (e.g. `"CBaseEntity"`)
- `base_vfunc_name` -- YAML artifact stem of the base-class vfunc that defines the slot index (e.g. `"CEntityInstance_Precache"`). Can be cross-module: `"../engine/INetworkMessages_FindNetworkGroup"`
- `generate_func_sig` -- (optional, default True) whether to generate a func_sig if no old YAML exists

**Key differences from other patterns:**
- No `TARGET_FUNCTION_NAMES`, `FUNC_XREFS`, `LLM_DECOMPILE`, or `FUNC_VTABLE_RELATIONS`
- Uses `inherit_vfuncs=` parameter instead of `func_names=`
- No `llm_config` parameter in `preprocess_skill`
- config.yaml `expected_input` must include both the base vfunc YAML and the derived class vtable YAML
- config.yaml symbol category is `vfunc`

### FULLMATCH: Prefix for Xref Strings (Patterns A & B)

When the xref string is short or generic (e.g. `"Precache"`, `"userid"`, `"team"`), use the `FULLMATCH:` prefix to require **exact string matching** instead of substring matching. Without it, `"Precache"` would match `"PrecacheModel"`, `"PrecacheSound"`, etc.

```python
FUNC_XREFS = [
    (
        "CEntityInstance_Precache",
        [
            "FULLMATCH:Precache",  # Only matches the exact string "Precache"
        ],
        [], [], [], [],
    ),
]
```

### Struct Member Mixin (for any pattern)

Struct member offsets can also be **mixed into** a function-finding script when they are discovered from the same function via signature matching (not LLM_DECOMPILE). Add `TARGET_STRUCT_MEMBER_NAMES` alongside `TARGET_FUNCTION_NAMES` and pass `struct_member_names=` to `preprocess_common_skill`:

```python
TARGET_FUNCTION_NAMES = [
    "SomeFunction",
]

TARGET_STRUCT_MEMBER_NAMES = [
    "SomeStruct_m_someField",
]

GENERATE_YAML_DESIRED_FIELDS = [
    ("SomeFunction", ["func_name", "func_sig", "func_va", "func_rva", "func_size"]),
    ("SomeStruct_m_someField", ["struct_name", "member_name", "offset", "size", "offset_sig", "offset_sig_disp"]),
]

# In preprocess_skill:
    return await preprocess_common_skill(
        ...
        func_names=TARGET_FUNCTION_NAMES,
        struct_member_names=TARGET_STRUCT_MEMBER_NAMES,
        ...
    )
```

### Key Differences Between Patterns

| Aspect | Pattern A (func + xref) | Pattern B (vfunc + xref) | Pattern C (vfunc + LLM) | Pattern D (func + LLM) | Pattern E (structmember + LLM) | Pattern F (vfunc + inherit) |
|--------|------------------------|--------------------------|------------------------|------------------------|-------------------------------|---------------------------|
| FUNC_XREFS | Yes | Yes | No | No | No | No |
| FUNC_VTABLE_RELATIONS | No | Yes | Yes | No | No | No |
| INHERIT_VFUNCS | No | No | No | No | No | Yes |
| LLM_DECOMPILE | No | No | Yes | Yes | Yes | No |
| `llm_config` param | No | No | Yes | Yes | Yes | No |
| Target list | `TARGET_FUNCTION_NAMES` | `TARGET_FUNCTION_NAMES` | `TARGET_FUNCTION_NAMES` | `TARGET_FUNCTION_NAMES` | `TARGET_STRUCT_MEMBER_NAMES` | (none -- defined in INHERIT_VFUNCS) |
| preprocess param | `func_names=` | `func_names=` | `func_names=` | `func_names=` | `struct_member_names=` | `inherit_vfuncs=` |
| YAML fields | func_name, func_sig, func_va, func_rva, func_size | Same + vtable_name, vfunc_offset, vfunc_index | func_name, vfunc_sig, vfunc_offset, vfunc_index, vtable_name | func_name, func_sig, func_va, func_rva, func_size | struct_name, member_name, offset, size, offset_sig, offset_sig_disp | func_name, func_va, func_rva, func_size, func_sig, vtable_name, vfunc_offset, vfunc_index |
| config category | `func` | `vfunc` | `vfunc` | `func` | `structmember` | `vfunc` |

---

## Step 3: Update config.yaml

### 3a. Skills Section

Each preprocessor script needs a corresponding skill entry under the appropriate module's `skills:` list.

Find the module section (e.g. `server`, `engine`, `networksystem`) and add entries in logical order (near related functions).

**Template:**

```yaml
      - name: find-{SKILL_NAME}
        expected_output:
          - {FUNC_NAME_1}.{platform}.yaml
          # - {FUNC_NAME_2}.{platform}.yaml  # One per target function
        # expected_input only if the skill depends on other YAMLs:
        expected_input:
          - {PREDECESSOR_FUNC}.{platform}.yaml    # For Patterns C & D: the reference function
          - {VTABLE_CLASS}_vtable.{platform}.yaml  # For Patterns B & C: the vtable
```

**Rules:**
- `expected_output`: One `.{platform}.yaml` per target function in the script
- `expected_input`: Include predecessor function YAML (Patterns C & D) and/or vtable YAML (Patterns B & C & F)
- Pattern A with no vtable: typically NO `expected_input`
- Pattern F: needs both the derived class vtable YAML and the base vfunc YAML in `expected_input`
- Multi-function scripts use `-AND-` in the name: `find-FuncA-AND-FuncB`
- Place the new entry near related functions (e.g. `CCSPlayer_MovementServices_*` entries together)

**Dependency chain example** (multi-script):

```yaml
      # Pattern A: found via xref string, no dependencies
      - name: find-FuncA
        expected_output:
          - FuncA.{platform}.yaml

      # Pattern C: found by decompiling FuncA, needs FuncA + vtable
      - name: find-FuncB
        expected_output:
          - FuncB.{platform}.yaml
        expected_input:
          - FuncA.{platform}.yaml
          - SomeClass_vtable.{platform}.yaml
```

### 3b. Symbols Section

For each target function, add a symbol entry under the same module's `symbols:` list (if not already present).

```yaml
      # Regular function (Pattern A)
      - name: {FUNC_NAME}
        category: func
        alias:
          - {ClassName}::{MethodName}   # e.g. CPlayer_MovementServices::PlayWaterStepSound

      # Virtual function (Patterns B & C)
      - name: {FUNC_NAME}
        category: vfunc
        alias:
          - {ClassName}::{MethodName}   # e.g. CBasePlayerPawn::OnTakeDamage

      # Struct member offset (Pattern E)
      - name: {STRUCT_MEMBER_NAME}
        category: structmember
        struct: {STRUCT_NAME}
        member: {MEMBER_NAME}
        alias:
          - {StructName}::{MemberName}
```

**Check existing symbols before adding -- do NOT create duplicates.**

Place the new symbol near related symbols (same class/subsystem).

---

## Step 4: Handle Reference YAMLs (Patterns C, D & E only)

Pattern C, D, and E scripts reference a predecessor function's YAML at:
`ida_preprocessor_scripts/references/{module}/{PREDECESSOR_FUNC}.{platform}.yaml`

**Check** if the reference YAML already exists:
- `ida_preprocessor_scripts/references/{module}/{PREDECESSOR_FUNC}.linux.yaml`
- `ida_preprocessor_scripts/references/{module}/{PREDECESSOR_FUNC}.windows.yaml`

If NOT present, generate them using `generate_reference_yaml.py`:

```bash
# Windows
uv run generate_reference_yaml.py -func_name {PREDECESSOR_FUNC} -auto_start_mcp -binary "bin/{gamever}/{module}/{binary_name}.dll" -debug

# Linux
uv run generate_reference_yaml.py -func_name {PREDECESSOR_FUNC} -auto_start_mcp -binary "bin/{gamever}/{module}/lib{module}.so" -debug
```

where `{gamever}` can be obtained from `.env` -> `CS2VIBE_GAMEVER`.

YOU MUST: rename known symbols / add necessary comments in the generated reference YAMLs so the LLM can find desired symbols by comparing reference ones with raw procedure/disassembly read from new binaries. See the `convert-finder-skill-to-preprocessor-scripts` SKILL.md Step 5 for detailed annotation examples.

**IMPORTANT — When the predecessor is a NEW function (no existing output YAMLs):** If the predecessor function is brand new (discovered by another new script you're creating at the same time), its output YAMLs don't exist yet and `generate_reference_yaml.py` cannot resolve its address. You must use a **multi-phase workflow**:

1. **Phase 1:** Create ALL scripts (vtable, xref_string, LLM_DECOMPILE) and update config.yaml
2. **Phase 2:** Run `uv run ida_analyze_bin.py -debug` — the vtable and xref_string scripts will succeed and populate the NEW predecessor's output YAMLs. The LLM_DECOMPILE script will fail (no reference YAML yet) or be skipped.
3. **Phase 3:** Now that the predecessor has output YAMLs, run `generate_reference_yaml.py` to create reference YAMLs, then annotate them.
4. **Phase 4:** Run `uv run ida_analyze_bin.py -debug` again — this time the LLM_DECOMPILE path runs and the full pipeline is validated.

---

## Step 5: Run Tests

After all creation steps are complete, run the full preprocessor test to validate the new script works.

Because the output is very long, redirect it to a temp file and then read just the summary:

```bash
uv run ida_analyze_bin.py -debug > /tmp/ida_test_output.txt 2>&1; tail -10 /tmp/ida_test_output.txt
```

Check the **Summary** at the end of the output:
- **Failed: 0** means the creation is correct
- If any failures, search the full output for the failing skill name to investigate:
  ```bash
  grep -A 5 "Failed\|Error" /tmp/ida_test_output.txt
  ```

This step is mandatory -- do not report completion without running and passing this validation.

---

## Step 6: Commit Changes

After validation passes, commit all changes to git:

```bash
git add ida_preprocessor_scripts/find-{SKILL_NAME}.py config.yaml
git commit -m "Add find-{SKILL_NAME} preprocessor script"
```

Include all files changed:
- The new preprocessor script
- config.yaml changes
- Any reference YAMLs generated (for Patterns C/D/E)

---

## Checklist

Before finishing, verify:

- [ ] Preprocessor script file name matches the `name` field in config.yaml skill entry
- [ ] `TARGET_FUNCTION_NAMES` lists all functions the script should find
- [ ] `FUNC_XREFS` xref strings match the user's specified debug strings (Pattern A/B)
- [ ] `LLM_DECOMPILE` reference path points to the correct predecessor function YAML (Patterns C/D/E)
- [ ] `FUNC_VTABLE_RELATIONS` lists correct vtable class for each virtual function (Patterns B/C only, NOT Patterns D/F)
- [ ] `INHERIT_VFUNCS` lists correct (target, derived_class, base_vfunc, gen_sig) tuples (Pattern F only)
- [ ] `GENERATE_YAML_DESIRED_FIELDS` uses correct field set for the pattern
- [ ] `preprocess_skill` signature includes `llm_config=None` if and only if LLM_DECOMPILE is used (NOT for Pattern F)
- [ ] `preprocess_common_skill` call passes all relevant lists (`func_xrefs`, `func_vtable_relations`, `llm_decompile_specs`, `llm_config`, `inherit_vfuncs`)
- [ ] config.yaml `expected_output` has one entry per target function
- [ ] config.yaml `expected_input` correctly chains dependencies
- [ ] config.yaml `symbols` section has entries for all target functions (no duplicates)
- [ ] Reference YAMLs exist or generated (Patterns C/D/E)
- [ ] `uv run ida_analyze_bin.py -debug` passes with 0 failures
- [ ] All changes committed to git

## Real-World Examples

### Example: Regular function via xref string (Pattern A)

**Issue says:** `CPlayer_MovementServices_PlayWaterStepSound` is a regular function in server dll. xref_strings: `"CT_Water.StepLeft"`. Fields needed: `func_name, func_sig, func_va, func_rva, func_size`.

**Result:** `ida_preprocessor_scripts/find-CPlayer_MovementServices_PlayWaterStepSound.py` with:
- `FUNC_XREFS` containing `"CT_Water.StepLeft"`
- `GENERATE_YAML_DESIRED_FIELDS` with `func_name, func_sig, func_va, func_rva, func_size`
- No `FUNC_VTABLE_RELATIONS`, no `LLM_DECOMPILE`
- config.yaml skill entry with `expected_output: CPlayer_MovementServices_PlayWaterStepSound.{platform}.yaml`
- config.yaml symbol entry with `category: func`, alias `CPlayer_MovementServices::PlayWaterStepSound`

### Example: Virtual function via xref string (Pattern B)

**Issue says:** `CSource2GameEntities_CheckTransmit` is a vfunc of `CSource2GameEntities` in server dll. xref_strings: `"CSource2GameEntities::CheckTransmit"` (Windows), `"./gameinterface.cpp:30"` (Linux).

**Result:** `ida_preprocessor_scripts/find-CSource2GameEntities_CheckTransmit.py` with:
- Platform-specific `FUNC_XREFS_WINDOWS` / `FUNC_XREFS_LINUX`
- `FUNC_VTABLE_RELATIONS`: `("CSource2GameEntities_CheckTransmit", "CSource2GameEntities")`
- `GENERATE_YAML_DESIRED_FIELDS` with vtable fields
- config.yaml `expected_input: CSource2GameEntities_vtable.{platform}.yaml`

### Example: Multiple functions from same xref (Pattern A, multi-target)

**Issue says:** Find both `FuncA` and `FuncB` in server. Both use xref string `"SharedDebugString"`.

**Result:** `ida_preprocessor_scripts/find-FuncA-AND-FuncB.py` with:
- Two entries in `TARGET_FUNCTION_NAMES`
- Two entries in `FUNC_XREFS` (each with the same or different xref strings)
- Two entries in `GENERATE_YAML_DESIRED_FIELDS`
- config.yaml skill name: `find-FuncA-AND-FuncB`
- config.yaml: two `expected_output` entries, two symbol entries

### Example: Derived-class vfunc via INHERIT_VFUNCS (Pattern F)

**Issue says:** `CBaseEntity_Precache` is a vfunc on `CBaseEntity` that overrides `CEntityInstance::Precache` at the same vtable slot. `CEntityInstance_Precache` is already found by another script.

**Result:** `ida_preprocessor_scripts/find-CBaseEntity_Precache.py` with:
- `INHERIT_VFUNCS`: `("CBaseEntity_Precache", "CBaseEntity", "CEntityInstance_Precache", True)`
- `GENERATE_YAML_DESIRED_FIELDS` with vtable fields
- No FUNC_XREFS, no LLM_DECOMPILE, no FUNC_VTABLE_RELATIONS
- config.yaml `expected_input`: `CBaseEntity_vtable.{platform}.yaml` + `CEntityInstance_Precache.{platform}.yaml`
- config.yaml symbol: category `vfunc`, alias `CBaseEntity::Precache`

### Example: Virtual function via xref string with FULLMATCH (Pattern B)

**Issue says:** `CEntityInstance_Precache` is a vfunc on `CEntityInstance`. xref_string: `"Precache"` (exact match needed since substring would hit `PrecacheModel`, etc.).

**Result:** `ida_preprocessor_scripts/find-CEntityInstance_Precache.py` with:
- `FUNC_XREFS` containing `"FULLMATCH:Precache"` (exact match)
- `FUNC_VTABLE_RELATIONS`: `("CEntityInstance_Precache", "CEntityInstance")`
- `GENERATE_YAML_DESIRED_FIELDS` with vtable fields
- config.yaml `expected_input`: `CEntityInstance_vtable.{platform}.yaml`

### Example: vtable + xref-string vfunc + LLM_DECOMPILE regular function (vtable + Patterns B + D, multi-phase)

**User says:** Find `LegacyGameEventListener` in server. It's a regular function called from `CSource2GameClients::StartHLTVServer` (a vfunc of `CSource2GameClients`). The xref string for StartHLTVServer is `"CSource2GameClients::StartHLTVServer: game event %s not found"`.

**Result — three scripts:**

1. `ida_preprocessor_scripts/find-CSource2GameClients_vtable.py` (vtable discovery):
   - `TARGET_CLASS_NAMES`: `["CSource2GameClients"]`
   - Pure vtable lookup, no dependencies

2. `ida_preprocessor_scripts/find-CSource2GameClients_StartHLTVServer.py` (Pattern B):
   - `FUNC_XREFS` containing `"CSource2GameClients::StartHLTVServer: game event %s not found"`
   - `FUNC_VTABLE_RELATIONS`: `("CSource2GameClients_StartHLTVServer", "CSource2GameClients")`
   - config.yaml `expected_input`: `CSource2GameClients_vtable.{platform}.yaml`

3. `ida_preprocessor_scripts/find-LegacyGameEventListener.py` (Pattern D):
   - `LLM_DECOMPILE` referencing `references/server/CSource2GameClients_StartHLTVServer.{platform}.yaml`
   - No `FUNC_VTABLE_RELATIONS` (regular function)
   - Reference YAMLs annotated: `sub_180B1AC80` / `sub_1516AB0` renamed to `LegacyGameEventListener` in both disasm and procedure
   - config.yaml `expected_input`: `CSource2GameClients_StartHLTVServer.{platform}.yaml`

**config.yaml dependency chain:**
```yaml
      - name: find-CSource2GameClients_vtable
        expected_output:
          - CSource2GameClients_vtable.{platform}.yaml

      - name: find-CSource2GameClients_StartHLTVServer
        expected_output:
          - CSource2GameClients_StartHLTVServer.{platform}.yaml
        expected_input:
          - CSource2GameClients_vtable.{platform}.yaml

      - name: find-LegacyGameEventListener
        expected_output:
          - LegacyGameEventListener.{platform}.yaml
        expected_input:
          - CSource2GameClients_StartHLTVServer.{platform}.yaml
```

**Key insight — multi-phase workflow required:** `CSource2GameClients_StartHLTVServer` was a brand-new function with no existing output YAMLs. `generate_reference_yaml.py` needs `func_va` from the predecessor's output YAML to locate it in IDA. So the workflow was:
1. Create all 3 scripts + config entries
2. Run `ida_analyze_bin.py -debug` → vtable + xref scripts succeed and create StartHLTVServer YAMLs
3. Run `generate_reference_yaml.py` using the newly created output YAMLs
4. Annotate reference YAMLs
5. Run `ida_analyze_bin.py -debug` again → LLM_DECOMPILE path runs and succeeds
