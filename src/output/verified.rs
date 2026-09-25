//! Verified feature recipes — how the features on cs2-sdk.com's Features tab
//! are built, in two flavours: **internal** (code running inside cs2.exe:
//! hooks, direct reads, calling game functions) and **external** (a separate
//! process reading and writing memory, no hooks, no calls).
//!
//! Every recipe was checked against a working implementation and against the
//! current build (IDA on the binaries, plus read-only checks of the live
//! process). Nothing numeric is typed in here: field offsets are resolved from
//! the schema dumped in the same run, engine-struct offsets from
//! `engine_structs.rs`, globals from the offset pass and function addresses
//! from the signature pass. The few values that exist nowhere else are marked
//! `manual` with the build they were verified on, and the self-checks fail the
//! run if any reference stops resolving.
//!
//! Output: `verified_features.json`.

use serde_json::{json, Value};

use crate::analysis::{Class, ClassField, OffsetMap, SchemaMap};
use crate::output::engine_structs::ENGINE_STRUCTS;
use crate::patterns::{display_name, PatternHit};

/// A memory offset the recipe reads or writes.
pub struct Field {
    /// Schema class, or an engine struct from engine_structs.rs.
    pub class: &'static str,
    /// Field name, a dotted path through embedded structs
    /// (`m_AttributeManager.m_Item.m_iItemDefinitionIndex`), `sizeof`, or a
    /// field plus a fixed delta (`m_modelState+0x80`).
    pub field: &'static str,
    /// Offsets that exist in no schema or engine struct: (value, build it was
    /// verified on).
    pub manual: Option<(u32, u32)>,
    pub ty: &'static str,
    pub note: &'static str,
}

/// A game function the recipe hooks or calls, by signature-database name.
pub struct Func {
    pub name: &'static str,
    pub module: &'static str,
    /// "hook" or "call".
    pub role: &'static str,
    pub purpose: &'static str,
}

/// A module-relative global from offsets.json (`dwXxx`).
pub struct Global {
    pub module: &'static str,
    pub name: &'static str,
    pub note: &'static str,
}

pub struct Variant {
    pub summary: &'static str,
    pub steps: &'static [&'static str],
    pub fields: &'static [Field],
    pub globals: &'static [Global],
    pub funcs: &'static [Func],
    /// (ConVar, how it is used)
    pub convars: &'static [(&'static str, &'static str)],
    pub notes: &'static [&'static str],
}

pub struct Feature {
    pub name: &'static str,
    pub category: &'static str,
    pub summary: &'static str,
    pub internal: Variant,
    pub external: Option<Variant>,
    /// Why there is no external version, when there is none.
    pub external_unavailable: &'static str,
}

const fn f(class: &'static str, field: &'static str, ty: &'static str, note: &'static str) -> Field {
    Field { class, field, manual: None, ty, note }
}
const fn m(class: &'static str, field: &'static str, value: u32, build: u32, ty: &'static str, note: &'static str) -> Field {
    Field { class, field, manual: Some((value, build)), ty, note }
}
const fn hook(name: &'static str, purpose: &'static str) -> Func {
    Func { name, module: "client.dll", role: "hook", purpose }
}
const fn call(name: &'static str, purpose: &'static str) -> Func {
    Func { name, module: "client.dll", role: "call", purpose }
}
const fn g(name: &'static str, note: &'static str) -> Global {
    Global { module: "client.dll", name, note }
}

// ---------------------------------------------------------------------------
// Shared pieces
// ---------------------------------------------------------------------------

const ENTITY_LIST_FIELDS: &[Field] = &[
    m("CGameEntitySystem", "chunk array", 0x10, 14184, "CEntityIdentity*[64]", "entity_system + 0x10 + 8 * (index >> 9) -> chunk of 512 identities"),
    f("CEntityIdentity", "sizeof", "", "identity stride inside a chunk: chunk + sizeof * (index & 0x1FF)"),
    m("CEntityIdentity", "m_pEntity", 0x0, 14184, "C_BaseEntity*", "the entity; 0 = empty slot"),
    m("CEntityIdentity", "handle", 0x10, 14184, "uint32", "the slot's current handle; compare with the handle you resolved to reject a reused slot"),
    f("CEntityIdentity", "m_designerName", "char*", "classname: \"cs_player_controller\", \"weapon_ak47\", ..."),
];

const BONE_NOTE: &str = "bone array = read(scene_node + offset); bone i is 32 bytes: Vector position, float scale, Quaternion rotation";

// ---------------------------------------------------------------------------
// Catalogue
// ---------------------------------------------------------------------------

