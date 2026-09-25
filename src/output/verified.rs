//! Verified working features — hand-curated catalogue of offsets, hooks
//! and ConVar tricks that have been **confirmed working in a live CS2
//! internal cheat** against the current build.
//!
//! Most of this data is already present in the auto-extracted offset
//! / Pattern / schema files this tool produces. The point of this
//! module is to give consumers a single place that says, in plain
//! English, *"yes, this offset on this entity does this exact thing,
//! and here is the gotcha you need to know to make it work."*
//!
//! a2x's pelite-based pipeline gives us the canonical RVAs and schema
//! offsets — that data is the source of truth and lives in the regular
//! `offsets.*` / `client_dll.*` / `patterns.*` files. This catalogue
//! cross-references those values with verified live-engine notes so a
//! cheat developer can copy-paste a feature and have it work first try.

use serde_json::json;
use crate::analysis::{Class, ClassField, SchemaMap};

#[derive(Clone, Copy)]
pub struct VerifiedField {
    /// Schema class the offset is relative to, e.g. "C_CSPlayerPawn".
    pub class: &'static str,
    /// Schema field name, or a dotted path through embedded structs
    /// ("m_AttributeManager.m_Item.m_iItemDefinitionIndex"). A trailing
    /// " (note)" is ignored for the lookup.
    pub field: &'static str,
    /// Only for fields the schema does not describe (CEntitySystem's
    /// listener vector, CEntityIdentity's entity pointer): the hand-verified
    /// offset. Everything else is resolved from the live schema at dump
    /// time, so it can never go stale.
    pub manual: Option<u32>,
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
    /// Hooks installed (function + module + Pattern key in database.rs)
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
    /// Pattern database key (look up in src/patterns/database.rs)
    pub signature: &'static str,
    /// what we do once hooked
    pub action: &'static str,
}

