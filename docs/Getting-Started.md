# Getting Started

There are three ways to use the project. Most people only need the first two.

## 1. Use the data without running anything

Everything the dumper produces for the current build is served by [cs2-sdk.com](https://cs2-sdk.com):

- **Browse it** — every dataset has a tab, `Ctrl+K` searches all of them, and you can bookmark items into collections and export them. See [Website](Website.md).
- **Query it** — `GET /api/offset/<name>`, `GET /api/pattern/<name>`, or `POST /api/query` for many names in one request. See [API](API.md).
- **Download it** — every generated file is served verbatim under `/raw/`: `https://cs2-sdk.com/raw/cs2.hpp`, `/raw/offsets.json`, `/raw/patterns/patterns.json`, `/raw/schemas/client_dll.hpp`, … `GET /api` lists them all.

## 2. Use the committed `include/` tree

The repository always carries the latest dump under `include/`. Add it as a submodule, put `include/` on your include path and include the single amalgamation header:

```bash
git submodule add https://github.com/scros22/cs2-universal-offsets.git external/cs2-sdk
```

```cpp
#include <cs2.hpp>   // external/cs2-sdk/include is on the include path

static_assert(CS2_BUILD == 14183, "regenerate the SDK for this CS2 build");

void heal(client::C_CSPlayerPawn* pawn) {
    if (pawn->m_iHealth() < 100)
        pawn->m_iHealth() = 100;
}
```

`cs2.hpp` pulls in every per-module schema header, `offsets.hpp`, `interfaces.hpp`, `buttons.hpp`, `protobufs.hpp`, `patterns.hpp` and the entity helpers. If you only need part of it, include the individual headers listed in [Output Layout](Output-Layout.md); each one is self-contained apart from `macros.hpp`.

After a CS2 patch, `git submodule update --remote external/cs2-sdk`. Each generated header records the build it was dumped from (`CS2_BUILD`, `ifc::CS2_BUILD`, `pb::CS2_BUILD`, `entity::CS2_BUILD`), so a stale SDK can fail at compile time instead of at runtime.

## 3. Run the dumper yourself

### Get a binary

Download `cs2-sdk.exe` from the [latest release](https://github.com/scros22/cs2-universal-offsets/releases/latest), or build it:

```bash
cargo build --release      # Rust 2024 edition -> target/release/cs2-sdk.exe
```

The default memflow connector is `memflow-native`, which is why the tool is Windows-only out of the box. Any other memflow connector can be selected with `--connector` / `--connector-args`.

### Run it

1. Start CS2. For the fullest `weapons.json` and `entities.json`, be in a match: those two catalogues are read from the entities that exist at dump time.
2. Open an **elevated** prompt (memflow needs administrator rights to open the process).
3. Run:

```powershell
.\cs2-sdk.exe
```

The SDK is written to `.\include\`. A full trace log is always written to `include\cs2-sdk.log`; `-v` / `-vv` / `-vvv` raise the terminal verbosity. The tool plays a short audio cue when a run finishes; `--no-sound` silences it.

| Flag | Default | Effect |
|---|---|---|
| `-o, --output <DIR>` | `include` | Output root |
| `-p, --process-name <NAME>` | `cs2.exe` | Process to attach to |
| `-c, --connector <NAME>` | memflow-native | memflow connector |
| `-a, --connector-args <ARGS>` | — | Arguments for the connector |
| `-i, --indent-size <N>` | `4` | Indentation of generated headers |
| `--skip-offsets` | off | Skip interfaces, globals and schemas |
| `--skip-patterns` | off | Skip the signature pass |
| `--no-sound` | off | No audio cue |
| `-v`, `-vv`, `-vvv` | warn | Terminal log level (the file log is always trace) |

`include/manifest.json` tells you whether the run was complete: `offsets_ok`, `signatures_ok`, and `signature_counts.found` / `.total`.

### What the dumper does not do

It never writes to the game, injects anything or hooks anything: it opens the process for reading, walks it and exits. What you do with the output is your responsibility.
