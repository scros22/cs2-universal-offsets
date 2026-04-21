# Patterns and Resolution

This guide explains the byte-pattern format the scanner expects and the
three resolver kinds that can turn a match site into a usable RVA.

## 1. Pattern Syntax

Patterns are written in classic IDA style: hex bytes separated by
spaces, with `?` (or `??`) marking wildcard bytes.

```text
48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?0
```

Rules:

- **Hex bytes** — case-insensitive, must be exactly two hex digits.
- **`??` or `?`** — wildcard byte, matches anything.
- **Half-wildcard nibbles** (e.g. `?0`, `4?`) — the half marked `?`
  matches any nibble; the other half must match. Useful when only the
  destination register changes between builds.
- **Whitespace is irrelevant** — single space, double space, tabs all
  fine.
- **No prefixes** — don't write `0x48`, just `48`.

The scanner is implemented on top of [`pelite`](https://docs.rs/pelite)
so any pattern that round-trips through pelite's ida-style parser is
accepted.

## 2. The Three Resolver Kinds

Once a pattern matches, the **match address** is the byte offset of the
*first byte* of the pattern in the module's `.text` section. That's
rarely what you actually want — usually the pattern lands on a `mov`,
`call`, or `lea`, and the value of interest is encoded *inside* the
following instruction. The `ResolveKind` enum tells the scanner what to
do with the match address.

### 2.1 `None` — match address itself

```rust
ResolveKind::None
```

Use this when the match address *is* the address you want — e.g. the
start of a function you intend to hook directly.

```text
RVA = match_rva
```

### 2.2 `Rel32 { rel_off }` — relative-32 displacement

```rust
ResolveKind::Rel32 { rel_off: 1 }
```

Used for `E8 xx xx xx xx` (`call rel32`) and `E9 xx xx xx xx`
(`jmp rel32`). The displacement is a *signed* 32-bit value, and the
"next instruction" anchor is `match + rel_off + 4`.

```text
disp = i32::from_le_bytes(text[match_rva + rel_off ..][..4])
RVA  = match_rva + rel_off + 4 + disp
```

`rel_off` is the byte offset *from the match address* to the
displacement field. For a pattern that starts on the `E8`/`E9` opcode
itself, `rel_off = 1`.

### 2.3 `RipRel { rel_off }` — RIP-relative addressing

```rust
ResolveKind::RipRel { rel_off: 3 }
```

Used for `48 8D 0D xx xx xx xx` (`lea rcx, [rip + disp32]`),
`48 8B 05 xx xx xx xx` (`mov rax, [rip + disp32]`), and most other
x64 RIP-relative loads. Same arithmetic as `Rel32`, just the resolver
is named differently for clarity at call sites and to leave room for
RIP-specific validation later.

```text
disp = i32::from_le_bytes(text[match_rva + rel_off ..][..4])
RVA  = match_rva + rel_off + 4 + disp
```

`rel_off` here is the offset to the *displacement byte*, which for a
3-byte ModRM-prefixed instruction (`48 8D 0D`) is `3`.

### 2.4 `StringRef` — find the xref to a string

```rust
ResolveKind::StringRef { needle: "MoveType_t" }
```

A meta-resolver: the scanner first locates the literal `needle` bytes
inside the module's `.rdata` section, then synthesizes a `lea
rXX, [rip + str_disp]` pattern and runs the normal `RipRel`
resolution against `.text`. Useful for symbols that are referenced by
exactly one string (debug names, schema names, etc.).

## 3. Why a Section-Aware Scan?

The pattern matcher only walks the bytes inside a *specific* PE section
of a *specific* module:

- `.text` for code patterns
- `.rdata` for string-ref needles
- (Other sections trivially supportable but not currently used.)

This matters because:

1. It's much faster than scanning the whole image.
2. It eliminates false positives from data sections that happen to
   contain bytes that look like opcodes.
3. The same RVA can be re-validated from the cache against the
   recorded pattern, which is how the warm-cache pass avoids
   re-scanning unchanged builds.

## 4. Adding Your Own Pattern

See [ADDING_SIGNATURES.md](ADDING_SIGNATURES.md) for the contributor
workflow. The short version:

1. Open the function in IDA / Ghidra.
2. Pick 12–20 bytes that look unique and resilient across builds —
   prefer the function prologue or a unique constant load.
3. Wildcard the bytes that change between builds (offsets, immediates,
   register encodings).
4. Add an entry to `src/signatures/database.rs` with the appropriate
   `ResolveKind`.
5. Run the dumper and verify the resolved RVA matches what you expect
   in the disassembler.
