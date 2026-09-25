# Output Layout

A run writes one `include/` tree. Everything in it is generated; nothing is hand-edited.

```text
include/
├── manifest.json                 build number, modules, stage status, signature counts
├── cs2.hpp                       single-include amalgamation (defines CS2_BUILD)
├── macros.hpp                    SCHEMA_FIELD, Source 2 type aliases, CHandle<T>
├── buttons.{hpp,json}            kbutton table (button::attack, …)
├── cs2-sdk.log                   full trace log of the run
│
├── schemas/
│   ├── <module>_dll.hpp          typed schema classes and enums, one header per module
│   └── schemas.json              the same classes and enums as structured data
├── patterns/
│   ├── patterns.json             every signature: pattern, RVA, prototype, prologue bytes, aliases
│   └── patterns.hpp              pattern::<module>::Name string_views
├── offsets/
│   ├── offsets.json              {module: {dwXxx: "0x…"}} — the 29 classic globals, a2x-compatible names
│   ├── offsets.hpp               offsets::<module>::Name — every resolved global (202)
│   └── offsets_all.json          every resolved global with its kind (global / signature / interface)
├── interfaces/
│   ├── interfaces.hpp            ifc::<module>::<Class> vtable structs
│   └── vtables.json              primary vtable of every registered interface, RTTI class names
├── protobufs/
│   ├── protobufs.hpp             #pragma pack(1) message structs with static_asserts
│   └── protobufs.json            field offsets, numbers, has-bits, wire types
├── engine/
│   ├── engine_structs.json       hand-verified non-schema layouts
│   └── <struct>.h                one drop-in header per struct (cusercmd.h, ccsgoinput.h, …)
├── impl/
│   └── entity_system.hpp         CGameEntitySystem helpers and the CHandle<T>::Get() resolvers
├── convars/convars.{json,hpp}    every ConVar / ConCommand with type, value, flags, description
├── gameevents/gameevents.json    every registered game event with typed keys
├── weapons/weapons.json          CCSWeaponBaseVData of the weapons present at dump time
├── entities/entities.json        live entity snapshot at dump time
└── verified_features.json        hand-verified feature recipes: the fields and hooks a working internal uses
```

## Which file do I want?

| I want to… | Use |
|---|---|
| Write an internal in C++ | `cs2.hpp`, or the individual headers below |
| Find functions at runtime | `patterns/patterns.json` or `patterns/patterns.hpp` — [Signatures](Signatures.md) |
| Read the classic `dwXxx` globals | `offsets/offsets.json` (same names as the a2x dumper) — [Offsets](Offsets.md) |
| Read every global, including interface instances | `offsets/offsets_all.json` or `offsets/offsets.hpp` |
| Get field offsets from another language | `schemas/schemas.json` — [Schemas](Schemas.md) |
| Call or hook interface methods | `interfaces/interfaces.hpp` + `interfaces/vtables.json` — [Interfaces and Vtables](Interfaces-and-Vtables.md) |
| Read or build user commands | `engine/*.h` + `protobufs/protobufs.hpp` — [Engine Structs](Engine-Structs.md), [Protobufs](Protobufs.md) |
| Look up a convar, event or weapon value | the catalogues — [Catalogues](Catalogues.md) |
| Check that a run was complete | `manifest.json` |

## Namespaces

| Header | Namespace | Build constant |
|---|---|---|
| `cs2.hpp` | — | `CS2_BUILD` (global) |
| `schemas/<module>_dll.hpp` | `client::`, `server::`, `engine2::`, … (module name without `_dll`) | — |
| `offsets/offsets.hpp` | `offsets::<module>::Name` | — |
| `patterns/patterns.hpp` | `pattern::<module>::Name` | — |
| `interfaces/interfaces.hpp` | `ifc::<module>::<Class>` | `ifc::CS2_BUILD` |
| `protobufs/protobufs.hpp` | `pb::<module>::<Message>` | `pb::CS2_BUILD` |
| `buttons.hpp` | `button::<name>` | — |
| `engine/<struct>.h` | `<Struct>::` (for example `CUserCmd::m_nCommandNumber`) | in the file banner |
| `impl/entity_system.hpp` | `CGameEntitySystem` (a global struct) | `entity::CS2_BUILD` |

Module namespaces are the DLL name without the extension: `client`, `server`, `engine2`, `schemasystem`, `animationsystem`, `materialsystem2`, `particles`, `scenesystem`, `soundsystem`, `tier0`, `vphysics2`, `networksystem`, `host`, `panorama`, `rendersystemdx11`, `resourcesystem`, `pulse_system`, `inputsystem`, `filesystem_stdio`.

## Formats

- Every `.json` file is strict JSON. Addresses are hex strings (`"0xB65A70"`), except in `vtables.json` and `buttons.json`, where RVAs are plain integers.
- Offsets inside `schemas.json` and `protobufs.json` are decimal integers; the headers print them in hex.
- Headers are UTF-8, `#pragma once`, and compile standalone with a C++17 compiler.
- Per-module schema headers with no classes or enums are not written.
- `patterns.json` is written one entry per line with aligned columns, so it diffs cleanly between builds.

## Stability

`patterns/patterns.json`, `offsets/offsets.json` and `schemas/*.hpp` are the contract that cs2-sdk.com consumes. Their shape does not change within a major version: fields may be added, existing ones are not renamed or removed.
