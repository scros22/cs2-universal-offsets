---
name: find-IGameSystem_LoopPostInitAllSystems_pEventDispatcher-AND-IGameSystem_LoopDestroyAllSystems_s_GameSystems
description: Find and identify the IGameSystem_LoopPostInitAllSystems_pEventDispatcher and IGameSystem_LoopDestroyAllSystems_s_GameSystems global variable in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the event dispatcher pointer by searching for the "IGameSystem::LoopPostInitAllSystems(finish)" string reference and analyzing cross-references.
disable-model-invocation: true
---

# Find IGameSystem_LoopPostInitAllSystems_pEventDispatcher and IGameSystem_LoopDestroyAllSystems_s_GameSystems

Locate `IGameSystem::LoopPostInitAllSystems` (function), `IGameSystem_LoopPostInitAllSystems_pEventDispatcher` (global variable), `IGameSystem_LoopDestroyAllSystems_s_GameSystems` (global variable) in CS2 `server.dll` or `libserver.so` using IDA Pro MCP tools.

## Method

### 1. Search for the debug string

Windows binary (server.dll):

```
mcp__ida-pro-mcp__find_regex pattern="IGameSystem::LoopPostInitAllSystems\(finish\)"
```

Linux binary (libserver.so):

```
mcp__ida-pro-mcp__find_regex pattern="IGameSystem::LoopInitAllSystems\(finish\)"
```

### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

### 3. Decompile the referencing function

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

Windows binary - Verify the function contains the pattern:
```c
COM_TimestampedLog("%s:  IGameSystem::LoopPostInitAllSystems(start)\n", "SV");
// ... initialization logic ...
COM_TimestampedLog("%s:  IGameSystem::LoopPostInitAllSystems(finish)\n", "SV");
```

Linux binary - Verify the function contains the pattern:
```c
COM_TimestampedLog("%s:  IGameSystem::LoopInitAllSystems(start)\n", "SV");
// ... initialization logic ...
COM_TimestampedLog("%s:  IGameSystem::LoopInitAllSystems(finish)\n", "SV");
```

### 4. Rename the function

Windows binary:

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "IGameSystem::LoopPostInitAllSystems"}]}
```

Linux binary :
```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "IGameSystem::LoopInitAllSystems"}]}
```

### 5. Identify the pEventDispatcher global variable

In the decompiled code, look for the characteristic if-block pattern:

Windows Binary:

```c
if ( qword_XXXXXXXX )  // <-- IGameSystem_LoopPostInitAllSystems_pEventDispatcher
{
    if ( dword_XXXXXXXX > v4 )
    {
        v7 = *(int *)(qword_XXXXXXXX + 24LL * v4);
        v8 = qword_XXXXXXXX + 24LL * v4;
        if ( v7 > 0 )
        {
            do
                sub_XXXXXXXX(*(_QWORD *)(*(_QWORD *)(v8 + 8) + 8 * v5++));
            while ( v5 < v7 );
        }
    }
}
v9 = byte_XXXXXXXX;
byte_XXXXXXXX = 0;
COM_TimestampedLog("%s:  IGameSystem::LoopPostInitAllSystems(finish)\n", "SV");
```

Linux Binary:

```c
if ( qword_XXXXXXXX )  // <-- IGameSystem_LoopPostInitAllSystems_pEventDispatcher
{
    if ( (int)qword_XXXXXXXX > v45 )
    {
        v46 = (int *)(qword_XXXXXXXX + 24LL * v45);
        v47 = *v46;
        if ( (int)v47 > 0 )
        {
            v48 = 8 * v47;
            v49 = 0LL;
            do
            {
                v50 = *(_QWORD *)(*((_QWORD *)v46 + 1) + v49);
                v49 += 8LL;
                (*(void (__fastcall **)(__int64, __m128i *))(*(_QWORD *)v50 + 24LL))(v50, &v77);
            }
            while ( v49 != v48 );
        }
    }
}
sub_DC6F20();
v51 = (unsigned __int8)byte_24C5AD9;
byte_24C5AD9 = 0;
sub_97D7E0("%s:  IGameSystem::LoopInitAllSystems(finish)", "SV")
```

The first `qword_XXXXXXXX` checked in the `if` condition (before the event dispatch loop and before the `LoopPostInitAllSystems(finish)` log) is `IGameSystem_LoopPostInitAllSystems_pEventDispatcher`.

### 6. Rename the global variable

```
mcp__ida-pro-mcp__rename batch={"data": {"old": "qword_XXXXXXXX", "new": "IGameSystem_LoopPostInitAllSystems_pEventDispatcher"}}
```

### 7. Generate and validate unique signature for IGameSystem_LoopPostInitAllSystems_pEventDispatcher

**ALWAYS** Use SKILL `/generate-signature-for-globalvar` to generate a robust and unique signature for the global variable for IGameSystem_LoopPostInitAllSystems_pEventDispatcher.

### 8. Write IDA analysis output as YAML beside the binary

**ALWAYS** Use SKILL `/write-globalvar-as-yaml` to write the analysis results of IGameSystem_LoopPostInitAllSystems_pEventDispatcher.

Required parameters:
- `gv_name`: `IGameSystem_LoopPostInitAllSystems_pEventDispatcher`
- `gv_addr`: The global variable address from step 5
- `gv_sig`: The validated signature from step 7
- `gv_sig_va`: The virtual address that signature matches
- `gv_inst_offset`: Offset from signature start to GV-accessing instruction
- `gv_inst_length`: Length of the GV-accessing instruction
- `gv_inst_disp`: Displacement offset within the instruction

### 9. Identify IGameSystem_LoopDestroyAllSystems_s_GameSystems

Cross-reference `IGameSystem_LoopPostInitAllSystems_pEventDispatcher` to find all referencing functions:

```
mcp__ida-pro-mcp__xrefs_to addrs="<pEventDispatcher_addr>"
```

Find the cleanup function that contains `mov cs:IGameSystem_LoopPostInitAllSystems_pEventDispatcher, 0`. Disassemble each xref function and look for this instruction. Then decompile that function and look for the characteristic destroy/cleanup pattern at the end:

Windows binary:

```c
    v7 = dword_XXXXXXXX - 1;  // <-- IGameSystem_LoopDestroyAllSystems_s_GameSystems
    if ( dword_XXXXXXXX - 1 >= 0 )
    {
      v8 = 16LL * (dword_XXXXXXXX - 1);
      while ( 1 )
      {
        (*(void (__fastcall **)(__int64, _QWORD))(*(_QWORD *)v2 + 8LL))(v2, *(_QWORD *)(v8 + qword_XXXXXXXX));
        --v7;
        v8 -= 16LL;
        if ( v7 < 0 )
          break;
        v2 = IGameSystem_LoopPostInitAllSystems_pEventDispatcher;
      }
    }
    IGameSystem_LoopPostInitAllSystems_pEventDispatcher = 0LL;
    byte_XXXXXXXX = 0;
