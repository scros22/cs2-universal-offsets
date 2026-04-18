---
name: find-IGameSystem_AddByName-AND-IGameSystem_Add
description: Find and identify IGameSystem_AddByName and IGameSystem_Add (static regular functions) in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 client.dll or libclient.so to locate both functions by decompiling CLoopModeGame_ReceivedServerInfo and identifying the calls that add game systems by name and by pointer.
disable-model-invocation: true
---

# Find IGameSystem_AddByName and IGameSystem_Add

Locate `IGameSystem_AddByName` and `IGameSystem_Add` (static regular functions) in CS2 `client.dll` or `libclient.so` using IDA Pro MCP tools.

## Method

### 1. Get CLoopModeGame_ReceivedServerInfo Function Info

**ALWAYS** Use SKILL `/get-func-from-yaml` with `func_name=CLoopModeGame_ReceivedServerInfo`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `func_va` for subsequent steps.

### 2. Decompile CLoopModeGame_ReceivedServerInfo

```
mcp__ida-pro-mcp__decompile addr="<func_va>"
```

### 3. Identify IGameSystem_AddByName and IGameSystem_Add from Code Pattern

In the decompiled output, look for a series of calls that add game systems by name string, followed by calls that add game systems by pointer:

**Pattern:**
```c
  IGameSystem_AddByName("GameRulesGameSystem");
  IGameSystem_AddByName("RenderGameSystem");
  IGameSystem_AddByName("ClientSoundscapeSystem");
  IGameSystem_AddByName("BodyGameSystem");
  IGameSystem_AddByName("AnimGraphUpdate");
  IGameSystem_AddByName("PhysicsGameSystem");
  IGameSystem_AddByName("LightQueryGameSystem");
  v29 = SoundEmitterSystem();
  IGameSystem_Add((__int64)v29);
  v30 = ViewportClientSystem();
  IGameSystem_Add((__int64)v30);
  IGameSystem_AddByName("Color Correction Mgr");
```

**Identification logic:**
1. `IGameSystem_AddByName` is the function called repeatedly with game system name strings like `"GameRulesGameSystem"`, `"RenderGameSystem"`, `"ClientSoundscapeSystem"`, etc. It takes a single `const char*` argument.
2. `IGameSystem_Add` is the function called with a pointer returned by functions like `SoundEmitterSystem()` or `ViewportClientSystem()`. It takes a single pointer argument (the game system instance).
3. Both functions appear in a cluster within `CLoopModeGame_ReceivedServerInfo`, with `IGameSystem_AddByName` calls interleaved with `IGameSystem_Add` calls.

**Key distinguishing features:**
- `IGameSystem_AddByName` is called with string literal arguments (game system names)
- `IGameSystem_Add` is called with return values from other functions (game system singletons)
- The string `"GameRulesGameSystem"` is typically the first `IGameSystem_AddByName` call in the sequence

### 4. Rename the functions (if not already named)

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<IGameSystem_AddByName_addr>", "name": "IGameSystem_AddByName"}, {"addr": "<IGameSystem_Add_addr>", "name": "IGameSystem_Add"}]}
```

### 5. Generate Function Signatures

#### 5a. IGameSystem_AddByName Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `IGameSystem_AddByName`.

#### 5b. IGameSystem_Add Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for `IGameSystem_Add`.

### 6. Write IDA Analysis Output as YAML

#### 6a. Write IGameSystem_AddByName YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystem_AddByName`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 5a

Note: This is a static regular function, NOT a virtual function, so no vtable parameters are needed.

#### 6b. Write IGameSystem_Add YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `IGameSystem_Add`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 5b

Note: This is a static regular function, NOT a virtual function, so no vtable parameters are needed.

## Function Characteristics

### IGameSystem_AddByName
- **Purpose**: Registers a game system by its name string, looking it up from a global registry
- **Type**: Static regular function (not a virtual function, not a member function)
- **Parameters**: `(const char* name)` -- the name of the game system to add
- **Called from**: `CLoopModeGame_ReceivedServerInfo` -- called during game session setup to register known game systems by name
- **Call context**: Called multiple times in succession with different game system name strings

### IGameSystem_Add
- **Purpose**: Registers a game system instance directly by pointer
- **Type**: Static regular function (not a virtual function, not a member function)
- **Parameters**: `(IGameSystem* pGameSystem)` -- pointer to the game system instance to add
- **Called from**: `CLoopModeGame_ReceivedServerInfo` -- called during game session setup to register game system singleton instances
- **Call context**: Called with return values from game system singleton accessor functions (e.g., `SoundEmitterSystem()`, `ViewportClientSystem()`)

## Identification Pattern

Both functions are identified by locating the game system registration block inside `CLoopModeGame_ReceivedServerInfo`:
1. A cluster of calls passing string literals like `"GameRulesGameSystem"`, `"RenderGameSystem"`, etc. identifies `IGameSystem_AddByName`
2. Calls passing pointer return values from singleton accessors (between or after the string calls) identify `IGameSystem_Add`
3. The string `"GameRulesGameSystem"` is a reliable anchor for finding the start of this registration block

This is robust because:
- `CLoopModeGame_ReceivedServerInfo` is reliably found via its vtable
- The game system name strings are distinctive and stable across game updates
- The pattern of interleaved `AddByName` (string arg) and `Add` (pointer arg) calls is unique

## Output YAML Format

The output YAML filenames depend on the platform:
- `client.dll`:
  - `IGameSystem_AddByName.windows.yaml`
  - `IGameSystem_Add.windows.yaml`
- `libclient.so`:
  - `IGameSystem_AddByName.linux.yaml`
  - `IGameSystem_Add.linux.yaml`
