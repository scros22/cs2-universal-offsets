---
name: convert-finder-skill-to-preprocessor-scripts
description: |
  Convert an existing find-XXXX SKILL.md into a preprocessor Python script, updating config.yaml
  and removing the old SKILL.md. Covers xref-string-based and LLM_DECOMPILE-based discovery patterns.
disable-model-invocation: true
---

# Convert Finder SKILL.md to Preprocessor Script

Port an existing `.claude/skills/find-XXXX/SKILL.md` into an `ida_preprocessor_scripts/find-XXXX.py`
preprocessor script, update `config.yaml` entries, and delete the old SKILL.md.

## When to Use

- A `find-XXXX` SKILL.md exists in `.claude/skills/` and needs to be converted to a preprocessor script
- The SKILL.md uses either **xref-string search** (`find_regex` / `xrefs_to`) or **decompile-based vtable analysis** to discover functions

## Overview

Six preprocessor patterns exist. The SKILL.md's discovery method and target type determine which to use:

| Pattern | Discovery Method | Has FUNC_XREFS | Has LLM_DECOMPILE | Has INHERIT_VFUNCS | Has FUNC_VTABLE_RELATIONS | preprocess_skill has llm_config |
|---------|-----------------|-----------------|---------------------|--------------------|---------------------------|-------------------------------|
| **A** — Regular function via xref strings | `find_regex` + `xrefs_to` on debug strings | Yes | No | No | No | No |
| **B** — Virtual function via xref strings | Same as A, but function is in a vtable | Yes | No | No | Yes | No |
| **C** — Virtual function via LLM_DECOMPILE | Decompile a known predecessor function, identify vfunc call offsets | No | Yes | No | Yes | Yes |
| **D** — Regular function via LLM_DECOMPILE | Decompile a known predecessor function, identify direct call targets | No | Yes | No | No | Yes |
| **E** — Struct member offset via LLM_DECOMPILE | Decompile a known predecessor function, identify struct field access offsets | No | Yes | No | No | Yes |
| **F** — Virtual function via INHERIT_VFUNCS | Inherit vtable slot index from a known base-class vfunc, look up same slot in derived-class vtable | No | No | Yes | No | No |

Additionally, **struct member offsets** can be mixed into any pattern as a secondary target (see "Struct Member Mixin" section below).

---

## Step 1: Read and Analyze the SKILL.md

Read the target `.claude/skills/find-XXXX/SKILL.md`.

Extract:
1. **Target function names** — all functions the skill identifies (may be 1 or many)
2. **Target struct member names** — all struct member offsets the skill identifies (e.g. `CCheckTransmitInfo_m_nPlayerSlot`)
3. **Discovery method** for each target:
   - Does it use `find_regex` / `xrefs_to` with debug strings? → **xref-string based** (Patterns A/B)
   - Does it load a predecessor YAML, decompile that function, and extract vfunc offsets / struct offsets from code patterns? → **LLM_DECOMPILE based** (Patterns C/D/E)
   - Is the target a derived-class override of a known base-class vfunc (same vtable slot, different class)? → **INHERIT_VFUNCS based** (Pattern F)
4. **Function category** — `func` (regular), `vfunc` (virtual, has vtable slot), or `structmember`
5. **VTable class name** — if virtual, e.g. `CBaseEntity`, `CBasePlayerPawn`, `INetworkMessages`
6. **Xref strings** — debug strings used in `find_regex` patterns (for xref-string patterns). **Check if these differ between Windows and Linux** — if so, you need platform-specific `FUNC_XREFS_WINDOWS` / `FUNC_XREFS_LINUX`. Use the `FULLMATCH:` prefix (e.g. `"FULLMATCH:Precache"`) when you need exact-string matching instead of substring matching — this prevents false positives when the target string is short or generic (e.g. `"Precache"`, `"userid"`, `"team"`).
7. **Predecessor function** — the function whose decompiled code reveals the target (for LLM_DECOMPILE patterns)
8. **Base vfunc for inheritance** — if the target is a derived-class override of a known base-class vfunc, the base vfunc name (for INHERIT_VFUNCS pattern)
9. **Dependencies** — which existing YAMLs are needed as inputs (vtable YAMLs, predecessor function YAMLs, base vfunc YAMLs)

## Step 2: Plan the Split

If the SKILL.md discovers multiple functions using **different methods** or from **different starting points**, split them into separate preprocessor scripts. Each script handles one "discovery unit" — a group of functions findable from the same method and starting point.

**Same script:** Functions found from the same xref string, or from the same decompiled reference.
**Separate scripts:** Functions found by xref strings vs. functions found by decompiling one of those xref-found functions.

