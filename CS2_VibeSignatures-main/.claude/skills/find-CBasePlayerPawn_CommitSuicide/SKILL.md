---
name: find-CBasePlayerPawn_CommitSuicide
description: Find and identify the CBasePlayerPawn_CommitSuicide function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the CommitSuicide function by searching for the "bot_kill" command string, tracing to its handler, and identifying the CommitSuicide vfunc call in the kill loop.
disable-model-invocation: true
---

# Find CBasePlayerPawn_CommitSuicide

Locate `CBasePlayerPawn_CommitSuicide` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for the `bot_kill` command string:

   ```
   mcp__ida-pro-mcp__find_regex pattern="bot_kill.*all"
   ```

   This should find a string like:
   `"bot_kill <all> <t|ct> <type> <difficulty> <name> - Kills a specific bot, or all bots, matching the given criteria."`

2. Find cross-references to the string:

   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

   This leads to a ConCommand registration function. Decompile it to find the command handler (callback) address — the first argument stored before the description string in the registration call.

3. Decompile the `bot_kill` command handler and locate the kill loop:

   ```
   mcp__ida-pro-mcp__decompile addr="<handler_addr>"
   ```

   Look for a loop pattern like this:

   ```c
   do
   {
     v24 = *(_QWORD *)v23;
     if ( (*(unsigned __int8 (__fastcall **)(_QWORD))(**(_QWORD **)(*(_QWORD *)v23 + 24LL) + <IsAlive_offset>))(*(_QWORD *)(*(_QWORD *)v23 + 24LL)) )
     {
       (*(void (__fastcall **)(_QWORD, _QWORD, _QWORD))(**(_QWORD **)(v24 + 24) + <CommitSuicide_offset>))(
         *(_QWORD *)(v24 + 24),
         0LL,
         0LL);
       if ( !v5 )
         break;
     }
     ++v21;
     v23 += 8;
   }
   while ( v21 < v20 );
   ```

   The loop iterates over matched bots. For each bot:
   - `*(v24 + 24)` dereferences the PlayerPawn pointer
   - The first vfunc call (with `<IsAlive_offset>`) checks if the pawn is alive
   - The second vfunc call (with `<CommitSuicide_offset>`) calls `pPlayerPawn->CommitSuicide(false, false)`
   - If not in "all" mode (`!v5`), it breaks after the first kill

   Extract `<CommitSuicide_offset>` from the decompiled code (e.g., `3200LL` = `0xC80`).

