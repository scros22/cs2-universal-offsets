//! CS2 signature database (ported + deduplicated from
//! `signature-dumper/cs2sign/CS2EnhancedSignatures.h`).
//!
//! Every entry is module-scoped to its DLL (.text first, .rdata fallback)
//! so scans stay fast and unambiguous.  Patterns that pointed into a
//! relative-addressing instruction use `Rel32` / `RipRel` resolution so the
//! reported address is the final function / global — not the midpoint of
//! whatever instruction matched.  Entries marked `StringRef` find the
//! function by the unique string it references (Ghidra workflow), which
//! is the most resilient kind across CS2 patches.

use super::{ResolveKind, Signature};

const NONE: ResolveKind = ResolveKind::None;
const STRREF: ResolveKind = ResolveKind::StringRef;
const REL32_1: ResolveKind = ResolveKind::Rel32 { rel_off: 1 };
const RIPREL_3: ResolveKind = ResolveKind::RipRel { rel_off: 3 };

pub static CS2_SIGNATURES: &[Signature] = &[
    // ---------- client.dll : input / movement --------------------------
    Signature {
        name: "CCSGOInput::CreateMove",
        module: "client.dll",
        needle: "48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CCSPlayer::ThirdPersonReset",
        module: "client.dll",
        needle: "48 8B 40 08 44 38 20 75 10 44 88 67 01",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "RegenerateWeaponSkins",
        module: "client.dll",
        needle: "48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CSkeletonInstance::SetMeshGroupMask",
        module: "client.dll",
        needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CCSGOViewAdvice::OverrideView",
        module: "client.dll",
        needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        // NOTE: DEAD on build 14154 (0 hits, IDA-verified 2026-04-25).
        // Use `GetWorldFovResolver` instead — see entry near bottom of
        // this file. Kept here so the dumper diff still surfaces 0/N
        // hits as a regression signal if a future build resurrects it.
        name: "SetWorldFov",
        module: "client.dll",
        // E8 disp32 at start of the match -> resolve as call target
        needle: "E8 ? ? ? ? F3 0F 11 45 ? 48 8B 5C 24",
        resolve: ResolveKind::Rel32 { rel_off: 1 },
        extra_off: 0,
    },
    Signature {
        name: "CalcViewmodel",
        module: "client.dll",
        needle: "40 55 53 56 41 56 41 57 48 8B EC",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "NoSpread1",
        module: "client.dll",
        needle: "48 89 5C 24 08 57 48 81 EC F0 00",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CalcSpread",
        module: "client.dll",
        needle: "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA",
        resolve: NONE,
        extra_off: 0,
    },

    // ---------- skin / knife / glove changer --------------------------
    Signature {
        name: "CBaseModelEntity::SetBodyGroup",
        module: "client.dll",
        needle: "85 D2 0F 88 ? ? ? ? 53 55 56 48 83 EC 70 41 8B F0 8B DA 48 8B E9",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CCSPlayerInventory::GetItemInLoadout",
        module: "client.dll",
        needle: "40 55 48 83 EC ? 49 63 E8",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CCSInventoryManager::EquipItemInLoadout",
        module: "client.dll",
        needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CEconItemView::GetCustomPaintKitIndex",
        module: "client.dll",
        needle: "48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25",
        resolve: NONE,
        extra_off: 0,
    },

    // ---------- econ schema -------------------------------------------
    Signature {
        name: "GetEconItemSystem",
        module: "client.dll",
        needle: "48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 48 89 5C 24",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CEconItemSchema::GetAttributeDefinitionByName",
        module: "client.dll",
        needle: "48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 60 48 8D 05",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "SetDynamicAttributeValue",
        module: "client.dll",
        needle: "48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 4D 8B F8",
        resolve: NONE,
        extra_off: 0,
    },

    // ---------- scenesystem.dll ---------------------------------------
    Signature {
        name: "DrawSkyboxArray",
        module: "scenesystem.dll",
        needle: "45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "DrawObject_legacy",
        module: "scenesystem.dll",
        needle: "48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "CSceneAnimatableObject::GeneratePrimitives",
        module: "scenesystem.dll",
        needle: "48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "DrawSmokeVertex",
        module: "client.dll",
        needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8",
        resolve: NONE,
        extra_off: 0,
    },

    // ---------- materialsystem2.dll -----------------------------------
    Signature {
        name: "CMaterialSystem2::CreateMaterial",
        module: "materialsystem2.dll",
        needle: "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "FindParameter",
        module: "materialsystem2.dll",
        needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48",
        resolve: NONE,
        extra_off: 0,
    },
    Signature {
        name: "UpdateParameter",
        module: "materialsystem2.dll",
        needle: "48 89 7C 24 ? 41 56 48 83 EC ? 8B 81",
        resolve: NONE,
        extra_off: 0,
    },

    // ---------- tier0.dll ---------------------------------------------
    Signature {
        name: "LoadKV3_callsite",
        module: "tier0.dll",
        needle: "48 8D 0D ? ? ? ? FF 15 ? ? ? ? 49 8B 06",
        resolve: NONE,
        extra_off: 0,
    },

    // ---------- engine2.dll -------------------------------------------
    Signature {
        name: "ClientStateSignOnState",
        module: "engine2.dll",
        needle: "83 3D ? ? ? ? 06 0F 94 C0",
        resolve: NONE,
        extra_off: 0,
    },

    // ---------- string-anchored (robust across patches) ---------------
    Signature { name: "Engine_GetTime",                module: "engine2.dll", needle: "Engine_GetTime",                resolve: STRREF, extra_off: 0 },
    Signature { name: "CL_FullyConnected",             module: "engine2.dll", needle: "CL_FullyConnected",             resolve: STRREF, extra_off: 0 },
    Signature { name: "Host_AccumulateTime",           module: "engine2.dll", needle: "Host_AccumulateTime",           resolve: STRREF, extra_off: 0 },
    Signature { name: "CNetChan_ProcessMessages",      module: "engine2.dll", needle: "CNetChan::ProcessMessages",     resolve: STRREF, extra_off: 0 },

    Signature { name: "CCSPlayer_WeaponServices",      module: "client.dll",  needle: "CCSPlayer_WeaponServices",      resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_MovementServices",    module: "client.dll",  needle: "CCSPlayer_MovementServices",    resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_BulletServices",      module: "client.dll",  needle: "CCSPlayer_BulletServices",      resolve: STRREF, extra_off: 0 },
    Signature { name: "CSGameRules",                   module: "client.dll",  needle: "CSGameRules",                   resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayerController",           module: "client.dll",  needle: "CCSPlayerController",           resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayerPawn",                 module: "client.dll",  needle: "CCSPlayerPawn",                 resolve: STRREF, extra_off: 0 },
    Signature { name: "CHudWeaponSelection",           module: "client.dll",  needle: "CHudWeaponSelection",           resolve: STRREF, extra_off: 0 },
    Signature { name: "CHudDeathNotice",               module: "client.dll",  needle: "CHudDeathNotice",               resolve: STRREF, extra_off: 0 },
    Signature { name: "paintkit_seed",                 module: "client.dll",  needle: "set item texture seed",         resolve: STRREF, extra_off: 0 },
    Signature { name: "paintkit_prefab",               module: "client.dll",  needle: "set item texture prefab",       resolve: STRREF, extra_off: 0 },
    Signature { name: "paintkit_wear",                 module: "client.dll",  needle: "set item texture wear",         resolve: STRREF, extra_off: 0 },
    Signature { name: "statTrak_killEater",            module: "client.dll",  needle: "kill eater",                    resolve: STRREF, extra_off: 0 },
    Signature { name: "statTrak_scoreType",            module: "client.dll",  needle: "kill eater score type",         resolve: STRREF, extra_off: 0 },

    Signature { name: "VacNet_OnEvent",                module: "client.dll",  needle: "VAC-Net Detection",             resolve: STRREF, extra_off: 0 },
    Signature { name: "Matchmaking_AcceptMatch",       module: "client.dll",  needle: "AcceptInviteToParty",           resolve: STRREF, extra_off: 0 },

    // ==================================================================
    // NUVORA APR-2026 EXPANSION (client.dll) ---------------------------
    // ==================================================================
    // Hooks / view / rendering -----------------------------------------
    Signature { name: "CGameEntitySystem::OnAddEntity",       module: "client.dll", needle: "48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81", resolve: NONE, extra_off: 0 },
    Signature { name: "CGameEntitySystem::OnRemoveEntity",    module: "client.dll", needle: "48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89", resolve: NONE, extra_off: 0 },
    Signature { name: "GetMatrixForView",                     module: "client.dll", needle: "40 53 48 83 EC 60 0F 29 74 24 50 0F 57 DB F3 0F 10 ? ? ? ? ? 49 8B D8", resolve: NONE, extra_off: 0 },
    Signature { name: "IsGlowing",                            module: "client.dll", needle: "E8 ? ? ? ? 33 DB 84 C0 0F 84 ? ? ? ? 48 8B 4F", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetGlowColor",                         module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B F9 48 8B 54 24", resolve: NONE, extra_off: 0 },
    Signature { name: "FlashOverlay",                         module: "client.dll", needle: "85 D2 0F 88 ? ? ? ? 48 89 4C 24", resolve: NONE, extra_off: 0 },
    Signature { name: "DrawOverHead",                         module: "client.dll", needle: "40 53 48 83 EC ? 48 8B D9 83 FA ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 10", resolve: NONE, extra_off: 0 },
    Signature { name: "DrawCrosshair",                        module: "client.dll", needle: "48 89 5C 24 08 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 85", resolve: NONE, extra_off: 0 },
    Signature { name: "FirstPersonLegs",                      module: "client.dll", needle: "40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42", resolve: NONE, extra_off: 0 },
    Signature { name: "HandleTeamIntro",                      module: "client.dll", needle: "48 83 EC ? ? ? ? ? 44 38 89", resolve: NONE, extra_off: 0 },
    Signature { name: "DrawViewPunch",                        module: "client.dll", needle: "48 89 5C 24 ? 55 56 57 48 83 EC ? 48 83 79", resolve: NONE, extra_off: 0 },
    Signature { name: "DrawScopeOverlay",                     module: "client.dll", needle: "48 8B C4 53 57 48 83 EC ? 48 8B FA", resolve: NONE, extra_off: 0 },
    Signature { name: "UpdatePostProcessing",                 module: "client.dll", needle: "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 08 57 48 83 EC 60 80", resolve: NONE, extra_off: 0 },
    Signature { name: "SetupMove",                            module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 4C 8B F1 E8 ? ? ? ? 48 8D 15", resolve: NONE, extra_off: 0 },

    // Interface / global pointers --------------------------------------
    Signature { name: "GlobalVariables_ptr",                  module: "client.dll", needle: "48 89 15 ? ? ? ? 48 89 42", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "GameRules_ptr",                        module: "client.dll", needle: "48 8B 1D ? ? ? ? 48 8D 54 24 ? 0F 28 D0 48 8D 4C 24 ?", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "GameEntitySystemPtr",                  module: "client.dll", needle: "48 8B 1D ? ? ? ? 48 89 1D ? ? ? ?", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "ParticleManager_ptr",                  module: "client.dll", needle: "48 8B 0D ? ? ? ? 41 B8 ? ? ? ? F3 0F 11 74 24 ? 48 C7 44 24 ? ? ? ? ?", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "SwapChain_ptr",                        module: "client.dll", needle: "48 89 2D ? ? ? ? 48 C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 89 2D", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "CSGOInput_ptr",                        module: "client.dll", needle: "48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "ClientMode_ptr",                       module: "client.dll", needle: "48 8D 0D ? ? ? ? 48 69 C0 ? ? ? ? 48 03 C1 C3 CC CC", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "ViewRender_ptr",                       module: "client.dll", needle: "48 89 05 ? ? ? ? 48 8B C8 48 85 C0", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "VPhys2World_ptr",                      module: "client.dll", needle: "4C 8B 25 ? ? ? ? 24", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "PVSManager_ptr",                       module: "client.dll", needle: "48 8D 0D ? ? ? ? 33 D2 FF 50", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "GetBBox_ptr",                          module: "client.dll", needle: "48 8B 0D ? ? ? ? 48 85 C9 74 ? ? ? ? 48 FF A0 ? ? ? ? 48 8D 05", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "GetInstanceS",                         module: "client.dll", needle: "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 8B 91 ? ? ? ? B8", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "ChamsRenderGameSystem",                module: "client.dll", needle: "48 8B 0D ? ? ? ? ? ? E8 ? ? ? ? 49 8B 8E ? ? ? ? 4C 8D 0D", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "CAM_ThinkReturn",                      module: "client.dll", needle: "BA 04 00 00 00 FF 15 ? ? ? ? 84 C0 0F 84", resolve: NONE, extra_off: 0 },

    // Features / aimbot / autowall / movement ---------------------------
    Signature { name: "CalculateShootPosition",               module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 44 8B 92 ? ? ? ?", resolve: NONE, extra_off: 0 },
    Signature { name: "AutowallInit",                         module: "client.dll", needle: "40 53 48 83 EC ? 48 8B D9 48 81 C1 ? ? ? ? E8 ? ? ? ?", resolve: NONE, extra_off: 0 },
    Signature { name: "AutowallResolveTracePos",              module: "client.dll", needle: "E8 ? ? ? ? 48 63 83 ? ? ? ? 48 8D 14 40", resolve: REL32_1, extra_off: 0 },
    Signature { name: "AutowallTracePos",                     module: "client.dll", needle: "40 55 56 41 54 41 55 41 57 48 8B EC", resolve: NONE, extra_off: 0 },
    Signature { name: "AutowallTraceData",                    module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 09", resolve: NONE, extra_off: 0 },
    Signature { name: "TestSurfaces",                         module: "client.dll", needle: "40 53 57 41 56 48 83 EC 50 8B", resolve: NONE, extra_off: 0 },
    Signature { name: "ReportHit",                            module: "client.dll", needle: "E8 ? ? ? ? 48 8B AC 24 D8 00 00 00 48 81 C4", resolve: REL32_1, extra_off: 0 },
    Signature { name: "SetTraceData",                         module: "client.dll", needle: "E8 ? ? ? ? 8B 85 ? ? ? ? 48 8D 54 24 ? F2 0F 10 45", resolve: REL32_1, extra_off: 0 },
    Signature { name: "SetTraceInit",                         module: "client.dll", needle: "E8 ? ? ? ? F2 0F 10 0B 4C 8D 0D", resolve: REL32_1, extra_off: 0 },
    Signature { name: "HandleEntityList",                     module: "client.dll", needle: "E8 ? ? ? ? 84 C0 74 ? 48 63 03", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetBasePlayerController",              module: "client.dll", needle: "48 8B F8 48 85 C0 74 ? 48 8B C8 E8 ? ? ? ? 44 39 A8", resolve: NONE, extra_off: 0 },
    Signature { name: "GetTickBase",                          module: "client.dll", needle: "E8 ? ? ? ? EB ? 48 8B 05 ? ? ? ? 8B 40", resolve: REL32_1, extra_off: 0 },
    Signature { name: "FindHudElement",                       module: "client.dll", needle: "48 8D 15 ? ? ? ? 45 33 C0 B9 ? ? ? ? FF 15 ? ? ? ? EB ? 48 8B 15", resolve: NONE, extra_off: 0 },
    Signature { name: "FindHudElement_panorama",              module: "client.dll", needle: "4C 8B DC 53 48 83 EC 50 48 8B 05", resolve: NONE, extra_off: 0 },
    Signature { name: "HudChatPrintf",                        module: "client.dll", needle: "E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?", resolve: REL32_1, extra_off: 0 },
    Signature { name: "Scope_callsite",                       module: "client.dll", needle: "E8 ? ? ? ? 80 7C 24 34 ? 74 ?", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetRemovedAimpunch",                   module: "client.dll", needle: "F2 0F 10 44 24 ? F2 0F 11 84 24 ? ? ? ? FF 15", resolve: NONE, extra_off: 0 },
    Signature { name: "GetRemovedAimPunch_E8",                module: "client.dll", needle: "E8 ? ? ? ? 4C 8B C0 48 8D 55 ? 48 8B CB E8 ? ? ? ? 48 8D 0D", resolve: REL32_1, extra_off: 0 },
    Signature { name: "ChamsGetWorldGroupID",                 module: "client.dll", needle: "E8 ? ? ? ? 48 8B 0D ? ? ? ? ? ? E8 ? ? ? ? 49 8B 8E ? ? ? ? 4C 8D 0D", resolve: REL32_1, extra_off: 0 },
    Signature { name: "ModulationUpdate",                     module: "client.dll", needle: "48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 E8 ? ? ? ? 84 C0 0F 84", resolve: NONE, extra_off: 0 },
    Signature { name: "ClearHUDWeaponIcon",                   module: "client.dll", needle: "E8 ? ? ? ? 8B F8 C6 84 24 ? ? ? ? ?", resolve: REL32_1, extra_off: 0 },
    Signature { name: "PlayVSound_client",                    module: "client.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 FF", resolve: NONE, extra_off: 0 },
    Signature { name: "ConvarGet",                            module: "client.dll", needle: "8B D0 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F 10 45 ? 83 F0 74", resolve: NONE, extra_off: 0 },

    // Player / pawn / entity functions ---------------------------------
    Signature { name: "SetViewAngle",                         module: "client.dll", needle: "85 D2 75 3D 48 63 81 ? ? ? ?", resolve: NONE, extra_off: 0 },
    Signature { name: "GetViewAngles",                        module: "client.dll", needle: "4C 8B C1 85 D2 74 08 48 8D 05 ? ? ? ? C3", resolve: NONE, extra_off: 0 },
    Signature { name: "GetBaseEntity",                        module: "client.dll", needle: "4C 8D 49 ? 81 FA", resolve: NONE, extra_off: 0 },
    Signature { name: "CalculateWorldSpaceBones",             module: "client.dll", needle: "48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81", resolve: NONE, extra_off: 0 },
    Signature { name: "TraceShape",                           module: "client.dll", needle: "48 89 5C 24 ? 48 89 4C 24 ? 55 57", resolve: NONE, extra_off: 0 },
    Signature { name: "ClipRayToEntity",                      module: "client.dll", needle: "48 8B C4 48 89 58 ? 55 56 57 41 54 41 56 48 8D 68 ? 48 81 EC ? ? ? ? 48 8B 5D", resolve: NONE, extra_off: 0 },
    Signature { name: "GetSurfaceData",                       module: "client.dll", needle: "E8 ? ? ? ? 80 78 18 00", resolve: REL32_1, extra_off: 0 },
    Signature { name: "SetTypeKV3",                           module: "client.dll", needle: "40 53 48 83 EC 30 4C 8B 11 41 B9 ? ? ? ? 49 83 CA 01 0F B6 C2 80 FA 06 48 8B D9 44 0F 45 C8", resolve: NONE, extra_off: 0 },
    Signature { name: "CreateParticleEffect",                 module: "client.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? F3 0F 10 1D ? ? ? ? 41 8B F8 8B DA 4C 8D 05", resolve: NONE, extra_off: 0 },
    Signature { name: "SetPlayerReady",                       module: "client.dll", needle: "40 53 48 83 EC 20 48 8B DA 48 8D 15 ? ? ? ? 48 8B CB FF 15 ? ? ? ? 85 C0 75 14 BA", resolve: NONE, extra_off: 0 },
    Signature { name: "CacheParticleEffect",                  module: "client.dll", needle: "4C 8B DC 53 48 81 EC ? ? ? ? F2 0F 10 05", resolve: NONE, extra_off: 0 },
    Signature { name: "GetEntityHandle",                      module: "client.dll", needle: "48 85 C9 74 32 48 8B 49 10 48 85 C9 74 29 44 8B 41 10 BA", resolve: NONE, extra_off: 0 },
    Signature { name: "LookupBone",                           module: "client.dll", needle: "E8 ? ? ? ? 48 8B 8D ? ? ? ? B3", resolve: REL32_1, extra_off: 0 },
    Signature { name: "TraceSmokeDensity",                    module: "client.dll", needle: "E8 ? ? ? ? 0F 28 F8 44 0F 28 54 24 ?", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetInventoryManager",                  module: "client.dll", needle: "E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02", resolve: REL32_1, extra_off: 0 },
    Signature { name: "UpdateCompositeMaterial",              module: "client.dll", needle: "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 44 0F B6 F2 48 8B F1 E8", resolve: NONE, extra_off: 0 },
    Signature { name: "RegenerateWeaponSkin",                 module: "client.dll", needle: "40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?", resolve: NONE, extra_off: 0 },
    Signature { name: "SetModel",                             module: "client.dll", needle: "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24", resolve: NONE, extra_off: 0 },
    Signature { name: "SetMeshGroupMask",                     module: "client.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71", resolve: NONE, extra_off: 0 },
    Signature { name: "AddNametagEntity",                     module: "client.dll", needle: "40 55 53 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA", resolve: NONE, extra_off: 0 },
    Signature { name: "AddStattrakEntity",                    module: "client.dll", needle: "48 8B C4 48 89 58 08 48 89 70 10 57 48 83 EC 50 33 F6 8B FA 48 8B D1", resolve: NONE, extra_off: 0 },
    Signature { name: "CreateSOSubclassEconItem",             module: "client.dll", needle: "48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85", resolve: NONE, extra_off: 0 },
    Signature { name: "CreateBaseTypeCache",                  module: "client.dll", needle: "40 53 48 83 EC ? 4C 8B 49 ? 44 8B D2", resolve: NONE, extra_off: 0 },
    Signature { name: "GetClientSystem",                      module: "client.dll", needle: "E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 8B D8 85 C0 74 33", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetClientSystem_inv",                  module: "client.dll", needle: "E8 ? ? ? ? 48 8B 47 10 8B 48 30 D1 E9 F6 C1 01 0F 84 8E", resolve: REL32_1, extra_off: 0 },
    Signature { name: "CAttributeStringInit",                 module: "client.dll", needle: "E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 7D ? 48 89 45 ? 49 8D 4F", resolve: REL32_1, extra_off: 0 },
    Signature { name: "CAttributeStringFill",                 module: "client.dll", needle: "E8 ? ? ? ? 41 83 CF 08", resolve: REL32_1, extra_off: 0 },
    Signature { name: "CBufferStringInit",                    module: "client.dll", needle: "48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 48 8D 79", resolve: NONE, extra_off: 0 },
    Signature { name: "DispatchEffect",                       module: "client.dll", needle: "48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B DA 48 8D 4C 24", resolve: NONE, extra_off: 0 },
    Signature { name: "LoadFileForMe",                        module: "client.dll", needle: "40 55 57 41 56 48 83 EC 20 4C", resolve: NONE, extra_off: 0 },
    Signature { name: "UpdateSubClass",                       module: "client.dll", needle: "48 8B 41 10 48 8B D9 8B 50 30", resolve: NONE, extra_off: 0 },
    Signature { name: "CreateNewSubtickMoveStep",             module: "client.dll", needle: "E8 ? ? ? ? 48 8B D0 48 8B CE E8 ? ? ? ? 48 8B C8", resolve: REL32_1, extra_off: 0 },
    Signature { name: "SetCollisionBounds",                   module: "client.dll", needle: "48 83 EC ? F2 0F 10 02 8B 42 08", resolve: NONE, extra_off: 0 },
    Signature { name: "CalculateInterpolation",               module: "client.dll", needle: "E8 ? ? ? ? 8B 45 ? 3B 45 60 75 04 32 D2 EB 09 BA 01 00 00 00 41 0F 4C D5 C0 EA 07 84 D2 0F 85 87", resolve: REL32_1, extra_off: 0 },
    Signature { name: "CalculateAnimState",                   module: "client.dll", needle: "40 55 56 57 41 54 41 55 41 56 41 57 B8 10 11 00 00 E8 ? ? ? ? 48 2B E0 48 8D 6C 24 40 48 8D 05 ? ? ? ? 48 C7 45 08 4B 0C 00 00 48 8B F1", resolve: NONE, extra_off: 0 },
    Signature { name: "SetAbsOrigin_BaseModel",               module: "client.dll", needle: "48 89 5C 24 08 57 48 83 EC 40 48 8B 01 48 8B FA", resolve: NONE, extra_off: 0 },
    Signature { name: "SetAbsOrigin_Pawn",                    module: "client.dll", needle: "48 89 5C 24 ? 57 48 83 EC ? ? ? ? 48 8B FA 48 8B D9 FF 90 ? ? ? ? 84 C0 0F 85", resolve: NONE, extra_off: 0 },
    Signature { name: "PhysicsRunThink_Pawn",                 module: "client.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 48 8B F9", resolve: NONE, extra_off: 0 },
    Signature { name: "SomeTimingFromPawn",                   module: "client.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 63 D8 48 8B F1", resolve: NONE, extra_off: 0 },
    Signature { name: "GetUserCmdManager",                    module: "client.dll", needle: "41 56 41 57 48 83 EC ? 48 8D 54 24", resolve: NONE, extra_off: 0 },
    Signature { name: "GetPlayerInterp",                      module: "client.dll", needle: "40 53 48 83 EC ? 48 8B D9 48 8B 0D ? ? ? ? 48 83 C1", resolve: NONE, extra_off: 0 },
    Signature { name: "PhysicsRunThink_Ctrl",                 module: "client.dll", needle: "48 89 5C 24 ? 57 48 81 EC ? ? ? ? ? ? ? 48 8B F9 FF 90", resolve: NONE, extra_off: 0 },
    Signature { name: "RunCommand",                           module: "client.dll", needle: "48 8B C4 48 81 EC ? ? ? ? 48 89 58 10", resolve: NONE, extra_off: 0 },
    Signature { name: "ProcessMovement",                      module: "client.dll", needle: "E8 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 48 85 DB", resolve: REL32_1, extra_off: 0 },
    Signature { name: "ForceButtonsDown",                     module: "client.dll", needle: "40 53 57 41 56 48 81 EC ? ? ? ? 48 83 79", resolve: NONE, extra_off: 0 },
    Signature { name: "SetupMovementMoves",                   module: "client.dll", needle: "48 8B ? E8 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 83 C4 30", resolve: NONE, extra_off: 0 },
    Signature { name: "ProcessImpacts",                       module: "client.dll", needle: "48 8B C4 53 56 41 55", resolve: NONE, extra_off: 0 },
    Signature { name: "CSBaseGunFireData_fn",                 module: "client.dll", needle: "48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 A8 48 81 EC ? ? ? ? 4C 8B 69", resolve: NONE, extra_off: 0 },
    Signature { name: "GetWeaponInAccuracyRecoveryTime",      module: "client.dll", needle: "E8 ? ? ? ? F3 0F 10 B7 ? ? ? ? F3 0F 5E F8", resolve: REL32_1, extra_off: 0 },
    Signature { name: "UpdateTurningInAccuracy",              module: "client.dll", needle: "E8 ? ? ? ? F3 0F 10 87 ? ? ? ? 44 0F 2F C8", resolve: REL32_1, extra_off: 0 },

    // Trace manager / filters ------------------------------------------
    Signature { name: "TraceToExit",                          module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? F2 0F 10 02", resolve: NONE, extra_off: 0 },
    Signature { name: "GetTraceInfo",                         module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 0F 29 74 24 ? 48 8B CA", resolve: NONE, extra_off: 0 },
    Signature { name: "InitFilter",                           module: "client.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF 24 C9 C7 41 ?", resolve: NONE, extra_off: 0 },
    Signature { name: "HandleBulletPenetration",              module: "client.dll", needle: "48 8B C4 44 89 48 ? 48 89 50 ? 48 89 48 ? 55", resolve: NONE, extra_off: 0 },
    Signature { name: "InitTraceInfo",                        module: "client.dll", needle: "40 55 41 55 41 57 48 83 EC", resolve: NONE, extra_off: 0 },
    Signature { name: "InitPlayerMovementTraceFilter",        module: "client.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 41 ? 33 FF C7 41 ?", resolve: NONE, extra_off: 0 },
    Signature { name: "TracePlayerBBox",                      module: "client.dll", needle: "48 89 5C 24 ? 55 57 41 54 41 55 41 56", resolve: NONE, extra_off: 0 },
    Signature { name: "CreateEntityByClassName",              module: "client.dll", needle: "4C 8D 05 ? ? ? ? 4C 8B CF BA 03 00 00 00 FF 15 ? ? ? ? EB ? 0F B7 C8 48", resolve: NONE, extra_off: 0 },
    Signature { name: "DestroyParticle",                      module: "client.dll", needle: "83 FA ? 0F 84 ? ? ? ? 41 54", resolve: NONE, extra_off: 0 },
    Signature { name: "LoadPath",                             module: "client.dll", needle: "E8 ? ? ? ? 8B 44 24 2C", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetChatObject",                        module: "client.dll", needle: "E8 ? ? ? ? 48 8B E8 48 85 C0 0F 84 ? ? ? ? 4C 8D 05", resolve: REL32_1, extra_off: 0 },
    Signature { name: "SendChatMessage",                      module: "client.dll", needle: "E8 ? ? ? ? 49 8B 4E 20 BA ? ? ? ?", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetInt2_Event",                        module: "client.dll", needle: "48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 48 63 FA 41", resolve: NONE, extra_off: 0 },
    Signature { name: "FindSOCache",                          module: "client.dll", needle: "48 89 5C 24 08 57 48 83 EC 30 4C 8B 52 08 48 8B D9 8B 0A", resolve: NONE, extra_off: 0 },
    Signature { name: "SetDynamicAttributeValue_raw",         module: "client.dll", needle: "48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24", resolve: NONE, extra_off: 0 },
    Signature { name: "SetBodyGroup_inv",                     module: "client.dll", needle: "85 D2 0F 88 ? ? ? ? 53 55", resolve: NONE, extra_off: 0 },
    Signature { name: "LevelInit",                            module: "client.dll", needle: "40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48", resolve: NONE, extra_off: 0 },
    Signature { name: "ParticleCollection",                   module: "client.dll", needle: "48 89 5C 24 ? 57 48 83 EC ? 0F 28 05", resolve: NONE, extra_off: 0 },
    Signature { name: "GetLocalControllerById",               module: "client.dll", needle: "48 83 EC 28 83 F9 FF 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 4C 8D 05", resolve: NONE, extra_off: 0 },
    Signature { name: "SetupCmd",                             module: "client.dll", needle: "48 83 EC 28 E8 ? ? ? ? 8B 80", resolve: NONE, extra_off: 0 },
    Signature { name: "GetControllerCmd",                     module: "client.dll", needle: "40 53 48 83 EC 20 8B DA E8 ? ? ? ? 4C", resolve: NONE, extra_off: 0 },
    Signature { name: "PhysicsRunThink",                      module: "client.dll", needle: "E8 ? ? ? ? 49 8B D6 48 8B CE E8 ? ? ? ? 48 8B 06", resolve: REL32_1, extra_off: 0 },
    Signature { name: "GetBasePlayerCtrl_Entity",             module: "client.dll", needle: "E8 ? ? ? ? 48 8B F8 48 85 C0 74 ? 48 8B C8 E8 ? ? ? ? EB", resolve: REL32_1, extra_off: 0 },

    // ==================================================================
    // scenesystem.dll --------------------------------------------------
    // ==================================================================
    Signature { name: "SceneSystem::DrawAggeregateObject",    module: "scenesystem.dll", needle: "48 8B C4 4C 89 48 20 4C 89 40 ? 48 89 50 ? 55 53 41 57 48 8D A8", resolve: NONE, extra_off: 0 },
    Signature { name: "SceneSystem::DrawArrayLight",          module: "scenesystem.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 54 24", resolve: NONE, extra_off: 0 },
    Signature { name: "SceneSystem::DrawAggregateSceneObject", module: "scenesystem.dll", needle: "48 8B C4 4C 89 48 20 48 89 50 ? 48 89 48 ? 55 48 8D A8 ? ? ? ? 48 81 EC 70 07 00 00", resolve: NONE, extra_off: 0 },

    // ==================================================================
    // particles.dll ----------------------------------------------------
    // ==================================================================
    Signature { name: "Particles::DrawArray",                 module: "particles.dll", needle: "40 55 53 56 57 48 8D 6C 24", resolve: NONE, extra_off: 0 },
    Signature { name: "Particles::FindKeyVar",                module: "particles.dll", needle: "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 33 C0 8B DA", resolve: NONE, extra_off: 0 },
    Signature { name: "Particles::SetMaterialShaderType",     module: "particles.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 4C 63 32", resolve: NONE, extra_off: 0 },
    Signature { name: "Particles::SetMaterialFunction",       module: "particles.dll", needle: "48 89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC E8 05 00 00", resolve: NONE, extra_off: 0 },

    // ==================================================================
    // soundsystem.dll --------------------------------------------------
    // ==================================================================
    Signature { name: "SoundSystem::SomeUtlSymbolFunc",       module: "soundsystem.dll", needle: "48 89 74 24 18 57 48 83 EC 20 48 63 F2 48 8B F9 3B 71 30", resolve: NONE, extra_off: 0 },
    Signature { name: "SoundSystem::PlayVSound",              module: "soundsystem.dll", needle: "48 8B C4 48 89 58 08 57 48 81 EC ? ? ? ? 33 FF 48 8B D9", resolve: NONE, extra_off: 0 },

    // ==================================================================
    // animationsystem.dll ----------------------------------------------
    // ==================================================================
    Signature { name: "Animation::ShouldUpdateSequences",     module: "animationsystem.dll", needle: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 49 8B 40 48", resolve: NONE, extra_off: 0 },

    // ==================================================================
    // materialsystem2.dll ----------------------------------------------
    // ==================================================================
    Signature { name: "MatSys::PrepareSceneMaterial",         module: "materialsystem2.dll", needle: "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 59 ? 48 8B F2 48 63 79 ? 48 C1 E7 06", resolve: NONE, extra_off: 0 },

    // ==================================================================
    // tier0.dll --------------------------------------------------------
    // ==================================================================
    Signature { name: "Tier0::UtlBuffer",                     module: "tier0.dll", needle: "48 89 5C 24 ? 57 48 83 EC ? 8B 41 ? 8D 7A", resolve: NONE, extra_off: 0 },
    Signature { name: "Tier0::LoadKeyValues",                 module: "tier0.dll", needle: "E8 ? ? ? ? 8B 4C 24 34 0F B6 D8", resolve: REL32_1, extra_off: 0 },

    // ==================================================================
    // engine2.dll ------------------------------------------------------
    // ==================================================================
    Signature { name: "Engine::PVSManager_ptr",               module: "engine2.dll", needle: "48 8D 0D ? ? ? ? 33 D2 FF 50", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "Engine::RunPrediction",                module: "engine2.dll", needle: "40 55 41 56 48 83 EC ? 80 B9", resolve: NONE, extra_off: 0 },
    Signature { name: "Engine::GetScreenAspectRatio",         module: "engine2.dll", needle: "48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8D 0D", resolve: NONE, extra_off: 0 },

    // ==================================================================
    // Additional string-ref anchors (enhanced_signatures.h) ------------
    // ==================================================================
    Signature { name: "CTonemapController2",                  module: "client.dll", needle: "CTonemapController2",                  resolve: STRREF, extra_off: 0 },
    Signature { name: "CFogController",                       module: "client.dll", needle: "CFogController",                       resolve: STRREF, extra_off: 0 },
    Signature { name: "CPostProcessingVolume",                module: "client.dll", needle: "CPostProcessingVolume",                 resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_ItemServices",               module: "client.dll", needle: "CCSPlayer_ItemServices",                resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_ViewModelServices",          module: "client.dll", needle: "CCSPlayer_ViewModelServices",           resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_CameraServices",             module: "client.dll", needle: "CCSPlayer_CameraServices",              resolve: STRREF, extra_off: 0 },
    Signature { name: "C_CSWeaponBase",                       module: "client.dll", needle: "C_CSWeaponBase",                       resolve: STRREF, extra_off: 0 },
    Signature { name: "C_BaseViewModel",                      module: "client.dll", needle: "C_BaseViewModel",                      resolve: STRREF, extra_off: 0 },
    Signature { name: "C_BaseFlex",                           module: "client.dll", needle: "C_BaseFlex",                            resolve: STRREF, extra_off: 0 },
    Signature { name: "C_EconItemView",                       module: "client.dll", needle: "C_EconItemView",                       resolve: STRREF, extra_off: 0 },
    Signature { name: "C_AttributeContainer",                 module: "client.dll", needle: "C_AttributeContainer",                  resolve: STRREF, extra_off: 0 },

    // ==================================================================
    // Community drops (UC forum, Apr 2026) -----------------------------
    // ==================================================================
    Signature { name: "draw_view_punch_v2",                   module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8", resolve: NONE, extra_off: 0 },
    Signature { name: "global_vars_v2",                       module: "client.dll", needle: "48 89 1D ? ? ? ? FF 15 ? ? ? ? 84 C0 74 ? 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ? 4C 8D 05 ? ? ? ? BA ? ? ? ? FF 15 ? ? ? ? 48 8B 74 24 ? 48 8B C3", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "local_controller",                     module: "client.dll", needle: "48 8B 05 ? ? ? ? 41 89 BE", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "entity_list_ptr",                      module: "client.dll", needle: "48 8B 1D ? ? ? ? 48 8D 46", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "view_matrix_ptr",                      module: "client.dll", needle: "48 8D 0D ? ? ? ? 48 89 44 24 ? 48 89 4C 24 ? 4C 8D 0D", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "planted_c4_ptr",                       module: "client.dll", needle: "48 8B 15 ? ? ? ? 48 8B 5C 24 ? FF C0 89 05 ? ? ? ? 48 8B C6 ? ? ? ? 80 BE ? ? ? ? 00", resolve: RIPREL_3, extra_off: 0 },
    Signature { name: "frame_stage_notify",                   module: "client.dll", needle: "4C 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 8F ? ? ? ? F3 41 0F 10 51 ? 45 8B 49 ? 0F 5A D2 66 49 0F 7E D0 FF 15 ? ? ? ? 48 8B 97 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E9", resolve: NONE, extra_off: 0 },
    Signature { name: "override_view_short",                  module: "client.dll", needle: "40 57 48 83 EC ? 48 8B FA E8 ? ? ? ? BA", resolve: NONE, extra_off: 0 },
    Signature { name: "level_shutdown",                       module: "client.dll", needle: "48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 ? ? ? FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 ? ? ? 41 FF 50 ? 48 83 C4", resolve: NONE, extra_off: 0 },
    Signature { name: "level_init_v2",                        module: "client.dll", needle: "40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 0D", resolve: NONE, extra_off: 0 },
    Signature { name: "update_post_processing_v2",            module: "client.dll", needle: "48 89 AC 24 ? ? ? ? 45 33 ED", resolve: NONE, extra_off: 0 },
    Signature { name: "remove_legs",                          module: "client.dll", needle: "40 55 53 56 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? F2 0F 10 42", resolve: NONE, extra_off: 0 },
    Signature { name: "mark_interp_latch_flags_dirty",        module: "client.dll", needle: "40 53 56 57 48 83 EC ? 80 3D ? ? ? ? 00", resolve: NONE, extra_off: 0 },
    Signature { name: "get_fov",                              module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B E9 49 8B F8", resolve: NONE, extra_off: 0 },
    Signature { name: "create_move_v2",                       module: "client.dll", needle: "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40", resolve: NONE, extra_off: 0 },
    Signature { name: "update_global_vars",                   module: "client.dll", needle: "48 8B 0D ? ? ? ? 4C 8D 05 ? ? ? ? 48 85 D2", resolve: NONE, extra_off: 0 },
    Signature { name: "on_add_entity_v2",                     module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 49 8B F0", resolve: NONE, extra_off: 0 },
    Signature { name: "draw_smoke_array",                     module: "client.dll", needle: "40 55 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B E2", resolve: NONE, extra_off: 0 },
    Signature { name: "get_view_angles_v2",                   module: "client.dll", needle: "4D 85 C0 74 ? 85 D2 74", resolve: NONE, extra_off: 0 },
    Signature { name: "unlock_inventory",                     module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 48 8B 0D ? ? ? ? ? ? ? FF 50", resolve: NONE, extra_off: 0 },
    Signature { name: "is_demo_or_hltv",                      module: "client.dll", needle: "48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 84 C0 75 ? 38 05", resolve: NONE, extra_off: 0 },
    Signature { name: "get_map_name",                         module: "client.dll", needle: "48 83 EC ? 48 8B 0D ? ? ? ? ? ? ? FF 90 ? ? ? ? 48 8B C8 48 83 C4", resolve: NONE, extra_off: 0 },
    Signature { name: "get_view_model",                       module: "client.dll", needle: "40 55 53 56 41 56 41 57 48 8B EC", resolve: NONE, extra_off: 0 },
    // CSGOInput global: 48 8B 0D xx xx xx xx ; disp32 at +3, then post-resolve add 0x7
    Signature { name: "CSGOInput_resolved",                   module: "client.dll", needle: "48 8B 0D ? ? ? ? 8B 10 E8 ? ? ? ? 45 32 FF", resolve: RIPREL_3, extra_off: 7 },
    // CreateMaterial(material, name, kv3, ...) callsite prologue
    Signature { name: "CreateMaterial_caller",                module: "client.dll", needle: "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 8B F2", resolve: NONE, extra_off: 0 },
    Signature { name: "GetBonePositionByName",                module: "client.dll", needle: "40 53 48 83 EC ? 48 8B 89 ? ? ? ? 48 8B DA 48 8B 01 FF 50 ? 48 8B C8", resolve: NONE, extra_off: 0 },

    // ==================================================================
    // String-ref class anchors (resilient across patches: the schema
    // class name string survives even when surrounding bytes shift).
    // Internals use these to resolve weapon/player/HUD class vftables
    // without hand-maintaining byte patterns.
    // ==================================================================
    Signature { name: "C_BaseEntity",                          module: "client.dll", needle: "C_BaseEntity",                          resolve: STRREF, extra_off: 0 },
    Signature { name: "C_BaseModelEntity",                     module: "client.dll", needle: "C_BaseModelEntity",                     resolve: STRREF, extra_off: 0 },
    Signature { name: "C_BasePlayerPawn",                      module: "client.dll", needle: "C_BasePlayerPawn",                      resolve: STRREF, extra_off: 0 },
    Signature { name: "C_CSPlayerPawn",                        module: "client.dll", needle: "C_CSPlayerPawn",                        resolve: STRREF, extra_off: 0 },
    Signature { name: "C_CSPlayerPawnBase",                    module: "client.dll", needle: "C_CSPlayerPawnBase",                    resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayerController",                   module: "client.dll", needle: "CCSPlayerController",                   resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayerController_ActionTrackingServices", module: "client.dll", needle: "CCSPlayerController_ActionTrackingServices", resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayerController_DamageServices",    module: "client.dll", needle: "CCSPlayerController_DamageServices",    resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayerController_InGameMoneyServices", module: "client.dll", needle: "CCSPlayerController_InGameMoneyServices", resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayerController_InventoryServices", module: "client.dll", needle: "CCSPlayerController_InventoryServices", resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_BulletServices",              module: "client.dll", needle: "CCSPlayer_BulletServices",              resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_HostageServices",             module: "client.dll", needle: "CCSPlayer_HostageServices",             resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_PingServices",                module: "client.dll", needle: "CCSPlayer_PingServices",                resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_UseServices",                 module: "client.dll", needle: "CCSPlayer_UseServices",                 resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_WaterServices",               module: "client.dll", needle: "CCSPlayer_WaterServices",                resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_WeaponServices",              module: "client.dll", needle: "CCSPlayer_WeaponServices",               resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_MovementServices",            module: "client.dll", needle: "CCSPlayer_MovementServices",             resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSPlayer_MovementServices_Humanoid",   module: "client.dll", needle: "CCSPlayer_MovementServices_Humanoid",    resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSWeaponBase",                         module: "client.dll", needle: "CCSWeaponBase",                          resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSWeaponBaseGun",                      module: "client.dll", needle: "CCSWeaponBaseGun",                       resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSWeaponBaseVData",                    module: "client.dll", needle: "CCSWeaponBaseVData",                     resolve: STRREF, extra_off: 0 },
    Signature { name: "CSmokeGrenadeProjectile",               module: "client.dll", needle: "CSmokeGrenadeProjectile",                resolve: STRREF, extra_off: 0 },
    Signature { name: "CMolotovProjectile",                    module: "client.dll", needle: "CMolotovProjectile",                     resolve: STRREF, extra_off: 0 },
    Signature { name: "CFlashbangProjectile",                  module: "client.dll", needle: "CFlashbangProjectile",                   resolve: STRREF, extra_off: 0 },
    Signature { name: "CHEGrenadeProjectile",                  module: "client.dll", needle: "CHEGrenadeProjectile",                   resolve: STRREF, extra_off: 0 },
    Signature { name: "CDecoyProjectile",                      module: "client.dll", needle: "CDecoyProjectile",                       resolve: STRREF, extra_off: 0 },
    Signature { name: "C_PlantedC4",                           module: "client.dll", needle: "C_PlantedC4",                            resolve: STRREF, extra_off: 0 },
    Signature { name: "C_C4",                                  module: "client.dll", needle: "C_C4",                                   resolve: STRREF, extra_off: 0 },
    Signature { name: "C_Hostage",                             module: "client.dll", needle: "C_Hostage",                              resolve: STRREF, extra_off: 0 },
    Signature { name: "C_Inferno",                             module: "client.dll", needle: "C_Inferno",                              resolve: STRREF, extra_off: 0 },
    Signature { name: "C_SmokeGrenadeProjectile",              module: "client.dll", needle: "C_SmokeGrenadeProjectile",               resolve: STRREF, extra_off: 0 },
    Signature { name: "C_RecipientFilter",                     module: "client.dll", needle: "C_RecipientFilter",                      resolve: STRREF, extra_off: 0 },
    Signature { name: "CGameSceneNode",                        module: "client.dll", needle: "CGameSceneNode",                         resolve: STRREF, extra_off: 0 },
    Signature { name: "CSkeletonInstance",                     module: "client.dll", needle: "CSkeletonInstance",                      resolve: STRREF, extra_off: 0 },
    Signature { name: "CBodyComponent",                        module: "client.dll", needle: "CBodyComponent",                         resolve: STRREF, extra_off: 0 },
    Signature { name: "CBodyComponentSkeletonInstance",        module: "client.dll", needle: "CBodyComponentSkeletonInstance",         resolve: STRREF, extra_off: 0 },
    Signature { name: "CGlowProperty",                         module: "client.dll", needle: "CGlowProperty",                          resolve: STRREF, extra_off: 0 },
    Signature { name: "CCollisionProperty",                    module: "client.dll", needle: "CCollisionProperty",                     resolve: STRREF, extra_off: 0 },
    Signature { name: "CWeaponCSBase",                         module: "client.dll", needle: "CWeaponCSBase",                          resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSGameRules",                          module: "client.dll", needle: "CCSGameRules",                           resolve: STRREF, extra_off: 0 },
    Signature { name: "CCSGameRulesProxy",                     module: "client.dll", needle: "CCSGameRulesProxy",                      resolve: STRREF, extra_off: 0 },
    Signature { name: "CSGameRulesObjectives",                 module: "client.dll", needle: "CSGameRulesObjectives",                  resolve: STRREF, extra_off: 0 },

    // engine2.dll string-ref anchors
    Signature { name: "CNetworkGameClient",                    module: "engine2.dll", needle: "CNetworkGameClient",                    resolve: STRREF, extra_off: 0 },
    Signature { name: "CNetworkGameServer",                    module: "engine2.dll", needle: "CNetworkGameServer",                    resolve: STRREF, extra_off: 0 },
    Signature { name: "CGameEventManager",                     module: "engine2.dll", needle: "CGameEventManager",                     resolve: STRREF, extra_off: 0 },
    Signature { name: "CSplitScreenSlot",                      module: "engine2.dll", needle: "CSplitScreenSlot",                      resolve: STRREF, extra_off: 0 },

    // ==================================================================
    // NUVORA APR-26-2026 EXPANSION  v0.6.0
    // ------------------------------------------------------------------
    // Functions reverse-engineered while building the CS2 cosmetics +
    // third-person internals on build 14154.  Every pattern below was
    // verified UNIQUE (single match) on client.dll 14154 via IDA Pro,
    // captured at the function entry, and uses literal byte runs that
    // are structurally stable (instruction selectors, not register
    // colourings).  These are the exact functions our internal calls
    // into directly — community use cases include drop-in skin/glove
    // changers and a console-quality third-person camera.
    // ==================================================================

    // -- Cosmetics: skin / knife / glove direct-call pipeline ----------
    // RegenerateWeaponSkin(weapon, bForce) — 0x18078C050 on 14154.
    // Bypasses the bulk-iterator gate at weapon+0xAA8/+0xAC0 so a
    // cheat can apply skins without spawning a fake EconItem first.
    Signature {
        name: "RegenerateWeaponSkin_v2",
        module: "client.dll",
        needle: "40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8",
        resolve: NONE,
        extra_off: 0,
    },
    // GloveApply orchestrator — sub_180BBFAA0 — runs every spawner tick
    // inside sub_180BC2620.  Reads m_EconGloves (embedded view at
    // pawn+0x1658), checks m_bNeedToReApplyGloves @ pawn+0x1655,
    // destroys old C_WorldModelGloves, spawns new bonemerged glove
    // entity with paint params written from the embedded view.
    Signature {
        name: "GloveApply_PerTick",
        module: "client.dll",
        needle: "40 55 56 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B B9 A0 00 00 00",
        resolve: NONE,
        extra_off: 0,
    },
    // Spawner orchestrator — sub_180BC2620 — checks pawn+0x13B1 each
    // tick to drive GloveApply_PerTick.  Useful for entities/loadout
    // refresh hooks.
    Signature {
        name: "Spawner_PerTickOrchestrator",
        module: "client.dll",
        needle: "48 8B C4 55 53 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 80 B9 B1 13 00 00 00",
        resolve: NONE,
        extra_off: 0,
    },
    // Bulk regen iterator — sub_18078E320 — iterates all C_CSWeaponBase,
    // gates per-weapon on weapon[0xAA8] || weapon[0xAC0], calls
    // RegenerateWeaponSkin.  Knowing this gate is the reason custom
    // skins are silently dropped without setting the fallback fields.
    Signature {
        name: "BulkRegenIterator",
        module: "client.dll",
        needle: "57 48 83 EC 40 0F B6 F9 E8 ? ? ? ? 48 85 C0 0F 84",
        resolve: NONE,
        extra_off: 0,
    },

    // -- Third-person: native ConCommand handlers ----------------------
    // sub_180AC8BD0 — `thirdperson` ConCommand handler.  Calling this
    // directly is identical to typing `thirdperson` in console: sets
    // CInput+0x229=1, seeds camera anchor at CInput+0x230..+0x238,
    // calls localPawn->vtable[+0x9C8](true) so the local pawn
    // renders.  ConCommand flags = 0x20002080 (NOT FCVAR_CHEAT).
    Signature {
        name: "ConCommand_thirdperson",
        module: "client.dll",
        needle: "48 83 EC 38 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 0F 85 ? ? ? ? 4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00",
        resolve: NONE,
        extra_off: 0,
    },
    // sub_180AC8AF0 — `firstperson` ConCommand handler.  Sister of the
    // above: clears CInput+0x229, calls localPawn->vtable[+0x9C8](false)
    // and broadcasts viewmodel/HUD reset.
    Signature {
        name: "ConCommand_firstperson",
        module: "client.dll",
        needle: "48 83 EC 28 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ? 00 75 ? 48 8B 05 ? ? ? ? C6 80 29 02 00 00 00 C7 80 A8 06 00 00 00",
        resolve: NONE,
        extra_off: 0,
    },
    // CInput global pointer slot — `off_1820613C0` in IDA.  This is the
    // ACTUAL CInput pointer (deref the qword at this RVA).  The
    // cs2-dumper `dwCSGOInput = 0x23386E0` value points to a separate
    // C_Item entity reservoir and is NOT useful for camera flags.
    // RIP-relative load pattern; rel32 disp lives at +3 of the match.
    Signature {
        name: "CInputPtrGlobal",
        module: "client.dll",
        needle: "4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00 85 C0",
        resolve: RIPREL_3,
        extra_off: 0,
    },

    // ==================================================================
    // NUVORA APR-25-2026 EXPANSION  (build 14154 audit pass)
    // ------------------------------------------------------------------
    // Functions hooked / referenced by the live internal that were
    // missing from the dumper catalog. All UNIQUE on client.dll 14154,
    // verified via 8-instance IDA Pro MCP (ports 13337-13344).
    // ==================================================================

    // GetWorldFov resolver — sub_18080BE50. Renderer's actual FOV-read
    // entry point. Replaces the now-dead `SetWorldFov` E8 callsite
    // (which has 0 hits on 14154 — see comment block at top of section
    // around line 57). The resolver:
    //   1. Honours fov_cs_debug ConVar override.
    //   2. Calls camera-services vfunc[33] for base world FOV.
    //   3. Applies weapon-zoom / desired-FOV math.
    //   4. Returns final view FOV float.
    // Hooking here is the clean way to globally override view FOV
    // without writing m_iFOV every tick. Pattern keys on prologue +
    // distinctive tail-call jmp opcode that wraps the cleanup.
    Signature {
        name: "GetWorldFovResolver",
        module: "client.dll",
        needle: "40 53 48 83 EC 50 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 48 83 C4 50 5B E9",
        resolve: NONE,
        extra_off: 0,
    },

    // CCSGOInput::WriteSubtickFromEntry — sub_180C53DB0 (drifted to
    // 0x180C53E10 on 14154 — sig auto-resolves either way). Per-subtick
    // writer that copies CInput entry+0x10..+0x18 (view angles) and
    // entry+0x1C..+0x24 (shoot angles) into outgoing CUserCmd subtick
    // message fields. Hooked by the silent-aim path to redirect BOTH
    // view and shoot blocks per subtick (server uses shoot angles for
    // hit-verification — view-only rewrite leaves bullets going to the
    // original aim direction).
    Signature {
        name: "WriteSubtickFromEntry",
        module: "client.dll",
        needle: "48 89 5C 24 ? 55 57 41 56 48 8D 6C 24 ? 48 81 EC B0 00 00 00 8B 01 48 8B F9 81 4A 10 00 02",
        resolve: NONE,
        extra_off: 0,
    },

    // ClientModeCSNormal::OnEvent — sub_180C5A0B0 (drifted +0x60 to
    // 0x180C5A110 on 14154; sig still unique). The dispatcher CS2 uses
    // for VAC/untrusted disconnect events. Inspects KeyValues::GetName
    // and branches on:
    //   "OnClientInsecureBlocked"        — VAC kicked us
    //   "OnClientUntrustedLaunch"        — unsigned/injected module
    //   "OnClientUntrustedSystemFiles"   — tampered files / cheat
    //   "OnClientUntrustedDisallowed"    — disallowed launch
    //   "OnTrustedLaunchFailed"          — trusted-mode init failed
    //   "OnClientPureFileStateDirty"     — sv_pure mismatch
    // Hooked by the watchdog so cleanup runs BEFORE the dialog renders.
    Signature {
        name: "ClientModeCSNormal_OnEvent",
        module: "client.dll",
        needle: "40 53 57 48 81 EC 78 02 00 00 48 8B CA 48 8B FA",
        resolve: NONE,
        extra_off: 0,
    },

    // ==================================================================
    // NUVORA APR-25-2026 EXPANSION v2 (build 14155)
    // ------------------------------------------------------------------
    // Cross-module deep-dive across 8 IDA instances (client/scenesystem
    // /engine2/networksystem/materialsystem2/resourcesystem
    // /animationsystem/rendersystemdx11). All UNIQUE on respective DLLs,
    // anchored from log-string xrefs (most resilient anchor type).
    // ==================================================================

    // -- client.dll: combat / world ------------------------------------

    // FX_FireBullets — sub_180C7BE80. Refs the "FX_FireBullets:
    // GetItemDefinition failed" / "GetWeaponEconDataFromItem failed"
    // / "GetCSWeaponDataFromItem failed" log strings. Single anchor
    // function for all client-side bullet trace effects (tracers,
    // impact decals, hit-feedback). Hook here for custom tracer or
    // bullet-impact features without touching the actual fire path.
    Signature {
        name: "FX_FireBullets",
        module: "client.dll",
        needle: "48 8B C4 4C 89 48 20 48 89 50 10 55 53 57 41 54 41 55 48 8D A8 58 FB FF FF 48 81 EC A0 05",
        resolve: NONE,
        extra_off: 0,
    },

    // DispatchSpawn caller — sub_1814D32A0. Iterates the pending-spawn
    // queue and calls per-entity DispatchSpawn vfunc. Refs the
    // "DispatchSpawn" string. Useful as a "wait until entity is
    // spawned" hook anchor for features that need post-spawn state
    // (e.g. inventory_changer applies after this fires).
    Signature {
        name: "DispatchSpawn_caller",
        module: "client.dll",
        needle: "4C 8B DC 55 56 48 83 EC 78 49 8B 68 08 48 8B F1 48 85 ED 0F 84 72 01 00 00",
        resolve: NONE,
        extra_off: 0,
    },

    // m_bNoClipEnabled OnChange — sub_1808ADB40. Schema OnChange
    // callback for noclip flag (loads m_bNoClipEnabled string by lea
    // immediately at prologue). Useful if implementing client-side
    // noclip for spectator demos / map exploration.
    Signature {
        name: "NoClipOnChange",
        module: "client.dll",
        needle: "48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 48 8B EC 48 83 EC 30 48 8D 05",
        resolve: NONE,
        extra_off: 0,
    },

    // SpectatorInput — sub_1807D9130. Refs "spec_next" / "spec_prev"
    // / "spec_player %d" — handles the spec_* ConCommand input. Useful
    // for spectator-list UI / coach-cam features.
    Signature {
        name: "SpectatorInput",
        module: "client.dll",
        needle: "48 89 5C 24 10 55 56 57 41 56 41 57 48 8B EC 48 83 EC 60 48 8B 01 41 8B F8 48 8B DA 48 8B F1 FF",
        resolve: NONE,
        extra_off: 0,
    },

    // ViewModel HideZoomed handler — sub_1807A03D0. Refs
    // m_bHideViewModelWhenZoomed; the function that gates viewmodel
    // visibility on zoom state. Hook to force viewmodel-on-while-scoped
    // (or always-hidden viewmodel for a clean screen).
    Signature {
        name: "ViewModelHideZoomed",
        module: "client.dll",
        needle: "48 89 5C 24 20 55 56 57 41 54 41 56 48 8B EC 48 83 EC 50 48 8D 05",
        resolve: NONE,
        extra_off: 0,
    },

    // CalcViewmodelTransform v2 — sub_1807A2460. The much larger
    // (0x1E8E byte) viewmodel-transform calculator. Hook for
    // viewmodel offset / FOV / hand-position customisation.
    Signature {
        name: "CalcViewmodelTransform_v2",
        module: "client.dll",
        needle: "48 89 5C 24 20 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 80 48 81 EC 80 01 00 00 48 8B FA",
        resolve: NONE,
        extra_off: 0,
    },

    // -- engine2.dll ---------------------------------------------------

    // RegisterConCommand impl — engine2!sub_1803FD270. Refs the
    // "RegisterConCommand: Unknown error..." log string. The function
    // to call directly to register a custom ConCommand from inside a
    // cheat (lets you bind cheat features to console commands).
    Signature {
        name: "Engine_RegisterConCommand",
        module: "engine2.dll",
        needle: "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 44 8B 15",
        resolve: NONE,
        extra_off: 0,
    },

    // Client-side Disconnect_main — engine2!sub_1801D1510. Primary
    // disconnect handler (0x751 bytes), refs "disconnect" string and
    // "CL: SendStringCmd(disconnect)". Used by VAC watchdog as a
    // clean-eject path: invoke this before tearing down hooks so the
    // game cleanly reports "user disconnected" instead of crashing.
    Signature {
        name: "Engine_Disconnect_main",
        module: "engine2.dll",
        needle: "48 89 5C 24 20 55 57 41 54 48 8B EC 48 83 EC 70 45 33 E4 48 C7 05",
        resolve: NONE,
        extra_off: 0,
    },

    // Netchan timeout disconnect — engine2!sub_180069780. Refs
    // "CL: Disconnected - Client delta ticks out of order!". Fires
    // when the netchan detects desync. Hook to suppress / log these
    // events (anti-VAC heuristic — desyncs from cheat hooks can
    // trigger this).
    Signature {
        name: "Engine_NetTimeoutDisconnect",
        module: "engine2.dll",
        needle: "40 53 55 56 57 41 56 48 81 EC 80 00 00 00 0F 29 74 24 70 49 8B F8",
        resolve: NONE,
        extra_off: 0,
    },

    // -- networksystem.dll ---------------------------------------------

    // CNetChan::ProcessMessages impl — networksystem!sub_1800BB280.
    // Refs "CNetChan::ProcessMessages" log string. NOTE: distinct from
    // the client.dll string-anchored entry of the same name which
    // resolves a thunk; this is the actual impl. Hook here for
    // wholesale net-message inspection (anti-cheat detection bypass /
    // per-message logging / message-drop).
    Signature {
        name: "NetSystem_CNetChan_ProcessMessages",
        module: "networksystem.dll",
        needle: "48 8B C4 53 57 41 54 41 56 48 81 EC A8 00 00 00 48 89 70 D0 45 33 E4 4C 89 68 C8 48 8B D9 48 89",
        resolve: NONE,
        extra_off: 0,
    },

    // CNetChan::SendNetMessage impl — networksystem!sub_1800BD670.
    // Refs all 3 SendNetMessage error log strings ("invalid category
    // for this channel", "buffer is full", "SerializeAbstract failed").
    // Send-side counterpart for outgoing protobuf monitoring/blocking.
    Signature {
        name: "NetSystem_CNetChan_SendNetMessage",
        module: "networksystem.dll",
        needle: "48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 41 0F B6 F0 48 8D 99 F8 73 00 00 4C 8B F2",
        resolve: NONE,
        extra_off: 0,
    },

    // ==================================================================
    // NUVORA APR-26-2026 EXPANSION v3 (build 14155 — feature-focused)
    // ------------------------------------------------------------------
    // Sigs hand-picked for direct utility in internal/external feature
    // code: usercmd processing, world traces, local-player accessors,
    // viewmodel calc, console-command/cvar registration, the engine
    // command-buffer dispatcher, host-state transitions, the material
    // shader-loader, and the DX11 device entry points (SetMode / Present
    // wait / gamma ramp). All anchored from log strings (most stable
    // across CS2 patches) and verified UNIQUE on respective DLLs.
    // ==================================================================

    // -- client.dll: combat/usercmd/world ------------------------------

    // RunCommand processor — sub_1809DA390. Refs the per-tick log
    // "runcommand:%04d,tick:%u". Single anchor for the function CS2
    // calls *before* prediction runs each subtick — useful as the
    // canonical entry to inject anti-aim / fake-lag manipulation
    // because angles in m_pCmd are still mutable here.
    Signature {
        name: "RunCommand_processor",
        module: "client.dll",
        needle: "48 8B C4 48 81 EC C8 00 00 00 48 89 58 10 48 89 68 18 48 8B EA 48 89 70 20 48 8B F1 48 89 78 F8",
        resolve: NONE,
        extra_off: 0,
    },

    // TraceShape (Client) — sub_18098D340. Refs the
    // "Physics/TraceShape (Client)" perfetto category. The client-side
    // raycast/sweep entry every visibility check funnels through.
    // ESSENTIAL for: aimbot visibility filter, autowall, no-spread
    // bullet path simulation, prediction-aware grenade lines.
    Signature {
        name: "TraceShape_Client",
        module: "client.dll",
        needle: "48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D AC 24 10 E0 FF FF B8 F0 20 00 00",
        resolve: NONE,
        extra_off: 0,
    },

    // GetLocalPlayer accessor — sub_180379150. Refs *both*
    // "GetLocalPlayerPawn" and "GetLocalPlayerController" interface
    // export strings (single dispatcher). The cleanest, version-stable
    // way to fetch the local controller/pawn without poking the entity
    // list directly. Hook target also lets you "spoof" who is local
    // for spectator-cam / replay tooling.
    Signature {
        name: "GetLocalPlayer_dispatcher",
        module: "client.dll",
        needle: "48 83 EC 38 48 8B 05 ? ? ? ? 48 85 C0 0F 85 14 06 00 00 48 89 5C 24 40 B9 50 00 00 00 48 89",
        resolve: NONE,
        extra_off: 0,
    },

    // GetPlayerByIndex export — sub_180F02D60. Refs the
    // "GetPlayerByIndex" interface export string. Server-authoritative
    // controller lookup by entity index (1..maxclients). Useful for
    // ESP/aimbot enumeration when you don't want to walk the entity
    // list raw.
    Signature {
        name: "GetPlayerByIndex_export",
        module: "client.dll",
        needle: "48 83 EC 28 4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 4C 8D",
        resolve: NONE,
        extra_off: 0,
    },

    // CalcViewmodelView — sub_180C699D0. Reads m_flFOVSensitivityAdjust
    // and the viewmodel_offset_{x,y,z} convars to build the viewmodel
    // transform. Hook target for: viewmodel FOV override (to match
    // world FOV without the sensitivity penalty), custom viewmodel
    // position, "true left-hand" mode swap.
    Signature {
        name: "CalcViewmodelView",
        module: "client.dll",
        needle: "40 53 48 83 EC 60 48 8B 41 08 49 8B D8 8B 48 30 48 C1 E9 0C F6 C1 01 0F 85 48 01 00 00 41 B8 07",
        resolve: NONE,
        extra_off: 0,
    },

    // -- engine2.dll: cvar / command / host-state ----------------------

    // RegisterConVar impl — engine2!sub_1803FC080. Refs the
    // "RegisterConVar: Unknown error registering convar" log. Direct
    // entry for *registering* a custom ConVar from inside a cheat
    // (mirrors RegisterConCommand). Combine with the existing
    // Engine_RegisterConCommand sig to give your menu console-bindable
    // settings.
    Signature {
        name: "Engine_RegisterConVar",
        module: "engine2.dll",
        needle: "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC D0 00 00",
        resolve: NONE,
        extra_off: 0,
    },

    // CHLTVClient::ExecuteStringCommand — engine2!sub_180120D70. Refs
    // "CHLTVClient::ExecuteStringCommand: Unknown command %s.". The
    // server-side string-command dispatcher used while in HLTV/GOTV
    // demo playback. Useful for replay/demo tooling, and for pushing
    // custom HLTV commands (e.g. forced spec-target switch).
    Signature {
        name: "Engine_HLTVClient_ExecuteStringCommand",
        module: "engine2.dll",
        needle: "40 53 56 48 81 EC 48 07 00 00 48 8B F1 48 8B DA 48 8B 4A 48 48 83 E1 FC 48 83 79 18 0F 76 03 48",
        resolve: NONE,
        extra_off: 0,
    },

    // CHostStateMgr::QueueNewRequest — engine2!sub_18021AFC0. Refs
    // the "CHostStateMgr::QueueNewRequest( %s, %u )" log. Single entry
    // for transitioning host state (HSR_GAME / HSR_QUIT / HSR_IDLE /
    // HSR_SOURCETV_RELAY). Useful for VAC watchdog clean-eject (queue
    // HSR_IDLE before unhooking) and for "rejoin last server" / map
    // change automation.
    Signature {
        name: "Engine_HostStateMgr_QueueNewRequest",
        module: "engine2.dll",
        needle: "48 89 6C 24 18 48 89 7C 24 20 41 56 48 83 EC 30 48 8B EA 48 8B F9 8B 0D ? ? ? ? BA 02 00 00",
        resolve: NONE,
        extra_off: 0,
    },

    // -- materialsystem2.dll -------------------------------------------

    // CMaterial2::LoadShadersAndSetupModes — materialsystem2!
    // sub_180010040. Refs the "Error creating shader %s for material
    // %s!" log block (multiple anchors). The function CS2 calls when
    // a material is loaded and its shader passes are compiled.
    // Useful for: chams (swap shader at load time so the override is
    // baked-in instead of swapped per-frame), shader-error diagnosis,
    // forcing a specific shader fallback for unsupported materials.
    Signature {
        name: "CMaterial2_LoadShadersAndSetupModes",
        module: "materialsystem2.dll",
        needle: "44 89 44 24 18 48 89 54 24 10 53 56 41 54 41 55 48 81 EC 88 00 00 00 4C 8B E9 48 C7 44 24 60",
        resolve: NONE,
        extra_off: 0,
    },

    // -- rendersystemdx11.dll ------------------------------------------

    // CRenderDeviceDx11::SetMode — rendersystemdx11!sub_1800399E0.
    // Refs "CRenderDeviceDx11::SetMode: Previous mode has not been
    // shut down!". The HUGE (0x2183 byte) function that
    // creates/recreates the swapchain + RTVs whenever resolution /
    // fullscreen / refresh-rate changes. Hook target to react to
    // resolution change (re-init D3D11 ImGui backend, recreate cham
    // textures sized to the new RT).
    Signature {
        name: "RenderSystemDx11_SetMode",
        module: "rendersystemdx11.dll",
        needle: "44 89 4C 24 20 44 89 44 24 18 89 54 24 10 55 53 56 57 41 54 41 55 41 56 41 57 48 81 EC D8 02 00",
        resolve: NONE,
        extra_off: 0,
    },

    // CSwapChainBase::QueuePresentAndWait — rendersystemdx11!
    // sub_180034650. Refs the "CSwapChainBase::QueuePresentAndWait()
    // looped for %d iterations without a present event" warning.
    // The wrapper around IDXGISwapChain::Present that CS2 actually
    // funnels every frame through. PRIMARY hook anchor for ImGui
    // overlay / external menu rendering when not hooking the vtable
    // directly. Fires exactly once per frame.
    Signature {
        name: "RenderSystemDx11_QueuePresentAndWait",
        module: "rendersystemdx11.dll",
        needle: "40 55 53 57 41 54 41 55 48 8D 6C 24 C9 48 81 EC C0 00 00 00 48 8D 05 ? ? ? ? 4C 89 B4 24",
        resolve: NONE,
        extra_off: 0,
    },

    // CRenderDeviceDx11::SetHardwareGammaRamp — rendersystemdx11!
    // sub_18003F790. Refs "CRenderDeviceDx11::SetHardwareGammaRamp:
    // Unable to set gamma controls!". Lets you set/read the live
    // gamma ramp — useful for nightmode/wallhack-style global
    // brightness boost without touching shaders, and for restoring
    // gamma cleanly on detach.
    Signature {
        name: "RenderSystemDx11_SetHardwareGammaRamp",
        module: "rendersystemdx11.dll",
        needle: "48 89 5C 24 18 57 B8 B0 40 00 00 E8 ? ? ? ? 48 2B E0 0F 29 BC 24 90 40 00 00 0F 57 C9 0F 28",
        resolve: NONE,
        extra_off: 0,
    },
];
