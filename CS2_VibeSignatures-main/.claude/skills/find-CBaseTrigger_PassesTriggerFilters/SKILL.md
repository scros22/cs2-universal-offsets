---
name: find-CBaseTrigger_PassesTriggerFilters
description: Find and identify the CBaseTrigger_PassesTriggerFilters virtual function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the PassesTriggerFilters function by first finding CBaseTrigger_StartTouch and extracting the first virtual call offset from its decompiled code.
disable-model-invocation: true
---

# Find CBaseTrigger_PassesTriggerFilters

Locate `CBaseTrigger_PassesTriggerFilters` in CS2 `server.dll` or `libserver.so` using IDA Pro MCP tools.

## Method

### 1. Get CBaseTrigger_StartTouch Function Info

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CBaseTrigger_StartTouch`.

If the skill returns an error, stop and report to user.

Otherwise, extract `func_va` for the next step.

### 2. Decompile CBaseTrigger_StartTouch and Extract VTable Offset

Decompile `CBaseTrigger_StartTouch` using `func_va` from step 1:

```
mcp__ida-pro-mcp__decompile addr="<func_va>"
```

Look for the **first virtual call** in the decompiled code:

```c
__int64 __fastcall CBaseTrigger_StartTouch(__int64 a1, _QWORD *a2)
{
  //First virtual call - CBaseTrigger_PassesTriggerFilters
  result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)a1 + 2128LL))(a1);
```

Extract the vtable offset from this first virtual call (e.g., `2128`). This is the `CBaseTrigger_PassesTriggerFilters` vtable offset.

Calculate:
- **VTable Offset**: The value from the pattern (e.g., 2128 = 0x850)
- **VTable Index**: offset / 8 (e.g., 2128 / 8 = **266**)

Also note the `inst_addr` — the address of this virtual call instruction (shown as a comment like `/*0x1803c8241*/` in the decompiled output).

### 3. Generate vfunc offset signature for CBaseTrigger_PassesTriggerFilters

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `CBaseTrigger_PassesTriggerFilters`, with `inst_addr` and `vfunc_offset` from step 2.

### 4. Write Analysis Results as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results for `CBaseTrigger_PassesTriggerFilters`.

Required parameters:
- `func_name`: `CBaseTrigger_PassesTriggerFilters`
- `func_addr`: `None` (this is a virtual call target, not a directly resolved function)
- `func_sig`: `None`
- `vfunc_sig`: `<vfunc_sig>` from step 3

VTable parameters:
- `vtable_name`: `CBaseTrigger`
- `vfunc_index`: The vtable index from step 2 (e.g., 266)
- `vfunc_offset`: The vtable offset from step 2 (e.g., 0x850)

## VTable Information

- **VTable Name**: `CBaseTrigger::`vftable'`
- **VTable Mangled Name (Windows)**: `??_7CBaseTrigger@@6B@`
- **VTable Mangled Name (Linux)**: `_ZTV12CBaseTrigger`
- **VTable Index**: 266 - This can change when game updates.
- **VTable Offset**: 0x850 (266 * 8 = 2128) - This can change when game updates.

* Note that for `libserver.so`, the first 16 bytes of "vftable" are for RTTI. The real vftable = `_ZTV12CBaseTrigger` + `0x10`.

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBaseTrigger_PassesTriggerFilters.windows.yaml`
- `libserver.so` → `CBaseTrigger_PassesTriggerFilters.linux.yaml`

```yaml
func_name: CBaseTrigger_PassesTriggerFilters
vfunc_sig: FF 90 50 08 00 00 84 C0 0F 84 ?? ?? ?? ?? 4C 89 7C 24 ??
vtable_name: CBaseTrigger
vfunc_offset: 0x850
vfunc_index: 266
```

## DLL Information

- **DLL**: `server.dll` (Windows) / `libserver.so` (Linux)

## Notes

- `CBaseTrigger_PassesTriggerFilters` is a virtual function called as the first check inside `CBaseTrigger_StartTouch`. It determines whether the touching entity passes the trigger's filter criteria before processing the touch event.
- The function is identified indirectly through the virtual call in `CBaseTrigger_StartTouch`, so `func_va`, `func_rva`, `func_size`, and `func_sig` are not included in the YAML output.
- The `vfunc_sig` captures the virtual call instruction pattern including the displacement bytes for the vtable offset.
