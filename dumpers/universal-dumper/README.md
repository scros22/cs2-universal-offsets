<div align="center">

# cs2-universal-offsets

**A single-binary, all-in-one external dumper for Counter-Strike 2.**

Offsets · Interfaces · Schemas · Buttons · Signatures &mdash; one run, one folder, one source of truth.

[![Rust](https://img.shields.io/badge/rust-2024-orange?logo=rust)](https://www.rust-lang.org/)
[![Platform](https://img.shields.io/badge/platform-windows-0078D6?logo=windows)](#requirements)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](#license)
[![Status](https://img.shields.io/badge/status-actively%20updated-success)]()

</div>

---

## What this is

`cs2-universal-offsets` ships a single Rust binary &mdash; `cs2-sdk.exe` &mdash; that performs
**both** of the dumping passes a CS2 internal/external developer normally needs:

| Pass | What it produces | Engine |
| --- | --- | --- |
| **Offsets** | `client_dll`, `engine2_dll`, `interfaces`, `buttons`, full schema dump in `cs / hpp / json / rs / zig` | memflow + pelite, schema walker ported from the well-known `cs2-dumper` |
| **Signatures** | A `signatures.json` with every entry resolved (raw / `rel32` / `rip-rel` / string-ref) | A custom PE/section-aware scanner with IDA-style patterns |

It reads CS2 memory **once** per module, runs both passes against the same snapshot,
and writes everything into a dated session folder so updates from patch to patch are
trivial to diff.

> A new dump is published in this repo on every meaningful CS2 update.

## Credits

This tool stands on the shoulders of two excellent open-source projects:

- **[a2x](https://github.com/a2x)** &mdash; original `cs2-dumper`. The offset / schema / interface /
  button extraction here is a faithful port of his pelite-based pipeline; same scanners,
  same output formats, same well-tested logic.
- **[Eternityforks](https://github.com/Eternityforks)** &mdash; the signature scanner draws
  directly from his `EnhancedScanner` design (section-aware scanning, `rel32` / `rip-rel`
  resolution, string-ref &rarr; function-prologue walk-back).

Huge thanks to both. Without their work this would not exist; this repo is essentially
the union of the two, glued together with a smooth UI and a shared memory pipeline.

## Output layout

Every run produces a dated session folder named `DD-MM-YY-CS2-SDK`:

```
dumps/
  21-04-26-CS2-SDK/
    manifest.json
    logs/
      cs2-sdk.log
    offsets/
      buttons.(cs|hpp|json|rs|zig)
      interfaces.(cs|hpp|json|rs|zig)
      offsets.(cs|hpp|json|rs|zig)
      client_dll.(cs|hpp|json|rs|zig)
      engine2_dll.(cs|hpp|json|rs|zig)
      ...                           (one set per schema module)
      info.json
      # SDK extras (cheat-developer-friendly outputs):
      cs2sdk.hpp                    # single-include amalgamation
      cs2sdk.rs                     # Rust amalgamation module
      netvars.(json|hpp|cs)         # split networked-field offsets
      interfaces_sdk.(hpp|cs)       # typed accessor stubs
      verified_features.(md|json|hpp) # hand-curated catalogue of features
                                      # confirmed working in a live internal
                                      # cheat (no smoke / no flash / smoke
                                      # color / skybox tint / FOV / chams /
                                      # fullbright / third-person / anti-fog
                                      # / no color correction / night mode)
      sdk/
        cs2sdk_macros.hpp           # SCHEMA_FIELD macro family
        client_dll.hpp              # typed schema classes (one per module)
        engine2_dll.hpp
        ...
    signatures/
      signatures.json   # hand-formatted, one entry per line
      signatures.cs     # C#  static class per module
      signatures.hpp    # C++ namespace per module
      signatures.rs     # Rust module per module
      SIGNATURES.md     # human-readable table
      diff.json         # delta vs. previous session (when found)
  latest/                            # mirror of the most recent
                                     # successful session
```

For cheat developers, the most useful single file is
[`offsets/cs2sdk.hpp`](docs/CONSUMING.md) — a single-include header that
gives you typed `SCHEMA_FIELD` accessors, resolved interfaces, netvars,
and signatures.

Full educational guides live in [`docs/`](docs/README.md): pattern
syntax, schema walking, the signature pipeline, how to consume the SDK
from C++/C#/Rust, and how to add a signature.

`signatures.json` is hand-formatted &mdash; one entry per line, columns aligned, with the
exact pattern that produced the hit:

```jsonc
{
  "total_scanned": 211,
  "found":         193,
  "missing":       18,
  "elapsed_ms":    742,
  "modules":       ["client.dll", "engine2.dll", "..."],
  "signatures": [
    { "name": "dwLocalPlayerController", "module": "client.dll", "resolve": "riprel", "va": 0x1812D34A0, "rva": 0x1934A0, "pattern": "48 8B 05 ? ? ? ? 48 85 C0 74 4F 48 8B 40" },
    { "name": "CCSGOInput::GetUserCmd",  "module": "client.dll", "resolve": "rel32",  "va": 0x18119A610, "rva": 0x59A610, "pattern": "E8 ? ? ? ? 48 8B C8 48 8D 15"           }
  ]
}
```

## Quick start

```powershell
# 1. Build (release = small, fast, LTO + strip + single binary)
cargo build --release

# 2. Run as Administrator while CS2 is open
.\target\release\cs2-sdk.exe
```

That's it. The tool will:

1. attach via `memflow-native`,
2. enumerate the CS2 modules it cares about,
3. run the offset / schema / interfaces / buttons pass,
4. run the signature pass,
5. write everything into `dumps\DD-MM-YY-CS2-SDK\`,
6. play short audio cues for start / progress / success / failure (skip with `--no-sound`).

### CLI

| Flag                        | Default                 | Description                                                |
|-----------------------------|-------------------------|------------------------------------------------------------|
| `-c, --connector <NAME>`    | _(native)_              | memflow connector (`pcileech`, `kvm`, ...)                 |
| `-a, --connector-args <A>`  | _(none)_                | extra args for the connector                               |
| `-f, --file-types <LIST>`   | `cs,hpp,json,rs,zig`    | offset output formats                                      |
| `-i, --indent-size <N>`     | `4`                     | indentation used for generated source files                |
| `-o, --output <DIR>`        | `dumps`                 | parent of the dated session folder                         |
| `-p, --process-name <NAME>` | `cs2.exe`               | target process name                                        |
| `-v` / `-vv` / `-vvv`       | warn                    | log verbosity (also written to `logs/cs2-sdk.log`)         |
| `--skip-offsets`            | off                     | run only the signature pass                                |
| `--skip-signatures`         | off                     | run only the offset pass                                   |
| `--no-sound`                | off                     | silence beeps                                              |
| `--cache <PATH>`            | _(auto)_                | warm-cache from a previous `signatures.json`               |
| `--no-cache`                | off                     | disable the auto-detected previous-session cache           |

### Example

```powershell
.\target\release\cs2-sdk.exe --output D:\cs2-sdk -vv
```

## Why a single binary?

- **Faster** &mdash; each target DLL is read once from game memory and reused for every
  stage that needs it; no cross-process orchestration, no duplicate attach/detach work.
- **Smoother** &mdash; one UI, one progress track, one manifest.
- **Cleaner** &mdash; everything a given run produced lives under one dated folder, so
  diffing two CS2 patches is `diff -ruN` between two folders.
- **Safer** &mdash; the well-known offset dumper's logic is kept intact (same `analysis`
  module, same pelite scanners, same outputs); the signature pass is purely additive.

## Signature pass details

The signature database lives in [`src/signatures/database.rs`](src/signatures/database.rs)
and is grouped by source so contributions are easy to review. Each entry opts into
one of four resolution modes:

| Mode | Meaning |
| --- | --- |
| `None`      | The match address is the answer. |
| `Rel32`     | Read `disp32` at `rel_off`; target is `match + rel_off + 4 + disp` (call / jmp). |
| `RipRel`    | Same math, but the target is data (global / vtable / string). |
| `StringRef` | The `needle` is a **literal string**; the scanner finds it in `.rdata`, locates the `.text` `LEA` that references it, and walks back to the enclosing function prologue. This is the Ghidra / IDA *find-by-string* workflow and is very resilient to minor prologue-byte changes between CS2 patches. |

Scans are section-aware (`.text` for code, `.rdata` for data), short-circuit on
first hit per signature, and report timing and per-module statistics.

## Repo layout

```
.
├── assets/              # embedded application icon
├── build.rs             # winresource icon embed (Windows)
├── Cargo.toml
├── src/
│   ├── analysis/        # offset / interface / schema / button scanners (a2x port)
│   ├── memory/          # memflow attach + module cache
│   ├── output/          # cs / hpp / json / rs / zig writers
│   ├── signatures/      # IDA-style scanner + signature database (Eternityforks-inspired)
│   ├── source2/         # Source 2 schema walker
│   ├── ui.rs            # smooth terminal UI + audio cues
│   └── main.rs
└── README.md
```

## Requirements

- **Rust 1.85+** (edition 2024)
- **Windows 10 / 11**
- **Administrator privileges** when using the default `memflow-native` backend
- **CS2 running** (the tool exits cleanly if it can't find the process)

## Contributing

PRs are very welcome &mdash; especially:

- new signatures (drop them in [`src/signatures/database.rs`](src/signatures/database.rs)
  in the appropriate group),
- new output formats,
- bug reports with a `cs2-sdk.log` attached.

If you maintain a community CS2 SDK and want your offset/sig set merged in, open an issue.

The active backlog lives in [TASKS.md](TASKS.md) &mdash; pick anything from there to work on.

## License

MIT. See [LICENSE](LICENSE).

The original projects this is built upon &mdash; **a2x/cs2-dumper** and
**Eternityforks/EnhancedScanner** &mdash; retain their respective licenses; this
repository preserves their authorship in source comments and in this README.
