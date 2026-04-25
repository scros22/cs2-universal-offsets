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

    // RegenerateWeaponSkin — sub_18078C050 (RVA 0x78C050, current build,
    // IDA-verified 2026-04-25). Call directly with (weapon, false) AFTER
    // writing m_nFallbackPaintKit/Seed/Wear/StatTrak and m_iItemIDHigh.
    // OLD sig (48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10) is
    // DEAD on current build — it was a wrapper that no longer exists.
    constexpr const char* RegenWeaponSkins =
        "40 55 53 41 57 48 8D AC 24 00 FE FF FF 48 81 EC";

    // SetMeshGroupMask — CSkeletonInstance method for fixing weapon mesh
    // rendering (sub_180A2C3F0, RVA 0xA2C3F0, IDA-verified 2026-04-25,
    // drift +0x60 from build 14154). Call on entity->m_pGameSceneNode.
    // Called on skeleton instance (entity + m_pGameSceneNode)
    // Param: uint64 mask — 2 = Legacy, 1 = !Legacy
    constexpr const char* SetMeshGroupMask =
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99";

    // OverrideView (CCSGOViewAdviceService) — camera FOV/angles
    // NOTE: This sig is DEAD on build 14154 (returns 0 hits). Kept for
    // archaeology. Third-person no longer relies on it — see
    // ThirdPersonOnHandler / ThirdPersonOffHandler below.
    constexpr const char* OverrideView =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8";

    // ConCommand handler for `thirdperson` (sub_180AC8C30 on current build,
    // was sub_180AC8BD0 on 14154 — drift +0x60).
    // Pattern keys on the `mov [r8+229h], 1` flag write + the unique
    // `mov dword [r8+6A8h], 0` transition reset that follows. Verified
    // unique across client.dll (IDA 2026-04-25).
    constexpr const char* ThirdPersonOnHandler =
        "48 83 EC 38 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 "
        "83 7C 24 ? 00 0F 85 ? ? ? ? 4C 8B 05 ? ? ? ? 41 8B 80 50 0B 00 00";

    // ConCommand handler for `firstperson` (sub_180AC8B50 on current build,
    // was sub_180AC8AF0 on 14154 — drift +0x60).
    // Sister of the above; keys on `mov [rax+229h], 0` + transition
    // reset. Verified unique across client.dll (IDA 2026-04-25).
    constexpr const char* ThirdPersonOffHandler =
        "48 83 EC 28 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 08 03 00 00 "
        "83 7C 24 ? 00 75 ? 48 8B 05 ? ? ? ? C6 80 29 02 00 00 00 "
        "C7 80 A8 06 00 00 00";

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

    // CCSPlayerInventory::GetItemInLoadout (sub_1807C3CB0, drift -0x40
    // from build 14154, IDA-verified 2026-04-25)
    constexpr const char* GetItemInLoadout =
        "40 55 48 83 EC ? 49 63 E8";

    // CCSInventoryManager::EquipItemInLoadout (sub_1807C2090, drift -0x40
    // from build 14154, IDA-verified 2026-04-25)
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