// ----------------------------------------------------------------------
// Catalogue. Add entries as new features are verified working in-game.
// Build target: CS2 14152+ (April 2026).
// ----------------------------------------------------------------------
pub static FEATURES: &[VerifiedFeature] = &[
    // ------------------------------------------------------------------
    // ==================================================================
    // PREREQUISITES — the plumbing every feature below assumes is in
    // place. These are the wiring tasks reviewed and stabilized in the
    // internal cheat; nothing else in the catalogue works without them.
    // ==================================================================

    VerifiedFeature {
        name: "Entity tracking (OnAddEntity / OnRemoveEntity)",
        status: "working",
        summary: "Hook the engine's entity-system listener instead of \
                  walking dwEntityList every frame. CEntitySystem keeps a \
                  CUtlVector<IEntityListener*> at +0x30; install a small \
                  shim with two virtuals (OnAddEntity, OnRemoveEntity) and \
                  the engine will call it whenever a CEntityIdentity is \
                  bound or torn down. Cache (handle → entity*) in your own \
                  flat array and you skip ~16k pointer-chases per second \
                  versus the loop. \n\nFalls back automatically: on the \
                  first frame after attach, do one full walk to seed the \
                  cache, then run from listener events thereafter.",
        fields: &[
            VerifiedField { class: "CEntitySystem",   field: "m_entityListeners (CUtlVector)", manual: Some(0x30),  ty: "CUtlVector<IEntityListener*>", note: "AddTail your shim here" },
            VerifiedField { class: "CEntityIdentity", field: "m_pEntity",                      manual: Some(0x0),   ty: "C_BaseEntity*",              note: "passed to OnAddEntity / OnRemoveEntity" },
            VerifiedField { class: "CEntityIdentity", field: "m_designerName",                 manual: None,  ty: "const char*",                note: "schema class name; cheap filter" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "IEntityListener::OnAddEntity",    module: "client.dll", signature: "(virtual)", action: "Insert into the cheat's entity cache." },
            VerifiedHook { function: "IEntityListener::OnRemoveEntity", module: "client.dll", signature: "(virtual)", action: "Remove from the cheat's entity cache; cancel any pending visual state." },
        ],
    },

    VerifiedFeature {
        name: "Tracing",
        status: "placeholder",
        summary: "Engine-trace pipeline (TraceInitData → TraceInfo → \
                  TraceFilter → TraceCreate → TraceGetInfo → \
                  TraceHandleBulletPen) is the prerequisite for any \
                  visibility-aware feature: aimbot vis-check, triggerbot \
                  shot prediction, autowall damage estimation. patterns \
                  are resolved in the database but the canonical wrapper \
                  prototypes are placeholders — they were not exhaustively \
                  reverse-engineered and the argument types are best-guess. \
                  Use the patterns to locate the calls, then replicate the \
                  argument shape from your own IDA pass before relying on \
                  the wrapper.",
        fields: &[],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "TraceInitData",          module: "client.dll", signature: "TraceInitData",          action: "Build the per-call data block. Prototype is a placeholder." },
            VerifiedHook { function: "TraceInfo",              module: "client.dll", signature: "TraceInfo",              action: "Populate trace info struct. Prototype is a placeholder." },
            VerifiedHook { function: "TraceFilter",            module: "client.dll", signature: "TraceFilter",            action: "Owner-skip filter. Prototype is a placeholder." },
            VerifiedHook { function: "TraceCreate",            module: "client.dll", signature: "TraceCreate",            action: "Issue the trace; verified working as the entry point." },
            VerifiedHook { function: "TraceGetInfo",           module: "client.dll", signature: "TraceGetInfo",           action: "Read back the result. Prototype is a placeholder." },
            VerifiedHook { function: "TraceHandleBulletPen",   module: "client.dll", signature: "TraceHandleBulletPen",   action: "Bullet-penetration secondary trace. Prototype is a placeholder." },
        ],
    },

    // ==================================================================
    // FEATURES — assume the cache from OnAddEntity is populated and the
    // trace pipeline is wired before reading these.
    // ==================================================================

    VerifiedFeature {
        name: "ESP",
        status: "working",
        summary: "Render data for each cached pawn: state (m_iHealth, \
                  m_lifeState, m_iTeamNum), world position via \
                  m_pGameSceneNode → CGameSceneNode::m_vecAbsOrigin, named \
                  via m_hController → CCSPlayerController::m_iszPlayerName, \
                  weapon via m_hActiveWeapon. Project to screen with \
                  dwViewMatrix (4×4 row-major). \n\nSkeleton: \
                  m_pGameSceneNode is actually CSkeletonInstance; the live \
                  bone array hangs off CSkeletonInstance::m_modelState. Bone 6 = \
                  head, 5 = chest on the standard CSPlayer skeleton. \
                  Money / armor / scoreboard / rank live on the \
                  controller-side service blocks listed below.",
        fields: &[
            VerifiedField { class: "C_BaseEntity",            field: "m_pGameSceneNode",     manual: None,  ty: "CSkeletonInstance*", note: "→ bone matrix + abs origin" },
            VerifiedField { class: "C_BaseEntity",            field: "m_iHealth",            manual: None,  ty: "int32",              note: "0 == dead" },
            VerifiedField { class: "C_BaseEntity",            field: "m_lifeState",          manual: None,  ty: "uint8",              note: "0 == ALIVE" },
            VerifiedField { class: "C_BaseEntity",            field: "m_iTeamNum",           manual: None,  ty: "uint8",              note: "2 = T, 3 = CT" },
            VerifiedField { class: "CGameSceneNode",          field: "m_vecAbsOrigin",       manual: None,   ty: "Vector3",            note: "world position (ESP root)" },
            VerifiedField { class: "CSkeletonInstance",       field: "m_modelState",         manual: None,  ty: "CModelState",        note: "embedded; live bone array inside" },
            VerifiedField { class: "CCSPlayerController",     field: "m_iszPlayerName",      manual: None,  ty: "char[128]",          note: "UTF-8 nickname" },
            VerifiedField { class: "CCSPlayerController",     field: "m_hPawn",              manual: None,  ty: "CHandle",            note: "controller → pawn handle" },
            VerifiedField { class: "CCSPlayerController",     field: "m_iCompetitiveRanking", manual: None, ty: "int32",              note: "Premier rating (revealed pre-warmup)" },
            VerifiedField { class: "C_CSPlayerPawnBase",      field: "m_pWeaponServices",    manual: None, ty: "ptr",                note: "→ active weapon handle" },
            VerifiedField { class: "C_EconEntity",      field: "m_AttributeManager.m_Item.m_iItemDefinitionIndex", manual: None, ty: "uint16",           note: "CSWeaponID for the held weapon" },
            VerifiedField { class: "C_BasePlayerWeapon",      field: "m_iClip1",             manual: None, ty: "int32",              note: "current magazine count" },
            VerifiedField { class: "CCSPlayerController_InGameMoneyServices", field: "m_iAccount", manual: None,  ty: "int32", note: "current cash" },
            VerifiedField { class: "C_CSPlayerPawn",          field: "m_ArmorValue",         manual: None, ty: "int32",              note: "armor 0..100" },
            VerifiedField { class: "CCSPlayer_ItemServices",  field: "m_bHasHelmet",         manual: None,   ty: "bool",               note: "kevlar+helmet flag" },
            VerifiedField { class: "CCSPlayerController_ActionTrackingServices", field: "m_matchStats.m_iKills",   manual: None, ty: "int32", note: "scoreboard kills" },
            VerifiedField { class: "CCSPlayerController_ActionTrackingServices", field: "m_matchStats.m_iDeaths",  manual: None, ty: "int32", note: "scoreboard deaths" },
            VerifiedField { class: "EntitySpottedState_t",    field: "m_bSpotted",           manual: None,    ty: "bool",               note: "force true to reveal on radar" },
            VerifiedField { class: "EntitySpottedState_t",    field: "m_bSpottedByMask",     manual: None,    ty: "uint32[2]",          note: "OR with 0xFFFFFFFF to spot for everyone" },
        ],
        convars: &[],
        hooks: &[],
    },

    VerifiedFeature {
        name: "FOV Changer",
        status: "working",
        summary: "Two-prong approach. (1) Hook the world-FOV resolver in \
                  client.dll (signature GetWorldFovResolver) and return the \
                  desired value when the local pawn is not scoped — keeps \
                  the value sticky against engine resets. (2) Every tick \
                  write the desired FOV into m_iFOV and m_iFOVStart on the \
                  camera services AND into m_iDesiredFOV on the local \
                  controller. The controller-level field is the canonical \
                  source the renderer reads; without it the camera-services \
                  side gets clobbered back to default.",
        fields: &[
            VerifiedField { class: "CBasePlayerController",     field: "m_iDesiredFOV",      manual: None,  ty: "uint32",                       note: "a2x-named m_iDesiredFOV_OnController — canonical write" },
            VerifiedField { class: "CCSPlayerBase_CameraServices",  field: "m_iFOV",             manual: None,  ty: "uint32",                       note: "current camera FOV" },
            VerifiedField { class: "CCSPlayerBase_CameraServices",  field: "m_iFOVStart",        manual: None,  ty: "uint32",                       note: "target camera FOV" },
            VerifiedField { class: "C_BasePlayerPawn",            field: "m_pCameraServices", manual: None, ty: "CCSPlayer_CameraServices*", note: "deref to reach m_iFOV / m_iFOVStart" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "GetWorldFov", module: "client.dll", signature: "GetWorldFovResolver", action: "Return cfg.fovValue when not scoped, else delegate to original." },
        ],
    },

    VerifiedFeature {
        name: "Aimbot",
        status: "working",
        summary: "Per-tick phase machine driven from CCSGOInput::CreateMove \
                  (IDLE → REACTING → ATTACKING → CORRECTING → LOCKED). \
                  Target selection scores enemies on FOV delta, distance, \
                  visibility (engine trace) and weighting flags. Final angle \
                  delivery happens via a hooked CSGOInputHistoryEntry::\
                  WriteSubtick (Pattern `48 89 5C 24 ? 55 57 41 56 48 8D \
                  6C 24 ? 48 81 EC B0 00 00 00 8B 01 48 8B F9 81 4A 10 00 \
                  02`, unique match @ 0x180C53DB0 in build 14152) so the \
                  override only touches per-subtick shoot angles \
                  (fe[7..9]) on attack subticks (a3 != 0). The real \
                  view-angle stream replayed for spectators / GOTV / \
                  Overwatch (fe[4..6]) is NEVER touched. \n\nHard-suppress \
                  gates (any true ⇒ skip the angle write entirely): freeze \
                  / warmup, m_bWaitForNoAttack, no-scope sniper, \
                  m_bNeedsBoltAction, m_bInReload, m_iClip1 == 0, \
                  m_nNextPrimaryAttackTick > tickBase + 1, m_bIsDefusing, \
                  m_bIsGrabbingHostage, MoveType not WALK / FLYGRAVITY. \
                  Soft throttle scales the 28°/subtick flick cap: \
                  m_bIsValveDS × 0.55, observerCount × 0.55, m_bSpotted + \
                  observers × 0.65, horizontal speed > 80 u/s linearly \
                  ramps to 0.5 (caps at 180 u/s), ±0.10° LCG jitter. \
                  Crosshair-aligned bypass: when local m_iIDEntIndex \
                  resolves to the silent-aim target, the throttle is \
                  bypassed.",
        fields: &[
            VerifiedField { class: "C_CSGameRules",         field: "m_bFreezePeriod",          manual: None,   ty: "bool",            note: "freeze — no attacks possible" },
            VerifiedField { class: "C_CSGameRules",         field: "m_bWarmupPeriod",          manual: None,   ty: "bool",            note: "warmup — no attacks possible" },
            VerifiedField { class: "C_CSGameRules",         field: "m_bIsValveDS",             manual: None,   ty: "bool",            note: "TRUE on Valve official MM — soft 0.55×" },
            VerifiedField { class: "C_CSGameRules",         field: "m_bHasMatchStarted",       manual: None,   ty: "bool",            note: "match-state gate" },
            VerifiedField { class: "C_CSPlayerPawn",        field: "m_bWaitForNoAttack",       manual: None, ty: "bool",            note: "post-respawn / weapon-switch lockout" },
            VerifiedField { class: "C_CSPlayerPawn",        field: "m_bIsDefusing",            manual: None, ty: "bool",            note: "server forbids attack while defusing" },
            VerifiedField { class: "C_CSPlayerPawn",        field: "m_bIsGrabbingHostage",     manual: None, ty: "bool",            note: "server forbids attack while grabbing hostage" },
            VerifiedField { class: "C_BaseEntity",          field: "m_MoveType",               manual: None,  ty: "MoveType_t",      note: "only WALK(2) / FLYGRAVITY(4) are normal play" },
            VerifiedField { class: "C_CSWeaponBaseGun",     field: "m_zoomLevel",              manual: None, ty: "int32",           note: "0 = unscoped — refuse silent fire on snipers when zoom == 0" },
            VerifiedField { class: "C_CSWeaponBaseGun",     field: "m_bNeedsBoltAction",       manual: None, ty: "bool",            note: "AWP/SSG/Scout bolt-cycle lockout" },
            VerifiedField { class: "C_CSWeaponBase",        field: "m_bInReload",              manual: None, ty: "bool",            note: "weapon mid-reload" },
            VerifiedField { class: "C_BasePlayerWeapon",    field: "m_iClip1",                 manual: None, ty: "int32",           note: "0 ⇒ no bullet possible" },
            VerifiedField { class: "C_BasePlayerWeapon",    field: "m_nNextPrimaryAttackTick", manual: None, ty: "int32",           note: "absolute server tick when next attack allowed" },
            VerifiedField { class: "CBasePlayerController", field: "m_nTickBase",              manual: None,  ty: "int32",           note: "compare against m_nNextPrimaryAttackTick" },
            VerifiedField { class: "EntitySpottedState_t",  field: "m_bSpottedByMask",         manual: None,    ty: "uint32[2]",       note: "real enemy in PVS ⇒ throttle 0.65×" },
            VerifiedField { class: "C_CSPlayerPawn",        field: "m_iIDEntIndex",            manual: None, ty: "int32",           note: "matches target ⇒ bypass throttle" },
            VerifiedField { class: "C_BaseEntity",          field: "m_vecVelocity",            manual: None,  ty: "Vector3",         note: "soft throttle 1.0 → 0.5 from 80 → 180 u/s" },
            VerifiedField { class: "C_CSPlayerPawn",        field: "m_pAimPunchServices",      manual: None, ty: "CCSPlayer_AimPunchServices*", note: "owns aim-punch cache vector" },
            VerifiedField { class: "C_CSPlayerPawn",        field: "m_iShotsFired",            manual: None, ty: "int32",           note: "drives spread seed" },
            VerifiedField { class: "C_CSWeaponBase",        field: "m_flRecoilIndex",          manual: None, ty: "float",           note: "recoil pattern index" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "CCSGOInput::CreateMove",                 module: "client.dll", signature: "CreateMove", action: "Phase machine + target select + angle snap; sets fire latch." },
            VerifiedHook { function: "CSGOInputHistoryEntry::WriteSubtick",   module: "client.dll", signature: "48 89 5C 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC B0 00 00 00 8B 01 48 8B F9 81 4A 10 00 02", action: "Per-subtick angle override — attack subticks only; fe[7..9] only." },
        ],
    },

    VerifiedFeature {
        name: "Triggerbot (Seeded)",
        status: "working",
        summary: "Per-tick seeded prediction. Reads the live spread seed \
                  (m_iShotsFired + aim-punch) and re-runs Valve's spread \
                  RNG via SpreadSeedGen + CalcSpread to compute exactly \
                  where the next bullet would fly, then traces from local \
                  eye to that point. Strict-window mode tests only ticks \
                  {0, +1} and ALL must hit before firing; wide-window mode \
                  accepts ANY hit in {0, +1, -1, +2}. Local eye position is \
                  projected by localVel × leadTime so test geometry matches \
                  engine fire-time eye pos. \n\nUses the REAL \
                  m_fAccuracyPenalty + m_flTurningInaccuracy in the \
                  predictor — the earlier 'perfect-shot' override that \
                  zeroed client spread caused server desync (kill sound \
                  but no damage); that path is OFF by default. \n\nCooperates \
                  with aimbot: trigger defers whenever Aimbot::state.phase \
                  ∈ {REACTING, ATTACKING, CORRECTING}. SendInput LBUTTON \
                  edges from trigger are masked out of aimbot's rawAim \
                  filter via an 80 ms synth-click window so the synthetic \
                  click can't wake aimbot uninvited.",
        fields: &[
            VerifiedField { class: "C_CSPlayerPawn", field: "m_iIDEntIndex",       manual: None, ty: "int32",                       note: "primary target signal" },
            VerifiedField { class: "C_CSPlayerPawn", field: "m_iShotsFired",       manual: None, ty: "int32",                       note: "drives spread seed" },
            VerifiedField { class: "C_CSPlayerPawn", field: "m_pAimPunchServices", manual: None, ty: "CCSPlayer_AimPunchServices*", note: "deref for live aim-punch vec used in the seed" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "CCSPlayerAnimGraphState::CalcSpread", module: "client.dll", signature: "CalcSpread",  action: "Cache (mode, baseSpread, inaccuracy) per itemDef." },
            VerifiedHook { function: "NoSpread1",                            module: "client.dll", signature: "NoSpread1",   action: "Optional perfect-shot path — DISABLED by default." },
        ],
    },

    VerifiedFeature {
        name: "Skin Changer",
        status: "working",
        summary: "Writes m_nFallbackPaintKit / m_nFallbackSeed / \
                  m_flFallbackWear / m_iEntityQuality on each weapon then \
                  forces the modern paint-apply path: \
                  ApplyEconCustomization(weapon, 1) → sub_181079790 → \
                  sub_18105AAF0 (which actually consumes the fallback \
                  fields and queues 'clientside_reload_custom_econ' to \
                  rebuild the composite material). RegenerateWeaponSkin \
                  alone is INSUFFICIENT — it only handles the legacy static \
                  paint table. GetCustomPaintKitIndex is polled to detect \
                  rejection and gate re-apply work instead of hammering \
                  ApplyEconCustomization every tick. Setting m_iItemIDLow / \
                  High to 0xFFFFFFFF forces the EconItemView lookup to fail \
                  → fallback path taken.",
        fields: &[
            VerifiedField { class: "C_EconItemView", field: "m_iItemDefinitionIndex", manual: None, ty: "uint16", note: "weapon definition (CSWeaponID)" },
            VerifiedField { class: "C_EconItemView", field: "m_iItemIDLow",           manual: None, ty: "uint32", note: "set 0xFFFFFFFF to force EconItemView lookup miss → fallback path" },
            VerifiedField { class: "C_EconItemView", field: "m_iItemIDHigh",          manual: None, ty: "uint32", note: "set 0xFFFFFFFF (paired with m_iItemIDLow)" },
            VerifiedField { class: "C_EconItemView", field: "m_iEntityQuality",       manual: None, ty: "int32",  note: "quality slot used by the composite shader" },
            VerifiedField { class: "C_EconEntity", field: "m_nFallbackPaintKit",    manual: None, ty: "uint32", note: "paint kit ID (the actual 'skin')" },
            VerifiedField { class: "C_EconEntity", field: "m_nFallbackSeed",        manual: None, ty: "int32",  note: "pattern seed" },
            VerifiedField { class: "C_EconEntity", field: "m_flFallbackWear",       manual: None, ty: "float",  note: "0.0 = factory new, 1.0 = battle-scarred" },
            VerifiedField { class: "C_EconEntity", field: "m_nFallbackStatTrak",    manual: None, ty: "int32",  note: "StatTrak counter (-1 disables)" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "ApplyEconCustomization", module: "client.dll", signature: "ApplyEconCustomization",                action: "Modern paint-apply entry; consumes m_nFallback* and queues composite rebuild." },
            VerifiedHook { function: "RegenerateWeaponSkin",   module: "client.dll", signature: "RegenerateWeaponSkin",                  action: "Legacy static-paint pass; called for completeness." },
            VerifiedHook { function: "GetCustomPaintKitIndex", module: "client.dll", signature: "CEconItemView::GetCustomPaintKitIndex", action: "Read live paint kit to detect rejection and gate re-apply." },
        ],
    },

    VerifiedFeature {
        name: "Knife Changer",
        status: "working",
        summary: "Spoofs m_nSubclassID on the knife entity, calls \
                  UpdateSubclass to re-bind the subclass-data pointer \
                  (weapon+0x388, right after m_nSubclassID; the per-knife \
                  VData and sequence set), then \
                  AnimGraphRebuild(controller, 2) to tear down the existing \
                  CNmGraphInstance (m_pGraphInstanceAG2) and let the manager \
                  re-bind from the (now-updated) vdata's animgraph. \
                  Without the rebuild the knife mesh swaps but inspect / \
                  deploy / swing animations stay on the OLD subclass's \
                  sequences (Emerald Butterfly mesh + default-knife \
                  inspect anim was the symptom). SetMeshGroupMask \
                  refreshes the visible mesh after the subclass change.",
        fields: &[
            VerifiedField { class: "C_BasePlayerWeapon", field: "m_nSubclassID",          manual: None, ty: "uint32", note: "knife subclass key (drives mesh + sequences + animgraph)" },
            VerifiedField { class: "C_EconEntity", field: "m_AttributeManager.m_Item.m_iItemDefinitionIndex", manual: None, ty: "uint16", note: "must match the knife type for the chosen subclass" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "UpdateSubclass",   module: "client.dll", signature: "C_BaseEntity_UpdateSubclass", action: "Re-bind the subclass-data pointer at weapon+0x388 (IDA-verified: writes a1[113] after looking up m_nSubclassID)." },
            VerifiedHook { function: "AnimGraphRebuild", module: "client.dll", signature: "AnimGraphRebuild",              action: "Mode = 2: destroy CNmGraphInstance and re-bind." },
            VerifiedHook { function: "SetMeshGroupMask", module: "client.dll", signature: "SetMeshGroupMask",              action: "Refresh visible mesh after subclass change." },
        ],
    },

    VerifiedFeature {
        name: "Glove Changer",
        status: "placeholder",
        summary: "WIP. Schema writes (m_nFallbackPaintKit on \
                  C_EconWearable, m_unMusicID etc.) propagate but the \
                  composite material fails to rebuild on the wearable's \
                  render slot in the current build. Suspected missing \
                  call: BuildLegacyGloveSkinMaterial + \
                  CompositeMaterialPanoramaPanel_Init + the per-panel \
                  render-request path. Sigs resolve cleanly — wiring is \
                  the blocker. Listed for visibility; do not rely on it \
                  yet.",
        fields: &[],
        convars: &[],
        hooks: &[],
    },

    // ------------------------------------------------------------------
    VerifiedFeature {
        name: "Engine Prediction Simulation",
        status: "partial",
        summary: "Per-tick re-execution of Valve's movement pipeline on the \
                  local pawn to obtain post-prediction flags (FL_ONGROUND, \
                  FL_DUCKING, etc.) for features that have to branch on the \
                  next-tick server view — bhop, autostrafe, predicted eye \
                  pos for ragebot. The engine itself ALREADY runs prediction \
                  via engine2!RunPrediction (sig `RunPrediction`); the \
                  cheap path is to hook that and snapshot m_fFlags before / \
                  after the original call. The simulation-from-scratch path \
                  (RunCommand + ProcessMovement + CalculateJumpHeight + \
                  FinishMove + ProcessImpacts) reproduces it for arbitrary \
                  CUserCmd inputs and is required for ragebot extrapolation. \
                  Schema offsets confirmed from C_BaseEntity::ScriptDesc in \
                  IDA: m_fFlags @ 0x3F8, m_vecAbsVelocity @ 0x3FC, \
                  m_vecVelocity @ 0x430, m_MoveType @ 0x525.",
        fields: &[
            VerifiedField { class: "C_BaseEntity",                field: "m_fFlags",            manual: None,  ty: "uint32",       note: "FL_ONGROUND=1, FL_DUCKING=2, FL_WATERJUMP=8 — post-prediction view of next-tick state" },
            VerifiedField { class: "C_BaseEntity",                field: "m_vecAbsVelocity",    manual: None,  ty: "Vector3",      note: "absolute world velocity, post-prediction" },
            VerifiedField { class: "C_BaseEntity",                field: "m_vecVelocity",       manual: None,  ty: "Vector3",      note: "local velocity, net-replicated" },
            VerifiedField { class: "C_BaseEntity",                field: "m_MoveType",          manual: None,  ty: "MoveType_t",   note: "2=WALK 4=FLYGRAVITY — only these allow normal prediction" },
            VerifiedField { class: "C_BasePlayerPawn",            field: "m_pMovementServices", manual: None, ty: "CPlayer_MovementServices*", note: "→ ProcessMovement / SetupMove / FinishMove receiver" },
            VerifiedField { class: "CCSPlayerController",         field: "m_hPawn",             manual: None,  ty: "CHandle",      note: "controller→pawn handle for local-player resolution" },
            VerifiedField { class: "CCSPlayerController",         field: "m_nTickBase",         manual: None,  ty: "uint32",      note: "absolute server tick — clock the simulation against this" },
            VerifiedField { class: "CCSPlayerController",         field: "m_bPawnIsAlive",      manual: None,  ty: "bool",         note: "guard: do not run prediction on dead pawn" },
        ],
        convars: &[],
        hooks: &[
            VerifiedHook { function: "CNetworkGameClient::RunPrediction", module: "engine2.dll", signature: "RunPrediction",      action: "Bracket original: capture pawn->m_fFlags + velocity before, then after. Diff exposes what engine predicted this tick." },
            VerifiedHook { function: "CCSGOInput::CreateMovePrePrediction", module: "client.dll", signature: "create_move_v2",   action: "Per-tick driver — refresh local pawn/controller ptrs, optionally drive manual RunSimulation here for ragebot extrapolation." },
        ],
    },
];

