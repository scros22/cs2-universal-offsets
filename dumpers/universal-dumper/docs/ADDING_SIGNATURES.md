# Adding a New Signature

A 5-minute checklist for contributors.

## 1. Find the Function

Open the disassembler (IDA / Ghidra / Binary Ninja) on the module you
care about. Locate the function you want to expose.

## 2. Pick Stable Bytes

Aim for **12–20 bytes** that survive small refactors:

- ✅ Function prologue (`48 89 5C 24 ??` patterns)
- ✅ Unique constant loads (`48 8D 0D ?? ?? ?? ??` to a unique string)
- ✅ Calls to clearly-named imports
- ❌ Long sequences of `mov reg, reg` — too generic
- ❌ Bytes that contain literal offsets that move every patch

## 3. Wildcard the Volatile Bits

Replace bytes that vary across builds with `??` (or `?0` / `4?` for
half-byte wildcards). When in doubt, wildcard.

```text
48 8D 0D ?? ?? ?? ??       ; LEA RCX, [rip + str] — wildcard the disp
E8 ?? ?? ?? ??              ; CALL rel32          — wildcard the disp
48 8B ?? 24 ??              ; MOV reg from stack  — wildcard reg + offset
```

## 4. Pick the Resolver

| Pattern starts with...               | Use                                      |
| ------------------------------------ | ---------------------------------------- |
| Function prologue                    | `ResolveKind::None`                      |
| `E8` (CALL) or `E9` (JMP)            | `ResolveKind::Rel32 { rel_off: 1 }`      |
| `48 8D 0D` (LEA RCX, [rip+x])        | `ResolveKind::RipRel { rel_off: 3 }`     |
| References a unique string           | `ResolveKind::StringRef { needle: "X" }` |

For `Rel32`/`RipRel`, `rel_off` = byte index of the displacement
*relative to the start of the pattern*. Off-by-one here will give you
a plausible but wrong RVA.

## 5. Add the Entry

Open `src/signatures/database.rs` and append:

```rust
SignatureEntry {
    name:    "MyNewThing",
    module:  "client.dll",
    pattern: "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 81 EC 50 02 ?? ??",
    section: ".text",
    resolve: ResolveKind::None,
},
```

Group it under the same comment block as related signatures.

## 6. Verify

```powershell
cd dumpers\universal-dumper
cargo run --release
```

Then look at the just-emitted `signatures.json` for your entry:

```json
{
  "name":     "MyNewThing",
  "module":   "client.dll",
  "found":    true,
  "matches":  1,
  "rva":      "0x1A2B3C4",
  "va":       "0x7FF6...",
  "resolve":  "None"
}
```

Cross-check the RVA against the disassembler. If `matches > 1` your
pattern is ambiguous — tighten it or drop it.

## 7. Open a PR

Title: `add signature: MyNewThing`. Include:

- The disassembled function name (or address) you targeted.
- Whether the pattern is verified live or reasoned-out.
- Any caveats (build-specific, only-on-Windows, etc).
