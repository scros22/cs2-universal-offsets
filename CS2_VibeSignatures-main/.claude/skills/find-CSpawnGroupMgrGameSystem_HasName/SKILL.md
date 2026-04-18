---
name: find-CSpawnGroupMgrGameSystem_HasName
description: |
  Find and identify the CSpawnGroupMgrGameSystem_HasName virtual function in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 client.dll or libclient.so to locate CSpawnGroupMgrGameSystem_HasName
  by looking up the CSpawnGroupMgrGameSystem vtable at the slot adjacent to IGameSystem_SetName,
  and verifying that the wrapper delegates to IGameSystemFactory_HasName at the expected vfunc offset.
  Trigger: CSpawnGroupMgrGameSystem_HasName
disable-model-invocation: true
---

# Find CSpawnGroupMgrGameSystem_HasName (via CSpawnGroupMgrGameSystem vtable)

Locate `CSpawnGroupMgrGameSystem_HasName` vfunc in CS2 `client.dll` or `libclient.so` using IDA Pro MCP tools.

## Method

### 1. Load IGameSystem_SetName from YAML

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=IGameSystem_SetName`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vfunc_index` of `IGameSystem_SetName`

### 2. Load IGameSystemFactory_HasName from YAML

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=IGameSystemFactory_HasName`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vfunc_index` of `IGameSystemFactory_HasName`
- `vfunc_offset` of `IGameSystemFactory_HasName`

### 3. Load CSpawnGroupMgrGameSystem VTable from YAML

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CSpawnGroupMgrGameSystem`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract:
- `vtable_numvfunc`
- `vtable_entries`

### 4. Resolve the Adjacent Slot

Compute the candidate slot for `CSpawnGroupMgrGameSystem_HasName`:

- `target_vfunc_index = IGameSystem_SetName.vfunc_index + 1`

Validate that `target_vfunc_index < vtable_numvfunc`, then read:

- `candidate_func_addr = CSpawnGroupMgrGameSystem_vtable[target_vfunc_index]`

This adjacent-slot rule is required because `IGameSystem::HasName` immediately follows `IGameSystem::SetName` in the `IGameSystem` vtable.

### 5. Decompile and Verify the Candidate

Decompile the candidate function:

```text
mcp__ida-pro-mcp__decompile addr="<candidate_func_addr>"
```

The candidate should be a small wrapper that delegates to `IGameSystemFactory::HasName` through the factory vtable. Confirm it matches the following pattern:

#### Windows (`client.dll`)

```c
__int64 CSpawnGroupMgrGameSystem_HasName()
{
  return (*(__int64 (__fastcall **)(__int64 *))(*g_pSpawnGroupManagerGameSystemFactory + <FACTORY_HASNAME_OFFSET>))(g_pSpawnGroupManagerGameSystemFactory);
}
```

Assembly pattern:

```asm
mov     rcx, cs:g_pSpawnGroupManagerGameSystemFactory
mov     rax, [rcx]
jmp     qword ptr [rax+<FACTORY_HASNAME_OFFSET>]
```

#### Linux (`libclient.so`)

```c
__int64 CSpawnGroupMgrGameSystem_HasName()
{
  return (*(__int64 (__fastcall **)(__int64 *))(*g_pSpawnGroupManagerGameSystemFactory + <FACTORY_HASNAME_OFFSET>))(g_pSpawnGroupManagerGameSystemFactory);
}
```

The key verification is:

1. The function is a small wrapper (very few instructions)
2. It loads a global factory pointer (`g_pSpawnGroupManagerGameSystemFactory`)
3. It makes a virtual call through the factory's vtable at offset `<FACTORY_HASNAME_OFFSET>`
4. `<FACTORY_HASNAME_OFFSET>` must equal `IGameSystemFactory_HasName.vfunc_offset` (or equivalently, `IGameSystemFactory_HasName.vfunc_index * 8`)

If the offset does not match, **STOP** and report to user.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CSpawnGroupMgrGameSystem_HasName`
- `func_addr`: `<candidate_func_addr>`
- `func_sig`: `None`
- `vfunc_sig`: Use exact the same `vfunc_sig` from `IGameSystemFactory_HasName`

VTable parameters:
- `vtable_name`: `CSpawnGroupMgrGameSystem`
- `vfunc_offset`: `<target_vfunc_offset>` in hex
- `vfunc_index`: `<target_vfunc_index>`

## Function Characteristics

- **Purpose**: Returns whether the game system has a name, by delegating to the corresponding `IGameSystemFactory::HasName` through the factory pointer
- **Binary**: `client.dll` / `libclient.so`
- **Parameters**: `(this)` only (implicit, the wrapper uses a global factory pointer)
- **Return value**: Boolean — whether the factory reports that this game system has a name

## Discovery Strategy

1. Load the existing `IGameSystem_SetName` YAML to obtain its vtable index
2. Load the existing `IGameSystemFactory_HasName` YAML to obtain the expected factory vfunc offset
3. Load the existing `CSpawnGroupMgrGameSystem_vtable` YAML to resolve the adjacent vtable entry
4. The slot at `IGameSystem_SetName.vfunc_index + 1` in the CSpawnGroupMgrGameSystem vtable is `CSpawnGroupMgrGameSystem_HasName`
5. Verify the wrapper calls through the factory vtable at `IGameSystemFactory_HasName.vfunc_offset`

This is robust because:
- The vtable adjacency (`SetName` followed by `HasName`) is a stable layout property
- The wrapper function pattern (delegate to factory `HasName`) is distinctive
- Cross-checking the factory vfunc offset provides a second independent verification

## Output YAML Format

The output YAML filename depends on the platform:
- `client.dll` -> `CSpawnGroupMgrGameSystem_HasName.windows.yaml`
- `libclient.so` -> `CSpawnGroupMgrGameSystem_HasName.linux.yaml`