// ----------------------------------------------------------------------
// Renderers
// ----------------------------------------------------------------------

fn find_class<'a>(map: &'a SchemaMap, name: &str) -> Option<&'a Class> {
    // Client first: every feature here is client-side and client/server
    // share class names with different layouts.
    if let Some((cs, _)) = map.get("client.dll")
        && let Some(c) = cs.iter().find(|c| c.name == name)
    {
        return Some(c);
    }
    map.iter()
        .filter(|(m, _)| m.as_str() != "client.dll")
        .find_map(|(_, (cs, _))| cs.iter().find(|c| c.name == name))
}

/// Base classes the schema binding does not record. Checked at use: the
/// derived class's first own field must start at or after the base's size.
const IMPLICIT_BASES: &[(&str, &str)] = &[("CSMatchStats_t", "CSPerRoundStats_t")];

fn parent_of(map: &SchemaMap, c: &Class) -> Option<String> {
    if let Some(p) = &c.parent_name {
        return Some(p.clone());
    }
    let (_, base) = IMPLICIT_BASES.iter().find(|(d, _)| *d == c.name)?;
    let b = find_class(map, base)?;
    let first = c.fields.iter().map(|f| f.offset).min().unwrap_or(0);
    (first >= b.size as i32).then(|| base.to_string())
}