Example split (what we did for CBaseEntity_TakeDamageOld):
- Script 1: `find-CBaseEntity_TakeDamageOld.py` — finds TakeDamageOld via xref string (Pattern A)
- Script 2: `find-CBaseEntity_OnTakeDamage.py` — finds OnTakeDamage by decompiling TakeDamageOld (Pattern C)
- Script 3: `find-CBaseEntity_OnTakeDamage_Alive-AND-Dying-AND-Dead.py` — finds 3 vfuncs by decompiling OnTakeDamage (Pattern C)

## Step 3: Generate the Preprocessor Script(s)

Script location: `ida_preprocessor_scripts/find-{skill_name}.py`

The filename MUST match the `name` field in `config.yaml` skill entry.

### Pattern A — Regular function via xref strings

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
            "{XREF_STRING_1}",  # Debug string from SKILL.md's find_regex pattern
        ],
        [],   # xref_signatures_list — byte patterns if needed, usually empty
        [],   # xref_funcs_list — known caller function names if needed
        [],   # exclude_funcs_list — function names to exclude from results
        [],   # exclude_strings_list — strings to exclude
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

### Pattern B — Virtual function via xref strings

Use when: function IS virtual (has vtable slot), but discovered via debug string cross-references.

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

When xref strings differ between Windows and Linux (e.g. Windows has full `ClassName::Method` assertion strings while Linux has only `./filename.cpp:linenum`), split into two variables:

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

This applies to both Pattern A and Pattern B — the only change is replacing the single `FUNC_XREFS` with the platform-specific pair.

### Pattern C — Virtual function via LLM_DECOMPILE

Use when: function IS virtual (has vtable slot), discovered by decompiling a known predecessor function and reading vfunc call offsets from the decompiled code.

```python
#!/usr/bin/env python3
"""Preprocess script for find-{SKILL_NAME} skill."""

from ida_analyze_util import preprocess_common_skill

TARGET_FUNCTION_NAMES = [
    "{FUNC_NAME_1}",
    # "{FUNC_NAME_2}",  # Add more if the skill finds multiple functions from the same reference
]

LLM_DECOMPILE = [
    # (symbol_name, path_to_prompt, path_to_reference)
    # ONE entry per target function. All entries sharing the same reference
    # YAML will be resolved from the same decompiled predecessor code.
    (
        "{FUNC_NAME_1}",
        "prompt/call_llm_decompile.md",
        "references/{MODULE}/{PREDECESSOR_FUNC}.{platform}.yaml",
    ),
    (
        "{FUNC_NAME_2}",
        "prompt/call_llm_decompile.md",
        "references/{MODULE}/{PREDECESSOR_FUNC}.{platform}.yaml",
    ),
    # ... one entry per target function, all pointing to the same reference
]

FUNC_VTABLE_RELATIONS = [
    # (func_name, vtable_class)
    ("{FUNC_NAME_1}", "{VTABLE_CLASS}"),
    ("{FUNC_NAME_2}", "{VTABLE_CLASS}"),
    # ... one entry per target function
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
    (
        "{FUNC_NAME_2}",
        [
            "func_name",
            "vfunc_sig",
            "vfunc_offset",
            "vfunc_index",
            "vtable_name",
        ],
    ),
    # ... one entry per target function
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

### Pattern D — Regular function via LLM_DECOMPILE

Use when: function is NOT virtual, discovered by decompiling a known predecessor function and identifying direct call targets (not vtable-based calls) from the decompiled code.

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

### Pattern E — Struct member offset via LLM_DECOMPILE

Use when: target is a **struct member offset** (not a function), discovered by decompiling a known predecessor function and identifying struct field access patterns (e.g. `*(int *)(ptr + 0x240)`).

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

### Pattern F — Virtual function via INHERIT_VFUNCS

Use when: the target is a **derived-class override** of a known base-class virtual function. The base vfunc has already been found (by another script), and this script inherits its vtable slot index to look up the same slot in the derived class's vtable.

This is the simplest pattern — no xref strings, no LLM decompilation needed. Just a vtable slot lookup.

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
- `target_func_name` — name for the derived-class function (e.g. `"CBaseEntity_Precache"`)
- `inherit_vtable_class` — class whose vtable to look up (e.g. `"CBaseEntity"`)
- `base_vfunc_name` — YAML artifact stem of the base-class vfunc that defines the slot index (e.g. `"CEntityInstance_Precache"`). Can be cross-module: `"../engine/INetworkMessages_FindNetworkGroup"`
- `generate_func_sig` — (optional, default True) whether to generate a func_sig if no old YAML exists

**Key differences from other patterns:**
- No `TARGET_FUNCTION_NAMES`, `FUNC_XREFS`, `LLM_DECOMPILE`, or `FUNC_VTABLE_RELATIONS`
- Uses `inherit_vfuncs=` parameter instead of `func_names=`
- No `llm_config` parameter in `preprocess_skill`
- config.yaml `expected_input` must include both the base vfunc YAML and the derived class vtable YAML
- config.yaml symbol category is `vfunc`

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
| Target list | `TARGET_FUNCTION_NAMES` | `TARGET_FUNCTION_NAMES` | `TARGET_FUNCTION_NAMES` | `TARGET_FUNCTION_NAMES` | `TARGET_STRUCT_MEMBER_NAMES` | (none — defined in INHERIT_VFUNCS) |
| preprocess param | `func_names=` | `func_names=` | `func_names=` | `func_names=` | `struct_member_names=` | `inherit_vfuncs=` |
| YAML fields | func_name, func_sig, func_va, func_rva, func_size | Same + vtable_name, vfunc_offset, vfunc_index | func_name, vfunc_sig, vfunc_offset, vfunc_index, vtable_name | func_name, func_sig, func_va, func_rva, func_size | struct_name, member_name, offset, size, offset_sig, offset_sig_disp | func_name, func_va, func_rva, func_size, func_sig, vtable_name, vfunc_offset, vfunc_index |
| config category | `func` | `vfunc` | `vfunc` | `func` | `structmember` | `vfunc` |

---

## Step 4: Update config.yaml

### 4a. Skills Section

Each preprocessor script needs a corresponding skill entry under the appropriate module's `skills:` list.

Find the module section (e.g. `server`, `engine`, `networksystem`) and add/update entries.

**Template:**

```yaml
      - name: find-{SKILL_NAME}
        expected_output:
          - {FUNC_NAME_1}.{platform}.yaml
          # - {FUNC_NAME_2}.{platform}.yaml  # One per target function
        # expected_input only if the skill depends on other YAMLs:
        expected_input:
          - {PREDECESSOR_FUNC}.{platform}.yaml    # For Pattern C: the reference function
          - {VTABLE_CLASS}_vtable.{platform}.yaml  # For Patterns B & C: the vtable
