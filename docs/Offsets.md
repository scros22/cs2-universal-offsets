# Offsets

"Offsets" here means **resolved globals**: addresses relative to a module base that hold a pointer or an object you read at runtime. Field offsets inside classes are a different thing — see [Schemas](Schemas.md).

The dumper resolves 202 globals on build 14183, of three kinds:

| `kind` | Count | Where they come from | Examples |
|---|---|---|---|
| `global` | 29 | The classic pattern-scanned globals, with the same `dwXxx` names the a2x dumper uses, so existing code keeps working | `dwEntityList`, `dwLocalPlayerPawn`, `dwViewMatrix`, `dwGlobalVars` |
| `signature` | 62 | `riprel` entries of the signature database ([Signatures](Signatures.md)): a RIP-relative load inside a verified function | `pGameRules`, `pCSGOInputInstance`, `pMaterialManager`, `pEconItemSystem` |
| `interface` | 111 | Every interface registered through `CreateInterface`, resolved to the object the factory returns | `Source2Client002`, `InputSystemVersion001`, `EngineTraceClient001` |

## Files

### `offsets/offsets.json` — the 29 classic globals

```json
{
  "client.dll": {
    "dwEntityList": "0x2711048",
    "dwLocalPlayerPawn": "0x255C5A8",
    "dwViewMatrix": "0x25618F0",
    …
  },
  "engine2.dll": { … }
}
```

Names and semantics follow the a2x conventions the community already uses. Most hold a pointer that you read once (`dwLocalPlayerPawn`, `dwLocalPlayerController`, `dwEntityList`, `dwGameRules`, `dwGlobalVars`); `dwViewMatrix` and `dwViewAngles` are the data itself. `dwGameEntitySystem_highestEntityIndex` is the one value that is not relative to a module base: it is the offset of the highest-index field *inside* `CGameEntitySystem`.

### `offsets/offsets_all.json` — every global, tagged

```json
{
  "count": 202,
  "modules": {
    "client.dll": [
      { "name": "dwEntityList",     "hpp_name": "EntityList",       "rva": "0x2711048", "kind": "global" },
      { "name": "pGameRules",       "hpp_name": "GameRules",        "rva": "0x1BADA70", "kind": "signature" },
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
        constexpr std::ptrdiff_t EntityList = 0x2711048;
        constexpr std::ptrdiff_t GameRules = 0x1BADA70;
        constexpr std::ptrdiff_t LocalPlayerPawn = 0x255C5A8;
        constexpr std::ptrdiff_t Prediction = 0x255C4B0;
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
