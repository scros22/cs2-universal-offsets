#pragma once

// ---------------------------------------------------------------
// CS2 SDK Auto-Offset Layer
// Pulls all offsets directly from the cs2-dumper generated headers.
// To update: run cs2-dumper, rebuild. No manual editing required.
// ---------------------------------------------------------------

#include <cstddef>
#include <cstdint>

// Dumper-generated SDK headers
#include "../sdk/offsets.hpp"
#include "../sdk/client_dll.hpp"
#include "../sdk/buttons.hpp"

namespace Offsets
{
    // ---- Global offsets (auto-sourced from sdk/offsets.hpp) ----
    namespace Global
    {
        using namespace cs2_dumper::offsets::client_dll;
        using namespace cs2_dumper::offsets::engine2_dll;
    }

    // ---- Button addresses (auto-sourced from sdk/buttons.hpp) ----
    namespace Buttons
    {
        using namespace cs2_dumper::buttons;
    }

    // ---- Entity schema offsets ----
    // These are tied to CS2 build and should be re-confirmed with each
    // major update. Dumper generates schemas in client_dll.hpp but
    // we define them here as constants for quick access without
    // parsing the full schema namespace at every callsite.

    // C_BaseEntity
    constexpr std::ptrdiff_t m_pGameSceneNode      = 0x338;
    constexpr std::ptrdiff_t m_nSubclassID         = 0x388;
    constexpr std::ptrdiff_t m_hOwnerEntity        = 0x528;
    constexpr std::ptrdiff_t m_iHealth             = 0x354;
    constexpr std::ptrdiff_t m_lifeState           = 0x35C;
    constexpr std::ptrdiff_t m_iTeamNum            = 0x3F3;
    constexpr std::ptrdiff_t m_flSimulationTime    = 0x3C0;
    constexpr std::ptrdiff_t m_fFlags              = 0x400;

    // C_BasePlayerPawn
    constexpr std::ptrdiff_t m_pWeaponServices     = 0x13D8;
    constexpr std::ptrdiff_t m_vecViewOffset        = 0xD58;
    constexpr std::ptrdiff_t m_pObserverServices   = 0x13F0;
    constexpr std::ptrdiff_t m_pCameraServices     = 0x1410;

    // CPlayer_WeaponServices
    constexpr std::ptrdiff_t m_hMyWeapons          = 0x48;
    constexpr std::ptrdiff_t m_hActiveWeapon       = 0x60;

    // C_BaseCombatCharacter
    constexpr std::ptrdiff_t m_hMyWearables        = 0x1350;

    // CPlayer_ObserverServices
    constexpr std::ptrdiff_t m_hObserverTarget     = 0x4C;
    constexpr std::ptrdiff_t m_iObserverMode       = 0x48;

    // CCSPlayerController
    constexpr std::ptrdiff_t m_pInventoryServices  = 0x810;
    constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x860;
    constexpr std::ptrdiff_t m_hPlayerPawn         = 0x90C;
    constexpr std::ptrdiff_t m_bPawnIsAlive        = 0x914;
    constexpr std::ptrdiff_t m_hObserverPawn       = 0x910;

    // CCSPlayerController_InventoryServices
    constexpr std::ptrdiff_t m_unMusicID           = 0x58;

    // C_CSPlayerPawn
    constexpr std::ptrdiff_t m_bNeedToReApplyGloves = 0x188D;
    constexpr std::ptrdiff_t m_EconGloves          = 0x1890;
    constexpr std::ptrdiff_t m_hHudModelArms       = 0x2400;
    constexpr std::ptrdiff_t m_pClippingWeapon     = 0x3DC0;
    constexpr std::ptrdiff_t m_nEconGlovesChanged  = 0x1D00;
    constexpr std::ptrdiff_t m_iShotsFired         = 0x270C;
    constexpr std::ptrdiff_t m_aimPunchAngle       = 0x16CC;
    constexpr std::ptrdiff_t m_angEyeAngles        = 0x3DD0;
    constexpr std::ptrdiff_t m_ArmorValue          = 0x272C;
    constexpr std::ptrdiff_t m_bGunGameImmunity    = 0x3D74;
    constexpr std::ptrdiff_t m_iIDEntIndex         = 0x3EAC;
    constexpr std::ptrdiff_t m_flFlashDuration     = 0x15F8;
    constexpr std::ptrdiff_t m_flFlashMaxAlpha     = 0x15F4;
    constexpr std::ptrdiff_t m_flLastSmokeOverlayAlpha = 0x1618;
    constexpr std::ptrdiff_t m_bIsScoped           = 0x26F8;
    constexpr std::ptrdiff_t m_entitySpottedState   = 0x26E0;

    // C_BaseModelEntity — alpha property
    constexpr std::ptrdiff_t m_pClientAlphaProperty = 0xE38;

    // CClientAlphaProperty
    constexpr std::ptrdiff_t m_nAlpha              = 0x17;

    // CSmokeGrenadeProjectile
    constexpr std::ptrdiff_t m_nSmokeEffectTickBegin = 0x1450;
    constexpr std::ptrdiff_t m_bDidSmokeEffect     = 0x1454;
    constexpr std::ptrdiff_t m_vSmokeColor         = 0x145C;  // Vector (3 floats)
    constexpr std::ptrdiff_t m_bSmokeEffectSpawned = 0x1499;

    // C_Inferno
    constexpr std::ptrdiff_t m_fireCount           = 0x1838;
    constexpr std::ptrdiff_t m_nFireEffectTickBegin = 0x184C;

    // C_BaseEntity velocity
    constexpr std::ptrdiff_t m_vecVelocity         = 0x438;

