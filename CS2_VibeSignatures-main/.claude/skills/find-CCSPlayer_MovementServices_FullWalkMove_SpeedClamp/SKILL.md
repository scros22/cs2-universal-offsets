---
name: find-CCSPlayer_MovementServices_FullWalkMove_SpeedClamp
description: |
  Find and identify the velocity clamping branch inside CCSPlayer_MovementServices_FullWalkMove in CS2 binary using IDA Pro MCP, then generate a patch signature to disable it.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate and patch the speed-clamp if-branch that caps player velocity to maxspeed inside FullWalkMove.
  Trigger: FullWalkMove speed clamp, velocity clamping patch, FullWalkMove SpeedClamp, disable maxspeed clamp
disable-model-invocation: true
---

# CCSPlayer_MovementServices_FullWalkMove_SpeedClamp Patch Workflow

## Overview

Locate the velocity clamping branch inside `CCSPlayer_MovementServices_FullWalkMove` and generate a patch that converts the conditional jump into an unconditional jump, making the speed-clamp code a dead path.

The target code pattern in pseudocode:
```c
v20 = (float)((float)(v16 * v16) + (float)(v19 * v19)) + (float)(v17 * v17);
if ( v20 > (float)(v18 * v18) )    // <-- patch target: disable this branch
{
    // velocity clamping logic: scale velocity down to maxspeed
    v21 = fsqrt(v20);
    v22 = v18 / v21;
    *(float *)(a2 + 56) = ... * v22;
    *(float *)(a2 + 60) = ... * v22;
    *(float *)(a2 + 64) = ... * v22;
    ...
}
```

In assembly, this is a `comiss` + `jbe` (or `jbe short`) pair. The `jbe` skips the clamping block when velocity <= maxspeed^2. Patching `jbe` to `jmp` makes it always skip, disabling the clamp entirely.

## Prerequisites

- `CCSPlayer_MovementServices_FullWalkMove` must already be identified. Use SKILL `/get-func-from-yaml` with `func_name=CCSPlayer_MovementServices_FullWalkMove` to load its address. If YAML does not exist, run SKILL `/find-CCSPlayer_MovementServices_FullWalkMove-AND-CCSPlayer_MovementServices_CheckVelocity-AND-CCSPlayer_MovementServices_WaterMove` first.

## Location Steps

### 1. Get FullWalkMove Function Address

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CCSPlayer_MovementServices_FullWalkMove`.

If the skill returns an error, stop and report to user.

### 2. Decompile and Locate the Speed Clamp Pattern

Decompile the function:

```
mcp__ida-pro-mcp__decompile(addr="<func_va>")
```

In the decompiled output, search for the velocity clamping pattern. The key indicators are:
- A sum-of-squares computation: `(x*x) + (y*y) + (z*z)` stored in a variable (e.g., `v20`)
- Compared against another float squared: `v20 > (float)(v18 * v18)`
- Inside the if-block: `fsqrt`, division, and writes to `a2+56`, `a2+60`, `a2+64` (velocity vector)

Note the address annotation on the comparison line (e.g., `/*0x180a00e28*/` or similar).

### 3. Disassemble Around the Comparison

Disassemble the function starting from slightly before the annotated address to find the exact `comiss` + `jbe`/`jbe short` instruction pair:

```
mcp__ida-pro-mcp__disasm(addr="<func_va>", offset=<estimated_offset>, max_instructions=30)
```

Look for this assembly pattern:
```asm
addss   xmm2, xmm1          ; v20 = sum of squares
comiss  xmm2, xmm0          ; compare v20 vs v18*v18
jbe     loc_XXXXXXXX         ; skip clamp block if v20 <= v18*v18
```

Record:
- **patch_va**: Address of the `jbe` instruction
- **jump_target**: The target address of the `jbe` (the `loc_XXXXXXXX` label)

### 4. Determine Patch Bytes

Read the original bytes of the `jbe` instruction:

```
mcp__ida-pro-mcp__get_bytes(regions={"addr": "<patch_va>", "size": 6})
```

Determine the patch based on the instruction encoding:

**Case A: Near `jbe` (`0F 86 rel32` — 6 bytes)**
- `patch_bytes` = `E9 <new_rel32_le> 90`
- Compute: `new_rel32 = jump_target - (patch_va + 5)`

**Case B: Short `jbe` (`76 rel8` — 2 bytes)**
- `patch_bytes` = `EB <rel8>`
- The `rel8` stays the same (same target, `jmp short` uses same displacement encoding as `jbe short`)

### 5. Generate Patch Signature

**ALWAYS** Use SKILL `/generate-signature-for-patch` to generate and validate the signature.

Required context for the skill:
- `func_name`: `CCSPlayer_MovementServices_FullWalkMove`
- `func_va`: From step 1
- `patch_va`: Address of the `jbe` instruction from step 3
- `original_instruction`: e.g., `jbe loc_180A00EE4`
- `patched_instruction`: e.g., `jmp loc_180A00EE4`
- `description`: `Disable velocity clamping in FullWalkMove - patch conditional jbe to unconditional jmp to skip the speed clamping if-branch`

### 6. Write YAML Output

**ALWAYS** Use SKILL `/write-patch-as-yaml` to persist the results.

Required parameters:
- `patch_name`: `CCSPlayer_MovementServices_FullWalkMove_SpeedClamp`
- `patch_sig`: The validated signature from step 5
- `patch_bytes`: The computed patch bytes from step 4
- `patch_sig_disp`: From step 5 result (omit if 0)

## Output YAML Files

- `CCSPlayer_MovementServices_FullWalkMove_SpeedClamp.windows.yaml`
- `CCSPlayer_MovementServices_FullWalkMove_SpeedClamp.linux.yaml`
