# Schemas

Source 2 describes its own classes at runtime through the schema system (`schemasystem.dll`). The dumper walks it and emits every class and enum of every module, so field offsets are read from the running game rather than guessed. Build 14184: 3,301 classes and 570 enums across 18 modules (`client.dll` alone: 472 classes).

## `schemas/<module>_dll.hpp`

One header per module; the namespace is the module name (`client::`, `server::`, …). Headers include the ones they need for cross-module types (`client_dll.hpp` includes `server_dll.hpp`), and all of them include `macros.hpp`.

```cpp
namespace client {
    // C_CSPlayerPawn
    //   parent: C_CSPlayerPawnBase
    //   fields: 104
    //   size: 0x3710
    class C_CSPlayerPawn : public C_CSPlayerPawnBase {
    public:
        SCHEMA_FIELD(CCSPlayer_BulletServices*  , m_pBulletServices  , 0x1570) // CCSPlayer_BulletServices*
        SCHEMA_FIELD(CCSPlayer_HostageServices* , m_pHostageServices , 0x1578) // CCSPlayer_HostageServices*
        SCHEMA_FIELD(CCSPlayer_BuyServices*     , m_pBuyServices     , 0x1580) // CCSPlayer_BuyServices*
        …
    };
}
```

`SCHEMA_FIELD(TYPE, NAME, OFFSET)` (from `macros.hpp`) expands to an accessor `NAME()` that returns a `TYPE&` at `this + OFFSET`. There is no data member and no constructor, so a class is a pure view over game memory: cast a pointer and use it.

```cpp
auto* pawn = reinterpret_cast<client::C_CSPlayerPawn*>(addr);
int hp = pawn->m_iHealth();      // declared on C_BaseEntity, reached through the parent chain
pawn->m_iHealth() = 100;
```

Inherited fields are reached through the parent class exactly as in the game's own hierarchy. Source 2 type names (`int32`, `float32`, `GameTime_t`, `CUtlVector<T>`, `CHandle<T>`, …) are provided by `macros.hpp` so the headers compile standalone. Enums are emitted as `enum class` with their underlying width, and every type a field refers to is declared, either in the header or as a forward declaration.

`client_dll.hpp` also carries `client::InputButton`, an enum of the kbutton RVAs (`attack`, `jump`, `duck`, …) with the same values as `buttons.hpp`.

## `schemas/schemas.json`

The same information as data, for tooling and other languages:

```json
{
  "client.dll": {
    "classes": [
      {
        "name": "C_CSPlayerPawn",
        "parent": "C_CSPlayerPawnBase",
        "size": 14096,
        "metadata": [],
        "fields": [
          { "name": "m_pBulletServices", "offset": 5488, "type": "CCSPlayer_BulletServices*", "metadata": [] },
          …
        ]
      }
    ],
    "enums": [
      { "name": "C_BaseCombatCharacter::WaterWakeMode_t", "alignment": 4,
        "members": [ { "name": "WATER_WAKE_NONE", "value": 0, "metadata": [] }, … ] }
    ]
  }
}
```

Offsets and sizes are decimal integers. `metadata` lists the schema attribute names attached to the class, field or enum member (`MNetworkEnable`, `MNetworkVarNames`, `MPropertyDescription`, …).

## Entity handles: `CHandle<T>::Get()`

`CHandle<T>` (in `macros.hpp`) is the 32-bit entity handle used throughout the schema: the low 15 bits are the entity index, the rest is a serial number, and `0xFFFFFFFF` is invalid.

```cpp
template <class T> struct CHandle {
    std::uint32_t m_Handle;
    std::uint32_t GetIndex()  const noexcept;   // m_Handle & 0x7FFF
    std::uint32_t GetSerial() const noexcept;   // m_Handle >> 15
    bool          IsValid()   const noexcept;   // m_Handle != 0xFFFFFFFF
    T*            Get()       const noexcept;   // the entity, or nullptr
};
```

`Get()` resolves the handle through `CGameEntitySystem`: it finds the entity identity for the index and returns the entity only if the identity's stored handle equals this one, so a handle to an entity that has been freed and whose slot was reused returns `nullptr` instead of the wrong entity. The resolvers it calls are defined inline in `impl/entity_system.hpp`; include that header (or `cs2.hpp`) in the translation units that call `Get()`.

```cpp
#include <cs2.hpp>

auto* controller = CGameEntitySystem::GetLocalPlayer();               // client::CCSPlayerController*
auto* pawn = controller ? controller->m_hPlayerPawn().Get() : nullptr; // client::C_CSPlayerPawn*
```

## `impl/entity_system.hpp`

Helpers over the entity list, generated against `offsets.hpp` and the schema headers of the same dump:

| Helper | Returns |
|---|---|
| `CGameEntitySystem::GetHighestEntityIndex()` | Highest live entity index |
| `CGameEntitySystem::GetLocalPlayer()` | The local `CCSPlayerController*` |
| `CGameEntitySystem::GetIdentityByIndex(i)` | `CEntityIdentity*` for an index (chunks of 512 identities, 0x70 bytes each) |
| `CGameEntitySystem::GetEntityByIndex(i)` | The entity pointer, or `nullptr` |
| `CGameEntitySystem::GetDesignerName(i)` | The entity's designer (class) name |
| `CGameEntitySystem::IsPlayerPawn(index)` | Whether a controller on team 2 or 3 owns this pawn index |
| `CS2_GetEntityByIndex`, `CS2_GetEntityByHandle` | The resolvers behind `CHandle<T>::Get()` |

```cpp
for (int i = 1; i <= CGameEntitySystem::GetHighestEntityIndex(); ++i) {
    auto* ent = reinterpret_cast<client::C_BaseEntity*>(CGameEntitySystem::GetEntityByIndex(i));
    if (!ent) continue;
    // …
}
```

## Renamed classes

Valve renames classes between builds and the dump always reflects the current names. `CSPlayerCamera`, for example, is `CCSCustomPlayerCamera` since September 2026 and is emitted under that name on both client and server.

## API

- `GET /api/schema?module=client_dll` — the header text.
- `GET /api/schemas?module=client_dll&class=Pawn` — structured classes and enums; without `module`, a per-module index.
- `POST /api/query` with `"schemas": ["C_CSPlayerPawn.m_iHealth", "CCSPlayerController"]` — a field resolves through the parent chain (`detail` shows `declared_in`); a class name returns all of its fields. See [API](API.md).