```

**Rules:**
- `expected_output`: One `.{platform}.yaml` per target function in the script
- `expected_input`: Include predecessor function YAML (Patterns C & D) and/or vtable YAML (Patterns B & C & F)
- Pattern A with no vtable: typically NO `expected_input` (Pattern D still needs predecessor in `expected_input`)
- Pattern F: needs both the derived class vtable YAML and the base vfunc YAML in `expected_input`
- If splitting a combined skill, each new entry should have its own `expected_input` referencing the predecessor's output

**Dependency chain example** (3-script split):

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

      # Pattern C: found by decompiling FuncB, needs FuncB + vtable
      - name: find-FuncC1-AND-FuncC2-AND-FuncC3
        expected_output:
          - FuncC1.{platform}.yaml
          - FuncC2.{platform}.yaml
          - FuncC3.{platform}.yaml
        expected_input:
          - FuncB.{platform}.yaml
          - SomeClass_vtable.{platform}.yaml
```

### 4b. Symbols Section

For each NEW target function, add a symbol entry under the same module's `symbols:` list (if not already present).

```yaml
      # Regular function (Pattern A)
      - name: {FUNC_NAME}
        category: func
        alias:
          - {ClassName}::{MethodName}   # e.g. CBaseEntity::TakeDamageOld

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
          - {StructName}::{MemberName}   # e.g. CCheckTransmitInfo::m_nPlayerSlot
```

Check existing symbols before adding — do NOT create duplicates.

---

## Step 5: Handle Reference YAMLs (Patterns C & D)

Pattern C and D scripts reference a predecessor function's YAML at:
`ida_preprocessor_scripts/references/{module}/{PREDECESSOR_FUNC}.{platform}.yaml`

These reference files contain the decompiled code of the predecessor function (both `disasm_code` and `procedure` fields) so the LLM can identify call patterns.

**Check** if the reference YAML already exists:
- `ida_preprocessor_scripts/references/{module}/{PREDECESSOR_FUNC}.linux.yaml`
- `ida_preprocessor_scripts/references/{module}/{PREDECESSOR_FUNC}.windows.yaml`

If NOT present, generate them using `generate_reference_yaml.py`:

