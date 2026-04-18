# ida_skill_preprocessor

## Overview
`ida_skill_preprocessor.py` now acts as the "preprocessing dispatch entry": it establishes the IDA MCP session, retrieves `image_base`, and then delegates concrete preprocessing logic to `ida_preprocessor_scripts/{skill_name}.py`.

Unlike the old flow, output-type branching is no longer hardcoded in the entry point. The concrete workflow is decided by each skill script.

## Responsibilities
- Dynamically load and cache the exported method `preprocess_skill` from `ida_preprocessor_scripts/{skill_name}.py`.
- Connect to MCP (`ClientSession` + `streamable_http_client`) and initialize the session in a unified way.
- Retrieve `image_base` via a single `py_eval` call and pass it as a context parameter to skill scripts.
- Invoke the skill script's exported method (supports async/sync) and normalize the result to a boolean for upper-layer return.
- Return `False` when script is missing, export method is missing, script execution fails, or MCP connection fails, so upper layers can fall back to Agent flow.

## Files Involved (no line numbers)
- ida_skill_preprocessor.py
- ida_analyze_util.py
- ida_preprocessor_scripts/*.py
- ida_preprocessor_scripts/find-CTriggerPush_vtable.py
- ida_preprocessor_scripts/find-CTriggerPush_Touch.py
- ida_preprocessor_scripts/find-CCSPlayer_MovementServices_FullWalkMove_SpeedClamp.py
- ida_analyze_bin.py

## Architecture
Entry point `preprocess_single_skill_via_mcp(...)`:
1. Load `ida_preprocessor_scripts/{skill_name}.py` via `_get_preprocess_entry(skill_name)`.
   - The script is expected to export a callable `preprocess_skill`.
   - Script entry points are cached to avoid repeated import overhead.
2. If the script does not exist or exported method is invalid, return `False` directly.
3. Establish MCP session and initialize it.
4. Retrieve `image_base` via one `py_eval` call.
5. Call the script's exported method:
   - Pass parameters: `session, skill_name, expected_outputs, old_yaml_map, new_binary_dir, platform, image_base, debug`.
   - If the return value is awaitable, `await` it; final preprocessing result is `bool(result)`.
6. Any exception returns `False`.

### Skill Script Conventions
- File name: `ida_preprocessor_scripts/{skill_name}.py`
- Exported method: `preprocess_skill(...)`
- Most scripts only need to declare constants and delegate to `preprocess_common_skill`:
  - func/vfunc scripts: declare `TARGET_FUNCTION_NAMES`, pass `func_names=TARGET_FUNCTION_NAMES`
  - func xref scripts: declare `TARGET_FUNCTION_NAMES` and `FUNC_XREFS` (list of `(func_name, xref_strings_list, xref_funcs_list, exclude_funcs_list)` tuples), pass both `func_names=TARGET_FUNCTION_NAMES` and `func_xrefs=FUNC_XREFS`. Unified xref lookup is used as a fallback when `func_sig` reuse fails.
  - gv scripts: declare `TARGET_GLOBALVAR_NAMES`, pass `gv_names=TARGET_GLOBALVAR_NAMES`
  - patch scripts: declare `TARGET_PATCH_NAMES`, pass `patch_names=TARGET_PATCH_NAMES`
  - vtable scripts: declare `TARGET_CLASS_NAME`, pass `vtable_class_names=[TARGET_CLASS_NAME]`
  - mixed scripts: can pass multiple parameter groups at once
- Special scripts (e.g., CTriggerPush_Touch, CBaseTrigger_StartTouch, CPointTeleport_Teleport) implement custom logic and directly use lower-level methods.

## Common Capabilities in ida_analyze_util.py

- MCP result parsing: `parse_mcp_result`
- vtable py_eval template and builder: `_VTABLE_PY_EVAL_TEMPLATE`, `_build_vtable_py_eval`
- YAML writers: `write_vtable_yaml`, `write_func_yaml`, `write_gv_yaml`, `write_patch_yaml`, `write_struct_offset_yaml`
- `preprocess_vtable_via_mcp`: locate and read vtables in IDA by class name, output standardized vtable YAML data.
- `preprocess_func_sig_via_mcp`: prioritize old `func_sig` reuse to locate functions; if missing, fall back to `vfunc_sig` + vtable index and complete new function metadata.
- `preprocess_gen_func_sig_via_mcp`: auto-generate the shortest unique `func_sig` from function prologue for function relocation in new versions and YAML writing.
- `preprocess_gen_gv_sig_via_mcp`: generate the shortest unique `gv_sig` around instructions accessing global variables, and return instruction-offset metadata.
- `preprocess_gv_sig_via_mcp`: relocate global variables in new binaries by reusing old `gv_sig` and rebuild `gv_*` fields.
- `preprocess_patch_via_mcp`: reuse `patch_sig/patch_bytes` from old patch YAML and only succeed when `patch_sig` has exactly one `find_bytes` hit in new binary (`== 1`).
- `preprocess_struct_offset_sig_via_mcp`: reuse old offset signatures to parse struct member offsets and sizes.
- `preprocess_index_based_vfunc_via_mcp`: resolve new addresses and metadata for inherited vfuncs via old base-class `vfunc_index` + new vtable.
- `preprocess_func_xrefs_via_mcp`: locate functions through unified xref constraints, combining string xrefs, dependency-function xrefs, optional vtable-entry filtering, and exclude-function filtering before generating or recovering function YAML data.
- **`preprocess_common_skill`**: unified `preprocess_skill` template that supports combined target types including func/vfunc, gv, patch, struct-member, vtable, inherit-vfunc, and unified xref-based function lookup. Most skill scripts only need constants plus delegation.

YAML writers consistently use `yaml.safe_dump`:
- Ensure key set and ordering control (`sort_keys=False`)
- Scalar quoting/style specifics are decided by PyYAML

## Dependencies
- PyYAML (YAML read/write)
- mcp Python SDK (`ClientSession`, `streamable_http_client`)
- IDA MCP tools: `py_eval`, `find_bytes`
- Standard library: `importlib.util`, `inspect`, `re`, `pathlib`, `json`, `os`

## Notes
- Preprocessing is an "acceleration path": returning `False` is acceptable, and upper layers will fall back to Agent SKILL.
- Now preprocessing success is controlled by each script; scripts should ensure output completeness on their own.
- `preprocess_func_sig_via_mcp` requires unique `find_bytes` hit; both 0 and multiple hits fail.
- `preprocess_patch_via_mcp` also requires unique `patch_sig` hit (`==1`); no hit or multiple hits both fail.
- vfunc offset is still computed as `index * 8` (64-bit assumption).
- If script is missing or export contract is invalid, that skill goes directly to Agent SKILL and does not block the main flow.

## Callers (optional)
- `process_binary` in `ida_analyze_bin.py` (called before `run_skill`)