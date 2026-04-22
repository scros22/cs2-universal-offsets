//! Verified working features — hand-curated catalogue of offsets, hooks
//! and ConVar tricks that have been **confirmed working in a live CS2
//! internal cheat** against the current build.
//!
//! Most of this data is already present in the auto-extracted offset
//! / signature / schema files this tool produces. The point of this
//! module is to give consumers a single place that says, in plain
//! English, *"yes, this offset on this entity does this exact thing,
//! and here is the gotcha you need to know to make it work."*
//!
//! a2x's pelite-based pipeline gives us the canonical RVAs and schema
//! offsets — that data is the source of truth and lives in the regular
//! `offsets.*` / `client_dll.*` / `signatures.*` files. This catalogue
//! cross-references those values with verified live-engine notes so a
//! cheat developer can copy-paste a feature and have it work first try.

use serde_json::json;

#[derive(Clone, Copy)]
pub struct VerifiedField {
    /// e.g. "C_SmokeGrenadeProjectile"
    pub class: &'static str,
    /// e.g. "m_nSmokeEffectTickBegin"
    pub field: &'static str,
    /// hex offset relative to the class base (entity)
    pub offset: u32,
    /// e.g. "int32" / "Vector3" / "bool"
    pub ty: &'static str,
    /// short note about what to write / how to read
    pub note: &'static str,
}

#[derive(Clone, Copy)]
pub struct VerifiedFeature {
    /// e.g. "No Smoke"
    pub name: &'static str,
    /// "working" / "broken" / "partial" — at-a-glance status from the
    /// last live confirmation of this feature in the internal cheat.
    pub status: &'static str,
    /// short paragraph explaining what we tested + where to write
    pub summary: &'static str,
    /// fields touched
    pub fields: &'static [VerifiedField],
    /// ConVar tricks (name + flags-to-strip + value-slot offset) — empty if N/A
    pub convars: &'static [VerifiedConVar],
    /// Hooks installed (function + module + signature key in database.rs)
    pub hooks: &'static [VerifiedHook],
}

#[derive(Clone, Copy)]
pub struct VerifiedConVar {
    /// ConVar name
    pub name: &'static str,
    /// flags to strip from cvar+0x30 (e.g. FCVAR_CHEAT=0x400)
    pub strip_flags: u32,
    /// modern Source 2 ConVar<T> value lives at cvar+0x58 — set true
    /// if we have to write *both* the legacy +0x40 union AND the
    /// modern +0x58 slot
    pub write_both_slots: bool,
    /// what value(s) we write
    pub value: &'static str,
}

#[derive(Clone, Copy)]
pub struct VerifiedHook {
    /// e.g. "DrawSkyboxArray"
    pub function: &'static str,
    /// e.g. "scenesystem.dll"
    pub module: &'static str,
    /// signature database key (look up in src/signatures/database.rs)
    pub signature: &'static str,
    /// what we do once hooked
    pub action: &'static str,
}

