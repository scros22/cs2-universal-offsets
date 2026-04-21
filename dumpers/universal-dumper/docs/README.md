# Educational Guides

These guides explain *how* the dumper works and *how to consume* its
output. They're written so that someone new to game-internals
development can pick up the project, follow what each pass is doing, and
extend it confidently.

| Guide                                         | What it covers                                                                                            |
| --------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| [PATTERNS.md](PATTERNS.md)                    | IDA-style byte-pattern syntax, wildcards, and how `Rel32`/`RipRel`/`StringRef` resolution works.           |
| [SCHEMA.md](SCHEMA.md)                        | How Source 2's runtime schema system is walked to produce typed class layouts and netvar metadata.        |
| [SIGNATURES.md](SIGNATURES.md)                | The signature scanner end-to-end: PE parsing, section gating, hot cache, multi-language emitters, diff.   |
| [CONSUMING.md](CONSUMING.md)                  | How to drop the generated SDK into a C++/C#/Rust internal cheat (with a worked `SCHEMA_FIELD` example).    |
| [ADDING_SIGNATURES.md](ADDING_SIGNATURES.md)  | The contributor workflow for adding a new signature, validating it, and getting it merged.                |
| [LAYOUT.md](LAYOUT.md)                        | What every file in `dumps/<session>/` is, and which one your project should depend on.                    |

If you only read one, read **CONSUMING.md** — it shows the new
`cs2sdk.hpp` single-include amalgamation and the `SCHEMA_FIELD` macro
in action.
