# Repository Layout

Top-level folders, what they contain, and who owns them. **Keep the root clean** — only
build artefacts the toolchain *requires* at the top level (`.sln`, `.vcxproj*`, `dllmain.cpp`,
`.gitignore`).

```
cs2/
├── CS2.sln                     # Visual Studio solution
├── CS2.vcxproj(.filters)       # MSBuild project (DLL target)
├── dllmain.cpp                 # Injected DLL entry point
├── .gitignore
│
├── core/                       # Core engine: memory, signatures, syscall, stealth, math
├── features/                   # Active feature modules (see list below)
├── render/                     # ImGui menu + DX11/Win32 hooks
├── sdk/                        # CS2 SDK shims (entity, schema, types)
├── vendor/                     # Third-party: imgui, minhook
│
├── dumper/                     # Standalone Rust offset/signature dumper
├── tools/                      # Misc dev tools (injectors, helpers)
├── scripts/                    # Build & maintenance scripts (.bat, .py)
│
├── docs/                       # Long-form architecture + engineering notes (committed)
├── offset-collection/          # All raw signature / offset dumps & research notes
│   ├── cs2/                    # ← active CS2 sources of truth
│   ├── research-notes/         # forum quotes, weapon tables, paint-kit notes
│   └── legacy-unreal/          # unrelated UE/Fortnite dumps (reference only)
│
├── archive/                    # Dead code kept for history (not in build)
│   └── unused-headers/         # orphaned root-level .h/.hpp from old layouts
│
├── temp/                       # Per-developer scratch (logs, assets) — gitignored
│   ├── logs/
│   └── assets/
│
├── private/                    # Local-only secrets (licenses, keys) — gitignored
│
├── cs2-lib-main/               # Vendored read-only TS library (item DB)
├── cs2-signatures-main/        # Vendored read-only signature reference
├── CS2_VibeSignatures-main/    # Vendored read-only signature pipeline
├── ida-pro-mcp-main/           # Vendored IDA Pro MCP integration
├── ImGuiMenu-s-main/           # Vendored ImGui menu reference
└── .kiro/                      # Spec / planning workspace metadata
```

## Rules of the road

1. **Nothing new at the root** unless the toolchain requires it.
2. **Signatures & offsets** → `offset-collection/cs2/`. Update `core/signatures.h` from there.
3. **Build scripts** → `scripts/`. Don't drop `.bat`/`.py` at the root.
4. **Logs, crash dumps, screenshots** → `temp/` (gitignored). Never commit these.
5. **Licenses, keys, IDA `.hexlic`** → `private/` (gitignored). Never commit these.
6. **Dead code** → `archive/`. Delete from build first, then move. Don't leave orphans loose.
7. **Documentation** → `docs/` for evergreen, `temp/` (or session notes) for in-progress.

## Build

```powershell
# From repo root
scripts\build_all.bat
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the runtime architecture.

## Active features (`features/`)

Anything not in this list is **not in the build**. If you need it, check [archive/](archive/).

| Header | Purpose |
|---|---|
| `aimbot.h`            | Aim assist + target selection |
| `anti_aim.h`          | View/send-angle manipulation |
| `auto_accept.h`       | Auto-accept matchmaking |
| `backtrack.h`         | Backtrack records |
| `bhop.h`              | Bunny hop |
| `bullet_tracer.h`     | Bullet tracer rendering |
| `chams.h`             | Player chams (DrawIndexed hook) |
| `damage_indicator.h`  | On-screen damage popups |
| `esp.h`               | World ESP (boxes, names, weapons) |
| `fake_lag.h`          | Choke / fake-lag |
| `grenade_prediction.h`| Grenade trajectory predictor |
| `knife_glove_manager.h`| Knife/glove menu UI + selection |
| `nade_helper.h`       | Lineup / nade helper |
| `rank_revealer.h`     | Reveal scoreboard ranks |
| `skinchanger_test.h`  | Skin/knife/glove apply pipeline (current impl) |
| `sound_esp.h`         | Footstep / sound visualisation |
| `triggerbot.h`        | Triggerbot |
| `weapon_icons.h`      | ImGui font icons for ESP weapon labels |
| `world_effects.h`     | Misc world tweaks (skybox, fog, etc.) |

Reachability is traced from `dllmain.cpp` → `render/hooks.h` + `render/menu.h`. If you add
a new feature header, include it from one of those entry points or it won't compile in.
