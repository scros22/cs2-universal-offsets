#pragma once

#include <cstddef>
#include <cstdint>

// ============================================
// Updated offsets for CS2 (2026-04-03)
// ============================================

namespace Offsets
{
    // === Global Offsets (offsets.hpp - client.dll) ===
    constexpr std::ptrdiff_t dwEntityList = 0x24B3268;
    constexpr std::ptrdiff_t dwLocalPlayerController = 0x22F8028;
    constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x206D9E0;
    constexpr std::ptrdiff_t dwPlantedC4 = 0x231BAB0;
    constexpr std::ptrdiff_t dwViewAngles = 0x231E9B8;
    constexpr std::ptrdiff_t dwCSGOInput = 0x231E330;
    constexpr std::ptrdiff_t dwGameRules = 0x2311ED0;
    constexpr std::ptrdiff_t dwGlobalVars = 0x2062540;
    constexpr std::ptrdiff_t dwViewMatrix = 0x2313F10;
    constexpr std::ptrdiff_t dwGlowManager = 0x231CD38;
    constexpr std::ptrdiff_t dwGameEntitySystem_highestEntityIndex = 0x20A0;

    // === C_BaseEntity ===
    constexpr std::ptrdiff_t m_pGameSceneNode = 0x338;
    constexpr std::ptrdiff_t m_hOwnerEntity = 0x528;
    constexpr std::ptrdiff_t m_szDesignerName = 0x20; // in CEntityIdentity at +0x10

    // === C_BasePlayerPawn ===
    constexpr std::ptrdiff_t m_pWeaponServices = 0x13D8;
    constexpr std::ptrdiff_t m_pViewModelServices = 0x1368;

    // === C_Player_WeaponServices ===
    constexpr std::ptrdiff_t m_hMyWeapons = 0x48;
    constexpr std::ptrdiff_t m_hActiveWeapon = 0x60;
    constexpr std::ptrdiff_t m_fAccuracyPenalty = 0x19C0;
    constexpr std::ptrdiff_t m_flRecoilIndex = 0x19D0;
    constexpr std::ptrdiff_t m_iRecoilIndex = 0x19CC;

    // === CCSPlayerController ===
    constexpr std::ptrdiff_t m_pInventoryServices = 0x810;
    constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x860;
    constexpr std::ptrdiff_t m_hPlayerPawn = 0x90C;

    // === CCSPlayerController_InventoryServices ===
    constexpr std::ptrdiff_t m_unMusicID = 0x58;

    // === C_CSPlayerPawn ===
    constexpr std::ptrdiff_t m_bNeedToReApplyGloves = 0x188D;
    constexpr std::ptrdiff_t m_EconGloves = 0x1890;
    constexpr std::ptrdiff_t m_hHudModelArms = 0x2400;
    constexpr std::ptrdiff_t m_pClippingWeapon = 0x3DC0;
    constexpr std::ptrdiff_t m_nEconGlovesChanged = 0x1D00;
    constexpr std::ptrdiff_t m_aimPunchAngle = 0x16CC;
    constexpr std::ptrdiff_t m_aimPunchAngleVel = 0x16D8;
    constexpr std::ptrdiff_t m_flFlashMaxAlpha = 0x15F4;
    constexpr std::ptrdiff_t m_flFlashDuration = 0x1478;
    constexpr std::ptrdiff_t m_vViewPunchAngle = 0x173C;
    constexpr std::ptrdiff_t m_bIsScoped = 0x26F8;


    // === C_BaseModelEntity — Render overrides ===
    constexpr std::ptrdiff_t m_nRenderMode = 0xB60;
    constexpr std::ptrdiff_t m_clrRender = 0xB80;
    constexpr std::ptrdiff_t m_Glow = 0xCC0;
    constexpr std::ptrdiff_t m_flGlowBackfaceMult = 0xD18;
    constexpr std::ptrdiff_t m_ClientOverrideTint = 0xE40;
    constexpr std::ptrdiff_t m_bUseClientOverrideTint = 0xE44;

    // === CSkeletonInstance ===
    constexpr std::ptrdiff_t m_materialGroup = 0x434;
    constexpr std::ptrdiff_t m_modelState = 0x160;

    // === CModelState ===
    constexpr std::ptrdiff_t m_MeshGroupMask = 0x220;

    // === C_EconEntity ===
    constexpr std::ptrdiff_t m_AttributeManager = 0x1378;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidLow = 0x1848;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidHigh = 0x184C;
    constexpr std::ptrdiff_t m_nFallbackPaintKit = 0x1850;
    constexpr std::ptrdiff_t m_nFallbackSeed = 0x1854;
    constexpr std::ptrdiff_t m_flFallbackWear = 0x1858;
    constexpr std::ptrdiff_t m_nFallbackStatTrak = 0x185C;