```bash
# Windows (module is inferred from the binary path)
uv run generate_reference_yaml.py -func_name {PREDECESSOR_FUNC} -auto_start_mcp -binary "bin/{gamever}/{module}/{binary_name}.dll" -debug

# Linux
uv run generate_reference_yaml.py -func_name {PREDECESSOR_FUNC} -auto_start_mcp -binary "bin/{gamever}/{module}/lib{module}.so" -debug
```

For example, for the `server` module:
```bash
uv run generate_reference_yaml.py -func_name CCSGameRules_TerminateRound -auto_start_mcp -binary "bin/{gamever}/server/server.dll" -debug
uv run generate_reference_yaml.py -func_name CCSGameRules_TerminateRound -auto_start_mcp -binary "bin/{gamever}/server/libserver.so" -debug
```

where `{gamever}` can be obtain from `.env` -> `CS2VIBE_GAMEVER`, or `14141c` if you can't read `.env`.

YOU MUST: rename known symbols / add necessary comments in the generated reference YAMLs the so LLM can find desired symbols by comparing reference ones with raw procedure/disassembly read from new binaries.

For example, if we want the LLM to find `CEntityInstance_AcceptInput` in the owner function:

```c
      do
      {
        sub_1811A0200(*(_QWORD *)(v28 + qword_181D6CD08), (__int64)"CTsWin", 0, 0, (__int64)&v124, 0, 0);
        ++v27;
        v28 += 8;
      }
      while ( v27 < dword_181D6CD00 );
```

```
  .text:00000001808BC82B                 call    sub_1811A0200
```

We **MUST** be renamed not only `procedure`:

```c
      do
      {
        CEntityInstance_AcceptInput(*(_QWORD *)(v28 + qword_181D6CD08), (__int64)"CTsWin", 0, 0, (__int64)&v124, 0, 0);
        ++v27;
        v28 += 8;
      }
      while ( v27 < dword_181D6CD00 );
```

but also `disassembly`:

```
  .text:00000001808BC82B                 call    CEntityInstance_AcceptInput
```

For example, if we want the LLM to find `CBaseEntity_OnTakeDamage` as an indirect call to virtual function in the owner function:

We **MUST** add comments not only in `procedure`:

```c
(*(void (__fastcall **)(_QWORD *, _DWORD *))(*a1 + 1008LL))(a1, v6); // 1008LL = CBaseEntity_OnTakeDamage
```

but also in `disassembly`:

```
00000001803CEF54 FF 90 F0 03 00 00    call    qword ptr [rax+3F0h] ; 0x3F0 = CBaseEntity_OnTakeDamage
```

For example, if we want the LLM to find `g_pNavMesh` as a global variable in the owner function:

```c
if ( !qword_18200B918 || !*(_BYTE *)(qword_18200B918 + 264) )
    return 0;
```

```
.text:00000001802A6E3C 48 8B 05 D5 4A D6 01                                mov     rax, cs:qword_18200B918
```

We **MUST** rename it not only in `procedure`:

```c
if ( !g_pNavMesh || !*(_BYTE *)(g_pNavMesh + 264) )
    return 0;
```

but also in `disassembly`:

```
.text:00000001802A6E3C 48 8B 05 D5 4A D6 01                                mov     rax, cs:g_pNavMesh
```

For example, if we want the LLM to find `CCheckTransmitInfo_m_nPlayerSlot` as a struct member offset (0x240) in the owner function:

```c
v18 = sub_180BCF9E0(*(unsigned int *)(*v6 + 576));
```

```
  .text:0000000180C99633                 mov     ecx, [rdi+240h]
```

We **MUST** add comments not only in `procedure`:

```c
v18 = sub_180BCF9E0(*(unsigned int *)(*v6 + 576)); // 576 = 0x240 = CCheckTransmitInfo::m_nPlayerSlot
```

but also in `disassembly`:

```
  .text:0000000180C99633                 mov     ecx, [rdi+240h] ; 0x240 = CCheckTransmitInfo::m_nPlayerSlot
```

**Prerequisites:** The predecessor function must already be named in the IDA database for the target binary. If it is not named yet, ask the user to either:
1. Connect IDA Pro MCP and rename the function first, or
2. Manually rename it in IDA before running the script

**IMPORTANT — `generate_reference_yaml.py` address resolution:** The script resolves the predecessor function's address by reading `func_va` from the existing output YAML at `bin/{gamever}/{module}/{PREDECESSOR_FUNC}.{platform}.yaml`. If the predecessor is one of the target functions being converted (e.g., splitting a combined skill where Script 1 finds FuncA and Script 2 decompiles FuncA), you **MUST generate the reference YAMLs BEFORE deleting existing output YAMLs** in Step 7. Otherwise, the address data needed by `generate_reference_yaml.py` will be destroyed and you'll need to recreate temporary YAMLs or ask the user for the function address.

