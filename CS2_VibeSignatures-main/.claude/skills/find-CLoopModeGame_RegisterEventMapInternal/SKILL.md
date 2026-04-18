---
name: find-CLoopModeGame_RegisterEventMapInternal_client
description: Find and identify CLoopModeGame_RegisterEventMapInternal, RegisterEventListener_Abstract, and CLoopModeGame_OnXXXXXXX event handler functions in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 client binary to locate these functions by searching for the "CLoopModeGame::OnClientPollNetworking" string reference.
disable-model-invocation: true
---

# Find CLoopModeGame_RegisterEventMapInternal

Locate `CLoopModeGame_RegisterEventMapInternal`, `RegisterEventListener_Abstract`, and all `CLoopModeGame_OnXXXXXXX` event handler functions in CS2 `client.dll` or `libclient.so` using IDA Pro MCP tools.

## Method

### 1. Search for the anchor string

```
mcp__ida-pro-mcp__find_regex pattern="CLoopModeGame::OnClientPollNetworking"
```

### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

There should be exactly one code xref — this is inside `CLoopModeGame_RegisterEventMapInternal`.

### 3. Decompile the referencing function

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

### 4. Match the characteristic code pattern

The function takes 4 parameters `(a1, a2, a3, a4)` and has two main branches:
- When `a3 != 0`: Unregister events branch
- When `a3 == 0`: Register events branch (the one we care about)

In the register branch, look for repeated sequences of this pattern:

```c
// Each event registration follows this pattern:
unknown_libname_XXX(&v_local, &unk_XXXXXXXX);   // Get event descriptor
v_temp = v_local;
v_callback = sub_XXXXXXXX;                        // <-- This is a CLoopModeGame_OnXXXXXXX callback
v_handle = sub_XXXXXXXX(a2);                       // Get handle from a2
sub_XXXXXXXX(a1, &v_handle, 1, 1, v_temp, a4, "CLoopModeGame::OnXXXXXXX");  // <-- This is RegisterEventListener_Abstract
```

Key identifiers:
- **`CLoopModeGame_RegisterEventMapInternal`**: The outer function containing all the event registrations
- **`RegisterEventListener_Abstract`**: The function called with 7 arguments including the event name string as the last argument — it is the same function called for every event registration in the function
- **`CLoopModeGame_OnClientPollNetworking`**: The callback function pointer stored before the call with string `"CLoopModeGame::OnClientPollNetworking"`
- **`CLoopModeGame_OnClientAdvanceTick`**: The callback function pointer stored before the call with string `"CLoopModeGame::OnClientAdvanceTick"`
- **`CLoopModeGame_OnClientPostAdvanceTick`**: The callback for `"CLoopModeGame::OnClientPostAdvanceTick"`
- **`CLoopModeGame_OnClientPreSimulate`**: The callback for `"CLoopModeGame::OnClientPreSimulate"`
- **`CLoopModeGame_OnClientPreOutput`**: The callback for `"CLoopModeGame::OnClientPreOutput"`
- **`CLoopModeGame_OnClientPreOutputParallelWithServer`**: The callback for `"CLoopModeGame::OnClientPreOutputParallelWithServer"`
- **`CLoopModeGame_OnClientPostOutput`**: The callback for `"CLoopModeGame::OnClientPostOutput"`
- **`CLoopModeGame_OnClientFrameSimulate`**: The callback for `"CLoopModeGame::OnClientFrameSimulate"`
- **`CLoopModeGame_OnClientAdvanceNonRenderedFrame`**: The callback for `"CLoopModeGame::OnClientAdvanceNonRenderedFrame"`
- **`CLoopModeGame_OnClientPostSimulate`**: The callback for `"CLoopModeGame::OnClientPostSimulate"`
- **`CLoopModeGame_OnClientPauseSimulate`**: The callback for `"CLoopModeGame::OnClientPauseSimulate"`
- **`CLoopModeGame_OnClientSimulate`**: The callback for `"CLoopModeGame::OnClientSimulate"`
- **`CLoopModeGame_OnPostDataUpdate`**: The callback for `"CLoopModeGame::OnPostDataUpdate"`
- **`CLoopModeGame_OnPreDataUpdate`**: The callback for `"CLoopModeGame::OnPreDataUpdate"`
- **`CLoopModeGame_OnFrameBoundary`**: The callback for `"CLoopModeGame::OnFrameBoundary"`

### 5. Identify all event handler callbacks

Scan through the entire register branch and collect ALL callback function addresses paired with their event name strings. The following 15 callbacks should be found:

1. `CLoopModeGame_OnClientPollNetworking` — `"CLoopModeGame::OnClientPollNetworking"`
2. `CLoopModeGame_OnClientAdvanceTick` — `"CLoopModeGame::OnClientAdvanceTick"`
3. `CLoopModeGame_OnClientPostAdvanceTick` — `"CLoopModeGame::OnClientPostAdvanceTick"`
4. `CLoopModeGame_OnClientPreSimulate` — `"CLoopModeGame::OnClientPreSimulate"`
5. `CLoopModeGame_OnClientPreOutput` — `"CLoopModeGame::OnClientPreOutput"`
6. `CLoopModeGame_OnClientPreOutputParallelWithServer` — `"CLoopModeGame::OnClientPreOutputParallelWithServer"`
7. `CLoopModeGame_OnClientPostOutput` — `"CLoopModeGame::OnClientPostOutput"`
8. `CLoopModeGame_OnClientFrameSimulate` — `"CLoopModeGame::OnClientFrameSimulate"`
9. `CLoopModeGame_OnClientAdvanceNonRenderedFrame` — `"CLoopModeGame::OnClientAdvanceNonRenderedFrame"`
10. `CLoopModeGame_OnClientPostSimulate` — `"CLoopModeGame::OnClientPostSimulate"`
11. `CLoopModeGame_OnClientPauseSimulate` — `"CLoopModeGame::OnClientPauseSimulate"`
12. `CLoopModeGame_OnClientSimulate` — `"CLoopModeGame::OnClientSimulate"`
13. `CLoopModeGame_OnPostDataUpdate` — `"CLoopModeGame::OnPostDataUpdate"`
14. `CLoopModeGame_OnPreDataUpdate` — `"CLoopModeGame::OnPreDataUpdate"`
15. `CLoopModeGame_OnFrameBoundary` — `"CLoopModeGame::OnFrameBoundary"`

For each registration block:
1. The callback function pointer is assigned to a local variable just before the `RegisterEventListener_Abstract` call
2. The event name string (last argument) tells you the callback name — strip the `::` and replace with `_`

### 6. Check if the functions are already renamed

```
mcp__ida-pro-mcp__lookup_funcs queries=["<RegisterEventMapInternal_addr>", "<RegisterEventListener_Abstract_addr>", "<callback1_addr>", "<callback2_addr>", ...]
```

### 7. Rename all functions that are still unnamed (`sub_` prefix)

```
mcp__ida-pro-mcp__rename batch={"func": [
  {"addr": "<addr>", "name": "CLoopModeGame_RegisterEventMapInternal"},
  {"addr": "<addr>", "name": "RegisterEventListener_Abstract"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPollNetworking"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientAdvanceTick"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPostAdvanceTick"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPreSimulate"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPreOutput"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPreOutputParallelWithServer"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPostOutput"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientFrameSimulate"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientAdvanceNonRenderedFrame"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPostSimulate"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientPauseSimulate"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnClientSimulate"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnPostDataUpdate"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnPreDataUpdate"},
  {"addr": "<addr>", "name": "CLoopModeGame_OnFrameBoundary"}
]}
```

### 8. Generate and validate unique signatures

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for each function:

- `CLoopModeGame_RegisterEventMapInternal`
- `RegisterEventListener_Abstract`
- `CLoopModeGame_OnClientPollNetworking`
- `CLoopModeGame_OnClientAdvanceTick`
- `CLoopModeGame_OnClientPostAdvanceTick`
- `CLoopModeGame_OnClientPreSimulate`
- `CLoopModeGame_OnClientPreOutput`
- `CLoopModeGame_OnClientPreOutputParallelWithServer`
- `CLoopModeGame_OnClientPostOutput`
- `CLoopModeGame_OnClientFrameSimulate`
- `CLoopModeGame_OnClientAdvanceNonRenderedFrame`
- `CLoopModeGame_OnClientPostSimulate`
- `CLoopModeGame_OnClientPauseSimulate`
- `CLoopModeGame_OnClientSimulate`
- `CLoopModeGame_OnPostDataUpdate`
- `CLoopModeGame_OnPreDataUpdate`
- `CLoopModeGame_OnFrameBoundary`

### 9. Write IDA analysis output as YAML beside the binary

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results for each function.

#### For `CLoopModeGame_RegisterEventMapInternal`:
Required parameters:
- `func_name`: `CLoopModeGame_RegisterEventMapInternal`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 8

#### For `RegisterEventListener_Abstract`:
Required parameters:
- `func_name`: `RegisterEventListener_Abstract`
- `func_addr`: The function address from step 4
- `func_sig`: The validated signature from step 8

#### For each callback:
Required parameters:
- `func_name`: The callback name from the list below
- `func_addr`: The callback function address from step 5
- `func_sig`: The validated signature from step 8

Callbacks to write:
- `CLoopModeGame_OnClientPollNetworking`
- `CLoopModeGame_OnClientAdvanceTick`
- `CLoopModeGame_OnClientPostAdvanceTick`
- `CLoopModeGame_OnClientPreSimulate`
- `CLoopModeGame_OnClientPreOutput`
- `CLoopModeGame_OnClientPreOutputParallelWithServer`
- `CLoopModeGame_OnClientPostOutput`
- `CLoopModeGame_OnClientFrameSimulate`
- `CLoopModeGame_OnClientAdvanceNonRenderedFrame`
- `CLoopModeGame_OnClientPostSimulate`
- `CLoopModeGame_OnClientPauseSimulate`
- `CLoopModeGame_OnClientSimulate`
- `CLoopModeGame_OnPostDataUpdate`
- `CLoopModeGame_OnPreDataUpdate`
- `CLoopModeGame_OnFrameBoundary`