/// Field lookup that climbs the parent chain, like the game's own accessors.
fn find_field<'a>(map: &'a SchemaMap, class: &str, field: &str) -> Option<(&'a ClassField, String)> {
    let mut cur = class.to_string();
    for _ in 0..32 {
        let c = find_class(map, &cur)?;
        if let Some(f) = c.fields.iter().find(|f| f.name == field) {
            return Some((f, cur));
        }
        cur = parent_of(map, c)?;
    }
    None
}

/// Resolve `class` + dotted `path` to an offset relative to `class`.
/// Returns the offset and the class that declares the last segment.
fn resolve(map: Option<&SchemaMap>, class: &str, path: &str) -> Option<(u64, String)> {
    let map = map?;
    let mut cls = class.to_string();
    let mut off = 0u64;
    let mut declared = String::new();
    let segs: Vec<&str> = path.split('.').collect();
    for (i, seg) in segs.iter().enumerate() {
        let (f, d) = find_field(map, &cls, seg.trim())?;
        off += f.offset.max(0) as u64;
        declared = d;
        let ty = f.type_name.trim();
        // A path may only continue through an embedded struct; offsets past a
        // pointer are not relative to `class` any more.
        if i + 1 < segs.len() && (ty.ends_with('*') || ty.starts_with("CHandle")) {
            return None;
        }
        cls = ty.to_string();
    }
    Some((off, declared))
}

