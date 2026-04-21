## CS2 Universal Offsets — v1.3.1

Standalone Windows x64 dumper for live Counter-Strike 2 processes. Reads offsets, schemas, interfaces, signatures, and per-interface vtables from a running `cs2.exe` and emits ready-to-consume artifacts in JSON / C++ / C# / Rust.

**Last verified build:** `14152`
**Sigs resolved:** `220 / 267`
**Vtable method slots dumped:** `7,176` across all resolved interfaces

### What's new since v1.2

- **Per-interface vtable dumper** (`offsets/vtables.{json,hpp,cs}`) — every resolved interface's vftable is walked and indexed. Slots whose RVA matches a known signature are auto-named (e.g. `Source2Client002::update_global_vars = 11`); others fall back to `method_<N>`.
- **+46 string-ref class anchors** — player services, weapon classes, projectiles, gamerules, engine networking. Survive byte-level codegen changes between patches.
- **`bytes` field on every found signature** — 24 wildcard-free hex bytes captured from the resolved function's prologue. StringRef entries now also ship a paste-ready IDA byte pattern alongside the source class string.

### Running

1. Launch CS2 and reach the main menu.
2. Run `cs2-sdk.exe` as Administrator.
3. Output appears under `dumps/<DD-MM-YY>-CS2-SDK/` and is mirrored to `dumps/latest/`.

### Assets

- **cs2-sdk-v1.3.1-windows-x64.zip** — exe + sample `vtables.hpp` + sample `signatures.json`
- **cs2-sdk.exe** — standalone binary (~2.5 MB, no runtime deps)

Source: https://github.com/scros22/cs2-universal-offsets