Note: These are all regular functions, NOT virtual functions, so no vtable parameters are needed.

## Function Characteristics

### CLoopModeGame_RegisterEventMapInternal

- **Prototype**: `void CLoopModeGame_RegisterEventMapInternal(void *pLoopModeGame, void *a2, int bUnregister, int a4)`
- **Parameters**:
  - `pLoopModeGame`: Pointer to the CLoopModeGame instance
  - `a2`: Context parameter passed to event handle creation
  - `bUnregister`: When non-zero, unregisters events; when zero, registers events
  - `a4`: Additional parameter passed through to RegisterEventListener_Abstract
- **Behavior**: Registers (or unregisters) multiple game event listeners for the CLoopModeGame class

### RegisterEventListener_Abstract

- **Prototype**: `void RegisterEventListener_Abstract(void *pListener, void *pHandle, int a3, int a4, void *pEventDescriptor, int a6, const char *pszEventName)`
- **Parameters**:
  - `pListener`: The listener object (CLoopModeGame instance)
  - `pHandle`: Event handle
  - `a3`: Flag (typically 1)
  - `a4`: Flag (typically 1)
  - `pEventDescriptor`: Event descriptor pointer
  - `a6`: Additional parameter
  - `pszEventName`: Debug name string like `"CLoopModeGame::OnClientPollNetworking"`

### Event Callbacks

The following 15 callbacks are individual event handler functions for various game loop events:

- `CLoopModeGame_OnClientPollNetworking`
- `CLoopModeGame_OnClientAdvanceTick`
- `CLoopModeGame_OnClientPostAdvanceTick`
- `CLoopModeGame_OnClientPreSimulate`
- `CLoopModeGame_OnClientPreOutput`
- `CLoopModeGame_OnClientPreOutputParallelWithServer`
- `CLoopModeGame_OnClientPostOutput`
- `CLoopModeGame_OnClientFrameSimulate`
- `CLoopModeGame_OnClientAdvanceNonRenderedFrame`
- `CLoopModeGame_OnClientPostSimulate`
- `CLoopModeGame_OnClientPauseSimulate`
- `CLoopModeGame_OnClientSimulate`
- `CLoopModeGame_OnPostDataUpdate`
- `CLoopModeGame_OnPreDataUpdate`
- `CLoopModeGame_OnFrameBoundary`

- **Prototype**: `void CLoopModeGame_OnXXXXXXX(void *pEvent)` (exact signature may vary)
- **Purpose**: Individual event handler callbacks for various game loop events

## DLL Information

- **DLL**: `client.dll` (Windows) / `libclient.so` (Linux)

## Notes

- All functions are regular functions, NOT virtual functions
- `RegisterEventListener_Abstract` is the same function called for every event registration — verify all calls reference the same address
- The number of event callbacks may vary between game versions — collect ALL of them from the register branch
- Each callback is uniquely identified by its paired event name string

## Output YAML Format

The output YAML filenames depend on the platform:
- `client.dll` → `<func_name>.windows.yaml`
- `libclient.so` → `<func_name>.linux.yaml`

The following 17 YAML files should be generated (where `{platform}` is `windows` or `linux`):
- `CLoopModeGame_RegisterEventMapInternal.{platform}.yaml`
- `RegisterEventListener_Abstract.{platform}.yaml`
- `CLoopModeGame_OnClientPollNetworking.{platform}.yaml`
- `CLoopModeGame_OnClientAdvanceTick.{platform}.yaml`
- `CLoopModeGame_OnClientPostAdvanceTick.{platform}.yaml`
- `CLoopModeGame_OnClientPreSimulate.{platform}.yaml`
- `CLoopModeGame_OnClientPreOutput.{platform}.yaml`
- `CLoopModeGame_OnClientPreOutputParallelWithServer.{platform}.yaml`
- `CLoopModeGame_OnClientPostOutput.{platform}.yaml`
- `CLoopModeGame_OnClientFrameSimulate.{platform}.yaml`
- `CLoopModeGame_OnClientAdvanceNonRenderedFrame.{platform}.yaml`
- `CLoopModeGame_OnClientPostSimulate.{platform}.yaml`
- `CLoopModeGame_OnClientPauseSimulate.{platform}.yaml`
- `CLoopModeGame_OnClientSimulate.{platform}.yaml`
- `CLoopModeGame_OnPostDataUpdate.{platform}.yaml`
- `CLoopModeGame_OnPreDataUpdate.{platform}.yaml`
- `CLoopModeGame_OnFrameBoundary.{platform}.yaml`
