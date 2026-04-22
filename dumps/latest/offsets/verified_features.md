# Verified Working Features

Hand-curated catalogue of CS2 features that have been **confirmed working in a live internal cheat** against the current build. Cross-reference with the auto-generated `offsets.*`, `signatures.*` and `client_dll.*` files for canonical addresses; this document captures the *gotchas* — the kind of thing that took an evening of IDA work to figure out.

**CS2 build:** `14153`

**Feature count:** 11

---

## No Smoke

Iterate the entity list, identify C_SmokeGrenadeProjectile via CEntityIdentity::m_designerName == "smokegrenade_projectile", zero m_nSmokeEffectTickBegin and clear m_bDidSmokeEffect. Engine re-evaluates every tick, so writes stick. Also zero m_flLastSmokeOverlayAlpha on the local pawn for the screen overlay.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_SmokeGrenadeProjectile` | `m_nSmokeEffectTickBegin` | `0x1250` | `int32` | set 0 to skip the puff |
| `C_SmokeGrenadeProjectile` | `m_bDidSmokeEffect` | `0x1254` | `bool` | set false |
| `C_CSPlayerPawn` | `m_flLastSmokeOverlayAlpha` | `0x1420` | `float` | local pawn's screen overlay alpha; set 0 |

---

## Smoke Color

Same entity walk as No Smoke; write a Vector (3 floats * 255) to m_vSmokeColor. Particle system reads this every frame.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_SmokeGrenadeProjectile` | `m_vSmokeColor` | `0x125C` | `Vector3 (RGB 0..255)` | 3 floats, multiply RGB by 255 |

---

## No Flash

Zero m_flFlashDuration and m_flFlashMaxAlpha on the local pawn. Trip the write only when duration > 0 to avoid spamming the engine.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `C_CSPlayerPawn` | `m_flFlashDuration` | `0x1400` | `float` | set 0 to clear flash |
| `C_CSPlayerPawn` | `m_flFlashMaxAlpha` | `0x13FC` | `float` | set 0 for full removal, 0..255 for partial |

---

## Skybox Tint

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

## FOV Changer

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

## Chams (Material Override)

Standard CS2 chams approach: override the material on CCSGOPlayerAnimGraphState::DrawObject path or material pointers on weapon/player render contexts. Use the cs2-dumper material signatures (FindParameter/UpdateParameter from materialsystem2) to swap shader params at draw time.

### Hooks

| Function | Module | Signature | Action |
|---|---|---|---|
| `GeneratePrimitives` | `scenesystem.dll` | `CSceneAnimatableObject::GeneratePrimitives` | Substitute material on player/weapon scene objects. |
| `FindParameter` | `materialsystem2.dll` | `FindParameter` | Look up the param slot. |
| `UpdateParameter` | `materialsystem2.dll` | `UpdateParameter` | Write the new value. |

---

## Fullbright

ConVar mat_fullbright is registered FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY. Source 2 ConVar layout (verified in scenesystem.dll sub_1804ACB70): cvar+0x30 flags DWORD, cvar+0x40 legacy value union, cvar+0x58 modern ConVar<T> value storage. The renderer's fullbright branch is gated on (flags & 0x400) == 0, so we MUST strip FCVAR_CHEAT (0x400) and FCVAR_DEVELOPMENTONLY (0x4000), then write the desired value to BOTH +0x40 (legacy) AND +0x58 (modern) slots — some code paths read each.

### ConVars

| Name | Strip Flags | Write Both Slots | Value |
|---|---|---|---|
| `mat_fullbright` | `0x4400` | true | 1 to enable / 0 to disable |

---

## Third Person

Hook CCSGOViewAdviceService::OverrideView (signature OverrideView, client.dll). After calling the original (lets engine set up first-person camera), read the eye position out of CViewSetup, build forward from view angles, and rewrite CViewSetup origin to eye - forward*dist + (0,0,8). Leave angles untouched. Also flip the engine input flag at clientBase + dwCSGOInput + 0x229 to true so the local model renders. Restore to false on disable. CViewSetup field offsets (CS2 build 14152): origin=+0x490, angles=+0x4A0, fov=+0x230. Do NOT use c_thirdpersonshoulder ConVar — it's FCVAR_CHEAT and reads from cvar+0x58 (same gotchas as mat_fullbright); direct CViewSetup write is the canonical CS2 approach.

### Hooks

| Function | Module | Signature | Action |
|---|---|---|---|
| `OverrideView` | `client.dll` | `CCSGOViewAdvice::OverrideView` | Rewrite CViewSetup origin (offset 0x490) to a third-person position derived from view angles. |

---

## Anti-Fog

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

## No Color Correction

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

## Night Mode / Asus Mode (Sky Tint via env_sky)

For env_sky entities: m_vTintColor (Color32) at +0xFB9 and m_flBrightnessScale at +0xFC4 DO update live and are safe to poke per-tick. Use these for night/asus presets. The sky-tint hook (DrawSkyboxArray) handles dynamic per-frame color; this entity-write path handles the slower per-preset apply.

### Fields

| Class | Field | Offset | Type | Note |
|---|---|---|---|---|
| `env_sky` | `m_vTintColor` | `0xFB9` | `Color32` | RGBA 0..255 packed |
| `env_sky` | `m_vTintColorLightingOnly` | `0xFBD` | `Color32` | match m_vTintColor for consistency |
| `env_sky` | `m_flBrightnessScale` | `0xFC4` | `float` | 1.0 = default |

---

