---
name: find-CCSPlayerController_SwitchTeam
description: Find and identify the CCSPlayerController_SwitchTeam function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the SwitchTeam function by searching for known debug string references and analyzing cross-references.
disable-model-invocation: true
---

# Find CCSPlayerController_SwitchTeam

Locate `CCSPlayerController_SwitchTeam` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the debug string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="SwitchTeam => ChangeBasePlayerTeamAndPendingTeam"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. The string is referenced in an internal function. Get cross-references to that function to find the public wrapper:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<internal_function_addr>"
   ```

4. Look for a small wrapper function (~0x5d bytes) that:
   - Validates team index (checks if team is 2 or 3)
   - Prints error if invalid: `"CCSPlayerPawnBase::SwitchTeam( %d ) - invalid team index.\n"`
   - Calls the internal function if team is valid and different from current team

5. Decompile the wrapper function:
   ```
   mcp__ida-pro-mcp__decompile addr="<wrapper_function_addr>"
   ```

6. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<wrapper_function_addr>", "name": "CCSPlayerController_SwitchTeam"}]}
   ```

7. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

8. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CCSPlayerController_SwitchTeam`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 7

   Note: This is NOT a virtual function, so no vtable parameters are needed.

## Signature Pattern

The function references two debug strings:
1. Internal function: `"%s<%i><%s><%s>" SwitchTeam => ChangeBasePlayerTeamAndPendingTeam =%d , req team %d %.2f \n`
2. Wrapper function: `"CCSPlayerPawnBase::SwitchTeam( %d ) - invalid team index.\n"`

## Function Characteristics

- **Parameters**: `(this, team_id)` where `this` is CCSPlayerController pointer, `team_id` is the target team
- **Validation**: Checks if team_id is 2 (Terrorist) or 3 (Counter-Terrorist)
- **Behavior**: Only calls internal switch if new team differs from current team (offset 0x624)

## Function Structure

```c
__int64 CCSPlayerController_SwitchTeam(__int64 this, unsigned int team_id)
{
  if ( !ValidateTeam(team_id) || team_id - 2 > 1 )
    return PrintError("CCSPlayerPawnBase::SwitchTeam( %d ) - invalid team index.\n", team_id);

  current_team = *(unsigned __int8 *)(this + 0x624);  // m_iTeamNum offset
  if ( team_id != current_team )
    return CCSPlayerController_SwitchTeam_Internal(this, team_id);

  return current_team;
}
```

## Team IDs

- `0`: Unassigned
- `1`: Spectator
- `2`: Terrorist
- `3`: Counter-Terrorist

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CCSPlayerController_SwitchTeam.windows.yaml`
- `libserver.so` → `CCSPlayerController_SwitchTeam.linux.yaml`