**IMPORTANT — When the predecessor is a NEW function (no existing output YAMLs):** If the predecessor function is brand new (discovered by another new script you're creating in the same conversion), its output YAMLs don't exist yet and `generate_reference_yaml.py` cannot resolve its address. You must use a **multi-phase workflow**:

1. **Phase 1:** Create ALL scripts (vtable, xref_string, LLM_DECOMPILE) and update config.yaml
2. **Phase 2:** Run `uv run ida_analyze_bin.py -debug` — the vtable and xref_string scripts will succeed and populate the NEW predecessor's output YAMLs. The LLM_DECOMPILE script's target may be skipped if old output YAMLs with valid `func_sig` still exist.
3. **Phase 3:** Now that the predecessor has output YAMLs, run `generate_reference_yaml.py` to create reference YAMLs, then annotate them.
4. **Phase 4:** Delete the old target output YAMLs (so the LLM_DECOMPILE path is actually exercised)
5. **Phase 5:** Run `uv run ida_analyze_bin.py -debug` again — this time the LLM_DECOMPILE path runs and the full pipeline is validated.

Run the command once per platform (windows/linux) that needs a reference YAML. The `-module` and `-platform` are inferred from the `-binary` path automatically.

---

## Step 6: Delete the SKILL.md

After the preprocessor script is created and config.yaml is updated:

1. Delete the SKILL.md file: `.claude/skills/find-{SKILL_NAME}/SKILL.md`
2. Delete the now-empty directory: `.claude/skills/find-{SKILL_NAME}/`

If a combined SKILL.md was split into multiple preprocessor scripts, delete the single original SKILL.md.

---

## Step 7: Delete Existing Output YAMLs

**IMPORTANT:** This step MUST happen AFTER Step 5 (reference YAML generation). The `generate_reference_yaml.py` script reads `func_va` from these output YAMLs to locate functions in IDA. Deleting them first will break reference generation.

After the preprocessor script is created, the old SKILL.md is deleted, and any needed reference YAMLs are generated, remove all previously generated output YAMLs so the user can validate the new preprocessor script from scratch by running `uv run ida_analyze_bin.py`.

For each target function, delete all matching YAMLs across all game versions:

```
bin/*/{module}/{FUNC_NAME}.windows.yaml
bin/*/{module}/{FUNC_NAME}.linux.yaml
```

For example, if the skill targets `CBasePlayerController_HandleCommand_JoinTeam` in the `server` module:

```bash
find bin -name "CBasePlayerController_HandleCommand_JoinTeam.*.yaml" -delete
```

If the skill was split into multiple scripts with multiple target functions, delete YAMLs for ALL target functions.

---

## Step 8: Remove Entry from docs/claude_skills_stats.yaml

After the conversion is complete and validated, delete the converted skill's entry from `docs/claude_skills_stats.yaml`. This file tracks skills that still use the old SKILL.md format — once converted to a preprocessor script, the entry is no longer relevant.

Remove the entire YAML block for each converted symbol, e.g.:

```yaml
# Delete this entire block:
- symbol_name: CBasePlayerController_HandleCommand_JoinTeam
  skill_name: find-CBasePlayerController_HandleCommand_JoinTeam
  classicy: with_xref_strings
  owner_func_name: CBasePlayerController_HandleCommand_JoinTeam
  owner_module: server
```

If the original SKILL.md covered multiple symbols, delete ALL corresponding entries from the stats file.

---

## Step 9: Run Tests

After all conversion steps are complete, run the full preprocessor test to validate the new script works.

Because the output is very long, redirect it to a temp file and then read just the summary:

```bash
uv run ida_analyze_bin.py -debug > /tmp/ida_test_output.txt 2>&1; tail -10 /tmp/ida_test_output.txt
```

Check the **Summary** at the end of the output:
- **Failed: 0** means the conversion is correct
- If any failures, search the full output for the failing skill name to investigate:
  ```bash
  grep -A 5 "Failed\|Error" /tmp/ida_test_output.txt
  ```

This step is mandatory — do not report completion without running and passing this validation.

---

## Step 10: Commit Changes

After validation passes, commit all conversion-related changes to git:

```bash
git add <preprocessor_script> <deleted_skill_md> <config.yaml if changed> docs/claude_skills_stats.yaml
git commit -m "Convert find-{SKILL_NAME} SKILL.md to preprocessor script"
```

Include all files changed during the conversion:
- The new/updated preprocessor script
- The deleted SKILL.md
- Any config.yaml changes
- The updated `docs/claude_skills_stats.yaml`

Do NOT include unrelated changes (e.g. `.claude/settings.json` permission changes).

---

## Checklist

Before finishing, verify:

- [ ] Preprocessor script file name matches the `name` field in config.yaml skill entry
- [ ] `TARGET_FUNCTION_NAMES` lists all functions the script should find
- [ ] `FUNC_XREFS` xref strings match the debug strings from the original SKILL.md (Pattern A/B)
- [ ] `LLM_DECOMPILE` reference path points to the correct predecessor function YAML (Patterns C/D)
- [ ] `FUNC_VTABLE_RELATIONS` lists correct vtable class for each virtual function (Patterns B/C only, NOT Patterns D/F)
- [ ] `INHERIT_VFUNCS` lists correct (target, derived_class, base_vfunc, gen_sig) tuples (Pattern F only)
- [ ] `GENERATE_YAML_DESIRED_FIELDS` uses correct field set for the pattern
- [ ] `preprocess_skill` signature includes `llm_config=None` if and only if LLM_DECOMPILE is used (NOT for Pattern F)
- [ ] `preprocess_common_skill` call passes all relevant lists (`func_xrefs`, `func_vtable_relations`, `llm_decompile_specs`, `llm_config`, `inherit_vfuncs`)
- [ ] config.yaml `expected_output` has one entry per target function
- [ ] config.yaml `expected_input` correctly chains dependencies
- [ ] config.yaml `symbols` section has entries for all target functions (no duplicates)
- [ ] Reference YAMLs exist or generated via `uv run generate_reference_yaml.py` (Patterns C/D) — **must be done BEFORE deleting output YAMLs**
- [ ] Old SKILL.md and its directory are deleted
- [ ] Existing output YAMLs under `bin/*/` are deleted for all target functions (AFTER reference YAML generation)
- [ ] Entry removed from `docs/claude_skills_stats.yaml` for all converted symbols
- [ ] `uv run ida_analyze_bin.py -debug` passes with 0 failures
- [ ] All conversion changes committed to git

## Real-World Examples

### Example: xref-string regular function (Pattern A)

**Before:** `.claude/skills/find-CBaseEntity_TakeDamageOld/SKILL.md` — used `find_regex pattern="TakeDamageOld.*GetDamageForce"` to locate the function.

**After:** `ida_preprocessor_scripts/find-CBaseEntity_TakeDamageOld.py` with:
- `FUNC_XREFS` containing `"CBaseEntity::TakeDamageOld: damagetype %d with info.GetDamageForce() == Vector::vZero"`
- `GENERATE_YAML_DESIRED_FIELDS` with `func_name, func_sig, func_va, func_rva, func_size`
- No `FUNC_VTABLE_RELATIONS`, no `LLM_DECOMPILE`

### Example: LLM_DECOMPILE virtual function (Pattern C)

**Derived from:** the same SKILL.md's "decompile TakeDamageOld and find vfunc call to OnTakeDamage" steps.

**Result:** `ida_preprocessor_scripts/find-CBaseEntity_OnTakeDamage.py` with:
- `LLM_DECOMPILE` referencing `references/server/CBaseEntity_TakeDamageOld.{platform}.yaml`
- `FUNC_VTABLE_RELATIONS`: `("CBaseEntity_OnTakeDamage", "CBasePlayerPawn")`
- `GENERATE_YAML_DESIRED_FIELDS` with `func_name, vfunc_sig, vfunc_offset, vfunc_index, vtable_name`

### Example: LLM_DECOMPILE multiple virtual functions (Pattern C)

**Derived from:** the same SKILL.md's "decompile OnTakeDamage and find Alive/Dying/Dead vfunc offsets" steps.

**Result:** `ida_preprocessor_scripts/find-CBaseEntity_OnTakeDamage_Alive-AND-Dying-AND-Dead.py` with:
- 3 target functions, 3 `LLM_DECOMPILE` entries (all referencing the same YAML), 3 `FUNC_VTABLE_RELATIONS` entries, 3 `GENERATE_YAML_DESIRED_FIELDS` entries
- Each `LLM_DECOMPILE` entry references `references/server/CBaseEntity_OnTakeDamage.{platform}.yaml`

### Example: Split xref-string + LLM_DECOMPILE regular function (Patterns A + D)

**Before:** `.claude/skills/find-CCSGameRules_TerminateRound-AND-CEntityInstance_AcceptInput/SKILL.md` — found TerminateRound via `find_regex pattern="TerminateRound"`, then decompiled it to find AcceptInput called with "CTsWin"/"TerroristsWin" string arguments.

**Split into two scripts:**

1. `ida_preprocessor_scripts/find-CCSGameRules_TerminateRound.py` (Pattern A):
   - `FUNC_XREFS` containing `"TerminateRound"`
   - `GENERATE_YAML_DESIRED_FIELDS` with `func_name, func_sig, func_va, func_rva, func_size`
   - No `FUNC_VTABLE_RELATIONS`, no `LLM_DECOMPILE`

2. `ida_preprocessor_scripts/find-CEntityInstance_AcceptInput.py` (Pattern D):
   - `LLM_DECOMPILE` referencing `references/server/CCSGameRules_TerminateRound.{platform}.yaml`
   - `GENERATE_YAML_DESIRED_FIELDS` with `func_name, func_sig, func_va, func_rva, func_size`
   - No `FUNC_VTABLE_RELATIONS` (it's a regular function, not virtual)
   - Reference YAMLs generated via `uv run generate_reference_yaml.py -func_name CCSGameRules_TerminateRound -auto_start_mcp -binary "bin/%CS2VIBE_GAMEVER%/server/server.dll" -debug`

**config.yaml dependency chain:**
```yaml
      - name: find-CCSGameRules_TerminateRound
        expected_output:
          - CCSGameRules_TerminateRound.{platform}.yaml

      - name: find-CEntityInstance_AcceptInput
        expected_output:
          - CEntityInstance_AcceptInput.{platform}.yaml
        expected_input:
          - CCSGameRules_TerminateRound.{platform}.yaml
```

### Example: Split xref-string vfunc + LLM_DECOMPILE struct offset (Patterns B + E with platform-specific xrefs)

**Before:** `.claude/skills/find-CSource2GameEntities_CheckTransmit-AND-CCheckTransmitInfo/SKILL.md` — found CheckTransmit via `find_regex pattern="CSource2GameEntities::CheckTransmit"`, then examined the decompiled code to find `CCheckTransmitInfo::m_nPlayerSlot` at offset 0x240.

**Split into two scripts:**

1. `ida_preprocessor_scripts/find-CSource2GameEntities_CheckTransmit.py` (Pattern B with platform-specific xrefs):
   - `FUNC_XREFS_WINDOWS` containing `"CSource2GameEntities::CheckTransmit"` (full assertion string on Windows)
   - `FUNC_XREFS_LINUX` containing `"./gameinterface.cpp:30"` (shorter path on Linux — Windows string not present in Linux binary)
   - `FUNC_VTABLE_RELATIONS`: `("CSource2GameEntities_CheckTransmit", "CSource2GameEntities")`
   - `GENERATE_YAML_DESIRED_FIELDS` with `func_name, func_va, func_rva, func_size, func_sig, vtable_name, vfunc_offset, vfunc_index`
   - Uses `func_xrefs=FUNC_XREFS_WINDOWS if platform == "windows" else FUNC_XREFS_LINUX` in the `preprocess_common_skill` call

2. `ida_preprocessor_scripts/find-CCheckTransmitInfo_m_nPlayerSlot.py` (Pattern E):
   - `TARGET_STRUCT_MEMBER_NAMES` containing `"CCheckTransmitInfo_m_nPlayerSlot"`
   - `LLM_DECOMPILE` referencing `references/server/CSource2GameEntities_CheckTransmit.{platform}.yaml`
   - `GENERATE_YAML_DESIRED_FIELDS` with `struct_name, member_name, offset, size, offset_sig, offset_sig_disp`
   - Reference YAMLs annotated with `; 0x240 = CCheckTransmitInfo::m_nPlayerSlot` in disasm and `// 576 = 0x240 = CCheckTransmitInfo::m_nPlayerSlot` in procedure

**config.yaml dependency chain:**
```yaml
      - name: find-CSource2GameEntities_CheckTransmit
        expected_output:
          - CSource2GameEntities_CheckTransmit.{platform}.yaml
        expected_input:
          - CSource2GameEntities_vtable.{platform}.yaml

      - name: find-CCheckTransmitInfo_m_nPlayerSlot
        expected_output:
          - CCheckTransmitInfo_m_nPlayerSlot.{platform}.yaml
        expected_input:
          - CSource2GameEntities_CheckTransmit.{platform}.yaml
```

### Example: Split xref-string vfunc + INHERIT_VFUNCS derived vfunc (Patterns B + F)

**Before:** `.claude/skills/find-CBaseEntity_Precache/SKILL.md` — found CBaseEntity_Precache via a wrapper function referencing `"bloodspray"`. CBaseEntity_Precache is actually a derived-class override of CEntityInstance::Precache.

**Split into two scripts with swapped relationship:**

1. `ida_preprocessor_scripts/find-CEntityInstance_Precache.py` (Pattern B with FULLMATCH):
   - `FUNC_XREFS` containing `"FULLMATCH:Precache"` (exact match to avoid false positives on short string)
   - `FUNC_VTABLE_RELATIONS`: `("CEntityInstance_Precache", "CEntityInstance")`
   - `GENERATE_YAML_DESIRED_FIELDS` with vtable fields
   - This is the **base class** vfunc — found directly via xref strings

2. `ida_preprocessor_scripts/find-CBaseEntity_Precache.py` (Pattern F):
   - `INHERIT_VFUNCS`: `("CBaseEntity_Precache", "CBaseEntity", "CEntityInstance_Precache", True)`
   - `GENERATE_YAML_DESIRED_FIELDS` with vtable fields
   - No FUNC_XREFS, no LLM_DECOMPILE — inherits the vtable slot from CEntityInstance_Precache and looks it up in CBaseEntity's vtable

**config.yaml dependency chain:**
```yaml
      - name: find-CEntityInstance_Precache
        expected_output:
          - CEntityInstance_Precache.{platform}.yaml
        expected_input:
          - CEntityInstance_vtable.{platform}.yaml

      - name: find-CBaseEntity_Precache
        expected_output:
          - CBaseEntity_Precache.{platform}.yaml
        expected_input:
          - CBaseEntity_vtable.{platform}.yaml
          - CEntityInstance_Precache.{platform}.yaml
```

**Key insight:** The SKILL.md originally found CBaseEntity_Precache directly. When converting, we recognized that `Precache` is a base-class virtual method on `CEntityInstance`, and `CBaseEntity::Precache` is just an override at the same vtable slot. So we split into: find the base method via xref strings → inherit to find the derived override.

### Example: Split vtable + xref-string vfunc + LLM_DECOMPILE regular function (vtable + Patterns B + D)

**Before:** `.claude/skills/find-LegacyGameEventListener/SKILL.md` — found CSource2GameClients_StartHLTVServer via `find_regex pattern="CSource2GameClients::StartHLTVServer: game event %s not found"`, then decompiled it to find LegacyGameEventListener called with `a2` parameter.

**Split into three scripts:**

1. `ida_preprocessor_scripts/find-CSource2GameClients_vtable.py` (vtable discovery):
   - `TARGET_CLASS_NAMES`: `["CSource2GameClients"]`
   - No FUNC_XREFS, no LLM_DECOMPILE — pure vtable lookup

2. `ida_preprocessor_scripts/find-CSource2GameClients_StartHLTVServer.py` (Pattern B):
   - `FUNC_XREFS` containing `"CSource2GameClients::StartHLTVServer: game event %s not found"`
   - `FUNC_VTABLE_RELATIONS`: `("CSource2GameClients_StartHLTVServer", "CSource2GameClients")`
   - `GENERATE_YAML_DESIRED_FIELDS` with vtable fields
   - This is the **intermediate function** — found via xref strings, serves as predecessor for the next script

3. `ida_preprocessor_scripts/find-LegacyGameEventListener.py` (Pattern D):
   - `LLM_DECOMPILE` referencing `references/server/CSource2GameClients_StartHLTVServer.{platform}.yaml`
   - `GENERATE_YAML_DESIRED_FIELDS` with `func_name, func_sig, func_va, func_rva, func_size`
   - No `FUNC_VTABLE_RELATIONS` (it's a regular function, not virtual)
   - Reference YAMLs annotated with `LegacyGameEventListener` replacing `sub_180B1AC80` / `sub_1516AB0` in both disasm and procedure

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

**Key insight — multi-phase workflow:** The predecessor `CSource2GameClients_StartHLTVServer` was a brand-new function with no existing output YAMLs. So `generate_reference_yaml.py` couldn't run until the xref_string script populated its output. The workflow was:
1. Create all 3 scripts + config entries
2. Run `ida_analyze_bin.py -debug` → vtable + xref_string scripts succeed, LegacyGameEventListener skipped (old output YAMLs with valid `func_sig` still exist)
3. Run `generate_reference_yaml.py` for both platforms using the newly created StartHLTVServer output YAMLs
4. Annotate reference YAMLs (rename `sub_180B1AC80` → `LegacyGameEventListener` in both disasm and procedure)
5. Delete old LegacyGameEventListener output YAMLs
6. Run `ida_analyze_bin.py -debug` again → LLM_DECOMPILE path runs and succeeds
