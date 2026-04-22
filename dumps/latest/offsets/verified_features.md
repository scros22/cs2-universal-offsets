# Verified Working Features

Hand-curated catalogue of CS2 features that have been **confirmed working in a live internal cheat** against the current build. Cross-reference with the auto-generated `offsets.*`, `signatures.*` and `client_dll.*` files for canonical addresses; this document captures the *gotchas* — the kind of thing that took an evening of IDA work to figure out.

**CS2 build:** `14153`

**Feature count:** 18

---

## No Smoke — _working_

Iterate the entity list, identify C_SmokeGrenadeProjectile via CEntityIdentity::m_designerName == "smokegrenade_projectile", zero m_nSmokeEffectTickBegin and clear m_bDidSmokeEffect. Engine re-evaluates every tick, so writes stick. Also zero m_flLastSmokeOverlayAlpha on the local pawn for the screen overlay.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_SmokeGrenadeProjectile` | `m_nSmokeEffectTickBegin` | `0x1250` | `int32` | set 0 to skip the puff |
| `C_SmokeGrenadeProjectile` | `m_bDidSmokeEffect` | `0x1254` | `bool` | set false |
| `C_CSPlayerPawn` | `m_flLastSmokeOverlayAlpha` | `0x1420` | `float` | local pawn's screen overlay alpha; set 0 |

---

## Smoke Color — _working_

Same entity walk as No Smoke; write a Vector (3 floats * 255) to m_vSmokeColor. Particle system reads this every frame.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_SmokeGrenadeProjectile` | `m_vSmokeColor` | `0x125C` | `Vector3 (RGB 0..255)` | 3 floats, multiply RGB by 255 |

---

## No Flash — _working_

