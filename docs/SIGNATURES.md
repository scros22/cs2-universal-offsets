# Signature Pass: End-to-End

This document walks through a single run of the signature pass — from
the entry in `src/signatures/database.rs` all the way to the emitted
multi-language consumer files.

## 1. Anatomy of a Signature Entry

```rust
SignatureEntry {
    name:    "CL_RecvUserCmds",
    module:  "client.dll",
    pattern: "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 81 EC 50 02 ?? ??",
    section: ".text",
    resolve: ResolveKind::None,
}
```

Five fields:

- `name` — symbol-style identifier; becomes a constant in every emitter.
- `module` — the DLL the pattern lives in.
- `pattern` — IDA-style bytes (see [PATTERNS.md](PATTERNS.md)).
- `section` — which PE section to scan (almost always `.text`).
- `resolve` — what to do with the match address (`None` / `Rel32` /
  `RipRel` / `StringRef`).

## 2. Scan Pipeline

For each entry the scanner:

1. **Resolves the module** from the live process via memflow.
2. **Slices the section bytes** with pelite — `.text` for code,
   `.rdata` for string-ref needles.
3. **Runs `find_all_pattern`** to count matches and pick the canonical
   first one. The match count is recorded in
   `SignatureHit::matches` so consumers can flag ambiguous patterns.
4. **Applies the resolver** to get the final RVA (see PATTERNS.md §2).
5. **Adds the module base** at runtime to produce a VA and stashes
   both in `SignatureHit { rva, va, ... }`.

Failures are recorded as `SignatureHit { found: false, error: Some(_),
... }` rather than aborting — one bad pattern doesn't take the rest
down.

## 3. Warm Cache

A cache file is just a previous `signatures.json`. When loaded:

```rust
let cache = SignatureCache::load("dumps/.../signatures/signatures.json")?;
```

For every entry whose `(module, name, pattern)` triple is in the cache,
the scanner re-validates the cached `match_rva` against the recorded
pattern bytes in the live module. If the bytes still match, the entry
is satisfied without a full module scan; otherwise it falls back to a
fresh scan.

This makes patch-day re-runs *fast* — only the entries whose anchors
actually moved get re-scanned.

## 4. Multi-Language Output

After scanning, `src/signatures/writers.rs` emits four parallel views
of the same hit list:

| File              | Shape                                                           |
| ----------------- | --------------------------------------------------------------- |
| `signatures.json` | `total/found/missing` summary + one entry per hit               |
| `signatures.hpp`  | `namespace cs2::signatures { inline constexpr nint X = 0xX; }`  |
| `signatures.cs`   | `namespace CS2Sdk.Signatures { static class { const nint ... } }` |
| `signatures.rs`   | `pub mod cs2_signatures { pub mod client_dll { pub const ... } }` |
| `SIGNATURES.md`   | Markdown table for humans to skim                                |

## 5. Diff Against the Previous Build

When a previous `signatures.json` is found (either via `--cache <PATH>`
or auto-detected from the newest sibling session), the scanner also
emits `diff.json`:

```json
{
  "previous": "dumps/20-04-26-CS2-SDK/signatures/signatures.json",
  "added":            [...],
  "removed":          [...],
  "shifted":          [{ "name": "...", "before": 0xA, "after": 0xB }],
  "pattern_changed":  [...]
}
```

Patch-day workflow: dump, look at `diff.json`, react.

## 6. Where to Look in the Source

| Concern                       | File                                |
| ----------------------------- | ----------------------------------- |
| Signature catalog             | `src/signatures/database.rs`        |
| Scanner + ResolveKind         | `src/signatures/mod.rs`             |
| Warm cache                    | `src/signatures/cache.rs`           |
| Multi-language emitters       | `src/signatures/writers.rs`         |
| Diff                          | `src/signatures/diff.rs`            |