    // === C_AttributeContainer ===
    constexpr std::ptrdiff_t m_Item = 0x50;

    // === C_EconItemView ===
    constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
    constexpr std::ptrdiff_t m_iEntityQuality = 0x1BC;
    constexpr std::ptrdiff_t m_iItemIDHigh = 0x1D0;
    constexpr std::ptrdiff_t m_iItemIDLow = 0x1D4;
    constexpr std::ptrdiff_t m_iAccountID = 0x1D8;
    constexpr std::ptrdiff_t m_bInitialized = 0x1E8;
    constexpr std::ptrdiff_t m_bRestoreCustomMaterialAfterPrecache = 0x1B8;
    constexpr std::ptrdiff_t m_AttributeList = 0x208;
    constexpr std::ptrdiff_t m_NetworkedDynamicAttributes = 0x280;
    constexpr std::ptrdiff_t m_szCustomName = 0x2F8;
    constexpr std::ptrdiff_t m_szCustomNameOverride = 0x399;

    // === CAttributeList ===
    constexpr std::ptrdiff_t m_Attributes = 0x8;

    // === CEconItemAttribute ===
    constexpr std::ptrdiff_t m_iAttributeDefinitionIndex = 0x30;
    constexpr std::ptrdiff_t m_flValue = 0x34;
    constexpr std::ptrdiff_t m_flInitialValue = 0x38;
    constexpr std::ptrdiff_t m_nRefundableCurrency = 0x3C;
    constexpr std::ptrdiff_t m_bSetBonus = 0x40;

    // === Aimbot / Entity Offsets ===
    constexpr std::ptrdiff_t m_iHealth = 0x354;
    constexpr std::ptrdiff_t m_lifeState = 0x35C;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3F3;
    constexpr std::ptrdiff_t m_vecAbsOrigin = 0xD0;
    constexpr std::ptrdiff_t m_vecViewOffset = 0xD58;
    constexpr std::ptrdiff_t m_BoneArray = 0x80;
    constexpr std::ptrdiff_t m_fFlags = 0x400;
    constexpr std::ptrdiff_t m_flSimulationTime = 0x3C0;
    constexpr std::ptrdiff_t m_bPawnIsAlive = 0x914;
    constexpr std::ptrdiff_t m_iDesiredFOV = 0x78C;

    // === Visuals (Bomb, Glow, Spectator) ===
    constexpr std::ptrdiff_t m_bBombTicking = 0x1170;
    constexpr std::ptrdiff_t m_flC4Blow = 0x11A0;
    constexpr std::ptrdiff_t m_flDefuseCountDown = 0x11C0;
    constexpr std::ptrdiff_t m_flDefuseLength = 0x11BC;

    constexpr std::ptrdiff_t m_hObserverPawn = 0x910;
    constexpr std::ptrdiff_t m_pObserverServices = 0x13F0;
    constexpr std::ptrdiff_t m_hObserverTarget = 0x4C;
    constexpr std::ptrdiff_t m_iObserverMode = 0x48;

    constexpr std::ptrdiff_t m_pCameraServices = 0x1410;
    constexpr std::ptrdiff_t m_iFOV = 0x290;
    constexpr std::ptrdiff_t m_iFOVStart = 0x294;
    constexpr std::ptrdiff_t m_flFOVRate = 0x29C;
    constexpr std::ptrdiff_t m_flFOVTime = 0x298;
    constexpr std::ptrdiff_t m_flViewmodelFOV = 0x2424;
    constexpr std::ptrdiff_t m_hViewModel = 0x40; // in CViewModelServices

    constexpr std::ptrdiff_t m_bGlowing = 0x51;
    constexpr std::ptrdiff_t m_glowColorOverride = 0x40;
    constexpr std::ptrdiff_t m_iGlowType = 0x30;

    constexpr int CCSGOInput_CreateMoveIdx = 5;

    // === Signature Patterns ===
    constexpr const char* sig_CreateMove_client = "48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55";
    constexpr const char* sig_ThirdPersonReset = "48 8B 40 08 44 38 20 75 10 44 88 67 01";
    constexpr const char* sig_DrawObject = "48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC";
    constexpr const char* sig_LoadKV3 = "48 8D 0D ? ? ? ? FF 15 ? ? ? ? 49 8B 06";
    constexpr const char* sig_CreateMaterial = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B F1 48 8B DA";
}