4. Get CBasePlayerPawn vtable information:

   **ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CBasePlayerPawn`.

   Extract `vtable_va`, `vtable_numvfunc` and `vtable_entries` from the result.

5. Map the vfunc offset to a vtable index and resolve the function address:

   ```
   vfunc_index = <CommitSuicide_offset> / 8
   ```

   Look up `vtable_entries[vfunc_index]` to get the function address.

6. Verify function characteristics to confirm `CBasePlayerPawn::CommitSuicide`:

   Decompile the resolved function address. The function should match:

   Windows:
   ```c
   char __fastcall CBasePlayerPawn_CommitSuicide(float *a1, unsigned __int8 a2, char a3)
   {
      __int64 v4; // rbp
      char result; // al
      _BYTE v7[112]; // [rsp+40h] [rbp-138h] BYREF
      __int64 v8; // [rsp+B0h] [rbp-C8h]
      int v9; // [rsp+180h] [rbp+8h] BYREF
      char v10; // [rsp+198h] [rbp+20h] BYREF

      v4 = a2;
      result = (*(__int64 (__fastcall **)(float *))(*(_QWORD *)a1 + 1336LL))(a1); // IsAlive check
      if ( result )
      {
         sub_XXX(&v9, *(_DWORD *)(*((_QWORD *)a1 + 2) + 56LL));
         result = sub_XXX(a1 + 824, (float *)&v9);
         if ( !result || a3 )
         {
            sub_XXX(&v9, *(_DWORD *)(*((_QWORD *)a1 + 2) + 56LL));
            a1[824] = *(float *)sub_XXX(&v10, &v9);
            sub_XXX((unsigned int)v7, (_DWORD)a1, (_DWORD)a1, 0, 1065353216, (_DWORD)v4 << 6, 0);
            v8 |= (32 * (v4 ^ 1) + 32) | 0x116;
            sub_XXX(a1, (__int64)v7, 0LL); // CBaseEntity::TakeDamageOld
            return sub_XXX((__int64)v7);
         }
      }
      return result;
   }
   ```

   Linux:
   ```c
   void __fastcall CBasePlayerPawn_CommitSuicide(__int64 a1, unsigned __int8 a2, char a3)
   {
      unsigned __int8 (*v4)(void); // rax
      _BYTE v5[112]; // [rsp+0h] [rbp-140h] BYREF
      __int64 v6; // [rsp+70h] [rbp-D0h]

      v4 = *(unsigned __int8 (**)(void))(*(_QWORD *)a1 + <IsAlive_offset>);
      if ( (char *)v4 == (char *)CBaseEntity_IsPlayerPawn )
      {
         if ( *(_BYTE *)(a1 + 1472) )
            return;
      }
      else if ( !v4() )
      {
         return;
      }
      if ( *(float *)(a1 + 4072) <= sub_XXX(...) || a3 )
      {
         *(float *)(a1 + 4072) = sub_XXX(...) + 5.0;
         sub_XXX(v5, a1, a1, 0LL, a2 << 6, 0LL, 1.0);
         v6 |= (a2 == 0 ? 64LL : 32LL) | 0x116;
         sub_XXX(a1, v5, 0LL); // CBaseEntity::TakeDamageOld
         sub_XXX(v5);
      }
   }
   ```

   Key verification points:
   - Calls `IsAlive` via vtable at the start
   - Constructs a `CTakeDamageInfo` on the stack (112-byte buffer)
   - Sets damage flags with `| 0x116`
   - Calls `CBaseEntity::TakeDamageOld` (verify by checking for string `"CBaseEntity::TakeDamageOld: damagetype %d with info.GetDamagePosition() == Vector::vZero\n"` in its callee)

   If the code pattern matches, proceed to rename.

7. Rename the function:

   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CBasePlayerPawn_CommitSuicide"}]}
   ```

8. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

9. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CBasePlayerPawn_CommitSuicide`
   - `func_addr`: The function address from step 5
   - `func_sig`: The validated signature from step 8

   VTable parameters:
   - `vtable_name`: `CBasePlayerPawn`
   - `vfunc_index`: The vtable index from step 5
   - `vfunc_offset`: `vfunc_offset = vfunc_index * 8`

## Function Characteristics

- **Parameters**: `(this, bool bExplodeDeath, bool bForce)` where `this` is CBasePlayerPawn pointer
- **Purpose**: Handles player suicide by constructing a CTakeDamageInfo and calling TakeDamageOld
- **Key Operations**:
  - Checks if pawn is alive via vtable call (IsAlive)
  - Checks cooldown timer to prevent rapid suicide calls
  - Constructs a 112-byte CTakeDamageInfo on the stack
  - Sets damage flags: `(32 * (bExplodeDeath ^ 1) + 32) | 0x116`
  - Calls `CBaseEntity::TakeDamageOld(this, info, 0)`
  - Destructs the CTakeDamageInfo

## VTable Information

- **VTable Name**: `CBasePlayerPawn::\`vftable'`
- **VTable Mangled Name**: `??_7CBasePlayerPawn@@6B@` (Windows) / `_ZTV16CBasePlayerPawn` (Linux)
- **VTable Index**: Changes with game updates. Resolve via `<CommitSuicide_offset> / 8`.
- **VTable Offset**: Changes with game updates. Extract from the `bot_kill` handler loop.

* Note that for `libserver.so`, the first 16 bytes of "vftable" are for RTTI. The real vftable = `_ZTV16CBasePlayerPawn` + `0x10`.

## String-Based Discovery

The primary discovery method uses the `bot_kill` console command:

1. **Search string**: `"bot_kill <all>"` or `"bot_kill.*all"`
2. **Xref chain**: String → ConCommand registration → command handler callback
3. **Kill loop**: The handler iterates matched bots, calling `pPlayerPawn->CommitSuicide(false, false)` via vtable

This is more robust than scanning vtable entries because:
- The `bot_kill` string is unique and stable across updates
- The kill loop structure is distinctive and unlikely to change
- The vfunc offset is extracted directly from the call site

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `CBasePlayerPawn_CommitSuicide.windows.yaml`
- `libserver.so` → `CBasePlayerPawn_CommitSuicide.linux.yaml`
