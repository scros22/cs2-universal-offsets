# Changelog

Each release carries a `cs2-sdk.exe` build and the dump for the CS2 build named in its title. Full notes are on the [releases page](https://github.com/scros22/cs2-universal-offsets/releases).

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
