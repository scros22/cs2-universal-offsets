# Status and Self-Healing

Two things make a dump trustworthy after a CS2 update: knowing exactly what is and is not right about it, and not losing signatures whose function did not actually change. Since v2.1.6 the dumper does both itself.

## `status.json` — what is wrong, if anything

After the protobuf stage the dumper runs a set of self-checks against the live process and writes `include/status.json`. The site shows it as the **Status** tab (with a banner on every page when something is off), the API serves it as `GET /api/status`, and the Discord bot answers `/status` from it. Any check with `status: "fail"` makes the dumper exit with code 1, so a broken dump cannot be published by accident.

| Check | What it catches | Fails the run |
|---|---|---|
| `signatures_resolve` | Database entries that did not match on this build. They are listed by name | no (warn) |
| `patterns_unique` | A pattern that matches more than once in its module and would resolve to whichever copy comes first | yes |
| `patterns_healed` | Entries the dumper had to re-anchor through the previous dump (below). The new patterns are published and must be copied into the database | no (warn) |
| `global_twins` | The a2x-style `dwXxx` globals and the `pXxx` signature globals for the same object must resolve to the same address: `dwGameRules` ↔ `pGameRules`, `dwSensitivity` ↔ `pSensitivity`, `dwPrediction` ↔ `pPrediction`, `dwGlowManager` ↔ `pGlowManager`, `dwLocalPlayerController` ↔ `pLocalPlayerController`, `dwCSGOInput` ↔ `pCSGOInputInstance`, `dwBuildNumber` ↔ `pBuildNumber`, `dwNetworkGameClient` ↔ `pNetworkGameClient`, `dwWindowWidth/Height` ↔ `pWindowWidth/Height`, `dwSoundSystem` ↔ `pSoundSystem` | yes |
| `globals_in_data` | A module-relative global that lands in `.text` or `.rdata` is a function or a vtable pointer, not the object it is named after (this is what `dwGameRules` did before v2.1.5) | yes |
| `function_prologues` | A function target should sit right after padding (`int3` / `ret`). The deliberate instruction sites are exempt | no (warn) |
| `globals_not_code` | `pXxx` signature globals must not resolve into `.text` | yes |
| `protobufs_vs_engine` | Every field of the hand-verified engine structs (`CBaseUserCmdPB`, `CSubtickMoveStep`, …) must have the same offset in the protobuf layout read from the reflection tables | yes |
| `verified_fields` | Every field in `verified_features.json` must resolve from this build's schema | yes |
| `verified_hooks` | Every hook in `verified_features.json` must name a signature that resolved on this build | yes |
| `weapons_snapshot` | How many weapons the session had; the full table needs a match with every weapon spawned | no (warn) |

```json
{
  "build_number": 14184,
  "client_version": "2000917",
  "patch_version": "1.41.8.4",
  "generated_at": "…",
  "dumper_version": "2.1.7",
  "ok": true,
  "summary": { "pass": 10, "warn": 1, "fail": 0 },
  "checks": [ { "name": "global_twins", "status": "pass", "detail": "11 global pairs agree", "items": [ … ] }, … ],
  "signatures": { "total": 578, "found": 577, "unique_functions": 527, "missing": [ { "name": "GameSystem_Think_CheckSteamBan", "module": "server.dll" } ] },
  "auto_healed": []
}
```

`client_version` and `patch_version` come from the game's `steam.inf`, so the file also says which Steam client build the dump belongs to.

## Self-healing — signatures that only moved

Most patterns that "break" on a CS2 update break because bytes inside them moved (a call displacement, a struct offset), not because the function changed. The dumper now re-anchors those on its own:

1. It reads the previous dump's `patterns/patterns.json` (`--previous <dir>`, default `include`, i.e. the committed dump). Every published entry there carries `bytes`, the first 24 bytes of the function.
2. For a function entry whose database pattern no longer matches (or matches several places), those 24 bytes are searched in the new module's `.text`. Exactly one hit, sitting right after padding, means the function is still there and only moved.
3. A fresh pattern that is unique in `.text` is generated at that address — the same synthesiser that produces `pattern_synth` for every hit — and published in place of the stale one. The entry's `healed_from` field carries the old pattern, `status.json` lists it under `auto_healed`, and the `patterns_healed` check warns.
4. `py tools/verify/heal.py --apply` copies the published patterns into `src/patterns/database.rs`, so the next build of the dumper matches directly again.

Globals (`riprel` entries) cannot be healed this way — there are no prologue bytes to look for — and functions whose prologue actually changed stay unresolved. Both are listed on the Status page until they are re-anchored by hand.

## Maintainer tools — `tools/verify/`

| Tool | What it does |
|---|---|
| `dbscan.py` | Rescans the whole database against the CS2 DLLs on disk with the dumper's exact resolve semantics — no game process needed. Reports entries that stopped matching, ambiguous ones, and prologue drift versus the committed dump |
| `genpat.py` | Deterministic pattern generation: `gen_func(module, rva)` grows a pattern instruction by instruction (wildcarding displacements and immediates) until it is unique in `.text`; `verify(module, needle)` resolves it the way the dumper would |
| `heal.py` | Writes the dumper's self-healed patterns into the database (above) |

The rest of the release gate is described in [Contributing](Contributing.md): fresh IDA analysis for every changed module, the per-entry verification (function start, Hex-Rays prototype, behaviour), the a2x cross-check, and an MSVC compile of the generated headers.
