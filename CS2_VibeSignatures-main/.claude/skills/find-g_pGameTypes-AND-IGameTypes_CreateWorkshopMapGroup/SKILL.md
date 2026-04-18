---
name: find-g_pGameTypes-AND-IGameTypes_CreateWorkshopMapGroup
description: Find and identify the g_pGameTypes global variable and IGameTypes_CreateWorkshopMapGroup virtual function call in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the IGameTypes interface pointer by searching for the "mapgroup workshop" string reference and analyzing the virtual function call pattern.
disable-model-invocation: true
---

# Find g_pGameTypes and IGameTypes_CreateWorkshopMapGroup

Locate `g_pGameTypes` (global variable) and `IGameTypes_CreateWorkshopMapGroup` (virtual function offset) in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

### 1. Search for the string

```
mcp__ida-pro-mcp__find_regex pattern="mapgroup workshop"
```

Expected match: `"mapgroup workshop;"` at some address in the binary.

### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

### 3. Decompile the referencing function

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

Verify the function contains the pattern:
```c
(*(void (__fastcall **)(__int64, const char *, __int64 *))(*(_QWORD *)qword_XXXXXXXX + 296LL))(
    qword_XXXXXXXX,      // This is g_pGameTypes
    "workshop",
    &v_local);
sub_XXXXXXXXX((char *)&v_local2 + 4, "mapgroup workshop;");
```

The key identifiers:
- `qword_XXXXXXXX` accessed with a virtual function call at offset `296` (0x128) — this is `g_pGameTypes`
- The vfunc call at offset 0x128 is `IGameTypes::CreateWorkshopMapGroup`, can change on game update

### 4. Disassemble around the call to find exact instruction addresses

```
mcp__ida-pro-mcp__disasm addr="<load_instruction_addr>" max_instructions=15
```

Look for the instruction sequence:
```asm
mov     rcx, cs:qword_XXXXXXXX     ; loads g_pGameTypes
lea     r8, [rsp+...]
lea     rdx, aWorkshop              ; "workshop"
mov     rax, [rcx]
call    qword ptr [rax+128h]        ; IGameTypes::CreateWorkshopMapGroup
lea     rdx, aMapgroupWorksh        ; "mapgroup workshop;"
```

Record:
- The address of `qword_XXXXXXXX` — this is the `g_pGameTypes` global variable
- The address of `call qword ptr [rax+128h]` — this is the vfunc call instruction

### 5. Rename the global variable

```
mcp__ida-pro-mcp__rename batch={"data": {"old": "qword_XXXXXXXX", "new": "g_pGameTypes"}}
```

### 6. Generate signature for g_pGameTypes

**ALWAYS** Use SKILL `/generate-signature-for-globalvar` to generate a robust and unique signature for `g_pGameTypes`.

### 7. Generate signature for IGameTypes_CreateWorkshopMapGroup

**ALWAYS** Use SKILL `/generate-signature-for-vfuncoffset` to generate a robust and unique signature for the vfunc call instruction.

Parameters:
- `inst_addr`: The address of the `call qword ptr [rax+128h]` instruction from step 4
- `vfunc_offset`: `0x128` (296 decimal), can change on game update.

### 8. Write IDA analysis output as YAML

#### For `IGameTypes_CreateWorkshopMapGroup`:

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameTypes_CreateWorkshopMapGroup`
- `func_addr`: `None` (this is a vfunc call site, not a direct function)
- `func_sig`: `None`
- `vfunc_sig`: The validated signature from step 7
- `vtable_name`: `IGameTypes`
- `vfunc_offset`: `0x128`
- `vfunc_index`: `37` (0x128 / 8)

#### For `g_pGameTypes`:

**ALWAYS** Use SKILL `/write-globalvar-as-yaml` to write the analysis results.

Required parameters:
- `gv_name`: `g_pGameTypes`
- `gv_addr`: The global variable address from step 4
- `gv_sig`: The validated signature from step 6
- `gv_sig_va`: The virtual address that signature matches
- `gv_inst_offset`: `0`
- `gv_inst_length`: Length of the GV-accessing instruction
- `gv_inst_disp`: Displacement offset within the instruction

## Signature Pattern

The function contains a workshop map group registration pattern:
```
"workshop"           — passed as the map group type identifier
"mapgroup workshop;" — used to build a console command string
```

## Function / Global Variable Characteristics

### g_pGameTypes

- **Type**: Global pointer (`IGameTypes*`)
- **Purpose**: Singleton interface pointer to the game types system, managing map groups, game modes, and game types
- **Access Pattern**: Typically accessed via `mov rcx, cs:g_pGameTypes` before calling virtual methods through the vtable
- **Virtual Function Table**: Contains methods for managing game types including `CreateWorkshopMapGroup` at offset 0x128

### IGameTypes_CreateWorkshopMapGroup

- **Interface**: `IGameTypes`
- **VTable Offset**: `0x128` (296 decimal)
- **VTable Index**: `37`
- **Purpose**: Creates/registers a workshop map group with the given map paths
- **Call Pattern**: `g_pGameTypes->vtable[37](g_pGameTypes, "workshop", &mapPathArray)`
- **Parameters**:
  1. `this` — `g_pGameTypes` pointer
  2. `const char*` — group type identifier (`"workshop"`)
  3. `CUtlVector*` — array of workshop map path strings

## Output YAML Format

The output YAML filename for IGameTypes_CreateWorkshopMapGroup depends on the platform:
- `server.dll` → `IGameTypes_CreateWorkshopMapGroup.windows.yaml`
- `libserver.so` / `libserver.so` → `IGameTypes_CreateWorkshopMapGroup.linux.yaml`

The output YAML filename for g_pGameTypes depends on the platform:
- `server.dll` → `g_pGameTypes.windows.yaml`
- `libserver.so` / `libserver.so` → `g_pGameTypes.linux.yaml`
