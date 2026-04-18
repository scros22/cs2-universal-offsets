---
name: find-CPhysBox_Use_PatchCaller
description: |
  Find and identify the CPhysBox_Use_PatchCaller patch in CS2 binary using IDA Pro MCP.
  This patch changes the third argument of the sub_XXXXXXXX call inside CPhysBox_Use from
  inputdata->pCaller to this (the CPhysBox entity pointer), so the caller is the physbox itself.
  Use this skill when reverse engineering CS2 server.dll or libserver.so to locate and patch the caller argument.
  Trigger: CPhysBox_Use patch caller, CPhysBox_Use_PatchCaller, physbox use caller patch
disable-model-invocation: true
---

# CPhysBox_Use_PatchCaller Patch Workflow

## Overview

Locate the instruction inside `CPhysBox_Use` that loads `inputdata->pCaller` (the third argument
to the inner call) and generate a patch that replaces it with `this` (the CPhysBox entity pointer).

The target code pattern in pseudocode (before patch):
```c
__int64 __fastcall CPhysBox_Use(__int64 pthis, __int64 inputdata)
{
  __int64 result;
  result = CBaseEntity_Use(pthis, inputdata);
  if ( *(_BYTE *)(pthis + 2093) )
    return sub_XXXXXXXX((int)pthis + 2176, *(_QWORD *)inputdata, *(_QWORD *)(inputdata + 8));
                                                                   // ^^^ patch target: pCaller
  return result;
}
```

After patch:
```c
    return sub_XXXXXXXX((int)pthis + 2176, *(_QWORD *)inputdata, pthis);
                                                                  // ^^^ now uses this
```

### Windows

The target instruction:
```asm
mov     r8, [rbx+8]          ; r8 = inputdata->pCaller
```
Patched to:
```asm
mov     r8, rdi              ; r8 = this (pthis)
nop
```
- Original bytes: `4C 8B 43 08`
- Patch bytes: `49 89 F8 90`

### Linux

The target instruction:
```asm
mov     rdx, [r12+8]         ; rdx = inputdata->pCaller
```
Patched to:
```asm
mov     rdx, rbx             ; rdx = this (pthis)
nop
nop
```
- Original bytes: `49 8B 54 24 08`
- Patch bytes: `48 89 DA 90 90`

## Prerequisites

- `CPhysBox_Use` must already be identified. Use SKILL `/get-func-from-yaml` with `func_name=CPhysBox_Use` to load its address. If YAML does not exist, run SKILL `/find-CPhysBox_Use` first.

## Location Steps

### 1. Get CPhysBox_Use Function Address

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CPhysBox_Use`.

If the skill returns an error, stop and report to user.

### 2. Decompile and Locate the Caller Argument

Decompile the function:

```
mcp__ida-pro-mcp__decompile(addr="<func_va>")
```

In the decompiled output, search for the inner call pattern. The key indicators are:
- A call to `CBaseEntity_Use(pthis, inputdata)` at the beginning
- A byte check: `if ( *(_BYTE *)(pthis + <offset>) )`
- Inside the if-block: a call with three arguments where the third is `*(_QWORD *)(inputdata + 8)` — this is `inputdata->pCaller`

Note the address annotation on the line that loads `inputdata + 8`.

### 3. Disassemble Around the Caller Load

Disassemble the function to find the exact instruction that loads `inputdata->pCaller`:

```
mcp__ida-pro-mcp__disasm(addr="<func_va>", max_instructions=30)
```

**Windows pattern:**
```asm
mov     r8, [rbx+8]          ; load inputdata->pCaller into r8 (3rd arg)
lea     rcx, [rdi+880h]      ; 1st arg
mov     rdx, [rbx]            ; 2nd arg = inputdata->pActivator
call    sub_XXXXXXXX
```

**Linux pattern:**
```asm
mov     rdx, [r12+8]         ; load inputdata->pCaller into rdx (3rd arg in SysV ABI)
...
call    sub_XXXXXXXX
```

Record:
- **patch_va**: Address of the `mov r8, [rbx+8]` (Windows) or `mov rdx, [r12+8]` (Linux) instruction

### 4. Determine Patch Bytes

Read the original bytes of the instruction:

```
mcp__ida-pro-mcp__get_bytes(regions={"addr": "<patch_va>", "size": 5})
```

**Windows** (`mov r8, [rbx+8]` — 4 bytes: `4C 8B 43 08`):
- Patch bytes: `49 89 F8 90` (`mov r8, rdi` + `nop`)

**Linux** (`mov rdx, [r12+8]` — 5 bytes: `49 8B 54 24 08`):
- Patch bytes: `48 89 DA 90 90` (`mov rdx, rbx` + 2x `nop`)

### 5. Generate Patch Signature

**ALWAYS** Use SKILL `/generate-signature-for-patch` to generate and validate the signature.

Required context for the skill:
- `func_name`: `CPhysBox_Use`
- `func_va`: From step 1
- `patch_va`: Address of the mov instruction from step 3
- `original_instruction`: `mov r8, [rbx+8]` (Windows) or `mov rdx, [r12+8]` (Linux)
- `patched_instruction`: `mov r8, rdi; nop` (Windows) or `mov rdx, rbx; nop; nop` (Linux)
- `description`: `Patch CPhysBox_Use to pass this instead of inputdata->pCaller as the third argument`

### 6. Write YAML Output

**ALWAYS** Use SKILL `/write-patch-as-yaml` to persist the results.

Required parameters:
- `patch_name`: `CPhysBox_Use_PatchCaller`
- `patch_sig`: The validated signature from step 5
- `patch_bytes`: The computed patch bytes from step 4
- `patch_sig_disp`: From step 5 result (omit if 0)

## Output YAML Files

- `CPhysBox_Use_PatchCaller.windows.yaml`
- `CPhysBox_Use_PatchCaller.linux.yaml`
