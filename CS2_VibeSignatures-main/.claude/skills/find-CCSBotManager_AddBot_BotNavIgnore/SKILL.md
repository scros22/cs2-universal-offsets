---
name: find-CCSBotManager_AddBot_BotNavIgnore
description: |
  Find and patch the g_pNavMesh null-check inside CCSBotManager_AddBot in CS2 binary using IDA Pro MCP.
  This patch removes the navigation mesh requirement so bots can be added even without a nav mesh loaded.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate and patch the nav mesh check in AddBot.
  Trigger: bot nav ignore, AddBot nav mesh patch, bot without nav, CCSBotManager nav ignore
disable-model-invocation: true
---

# CCSBotManager_AddBot_BotNavIgnore Patch Workflow

## Overview

Locate the `g_pNavMesh` null-check at the beginning of `CCSBotManager_AddBot` and generate a patch that converts the conditional `jz` (jump-if-zero) into an unconditional `jmp` past the early-return, so bots can be added even when no navigation mesh is loaded.

The target code pattern in pseudocode:
```c
if ( !g_pNavMesh || !*(_BYTE *)(g_pNavMesh + 264) )   // <-- patch target: skip this check
    return 0;
```

In assembly, the first branch of this check is:
```asm
mov     rax, cs:g_pNavMesh
...
test    rax, rax
jz      loc_XXXXXXXX         ; <-- patch this: jz -> jmp to loc after the nav check
```

The `jz` is a near conditional jump (`0F 84 rel32` — 6 bytes). Patching it to `jmp rel32` (`E9 <new_rel32> 90` — 5 bytes + 1 NOP) makes it always jump past both the null-pointer check and the `g_pNavMesh + 264` byte check, landing at the code that continues bot creation.

The jump target should be the `loc_` label that is reached after both nav mesh checks pass (the code right after the second `jz` that also targets the same early-return).

## Prerequisites

- `CCSBotManager_AddBot` must already be identified. Use SKILL `/get-func-from-yaml` with `func_name=CCSBotManager_AddBot` to load its address. If YAML does not exist, run SKILL `/find-CCSBotManager_AddBot-AND-g_pCSBotManager-AND-g_pNavMesh` first.

## Location Steps

### 1. Get CCSBotManager_AddBot Function Address

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CCSBotManager_AddBot`.

If the skill returns an error, stop and report to user.

### 2. Decompile and Locate the Nav Mesh Check

Decompile the function:

```
mcp__ida-pro-mcp__decompile(addr="<func_va>")
```

In the decompiled output, search for the nav mesh check pattern at the very beginning of the function body. The key indicators are:
- `if ( !g_pNavMesh || !*(_BYTE *)(g_pNavMesh + 264) )` or equivalent
- Immediately followed by `return 0;`

Note the address annotation on the if-statement line.

### 3. Disassemble the Function Prologue

Disassemble the first ~30 instructions of the function to find the exact `test rax, rax` + `jz` pair:

```
mcp__ida-pro-mcp__disasm(addr="<func_va>", max_instructions=30)
```

Look for this assembly pattern:
```asm
mov     rax, cs:g_pNavMesh
...
test    rax, rax
jz      loc_XXXXXXXX         ; early return if g_pNavMesh == NULL
cmp     byte ptr [rax+108h], 0
jz      loc_XXXXXXXX         ; early return if nav mesh not loaded
```

Record:
- **patch_va**: Address of the first `jz` instruction (the `test rax, rax` / `jz` pair)
- **jump_target**: The address of the code right AFTER the second `jz` — this is where execution continues when both checks pass. This is the label we want to jump to unconditionally.

### 4. Determine Patch Bytes

Read the original bytes of the `jz` instruction:

```
mcp__ida-pro-mcp__get_bytes(regions={"addr": "<patch_va>", "size": 6})
```

The `jz` is a near conditional jump: `0F 84 rel32` (6 bytes).

Compute the new unconditional jump to the target (the code after both nav checks):
- `new_rel32 = jump_target - (patch_va + 5)`
- `patch_bytes` = `E9 <new_rel32_le> 90`

Where `<new_rel32_le>` is the 4-byte little-endian encoding of `new_rel32`.

### 5. Generate Patch Signature

**ALWAYS** Use SKILL `/generate-signature-for-patch` to generate and validate the signature.

Required context for the skill:
- `func_name`: `CCSBotManager_AddBot`
- `func_va`: From step 1
- `patch_va`: Address of the `jz` instruction from step 3
- `original_instruction`: e.g., `jz loc_1802A1485`
- `patched_instruction`: e.g., `jmp loc_1802A104F` + `nop`
- `description`: `Remove g_pNavMesh null-check in CCSBotManager_AddBot - patch conditional jz to unconditional jmp to skip nav mesh validation`

### 6. Write YAML Output

**ALWAYS** Use SKILL `/write-patch-as-yaml` to persist the results.

Required parameters:
- `patch_name`: `CCSBotManager_AddBot_BotNavIgnore`
- `patch_sig`: The validated signature from step 5
- `patch_bytes`: The computed patch bytes from step 4
- `patch_sig_disp`: From step 5 result (omit if 0)

## Output YAML Files

- `CCSBotManager_AddBot_BotNavIgnore.windows.yaml`
- `CCSBotManager_AddBot_BotNavIgnore.linux.yaml`