pub static FEATURES: &[Feature] = &[
    Feature {
        name: "Entity list",
        category: "Core",
        summary: "Find every player, weapon and projectile. Everything else on this page starts from here.",
        internal: Variant {
            summary: "Let the game tell you when entities come and go: hook the entity system's own add/remove notifications and keep a small cache, instead of rescanning the list every frame.",
            steps: &[
                "Hook CGameEntitySystem::OnAddEntity (vtable slot 15) and OnRemoveEntity (slot 16). Both are called with (entity_system, entity, handle); the entity index is handle & 0x7FFF.",
                "Always call the original, and never let an exception leave your handler: the engine calls these from inside its spawn and destroy loops, and unwinding out of them leaves the entity system half-updated.",
                "On add: read the classname (entity -> CEntityInstance::m_pEntity -> CEntityIdentity::m_designerName) and keep {index, pointer} for the types you use: cs_player_controller, the player pawns, weapons, projectiles.",
                "On remove: drop the entry with that index. Remove before the original runs so nothing of yours outlives the entity.",
                "When you attach (and after a map change) seed the cache once by walking the list the external way; the hooks only report changes.",
            ],
            fields: &[
                f("CEntityInstance", "m_pEntity", "CEntityIdentity*", "entity -> its identity"),
                f("CEntityIdentity", "m_designerName", "char*", "classname used to classify"),
            ],
            globals: &[],
            funcs: &[
                hook("OnAddEntity", "CGameEntitySystem vtable slot 15: an entity became valid"),
                hook("OnRemoveEntity", "CGameEntitySystem vtable slot 16: an entity is being destroyed"),
            ],
            convars: &[],
            notes: &["Player controllers are not guaranteed to sit at indices 1-10: walk or cache all of 1-64."],
        },
        external: Some(Variant {
            summary: "Walk the entity list directly. It is a chunked array of CEntityIdentity, 512 per chunk.",
            steps: &[
                "entity_system = read<u64>(client + dwEntityList).",
                "highest = read<i32>(entity_system + dwGameEntitySystem_highestEntityIndex) bounds the walk.",
                "For index i: chunk = read<u64>(entity_system + 0x10 + 8 * (i >> 9)); identity = chunk + sizeof(CEntityIdentity) * (i & 0x1FF); entity = read<u64>(identity). 0 = empty.",
                "Players are CCSPlayerController entities in 1-64 (classname at identity + m_designerName). Their pawn comes from m_hPlayerPawn: resolve the handle with index = handle & 0x7FFF, and accept the entity only if read<u32>(identity + 0x10) == handle — otherwise the slot was reused.",
                "Cache the chunk pointers and re-read them only when the entity_system pointer changes (map change).",
            ],
            fields: ENTITY_LIST_FIELDS,
            globals: &[
                g("dwEntityList", "CGameEntitySystem*"),
                g("dwGameEntitySystem_highestEntityIndex", "member offset inside CGameEntitySystem"),
            ],
            funcs: &[],
            convars: &[],
            notes: &["Verified live on build 14184: all 10 players of a 5v5 resolve controller -> pawn with the handle check."],
        }),
        external_unavailable: "",
    },
    Feature {
        name: "ESP",
        category: "Visuals",
        summary: "Boxes, health, names and skeletons for every player you can see on screen.",
        internal: Variant {
            summary: "Gather player data once per frame from the game thread, draw it from a Present hook.",
            steps: &[
                "From the entity cache take each cs_player_controller with m_bPawnIsAlive set and resolve its pawn from m_hPlayerPawn. Skip your own pawn (dwLocalPlayerPawn).",
                "Skip dormant pawns (m_pGameSceneNode -> m_bDormant): their data stops updating outside your PVS.",
                "Read health (m_iHealth), team (m_iTeamNum: 2 T, 3 CT) and the feet position (scene node m_vecAbsOrigin).",
                "Bones: resolve names once per model with C_BaseEntity_GetBoneIdByName(pawn, \"head_0\") (\"pelvis\", \"neck_0\", \"hand_L\", ...), then read positions from the bone array at scene node + m_modelState + 0x80.",
                "Project with the view matrix (dwViewMatrix, 4x4 row-major): w = m[3]·p; skip w < 0.001; screen x = W/2 * (1 + m[0]·p / w), y = H/2 * (1 - m[1]·p / w).",
                "Box from the projected head (plus headroom) and feet, or fit it to the projected bones; name from m_iszPlayerName; weapon from m_pWeaponServices -> m_hActiveWeapon.",
                "Draw from IDXGISwapChain::Present (vtable slot 8). The swap chain is CSwapChainDx11::m_pSwapChain, filled in by CreateSwapChain.",
            ],
            fields: &[
                f("CCSPlayerController", "m_bPawnIsAlive", "bool", ""),
                f("CCSPlayerController", "m_hPlayerPawn", "CHandle<C_CSPlayerPawn>", "always the player pawn, also while dead (m_hPawn can be the observer)"),
                f("CBasePlayerController", "m_iszPlayerName", "char[128]", "inline UTF-8 name"),
                f("C_BaseEntity", "m_iHealth", "int32", ""),
                f("C_BaseEntity", "m_iTeamNum", "uint8", "2 = T, 3 = CT"),
                f("C_BaseEntity", "m_pGameSceneNode", "CGameSceneNode*", "a CSkeletonInstance on pawns"),
                f("CGameSceneNode", "m_vecAbsOrigin", "Vector", "feet"),
                f("CGameSceneNode", "m_bDormant", "bool", "skip when set"),
                f("CSkeletonInstance", "m_modelState+0x80", "CTransform*", BONE_NOTE),
                f("C_BasePlayerPawn", "m_pWeaponServices", "CPlayer_WeaponServices*", ""),
                f("CPlayer_WeaponServices", "m_hActiveWeapon", "CHandle<C_BasePlayerWeapon>", ""),
                f("C_EconEntity", "m_AttributeManager.m_Item.m_iItemDefinitionIndex", "uint16", "weapon id of the active weapon"),
                f("CSwapChainDx11", "m_pSwapChain", "IDXGISwapChain*", "Present = vtable slot 8, ResizeBuffers = 13"),
            ],
            globals: &[g("dwViewMatrix", "float[4][4], row-major"), g("dwLocalPlayerPawn", "C_CSPlayerPawn*")],
            funcs: &[
                hook("FrameStageNotify", "gather player data on the game thread"),
                call("C_BaseEntity_GetBoneIdByName", "bone name -> index, once per model"),
                Func { name: "CreateSwapChain", module: "rendersystemdx11.dll", role: "hook", purpose: "catch the CSwapChainDx11 instance to reach IDXGISwapChain::Present" },
            ],
            convars: &[],
            notes: &[
                "Stock agent skeleton (read from the live model on 14184): 1 pelvis, 2-5 spine_0-3, 6 neck_0, 7 head_0, 8-11 left arm (clavicle, upper, lower, hand), 12-15 right arm, 17-19 left leg (upper, lower, ankle), 20-22 right leg. Resolve by name rather than trusting these.",
            ],
        },
        external: Some(Variant {
            summary: "The same data through ReadProcessMemory, drawn on your own overlay window.",
            steps: &[
                "Once per frame: read the view matrix (client + dwViewMatrix, 64 bytes) and walk controllers 1-64 as in Entity list.",
                "For each controller with m_bPawnIsAlive: resolve m_hPlayerPawn (with the handle check), skip your own pawn (client + dwLocalPlayerPawn) and dormant pawns.",
                "Read m_iHealth, m_iTeamNum, the scene node's m_vecAbsOrigin (feet) and m_iszPlayerName.",
                "Bones: bone_array = read<u64>(scene_node + m_modelState + 0x80); bone i position = read<Vector>(bone_array + 32 * i). Head is head_0 = 7 on the stock skeleton — or read the model's own names: model = read<u64>(read<u64>(scene_node + m_modelState + m_hModel)); names = read<char**>(model + 0x168), count = read<i32>(model + 0x160).",
                "Project exactly as internal: w = m[3]·p; skip w < 0.001; x = W/2 * (1 + m[0]·p / w), y = H/2 * (1 - m[1]·p / w).",
                "Batch reads: read each pawn's small field ranges in one call, and only walk the bones you draw.",
            ],
            fields: &[
                f("CCSPlayerController", "m_bPawnIsAlive", "bool", ""),
                f("CCSPlayerController", "m_hPlayerPawn", "CHandle<C_CSPlayerPawn>", "resolve through the entity list"),
                f("CBasePlayerController", "m_iszPlayerName", "char[128]", ""),
                f("C_BaseEntity", "m_iHealth", "int32", ""),
                f("C_BaseEntity", "m_iTeamNum", "uint8", "2 = T, 3 = CT"),
                f("C_BaseEntity", "m_pGameSceneNode", "CGameSceneNode*", ""),
                f("CGameSceneNode", "m_vecAbsOrigin", "Vector", "feet"),
                f("CGameSceneNode", "m_bDormant", "bool", "skip when set"),
                f("CSkeletonInstance", "m_modelState+0x80", "CTransform*", BONE_NOTE),
                f("CModelState", "m_hModel", "CModel**", "-> CModel, for bone names"),
                m("CModel", "bone count", 0x160, 14184, "int32", "number of bones"),
                m("CModel", "bone names", 0x168, 14184, "char**", "names[i] is bone i (\"head_0\", \"pelvis\", ...)"),
            ],
            globals: &[
                g("dwEntityList", "CGameEntitySystem*"),
                g("dwLocalPlayerPawn", "C_CSPlayerPawn*"),
                g("dwViewMatrix", "float[4][4], row-major"),
            ],
            funcs: &[],
            convars: &[],
            notes: &["Verified live on build 14184: head_0 = bone 7 sits 58-65 units above the feet on every player; w < 0 for players behind the camera."],
        }),
        external_unavailable: "",
    },
    Feature {
        name: "FOV changer",
        category: "Visuals",
        summary: "Change your first-person field of view without touching the scope zoom.",
        internal: Variant {
            summary: "Override the camera the game is about to render with: hook OverrideView and write the view setup's FOV.",
            steps: &[
                "Hook ClientMode::OverrideView(this, CViewSetup*). Call the original first.",
                "If your pawn is alive and not scoped (C_CSPlayerPawn::m_bIsScoped), write your FOV (degrees, float) to CViewSetup::m_flFov.",
                "Optional: keep aim feeling the same at a wider FOV — write C_BasePlayerPawn::m_flFOVSensitivityAdjust = zoom_sensitivity_ratio * fov / 90.",
            ],
            fields: &[
                f("CViewSetup", "m_flFov", "float", "the FOV the frame renders with"),
                f("C_CSPlayerPawn", "m_bIsScoped", "bool", "leave the scope zoom alone"),
                f("C_BasePlayerPawn", "m_flFOVSensitivityAdjust", "float", "optional sensitivity scale"),
            ],
            globals: &[g("dwLocalPlayerPawn", "C_CSPlayerPawn*")],
            funcs: &[hook("OverrideView", "ClientMode::OverrideView(this, CViewSetup*)")],
            convars: &[("zoom_sensitivity_ratio", "read, for the optional sensitivity scale")],
            notes: &["OverrideView writes origin (+0x4A0), angles (+0x4B8) and FOV (+0x498) of the view setup — see CViewSetup under Engine structs."],
        },
        external: Some(Variant {
            summary: "Set the FOV the game itself falls back to when you are not zoomed: the controller's m_iDesiredFOV.",
            steps: &[
                "controller = read<u64>(client + dwLocalPlayerController).",
                "Write your FOV as an integer to controller + m_iDesiredFOV.",
                "Re-write it now and then (every second, or on respawn): the server can replicate its own value back.",
                "Do not write the camera services' m_iFOV: while it is non-zero it overrides the default, and the server sets it itself when you zoom.",
            ],
            fields: &[
                f("CBasePlayerController", "m_iDesiredFOV", "uint32", "0 = game default (90)"),
                f("CCSPlayerBase_CameraServices", "m_iFOV", "uint32", "read-only for you: 0 means \"use m_iDesiredFOV\""),
            ],
            globals: &[g("dwLocalPlayerController", "CCSPlayerController*")],
            funcs: &[],
            convars: &[],
            notes: &["How the game picks the FOV each frame (IDA, 14184): camera m_iFOV if non-zero, else controller m_iDesiredFOV if non-zero, else the game rules default (90); the scope zoom is applied on top."],
        }),
        external_unavailable: "",
    },
    Feature {
        name: "Aimbot",
        category: "Aim",
        summary: "Turn your view onto the closest visible enemy, with recoil control and smoothing.",
        internal: Variant {
            summary: "Steer inside CCSGOInput::CreateMove, before the game builds the user command from the input's view angles.",
            steps: &[
                "Hook CCSGOInput::CreateMove. It runs before the command is built, so angles you set are the angles the command carries.",
                "Eye position: your pawn's scene node m_vecAbsOrigin + m_vecViewOffset.",
                "Candidates: enemy pawns from the entity cache that are alive, not dormant, on the other team. Aim point: the head_0 bone (C_BaseEntity_GetBoneIdByName) from the bone array.",
                "Angle to a point: pitch = -atan2(dz, sqrt(dx² + dy²)), yaw = atan2(dy, dx), in degrees. Pick the target with the smallest angle to your current view (GetViewAngles) inside your FOV limit, and confirm it is visible with a trace (TraceShape) before locking.",
                "Recoil: subtract the aim punch from C_CSPlayerPawn_GetAimPunch(m_pAimPunchServices, &out, 0) — already doubled — once m_iShotsFired > 1.",
                "Smooth: move by a fraction of the remaining angle each call. Clamp pitch to ±89 and wrap yaw to ±180.",
                "Apply with SetViewAngles(input, 0, &angles) (the input's view angles at +0x688): the command and the camera both follow.",
            ],
            fields: &[
                f("CCSGOInput", "m_angViewAngles", "QAngle", "what CreateMove sends"),
                f("CGameSceneNode", "m_vecAbsOrigin", "Vector", ""),
                f("C_BaseModelEntity", "m_vecViewOffset", "Vector", "origin + this = eye"),
                f("CSkeletonInstance", "m_modelState+0x80", "CTransform*", BONE_NOTE),
                f("C_BaseEntity", "m_iHealth", "int32", ""),
                f("C_BaseEntity", "m_iTeamNum", "uint8", ""),
                f("CGameSceneNode", "m_bDormant", "bool", ""),
                f("C_CSPlayerPawn", "m_pAimPunchServices", "CCSPlayer_AimPunchServices*", "argument for GetAimPunch"),
                f("C_CSPlayerPawn", "m_iShotsFired", "int32", "recoil control from the second shot"),
            ],
            globals: &[],
            funcs: &[
                hook("CreateMove", "CCSGOInput::CreateMove: aim here"),
                call("GetViewAngles", "current view (input, slot 0)"),
                call("SetViewAngles", "apply the new view (input, slot 0, &angles)"),
                call("C_CSPlayerPawn_GetAimPunch", "aim punch (services, &out, 0)"),
                call("C_BaseEntity_GetBoneIdByName", "\"head_0\" -> bone index"),
                call("TraceShape", "visibility check"),
            ],
            convars: &[],
            notes: &[],
        },
        external: Some(Variant {
            summary: "Same maths from outside the process; steer by writing the view angles the game reads, or by moving the mouse.",
            steps: &[
                "Read your view from client + dwViewAngles (pitch, yaw, roll floats — the same storage CreateMove sends) and your eye position (pawn scene node m_vecAbsOrigin + m_vecViewOffset).",
                "Candidates and head_0 positions as in ESP (external).",
                "Visibility without traces: the enemy pawn's m_entitySpottedState.m_bSpottedByMask has the bit of your own player slot set when the game considers them visible to you (slot = your controller's index - 1).",
                "Recoil: the aim punch is not a readable field any more; the game integrates it from CCSPlayer_AimPunchServices (base tick, angle, angular velocity) in 1/128-tick steps. Reproduce that, or leave recoil control out.",
                "Apply: write the new angles to client + dwViewAngles, or move the mouse — counts = degrees / (sensitivity * 0.022 * m_flFOVSensitivityAdjust), with sensitivity = read<float>(read<u64>(client + dwSensitivity) + dwSensitivity_sensitivity).",
            ],
            fields: &[
                f("CGameSceneNode", "m_vecAbsOrigin", "Vector", ""),
                f("C_BaseModelEntity", "m_vecViewOffset", "Vector", "origin + this = eye"),
                f("CSkeletonInstance", "m_modelState+0x80", "CTransform*", BONE_NOTE),
                f("C_CSPlayerPawn", "m_entitySpottedState.m_bSpottedByMask", "uint32[2]", "bit (your slot) = visible to you"),
                f("C_BasePlayerPawn", "m_flFOVSensitivityAdjust", "float", "mouse scaling (scoped)"),
                f("CCSPlayer_AimPunchServices", "m_predictableBaseAngle", "QAngle", "recoil state, see notes"),
            ],
            globals: &[
                g("dwViewAngles", "QAngle, read and write"),
                g("dwSensitivity", "ConVar pointer; value at dwSensitivity_sensitivity"),
                g("dwSensitivity_sensitivity", "value offset inside the ConVar"),
                g("dwLocalPlayerPawn", "C_CSPlayerPawn*"),
                g("dwLocalPlayerController", "CCSPlayerController*"),
                g("dwEntityList", "CGameEntitySystem*"),
            ],
            funcs: &[],
            convars: &[("sensitivity", "read through dwSensitivity"), ("m_yaw", "0.022 unless changed")],
            notes: &["Verified live on build 14184: dwViewAngles holds the current view; dwSensitivity reads the player's sensitivity."],
        }),
        external_unavailable: "",
    },
    Feature {
        name: "Skin changer",
        category: "Skins",
        summary: "Show any paint kit, seed and wear on your weapons.",
        internal: Variant {
            summary: "Turn each weapon's item into a client-side item that carries your paint, then have the game rebuild the weapon's material.",
            steps: &[
                "Hook FrameStageNotify and work after the network update. Your weapons: pawn m_pWeaponServices -> m_hMyWeapons.",
                "Item view = weapon + m_AttributeManager.m_Item. Give it a client-side identity: m_iItemID = 0xF000000000000010 (and m_iItemIDHigh / m_iItemIDLow to its halves), m_iAccountID = your account id, m_bInitialized = true, m_bDisallowSOC = true (stops the game re-resolving it from your real inventory).",
                "Write the paint on the weapon: m_nFallbackPaintKit, m_nFallbackSeed, m_flFallbackWear, m_nFallbackStatTrak (-1 = none).",
                "Write the same paint as item attributes with C_EconItemView_SetAttribute: \"set item texture prefab\", \"set item texture seed\", \"set item texture wear\", as floats. The weapon's name and rarity are built from these, not from the fallback fields.",
                "Rebuild: m_nCustomEconReloadEventId = -1; call the weapon's PostDataUpdate (vtable slot 10) with 1; call ApplyEconCustomization(weapon, 1); if the weapon already has composite materials, C_CSWeaponBase_UpdateCompositeMaterial on its composite set; SetMeshGroupMask on the scene node (2 for legacy-model paint kits, else 1); finally C_CSWeaponBase_UpdateCompositeMaterialSet(weapon, 1).",
                "For the weapon in your hand, set the same mesh group on the view model (m_hHudModelArms). If the name changed, C_EconItemView_InvalidateDescription so the HUD re-reads it.",
                "Only re-apply when the game has rebuilt the weapon over you (paint attributes or composite gone), not every frame: the rebuild is asynchronous and restarting it cancels it.",
            ],
            fields: &[
                f("C_BasePlayerPawn", "m_pWeaponServices", "CPlayer_WeaponServices*", ""),
                f("CPlayer_WeaponServices", "m_hMyWeapons", "CUtlVector<CHandle<C_BasePlayerWeapon>>", ""),
                f("C_EconEntity", "m_AttributeManager.m_Item", "C_EconItemView", "the weapon's item view"),
                f("C_EconItemView", "m_iItemID", "uint64", "0xF000000000000010: a client-side item"),
                f("C_EconItemView", "m_iItemIDHigh", "uint32", "high half of m_iItemID"),
                f("C_EconItemView", "m_iItemIDLow", "uint32", "low half of m_iItemID"),
                f("C_EconItemView", "m_iAccountID", "uint32", "your account id"),
                f("C_EconItemView", "m_bInitialized", "bool", "true"),
                f("C_EconItemView", "m_bDisallowSOC", "bool", "true: never re-resolve from the real inventory"),
                f("C_EconEntity", "m_nFallbackPaintKit", "int32", ""),
                f("C_EconEntity", "m_nFallbackSeed", "int32", ""),
                f("C_EconEntity", "m_flFallbackWear", "float", "0.0 factory new … 1.0 battle-scarred"),
                f("C_EconEntity", "m_nFallbackStatTrak", "int32", "-1 = none"),
                f("C_CSWeaponBase", "m_nCustomEconReloadEventId", "int32", "-1 before the rebuild"),
                m("C_CSWeaponBase", "composite material set", 0x610, 14184, "", "argument for UpdateCompositeMaterial; live material count at +0x4A0"),
                f("C_CSPlayerPawn", "m_hHudModelArms", "CHandle", "first-person arms -> view-model weapon"),
            ],
            globals: &[],
            funcs: &[
                hook("FrameStageNotify", "apply after the network update"),
                call("C_EconItemView_SetAttribute", "paint attributes on the item view"),
                call("ApplyEconCustomization", "queue the econ reload (weapon, 1)"),
                call("C_CSWeaponBase_UpdateCompositeMaterial", "rebuild the composite material"),
                call("SetMeshGroupMask", "legacy (2) or modern (1) model"),
                call("C_CSWeaponBase_UpdateCompositeMaterialSet", "regenerate the weapon skin (weapon, 1)"),
                call("C_EconItemView_InvalidateDescription", "refresh the HUD name"),
            ],
            convars: &[("cl_weapon_selection_rarity_color", "defaults to 0; the rarity outline only shows when it is 1")],
            notes: &["The client-side item id marks the item as not coming from the Game Coordinator, so the game uses the fallback paint instead of looking the item up."],
        },
        external: None,
        external_unavailable: "The new paint only renders after the game rebuilds the weapon's composite material, and that means calling game functions (ApplyEconCustomization, UpdateCompositeMaterial, ...). An external process can write the fields but cannot make the game repaint.",
    },
    Feature {
        name: "Knife changer",
        category: "Skins",
        summary: "Swap your knife for any other knife model, with a paint.",
        internal: Variant {
            summary: "Change the knife's definition and subclass, load the new model, rebind its animations, then paint it like any skin.",
            steps: &[
                "Hook FrameStageNotify; find your knife among m_hMyWeapons.",
                "Item view: set the client-side identity as for skins, m_iItemDefinitionIndex = the new knife, m_iEntityQuality = 3 (the ★).",
                "m_nSubclassID = MurmurHash2 of the definition index as a lowercase decimal string, seed 0x31415926. It selects the knife's data: model, sequences, animation graph.",
                "Call C_CSWeaponBase_GetViewModel(weapon), then ChangeModel(weapon, C_CSWeaponBase_GetModelPath(item_view)).",
                "Rebind the animation graph for the new model: the weapon's CBaseAnimGraphController, vtable slot 15, mode 2 (animgraph2) — only when m_nAnimationAlgorithm is 2 and the model is loaded. Without it the new knife plays the old knife's animations.",
                "Paint it exactly like the skin changer (fallback fields, paint attributes, rebuild), and set the mesh group on the view model too.",
                "Remember the original definition, subclass and paint, and put them back when the feature is turned off.",
            ],
            fields: &[
                f("C_EconEntity", "m_AttributeManager.m_Item.m_iItemDefinitionIndex", "uint16", "the new knife"),
                f("C_EconItemView", "m_iEntityQuality", "int32", "3 = unusual (★)"),
                f("C_BaseEntity", "m_nSubclassID", "CUtlStringToken", "MurmurHash2(lowercase \"<def index>\", 0x31415926)"),
                f("CBaseAnimGraphController", "m_nAnimationAlgorithm", "AnimationAlgorithm_t", "rebind only when 2"),
                f("CBaseAnimGraphController", "m_sAnimGraph2Identifier", "CGlobalSymbol", "graph the new model must provide"),
                f("C_CSPlayerPawn", "m_hHudModelArms", "CHandle", "view model"),
            ],
            globals: &[],
            funcs: &[
                hook("FrameStageNotify", "apply after the network update"),
                call("C_CSWeaponBase_GetViewModel", "refresh after the subclass change"),
                call("C_CSWeaponBase_GetModelPath", "model for the item view's definition"),
                call("ChangeModel", "load the new knife model (weapon, path)"),
                call("C_EconItemView_SetAttribute", "paint attributes"),
                call("ApplyEconCustomization", "queue the econ reload"),
                call("SetMeshGroupMask", "legacy (2) or modern (1) model"),
                call("C_CSWeaponBase_UpdateCompositeMaterialSet", "regenerate the skin"),
            ],
            convars: &[],
            notes: &[],
        },
        external: None,
        external_unavailable: "Changing the knife needs the game to load a model and rebind its animation graph, which only game code can do.",
    },
];

