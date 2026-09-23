# cs2-sdk

External CS2 SDK generator. Attaches to a running `cs2.exe` (read-only) and
emits an `include/` tree: schema classes, IDA-style signatures, offsets,
interfaces + vtables, protobuf layouts, engine structs and a set of runtime
catalogues (convars, game events, weapons, entities).

Live browser + API for the latest output: <https://cs2-sdk.com>.

## What you get

```
include/
├── manifest.json             # build number, modules, stage status, signature counts
├── cs2.hpp                   # single-include amalgamation
├── macros.hpp                # SCHEMA_FIELD helpers
├── buttons.{hpp,json}        # kbutton table
├── schemas/
│   ├── <module>_dll.hpp      # per-module schema classes (size + field metadata)
│   └── schemas.json          # structured classes/enums for tooling
├── patterns/
│   ├── patterns.json         # every signature: pattern, rva, prototype, prologue bytes
│   └── patterns.hpp
├── offsets/
│   └── offsets.{hpp,json}    # dwXxx globals (a2x-compatible) + RIP-relative sig globals
├── interfaces/
│   ├── interfaces.hpp        # typed ifc::<module>::<Class> wrappers
│   └── vtables.json          # primary vtable of every registered interface
├── protobufs/protobufs.{hpp,json}
├── engine/                   # hand-verified non-schema structs + drop-in .h each:
│   └── engine_structs.json   #   CCSGOInput, CUserCmd, CCSGOUserCmdPB, CBaseUserCmdPB,
│                             #   CSubtickMoveStep, CInButtonStatePB, CCSGOInputHistoryEntryPB,
│                             #   CSGOInterpolationInfoPB, CMsgQAngle, CMsgVector, CViewSetup
├── convars/convars.{json,hpp}      # every ConVar / ConCommand with type, value, flags
├── gameevents/gameevents.json      # every registered game event + typed keys
├── weapons/weapons.json            # CCSWeaponBaseVData of weapons present at dump time
├── entities/entities.json          # live entity snapshot at dump time
└── verified_features.json
```

Drop the repo in as a git submodule and `#include "cs2-universal-offsets/include/cs2.hpp"`.

## Build

```
cargo build --release
```

Rust 2024 edition. Windows-only by default (uses `memflow-native`); pass
`--connector` for other memflow connectors.

## Run

Start CS2, then from an **elevated** prompt (memflow needs admin to open the
process):

```
.\target\release\cs2-sdk.exe
```

For the fullest weapons/entities catalogues, dump while in a match.

| flag | default | what it does |
|---|---|---|
| `-o, --output <DIR>` | `include` | output root |
| `-p, --process-name <NAME>` | `cs2.exe` | target process |
| `--skip-offsets` | off | skip interfaces/offsets/schemas |
| `--skip-patterns` | off | skip the signature pass |
| `--no-sound` | off | silence the UI cues |
| `-v / -vv / -vvv` | warn | terminal log verbosity (the file log is always trace) |

## How it stays correct across CS2 updates

* Signatures resolve by pattern (`Rel32` / `RipRel` / raw with `extra_off`),
  and every hit records how many times it matched, so an ambiguous pattern is
  visible in `patterns.json` instead of silently resolving to the wrong place.
* Engine-struct function and instance addresses are looked up from the
  signature pass by name, never hardcoded.
* The weapon and entity walkers take their field offsets from the schema
  dumped in the same run, and only fall back to last-known values (logged)
  when the schema pass is skipped.

## Output guarantees

* `patterns/patterns.json`, `offsets/offsets.json` and `schemas/*.hpp` are the
  public contract consumed by cs2-sdk.com - their shape will not break in a
  minor version.
* Per-module schema headers with no classes or enums are skipped.

## License

MIT.