```

Linux binary:

```c
  if ( dword_XXXXXXXX - 1 >= 0 )  // <-- IGameSystem_LoopDestroyAllSystems_s_GameSystems
  {
    v6 = 16LL * (dword_XXXXXXXX - 1);
    v18 = 16 * (dword_XXXXXXXX - (unsigned __int64)(unsigned int)(dword_XXXXXXXX - 1)) - 32;
    do
    {
      v7 = IGameSystem_LoopPostInitAllSystems_pEventDispatcher;
      v8 = *(_QWORD *)(qword_XXXXXXXX + v6);
      v9 = *(__int64 (__fastcall **)())(*(_QWORD *)IGameSystem_LoopPostInitAllSystems_pEventDispatcher + 8LL);
      // ... dispatch logic (may be inlined on Linux) ...
      v6 -= 16LL;
    }
    while ( v18 != v6 );
  }
  IGameSystem_LoopPostInitAllSystems_pEventDispatcher = 0LL;
  byte_XXXXXXXX = 0;
```

The `dword_XXXXXXXX` used as the loop count (`dword_XXXXXXXX - 1 >= 0`) in the backward iteration loop just before `pEventDispatcher` is zeroed is `IGameSystem_LoopDestroyAllSystems_s_GameSystems`.

### 10. Rename the global variable

```
mcp__ida-pro-mcp__rename batch={"data": {"old": "dword_XXXXXXXX", "new": "IGameSystem_LoopDestroyAllSystems_s_GameSystems"}}
```

### 11. Generate and validate unique signature for IGameSystem_LoopDestroyAllSystems_s_GameSystems

**ALWAYS** Use SKILL `/generate-signature-for-globalvar` to generate a robust and unique signature for the global variable for IGameSystem_LoopDestroyAllSystems_s_GameSystems.

Set `target_func` to the cleanup function (the one containing `mov cs:pEventDispatcher, 0`) to ensure the signature comes from a robust context with properly wildcarded operands.

### 12. Write IDA analysis output as YAML for IGameSystem_LoopDestroyAllSystems_s_GameSystems

**ALWAYS** Use SKILL `/write-globalvar-as-yaml` to write the analysis results of IGameSystem_LoopDestroyAllSystems_s_GameSystems.

Required parameters:
- `gv_name`: `IGameSystem_LoopDestroyAllSystems_s_GameSystems`
- `gv_addr`: The global variable address from step 9
- `gv_sig`: The validated signature from step 11
- `gv_sig_va`: The virtual address that signature matches
- `gv_inst_offset`: Offset from signature start to GV-accessing instruction
- `gv_inst_length`: Length of the GV-accessing instruction
- `gv_inst_disp`: Displacement offset within the instruction

## Signature Pattern

The function is identified by the debug string `"%s:  IGameSystem::LoopPostInitAllSystems(finish)\n"`. The global variable is the first qword pointer checked in the if-condition immediately before the event dispatch loop that precedes the `(finish)` log call.

## Function Characteristics

### IGameSystem::LoopPostInitAllSystems

- **Prototype**: `bool IGameSystem::LoopPostInitAllSystems(__int64 a1, __int64 a2)`
- **Return type**: `bool` (returns `true` if no system reported an error)
- **Purpose**: Iterates over all registered IGameSystem instances and calls their PostInit callbacks during server initialization
- **Behavior**:
  1. Logs `IGameSystem::LoopPostInitAllSystems(start)` via `COM_TimestampedLog`
  2. Resolves an event dispatch index (TLS-based on Windows)
  3. If `pEventDispatcher` is valid, iterates over registered systems and dispatches PostInit
  4. Checks error flag and resets it
  5. Logs `IGameSystem::LoopPostInitAllSystems(finish)` via `COM_TimestampedLog`
  6. Returns whether no errors occurred

### IGameSystem_LoopPostInitAllSystems_pEventDispatcher

- **Type**: Global pointer (event dispatcher)
- **Purpose**: Points to the event dispatcher structure used to iterate and invoke PostInit on all registered game systems
- **Access Pattern**: Checked for non-null before the dispatch loop; the associated array at offset +0x18 (24 bytes stride) holds per-system callback entries

### IGameSystem_LoopDestroyAllSystems_s_GameSystems

- **Type**: Global `int` (count of registered game systems)
- **Purpose**: Stores the number of registered game system entries in the system array (16-byte stride)
- **Access Pattern**: Read as a loop bound in the cleanup function; the backward iteration loop uses `s_GameSystems - 1` as the starting index, iterating down to 0, calling the event dispatcher's vtable `+0x8` method for each system entry
- **Identification**: Found in the cleanup function that zeroes `pEventDispatcher`; it is the `dword_XXXXXXXX` used in `dword_XXXXXXXX - 1 >= 0` just before the backward dispatch loop

## DLL Information

- **DLL**: `server.dll` (Windows) / `libserver.so` or `libserver.so` (Linux)

## Notes

- `IGameSystem::LoopPostInitAllSystems` is a regular function, NOT a virtual function
- `IGameSystem_LoopPostInitAllSystems_pEventDispatcher` is a global variable (pointer)
- `IGameSystem_LoopDestroyAllSystems_s_GameSystems` is a global variable (`int`, count of registered game systems)
- The event dispatch loop uses a 24-byte stride array structure
- The cleanup function that zeroes `pEventDispatcher` also contains the backward iteration loop using `s_GameSystems` as the count, with a 16-byte stride system entry array

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `IGameSystem_LoopPostInitAllSystems_pEventDispatcher.windows.yaml`
- `libserver.so` / `libserver.so` → `IGameSystem_LoopPostInitAllSystems_pEventDispatcher.linux.yaml`
- `server.dll` → `IGameSystem_LoopDestroyAllSystems_s_GameSystems.windows.yaml`
- `libserver.so` / `libserver.so` → `IGameSystem_LoopDestroyAllSystems_s_GameSystems.linux.yaml`

### IGameSystem_LoopPostInitAllSystems_pEventDispatcher

```yaml
gv_va: '0x181d80450'     # Global variable virtual address - changes with game updates
gv_rva: '0x1d80450'      # Relative virtual address (VA - image base) - changes with game updates
gv_sig: 48 83 3D ?? ?? ?? ?? ?? 49 8B E8 4C 8B F2 74 ??  # Unique byte signature
gv_sig_va: '0x1804e660b'  # Virtual address where signature matches - changes with game updates
gv_inst_offset: 0         # Always 0 - signature starts at GV-referencing instruction
gv_inst_length: 8         # Instruction length in bytes
gv_inst_disp: 3           # Displacement offset within instruction
```

### IGameSystem_LoopDestroyAllSystems_s_GameSystems

```yaml
gv_va: '0x181b18148'     # Global variable virtual address - changes with game updates
gv_rva: '0x1b18148'      # Relative virtual address (VA - image base) - changes with game updates
gv_sig: 8B 05 ?? ?? ?? ?? 83 E8 ?? 48 63 D8 78 ?? 48 8B FB  # Unique byte signature
gv_sig_va: '0x1804f9f7c'  # Virtual address where signature matches - changes with game updates
gv_inst_offset: 0         # Always 0 - signature starts at GV-referencing instruction
gv_inst_length: 6         # Instruction length in bytes
gv_inst_disp: 2           # Displacement offset within instruction
```
