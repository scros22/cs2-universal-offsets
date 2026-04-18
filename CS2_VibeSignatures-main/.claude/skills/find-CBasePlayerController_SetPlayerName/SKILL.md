---
name: find-CBasePlayerController_SetPlayerName
description: Find and identify the CBasePlayerController_SetPlayerName function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the SetPlayerName function by searching for known string references and analyzing cross-references.
disable-model-invocation: true
---

# Find CBasePlayerController_SetPlayerName

Locate `CBasePlayerController_SetPlayerName` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for strings `fov_desired` and `newname`:
   ```
   mcp__ida-pro-mcp__find_regex pattern="fov_desired"
   mcp__ida-pro-mcp__find_regex pattern="newname"
   ```

2. Get cross-references to both strings:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs=["<fov_desired_addr>", "<newname_addr>"]
   ```

3. Find the function that references **both** strings - this is the player info sync function.

4. Decompile that function and look for the call to `CBasePlayerController_SetPlayerName`:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

5. In the decompiled output, find the pattern:
   ```c
   CBasePlayerController_SetPlayerName(a2, v6);  // after name comparison and event firing
   ```

6. Rename if needed:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<target_addr>", "name": "CBasePlayerController_SetPlayerName"}]}
   ```

7. Get function details for YAML:
   ```
   mcp__ida-pro-mcp__lookup_funcs queries="<target_addr>"
   ```

8. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

9. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CBasePlayerController_SetPlayerName`
   - `func_addr`: The function address from step 7
   - `func_sig`: The validated signature from step 8

   Note: This is NOT a virtual function, so no vtable parameters are needed.

## Signature Pattern

The function is called after:
- Creating `CMsgPlayerInfo` message
- Firing `player_changename` event with `userid`, `oldname`, `newname` fields
- Comparing old and new player names

The surrounding function also handles `fov_desired` cvar (clamps FOV between 1-135).

## Function Characteristics

- **Type**: Regular member function (NOT virtual)
- **Parameters**: `(CBasePlayerController* this, const char* name)`
- **Behavior**:
  - Copies player name to `this + 0x510` using `V_strncpy` with max length 128
  - Calls network state change notification

## Hex Signature

| Bytes | Instruction | Description |
|-------|-------------|-------------|
| `41 B8 80 00 00 00` | `mov r8d, 80h` | 128 byte max name length (unique) |
| `48 8D 99 10 05 00 00` | `lea rbx, [rcx+510h]` | Name storage offset 0x510 (unique) |

**Final signature**: `41 B8 80 00 00 00 48 8D 99 10 05 00 00`

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBasePlayerController_SetPlayerName.windows.yaml`
- `libserver.so` → `CBasePlayerController_SetPlayerName.linux.yaml`
