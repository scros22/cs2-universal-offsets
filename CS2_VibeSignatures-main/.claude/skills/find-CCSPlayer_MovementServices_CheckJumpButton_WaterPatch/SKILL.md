---
name: find-CCSPlayer_MovementServices_CheckJumpButton_WaterPatch
description: |
  Find and patch the water jump velocity inside CCSPlayer_MovementServices_CheckJumpButton in CS2 binary using IDA Pro MCP.
  This patch changes the water jump velocity from 100.0f to 145.0f by modifying the immediate operand of a mov instruction.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate and patch the water jump height value.
  Trigger: water jump patch, CheckJumpButton water velocity, jump height patch, water jump 145
disable-model-invocation: true
---

# CCSPlayer_MovementServices_CheckJumpButton_WaterPatch

## Overview

Locate the water jump velocity assignment inside `CCSPlayer_MovementServices_CheckJumpButton` and generate a patch that changes the immediate value from `100.0f` (0x42C80000) to `145.0f` (0x43110000).

The target code pattern in pseudocode:
```c
if ( v5 == (int *)0x8000 )    // move type check for water
{
    *(_DWORD *)(a2 + 64) = 1120403456;  // <-- patch target: 100.0f -> 145.0f
}
```

In assembly, this is a `mov dword ptr [reg+40h], 42C80000h` instruction. The patch changes the immediate operand from `42C80000h` (100.0f) to `43110000h` (145.0f).

## Prerequisites

- `CCSPlayer_MovementServices_CheckJumpButton` must already be identified. Use SKILL `/get-func-from-yaml` with `func_name=CCSPlayer_MovementServices_CheckJumpButton` to load its address. If YAML does not exist, run SKILL `/find-CCSPlayer_MovementServices_CheckJumpButton` first.

## Location Steps

### 1. Get CheckJumpButton Function Address

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CCSPlayer_MovementServices_CheckJumpButton`.

If the skill returns an error, stop and report to user.

### 2. Decompile and Locate the Water Jump Pattern

Decompile the function:

```
mcp__ida-pro-mcp__decompile(addr="<func_va>")
```

In the decompiled output, search for the water jump velocity pattern. The key indicators are:
- A comparison against `0x8000` (water move type)
- Inside the if-block: `*(_DWORD *)(a2 + 64) = 1120403456` (which is 100.0f as IEEE 754)
- Note: `1120403456` decimal = `0x42C80000` = `100.0f` in IEEE 754 floating point

Note the address annotation on the assignment line.

### 3. Disassemble Around the Assignment

Disassemble the function around the annotated address to find the exact `mov` instruction:

```
mcp__ida-pro-mcp__disasm(addr="<func_va>", offset=<estimated_offset>, max_instructions=20)
```

Look for this assembly pattern:
```asm
cmp     rax, 8000h              ; check water move type
jnz     short loc_XXXXXXXX      ; skip if not water
mov     dword ptr [rsi+40h], 42C80000h  ; <-- patch target: 100.0f
```

Record:
- **patch_va**: Address of the `mov dword ptr [reg+40h], 42C80000h` instruction

### 4. Determine Patch Bytes

Read the original bytes of the `mov` instruction:

```
mcp__ida-pro-mcp__get_bytes(regions={"addr": "<patch_va>", "size": 7})
```

The original 7-byte instruction: `C7 46 40 00 00 C8 42`
- `C7 46 40` = opcode + ModR/M for `mov dword ptr [rsi+40h]`
- `00 00 C8 42` = immediate `0x42C80000` (100.0f, little-endian)

Patch bytes: `C7 46 40 00 00 11 43`
- Same opcode + ModR/M
- `00 00 11 43` = immediate `0x43110000` (145.0f, little-endian)

### 5. Generate Patch Signature

**ALWAYS** Use SKILL `/generate-signature-for-patch` to generate and validate the signature.

Required context for the skill:
- `func_name`: `CCSPlayer_MovementServices_CheckJumpButton`
- `func_va`: From step 1
- `patch_va`: Address of the `mov` instruction from step 3
- `original_instruction`: `mov dword ptr [rsi+40h], 42C80000h`
- `patched_instruction`: `mov dword ptr [rsi+40h], 43110000h`
- `description`: `Change water jump velocity from 100.0f to 145.0f in CheckJumpButton`

### 6. Write YAML Output

**ALWAYS** Use SKILL `/write-patch-as-yaml` to persist the results.

Required parameters:
- `patch_name`: `CCSPlayer_MovementServices_CheckJumpButton_WaterPatch`
- `patch_sig`: The validated signature from step 5
- `patch_bytes`: `C7 46 40 00 00 11 43`
- `patch_sig_disp`: From step 5 result (omit if 0)

## IEEE 754 Float Reference

- `100.0f` = `0x42C80000` = `1120403456` decimal
- `145.0f` = `0x43110000` = `1125122048` decimal

## Output YAML Files

- `CCSPlayer_MovementServices_CheckJumpButton_WaterPatch.windows.yaml`
- `CCSPlayer_MovementServices_CheckJumpButton_WaterPatch.linux.yaml`