// ---------------------------------------------------------------------------
// Resolution
// ---------------------------------------------------------------------------

fn find_class<'a>(map: &'a SchemaMap, name: &str) -> Option<&'a Class> {
    // Client first: client and server share class names with different layouts.
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
const IMPLICIT_BASES: &[(&str, &str)] = &[
    ("CSMatchStats_t", "CSPerRoundStats_t"),
    ("CCSPlayer_CameraServices", "CCSPlayerBase_CameraServices"),
];

fn parent_of(map: &SchemaMap, c: &Class) -> Option<String> {
    if let Some(p) = &c.parent_name {
        return Some(p.clone());
    }
    let (_, base) = IMPLICIT_BASES.iter().find(|(d, _)| *d == c.name)?;
    let b = find_class(map, base)?;
    let first = c.fields.iter().map(|f| f.offset).min().unwrap_or(0);
    (first >= b.size as i32).then(|| base.to_string())
}

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

/// Resolve a field reference. Returns (offset, source, declared_in).
fn resolve_field(map: Option<&SchemaMap>, fld: &Field) -> Option<(u64, &'static str, Option<String>)> {
    if let Some((v, _)) = fld.manual {
        return Some((v as u64, "manual", None));
    }
    // Engine structs (hand-verified, see engine_structs.rs).
    if let Some(es) = ENGINE_STRUCTS.iter().find(|s| s.name == fld.class) {
        return es.fields.iter().find(|x| x.name == fld.field).map(|x| (x.offset as u64, "engine", None));
    }
    let map = map?;
    if fld.field == "sizeof" {
        return find_class(map, fld.class).map(|c| (c.size as u64, "schema", None));
    }
    let (path, delta) = match fld.field.split_once('+') {
        Some((p, d)) => (p, u64::from_str_radix(d.trim().trim_start_matches("0x"), 16).ok()?),
        None => (fld.field, 0),
    };
    let segs: Vec<&str> = path.split('.').collect();
    let mut cls = fld.class.to_string();
    let mut off = 0u64;
    let mut declared = String::new();
    for (i, seg) in segs.iter().enumerate() {
        let (f, d) = find_field(map, &cls, seg.trim())?;
        off += f.offset.max(0) as u64;
        declared = d;
        let ty = f.type_name.trim();
        // A path may only continue through an embedded struct.
        if i + 1 < segs.len() && (ty.ends_with('*') || ty.starts_with("CHandle")) {
            return None;
        }
        cls = ty.to_string();
    }
    let source = if delta != 0 { "schema+manual" } else { "schema" };
    let declared_in = (segs.len() == 1 && declared != fld.class).then_some(declared);
    Some((off + delta, source, declared_in))
}

