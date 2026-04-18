# config.yaml format and field reference

## Purpose
`config.yaml` is the central orchestration file for this repository. It feeds:
- `download_bin.py` (binary download targets)
- `ida_analyze_bin.py` (skill execution graph and YAML generation)
- `update_gamedata.py` (symbol/category mapping into final gamedata outputs)
- `run_cpp_tests.py` (optional C++/vtable validation pipeline)

## High-level schema
```yaml
modules:
  - name: <module_name>
    path_windows: <source_bin_relative_path_for_windows>
    path_linux: <source_bin_relative_path_for_linux>
    skills:
      - name: <skill_name>
        expected_output:
          - <artifact_name.{platform}.yaml>
        expected_input:            # optional
          - <artifact_name.{platform}.yaml>
        prerequisite:              # optional (legacy dependency hint)
          - <skill_name>
        max_retries: <int>         # optional
    symbols:
      - name: <symbol_name>
        category: <vtable|vfunc|func|gv|struct|structmember|patch>
        alias:                     # optional; string or list
          - <alternate_name>
        struct: <struct_name>      # required for category=structmember
        member: <member_name>      # required for category=structmember

cpp_tests:
  - name: <test_name>
    symbol: <symbol_or_class_name>
    cpp: <path_to_cpp_file>
    headers:                       # optional
      - <path_to_header>
    target: <clang_target_triple>
    include_directories:           # optional
      - <include_dir>
    defines:                       # optional
      - <macro_or_macro_value>
    additional_compiler_options:   # optional
      - <clang_option>
    reference_modules:             # optional
      - <module_name>
```

## Field-by-field meaning

### Top-level
- `modules` (list): Module declarations used by download/analyze/update flows.
- `cpp_tests` (list): Optional compile-based verification tasks consumed by `run_cpp_tests.py`.

### `modules[]`
- `name` (string): Canonical module key (also output subdirectory name under `bin/{gamever}/`).
- `path_windows` (string): Windows binary path (source-style path). Used to derive file name and download URL; local binary path resolves to `bin/{gamever}/{module}/{filename}`.
- `path_linux` (string): Linux binary path with the same semantics as `path_windows`.
- `skills` (list): Ordered-by-dependency skill definitions for `ida_analyze_bin.py`.
- `symbols` (list): Symbol declarations used by `update_gamedata.py` to load/merge generated YAML and write final gamedata.

### `modules[].skills[]`
- `name` (string): Skill identifier (e.g. `find-...`). Used as the executable skill key (mapped to `/.<skill>` style invocation and project skill assets).
- `expected_output` (list[string]): Output YAML artifacts expected from this skill. Supports `{platform}` placeholder expansion at runtime.
  - Used to skip already completed skills.
  - Used to verify that a run actually produced required files.
- `expected_input` (list[string], optional): Input artifacts required before running the skill. Also supports `{platform}`.
  - Missing files cause the skill to fail early.
  - Also used to infer dependency edges for topological sorting.
- `prerequisite` (list[string], optional, legacy): Explicit dependency skill names kept for backward compatibility.
- `max_retries` (int, optional): Per-skill retry override; if absent, global CLI retry value is used.

### `modules[].symbols[]`
- `name` (string): Canonical symbol name and primary YAML base filename.
- `category` (string): Symbol class. Current values in this repo:
  - `vtable`
  - `vfunc`
  - `func`
  - `gv`
  - `struct`
  - `structmember`
  - `patch`
- `alias` (string | list[string], optional): Alternate names.
  - Used for alias-to-canonical mapping.
  - For `patch`, aliases may also be used as fallback candidate YAML filenames.
- `struct` (string, required when `category=structmember`): Parent struct name.
- `member` (string, required when `category=structmember`): Target member name inside the struct.

## Category-specific behavior notes
- `structmember`:
  - Loader resolves `member` offset from `{name}.{platform}.yaml` first.
  - Falls back to legacy `{struct}.{platform}.yaml` if needed.
- `patch`:
  - Loaded YAML must include `patch_bytes`.
  - If canonical file is unavailable/invalid, alias-based candidate files are attempted.

## `cpp_tests[]`
- `name` (string, optional): Human-readable test label; defaults to `unnamed_test` if omitted.
- `symbol` (string, required): Class/symbol key used for report labeling and vtable comparison target.
- `cpp` (string, required): C++ source path (resolved relative to config directory when not absolute).
- `headers` (list[string], optional): Header paths used by `--fixheader` workflow.
- `target` (string, required): Clang target triple.
- `include_directories` (list[string], optional): Added as `-I` include flags.
- `defines` (list[string], optional): Added as `-D` preprocessor defines.
- `additional_compiler_options` (list[string], optional): Appended compiler options.
  - If options include `fdump-vtable-layouts`, vtable parsing/comparison is enabled.
- `reference_modules` (list[string], optional): Module lookup order for reference YAML in vtable comparison.
  - If multiple are provided, each module is compared separately.
  - If omitted/empty, comparison still runs but reference YAML is typically not found.

## Practical notes
- Skill artifact paths are generally treated as filenames under the active module binary folder (`bin/{gamever}/{module}/...`), with `{platform}` expanded at runtime.
- `expected_output` is strongly recommended for reliable skip/success behavior.
- `run_cpp_tests.py` also accepts `additional_compile_options` as a compatibility alias, but this config uses `additional_compiler_options`.