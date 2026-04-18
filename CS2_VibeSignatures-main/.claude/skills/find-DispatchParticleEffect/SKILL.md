---
name: find-DispatchParticleEffect
description: Find and identify the DispatchParticleEffect function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the DispatchParticleEffect function by searching for the string "Invalid attach type specified for particle_test in cvar 'particle_test_attach_mode." and analyzing the decompiled code of the referencing function.
disable-model-invocation: true
---

# Find DispatchParticleEffect

Locate `DispatchParticleEffect` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

### 1. Search for Signature String

Use `find_regex` to search for the particle_test attach mode error string:

```
mcp__ida-pro-mcp__find_regex(pattern="Invalid attach type specified for particle_test in cvar")
```

Expected result: Find string address containing "Invalid attach type specified for particle_test in cvar 'particle_test_attach_mode."

### 2. Find Cross-References

Use `xrefs_to` to find locations that reference this string:

```
mcp__ida-pro-mcp__xrefs_to(addrs="<string_addr>")
```

Expected result: Find the function that references this string. There should typically be one referencing function.

### 3. Decompile the Referencing Function

Decompile the function that references the string:

```
mcp__ida-pro-mcp__decompile(addr="<function_addr>")
```

### 4. Identify DispatchParticleEffect in the Decompiled Code

Look for the following code pattern in the **else** branch (the branch where `v9 != -1`, i.e., the attach type is valid):

```c
    if ( v9 == -1 )
    {
      return Warning("Invalid attach type specified for particle_test in cvar 'particle_test_attach_mode.\n");
    }
    else
    {
      v10 = (char *)sub_AAAAAAAA((__int64)&unk_XXXXXXXX, -1);
      if ( !v10 )
        v10 = *(char **)(qword_XXXXXXXX + 8);
      v11 = *v10;
      v12 = (char **)sub_AAAAAAAA((__int64)&unk_YYYYYYYY, -1);
      if ( !v12 )
        v12 = *(char ***)(qword_YYYYYYYY + 8);
      v13 = *v12;
      if ( v13 )
        v2 = v13;
      result = sub_BBBBBBBB(v5, v3, 0LL);
      for ( i = result; result; i = result )
      {
        sub_CCCCCCCC(v2, v9, i, v11, 0, 1, -1, 0LL, 0); // <-- This is DispatchParticleEffect
        result = sub_BBBBBBBB(v5, v3, i);
      }
    }
```

**Identification criteria:**
- Located inside the `else` branch (valid attach type path)
- Called inside a `for` loop at the bottom of the else branch
- Takes **9 arguments**: `(particle_name, attach_type, entity, effect_name, 0, 1, -1, NULL, 0)`
- The first argument is a string (particle name), second is the attach type integer
- The loop iterates over entities returned by a "find next entity" helper

Extract the address of this call target (`sub_CCCCCCCC`) as the `DispatchParticleEffect` address.

### 5. Rename the Function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "DispatchParticleEffect"}]}
```

### 6. Generate Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 7. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `DispatchParticleEffect`
- `func_addr`: The function address from step 4
- `func_sig`: The validated signature from step 6

## Function Characteristics

- **Type**: Non-virtual (regular) function
- **Parameters**: `(particle_name, attach_type, entity, effect_name, unknown1, unknown2, unknown3, unknown4, unknown5)` — 9 arguments total
- **Purpose**: Dispatches a particle effect on an entity with a specified attachment type. Used by the particle_test console command system.

## String References

- `"Invalid attach type specified for particle_test in cvar 'particle_test_attach_mode."` — Found in the caller function, used to locate DispatchParticleEffect indirectly

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `DispatchParticleEffect.windows.yaml`
- `libserver.so` / `libserver.so` → `DispatchParticleEffect.linux.yaml`