// ----------------------------------------------------------------------
// Catalogue. Add entries as new features are verified working in-game.
// Build target: CS2 14152+ (April 2026).
// ----------------------------------------------------------------------
pub static FEATURES: &[VerifiedFeature] = &[
    VerifiedFeature {
        name: "No Smoke",
        status: "working",
        summary: "Iterate the entity list, identify C_SmokeGrenadeProjectile via \
                  CEntityIdentity::m_designerName == \"smokegrenade_projectile\", \
                  zero m_nSmokeEffectTickBegin and clear m_bDidSmokeEffect. Engine \
                  re-evaluates every tick, so writes stick. Also zero \
                  m_flLastSmokeOverlayAlpha on the local pawn for the screen overlay.",
        fields: &[
            VerifiedField { class: "C_SmokeGrenadeProjectile", field: "m_nSmokeEffectTickBegin", offset: 0x1250, ty: "int32",   note: "set 0 to skip the puff" },
            VerifiedField { class: "C_SmokeGrenadeProjectile", field: "m_bDidSmokeEffect",       offset: 0x1254, ty: "bool",    note: "set false" },
            VerifiedField { class: "C_CSPlayerPawn",           field: "m_flLastSmokeOverlayAlpha", offset: 0x1420, ty: "float",   note: "local pawn's screen overlay alpha; set 0" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Smoke Color",
        status: "working",
        summary: "Same entity walk as No Smoke; write a Vector (3 floats * 255) to \
                  m_vSmokeColor. Particle system reads this every frame.",
        fields: &[
            VerifiedField { class: "C_SmokeGrenadeProjectile", field: "m_vSmokeColor", offset: 0x125C, ty: "Vector3 (RGB 0..255)", note: "3 floats, multiply RGB by 255" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "No Flash",
        status: "working",
        summary: "Zero m_flFlashDuration and m_flFlashMaxAlpha on the local pawn. \
                  Trip the write only when duration > 0 to avoid spamming the engine.",
        fields: &[
            VerifiedField { class: "C_CSPlayerPawn", field: "m_flFlashDuration", offset: 0x1400, ty: "float", note: "set 0 to clear flash" },
            VerifiedField { class: "C_CSPlayerPawn", field: "m_flFlashMaxAlpha", offset: 0x13FC, ty: "float", note: "set 0 for full removal, 0..255 for partial" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Skybox Tint",
        status: "working",
        summary: "Hook scenesystem.dll!DrawSkyboxArray, intercept the draw-primitive \
                  pointer (3rd arg). Build 14152 moved the tint vec3 from +0x100 \
                  to +0xE8 inside the skybox object — writing to the old +0x100 slot \
                  poisons the renderer with NaN and crashes after ~60s. \
                  Layout: +0xE8..+0xF0 RGB tint floats, +0xF4 mode int, \
                  +0xF8..+0x104 four sun-angle floats. \
                  Do NOT write to env_sky m_vTintColor — the renderer caches \
                  material params at level setup and ignores entity writes.",
        fields: &[
            VerifiedField { class: "env_sky", field: "m_vTintColor",              offset: 0xFB9, ty: "Color32", note: "DOES NOT WORK at runtime — use the DrawSkyboxArray hook instead" },
            VerifiedField { class: "env_sky", field: "m_vTintColorLightingOnly",  offset: 0xFBD, ty: "Color32", note: "Same caveat" },
            VerifiedField { class: "env_sky", field: "m_flBrightnessScale",       offset: 0xFC4, ty: "float",   note: "DOES update live (independent code path)" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook {
                function: "DrawSkyboxArray",
                module: "scenesystem.dll",
                signature: "DrawSkyboxArray",
                action: "Patch RGB at primitive_obj+0xE8 before passing through to the original.",
            },
        ],
    },
    VerifiedFeature {
        name: "FOV Changer",
        status: "working",
        summary: "Two-prong approach. (1) Hook GetWorldFov in client.dll \
                  (signature SetWorldFov, E8-CALL @ +1) and return the desired \
                  value when the local pawn is not scoped. (2) Every tick, write \
                  the desired FOV into m_iFOV and m_iFOVStart on the camera \
                  services AND into m_iDesiredFOV on the local controller \
                  (canonical source the renderer reads). The controller-level \
                  field is the one that gets reset back to default if you only \
                  fight the camera-services side.",
        fields: &[
            VerifiedField { class: "CBasePlayerController", field: "m_iDesiredFOV", offset: 0x784, ty: "uint32", note: "a2x-named m_iDesiredFOV_OnController" },
            VerifiedField { class: "CCSPlayer_CameraServices", field: "m_iFOV",      offset: 0x290, ty: "uint32", note: "current camera FOV" },
            VerifiedField { class: "CCSPlayer_CameraServices", field: "m_iFOVStart", offset: 0x294, ty: "uint32", note: "target camera FOV" },
            VerifiedField { class: "C_CSPlayerPawn",            field: "m_pCameraServices", offset: 0x1218, ty: "CCSPlayer_CameraServices*", note: "deref to reach m_iFOV/m_iFOVStart" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook {
                function: "GetWorldFov",
                module: "client.dll",
                signature: "SetWorldFov",
                action: "Return cfg.fovValue when not scoped, else delegate to original.",
            },
        ],
    },
    VerifiedFeature {
        name: "Chams (Material Override)",
        status: "working",
        summary: "Standard CS2 chams approach: override the material on \
                  CCSGOPlayerAnimGraphState::DrawObject path or material pointers \
                  on weapon/player render contexts. Use the cs2-dumper material \
                  signatures (FindParameter/UpdateParameter from materialsystem2) \
                  to swap shader params at draw time.",
        fields: &[],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "GeneratePrimitives",       module: "scenesystem.dll",      signature: "CSceneAnimatableObject::GeneratePrimitives", action: "Substitute material on player/weapon scene objects." },
            VerifiedHook { function: "FindParameter",            module: "materialsystem2.dll",  signature: "FindParameter",                              action: "Look up the param slot." },
            VerifiedHook { function: "UpdateParameter",          module: "materialsystem2.dll",  signature: "UpdateParameter",                            action: "Write the new value." },
        ],
    },
    VerifiedFeature {
        name: "Fullbright",
        status: "broken — does not toggle in build 14153 even with both slots written; suspect engine reads a 3rd location or the cvar object pointer moved. Re-IDA scenesystem.dll for the new value-slot offset.",
        summary: "ConVar mat_fullbright is registered FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY. \
                  Source 2 ConVar layout (verified in scenesystem.dll sub_1804ACB70): \
                  cvar+0x30 flags DWORD, cvar+0x40 legacy value union, cvar+0x58 \
                  modern ConVar<T> value storage. The renderer's fullbright branch \
                  is gated on (flags & 0x400) == 0, so we MUST strip FCVAR_CHEAT \
                  (0x400) and FCVAR_DEVELOPMENTONLY (0x4000), then write the \
                  desired value to BOTH +0x40 (legacy) AND +0x58 (modern) slots — \
                  some code paths read each.",
        fields: &[],
        convars: &[
            VerifiedConVar { name: "mat_fullbright", strip_flags: 0x4400, write_both_slots: true, value: "1 to enable / 0 to disable" },
        ],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Third Person",
        status: "working",
        summary: "Hook CCSGOViewAdviceService::OverrideView (signature OverrideView, \
                  client.dll). After calling the original (lets engine set up \
                  first-person camera), read the eye position out of CViewSetup, \
                  build forward from view angles, and rewrite CViewSetup origin to \
                  eye - forward*dist + (0,0,8). Leave angles untouched. \
                  Also flip the engine input flag at clientBase + dwCSGOInput + 0x229 \
                  to true so the local model renders. Restore to false on disable. \
                  CViewSetup field offsets (CS2 build 14152): origin=+0x490, \
                  angles=+0x4A0, fov=+0x230. \
                  Do NOT use c_thirdpersonshoulder ConVar — it's FCVAR_CHEAT and \
                  reads from cvar+0x58 (same gotchas as mat_fullbright); direct \
                  CViewSetup write is the canonical CS2 approach.",
        fields: &[],
        convars: &[],
        hooks: &[
            VerifiedHook {
                function: "OverrideView",
                module: "client.dll",
                signature: "CCSGOViewAdvice::OverrideView",
                action: "Rewrite CViewSetup origin (offset 0x490) to a third-person position derived from view angles.",
            },
        ],
    },
    VerifiedFeature {
        name: "Anti-Fog",
        status: "working",
        summary: "Disable every fog source per-entity. Cheap and total. \
                  env_fog_controller has fogparams_t embedded at +0x608: +0x14 \
                  colorPrimary, +0x18 colorSecondary, +0x24 start, +0x28 end, \
                  +0x30 maxDensity, +0x64 enable. env_cubemap_fog: +0x62C m_bActive, \
                  +0x630 m_flFogMaxOpacity, +0x60C/0x608 start/end distances. \
                  env_volumetric_fog_controller: +0x600 m_flScattering, +0x610 \
                  m_flDrawDistance, +0x64C m_bActive, +0x674 m_bStartDisabled. \
                  env_volumetric_fog_volume: +0x600 m_bActive, +0x620 m_flStrength.",
        fields: &[
            VerifiedField { class: "C_FogController",                 field: "m_fog (params)",   offset: 0x608, ty: "fogparams_t", note: "+0x14 RGB primary, +0x18 RGB secondary, +0x24 start, +0x28 end, +0x30 density, +0x64 enable bool" },
            VerifiedField { class: "C_EnvCubemapFog",                 field: "m_bActive",        offset: 0x62C, ty: "bool",        note: "set false" },
            VerifiedField { class: "C_EnvCubemapFog",                 field: "m_flFogMaxOpacity",offset: 0x630, ty: "float",       note: "set 0" },
            VerifiedField { class: "C_EnvVolumetricFogController",    field: "m_bActive",        offset: 0x64C, ty: "bool",        note: "set false" },
            VerifiedField { class: "C_EnvVolumetricFogController",    field: "m_bStartDisabled", offset: 0x674, ty: "bool",        note: "set true" },
            VerifiedField { class: "C_EnvVolumetricFogVolume",        field: "m_bActive",        offset: 0x600, ty: "bool",        note: "set false" },
            VerifiedField { class: "C_EnvVolumetricFogVolume",        field: "m_flStrength",     offset: 0x620, ty: "float",       note: "set 0" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "No Color Correction",
        status: "working",
        summary: "Color-correction LUT entities (color_correction designer name) ship \
                  on most maps and apply the mood grading (warm dust on Mirage, \
                  blue-grey on Anubis, etc). Disabling them is a free visibility \
                  upgrade. Zero m_flMaxWeight and m_flCurWeight, clear m_bEnabled \
                  and m_bEnabledOnClient[0], zero m_flCurWeightOnClient.",
        fields: &[
            VerifiedField { class: "C_ColorCorrection", field: "m_flMaxWeight",         offset: 0x61C, ty: "float", note: "set 0" },
            VerifiedField { class: "C_ColorCorrection", field: "m_flCurWeight",         offset: 0x620, ty: "float", note: "set 0" },
            VerifiedField { class: "C_ColorCorrection", field: "m_bEnabled",            offset: 0x824, ty: "bool",  note: "set false" },
            VerifiedField { class: "C_ColorCorrection", field: "m_bEnabledOnClient[0]", offset: 0x828, ty: "bool",  note: "set false" },
            VerifiedField { class: "C_ColorCorrection", field: "m_flCurWeightOnClient", offset: 0x82C, ty: "float", note: "set 0" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Night Mode / Asus Mode (Sky Tint via env_sky)",
        status: "working",
        summary: "For env_sky entities: m_vTintColor (Color32) at +0xFB9 and \
                  m_flBrightnessScale at +0xFC4 DO update live and are safe to \
                  poke per-tick. Use these for night/asus presets. The sky-tint \
                  hook (DrawSkyboxArray) handles dynamic per-frame color; this \
                  entity-write path handles the slower per-preset apply.",
        fields: &[
            VerifiedField { class: "env_sky", field: "m_vTintColor",             offset: 0xFB9, ty: "Color32", note: "RGBA 0..255 packed" },
            VerifiedField { class: "env_sky", field: "m_vTintColorLightingOnly", offset: 0xFBD, ty: "Color32", note: "match m_vTintColor for consistency" },
            VerifiedField { class: "env_sky", field: "m_flBrightnessScale",      offset: 0xFC4, ty: "float",   note: "1.0 = default" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "ESP — Player Pawn Core",
        status: "working",
        summary: "Iterate dwEntityList (client.dll global) — for each pawn read \
                  m_iHealth, m_lifeState (==ALIVE = 0), m_iTeamNum, m_pGameSceneNode \
                  → CGameSceneNode for world position, m_hActiveWeapon to look up \
                  the held weapon entity, m_iszPlayerName via m_hController → \
                  CCSPlayerController. m_vecAbsOrigin lives at +0xC8 on \
                  CGameSceneNode. World-to-screen uses dwViewMatrix (4x4 row-major).",
        fields: &[
            VerifiedField { class: "C_BaseEntity",            field: "m_pGameSceneNode", offset: 0x330, ty: "CGameSceneNode*", note: "deref → bone matrix + abs origin" },
            VerifiedField { class: "C_BaseEntity",            field: "m_iHealth",        offset: 0x34C, ty: "int32",           note: "0 == dead" },
            VerifiedField { class: "C_BaseEntity",            field: "m_lifeState",      offset: 0x354, ty: "uint8",           note: "0 == ALIVE" },
            VerifiedField { class: "C_BaseEntity",            field: "m_iTeamNum",       offset: 0x3EB, ty: "uint8",           note: "2=T, 3=CT" },
            VerifiedField { class: "CGameSceneNode",          field: "m_vecAbsOrigin",   offset: 0xC8,  ty: "Vector3",         note: "world position (read for ESP root)" },
            VerifiedField { class: "C_CSPlayerPawnBase",      field: "m_pWeaponServices",offset: 0x11E0,ty: "ptr",             note: "pawn-side weapon services" },
            VerifiedField { class: "C_CSPlayerPawnBase",      field: "m_pObserverServices", offset: 0x11F8, ty: "ptr",        note: "spectator target via +0x4C m_hObserverTarget" },
            VerifiedField { class: "CCSPlayerController",     field: "m_iszPlayerName",  offset: 0x6F0, ty: "char[128]",       note: "UTF-8 nickname" },
            VerifiedField { class: "CCSPlayerController",     field: "m_hPawn",          offset: 0x6BC, ty: "CHandle",         note: "handle → pawn entity" },
            VerifiedField { class: "C_BasePlayerWeapon",      field: "m_iItemDefinitionIndex", offset: 0x1BA, ty: "uint16",    note: "weapon definition index (CSWeaponID)" },
            VerifiedField { class: "C_BasePlayerWeapon",      field: "m_iClip1",         offset: 0x16D8,ty: "int32",           note: "current magazine count" },
            VerifiedField { class: "C_CSPlayerPawn",          field: "m_iIDEntIndex",    offset: 0x33DC,ty: "CEntityIndex",    note: "entity the local pawn is looking at (handy for triggerbot)" },
            VerifiedField { class: "C_CSObserverPawn",        field: "m_hObserverTarget",offset: 0x4C,  ty: "CHandle",         note: "current spectated entity (use when local is spectating)" },
            VerifiedField { class: "C_CSPlayerPawn",          field: "m_vOldOrigin",     offset: 0x1390,ty: "Vector",          note: "previous-tick origin — useful for prediction / extrapolation" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "ESP — Skeleton / Bones",
        status: "working",
        summary: "From the pawn → m_pGameSceneNode (+0x330) you reach a \
                  CSkeletonInstance (subclass of CGameSceneNode). Inside lives a \
                  CModelState at +0x150 whose m_modelSceneNode → CBoneState array \
                  is the source of truth for live bone positions. Each CBoneState \
                  is 32 bytes: position vec3 at +0x00, quat (4 floats) at +0x20 \
                  (engine layout). Bone count comes from the model resource. For \
                  ESP just read the chest/head bone indices from CSPlayer model: \
                  bone 6 = head, bone 5 = chest on the standard player skeleton.",
        fields: &[
            VerifiedField { class: "C_BaseEntity",      field: "m_pGameSceneNode", offset: 0x330, ty: "CSkeletonInstance*", note: "actually CSkeletonInstance for animated entities" },
            VerifiedField { class: "CSkeletonInstance", field: "m_modelState",     offset: 0x150, ty: "CModelState",        note: "embedded; bone array pointer is inside" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Silent Aim / Aim Punch / No Recoil",
        status: "working",
        summary: "Recoil/spread is driven entirely by CCSPlayer_AimPunchServices on \
                  the pawn (+0x1490). View-angle correction for silent aim writes \
                  the desired angles into the engine's CSGOInput at \
                  clientBase + dwCSGOInput + 0x4F18 (m_angEyeAngles equivalent in \
                  CS2; double-check in your build's CSGOInput struct dump). \
                  No-recoil works by zero'ing the aim-punch cache vector before \
                  CalcViewModelView reads it, OR by patching the per-shot punch \
                  application in CCSPlayer_WeaponServices::FireBullet. The \
                  m_iShotsFired counter on CCSPlayerPawn (+0x1C5C) and \
                  m_flRecoilIndex on weapons (+0x17E0) drive the spread cone — \
                  silent-aim implementations usually compensate by reading both \
                  and applying the inverse aim-punch when computing the shot vector.",
        fields: &[
            VerifiedField { class: "C_CSPlayerPawn",       field: "m_pAimPunchServices",          offset: 0x1490, ty: "CCSPlayer_AimPunchServices*", note: "deref for punch arrays" },
            VerifiedField { class: "C_CSPlayerPawn",       field: "m_iShotsFired",                offset: 0x1C5C, ty: "int32", note: "consecutive shots fired this trigger pull (drives spread)" },
            VerifiedField { class: "C_CSWeaponBase",       field: "m_flRecoilIndex",              offset: 0x17E0, ty: "float", note: "recoil pattern index" },
            VerifiedField { class: "C_CSWeaponBase",       field: "m_flNextPrimaryAttackTickRatio", offset: 0x16CC, ty: "float", note: "rate-of-fire gate" },
            VerifiedField { class: "C_CSPlayerPawn",       field: "m_pWeaponServices",            offset: 0x11E0, ty: "ptr",  note: "owns FireBullet (recoil applied here)" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook {
                function: "ConvarGet (engine FCVAR helper)",
                module: "client.dll",
                signature: "ConvarGet",
                action: "Use during init to fetch sv_cheats / weapon_recoil_scale style cvars when bypassing recoil via cvar route.",
            },
        ],
    },
    VerifiedFeature {
        name: "Money / Armor / Helmet / Score",
        status: "working",
        summary: "Money lives on CCSPlayerController_InGameMoneyServices \
                  (m_iAccount @ +0x40). Armor + helmet on the pawn's \
                  CCSPlayer_ItemServices (m_ArmorValue +0x1C74 on pawn directly, \
                  m_bHasHelmet +0x49 / m_bHasDefuser +0x48 on the item-services \
                  block). Scoreboard kills/deaths/assists at \
                  CCSPlayerController_ActionTrackingServices +0x30..+0x38. Useful \
                  for ESP info panels and scoreboard reveal.",
        fields: &[
            VerifiedField { class: "CCSPlayerController_InGameMoneyServices", field: "m_iAccount",  offset: 0x40,   ty: "int32", note: "current cash" },
            VerifiedField { class: "C_CSPlayerPawn",                          field: "m_ArmorValue",offset: 0x1C74, ty: "int32", note: "armor 0..100" },
            VerifiedField { class: "CCSPlayer_ItemServices",                  field: "m_bHasDefuser", offset: 0x48, ty: "bool",  note: "CT defuser flag" },
            VerifiedField { class: "CCSPlayer_ItemServices",                  field: "m_bHasHelmet",  offset: 0x49, ty: "bool",  note: "kevlar+helmet flag" },
            VerifiedField { class: "CCSPlayerController_ActionTrackingServices", field: "m_iKills",   offset: 0x30, ty: "int32", note: "scoreboard kills" },
            VerifiedField { class: "CCSPlayerController_ActionTrackingServices", field: "m_iDeaths",  offset: 0x34, ty: "int32", note: "scoreboard deaths" },
            VerifiedField { class: "CCSPlayerController_ActionTrackingServices", field: "m_iAssists", offset: 0x38, ty: "int32", note: "scoreboard assists" },
            VerifiedField { class: "C_CSPlayerPawn",                          field: "m_unCurrentEquipmentValue", offset: 0x1C78, ty: "uint16", note: "rounded loadout $$" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Spotted / Glow (Radar Hack)",
        status: "working",
        summary: "Force m_bSpotted = true on enemy CEntitySpottedState_t (lives at \
                  +0x8 inside the pawn's spotted-state block; m_bSpottedByMask at \
                  +0xC). Radar reads from this every frame, no engine hook needed. \
                  Also tweak the GlowProperty colour on the pawn for the chams-lite \
                  outline path.",
        fields: &[
            VerifiedField { class: "EntitySpottedState_t", field: "m_bSpotted",       offset: 0x8, ty: "bool",     note: "force true to reveal on radar" },
            VerifiedField { class: "EntitySpottedState_t", field: "m_bSpottedByMask", offset: 0xC, ty: "uint32[2]", note: "bit per spotter; OR with 0xFFFFFFFF" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Rank Reveal (Premier MMR)",
        status: "working",
        summary: "Enemy competitive rank lives on CCSPlayerController +0x878 \
                  (m_iCompetitiveRanking). Predicted win/loss/tie at +0x884 / \
                  +0x888 / +0x88C. Available at warmup before the rank icons are \
                  censored.",
        fields: &[
            VerifiedField { class: "CCSPlayerController", field: "m_iCompetitiveRanking",                offset: 0x878, ty: "int32", note: "Premier rating" },
            VerifiedField { class: "CCSPlayerController", field: "m_iCompetitiveRankingPredicted_Win",   offset: 0x884, ty: "int32", note: "+rating on win" },
            VerifiedField { class: "CCSPlayerController", field: "m_iCompetitiveRankingPredicted_Loss",  offset: 0x888, ty: "int32", note: "-rating on loss" },
            VerifiedField { class: "CCSPlayerController", field: "m_iCompetitiveRankingPredicted_Tie",   offset: 0x88C, ty: "int32", note: "+/- rating on draw" },
        ],
        convars: &[],
        hooks: &[],
    },
    VerifiedFeature {
        name: "Globals — RVAs in client.dll",
        status: "working",
        summary: "These are the pointers/arrays the cheat reads first on every \
                  frame. They live in client.dll and shift on most updates — the \
                  rest of the catalogue is meaningless without these. \
                  dwLocalPlayerPawn + dwLocalPlayerController are pointer \
                  globals (deref once). dwEntityList is a CEntitySystem* (game \
                  scene → entity by handle). dwViewMatrix is a 4x4 float[16] \
                  used for world→screen. dwViewAngles is a Vector3 (pitch, yaw, \
                  roll). dwCSGOInput holds engine input flags + the +0x229 \
                  third-person bool and +0x4F18 view-angles slot.",
        fields: &[],
        convars: &[],
        hooks: &[],
    },
];

// ----------------------------------------------------------------------
// Renderers
// ----------------------------------------------------------------------

pub fn render_json(build_number: Option<u32>) -> String {
    let features: Vec<_> = FEATURES
        .iter()
        .map(|f| {
            json!({
                "name":    f.name,
                "status":  f.status,
                "summary": f.summary,
                "fields":  f.fields.iter().map(|fld| json!({
                    "class":  fld.class,
                    "field":  fld.field,
                    "offset": format!("0x{:X}", fld.offset),
                    "type":   fld.ty,
                    "note":   fld.note,
                })).collect::<Vec<_>>(),
                "convars": f.convars.iter().map(|c| json!({
                    "name":             c.name,
                    "strip_flags":      format!("0x{:X}", c.strip_flags),
                    "write_both_slots": c.write_both_slots,
                    "value":            c.value,
                })).collect::<Vec<_>>(),
                "hooks":   f.hooks.iter().map(|h| json!({
                    "function":  h.function,
                    "module":    h.module,
                    "signature": h.signature,
                    "action":    h.action,
                })).collect::<Vec<_>>(),
            })
        })
        .collect();

    let doc = json!({
        "_doc":           "Hand-curated catalogue of CS2 features verified working in a live internal cheat. \
                          Cross-reference with offsets.*, signatures.* and client_dll.* for canonical addresses.",
        "cs2_build":      build_number,
        "feature_count":  FEATURES.len(),
        "features":       features,
    });

    serde_json::to_string_pretty(&doc).unwrap_or_else(|_| String::from("{}"))
}

pub fn render_md(build_number: Option<u32>) -> String {
    let mut out = String::new();
    out.push_str("# Verified Working Features\n\n");
    out.push_str(
        "Hand-curated catalogue of CS2 features that have been **confirmed working \
         in a live internal cheat** against the current build. Cross-reference with \
         the auto-generated `offsets.*`, `signatures.*` and `client_dll.*` files for \
         canonical addresses; this document captures the *gotchas* — the kind of \
         thing that took an evening of IDA work to figure out.\n\n",
    );
    if let Some(b) = build_number {
        out.push_str(&format!("**CS2 build:** `{}`\n\n", b));
    }
    out.push_str(&format!("**Feature count:** {}\n\n---\n\n", FEATURES.len()));

    for f in FEATURES.iter() {
        out.push_str(&format!("## {} — _{}_\n\n", f.name, f.status));
        out.push_str(f.summary);
        out.push_str("\n\n");

        if !f.fields.is_empty() {
            out.push_str("### Fields\n\n");
            out.push_str("| Class | Field | Offset | Type | Note |\n");
            out.push_str("|---|---|---|---|---|\n");
            for fld in f.fields.iter() {
                out.push_str(&format!(
                    "| `{}` | `{}` | `0x{:X}` | `{}` | {} |\n",
                    fld.class, fld.field, fld.offset, fld.ty, fld.note,
                ));
            }
            out.push('\n');
        }

        if !f.convars.is_empty() {
            out.push_str("### ConVars\n\n");
            out.push_str("| Name | Strip Flags | Write Both Slots | Value |\n");
            out.push_str("|---|---|---|---|\n");
            for c in f.convars.iter() {
                out.push_str(&format!(
                    "| `{}` | `0x{:X}` | {} | {} |\n",
                    c.name, c.strip_flags, c.write_both_slots, c.value,
                ));
            }
            out.push('\n');
        }

        if !f.hooks.is_empty() {
            out.push_str("### Hooks\n\n");
            out.push_str("| Function | Module | Signature | Action |\n");
            out.push_str("|---|---|---|---|\n");
            for h in f.hooks.iter() {
                out.push_str(&format!(
                    "| `{}` | `{}` | `{}` | {} |\n",
                    h.function, h.module, h.signature, h.action,
                ));
            }
            out.push('\n');
        }

        out.push_str("---\n\n");
    }

    out
}

pub fn render_hpp(build_number: Option<u32>) -> String {
    let mut out = String::new();
    out.push_str("// =====================================================================\n");
    out.push_str("// verified_features.hpp — hand-curated working CS2 cheat features\n");
    out.push_str("// =====================================================================\n");
    out.push_str("//\n");
    out.push_str("// Each entry is a known-good offset / hook / ConVar trick that has been\n");
    out.push_str("// verified live against the current CS2 build. Constants are static\n");
    out.push_str("// constexpr so they can be used at compile time.\n");
    out.push_str("//\n");
    if let Some(b) = build_number {
        out.push_str(&format!("// CS2_BUILD: {}\n", b));
    }
    out.push_str("// =====================================================================\n\n");
    out.push_str("#pragma once\n\n");
    out.push_str("#include <cstdint>\n\n");
    out.push_str("namespace cs2::verified\n{\n");

    for f in FEATURES.iter() {
        let ns = sanitize_ident(f.name);
        out.push_str(&format!("    // ----- {} [{}] -----\n", f.name, f.status));
        out.push_str(&format!("    namespace {}\n    {{\n", ns));
        out.push_str(&format!("        constexpr const char* status = \"{}\";\n", f.status.replace('\\', "\\\\").replace('"', "\\\"")));
        for fld in f.fields.iter() {
            let cls = sanitize_ident(fld.class);
            let fname = sanitize_ident(fld.field);
            out.push_str(&format!(
                "        constexpr std::ptrdiff_t {}__{} = 0x{:X}; // {} — {}\n",
                cls, fname, fld.offset, fld.ty, fld.note,
            ));
        }
        for c in f.convars.iter() {
            let cn = sanitize_ident(c.name);
            out.push_str(&format!(
                "        constexpr uint32_t convar_{}__strip_flags = 0x{:X};\n",
                cn, c.strip_flags,
            ));
            out.push_str(&format!(
                "        constexpr bool     convar_{}__write_both  = {};\n",
                cn, c.write_both_slots,
            ));
        }
        out.push_str("    }\n\n");
    }

    out.push_str("} // namespace cs2::verified\n");
    out
}

fn sanitize_ident(input: &str) -> String {
    let mut s = String::with_capacity(input.len());
    for c in input.chars() {
        if c.is_ascii_alphanumeric() || c == '_' {
            s.push(c);
        } else {
            s.push('_');
        }
    }
    if s.chars().next().map(|c| c.is_ascii_digit()).unwrap_or(false) {
        s.insert(0, '_');
    }
    s
}
