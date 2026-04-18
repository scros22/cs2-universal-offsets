---
name: find-IGameSystem_GetName-AND-IGameSystemFactory_Deallocate
description: |
  Find and identify IGameSystem_GetName and IGameSystemFactory_Deallocate virtual function calls in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 client.dll or libclient.so to locate both vfunc calls
  by decompiling IGameSystem_DestroyAllGameSystems and identifying the virtual calls through IGameSystem and IGameSystemFactory vtable pointers.
  Trigger: IGameSystem_GetName, IGameSystemFactory_Deallocate
disable-model-invocation: true
---

# Find IGameSystem_GetName and IGameSystemFactory_Deallocate

Locate `IGameSystem_GetName` and `IGameSystemFactory_Deallocate` vfunc calls in CS2 client.dll or libclient.so using IDA Pro MCP tools.

## Method

### 1. Get IGameSystem_DestroyAllGameSystems Function Info

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=IGameSystem_DestroyAllGameSystems`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `func_va` for subsequent steps.

### 2. Decompile IGameSystem_DestroyAllGameSystems

```
mcp__ida-pro-mcp__decompile addr="<func_va>"
```

### 3. Identify Both VFunc Offsets from Code Pattern

In the decompiled output, look for the **loop that destroys all game systems** with two virtual calls:

**Pattern:**
```c
    do
    {
      pGameSystem = *(_QWORD *)(v1 + qword_XXXXXXXX);
      v3 = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)pGameSystem + <GETNAME_VFUNC_OFFSET>))(pGameSystem);// IGameSystem::GetName
      CUtlSymbolTable::Find(&unk_XXXXXXXX, &v7, v3);
      v4 = v7;
      if ( v7 == 0xFFFF )
        v4 = -1;
      byte_XXXXXXXX = 1;
      v5 = (_BYTE *)(qword_XXXXXXXX + 16LL * v4);
      (*(void (__fastcall **)(_QWORD, __int64))(**(_QWORD **)v5 + <DEALLOC_VFUNC_OFFSET>))(*(_QWORD *)v5, pGameSystem);// IGameSystemFactory::Deallocate
      ...
    }
    while ( v0 >= 0 );
```

- `<GETNAME_VFUNC_OFFSET>` (e.g. `456` = `0x1C8`) is the vfunc offset of `IGameSystem_GetName` -- the first virtual call through the IGameSystem vtable pointer (`pGameSystem`).
- `<DEALLOC_VFUNC_OFFSET>` (e.g. `32` = `0x20`) is the vfunc offset of `IGameSystemFactory_Deallocate` -- the second virtual call through a double-dereferenced factory pointer (`**v5`).

Extract both `<GETNAME_VFUNC_OFFSET>` and `<DEALLOC_VFUNC_OFFSET>` from the call sites. Calculate vtable indices:
- `IGameSystem_GetName` index = `<GETNAME_VFUNC_OFFSET> / 8`
- `IGameSystemFactory_Deallocate` index = `<DEALLOC_VFUNC_OFFSET> / 8`

### 4. Generate VFunc Offset Signatures

#### 4a. IGameSystem_GetName Signature

Identify the instruction address (`inst_addr`) of the virtual call `call qword ptr [rax+<GETNAME_VFUNC_OFFSET>]` or `call qword ptr [rcx+<GETNAME_VFUNC_OFFSET>]` at the first call site (IGameSystem::GetName).

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `IGameSystem_GetName`, with `inst_addr` and `vfunc_offset` from this step.

#### 4b. IGameSystemFactory_Deallocate Signature

Identify the instruction address (`inst_addr`) of the virtual call `call qword ptr [rax+<DEALLOC_VFUNC_OFFSET>]` or `call qword ptr [rcx+<DEALLOC_VFUNC_OFFSET>]` at the second call site (IGameSystemFactory::Deallocate).

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `IGameSystemFactory_Deallocate`, with `inst_addr` and `vfunc_offset` from this step.

### 5. Write IDA Analysis Output as YAML

#### 5a. Write IGameSystem_GetName YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystem_GetName`
- `func_addr`: `None` (virtual call, actual address resolved at runtime)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 4a

VTable parameters:
- `vtable_name`: `IGameSystem`
- `vfunc_offset`: `<GETNAME_VFUNC_OFFSET>` in hex (e.g. `0x1C8`)
- `vfunc_index`: The calculated index (e.g. `57`)

#### 5b. Write IGameSystemFactory_Deallocate YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystemFactory_Deallocate`
- `func_addr`: `None` (virtual call, actual address resolved at runtime)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 4b

VTable parameters:
- `vtable_name`: `IGameSystemFactory`
- `vfunc_offset`: `<DEALLOC_VFUNC_OFFSET>` in hex (e.g. `0x20`)
- `vfunc_index`: The calculated index (e.g. `4`)

## Function Characteristics

### IGameSystem_GetName
- **Purpose**: Returns the name of an IGameSystem instance as a const char*
- **Called from**: `IGameSystem_DestroyAllGameSystems` -- the function that tears down all registered game systems
- **Call context**: Called through the IGameSystem vtable pointer with no parameters (besides `this`), the return value is passed to `CUtlSymbolTable::Find` to look up the corresponding factory
- **Parameters**: `(this)` where `this` is the IGameSystem instance pointer

### IGameSystemFactory_Deallocate
- **Purpose**: Deallocates a game system instance through its factory
- **Called from**: `IGameSystem_DestroyAllGameSystems` -- the function that tears down all registered game systems
- **Call context**: Called through a double-dereferenced factory pointer (`**v5`) with the IGameSystem pointer as the argument, after the factory is looked up via the system's name
- **Parameters**: `(this, pGameSystem)` where `this` is the IGameSystemFactory instance pointer and `pGameSystem` is the IGameSystem instance to deallocate

## VTable Information

### IGameSystem_GetName
- **VTable Name**: `IGameSystem`
- **VTable Offset**: Changes with game updates. Extract from the `IGameSystem_DestroyAllGameSystems` decompiled code.
- **VTable Index**: Changes with game updates. Resolve via `<GETNAME_VFUNC_OFFSET> / 8`.

### IGameSystemFactory_Deallocate
- **VTable Name**: `IGameSystemFactory`
- **VTable Offset**: Changes with game updates. Extract from the `IGameSystem_DestroyAllGameSystems` decompiled code.
- **VTable Index**: Changes with game updates. Resolve via `<DEALLOC_VFUNC_OFFSET> / 8`.

## Identification Pattern

Both functions are identified by locating the virtual calls inside `IGameSystem_DestroyAllGameSystems`:
1. The function iterates backward over all registered game systems
2. For each system, it calls `IGameSystem::GetName()` through the system's vtable at offset `<GETNAME_VFUNC_OFFSET>`
3. The name is passed to `CUtlSymbolTable::Find` to look up the factory index
4. The factory pointer is double-dereferenced (`**v5`), and `DeallocateGameSystem` is called at offset `<DEALLOC_VFUNC_OFFSET>`
5. A global byte flag is toggled around the deallocation call

This is robust because:
- `IGameSystem_DestroyAllGameSystems` is reliably found via its own skill
- The pattern of calling GetName then looking up a factory by symbol name then deallocating is unique
- The loop structure with backward iteration and the `0xFFFF` sentinel check is distinctive

## Output YAML Format

The output YAML filenames depend on the platform:
- `client.dll`:
  - `IGameSystem_GetName.windows.yaml`
  - `IGameSystemFactory_Deallocate.windows.yaml`
- `libclient.so`:
  - `IGameSystem_GetName.linux.yaml`
  - `IGameSystemFactory_Deallocate.linux.yaml`
