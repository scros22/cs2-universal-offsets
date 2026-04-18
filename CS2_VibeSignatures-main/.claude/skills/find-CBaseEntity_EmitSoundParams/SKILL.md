---
name: find-CBaseEntity_EmitSoundParams
description: Find and identify the CBaseEntity_EmitSoundParams function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the EmitSoundParams function by searching for the "Stops a named sound playing from this" string, tracing xrefs, and identifying the function via a volume-check code pattern.
disable-model-invocation: true
---

# Find CBaseEntity_EmitSoundParams

Locate `CBaseEntity_EmitSoundParams` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="Stops a named sound playing from this"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Disassemble around the xref address (*DO NOT decompile the whole referencing function — it is too large):
   ```
   mcp__ida-pro-mcp__disasm addr="<xref_addr - 0x60>" max_instructions=40
   ```

4. In the disassembly before `lea REG, aStopsANamedSou`, find the last two `lea REG, sub_XXXXXXX` instructions. These reference two candidate functions.

5. Decompile both candidate functions and look for the volume-check pattern:
   ```c
   if ( a3 != 100 && a3 > 0 )
     v19 = a3;
   ```
   or equivalent:
   ```c
   if ( a3 != 100 )
   {
     if ( a3 > 0 )
       v5 = a3;
     v15 = v5;
   }
   ```
   The function containing this pattern is `CBaseEntity_EmitSoundParams`.

6. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CBaseEntity_EmitSoundParams"}]}
   ```

7. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `CBaseEntity_EmitSoundParams`.

8. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CBaseEntity_EmitSoundParams`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 7

## Function Characteristics

- **Parameters**: `(this, sound_name, volume, pitch, delay)` where `this` is CBaseEntity pointer
- **Volume default**: 100 — overridden by `a3` when `a3 != 100 && a3 > 0`
- **Not a virtual function** — use `/write-func-as-yaml` (not vfunc variant)

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBaseEntity_EmitSoundParams.windows.yaml`
- `libserver.so` → `CBaseEntity_EmitSoundParams.linux.yaml`
