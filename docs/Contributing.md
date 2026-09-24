# Contributing

Issues and pull requests are welcome. The bar is the same for both: a claim about the game has to be verified against the binary, not copied from somewhere.

## Reporting a wrong or missing signature

Open an issue with:

- the name (or what the function does),
- the module and the address in IDA (`sub_18xxxxxxx` at the default image base is fine),
- how you verified it: a string it references, a caller, the Hex-Rays prototype,
- the CS2 build number (`manifest.json` or `GET /api/summary`).

## Adding a signature

Everything lives in one file, [`src/patterns/database.rs`](https://github.com/scros22/cs2-universal-offsets/blob/main/src/patterns/database.rs): one entry per line, grouped by module.

```rust
Pattern { name: "CSwapChainDx11_CreateSwapChain",       module: "rendersystemdx11.dll", needle: "44 88 4C 24 20 55 53 57 41 54", resolve: NONE, extra_off: 0, prototype: "bool __fastcall sub_18003E7D0(__int64 a1, __int64 *a2, __int64 a3, char a4)" },
```

| Field | Rule |
|---|---|
| `name` | `Class_Method` when the class matters, the bare method name when the community name is unambiguous. Globals resolved by `RIPREL_*` start with `p`. Never a bare word such as `Get` or `New` on its own |
| `module` | The DLL the bytes are in |
| `needle` | IDA-style, space-separated, `?` per wildcard byte. It must match **exactly once** in the module's `.text`. Prefer the function's own prologue; wildcard every CALL/JMP and RIP-relative displacement and any immediate that is a build-specific address |
| `resolve` | `NONE` when the match *is* the address; `REL32_1` when the needle starts on the `E8`/`E9` of a call/jmp to the function; `RIPREL_3` (or `RIPREL_2`) when it starts on a RIP-relative `lea`/`mov` to a global |
| `extra_off` | Bytes to add to a `NONE` match when the needle cannot start on the function itself (`ConvarGet` uses 4). Normally 0 |
| `prototype` | The Hex-Rays prototype, verbatim. Empty only for globals |

The file is UTF-8 with a BOM and CRLF line endings; keep both as they are.

### Verify before opening the PR

1. Confirm the function in IDA (or Ghidra / Binary Ninja): what it does, what calls it, the strings it references. A name is a claim about behaviour; the v2.1.2 audit removed sixteen names that had been copied around without one.
2. Check that the needle matches once. Any pattern scanner will do; the dumper itself reports ambiguity in its log.
3. Run the dumper against the current build and read your entry in `include/patterns/patterns.json`: `rva` must be the function's start (or the global), and `prototype` must be there.
4. If another entry already resolves to the same address, do not add a second one: fix the existing entry's name if it is wrong, or open an issue if you think the function deserves an alias.

```powershell
cargo run --release -- -o include_new     # CS2 running, elevated prompt
```

### Pull request

Title `add signature: <Name>` or `fix signature: <Name>`. Say which build you verified on and how. One logical change per PR; do not commit `include_new/`.

## Engine structs

Hand-verified layouts live in [`src/output/engine_structs.rs`](https://github.com/scros22/cs2-universal-offsets/blob/main/src/output/engine_structs.rs) as

```rust
EStruct { name, module, desc, size, instance_pattern, instance_note,
          fields:    &[EField { name, offset, ty, note }],
          functions: &[EFunc  { name, pattern }] }
```

Function and instance addresses are never typed in: `pattern` names a database entry and is resolved from the same run, so the struct stays correct as long as its patterns do. Add fields only with a `note` that says how the offset was confirmed.

## Where things are

| Path | What |
|---|---|
| `src/main.rs` | CLI, the run pipeline, the `patterns.json` writer |
| `src/patterns/database.rs` | The signature database |
| `src/patterns/mod.rs` | Scanner, resolve kinds, alias folding, display names |
| `src/patterns/writers.rs`, `src/patterns/offsets_writer.rs` | `patterns.hpp`; `offsets.hpp`, `offsets.json`, `offsets_all.json` |
| `src/analysis/` | Walkers: `offsets.rs`, `schemas.rs`, `interfaces.rs`, `vtables.rs`, `rtti.rs`, `protobufs.rs`, `convars.rs`, `gameevents.rs`, `weapons.rs`, `entities.rs`, `buttons.rs`, and `schema_lookup.rs` (the parent-climbing field lookup the walkers use) |
| `src/output/` | Emitters: `sdk_classes.rs` (schema headers), `macros_base.hpp` (the `macros.hpp` template, `CHandle<T>`), `entity_system.rs`, `interface_classes.rs`, `vtables.rs`, `protobufs.rs`, `engine_structs.rs`, `convars.rs`, `gameevents.rs`, `weapons.rs`, `entities.rs`, `buttons.rs`, `amalgamation.rs` (`cs2.hpp`), `verified.rs` |
| `src/source2/` | Read-only models of the engine's own structures: schema system, tier0/tier1 containers, `CreateInterface` |
| `src/memory/` | memflow process and module access |
| `include/` | The committed dump |
| `docs/` | A mirror of this wiki |
| `discord-bot/` | The Discord bot |

## Releases

A release is a tag `vX.Y.Z` with `cs2-sdk.exe` attached and the dump for the named CS2 build committed under `include/`. The site is updated from `main`, and the Discord bot announces the new build on its own.
