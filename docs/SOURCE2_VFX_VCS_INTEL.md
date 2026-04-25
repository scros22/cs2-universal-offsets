# Source2 VFX/VCS Intel (Apr 25, 2026)

This note captures the latest reverse-engineering pass for the
"custom Source2 shader" question (non-HLSL runtime path vs compiled assets).

## Executive summary

- CS2 exposes a runtime `D3DCompile` path in `rendersystemdx11.dll`, but this
  is the DirectX/HLSL layer, not a full Source2 VFX authoring/runtime compiler.
- The Source2 material path in `materialsystem2.dll` still depends on static
  combo/cache lookups tied to VCS program data.
- Practical implication: if pure runtime injection is blocked, custom material
  workflows should target precompiled assets (`.vmat_c` + matching compiled
  shader/program assets) and loader-compatible mounting.

## MCP-validated function chain

### Runtime compile + queue path

- `rendersystemdx11!sub_18003FAF0`
  - Calls `D3DCompile(..., "main", profile, 0x1200, ...)`.
  - Signature name: `CRenderDeviceDx11_CompileShaderSourceMain`.
- `materialsystem2!sub_18003A200`
  - Batch driver that logs `Compiling %i shaders:` and
    `Compiled %i shaders (%i cached) in %.1fs`.
  - Calls `materialsystem2!sub_180013FA0` per queued item.
  - Added signature name: `CMaterialSystem2_DynamicShaderCompile_ProcessQueue`.
- `materialsystem2!sub_180013FA0`
  - `CMaterial2_CompileComboAndGetVariables_DynamicShaderCompile`.

### Static combo + VCS cache path

- `materialsystem2!sub_180015BC0`
  - `CMaterialLayer_ComputeWorkItemsToSetupStaticCombosForMode`.
  - Logs `Failed call to FindOrLoadStaticComboData()`.
  - Calls `sub_1800BDAE0` (merge/validation worker).
- `materialsystem2!sub_1800AE220`
  - `CVfxProgramData_FindOrCreateStaticComboDataInCache`.
  - Logs `Error finding static combo data in VCS file!` and cache consistency errors.
- `materialsystem2!sub_1800AE950`
  - Wrapper/cache gate that calls `sub_1800AE220` on miss or invalid state.
- `materialsystem2!sub_1800BDAE0`
  - Static combo merge/validation worker (now signatured as
    `CVfxProgramData_FindOrLoadStaticComboData`).
  - Calls `sub_1800AE950`; emits unique attribute-consistency warning for
    combo mismatch debugging.

### Client-side template material shaping

- `client!sub_1813BA1E0`
  - `CS2ItemEditor_BuildTemplateMaterialFromFile`.
  - Reads `.vmat` template content and maps `exposed_param_*` style fields
    (`g_fl*`, `g_v*`, `Texture*`, etc.) into runtime structures.
  - This is parameter/template shaping, not runtime Source2 VFX source compile.

## Current conclusion for custom VFX/VCS work

If the goal is custom Source2 shader graphs/material behavior beyond param edits:

1. Build/obtain compiled assets expected by the resource pipeline.
2. Ensure mount path + extension/type expectations match engine loaders.
3. Use runtime hooks for queue/cache observability and fallback behavior,
   not as the only shader-source ingest mechanism.

## Follow-up findings (Apr 25, 2026, later pass)

### Call-chain resolution deltas

- `materialsystem2!sub_18003A200` (compile-queue driver) callers:
  - `sub_18000C8C0`
  - `sub_180016D10` (`CMaterialSystem2_GetErrorMaterial` path)
  - `sub_1800355C0` (`CMaterialSystem2_DynamicShaderCompile_UnloadAllMaterials` path)
- `materialsystem2!sub_1800BDAE0` is called from
  `CMaterialLayer_ComputeWorkItemsToSetupStaticCombosForMode`.
- `materialsystem2!sub_1800AE950` is called from `sub_1800BDAE0`.

### Resourcesystem evidence for compiled-asset contract mapping

- In `resourcesystem.dll`, a resource-type mapping table in `.rdata` around
  `0x180064640` includes paired entries such as:
  - `offset aMresourcetypef`
  - `offset aVcompmat` (at `0x180064698`)
- The table is linked from typed descriptor-like `.data` entries (example:
  `0x180074BF8`) that also carry tiny type callbacks (example:
  `sub_18002A510`) and `InfoForResourceType...`-style naming strings.
- This supports the current model: extension/type registration is explicit and
  tightly coupled to resource-type handlers, reinforcing that loader-compatible
  compiled assets are the practical path.

## Next reverse targets

1. Add a robust signature for `materialsystem2!sub_1800AE950` (cache gate
  wrapper) once a unique anchor is validated.
2. Name/label `sub_18000C8C0` and `sub_1800355C0` semantically in notes to
  better describe compile enqueue/reload orchestration.
3. Walk code that consumes the resourcesystem descriptor blocks containing
  `aVcompmat` to fully map extension->type handler dispatch.
4. Add a small dumper report mode that logs compile-queue and cache outcomes
  in one run for quick regression checks across builds.
