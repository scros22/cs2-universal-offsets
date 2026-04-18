---
name: find-GetCSWeaponDataFromKey
description: Find and identify the GetCSWeaponDataFromKey function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the GetCSWeaponDataFromKey function by searching for known projectile string references and analyzing the characteristic code pattern.
disable-model-invocation: true
---

# Find GetCSWeaponDataFromKey

Locate `GetCSWeaponDataFromKey` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

1. Search for projectile string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="smokegrenade_projectile"
   ```

2. Get cross-references to the base string (not the .cpp path):
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile each referencing function and look for the characteristic pattern:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify `GetCSWeaponDataFromKey` by finding this code pattern:
   ```c
   v9 = sub_XXXXXXXX("smokegrenade_projectile", a1, a2, a5);
   v10 = sub_YYYYYYYY();
   v11 = a6;
   v12 = sub_ZZZZZZZZ(v10, (unsigned __int16)a6, 0LL, 0LL);
   if ( v12 )
   {
     v14 = sub_AAAAAAAA(v12);
     if ( v14 )
     {
       v15 = sub_BBBBBBBB(v33, "%d", *(unsigned __int16 *)(v14 + 16));
       v16 = *(_DWORD *)(v15 + 4);
       if ( (v16 & 0x40000000) != 0 )
       {
         v17 = (void *)(v15 + 8);
       }
       else if ( (v16 & 0x3FFFFFFF) != 0 )
       {
         v17 = *(void **)(v15 + 8);
       }
       else
       {
         v17 = &unk_XXXXXXXX;
       }
       v13 = sub_CCCCCCCC(1LL, v17);  // <-- This is GetCSWeaponDataFromKey
       CBufferString::Purge((CBufferString *)v33, 0);
     }
     ...
   }
   ```
   The function called with `(1LL, v17)` after the `CBufferString` formatting is `GetCSWeaponDataFromKey`.

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": {"addr": "<function_addr>", "name": "GetCSWeaponDataFromKey"}}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `GetCSWeaponDataFromKey`
   - `func_addr`: The function address from step 6
   - `func_sig`: The validated signature from step 7

## Function Characteristics

- **Signature**: `__int64 __fastcall GetCSWeaponDataFromKey(__int64 a1, void* key)`
- **First parameter**: Usually `1LL` (constant)
- **Second parameter**: A string key (weapon name like "smokegrenade_projectile")
- **Return value**: Pointer to weapon data structure or 0 if not found

## Identifying Strings

The function is referenced in projectile creation functions. Search for any of these strings:
- `smokegrenade_projectile`
- `flashbang_projectile`
- `hegrenade_projectile`
- `molotov_projectile`
- `decoy_projectile`

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `GetCSWeaponDataFromKey.windows.yaml`
- `libserver.so` → `GetCSWeaponDataFromKey.linux.yaml`
