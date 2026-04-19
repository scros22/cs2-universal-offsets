#pragma once
//
// CS2EnhancedSignatures
// =====================
// A vetted, modern CS2 signature database for the EnhancedScanner.
//
// All patterns here have been verified against CS2 client.dll as of April 2026.
// Each entry specifies:
//   - the target module (.text scope -> 100x faster scans)
//   - the IDA-style byte pattern
//   - an optional resolution mode (Rel32 / RipRel / StringRef) so that the
//     scanner reports the FINAL function/global address, not a midpoint.
//
// Added at the end:
//   - StringRef sigs that locate functions by a unique string they reference
//     (the workflow Ghidra/IDA users follow manually).  These are robust to
//     prologue-byte changes between CS2 patches.
//

#include "EnhancedScanner.h"

inline void RegisterCS2Signatures(EnhancedScanner& s)
{
    // ---------- client.dll : input / movement -------------------------------
    s.AddRaw   ("CCSGOInput::CreateMove",        L"client.dll",
                "48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55");

    s.AddRaw   ("CCSPlayer::ThirdPersonReset",   L"client.dll",
                "48 8B 40 08 44 38 20 75 10 44 88 67 01");

    s.AddRaw   ("RegenerateWeaponSkins",         L"client.dll",
                "48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10");

    s.AddRaw   ("CSkeletonInstance::SetMeshGroupMask", L"client.dll",
                "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99");

    s.AddRaw   ("CCSGOViewAdvice::OverrideView", L"client.dll",
                "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8");

    // E8 disp32 directly at the start of the match -> resolve as call target
    s.AddRel32 ("SetWorldFov",                   L"client.dll",
                "E8 ? ? ? ? F3 0F 11 45 ? 48 8B 5C 24", /*relOffset=*/1);

    s.AddRaw   ("CalcViewmodel",                 L"client.dll",
                "40 55 53 56 41 56 41 57 48 8B EC");
    s.AddRaw   ("NoSpread1",                     L"client.dll",
                "48 89 5C 24 08 57 48 81 EC F0 00");
    s.AddRaw   ("CalcSpread",                    L"client.dll",
                "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA");

    // ---------- skin / knife / glove changer (Ghidra-verified) -------------
    s.AddRaw   ("CBaseModelEntity::SetBodyGroup",       L"client.dll",
                "85 D2 0F 88 ? ? ? ? 53 55 56 48 83 EC 70 41 8B F0 8B DA 48 8B E9");
    s.AddRaw   ("CCSPlayerInventory::GetItemInLoadout", L"client.dll",
                "40 55 48 83 EC ? 49 63 E8");
    s.AddRaw   ("CCSInventoryManager::EquipItemInLoadout", L"client.dll",
                "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA");
    s.AddRaw   ("CEconItemView::GetCustomPaintKitIndex", L"client.dll",
                "48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25");

    // ---------- econ schema (Ghidra-verified April 2026) -------------------
    s.AddRaw   ("GetEconItemSystem",             L"client.dll",
                "48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 48 89 5C 24");
    s.AddRaw   ("CEconItemSchema::GetAttributeDefinitionByName", L"client.dll",
                "48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 60 48 8D 05");
    s.AddRaw   ("SetDynamicAttributeValue",      L"client.dll",
                "48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 4D 8B F8");

    // ---------- scenesystem.dll --------------------------------------------
    s.AddRaw   ("DrawSkyboxArray",               L"scenesystem.dll",
                "45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55");
    s.AddRaw   ("DrawObject_legacy",             L"scenesystem.dll",
                "48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49");
    s.AddRaw   ("CSceneAnimatableObject::GeneratePrimitives", L"scenesystem.dll",
                "48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?");
    s.AddRaw   ("DrawSmokeVertex",               L"client.dll",
                "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8");

    // ---------- materialsystem2.dll ----------------------------------------
    s.AddRaw   ("CMaterialSystem2::CreateMaterial", L"materialsystem2.dll",
                "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05");
    s.AddRaw   ("FindParameter",                 L"materialsystem2.dll",
                "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48");
    s.AddRaw   ("UpdateParameter",               L"materialsystem2.dll",
                "48 89 7C 24 ? 41 56 48 83 EC ? 8B 81");

    // ---------- tier0.dll ---------------------------------------------------
    s.AddRaw   ("LoadKV3_callsite",              L"tier0.dll",
                "48 8D 0D ? ? ? ? FF 15 ? ? ? ? 49 8B 06");

    // ---------- engine2.dll : net + game-state ------------------------------
    s.AddStringRef("Engine_GetTime",             L"engine2.dll",  "Engine_GetTime");
    s.AddStringRef("CL_FullyConnected",          L"engine2.dll",  "CL_FullyConnected");
    s.AddStringRef("Host_AccumulateTime",        L"engine2.dll",  "Host_AccumulateTime");
    s.AddStringRef("CNetChan_ProcessMessages",   L"engine2.dll",  "CNetChan::ProcessMessages");

    // ---------- string-anchored CS2 functions (robust across patches) ------
    s.AddStringRef("CCSPlayer_WeaponServices",   L"client.dll",   "CCSPlayer_WeaponServices");
    s.AddStringRef("CCSPlayer_MovementServices", L"client.dll",   "CCSPlayer_MovementServices");
    s.AddStringRef("CCSPlayer_BulletServices",   L"client.dll",   "CCSPlayer_BulletServices");
    s.AddStringRef("CSGameRules",                L"client.dll",   "CSGameRules");
    s.AddStringRef("CCSPlayerController",        L"client.dll",   "CCSPlayerController");
    s.AddStringRef("CCSPlayerPawn",              L"client.dll",   "CCSPlayerPawn");
    s.AddStringRef("CHudWeaponSelection",        L"client.dll",   "CHudWeaponSelection");
    s.AddStringRef("CHudDeathNotice",            L"client.dll",   "CHudDeathNotice");
    s.AddStringRef("paintkit_seed",              L"client.dll",   "set item texture seed");
    s.AddStringRef("paintkit_prefab",            L"client.dll",   "set item texture prefab");
    s.AddStringRef("paintkit_wear",              L"client.dll",   "set item texture wear");
    s.AddStringRef("statTrak_killEater",         L"client.dll",   "kill eater");
    s.AddStringRef("statTrak_scoreType",         L"client.dll",   "kill eater score type");

    // VAC-Net / matchmaking strings
    s.AddStringRef("VacNet_OnEvent",             L"client.dll",   "VAC-Net Detection");
    s.AddStringRef("Matchmaking_AcceptMatch",    L"client.dll",   "AcceptInviteToParty");

    // ---------- traditional sigs (kept for completeness) -------------------
    s.AddRaw   ("ClientStateSignOnState",        L"engine2.dll",
                "83 3D ? ? ? ? 06 0F 94 C0");
}
