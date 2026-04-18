---
name: find-IGameSystemFactory_Allocate-AND-IGameSystemFactory_HasName-AND-IGameSystem_SetName
description: |
  Find and identify IGameSystemFactory_Allocate, IGameSystemFactory_HasName, and IGameSystem_SetName virtual function calls in CS2 binary using IDA Pro MCP.
  Use this skill when reverse engineering CS2 client.dll or libclient.so to locate all three vfunc calls
  by decompiling IGameSystem_AddByName and identifying the virtual calls through IGameSystemFactory and IGameSystem vtable pointers.
  Trigger: IGameSystemFactory_Allocate, IGameSystemFactory_HasName, IGameSystem_SetName
disable-model-invocation: true
---

# Find IGameSystemFactory_Allocate, IGameSystemFactory_HasName, and IGameSystem_SetName

Locate `IGameSystemFactory_Allocate`, `IGameSystemFactory_HasName`, and `IGameSystem_SetName` vfunc calls in CS2 `client.dll` or `libclient.so` using IDA Pro MCP tools.

## Method

### 1. Get IGameSystem_AddByName Function Info

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=IGameSystem_AddByName`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `func_va` for subsequent steps.

### 2. Decompile IGameSystem_AddByName

```
mcp__ida-pro-mcp__decompile addr="<func_va>"
```

### 3. Identify All Three VFunc Offsets from Code Pattern

In the decompiled output, look for the **three virtual calls** through IGameSystemFactory and IGameSystem pointers:

**Pattern:**
```c
  byte_XXXXXXXXX = 1;
  v6 = (*(__int64 (__fastcall **)(_QWORD))(*(_QWORD *)*v2 + <ALLOCATE_VFUNC_OFFSET>))(*v2);// IGameSystemFactory_Allocate
  byte_XXXXXXXXX = 0;
  v7 = v6;
  if ( (*(unsigned __int8 (__fastcall **)(_QWORD))(*(_QWORD *)*v2 + <HASNAME_VFUNC_OFFSET>))(*v2) )// IGameSystemFactory_HasName
    (*(void (__fastcall **)(__int64, __int64))(*(_QWORD *)v7 + <SETNAME_VFUNC_OFFSET>))(v7, v4);// IGameSystem_SetName
  v8 = *v2;
  *((_BYTE *)v2 + 8) = 1;
  v9 = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v8 + 48LL))(v8);
```

- `*v2` is an `IGameSystemFactory` pointer. Virtual calls through `*(_QWORD *)*v2` access the IGameSystemFactory vtable.
- `v7` (= `v6`, the return value of `Allocate`) is an `IGameSystem` pointer. The virtual call through `*(_QWORD *)v7` accesses the IGameSystem vtable.

- `<ALLOCATE_VFUNC_OFFSET>` (e.g. `24` = `0x18`) is the vfunc offset of `IGameSystemFactory_Allocate` -- the **first** virtual call through the IGameSystemFactory vtable pointer (`*v2`), which allocates a new game system instance.
- `<HASNAME_VFUNC_OFFSET>` (e.g. `64` = `0x40`) is the vfunc offset of `IGameSystemFactory_HasName` -- the **second** virtual call through the IGameSystemFactory vtable pointer (`*v2`), a boolean check.
- `<SETNAME_VFUNC_OFFSET>` (e.g. `472` = `0x1D8`) is the vfunc offset of `IGameSystem_SetName` -- the virtual call through the IGameSystem vtable pointer (`v7`), called conditionally when `HasName` returns true.

Extract all three offsets from the call sites. Calculate vtable indices:
- `IGameSystemFactory_Allocate` index = `<ALLOCATE_VFUNC_OFFSET> / 8`
- `IGameSystemFactory_HasName` index = `<HASNAME_VFUNC_OFFSET> / 8`
- `IGameSystem_SetName` index = `<SETNAME_VFUNC_OFFSET> / 8`

### 4. Generate VFunc Offset Signatures

#### 4a. IGameSystemFactory_Allocate Signature

Identify the instruction address (`inst_addr`) of the virtual call `call qword ptr [rax+<ALLOCATE_VFUNC_OFFSET>]` or `call qword ptr [rcx+<ALLOCATE_VFUNC_OFFSET>]` at the first call site (IGameSystemFactory::Allocate).

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `IGameSystemFactory_Allocate`, with `inst_addr` and `vfunc_offset` from this step.

#### 4b. IGameSystemFactory_HasName Signature

Identify the instruction address (`inst_addr`) of the virtual call `call qword ptr [rax+<HASNAME_VFUNC_OFFSET>]` or `call qword ptr [rcx+<HASNAME_VFUNC_OFFSET>]` at the second call site (IGameSystemFactory::HasName).

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `IGameSystemFactory_HasName`, with `inst_addr` and `vfunc_offset` from this step.

#### 4c. IGameSystem_SetName Signature

Identify the instruction address (`inst_addr`) of the virtual call `call qword ptr [rax+<SETNAME_VFUNC_OFFSET>]` or `call qword ptr [rcx+<SETNAME_VFUNC_OFFSET>]` at the third call site (IGameSystem::SetName).

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for `IGameSystem_SetName`, with `inst_addr` and `vfunc_offset` from this step.

### 5. Write IDA Analysis Output as YAML

#### 5a. Write IGameSystemFactory_Allocate YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystemFactory_Allocate`
- `func_addr`: `None` (virtual call, actual address resolved at runtime)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 4a

