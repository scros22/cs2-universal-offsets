---
name: find-CNetworkMessages_AllowAdditionalMessageRegistration-AND-CNetworkMessages_IsAdditionalMessageRegistrationAllowed
description: |
  Find and identify CNetworkMessages_AllowAdditionalMessageRegistration and CNetworkMessages_IsAdditionalMessageRegistrationAllowed
  virtual functions in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 networksystem.dll or
  libnetworksystem.so to locate both vfuncs by reusing the known CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal
  slot and checking the two adjacent CNetworkMessages vtable entries.
  Trigger: CNetworkMessages_AllowAdditionalMessageRegistration, CNetworkMessages_IsAdditionalMessageRegistrationAllowed
disable-model-invocation: true
---

# Find CNetworkMessages_AllowAdditionalMessageRegistration and CNetworkMessages_IsAdditionalMessageRegistrationAllowed

Locate `CNetworkMessages_AllowAdditionalMessageRegistration` and `CNetworkMessages_IsAdditionalMessageRegistrationAllowed` vfuncs in CS2 `networksystem.dll` or `libnetworksystem.so` using IDA Pro MCP tools.

## Method

### 1. Load CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal from YAML

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vfunc_index` of `CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal`
- `vfunc_offset` of `CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal`

### 2. Load CNetworkMessages VTable from YAML

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CNetworkMessages`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vtable_numvfunc`
- `vtable_entries`

### 3. Resolve the Two Adjacent Slots

Compute the candidate slots:

- `allow_vfunc_index = CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.vfunc_index + 1`
- `allow_vfunc_offset = CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.vfunc_offset + 8`
- `isallowed_vfunc_index = CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.vfunc_index + 2`
- `isallowed_vfunc_offset = CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal.vfunc_offset + 16`

Validate that `isallowed_vfunc_index < vtable_numvfunc`, then read:

- `allow_func_addr = CNetworkMessages_vtable[allow_vfunc_index]`
- `isallowed_func_addr = CNetworkMessages_vtable[isallowed_vfunc_index]`

This adjacent-slot rule is required because `AllowAdditionalMessageRegistration` and `IsAdditionalMessageRegistrationAllowed` immediately follow `RegisterNetworkFieldChangeCallbackInternal` in the `CNetworkMessages` vtable.

### 4. Decompile Both Candidate Functions

Decompile both candidates:

```text
mcp__ida-pro-mcp__decompile addr="<allow_func_addr>"
mcp__ida-pro-mcp__decompile addr="<isallowed_func_addr>"
```

Confirm `allow_func_addr` is the simple byte setter pattern:

```c
void __fastcall sub_18009B330(__int64 a1, char a2)
{
  *(_BYTE *)(a1 + 1372) = a2;
}
```

Windows assembly example:

```asm
mov     [rcx+55Ch], dl
retn
```

Then confirm `isallowed_func_addr` is the matching byte getter using the **same member offset**:

```c
__int64 __fastcall sub_18009B340(__int64 a1)
{
  return *(unsigned __int8 *)(a1 + 1372);
}
```

Windows assembly example:

```asm
movzx   eax, byte ptr [rcx+55Ch]
retn
```

The exact member offset may change across game updates. The identification rule is:

1. `allow_func_addr` writes one byte to `this + <member_offset>` (setter pattern)
2. `isallowed_func_addr` is the next vtable entry (`allow_vfunc_index + 1`)
3. The candidate reads and returns one unsigned byte from the **same** `this + <member_offset>`

If both conditions hold, the candidates are `CNetworkMessages_AllowAdditionalMessageRegistration` and `CNetworkMessages_IsAdditionalMessageRegistrationAllowed`.

### 5. Generate Function Signatures

**ALWAYS** Use SKILL `/generate-signature-for-function` with `addr=<allow_func_addr>` to generate a robust and unique `func_sig` for `CNetworkMessages_AllowAdditionalMessageRegistration`.

**ALWAYS** Use SKILL `/generate-signature-for-function` with `addr=<isallowed_func_addr>` to generate a robust and unique `func_sig` for `CNetworkMessages_IsAdditionalMessageRegistrationAllowed`.

Use the returned validated `func_sig` values in the next steps.

### 6. Write IDA Analysis Output as YAML for AllowAdditionalMessageRegistration

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CNetworkMessages_AllowAdditionalMessageRegistration`
- `func_addr`: `<allow_func_addr>`
- `func_sig`: The validated signature from step 5
- `vfunc_sig`: `None`

VTable parameters:
- `vtable_name`: `CNetworkMessages`
- `vfunc_offset`: `<allow_vfunc_offset>` in hex
- `vfunc_index`: `<allow_vfunc_index>`

### 7. Write IDA Analysis Output as YAML for IsAdditionalMessageRegistrationAllowed

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CNetworkMessages_IsAdditionalMessageRegistrationAllowed`
- `func_addr`: `<isallowed_func_addr>`
- `func_sig`: The validated signature from step 5
- `vfunc_sig`: `None`

VTable parameters:
- `vtable_name`: `CNetworkMessages`
- `vfunc_offset`: `<isallowed_vfunc_offset>` in hex
- `vfunc_index`: `<isallowed_vfunc_index>`

## Function Characteristics

### CNetworkMessages_AllowAdditionalMessageRegistration

- **Purpose**: Sets whether additional network message registration is allowed on the `CNetworkMessages` instance
- **Binary**: `networksystem.dll` / `libnetworksystem.so`
- **Parameters**: `(this, bool bAllow)`
- **Return value**: `void`

### CNetworkMessages_IsAdditionalMessageRegistrationAllowed

- **Purpose**: Returns whether additional network message registration is currently allowed
- **Binary**: `networksystem.dll` / `libnetworksystem.so`
- **Parameters**: `(this)` only
- **Return value**: An unsigned byte / boolean flag loaded from the same member written by `CNetworkMessages_AllowAdditionalMessageRegistration`

## Discovery Strategy

1. Reuse the existing `CNetworkMessages_RegisterNetworkFieldChangeCallbackInternal` YAML to obtain the authoritative slot index
2. Reuse the existing `CNetworkMessages_vtable` YAML to resolve the two adjacent vtable entries
3. Confirm the semantic pair:
   - setter writes `this + <member_offset>`
   - adjacent getter returns `this + <member_offset>`
4. Generate stable `func_sig` values from the resolved function bodies

This is robust because:
- The vtable adjacency (`RegisterNetworkFieldChangeCallbackInternal` followed by `AllowAdditionalMessageRegistration` then `IsAdditionalMessageRegistrationAllowed`) is stable and explicit
- The setter/getter pair must touch the same byte member
- The final YAMLs store both the resolved function signatures and the precise vtable metadata

## Output YAML Format

The output YAML filenames depend on the platform:
- `networksystem.dll` -> `CNetworkMessages_AllowAdditionalMessageRegistration.windows.yaml`, `CNetworkMessages_IsAdditionalMessageRegistrationAllowed.windows.yaml`
- `libnetworksystem.so` -> `CNetworkMessages_AllowAdditionalMessageRegistration.linux.yaml`, `CNetworkMessages_IsAdditionalMessageRegistrationAllowed.linux.yaml`
