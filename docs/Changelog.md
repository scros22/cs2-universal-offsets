# Changelog

Each release carries a `cs2-sdk.exe` build and the dump for the CS2 build named in its title. Full notes are on the [releases page](https://github.com/scros22/cs2-universal-offsets/releases).

## v2.1.8 — 2026-09-25 — feature recipes, internal and external

- **The Features tab was rebuilt** around recipes that are checked against a working implementation and the current build, each with an **Internal** and an **External** version (a switch on the site): Entity list, ESP, FOV changer, Aimbot, Skin changer, Knife changer. Numbered steps, the exact offsets, globals and functions (with this build's RVA and pattern), and **Copy as C++** per recipe.
- Corrected along the way: entity tracking now hooks `CGameEntitySystem::OnAddEntity` / `OnRemoveEntity` (vtable slots 15/16; the old "listener vector at +0x30" was wrong — it is at `+0x2150`); the head bone is **7** (`head_0`), not 6 (`neck_0`) — read from the live model; the FOV recipe follows the game's own resolver (camera `m_iFOV`, else controller `m_iDesiredFOV`, else 90); the aimbot uses `C_CSPlayerPawn_GetAimPunch` for recoil and `SetViewAngles` to steer; the skin and knife changers follow the flow that works on build 14184 (client-side item identity, fallback paint + paint attributes, composite rebuild; subclass token, model change, animation-graph rebind).
- Offsets in the recipes resolve from this dump (schema, engine structs, or schema + fixed delta); the few hand-verified values carry the build they were checked on, and a new `verified_manual` check warns when that is not the current build.

## v2.1.7 — 2026-09-25 — exact feature offsets, polish

- **`verified_features.json` offsets were stale.** 38 of the 67 field offsets had been typed in on an older build (`m_iTeamNum 0x3EB`, `m_iClip1 0x16D8`, `m_iShotsFired 0x1C5C`, …) and ten named the wrong class or a field that is not where they said (`m_iKills` / `m_iDeaths` live in `m_matchStats`, the fallback paint-kit fields on `C_EconEntity`, `m_iFOV` on `CCSPlayerBase_CameraServices`, `m_iItemDefinitionIndex` under `m_AttributeManager.m_Item`). Offsets are now resolved from the schema dumped in the same run; only the two non-schema fields keep hand-verified values. The knife changer's `UpdateSubclass` hook pointed 11 bytes into its function — it now names a new function-start signature, `C_BaseEntity_UpdateSubclass` (IDA-verified: it writes the subclass-data pointer at `+0x388`). The FOV changer's hook named a signature that does not exist; it now names `GetWorldFovResolver`.
- New self-checks `verified_fields` and `verified_hooks` fail the run if a feature field or hook stops resolving.
- `CCSGOInput::m_FrameInput` (`+0x228`) removed from the engine structs: nothing on build 14184 confirms it. Every remaining engine-struct field was re-checked against the code that reads or writes it.
- The three hand-typed interface slots (`CInputSystem::SetRelativeMouseMode`, `CPanoramaUIEngine::GetUIEngine`, `CEnginePVSManager::SetPvsEnabled`) re-checked in IDA.
- New icon (a gold C) for `cs2-sdk.exe` and the site; the exe carries proper version details; the terminal output is restyled.

## v2.1.6 — 2026-09-25 — self-checks, status page, self-healing

- **Self-checks after every dump**, written to `include/status.json`: signature coverage and uniqueness, `dwXxx` globals versus their signature twins, globals in a data section, function targets after padding, `pXxx` globals not in code, protobuf layouts versus the hand-verified engine structs, weapon-table coverage, plus the game's `ClientVersion` / `PatchVersion` from `steam.inf`. A failed check makes the dumper exit 1. Four of the five defects fixed in v2.1.5 would have been caught by these.
- **Status on the site, API and bot**: a Status tab and `GET /api/status` with every check, its details, unresolved signatures and known issues; a banner on every page when a check failed; `/status` in the Discord bot and a self-check line in its status embed.
- **Self-healing signatures**: when a database pattern stops matching after an update but the function did not change, the dumper re-finds it through the previous dump's 24 prologue bytes (`--previous`, default `include`), generates a fresh unique pattern and publishes it with a `healed_from` field. `tools/verify/heal.py --apply` writes those into the database.
- `tools/verify/`: the offline rescan (`dbscan.py`) and pattern generator (`genpat.py`) used for every release are in the repository.

## v2.1.5 — 2026-09-25 — CS2 build 14184, full verification pass

CS2 updated to build 14184 on the evening of 2026-09-24 (client, server, engine2 and networksystem changed). Every module was re-analysed from scratch in IDA and the whole dump was checked, not just re-scanned. What was wrong, and is now fixed:

- **`dwGameRules` pointed at a vtable pointer in `.rdata`**, not at `g_pGameRules`. It now uses the same read-site anchor as a2x and resolves to the global the `pGameRules` signature already found (651 references).
- **`dwSensitivity` was the `cl_leveloverview` ConVar**, not `sensitivity` (both are resolved through the same ConVarRef helper; the registration site names them). Re-anchored on the `sensitivity * zoom_sensitivity_ratio` site, with `dwSensitivity_sensitivity = 0x58` (the value offset inside `ConVarInfo_t`). The `pSensitivity` signature resolves to the same slot.
- **`dwSoundSystem_engineViewData` was `0x7C`**, read from a stack store in an unrelated function; the view block is at `0x6C` (`movups [rdi+6Ch]` in the view-setup writer).
- `dwSoundSystem` and `dwNetworkGameClient_isBackgroundMap` had stopped resolving and were silently missing; both re-anchored. The classic globals are now the same 32 a2x publishes, and every one matches a2x's value for the build.
- **`CBaseUserCmdPB` in `protobufs.json` / `protobufs.hpp` was wrong**: the reader sorted fields by number while libprotobuf's offset table is in declaration order, which shifted `viewangles`, `forwardmove`, `leftmove`, … by one slot. Fixed at the source; the layout now agrees with `_InternalParse` and with the hand-verified engine struct.
- **The generated headers did not compile under MSVC**: `engine2_dll.hpp` used `CUtlStringTokenNoRegistration` without a declaration (it only appeared inside a `using` alias the forward-declaration pass did not scan) and `soundsystem_dll.hpp` used `std::unique_ptr` without `<memory>`. Both fixed; `cs2.hpp` now compiles cleanly with `cl /std:c++17` and `/std:c++20 /permissive-`.
- Signatures, verified entry by entry against fresh IDA analysis (function start, prototype, behaviour):
  - two patterns started one byte into the function (`BulkRegenIterator`, `SDL_EventHandler`) and published an RVA one byte late;
  - `CreateEntityByClassName` had drifted onto a flex-controller warning; re-anchored on the client's networked entity creator (`CL: Forcing ExecuteQueuedOperations …`);
  - renamed to what the function does: `SetupCmd` → `GetUserCmdSequence`, `DrawOverHead` → `IsRenderingEnabledForSlot`, `UnlockInventory` → `IsInventoryUnlocked`, `DrawViewPunch2` → `CalcLocalPlayerView`, `DrawCrosshair` → `ShouldDrawCrosshair`, `DynamicLight_SetDieTime` → `GetGameTimeOrCurrent`, `CreateParticleEffect` → `Particles_SetControlPointPosition`, `AutowallInit` → `C_BaseModelEntity_UpdateOnRemove` (vtable slot 14 of the model-entity classes), `PrepareSceneMaterial` → `CMaterial2_GetFloatParam`, `UtlBuffer` → `CBufferString_Purge`, `LoadKeyValues` → `LoadKV3`;
  - removed, because the name described a different function: `ReportHit` (a `CCLCMsg_HltvReplay` destructor), `SetupMove` (the quick-buy radial), `SetupMovementMoves` (a call site in a schema helper), `AutowallTracePos` (`C_GlobalLight` skybox slots), `CCSGOInput_HandleViewAngles` (an input-state reset), `UpdatePostProcessing` (watch-menu match selection), `DrawLightScene` (a struct copy), and the placeholders `UnknownParticleFunction` and `SomeTimingFromPawn`;
  - 162 prototypes filled in or refreshed from the current build's Hex-Rays output.
- 577 entries, 576 resolve, 0 ambiguous. Schema field offsets (9,249 compared), buttons and interface instances match a2x's dump exactly.

## v2.1.4 — 2026-09-24 — strict JSON, documentation

- `patterns/patterns.json` is strict JSON: `va` and `rva` are hex strings (`"0xB64A10"`), as in every other file. Earlier dumps wrote them as bare hex literals, which standard JSON parsers reject. The one-entry-per-line, column-aligned layout is unchanged.
- `/api/pattern/<name>` resolves aliases, display names and `module/Name` exactly like `/api/query`, and both report a folded-alias hit as `match: "alias"`.
- This wiki. `docs/` in the repository mirrors it and replaces the out-of-date guides from June.
- The Discord bot's source is in `discord-bot/`.

## v2.1.3 — 2026-09-24 — swap chain

- New signature `CSwapChainDx11_CreateSwapChain` (rendersystemdx11.dll), requested on Discord: `bool (this, IDXGIFactory*, device, flags)`; it calls `IDXGIFactory::CreateSwapChain` with `&this->m_pSwapChain` as the out pointer.
- New engine struct `CSwapChainDx11` with `m_pSwapChain` at `+0x170` and the `IDXGISwapChain` slots hook authors need (8 `Present`, 13 `ResizeBuffers`, …).
- 586 entries, 585 resolve, 0 ambiguous.

## v2.1.2 — 2026-09-24 — signature audit

- Every function that was listed under two or three names was decompiled and checked. 41 groups are genuine aliases and are now published once with an `aliases` field; 16 names described a different function and were removed: `GetLocalControllerById`, `GetBonePositionByName`, `CGameTraceManager_TraceRay`, `SendChatMessage`, `DrawTeamIntro`, `RemoveLegs`, `TraceToExit`, `pVPhys2World`, `pGetBBox`, `CCSPlayerController_SwitchTeam`, and six duplicates.
- `CalculateInterpolation` / `GetCUserCmdTick` was misnamed under both names; it is `CEntityInstance::GetEntityIndex` and is published as `CEntityInstance_GetEntityIndex`.
- Display names never strip a class down to a bare word (`CCSInventoryManager_Get` stays).
- `patterns.json` gains `unique_functions` and per-entry `aliases`; `patterns.hpp` notes aliases in comments.
- 585 entries, 584 resolve, 0 ambiguous.

## v2.1.1 — 2026-09-24 — entity I/O, all offsets, `CHandle::Get()`

- The four entity I/O functions are back and verified: `CEntityInstance_AcceptInput`, `CEntityIdentity_AcceptInput`, `CEntitySystem_AddEntityIOEvent`, `CEntityIOOutput_FireOutputInternal`. 600/601 resolve.
- No pattern matches more than once any more (`CAM_ThinkReturn`, `NoClipOnChange`, `CBaseModelEntity_SetModel`, `PostProcessQuery`, `CBaseEntity_SetGravityScale`, `CTakeDamageInfo` and `CS2ItemEditor_BuildTemplateMaterialFromFile` were fixed).
- New `offsets/offsets_all.json`: every resolved global, tagged by kind. The site's Offsets tab shows all of them; it showed only the 29 `dwXxx` ones before.
- `CHandle<T>::Get()`, `GetSerial()`, `==` / `!=` in `macros.hpp`, with the resolvers in `impl/entity_system.hpp`.
- `CCSCustomPlayerCamera` (the renamed `CSPlayerCamera`) in the schema dump.
- Site: automatic retry on transient errors; malformed requests get a 400; unknown hostnames pointed at the server are dropped.

## v2.1.0 — 2026-09-24 — CS2 build 14183

- Resync to Steam client builds 2000914 / 2000915 (CS2 build 14183). 598/601 signatures: every one broken by the last few updates recovered and 98 new ones added, each verified in IDA with a Hex-Rays prototype — among them `HudVoiceStatus_SetVoiceData`, `CCSGOInput_AddInputHistoryEntry`, `HandleViewAngles`, `ReadFrameInput`, `ProcessInputEvent`, `C_CSPlayerPawn_GetAimPunch` / `GetAimPunchAtTick` / `GetInterpolatedShootPosition`, `CGlowProperty_GetGlowColor` / `IsGlowing`, weapon, econ, inventory, prediction, trace and render functions, and new globals (`pCSGOInputInstance`, `pDynamicLightManager`, `pPredictionPlayer`, `pMaterialManager`, `pEconItemSystem`, `pCSInventoryManager`, …).
- User-command engine structs: `CUserCmd` (150-entry ring, 0x98 stride), `CCSGOUserCmdPB`, `CBaseUserCmdPB`, `CSubtickMoveStep`, `CInButtonStatePB`, `CCSGOInputHistoryEntryPB`, `CSGOInterpolationInfoPB`, `CMsgQAngle`, `CMsgVector`, each with offsets, has-bits and a drop-in `.h`.
- All 44 weapons in `weapons.json`.
- Fixes: several signatures that matched but pointed at the wrong function (`pEntitySystem`, `FindHudElement`, `LevelShutdown`, `GetEconItemSystem`, `GetRemovedAimpunch`, `CheckJumpButton`, …); raw patterns with an `extra_off` publish the correct RVA; the weapon and entity walkers read their offsets from the schema of the same run; engine-struct function addresses come from the signature pass; `patterns/` is lowercase on every platform.
- Site: batch API (`POST /api/query`), bookmarks and collections, API page, Offsets tab, engine-structs panel.

## v1.24.0 — 2026-08-14 — CS2 build 14175

- Resync to build 14175; two broken signatures recovered, three dead ones dropped (503/503).
- Since v2.0.0: `ViewModelHideZoomed` and `ConvarGet` recovered on build 14172; engine struct layouts (`CCSGOInput`, `CUserCmd`, `CViewSetup`); the live entity snapshot; the game-events registry (273 events, typed fields); the weapon `CCSWeaponBaseVData` table.

## v2.0.0 — 2026-07-13 — ConVars, vtables and richer schemas

- ConVar / ConCommand catalogue (`convars.json`, `convars.hpp`) read from the tier0 registry: type, value, flags, description.
- Interface vtables with RTTI class names (`vtables.json`, typed `interfaces.hpp`).
- Schema headers with class size, parent and field metadata; `schemas.json`.

## v1.23.0 — 2026-05-22

- 16 signatures imported from cspatterns.dev; new icon.

## v1.22.x — May 2026

- CS2 build 14162 resync and follow-ups.

## v1.7.0 – v1.21.x — April to May 2026

- The first public series: one binary for offsets and signatures, the section-aware IDA-style scanner with `Rel32` / `RipRel` resolution, hand-formatted `signatures.json`, multi-language outputs, per-module schema headers, `SCHEMA_FIELD` SDK classes, interface accessors and the single-include amalgamation.