VTable parameters:
- `vtable_name`: `IGameSystemFactory`
- `vfunc_offset`: `<ALLOCATE_VFUNC_OFFSET>` in hex (e.g. `0x18`)
- `vfunc_index`: The calculated index (e.g. `3`)

#### 5b. Write IGameSystemFactory_HasName YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystemFactory_HasName`
- `func_addr`: `None` (virtual call, actual address resolved at runtime)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 4b

VTable parameters:
- `vtable_name`: `IGameSystemFactory`
- `vfunc_offset`: `<HASNAME_VFUNC_OFFSET>` in hex (e.g. `0x40`)
- `vfunc_index`: The calculated index (e.g. `8`)

#### 5c. Write IGameSystem_SetName YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystem_SetName`
- `func_addr`: `None` (virtual call, actual address resolved at runtime)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 4c

VTable parameters:
- `vtable_name`: `IGameSystem`
- `vfunc_offset`: `<SETNAME_VFUNC_OFFSET>` in hex (e.g. `0x1D8`)
- `vfunc_index`: The calculated index (e.g. `59`)

## Function Characteristics

### IGameSystemFactory_Allocate
- **Purpose**: Allocates (creates) a new IGameSystem instance from the factory
- **Called from**: `IGameSystem_AddByName` -- called during game system registration to instantiate a game system
- **Call context**: Called through the IGameSystemFactory vtable pointer with the factory pointer as `this`. A global byte flag is set to 1 before the call and reset to 0 after.
- **Parameters**: `(this)` where `this` is the IGameSystemFactory instance pointer
- **Return**: Pointer to the newly allocated IGameSystem instance

### IGameSystemFactory_HasName
- **Purpose**: Boolean check that determines whether the newly allocated game system should have its name set
- **Called from**: `IGameSystem_AddByName` -- called after Allocate to decide whether to call SetName
- **Call context**: Called through the IGameSystemFactory vtable pointer with the factory pointer as `this`. The return value gates whether SetName is called.
- **Parameters**: `(this)` where `this` is the IGameSystemFactory instance pointer
- **Return**: `bool` -- if true, `IGameSystem_SetName` is called on the allocated system

### IGameSystem_SetName
- **Purpose**: Sets the name of a game system instance
- **Called from**: `IGameSystem_AddByName` -- called conditionally (when HasName returns true) to assign the name string to the newly allocated game system
- **Call context**: Called through the IGameSystem vtable pointer on the newly allocated instance, with the name string as the second argument
- **Parameters**: `(this, name)` where `this` is the IGameSystem instance pointer and `name` is the game system name string (passed as `v4` from the function parameter)

## VTable Information

### IGameSystemFactory_Allocate
- **VTable Name**: `IGameSystemFactory`
- **VTable Offset**: Changes with game updates. Extract from the `IGameSystem_AddByName` decompiled code.
- **VTable Index**: Changes with game updates. Resolve via `<ALLOCATE_VFUNC_OFFSET> / 8`.

### IGameSystemFactory_HasName
- **VTable Name**: `IGameSystemFactory`
- **VTable Offset**: Changes with game updates. Extract from the `IGameSystem_AddByName` decompiled code.
- **VTable Index**: Changes with game updates. Resolve via `<HASNAME_VFUNC_OFFSET> / 8`.

### IGameSystem_SetName
- **VTable Name**: `IGameSystem`
- **VTable Offset**: Changes with game updates. Extract from the `IGameSystem_AddByName` decompiled code.
- **VTable Index**: Changes with game updates. Resolve via `<SETNAME_VFUNC_OFFSET> / 8`.

## Identification Pattern

All three functions are identified by locating the virtual calls inside `IGameSystem_AddByName`:
1. A global byte flag is set to 1
2. The first virtual call through `*(_QWORD *)*v2` at `<ALLOCATE_VFUNC_OFFSET>` is `IGameSystemFactory_Allocate` -- allocates a new game system
3. The global byte flag is reset to 0
4. The second virtual call through `*(_QWORD *)*v2` at `<HASNAME_VFUNC_OFFSET>` is `IGameSystemFactory_HasName` -- boolean check
5. If `HasName` returns true, the third virtual call through `*(_QWORD *)v7` (the allocated system) at `<SETNAME_VFUNC_OFFSET>` is `IGameSystem_SetName` -- sets the system's name

This is robust because:
- `IGameSystem_AddByName` is reliably found via its own skill
- The pattern of set-flag → Allocate → clear-flag → HasName → conditional SetName is distinctive
- The two different vtable pointer bases (`*v2` for factory calls vs `v7` for system call) make the pattern unique
- The conditional structure (if HasName then SetName) is a clear marker

## Output YAML Format

The output YAML filenames depend on the platform:
- `client.dll`:
  - `IGameSystemFactory_Allocate.windows.yaml`
  - `IGameSystemFactory_HasName.windows.yaml`
  - `IGameSystem_SetName.windows.yaml`
- `libclient.so`:
  - `IGameSystemFactory_Allocate.linux.yaml`
  - `IGameSystemFactory_HasName.linux.yaml`
  - `IGameSystem_SetName.linux.yaml`