Zero m_flFlashDuration and m_flFlashMaxAlpha on the local pawn. Trip the write only when duration > 0 to avoid spamming the engine.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_CSPlayerPawn` | `m_flFlashDuration` | `0x1400` | `float` | set 0 to clear flash |
| `C_CSPlayerPawn` | `m_flFlashMaxAlpha` | `0x13FC` | `float` | set 0 for full removal, 0..255 for partial |

---

## Skybox Tint — _working_

Hook scenesystem.dll!DrawSkyboxArray, intercept the draw-primitive pointer (3rd arg). Build 14152 moved the tint vec3 from +0x100 to +0xE8 inside the skybox object — writing to the old +0x100 slot poisons the renderer with NaN and crashes after ~60s. Layout: +0xE8..+0xF0 RGB tint floats, +0xF4 mode int, +0xF8..+0x104 four sun-angle floats. Do NOT write to env_sky m_vTintColor — the renderer caches material params at level setup and ignores entity writes.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `env_sky` | `m_vTintColor` | `0xFB9` | `Color32` | DOES NOT WORK at runtime — use the DrawSkyboxArray hook instead |
| `env_sky` | `m_vTintColorLightingOnly` | `0xFBD` | `Color32` | Same caveat |
| `env_sky` | `m_flBrightnessScale` | `0xFC4` | `float` | DOES update live (independent code path) |

### Hooks

| Function | Module | Signature | Action |
|---|---|---|---|
| `DrawSkyboxArray` | `scenesystem.dll` | `DrawSkyboxArray` | Patch RGB at primitive_obj+0xE8 before passing through to the original. |

---

## FOV Changer — _working_

Two-prong approach. (1) Hook GetWorldFov in client.dll (signature SetWorldFov, E8-CALL @ +1) and return the desired value when the local pawn is not scoped. (2) Every tick, write the desired FOV into m_iFOV and m_iFOVStart on the camera services AND into m_iDesiredFOV on the local controller (canonical source the renderer reads). The controller-level field is the one that gets reset back to default if you only fight the camera-services side.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `CBasePlayerController` | `m_iDesiredFOV` | `0x784` | `uint32` | a2x-named m_iDesiredFOV_OnController |
| `CCSPlayer_CameraServices` | `m_iFOV` | `0x290` | `uint32` | current camera FOV |
| `CCSPlayer_CameraServices` | `m_iFOVStart` | `0x294` | `uint32` | target camera FOV |
| `C_CSPlayerPawn` | `m_pCameraServices` | `0x1218` | `CCSPlayer_CameraServices*` | deref to reach m_iFOV/m_iFOVStart |

### Hooks

| Function | Module | Signature | Action |
|---|---|---|---|
| `GetWorldFov` | `client.dll` | `SetWorldFov` | Return cfg.fovValue when not scoped, else delegate to original. |

---

## Chams (Material Override) — _working_

Standard CS2 chams approach: override the material on CCSGOPlayerAnimGraphState::DrawObject path or material pointers on weapon/player render contexts. Use the cs2-dumper material signatures (FindParameter/UpdateParameter from materialsystem2) to swap shader params at draw time.

### Hooks

| Function | Module | Signature | Action |
|---|---|---|---|
| `GeneratePrimitives` | `scenesystem.dll` | `CSceneAnimatableObject::GeneratePrimitives` | Substitute material on player/weapon scene objects. |
| `FindParameter` | `materialsystem2.dll` | `FindParameter` | Look up the param slot. |
| `UpdateParameter` | `materialsystem2.dll` | `UpdateParameter` | Write the new value. |

---

## Fullbright — _broken — does not toggle in build 14153 even with both slots written; suspect engine reads a 3rd location or the cvar object pointer moved. Re-IDA scenesystem.dll for the new value-slot offset._

ConVar mat_fullbright is registered FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY. Source 2 ConVar layout (verified in scenesystem.dll sub_1804ACB70): cvar+0x30 flags DWORD, cvar+0x40 legacy value union, cvar+0x58 modern ConVar<T> value storage. The renderer's fullbright branch is gated on (flags & 0x400) == 0, so we MUST strip FCVAR_CHEAT (0x400) and FCVAR_DEVELOPMENTONLY (0x4000), then write the desired value to BOTH +0x40 (legacy) AND +0x58 (modern) slots — some code paths read each.

### ConVars

| Name | Strip Flags | Write Both Slots | Value |
|---|---|---|---|
| `mat_fullbright` | `0x4400` | true | 1 to enable / 0 to disable |

---

## Third Person — _working_

Hook CCSGOViewAdviceService::OverrideView (signature OverrideView, client.dll). After calling the original (lets engine set up first-person camera), read the eye position out of CViewSetup, build forward from view angles, and rewrite CViewSetup origin to eye - forward*dist + (0,0,8). Leave angles untouched. Also flip the engine input flag at clientBase + dwCSGOInput + 0x229 to true so the local model renders. Restore to false on disable. CViewSetup field offsets (CS2 build 14152): origin=+0x490, angles=+0x4A0, fov=+0x230. Do NOT use c_thirdpersonshoulder ConVar — it's FCVAR_CHEAT and reads from cvar+0x58 (same gotchas as mat_fullbright); direct CViewSetup write is the canonical CS2 approach.

### Hooks

| Function | Module | Signature | Action |
|---|---|---|---|
| `OverrideView` | `client.dll` | `CCSGOViewAdvice::OverrideView` | Rewrite CViewSetup origin (offset 0x490) to a third-person position derived from view angles. |

---

## Anti-Fog — _working_

Disable every fog source per-entity. Cheap and total. env_fog_controller has fogparams_t embedded at +0x608: +0x14 colorPrimary, +0x18 colorSecondary, +0x24 start, +0x28 end, +0x30 maxDensity, +0x64 enable. env_cubemap_fog: +0x62C m_bActive, +0x630 m_flFogMaxOpacity, +0x60C/0x608 start/end distances. env_volumetric_fog_controller: +0x600 m_flScattering, +0x610 m_flDrawDistance, +0x64C m_bActive, +0x674 m_bStartDisabled. env_volumetric_fog_volume: +0x600 m_bActive, +0x620 m_flStrength.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_FogController` | `m_fog (params)` | `0x608` | `fogparams_t` | +0x14 RGB primary, +0x18 RGB secondary, +0x24 start, +0x28 end, +0x30 density, +0x64 enable bool |
| `C_EnvCubemapFog` | `m_bActive` | `0x62C` | `bool` | set false |
| `C_EnvCubemapFog` | `m_flFogMaxOpacity` | `0x630` | `float` | set 0 |
| `C_EnvVolumetricFogController` | `m_bActive` | `0x64C` | `bool` | set false |
| `C_EnvVolumetricFogController` | `m_bStartDisabled` | `0x674` | `bool` | set true |
| `C_EnvVolumetricFogVolume` | `m_bActive` | `0x600` | `bool` | set false |
| `C_EnvVolumetricFogVolume` | `m_flStrength` | `0x620` | `float` | set 0 |

---

## No Color Correction — _working_