/// Signature-database names referenced by the hooks of published features
/// (raw byte patterns and "(virtual)" excluded), for the self-checks.
pub fn hook_signatures() -> Vec<(&'static str, &'static str, &'static str)> {
    FEATURES
        .iter()
        .filter(|f| f.status == "working")
        .flat_map(|f| f.hooks.iter().map(move |h| (f.name, h.module, h.signature)))
        .filter(|(_, _, s)| *s != "(virtual)" && !s.chars().all(|c| c.is_ascii_hexdigit() || c == ' ' || c == '?'))
        .collect()
}

/// Number of fields that could not be resolved in the last render, for
/// the self-checks.
pub static UNRESOLVED: std::sync::Mutex<Vec<String>> = std::sync::Mutex::new(Vec::new());

pub fn render_json(build_number: Option<u32>, schemas: Option<&SchemaMap>) -> String {
    let working: Vec<&VerifiedFeature> = FEATURES.iter().filter(|f| f.status == "working").collect();
    let mut unresolved: Vec<String> = Vec::new();
    let features: Vec<_> = working
        .iter()
        .map(|f| {
            json!({
                "name":    f.name,
                "summary": f.summary,
                "fields":  f.fields.iter().map(|fld| {
                    let path = fld.field.split(' ').next().unwrap_or(fld.field);
                    let (offset, source, declared_in) = match (fld.manual, resolve(schemas, fld.class, path)) {
                        (_, Some((off, d))) => (format!("0x{:X}", off), "schema", if d != fld.class { Some(d) } else { None }),
                        (Some(m), None) => (format!("0x{:X}", m), "manual", None),
                        (None, None) => {
                            unresolved.push(format!("{}::{}", fld.class, path));
                            (String::new(), "unresolved", None)
                        }
                    };
                    json!({
                        "class":  fld.class,
                        "field":  fld.field,
                        "offset": offset,
                        "source": source,
                        "declared_in": declared_in,
                        "type":   fld.ty,
                        "note":   fld.note,
                    })
                }).collect::<Vec<_>>(),
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

    if let Ok(mut u) = UNRESOLVED.lock() {
        *u = unresolved.clone();
    }
    let doc = json!({
        "cs2_build":      build_number,
        "feature_count":  working.len(),
        "note":           "Field offsets are resolved from the schema dumped in the same run (source: schema); only non-schema fields carry hand-verified values (source: manual). An empty offset means the field is not in this build's schema.",
        "unresolved":     unresolved,
        "features":       features,
    });

    serde_json::to_string_pretty(&doc).unwrap_or_else(|_| String::from("{}"))
}


