---
name: find-IGameSystem_DestroyAllGameSystems
description: Find and identify IGameSystem_DestroyAllGameSystems (static regular function) in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 client.dll or libclient.so to locate the game system destruction function by decompiling CLoopModeGame_ReceivedServerInfo and identifying the call after the "--CLoopModeGame::SetWorldSession" code pattern.
disable-model-invocation: true
---

# Find IGameSystem_DestroyAllGameSystems

Locate `IGameSystem_DestroyAllGameSystems` (static regular function) in CS2 `client.dll` or `libclient.so` using IDA Pro MCP tools.

## Method

### 1. Get CLoopModeGame_ReceivedServerInfo Function Info

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CLoopModeGame_ReceivedServerInfo`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `func_va` for subsequent steps.

### 2. Decompile CLoopModeGame_ReceivedServerInfo

```
mcp__ida-pro-mcp__decompile addr="<func_va>"
```

### 3. Identify IGameSystem_DestroyAllGameSystems from Code Pattern

In the decompiled output, search for the string `"--CLoopModeGame::SetWorldSession"`. The `IGameSystem_DestroyAllGameSystems` call is located shortly after this string reference.

**Windows pattern:**
```c
      v15 = *(_QWORD *)(a1 + 80);
      if ( v15 )
      {
        (*(void (__fastcall **)(__int64, const char *))(*(_QWORD *)v15 + 8LL))(v15, "--CLoopModeGame::SetWorldSession");
        *(_QWORD *)(a1 + 80) = 0;
      }
      if ( *(int *)(a1 + 1112) >= 1 )
        IGameSystem_DestroyAllGameSystems();      // <-- THIS IS THE TARGET
      sub_XXXXXXXXX(a1 - 16, 0);
```

**Linux pattern:**
```c
  v2 = *(_QWORD *)(a1 + 96);
  if ( v2 )
  {
    (*(void (__fastcall **)(__int64, const char *))(*(_QWORD *)v2 + 8LL))(v2, "--CLoopModeGame::SetWorldSession");
    *(_QWORD *)(a1 + 96) = 0;
  }
  v3 = *(_DWORD *)(a1 + 1128);
  if ( v3 > 0 )
  {
    IGameSystem_DestroyAllGameSystems();          // <-- THIS IS THE TARGET
    v3 = *(_DWORD *)(a1 + 1128);
  }
  if ( v3 )
    sub_XXXXXXXX(a1, 0);
```

**Identification logic:**
1. Find the code that references `"--CLoopModeGame::SetWorldSession"` string
2. After the string reference and pointer null-out, there is an integer comparison (member offset may vary)
3. The **first function call** inside that conditional block (with no arguments) is `IGameSystem_DestroyAllGameSystems`

### 4. Rename the function (if not already named)

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "IGameSystem_DestroyAllGameSystems"}]}
```

### 5. Generate Function Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `IGameSystem_DestroyAllGameSystems`.

### 6. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystem_DestroyAllGameSystems`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 5

Note: This is a static regular function, NOT a virtual function, so no vtable parameters are needed.

## Function Characteristics

- **Purpose**: Destroys and shuts down all registered game systems during disconnect/cleanup
- **Type**: Static regular function (not a virtual function, not a member function)
- **Parameters**: None (takes no arguments)
- **Called from**: `CLoopModeGame_ReceivedServerInfo` — called during world session teardown when `SetWorldSession` is clearing the current session
- **Call context**: Called after the `"--CLoopModeGame::SetWorldSession"` virtual call and pointer null-out, inside a conditional check on a member field (connection state counter)

## Identification Pattern

The function is identified by:
1. Decompiling `CLoopModeGame_ReceivedServerInfo` (found via vtable)
2. Locating the `"--CLoopModeGame::SetWorldSession"` string reference
3. Finding the no-argument function call in the conditional block immediately following the string reference and pointer null-out

This is robust because:
- `CLoopModeGame_ReceivedServerInfo` is reliably found via its vtable
- The `"--CLoopModeGame::SetWorldSession"` string is a distinctive anchor
- `IGameSystem_DestroyAllGameSystems` is the only no-argument call in that specific conditional block

## Output YAML Format

The output YAML filename depends on the platform:
- `client.dll` -> `IGameSystem_DestroyAllGameSystems.windows.yaml`
- `libclient.so` -> `IGameSystem_DestroyAllGameSystems.linux.yaml`
