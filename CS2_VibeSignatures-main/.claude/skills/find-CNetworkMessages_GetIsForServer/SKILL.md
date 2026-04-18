---
name: find-CNetworkMessages_GetIsForServer
description: |
  Find and identify the CNetworkMessages_GetIsForServer virtual function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 networksystem.dll or libnetworksystem.so to locate the GetIsForServer vfunc
  by reusing the known CNetworkMessages_SetIsForServer slot and checking the adjacent CNetworkMessages vtable entry.
  Trigger: CNetworkMessages_GetIsForServer
disable-model-invocation: true
---

# Find CNetworkMessages_GetIsForServer

Locate `CNetworkMessages_GetIsForServer` vfunc in CS2 `networksystem.dll` or `libnetworksystem.so` using IDA Pro MCP tools.

## Method

### 1. Load CNetworkMessages_SetIsForServer from YAML

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CNetworkMessages_SetIsForServer`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `func_va` of `CNetworkMessages_SetIsForServer`
- `vfunc_index`
- `vfunc_offset`

### 2. Load CNetworkMessages VTable from YAML

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CNetworkMessages`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vtable_numvfunc`
- `vtable_entries`

### 3. Resolve the Adjacent Getter Slot

Compute the candidate slot for `CNetworkMessages_GetIsForServer`:

- `target_vfunc_index = CNetworkMessages_SetIsForServer.vfunc_index + 1`
- `target_vfunc_offset = CNetworkMessages_SetIsForServer.vfunc_offset + 8`

Validate that `target_vfunc_index < vtable_numvfunc`, then read:

- `candidate_func_addr = CNetworkMessages_vtable[target_vfunc_index]`

This adjacent-slot rule is required because `GetIsForServer` immediately follows `SetIsForServer` in the `CNetworkMessages` vtable for `networksystem.dll` / `libnetworksystem.so`.

### 4. Decompile the Setter and Getter Candidate

Decompile both functions:

```text
mcp__ida-pro-mcp__decompile addr="<CNetworkMessages_SetIsForServer_func_va>"
mcp__ida-pro-mcp__decompile addr="<candidate_func_addr>"
```

First, confirm `CNetworkMessages_SetIsForServer` is the simple byte setter pattern:

```c
void __fastcall CNetworkMessages_SetIsForServer(__int64 a1, char a2)
{
  *(_BYTE *)(a1 + 1374) = a2;
}
```

Windows assembly example:

```asm
mov     [rcx+55Eh], dl
retn
```

Then confirm the adjacent vtable entry is the matching byte getter using the **same member offset**:

```c
__int64 __fastcall sub_1800CB790(__int64 a1)
{
  return *(unsigned __int8 *)(a1 + 1374);
}
```

Windows assembly example:

```asm
movzx   eax, byte ptr [rcx+55Eh]
retn
```

The exact member offset may change across game updates. The identification rule is:

1. `SetIsForServer` writes one byte to `this + <member_offset>`
2. `candidate_func_addr` is the next vtable entry (`index + 1`)
3. The candidate reads and returns one unsigned byte from the **same** `this + <member_offset>`

If all three conditions hold, the candidate is `CNetworkMessages_GetIsForServer`.

### 5. Generate Function Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` with `addr=<candidate_func_addr>` to generate a robust and unique `func_sig` for `CNetworkMessages_GetIsForServer`.

Use the returned validated `func_sig` in the next step.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CNetworkMessages_GetIsForServer`
- `func_addr`: `<candidate_func_addr>`
- `func_sig`: The validated signature from step 5
- `vfunc_sig`: `None`

VTable parameters:
- `vtable_name`: `CNetworkMessages`
- `vfunc_offset`: `<target_vfunc_offset>` in hex
- `vfunc_index`: `<target_vfunc_index>`

## Function Characteristics

- **Purpose**: Returns whether the `CNetworkMessages` instance is configured for server-side behavior
- **Binary**: `networksystem.dll` / `libnetworksystem.so`
- **Parameters**: `(this)` only
- **Return value**: An unsigned byte / boolean flag loaded from the same member written by `CNetworkMessages_SetIsForServer`

## Discovery Strategy

1. Reuse the existing `CNetworkMessages_SetIsForServer` YAML to obtain the authoritative slot index
2. Reuse the existing `CNetworkMessages_vtable` YAML to resolve the adjacent vtable entry
3. Confirm the semantic pair:
   - setter writes `this + <member_offset>`
   - adjacent getter returns `this + <member_offset>`
4. Generate a stable `func_sig` from the resolved getter body

This is robust because:
- The vtable adjacency (`SetIsForServer` followed by `GetIsForServer`) is stable and explicit
- The setter/getter pair must touch the same byte member
- The final YAML stores both the resolved function signature and the precise vtable metadata

## Output YAML Format

The output YAML filename depends on the platform:
- `networksystem.dll` -> `CNetworkMessages_GetIsForServer.windows.yaml`
- `libnetworksystem.so` -> `CNetworkMessages_GetIsForServer.linux.yaml`