Color-correction LUT entities (color_correction designer name) ship on most maps and apply the mood grading (warm dust on Mirage, blue-grey on Anubis, etc). Disabling them is a free visibility upgrade. Zero m_flMaxWeight and m_flCurWeight, clear m_bEnabled and m_bEnabledOnClient[0], zero m_flCurWeightOnClient.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_ColorCorrection` | `m_flMaxWeight` | `0x61C` | `float` | set 0 |
| `C_ColorCorrection` | `m_flCurWeight` | `0x620` | `float` | set 0 |
| `C_ColorCorrection` | `m_bEnabled` | `0x824` | `bool` | set false |
| `C_ColorCorrection` | `m_bEnabledOnClient[0]` | `0x828` | `bool` | set false |
| `C_ColorCorrection` | `m_flCurWeightOnClient` | `0x82C` | `float` | set 0 |

---

## Night Mode / Asus Mode (Sky Tint via env_sky) — _working_

For env_sky entities: m_vTintColor (Color32) at +0xFB9 and m_flBrightnessScale at +0xFC4 DO update live and are safe to poke per-tick. Use these for night/asus presets. The sky-tint hook (DrawSkyboxArray) handles dynamic per-frame color; this entity-write path handles the slower per-preset apply.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `env_sky` | `m_vTintColor` | `0xFB9` | `Color32` | RGBA 0..255 packed |
| `env_sky` | `m_vTintColorLightingOnly` | `0xFBD` | `Color32` | match m_vTintColor for consistency |
| `env_sky` | `m_flBrightnessScale` | `0xFC4` | `float` | 1.0 = default |

---

## ESP — Player Pawn Core — _working_

Iterate dwEntityList (client.dll global) — for each pawn read m_iHealth, m_lifeState (==ALIVE = 0), m_iTeamNum, m_pGameSceneNode → CGameSceneNode for world position, m_hActiveWeapon to look up the held weapon entity, m_iszPlayerName via m_hController → CCSPlayerController. m_vecAbsOrigin lives at +0xC8 on CGameSceneNode. World-to-screen uses dwViewMatrix (4x4 row-major).

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_BaseEntity` | `m_pGameSceneNode` | `0x330` | `CGameSceneNode*` | deref → bone matrix + abs origin |
| `C_BaseEntity` | `m_iHealth` | `0x34C` | `int32` | 0 == dead |
| `C_BaseEntity` | `m_lifeState` | `0x354` | `uint8` | 0 == ALIVE |
| `C_BaseEntity` | `m_iTeamNum` | `0x3EB` | `uint8` | 2=T, 3=CT |
| `CGameSceneNode` | `m_vecAbsOrigin` | `0xC8` | `Vector3` | world position (read for ESP root) |
| `C_CSPlayerPawnBase` | `m_pWeaponServices` | `0x11E0` | `ptr` | pawn-side weapon services |
| `C_CSPlayerPawnBase` | `m_pObserverServices` | `0x11F8` | `ptr` | spectator target via +0x4C m_hObserverTarget |
| `CCSPlayerController` | `m_iszPlayerName` | `0x6F0` | `char[128]` | UTF-8 nickname |
| `CCSPlayerController` | `m_hPawn` | `0x6BC` | `CHandle` | handle → pawn entity |
| `C_BasePlayerWeapon` | `m_iItemDefinitionIndex` | `0x1BA` | `uint16` | weapon definition index (CSWeaponID) |
| `C_BasePlayerWeapon` | `m_iClip1` | `0x16D8` | `int32` | current magazine count |
| `C_CSPlayerPawn` | `m_iIDEntIndex` | `0x33DC` | `CEntityIndex` | entity the local pawn is looking at (handy for triggerbot) |
| `C_CSObserverPawn` | `m_hObserverTarget` | `0x4C` | `CHandle` | current spectated entity (use when local is spectating) |
| `C_CSPlayerPawn` | `m_vOldOrigin` | `0x1390` | `Vector` | previous-tick origin — useful for prediction / extrapolation |

---

## ESP — Skeleton / Bones — _working_

From the pawn → m_pGameSceneNode (+0x330) you reach a CSkeletonInstance (subclass of CGameSceneNode). Inside lives a CModelState at +0x150 whose m_modelSceneNode → CBoneState array is the source of truth for live bone positions. Each CBoneState is 32 bytes: position vec3 at +0x00, quat (4 floats) at +0x20 (engine layout). Bone count comes from the model resource. For ESP just read the chest/head bone indices from CSPlayer model: bone 6 = head, bone 5 = chest on the standard player skeleton.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_BaseEntity` | `m_pGameSceneNode` | `0x330` | `CSkeletonInstance*` | actually CSkeletonInstance for animated entities |
| `CSkeletonInstance` | `m_modelState` | `0x150` | `CModelState` | embedded; bone array pointer is inside |

---

## Silent Aim / Aim Punch / No Recoil — _working_

Recoil/spread is driven entirely by CCSPlayer_AimPunchServices on the pawn (+0x1490). View-angle correction for silent aim writes the desired angles into the engine's CSGOInput at clientBase + dwCSGOInput + 0x4F18 (m_angEyeAngles equivalent in CS2; double-check in your build's CSGOInput struct dump). No-recoil works by zero'ing the aim-punch cache vector before CalcViewModelView reads it, OR by patching the per-shot punch application in CCSPlayer_WeaponServices::FireBullet. The m_iShotsFired counter on CCSPlayerPawn (+0x1C5C) and m_flRecoilIndex on weapons (+0x17E0) drive the spread cone — silent-aim implementations usually compensate by reading both and applying the inverse aim-punch when computing the shot vector.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_CSPlayerPawn` | `m_pAimPunchServices` | `0x1490` | `CCSPlayer_AimPunchServices*` | deref for punch arrays |
| `C_CSPlayerPawn` | `m_iShotsFired` | `0x1C5C` | `int32` | consecutive shots fired this trigger pull (drives spread) |
| `C_CSWeaponBase` | `m_flRecoilIndex` | `0x17E0` | `float` | recoil pattern index |
| `C_CSWeaponBase` | `m_flNextPrimaryAttackTickRatio` | `0x16CC` | `float` | rate-of-fire gate |
| `C_CSPlayerPawn` | `m_pWeaponServices` | `0x11E0` | `ptr` | owns FireBullet (recoil applied here) |

