---
name: find-IGameSystem_SetGameSystemGlobalPtrs-AND-IGameSystem_dtor
description: |
  Find and identify IGameSystem_SetGameSystemGlobalPtrs and IGameSystem_dtor virtual function calls in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 client.dll or libclient.so to locate both vfunc calls
  by decompiling CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate and identifying the virtual calls through IGameSystem vtable pointer.
  Trigger: IGameSystem_SetGameSystemGlobalPtrs, IGameSystem_dtor
disable-model-invocation: true
---

# Find IGameSystem_SetGameSystemGlobalPtrs and IGameSystem_dtor

Locate `IGameSystem_SetGameSystemGlobalPtrs` and `IGameSystem_dtor` vfunc calls in CS2 client.dll or libclient.so using IDA Pro MCP tools.

## Method

### 1. Get CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate Function Info

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `func_va` for subsequent steps.

### 2. Decompile CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate

```
mcp__ida-pro-mcp__decompile addr="<func_va>"
```

### 3. Identify Both VFunc Offsets from Code Pattern

In the decompiled output, look for the **two consecutive virtual calls through an IGameSystem pointer**:

**Pattern:**
```c
__int64 __fastcall CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate(__int64 a1, __int64 a2)
{
  __int64 v2; // rdi

  v2 = a2 - 8;
  if ( !a2 )
    v2 = 0;
  (*(void (__fastcall **)(__int64, _QWORD))(*(_QWORD *)(v2 + 8) + <SETGLOBALPTRS_VFUNC_OFFSET>))(v2 + 8, 0);// IGameSystem_SetGameSystemGlobalPtrs
  (*(void (__fastcall **)(__int64, _QWORD))(*(_QWORD *)(v2 + 8) + <DTOR_VFUNC_OFFSET>))(v2 + 8, 0);// IGameSystem_dtor
  return (*(__int64 (__fastcall **)(_QWORD, __int64))(*g_pMemAlloc + 24LL))(g_pMemAlloc, v2);
}
```

- `<SETGLOBALPTRS_VFUNC_OFFSET>` (e.g. `464` = `0x1D0`) is the vfunc offset of `IGameSystem_SetGameSystemGlobalPtrs` -- the **first** virtual call through the IGameSystem vtable pointer (`v2 + 8`).
- `<DTOR_VFUNC_OFFSET>` (e.g. `488` = `0x1E8`) is the vfunc offset of `IGameSystem_dtor` -- the **second** virtual call through the IGameSystem vtable pointer (`v2 + 8`).

The two virtual calls are consecutive and both go through the same IGameSystem pointer (`v2 + 8`). The first call is `SetGameSystemGlobalPtrs` and the second is the destructor. Both pass `0` as the second argument.

Extract both `<SETGLOBALPTRS_VFUNC_OFFSET>` and `<DTOR_VFUNC_OFFSET>` from the call sites. Calculate vtable indices:
- `IGameSystem_SetGameSystemGlobalPtrs` index = `<SETGLOBALPTRS_VFUNC_OFFSET> / 8`
- `IGameSystem_dtor` index = `<DTOR_VFUNC_OFFSET> / 8`

### 4. Generate VFunc Offset Signatures

#### 4a. IGameSystem_SetGameSystemGlobalPtrs Signature

Identify the instruction address (`inst_addr`) of the virtual call `call qword ptr [rax+<SETGLOBALPTRS_VFUNC_OFFSET>]` or `call qword ptr [rcx+<SETGLOBALPTRS_VFUNC_OFFSET>]` at the first call site (IGameSystem::SetGameSystemGlobalPtrs).

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `IGameSystem_SetGameSystemGlobalPtrs`, with `inst_addr` and `vfunc_offset` from this step.

#### 4b. IGameSystem_dtor Signature

Identify the instruction address (`inst_addr`) of the virtual call `call qword ptr [rax+<DTOR_VFUNC_OFFSET>]` or `call qword ptr [rcx+<DTOR_VFUNC_OFFSET>]` at the second call site (IGameSystem::~IGameSystem).

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `IGameSystem_dtor`, with `inst_addr` and `vfunc_offset` from this step.