    // CPlayer_CameraServices
    constexpr std::ptrdiff_t m_iFOV                = 0x290;   // uint32, NOT float
    constexpr std::ptrdiff_t m_iFOVStart           = 0x294;   // uint32
    constexpr std::ptrdiff_t m_flFOVTime           = 0x298;   // GameTime_t
    constexpr std::ptrdiff_t m_flFOVRate           = 0x29C;   // float32
    constexpr std::ptrdiff_t m_hZoomOwner          = 0x2A0;   // CHandle
    constexpr std::ptrdiff_t m_flLastShotFOV       = 0x2A4;   // float32

    // C_EnvSky
    constexpr std::ptrdiff_t m_vTintColor          = 0xE99;   // Color (RGBA bytes)
    constexpr std::ptrdiff_t m_vTintColorLightingOnly = 0xE9D; // Color
    constexpr std::ptrdiff_t m_flSkyBrightnessScale = 0xEA4;  // float32
    constexpr std::ptrdiff_t m_bSkyEnabled         = 0xEBC;   // bool (m_bEnabled on C_EnvSky)

    // CGameSceneNode
    constexpr std::ptrdiff_t m_vecAbsOrigin        = 0xD0;

    // C_BaseEntity — viewmodel detection
    constexpr std::ptrdiff_t m_bRenderWithViewModels = 0x572;

    // C_TonemapController2 — exposure control
    constexpr std::ptrdiff_t m_flAutoExposureMin   = 0x608;
    constexpr std::ptrdiff_t m_flAutoExposureMax   = 0x60C;

    // CSkeletonInstance
    constexpr std::ptrdiff_t m_materialGroup       = 0x434;
    constexpr std::ptrdiff_t m_modelState          = 0x160;

    // CModelState
    constexpr std::ptrdiff_t m_MeshGroupMask       = 0x220;
    constexpr std::ptrdiff_t m_BoneArray           = 0x80;

    // C_BaseModelEntity (render / visual)
    constexpr std::ptrdiff_t m_nRenderMode         = 0xB60;
    constexpr std::ptrdiff_t m_clrRender           = 0xB80;
    constexpr std::ptrdiff_t m_Glow                = 0xCC0;
    constexpr std::ptrdiff_t m_flGlowBackfaceMult  = 0xD18;
    constexpr std::ptrdiff_t m_ClientOverrideTint  = 0xE40;
    constexpr std::ptrdiff_t m_bUseClientOverrideTint = 0xE44;

    // CGlowProperty
    constexpr std::ptrdiff_t m_bGlowing            = 0x51;
    constexpr std::ptrdiff_t m_glowColorOverride   = 0x40;
    constexpr std::ptrdiff_t m_iGlowType           = 0x30;
    constexpr std::ptrdiff_t m_nGlowRange          = 0x38;
    constexpr std::ptrdiff_t m_nGlowRangeMin       = 0x3C;
    constexpr std::ptrdiff_t m_flGlowTime          = 0x48;
    constexpr std::ptrdiff_t m_flGlowStartTime     = 0x4C;

    // C_EconEntity
    constexpr std::ptrdiff_t m_AttributeManager    = 0x1378;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidLow  = 0x1848;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidHigh = 0x184C;
    constexpr std::ptrdiff_t m_nFallbackPaintKit   = 0x1850;
    constexpr std::ptrdiff_t m_nFallbackSeed       = 0x1854;
    constexpr std::ptrdiff_t m_flFallbackWear      = 0x1858;
    constexpr std::ptrdiff_t m_nFallbackStatTrak   = 0x185C;

    // C_AttributeContainer
    constexpr std::ptrdiff_t m_Item                = 0x50;

    // C_EconItemView
    constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
    constexpr std::ptrdiff_t m_iEntityQuality      = 0x1BC;
    constexpr std::ptrdiff_t m_iItemID             = 0x1C8;
    constexpr std::ptrdiff_t m_iItemIDHigh         = 0x1D0;
    constexpr std::ptrdiff_t m_iItemIDLow          = 0x1D4;
    constexpr std::ptrdiff_t m_iAccountID          = 0x1D8;
    constexpr std::ptrdiff_t m_bInitialized        = 0x1E8;
    constexpr std::ptrdiff_t m_bDisallowSOC       = 0x1E9;
    constexpr std::ptrdiff_t m_bRestoreCustomMaterialAfterPrecache = 0x1B8;
    constexpr std::ptrdiff_t m_AttributeList       = 0x208;
    constexpr std::ptrdiff_t m_NetworkedDynamicAttributes = 0x280;
    constexpr std::ptrdiff_t m_szCustomName        = 0x2F8;
    constexpr std::ptrdiff_t m_szCustomNameOverride = 0x399;

    // CAttributeList
    constexpr std::ptrdiff_t m_Attributes          = 0x8;

    // CEconItemAttribute
    constexpr std::ptrdiff_t m_iAttributeDefinitionIndex = 0x30;
    constexpr std::ptrdiff_t m_flValue             = 0x34;
    constexpr std::ptrdiff_t m_flInitialValue      = 0x38;
    constexpr std::ptrdiff_t m_nRefundableCurrency = 0x3C;
    constexpr std::ptrdiff_t m_bSetBonus           = 0x40;

    // C_PlantedC4
    constexpr std::ptrdiff_t m_bBombTicking        = 0x1170;
    constexpr std::ptrdiff_t m_flC4Blow            = 0x11A0;
    constexpr std::ptrdiff_t m_bBombDefused        = 0x11C4;
    constexpr std::ptrdiff_t m_flDefuseCountDown   = 0x11C0;
    constexpr std::ptrdiff_t m_flDefuseLength      = 0x11BC;
}