### Hooks

| Function | Module | Signature | Action |
|---|---|---|---|
| `ConvarGet (engine FCVAR helper)` | `client.dll` | `ConvarGet` | Use during init to fetch sv_cheats / weapon_recoil_scale style cvars when bypassing recoil via cvar route. |

---

## Money / Armor / Helmet / Score — _working_

Money lives on CCSPlayerController_InGameMoneyServices (m_iAccount @ +0x40). Armor + helmet on the pawn's CCSPlayer_ItemServices (m_ArmorValue +0x1C74 on pawn directly, m_bHasHelmet +0x49 / m_bHasDefuser +0x48 on the item-services block). Scoreboard kills/deaths/assists at CCSPlayerController_ActionTrackingServices +0x30..+0x38. Useful for ESP info panels and scoreboard reveal.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `CCSPlayerController_InGameMoneyServices` | `m_iAccount` | `0x40` | `int32` | current cash |
| `C_CSPlayerPawn` | `m_ArmorValue` | `0x1C74` | `int32` | armor 0..100 |
| `CCSPlayer_ItemServices` | `m_bHasDefuser` | `0x48` | `bool` | CT defuser flag |
| `CCSPlayer_ItemServices` | `m_bHasHelmet` | `0x49` | `bool` | kevlar+helmet flag |
| `CCSPlayerController_ActionTrackingServices` | `m_iKills` | `0x30` | `int32` | scoreboard kills |
| `CCSPlayerController_ActionTrackingServices` | `m_iDeaths` | `0x34` | `int32` | scoreboard deaths |
| `CCSPlayerController_ActionTrackingServices` | `m_iAssists` | `0x38` | `int32` | scoreboard assists |
| `C_CSPlayerPawn` | `m_unCurrentEquipmentValue` | `0x1C78` | `uint16` | rounded loadout $$ |

---

## Spotted / Glow (Radar Hack) — _working_

Force m_bSpotted = true on enemy CEntitySpottedState_t (lives at +0x8 inside the pawn's spotted-state block; m_bSpottedByMask at +0xC). Radar reads from this every frame, no engine hook needed. Also tweak the GlowProperty colour on the pawn for the chams-lite outline path.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `EntitySpottedState_t` | `m_bSpotted` | `0x8` | `bool` | force true to reveal on radar |
| `EntitySpottedState_t` | `m_bSpottedByMask` | `0xC` | `uint32[2]` | bit per spotter; OR with 0xFFFFFFFF |

---

## Rank Reveal (Premier MMR) — _working_

Enemy competitive rank lives on CCSPlayerController +0x878 (m_iCompetitiveRanking). Predicted win/loss/tie at +0x884 / +0x888 / +0x88C. Available at warmup before the rank icons are censored.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `CCSPlayerController` | `m_iCompetitiveRanking` | `0x878` | `int32` | Premier rating |
| `CCSPlayerController` | `m_iCompetitiveRankingPredicted_Win` | `0x884` | `int32` | +rating on win |
| `CCSPlayerController` | `m_iCompetitiveRankingPredicted_Loss` | `0x888` | `int32` | -rating on loss |
| `CCSPlayerController` | `m_iCompetitiveRankingPredicted_Tie` | `0x88C` | `int32` | +/- rating on draw |

---

## Globals — RVAs in client.dll — _working_

These are the pointers/arrays the cheat reads first on every frame. They live in client.dll and shift on most updates — the rest of the catalogue is meaningless without these. dwLocalPlayerPawn + dwLocalPlayerController are pointer globals (deref once). dwEntityList is a CEntitySystem* (game scene → entity by handle). dwViewMatrix is a 4x4 float[16] used for world→screen. dwViewAngles is a Vector3 (pitch, yaw, roll). dwCSGOInput holds engine input flags + the +0x229 third-person bool and +0x4F18 view-angles slot.

---

