---
name: find-CBasePlayerController_SetPawn
description: Find and identify the CBasePlayerController_SetPawn function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the SetPawn function by finding CSource2GameClients_ClientDisconnect (via "player_disconnect" + "xuid" string xrefs) and identifying the characteristic call pattern within it.
disable-model-invocation: true
---

# Find CBasePlayerController_SetPawn

Locate `CBasePlayerController_SetPawn` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

### 1. Find CSource2GameClients_ClientDisconnect

Search for strings `"player_disconnect"` and `"xuid"` in IDA, then find the function that references both.

#### 1a. Search for both strings

```
mcp__ida-pro-mcp__find_regex pattern="player_disconnect"
mcp__ida-pro-mcp__find_regex pattern="^xuid$"
```

#### 1b. Get xrefs and find intersection

Get cross-references for each string match and find the function that appears in both xref sets. That function is `CSource2GameClients_ClientDisconnect`.

```
mcp__ida-pro-mcp__xrefs_to addrs="<player_disconnect_addr>"
mcp__ida-pro-mcp__xrefs_to addrs="<xuid_addr>"
```

The intersecting function fires the `player_disconnect` game event with fields: `userid`, `reason`, `name`, `xuid`, `networkid`.

#### 1c. Rename CSource2GameClients_ClientDisconnect (if not already named)

```
mcp__ida-pro-mcp__rename batch={"func": {"addr": "<function_addr>", "name": "CSource2GameClients_ClientDisconnect"}}
```

### 2. Decompile CSource2GameClients_ClientDisconnect

```
mcp__ida-pro-mcp__decompile addr="<CSource2GameClients_ClientDisconnect_addr>"
```

### 3. Identify CBasePlayerController_SetPawn call pattern

In the decompiled code of `CSource2GameClients_ClientDisconnect`, look for this pattern near the end of the function:

```c
      v38 = sub_XXXXXXX(v8);       // get pawn from controller
      if ( v38 )
      {
        sub_YYYYYYY(v8, 0LL, 0, 0, 0, 0);  // <- This is CBasePlayerController_SetPawn
        sub_ZZZZZZZ(v38);                    // remove/destroy the pawn
      }
```

The target function (`sub_YYYYYYY`) is identified by:
- Called with the player controller (`v8` obtained from `UTIL_PlayerSlotToPlayerController`) as first argument
- All remaining 5 arguments are zero (clearing/nullifying the pawn assignment)
- Called inside a null-check block after retrieving the pawn
- Immediately followed by a cleanup/destroy call on the pawn

### 4. Rename the function

```
mcp__ida-pro-mcp__rename batch={"func": {"addr": "<function_addr>", "name": "CBasePlayerController_SetPawn"}}
```

### 5. Generate and validate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function `CBasePlayerController_SetPawn`.

### 6. Write IDA analysis output as YAML beside the binary

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results of `CBasePlayerController_SetPawn`.

Required parameters:
- `func_name`: `CBasePlayerController_SetPawn`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 5

## Function Characteristics

- **Purpose**: Associates or disassociates a pawn with the player controller
- **Parameters**: `(controller, pawn_ptr, param2, param3, param4, param5)`
  - When called with all zeros: clears/nullifies the pawn assignment
- **Called from**: `CSource2GameClients_ClientDisconnect` (player disconnect handler)
- **Function size**: ~0x528 bytes - may vary between versions

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBasePlayerController_SetPawn.windows.yaml`
- `libserver.so` → `CBasePlayerController_SetPawn.linux.yaml`
