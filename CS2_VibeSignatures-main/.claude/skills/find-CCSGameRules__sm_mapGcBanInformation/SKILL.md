---
name: find-CCSGameRules__sm_mapGcBanInformation
description: Find and identify the CCSGameRules__sm_mapGcBanInformation global variable in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the GC ban information map structure by decompiling CCSPlayerController_ResourceDataThink's sub-function (Linux) or searching for the "Notification about user penalty" string (Windows).
disable-model-invocation: true
---

# Find CCSGameRules__sm_mapGcBanInformation

Locate `CCSGameRules__sm_mapGcBanInformation` (global variable) in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Prerequisites

- `CCSPlayerController_ResourceDataThink` must be identified first (Linux method depends on it)
- Use SKILL `/get-func-from-yaml` to load `CCSPlayerController_ResourceDataThink` if its YAML already exists

## Method

### Windows: String-based approach

#### 1. Search for the debug string

```
mcp__ida-pro-mcp__find_regex pattern="Notification about user penalty: %u/%u"
```

#### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

#### 3. Decompile the referencing function

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

#### 4. Identify the global variable

Look for the pattern after the `DevMsg("Notification about user penalty: %u/%u (%u sec)\n", ...)` call:

```c
sub_XXXXXXXX(&unk_YYYYYYYY, &v10, &v12);
```

Where `unk_YYYYYYYY` (the second argument, offset by +8 from the base) is `CCSGameRules__sm_mapGcBanInformation`. The base address is `unk_YYYYYYYY - 8`.

More specifically, look for:
```c
v12 = &unk_181E4C960;       // unk_181E4C960 is CCSGameRules__sm_mapGcBanInformation - 8
LOBYTE(v13) = 1;
v14 = &v17;
v18 = 0LL;
v19 = 0LL;
sub_XXXXXXXX(&unk_181E4C968, &v10, &v12);  // unk_181E4C968 is CCSGameRules__sm_mapGcBanInformation
```

The global variable to rename is the one passed as the first argument to the map lookup function (e.g., `unk_181E4C968`).

### Linux: Decompilation-based approach

#### 1. Load CCSPlayerController_ResourceDataThink address

Use SKILL `/get-func-from-yaml` to get the address, or decompile it directly:

```
mcp__ida-pro-mcp__decompile addr="<CCSPlayerController_ResourceDataThink_addr>"
```

#### 2. Decompile the sub-function called inside ResourceDataThink

The wrapper function calls a sub-function as its second operation:

```c
__int64 __fastcall CCSPlayerController_ResourceDataThink(__int64 a1, ...)
{
  ++*(_DWORD *)(a1 + 3084);
  sub_YYYYYY(a1, ...);    // <--- decompile this function
  ...
}
```

```
mcp__ida-pro-mcp__decompile addr="<sub_function_addr>"
```

#### 3. Identify the global variable

In the decompiled sub-function, look for the pattern:

```c
v43 = *((int *)&unk_XXXXXXXX + 6)
```

or equivalently:

```c
if ( !*v26 || (v42 = sub_A85C10(v7), v43 = *((int *)&unk_XXXXXXXX + 6), (_DWORD)v43 == -1) )
```

Where `unk_XXXXXXXX` is `CCSGameRules__sm_mapGcBanInformation`.

Additional verification — the same variable appears in the tree traversal loop:

```c
if ( (*((_DWORD *)&unk_XXXXXXXX + 3) & 0x7FFFFFFF) != 0 )
{
    v44 = *((_QWORD *)&unk_XXXXXXXX + 2);
    do
    {
      v45 = 56 * v43;
      v46 = (int *)(v44 + v45);
      ...
    }
    while ( (_DWORD)v43 != -1 );
}
```

### Common steps (both platforms)

#### 5. Rename the global variable

```
mcp__ida-pro-mcp__rename batch={"data": {"old": "<unk_XXXXXXXX>", "new": "CCSGameRules__sm_mapGcBanInformation"}}
```

#### 6. Generate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-globalvar` to generate a robust and unique signature for `CCSGameRules__sm_mapGcBanInformation`.

#### 7. Write IDA analysis output as YAML

**ALWAYS** Use SKILL `/write-globalvar-as-yaml` to write the analysis results.

Required parameters:
- `gv_name`: `CCSGameRules__sm_mapGcBanInformation`
- `gv_addr`: The global variable address from step 3/4
- `gv_sig`: The validated signature from step 6
- `gv_sig_va`: The virtual address that signature matches
- `gv_inst_offset`: Offset from signature start to GV-accessing instruction
- `gv_inst_length`: Length of the GV-accessing instruction
- `gv_inst_disp`: Displacement offset within the instruction

## Global Variable Characteristics

- **Type**: Map/tree structure (CUtlMap or similar)
- **Purpose**: Stores GC (Game Coordinator) ban information per player, used during resource data updates
- **Usage**: Queried during `CCSPlayerController_ResourceDataThink` to check player penalty status
- **Structure fields accessed**:
  - `+0x0C` (`+3` as DWORD): Node count / flags (masked with `0x7FFFFFFF`)
  - `+0x10` (`+2` as QWORD): Pointer to tree node array
  - `+0x18` (`+6` as int): Root node index (or `-1` if empty)
  - Each node is 56 bytes with key at offset +16 and data at offset +48

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSGameRules__sm_mapGcBanInformation.windows.yaml`
- `libserver.so` / `libserver.so` → `CCSGameRules__sm_mapGcBanInformation.linux.yaml`

## Notes

- This is a global variable, NOT a function — use `/write-globalvar-as-yaml`, not `/write-func-as-yaml`
- The variable name uses double underscore (`__`) to represent the `::` scope separator
- On Windows, the string "Notification about user penalty" provides a direct path to the variable
- On Linux, the variable must be found through decompilation of the ResourceDataThink sub-function