### 5. Write IDA Analysis Output as YAML

#### 5a. Write IGameSystem_SetGameSystemGlobalPtrs YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystem_SetGameSystemGlobalPtrs`
- `func_addr`: `None` (virtual call, actual address resolved at runtime)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 4a

VTable parameters:
- `vtable_name`: `IGameSystem`
- `vfunc_offset`: `<SETGLOBALPTRS_VFUNC_OFFSET>` in hex (e.g. `0x1D0`)
- `vfunc_index`: The calculated index (e.g. `58`)

#### 5b. Write IGameSystem_dtor YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystem_dtor`
- `func_addr`: `None` (virtual call, actual address resolved at runtime)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 4b

VTable parameters:
- `vtable_name`: `IGameSystem`
- `vfunc_offset`: `<DTOR_VFUNC_OFFSET>` in hex (e.g. `0x1E8`)
- `vfunc_index`: The calculated index (e.g. `61`)

## Function Characteristics

### IGameSystem_SetGameSystemGlobalPtrs
- **Purpose**: Sets global pointers for a game system, called during deallocation to clear them (passing 0/NULL)
- **Called from**: `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate` -- the factory deallocator for CSpawnGroupMgrGameSystem
- **Call context**: Called through the IGameSystem vtable pointer with 0 as the second argument, before the destructor call
- **Parameters**: `(this, pGlobalPtrs)` where `this` is the IGameSystem instance pointer and `pGlobalPtrs` is 0 (clearing globals)

### IGameSystem_dtor
- **Purpose**: Virtual destructor for IGameSystem, called to tear down the game system instance
- **Called from**: `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate` -- the factory deallocator for CSpawnGroupMgrGameSystem
- **Call context**: Called through the IGameSystem vtable pointer with 0 as the second argument, after SetGameSystemGlobalPtrs and before the memory free call
- **Parameters**: `(this, flags)` where `this` is the IGameSystem instance pointer and `flags` is 0

## VTable Information

### IGameSystem_SetGameSystemGlobalPtrs
- **VTable Name**: `IGameSystem`
- **VTable Offset**: Changes with game updates. Extract from the `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate` decompiled code.
- **VTable Index**: Changes with game updates. Resolve via `<SETGLOBALPTRS_VFUNC_OFFSET> / 8`.

### IGameSystem_dtor
- **VTable Name**: `IGameSystem`
- **VTable Offset**: Changes with game updates. Extract from the `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate` decompiled code.
- **VTable Index**: Changes with game updates. Resolve via `<DTOR_VFUNC_OFFSET> / 8`.

## Identification Pattern

Both functions are identified by locating the virtual calls inside `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate`:
1. The function receives a parameter `a2` and computes `v2 = a2 - 8` (adjusting for a header/prefix)
2. If `a2` is non-null, two consecutive virtual calls are made through `*(_QWORD *)(v2 + 8)` (the IGameSystem vtable pointer)
3. The first virtual call at offset `<SETGLOBALPTRS_VFUNC_OFFSET>` is `SetGameSystemGlobalPtrs(0)` -- clears global pointers
4. The second virtual call at offset `<DTOR_VFUNC_OFFSET>` is the destructor `~IGameSystem(0)` -- destroys the instance
5. Finally, `g_pMemAlloc->Free(v2)` is called to release memory

This is robust because:
- `CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate` is reliably found via its own skill
- The pattern of two consecutive virtual calls through the same IGameSystem pointer followed by a memory free is distinctive
- The null check on `a2` and the `a2 - 8` offset adjustment are unique to this factory deallocator

## Output YAML Format

The output YAML filenames depend on the platform:
- `client.dll`:
  - `IGameSystem_SetGameSystemGlobalPtrs.windows.yaml`
  - `IGameSystem_dtor.windows.yaml`
- `libclient.so`:
  - `IGameSystem_SetGameSystemGlobalPtrs.linux.yaml`
  - `IGameSystem_dtor.linux.yaml`
