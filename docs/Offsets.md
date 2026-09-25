# Offsets

"Offsets" here means **resolved globals**: addresses relative to a module base that hold a pointer or an object you read at runtime. Field offsets inside classes are a different thing — see [Schemas](Schemas.md).

The dumper resolves about two hundred globals per build, of three kinds:

| `kind` | Count | Where they come from | Examples |
|---|---|---|---|
| `global` | 32 | The classic pattern-scanned globals, with the same names, patterns and semantics as the a2x dumper, so existing code keeps working and the two dumps can be diffed | `dwEntityList`, `dwLocalPlayerPawn`, `dwViewMatrix`, `dwGlobalVars` |
| `signature` | 62 | `riprel` entries of the signature database ([Signatures](Signatures)): a RIP-relative load inside a verified function | `pGameRules`, `pCSGOInputInstance`, `pMaterialManager`, `pEconItemSystem` |
| `interface` | 111 | Every interface registered through `CreateInterface`, resolved to the object the factory returns | `Source2Client002`, `InputSystemVersion001`, `EngineTraceClient001` |

## Files

### `offsets/offsets.json` — the 29 classic globals

```json
{
  "client.dll": {
    "dwEntityList": "0x27130E8",
    "dwLocalPlayerPawn": "0x255E658",
    "dwViewMatrix": "0x25639A0",
    …
  },
  "engine2.dll": { … }
}
```

Names and semantics follow the a2x conventions the community already uses, and the values are cross-checked against a2x's dump of the same build. Most hold a pointer that you read once (`dwLocalPlayerPawn`, `dwLocalPlayerController`, `dwEntityList`, `dwGameRules`, `dwGlobalVars`); `dwViewMatrix` and `dwViewAngles` are the data itself.

A few entries are member offsets rather than module-relative addresses, and are named `dwOwner_member`:

| Name | Meaning |
|---|---|
| `dwGameEntitySystem_highestEntityIndex` | offset of the highest-index field inside `CGameEntitySystem` |
| `dwSensitivity_sensitivity` (`0x58`) | `client + dwSensitivity` holds a `ConVarInfo_t*` for the `sensitivity` ConVar; the float value is at that offset inside it (the tier0 ConVar value union) |
| `dwNetworkGameClient_*` | fields of `CNetworkGameClient` (`engine2 + dwNetworkGameClient` holds the pointer) |
| `dwSoundSystem_engineViewData` (`0x6C`) | the view block (origin, then angles at `+0x10`) inside the sound system object at `soundsystem + dwSoundSystem` |

### `offsets/offsets_all.json` — every global, tagged

```json
{
  "count": 202,
  "modules": {
    "client.dll": [
      { "name": "dwEntityList",     "hpp_name": "EntityList",       "rva": "0x27130E8", "kind": "global" },
      { "name": "pGameRules",       "hpp_name": "GameRules",        "rva": "0x255A858", "kind": "signature" },
      { "name": "Source2Client002", "hpp_name": "Source2Client002", "rva": "0x…",       "kind": "interface", "deref": false },
      …
    ]
  }
}
```

`hpp_name` is the identifier used in `offsets.hpp`. For interfaces, `deref: false` means `module + rva` is the interface object itself.

### `offsets/offsets.hpp` — everything, as constants

```cpp
namespace offsets {
    namespace client {
        constexpr std::ptrdiff_t EntityList = 0x27130E8;
        constexpr std::ptrdiff_t GameRules = 0x255A858;
        constexpr std::ptrdiff_t LocalPlayerPawn = 0x255E658;
        constexpr std::ptrdiff_t Prediction = 0x255E560;
        …
    }
    namespace inputsystem {
        constexpr std::ptrdiff_t InputSystemVersion001 = 0x46BC0;
    }
}
```

The three kinds are merged into one tree per module. Identifiers drop the `dw` / `p` / `_ptr` affixes and replace `::` with `_`; the JSON names are the ones to use with the API (both spellings resolve there).

## Reading them

```cpp
#include <cs2.hpp>

auto client = reinterpret_cast<std::uintptr_t>(GetModuleHandleA("client.dll"));

// a pointer-holding global: read once
auto* local_pawn = *reinterpret_cast<client::C_CSPlayerPawn**>(client + offsets::client::LocalPlayerPawn);

// inline data
auto* view_matrix = reinterpret_cast<const float*>(client + offsets::client::ViewMatrix);   // 4x4

// an interface instance: the address is the object
auto* input = reinterpret_cast<ifc::inputsystem::CInputSystem*>(
    reinterpret_cast<std::uintptr_t>(GetModuleHandleA("inputsystem.dll")) + offsets::inputsystem::InputSystemVersion001);
```

## API

- `GET /api/offset/dwLocalPlayerPawn` — one global across modules, with `value_dec`.
- `GET /api/offsets` — `{module: {name: "0x…"}}` for every global; `?module=client.dll` to filter.
- `GET /api/offsets?detail=1` — the tagged rows of `offsets_all.json`; `?kind=global|signature|interface` and `?module=` filter.
- `POST /api/query` with `"offsets": ["dwEntityList", "client/GameRules"]` — names resolve with or without the `dw` prefix and by their `offsets.hpp` identifier; `module/` pins a module. See [API](API.md).
