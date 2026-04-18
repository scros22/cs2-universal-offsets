---
name: find-CGamePlayerEquip_InputTriggerForAllPlayers-AND-CGamePlayerEquip_InputTriggerForActivatedPlayer
description: Find and identify the CGamePlayerEquip_InputTriggerForAllPlayers and CGamePlayerEquip_InputTriggerForActivatedPlayer functions in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate these input handlers by searching for their string pointers in the data section and resolving the function pointers at string_ptr + 0x10.
disable-model-invocation: true
---

# Find CGamePlayerEquip_InputTriggerForAllPlayers and CGamePlayerEquip_InputTriggerForActivatedPlayer

Locate both `CGamePlayerEquip_InputTriggerForAllPlayers` and `CGamePlayerEquip_InputTriggerForActivatedPlayer` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

Both functions use the same string-pointer + offset pattern in the `.data` section.

## Method

### Part A: CGamePlayerEquip_InputTriggerForAllPlayers

#### 1. Search for the string

```
mcp__ida-pro-mcp__find_regex pattern="TriggerForAllPlayers"
```

#### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

Use the exact match `"TriggerForAllPlayers"` (not `"InputTriggerForAllPlayers"`).

#### 3. Identify the function pointer from the data section

Look for a cross-reference in the `.data` section (not in code). The data layout is:

**Windows:**
```
.data:XXXXXXXX     dq offset aTriggerforallp ; "TriggerForAllPlayers"
.data:XXXXXXXX+8   align 10h
.data:XXXXXXXX+10  dq offset sub_YYYYYYYY    ; <-- CGamePlayerEquip_InputTriggerForAllPlayers
```

**Linux:**
```
.data:XXXXXXXX     dq offset aTriggerforallp ; "TriggerForAllPlayers"
.data:XXXXXXXX+8   dq 0
.data:XXXXXXXX+10  dq offset sub_YYYYYYYY    ; <-- CGamePlayerEquip_InputTriggerForAllPlayers
```

Read the function pointer at `string_ptr_addr + 0x10`:

```
mcp__ida-pro-mcp__get_int queries={"addr": "<string_ptr_addr + 0x10>", "ty": "u64"}
```

#### 4. Decompile and verify the function

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

Verify this is an entity input handler that iterates over all players and triggers equipment for each.

#### 5. Rename the function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CGamePlayerEquip_InputTriggerForAllPlayers"}]}
```

#### 6. Generate and validate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

#### 7. Write IDA analysis output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CGamePlayerEquip_InputTriggerForAllPlayers`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 6

---

### Part B: CGamePlayerEquip_InputTriggerForActivatedPlayer

#### 1. Search for the string

```
mcp__ida-pro-mcp__find_regex pattern="TriggerForActivatedPlayer"
```

#### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

Use the exact match `"TriggerForActivatedPlayer"` (not `"InputTriggerForActivatedPlayer"`).

#### 3. Identify the function pointer from the data section

Same pattern as Part A. The data layout is:

**Windows:**
```
.data:XXXXXXXX     dq offset aTriggerforacti ; "TriggerForActivatedPlayer"
.data:XXXXXXXX+8   align / padding
.data:XXXXXXXX+10  dq offset sub_ZZZZZZZZ    ; <-- CGamePlayerEquip_InputTriggerForActivatedPlayer
```

**Linux:**
```
.data:XXXXXXXX     dq offset aTriggerforacti ; "TriggerForActivatedPlayer"
.data:XXXXXXXX+8   dq 0
.data:XXXXXXXX+10  dq offset sub_ZZZZZZZZ    ; <-- CGamePlayerEquip_InputTriggerForActivatedPlayer
```

Read the function pointer at `string_ptr_addr + 0x10`:

```
mcp__ida-pro-mcp__get_int queries={"addr": "<string_ptr_addr + 0x10>", "ty": "u64"}
```

#### 4. Decompile and verify the function

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

Verify this is an entity input handler that triggers equipment for the activated player (single player, not all).

#### 5. Rename the function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CGamePlayerEquip_InputTriggerForActivatedPlayer"}]}
```

#### 6. Generate and validate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

#### 7. Write IDA analysis output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CGamePlayerEquip_InputTriggerForActivatedPlayer`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 6

## String Reference Pattern

Both functions are located by finding entity input descriptors in the `.data` section:
- String pointer at offset +0x00
- Padding/alignment at offset +0x08
- Function pointer at offset +0x10

| String | Function |
|--------|----------|
| `"TriggerForAllPlayers"` | `CGamePlayerEquip_InputTriggerForAllPlayers` |
| `"TriggerForActivatedPlayer"` | `CGamePlayerEquip_InputTriggerForActivatedPlayer` |

## Function Characteristics

- **Type**: Entity input handlers for CGamePlayerEquip
- **Class**: CGamePlayerEquip (game_player_equip entity)
- `InputTriggerForAllPlayers`: Triggers equipment distribution to all players
- `InputTriggerForActivatedPlayer`: Triggers equipment distribution to the activated player only

## Output YAML Format

The output YAML filenames depend on the platform:
- `server.dll` → `CGamePlayerEquip_InputTriggerForAllPlayers.windows.yaml`, `CGamePlayerEquip_InputTriggerForActivatedPlayer.windows.yaml`
- `libserver.so` / `libserver.so` → `CGamePlayerEquip_InputTriggerForAllPlayers.linux.yaml`, `CGamePlayerEquip_InputTriggerForActivatedPlayer.linux.yaml`
