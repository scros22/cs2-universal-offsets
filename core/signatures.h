#pragma once

// ---------------------------------------------------------------
// Signature patterns for hooks.
// Kept separate so they can be reviewed / patched in one place.
// ---------------------------------------------------------------

namespace Signatures
{
    // ----------------------------------------------------------------
    // client.dll
    // ----------------------------------------------------------------

    // CCSGOInput::CreateMove
    constexpr const char* CreateMove =
        "48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55";

    // Third-person camera reset branch
    constexpr const char* ThirdPersonReset =
        "48 8B 40 08 44 38 20 75 10 44 88 67 01";

    // RegenerateWeaponSkins
    constexpr const char* RegenWeaponSkins =
        "48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10";

    // SetMeshGroupMask — CSkeletonInstance method for fixing weapon mesh rendering
    // Called on skeleton instance (entity + m_pGameSceneNode)
    // Param: uint64 mask — 2 = Legacy, 1 = !Legacy
    constexpr const char* SetMeshGroupMask =
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99";

    // OverrideView (CCSGOViewAdviceService) — camera FOV/angles
    constexpr const char* OverrideView =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8";

    // SetWorldFov — E8-CALL to world-FOV setter
    constexpr const char* SetWorldFov =
        "E8 ? ? ? ? F3 0F 11 45 ? 48 8B 5C 24";

    // DrawSkyboxArray — scenesystem.dll, skybox render function
    constexpr const char* DrawSkyboxArray =
        "45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55";

    // DrawSmokeVertex — smoke particle rendering
    constexpr const char* DrawSmokeVertex =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 9C 24 ? ? ? ? 4D 8B F8";

    // CalcViewmodel — viewmodel animation/position
    constexpr const char* CalcViewmodel =
        "40 55 53 56 41 56 41 57 48 8B EC";

    // NoSpread1 — spread calculation entry
    constexpr const char* NoSpread1 =
        "48 89 5C 24 08 57 48 81 EC F0 00";

    // CalcSpread — full spread computation
    constexpr const char* CalcSpread =
        "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 63 EA";

    // ----------------------------------------------------------------
    // Skin / Knife / Glove changer (from UC thread, March 2026)
    // ----------------------------------------------------------------

    // CBaseEntity::SetBodyGroup — required for glove application
    constexpr const char* SetBodyGroup =
        "85 D2 0F 88 5C";

    // CCSPlayerInventory::GetItemInLoadout
    constexpr const char* GetItemInLoadout =
        "40 55 48 83 EC ? 49 63 E8";

    // CCSInventoryManager::EquipItemInLoadout
    constexpr const char* EquipItemInLoadout =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA";

    // CEconItemView::GetPaintKitIndex — resolves paint kit from econ item
    constexpr const char* GetPaintKitIndex =
        "48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25";

    // ----------------------------------------------------------------
    // scenesystem.dll
    // ----------------------------------------------------------------

    // DrawObject (legacy)
    constexpr const char* DrawObject =
        "48 8B C4 53 57 41 54 48 81 EC D0 00 00 00 49 63 F9 49";

    // GeneratePrimitives — CSceneAnimatableObject vtable[4], main chams hook point
    constexpr const char* GeneratePrimitives =
        "48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?";

    // ----------------------------------------------------------------
    // materialsystem2.dll
    // ----------------------------------------------------------------

    // CreateMaterial
    constexpr const char* CreateMaterial =
        "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05";

    // FindParameter — material param lookup
    constexpr const char* FindParameter =
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48";

    // UpdateParameter — material param update
    constexpr const char* UpdateParameter =
        "48 89 7C 24 ? 41 56 48 83 EC ? 8B 81";

    // ----------------------------------------------------------------
    // tier0.dll
    // ----------------------------------------------------------------

    // LoadKV3 call site
    constexpr const char* LoadKV3 =
        "48 8D 0D ? ? ? ? FF 15 ? ? ? ? 49 8B 06";
}
