---
name: find-ClientPrint
description: Find and identify the ClientPrint function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the ClientPrint function by searching for known localization string references like "#Player_Cash_Award_ExplainSuicide_TeammateGotCash" and analyzing cross-references to find the print function.
disable-model-invocation: true
---

# Find ClientPrint

Locate `ClientPrint` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the localization string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="#Player_Cash_Award_ExplainSuicide_TeammateGotCash"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function to find the ClientPrint call:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify the ClientPrint function:
   - Look for a call that takes the localization string as a parameter
   - The function signature pattern: `ClientPrint(recipient_filter, msg_type, localization_string, param1, param2, param3, param4)`
   - In the decompiled code, look for calls like:
     ```c
     sub_XXXXXX((__int64)v100, 3, "#Player_Cash_Award_ExplainSuicide_TeammateGotCash", v54, v90, v89, 0);
     ```

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "ClientPrint"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `ClientPrint`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## String References

The function is commonly called with these localization strings:
- `#Player_Cash_Award_ExplainSuicide_TeammateGotCash`
- `#Player_Cash_Award_ExplainSuicide_EnemyGotCash`
- `#Player_Cash_Award_ExplainSuicide_Spectators`
- `#Player_Cash_Award_ExplainSuicide_YouGotCash` (used by similar function)

## Function Characteristics

- **Parameters**:
  - `arg0`: Recipient filter (player/team filter object)
  - `arg1`: Message type (int, typically 3 for HUD_PRINTTALK)
  - `arg2`: Localization string (const char*)
  - `arg3-arg6`: Format parameters (const char*)

- **Message Types**:
  - `1`: HUD_PRINTNOTIFY
  - `2`: HUD_PRINTCONSOLE
  - `3`: HUD_PRINTTALK
  - `4`: HUD_PRINTCENTER

## Related Functions

- `UTIL_ClientPrintAll` - Prints to all clients (uses different recipient filter)
- `ClientPrint` - Prints to specific recipient filter

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `ClientPrint.windows.yaml`
- `libserver.so` → `ClientPrint.linux.yaml`
