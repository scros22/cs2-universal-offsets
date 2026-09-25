# Signatures

A signature is an IDA-style byte pattern that finds one function or one global in one module, together with everything needed to use it: the resolved RVA on the current build, the function's Hex-Rays prototype, the first bytes of its prologue, and the other names the community knows it by.

The database is [`src/patterns/database.rs`](https://github.com/scros22/cs2-universal-offsets/blob/main/src/patterns/database.rs). On build 14184 it holds **577 entries; 576 resolve** (some functions are reached by more than one entry — see *Aliases* below). Every published pattern matches **exactly once** in its module: a pattern that matches more than once is treated as broken and fixed before release.

## `patterns/patterns.json`

```json
{
  "total_scanned":  586,
  "found":          585,
  "unique_functions": 536,
  "missing":        1,
  "modules":        ["animationsystem.dll", "client.dll", "engine2.dll", …],
  "patterns": [
    { "name": "CreateMove", "module": "client.dll", "resolve": "raw", "va": "0x7FFDD6665A70", "rva": "0xB65A70",
      "pattern": "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18",
      "bytes": "85 D2 0F 85 CC 12 00 00 48 8B C4 44 88 40 18 89 50 10 48 89 48 08 55 53",
      "pattern_synth": "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18 89",
      "prototype": "void __fastcall CreateMove(_QWORD *a1, int a2, char a3)" },
    { "name": "CalculateWorldSpaceBones", "module": "client.dll", "resolve": "raw", …,
      "aliases": ["CalcWorldSpaceBones"] }
  ]
}
```

| Field | Meaning |
|---|---|
| `name` | The published name: the constant in `patterns.hpp` and the key for `/api/pattern/<name>` |
| `module` | The DLL the pattern is scanned in |
| `resolve` | How the match address becomes the final address: `raw`, `rel32` or `riprel` (below) |
| `pattern` | The database pattern. `?` is a one-byte wildcard |
| `rva` | The final address relative to the module base, after resolution: the function (or global) itself, not the match |
| `va` | The same address in the dumped process — only meaningful for that session |
| `prototype` | The function's prototype as recovered in IDA / Hex-Rays. A `sub_18xxxxxxx` name is the function's address at IDA's default image base |
| `bytes` | The first 24 bytes at `rva`, no wildcards. Present when `rva` is inside `.text` |
| `pattern_synth` | An auto-generated pattern for the same function: the shortest prefix of `bytes` that is unique in `.text`, with `?` on relocatable bytes (CALL/JMP and RIP-relative displacements). Pastes straight into IDA, x64dbg or ReClass.NET |
| `aliases` | Other database names that resolved to the same address; present only when there are any |

Entries that did not resolve are not listed; `missing` counts them and the run log names them.

## Resolve kinds

The pattern is scanned in the module's `.text` section (`.rdata` as a fallback for data patterns). `resolve` says what the dumper did with the match address to obtain `rva`:

| `resolve` | Database constant | Used for | `rva` |
|---|---|---|---|
| `raw` | `NONE` | Function prologues, inline code | `match + extra_off` |
| `rel32` | `REL32_1` | Patterns that start on an `E8` call or `E9` jmp | `match + 1 + 4 + int32(match + 1)` — the call target |
| `riprel` | `RIPREL_3` (`RIPREL_2` without a REX prefix) | A RIP-relative `lea`/`mov` to a global: `48 8D 0D ? ? ? ?`, `48 8B 05 ? ? ? ?` | `match + off + 4 + int32(match + off)` — the global |

`extra_off` (a byte offset applied to a raw match — `ConvarGet` starts 4 bytes before its function, so its entry carries `extra_off: 4`) and `rel_off` (the offset of the displacement inside the pattern) are fixed per entry in the database. You do not need either at runtime: `rva` is already the final address. If you scan the patterns yourself, apply the rule that your entry's `resolve` names.

`riprel` entries are globals, not functions: `pGameRules`, `pCSGOInputInstance`, `pMaterialManager`, … They are also published in `offsets/offsets_all.json` with `kind: "signature"` — see [Offsets](Offsets.md).

## Names, display names and aliases

**Database name.** Entries in `database.rs` are named `Class_Method` when the class matters (`CCSGOInput_ProcessInputEvent`, `CSwapChainDx11_CreateSwapChain`) and bare when the community name is unambiguous (`CreateMove`, `TraceShape`). Globals resolved by `riprel` start with `p`.

**Published name.** What `patterns.json` carries is the database name with its C++ class prefix stripped, as long as what is left is still a descriptive method name: `CCSGOInput_ProcessInputEvent` is published as `ProcessInputEvent`, `CSwapChainDx11_CreateSwapChain` as `CreateSwapChain`. The prefix is kept when stripping would leave a bare word (`CCSInventoryManager_Get`), and names that start with `C_` (client entity classes), `dw`, `g_` or `m_` are never touched (`C_CSWeaponBaseGun_GetInaccuracy` stays whole).

**Aliases.** When several database entries resolve to the same address they are folded into one published entry. The primary is the most descriptive name — a class-qualified one beats a bare one, and `_v2` / `_raw` / `_legacy` / `_Client` style variants never win — and the rest go into `aliases`. On build 14183, 57 functions had two or three names; every group was decompiled and checked before being folded, and 16 names that turned out to describe a different function were removed rather than kept as aliases ([Changelog](Changelog.md) v2.1.2).

All of these resolve in the API. `/api/pattern/<name>` and `/api/query` accept the published name, the full database name, any alias, the `Class::Method` spelling, the method name alone, and `module.dll/Name` to pin a module. A non-exact match is reported in the response, so you can see which entry you got.

## Using a signature at runtime

The dumped `rva` is correct for the build it was dumped from. If you ship the RVA, pin the build (`CS2_BUILD`). If you want to survive small patches, ship the pattern and scan at start-up:

```cpp
#include <patterns/patterns.hpp>   // pattern::client::CreateMove == "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18 89"

// Minimal IDA-style scanner. Scan only the module's .text section.
std::uint8_t* find(std::uint8_t* text, std::size_t size, std::string_view ida) {
    std::vector<int> needle;                                   // -1 = wildcard
    for (std::size_t i = 0; i < ida.size();) {
        if (ida[i] == ' ') { ++i; continue; }
        if (ida[i] == '?') { needle.push_back(-1); i += (i + 1 < ida.size() && ida[i + 1] == '?') ? 2 : 1; continue; }
        needle.push_back(std::stoi(std::string(ida.substr(i, 2)), nullptr, 16)); i += 2;
    }
    for (std::size_t i = 0; i + needle.size() <= size; ++i) {
        std::size_t j = 0;
        while (j < needle.size() && (needle[j] < 0 || text[i + j] == needle[j])) ++j;
        if (j == needle.size()) return text + i;
    }
    return nullptr;
}

// raw:    fn     = match (+ extra_off)
// rel32:  target = match + 1 + 4 + *reinterpret_cast<std::int32_t*>(match + 1)
// riprel: global = match + 3 + 4 + *reinterpret_cast<std::int32_t*>(match + 3)
```

Treat more than one hit as a failure. That is what the dumper does, and it is why every published pattern is unique.

After a CS2 update, `pattern_synth` and `bytes` give you a second chance: when the database pattern stops matching, the synthesised one often still does, and the raw prologue bytes let you find the function in a disassembler by hand.

## Other views of the same data

- `patterns/patterns.hpp` — `pattern::<module>::<Name>` as `constexpr std::string_view`, with `// also known as:` comments for aliases.
- `GET /api/patterns` — the list as JSON (`?module=`, `?name=` substring; `?raw=1` for the unfolded list with one entry per database name).
- `GET /api/export/patterns.txt` — every pattern as an aligned, module-grouped text file.
- The **Patterns** tab on [cs2-sdk.com](https://cs2-sdk.com): searchable, with a detail drawer per function.

## Code sites

A handful of entries are deliberately **not** function starts: they resolve to one instruction inside a function, for people who patch or read at that exact place. They have no prototype and are not meant to be hooked as functions:

| Entry | Module | What the address is |
|---|---|---|
| `UntrustedFlagSetter` | client.dll | the `mov byte [g_bUntrusted], 1` store |
| `CAM_ThinkReturn` | client.dll | the instruction after `CAM_Think`'s early return |
| `CCSPlayer_ThirdPersonReset` | client.dll | the `cmp [cvar+58h], 0` that guards the third-person reset |
| `DisablePvsAccessor` | engine2.dll | the `lea rcx, [g_pPVSManager]` read |
| `IGameSystem_InitAllSystems_pFirst`, `IGameSystem_LoopDestroyAllSystems_s_GameSystems`, `IGameSystem_LoopPostInitAllSystems_pEventDispatcher` | server.dll | the global references named in the entry |

Everything else resolves to the first byte of a function (or, for `riprel` entries, to a global in `.data`).

## How the database is verified

Every function entry is checked against a fresh IDA analysis of the current build, not just re-scanned: the resolved address must be a function start, its Hex-Rays prototype is compared with (and refreshed into) the database, and the name is checked against what the function does — the strings it references, its callers, the vtable it sits in. Entries that fail are fixed or, when the name describes a different function, removed and listed in the [Changelog](Changelog.md). Cross-checks against the a2x dumper's output for the same build cover the classic globals, buttons, interfaces and every schema field offset.

## Coverage on build 14184

576 of 577 entries resolve. The one that does not is `GameSystem_Think_CheckSteamBan` (server.dll): the function still exists, but nothing unique is left to anchor a pattern on. It stays in the database so it is retried on every build instead of being forgotten.