fn find_hit<'a>(hits: &'a [PatternHit], module: &str, name: &str) -> Option<&'a PatternHit> {
    let dn = display_name(name);
    hits.iter().find(|h| {
        h.found
            && h.module.eq_ignore_ascii_case(module)
            && (h.name == name || h.name == dn || h.aliases.iter().any(|a| a == name))
    })
}

/// Hand-verified offsets: (feature, class, field, build verified on).
pub fn manual_fields() -> Vec<(&'static str, &'static str, &'static str, u32)> {
    let mut v = Vec::new();
    for ft in FEATURES {
        for var in std::iter::once(&ft.internal).chain(ft.external.as_ref()) {
            for fl in var.fields {
                if let Some((_, b)) = fl.manual
                    && !v.iter().any(|(_, c, n, _): &(&str, &str, &str, u32)| *c == fl.class && *n == fl.field)
                {
                    v.push((ft.name, fl.class, fl.field, b));
                }
            }
        }
    }
    v
}

/// References that did not resolve in the last render: (kind, feature, name).
pub static UNRESOLVED: std::sync::Mutex<Vec<(String, String, String)>> = std::sync::Mutex::new(Vec::new());

/// Signature-database names the published recipes reference (for the checks).
pub fn hook_signatures() -> Vec<(&'static str, &'static str, &'static str)> {
    let mut v = Vec::new();
    for ft in FEATURES {
        for var in std::iter::once(&ft.internal).chain(ft.external.as_ref()) {
            for fu in var.funcs {
                v.push((ft.name, fu.module, fu.name));
            }
        }
    }
    v
}

