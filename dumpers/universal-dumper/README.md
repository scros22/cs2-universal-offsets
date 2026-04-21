# cs2-universal-dumper

A single-binary, all-in-one external dumper for Counter-Strike 2.

It merges the two previously separate tools in this repository:

- the classic **offset / interface / schema / button** dumper (memflow + pelite)
- the **PE/section-aware signature scanner** with `rel32` / `rip-rel` /
  string-reference resolution

…into one cohesive Rust program with a minimal, smooth terminal UI and
audible cues for start / progress / success / failure.

## Output layout

Every run produces a **dated session folder** named `DD-MM-YY-CS2-SDK`
(e.g. `21-04-26-CS2-SDK`). A run on a given day reuses that folder:

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
      …   (one set per schema module)
      info.json
    signatures/
      signatures.json
```

`signatures.json` contains every scanned entry with:

- `name`, `module`, `resolve` (`raw` / `rel32` / `riprel` / `stringref`)
- `pattern` (IDA pattern or literal string used for string-ref)
- `found`, `match_rva`, `match_va`, `rva`, `va`, `error`
- plus the top-level `total`, `found`, `elapsed_ms`, `modules`

## Usage

```powershell
# Build once (release = small, fast binary, LTO + strip)
cargo build --release

# Run the produced binary
.\target\release\cs2-sdk.exe
```

CS2 must be running and the launcher needs permission to read its memory
(run as Administrator on Windows when using the default `memflow-native`
backend).

### CLI

| Flag                        | Default                 | Description                                                |
|-----------------------------|-------------------------|------------------------------------------------------------|
| `-c, --connector <NAME>`    | _(native)_              | memflow connector (`pcileech`, `kvm`, ...)                 |
| `-a, --connector-args <A>`  | _(none)_                | extra args for the connector                               |
| `-f, --file-types <LIST>`   | `cs,hpp,json,rs,zig`    | offset output formats                                      |
| `-i, --indent-size <N>`     | `4`                     | indentation used for generated source files                |
| `-o, --output <DIR>`        | `dumps`                 | parent of the dated session folder                         |
| `-p, --process-name <NAME>` | `cs2.exe`               | target process                                             |
| `-v` / `-vv` / `-vvv`       | warn                    | log verbosity (written to `logs/cs2-sdk.log`)              |
| `--skip-offsets`            | off                     | run only the signature pass                                |
| `--skip-signatures`         | off                     | run only the offset pass                                   |
| `--no-sound`                | off                     | silence beeps                                              |

### Example

```powershell
.\target\release\cs2-sdk.exe --output D:\cs2-sdk -vv
```

## Why a single binary?

- **Faster**: each target DLL is read once from game memory and reused for
  every stage that needs it; there is no cross-process orchestration or
  duplicate attach/detach work.
- **Smoother**: one UI, one progress track, one manifest.
- **Cleaner**: everything a given run produced lives under one dated folder.
- **Safer**: the well-known offset dumper's logic is kept intact (same
  `analysis` module, same pelite scanners, same outputs) — the signature
  pass is purely additive.

## Signature pass details

The signature database lives in
[`src/signatures/database.rs`](src/signatures/database.rs) and is ported from
the standalone C++ `EnhancedScanner`.  Each entry can opt into one of four
resolution modes:

- `None`      — the match address is the answer.
- `Rel32`     — read `disp32` at `rel_off`, target is `match + rel_off + 4 + disp`.
- `RipRel`    — same math, but the target is data (global / vtable / string).
- `StringRef` — the `needle` is interpreted as a **literal string**; the
  scanner finds it in `.rdata`, locates the `.text` LEA that references it,
  and walks back to the enclosing function prologue.  This is the
  Ghidra / IDA "find-by-string" workflow and is very resilient to minor
  prologue-byte changes between CS2 patches.

## Requirements

- Rust 1.74+ (edition 2024) with `cargo build --release`
- Windows 10/11 (UTF-8 + virtual-terminal console output is enabled
  automatically for the smooth box-drawing glyphs)
- Administrator privileges when using the built-in `memflow-native` backend
