---
name: find-UTIL_ClientPrintAll
description: Find and identify the UTIL_ClientPrintAll function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the UTIL_ClientPrintAll function by searching for the "#Game_idle_kick" string reference and analyzing cross-references.
disable-model-invocation: true
---

# Find UTIL_ClientPrintAll

Locate `UTIL_ClientPrintAll` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the idle kick string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="#Game_idle_kick"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. The xref will point to the idle kick handler function. Decompile it:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. In the decompiled code, locate the call that uses `"#Game_idle_kick"` as a parameter:
   ```c
   sub_XXXXXX(2, (int)"#Game_idle_kick", (int)v16, 0, 0, 0);
   ```
   This `sub_XXXXXX` is `UTIL_ClientPrintAll`.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<UTIL_ClientPrintAll_addr>", "name": "UTIL_ClientPrintAll"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `UTIL_ClientPrintAll`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 6

## String Reference

The function is called with the localization string `#Game_idle_kick` when a player is kicked for being idle:

```c
UTIL_ClientPrintAll(2, "#Game_idle_kick", player_name, 0, 0, 0);
```

## Function Characteristics

- **Purpose**: Broadcasts a message to all connected clients
- **Parameters**:
  - `arg0` (int): Message destination type (2 = HUD_PRINTCENTER or similar)
  - `arg1` (const char*): Localization string or message
  - `arg2` (const char*): Parameter 1 (e.g., player name)
  - `arg3-arg5`: Additional parameters (usually 0)

## Function Signature Pattern

The function has a distinctive prologue that saves multiple callee-saved registers:

```asm
push    rbp
mov     rbp, rsp
push    r15
mov     r15, r9
push    r14
mov     r14, r8
push    r13
mov     r13, rcx
push    r12
mov     r12, rdx
push    rbx
lea     rbx, [rbp-50h]
sub     rsp, 38h
```

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `UTIL_ClientPrintAll.windows.yaml`
- `libserver.so` → `UTIL_ClientPrintAll.linux.yaml`