fn render_variant(
    feature: &str,
    v: &Variant,
    build: Option<u32>,
    schemas: Option<&SchemaMap>,
    offsets: Option<&OffsetMap>,
    hits: &[PatternHit],
    unresolved: &mut Vec<(String, String, String)>,
) -> Value {
    let fields: Vec<Value> = v
        .fields
        .iter()
        .map(|fld| {
            let r = resolve_field(schemas, fld);
            if r.is_none() {
                unresolved.push(("field".into(), feature.into(), format!("{}::{}", fld.class, fld.field)));
            }
            let (offset, source, declared_in) = r.map(|(o, s, d)| (format!("0x{:X}", o), s, d)).unwrap_or_default();
            let verified_build = fld.manual.map(|(_, b)| b);
            json!({
                "class": fld.class,
                "field": fld.field,
                "offset": offset,
                "source": if source.is_empty() { "unresolved" } else { source },
                "declared_in": declared_in,
                "verified_build": verified_build,
                "stale": verified_build.zip(build).map(|(vb, b)| vb != b),
                "type": fld.ty,
                "note": fld.note,
            })
        })
        .collect();
    let globals: Vec<Value> = v
        .globals
        .iter()
        .map(|gl| {
            let value = offsets.and_then(|o| o.get(gl.module)).and_then(|m| m.get(gl.name)).map(|r| format!("0x{:X}", *r as u64));
            if value.is_none() {
                unresolved.push(("global".into(), feature.into(), format!("{}!{}", gl.module, gl.name)));
            }
            json!({ "module": gl.module, "name": gl.name, "value": value, "note": gl.note })
        })
        .collect();
    let funcs: Vec<Value> = v
        .funcs
        .iter()
        .map(|fu| {
            let h = find_hit(hits, fu.module, fu.name);
            if h.is_none() {
                unresolved.push(("function".into(), feature.into(), format!("{}!{}", fu.module, fu.name)));
            }
            json!({
                "name": fu.name,
                "module": fu.module,
                "role": fu.role,
                "purpose": fu.purpose,
                "rva": h.and_then(|h| h.rva).map(|r| format!("0x{:X}", r)),
                "pattern": h.map(|h| h.pattern.clone()),
                "prototype": h.and_then(|h| h.prototype.clone()),
            })
        })
        .collect();
    json!({
        "summary": v.summary,
        "steps": v.steps,
        "fields": fields,
        "globals": globals,
        "functions": funcs,
        "convars": v.convars.iter().map(|(n, u)| json!({ "name": n, "use": u })).collect::<Vec<_>>(),
        "notes": v.notes,
    })
}

pub fn render_json(
    build_number: Option<u32>,
    schemas: Option<&SchemaMap>,
    offsets: Option<&OffsetMap>,
    hits: &[PatternHit],
) -> String {
    let mut unresolved = Vec::new();
    let features: Vec<Value> = FEATURES
        .iter()
        .map(|ft| {
            let internal = render_variant(ft.name, &ft.internal, build_number, schemas, offsets, hits, &mut unresolved);
            let external = ft
                .external
                .as_ref()
                .map(|v| render_variant(ft.name, v, build_number, schemas, offsets, hits, &mut unresolved));
            // `summary`, `fields`, `hooks`, `convars` at the top level mirror the
            // internal variant for consumers of the pre-2.1.8 shape.
            json!({
                "name": ft.name,
                "status": "working",
                "category": ft.category,
                "summary": ft.summary,
                "internal": internal,
                "external": external,
                "external_unavailable": if ft.external.is_none() { Some(ft.external_unavailable) } else { None },
                "fields": internal["fields"],
                "hooks": internal["functions"].as_array().map(|a| a.iter().map(|x| json!({
                    "function": x["name"], "module": x["module"], "signature": x["name"], "action": x["purpose"],
                })).collect::<Vec<_>>()).unwrap_or_default(),
                "convars": internal["convars"],
            })
        })
        .collect();
    if let Ok(mut u) = UNRESOLVED.lock() {
        *u = unresolved.clone();
    }
    let doc = json!({
        "cs2_build": build_number,
        "feature_count": features.len(),
        "note": "How each feature is built, internal (in-process) and external (out-of-process). Offsets are resolved from this dump: source schema = schema field, engine = engine struct, schema+manual = schema field plus a fixed delta, manual = not described anywhere else (verified_build says where it was checked; stale = true when that is not this build). Functions carry this build's RVA and pattern.",
        "unresolved": unresolved.iter().map(|(k, f, n)| json!({ "kind": k, "feature": f, "name": n })).collect::<Vec<_>>(),
        "features": features,
    });
    serde_json::to_string_pretty(&doc).unwrap_or_else(|_| String::from("{}"))
}
