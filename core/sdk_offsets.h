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

    // ---- Entity-system primitives (CGameEntitySystem layout) ----
    // These are reverse-engineered constants — not in the schema dump —
    // but stable across CS2 builds. Use these instead of bare magic
    // numbers so a future layout change can be patched in one place.
    namespace EntitySys
    {
        constexpr std::ptrdiff_t kChunkArrayBase   = 0x10;   // first chunk pointer slot
        constexpr std::ptrdiff_t kChunkPtrStride   = 0x8;    // sizeof(void*)
        constexpr std::ptrdiff_t kChunkEntryStride = 0x70;   // bytes between entity slots inside a chunk
        constexpr uint32_t       kSlotIndexMask    = 0x1FF;  // 512 entries per chunk
        constexpr uint32_t       kHandleIndexMask  = 0x7FFF; // CHandle index bits

        // Per-entity: CEntityInstance layout
        constexpr std::ptrdiff_t kInstanceToIdentity = 0x10; // CEntityInstance + 0x10 -> CEntityIdentity*
        // CEntityIdentity layout
        constexpr std::ptrdiff_t kIdentityDesignerName = 0x20; // CUtlSymbolLarge -> char*
    }

    // ---- CGameSceneNode hand-RE'd offsets (not in schema dump) ----
    namespace SceneNode
    {
        constexpr std::ptrdiff_t kDormant = 0x10B;  // bool m_bDormant (chams/ESP rely on this)
    }

    // ---- Entity schema offsets ----
    // These are tied to CS2 build and should be re-confirmed with each
    // major update. Dumper generates schemas in client_dll.hpp but
    // we define them here as constants for quick access without
    // parsing the full schema namespace at every callsite.

    // C_BaseEntity (build 14152)
    constexpr std::ptrdiff_t m_pGameSceneNode      = 0x330;
    constexpr std::ptrdiff_t m_nSubclassID         = 0x380;
    constexpr std::ptrdiff_t m_hOwnerEntity        = 0x520;
    constexpr std::ptrdiff_t m_iHealth             = 0x34C;
    constexpr std::ptrdiff_t m_lifeState           = 0x354;
    constexpr std::ptrdiff_t m_iTeamNum            = 0x3EB;
    constexpr std::ptrdiff_t m_flSimulationTime    = 0x3B8;
    constexpr std::ptrdiff_t m_fFlags              = 0x3F8;

    // C_BasePlayerPawn
    constexpr std::ptrdiff_t m_pWeaponServices     = 0x11E0;
    constexpr std::ptrdiff_t m_vecViewOffset        = 0xE70;
    constexpr std::ptrdiff_t m_pObserverServices   = 0x11F8;
    constexpr std::ptrdiff_t m_pCameraServices     = 0x1218;

    // CPlayer_WeaponServices
    constexpr std::ptrdiff_t m_hMyWeapons          = 0x48;
    constexpr std::ptrdiff_t m_hActiveWeapon       = 0x60;

    // C_BaseCombatCharacter
    constexpr std::ptrdiff_t m_hMyWearables        = 0x1158;

    // CPlayer_ObserverServices
    constexpr std::ptrdiff_t m_hObserverTarget     = 0x4C;
    constexpr std::ptrdiff_t m_iObserverMode       = 0x48;

    // CCSPlayerController
    constexpr std::ptrdiff_t m_pInventoryServices  = 0x808;
    constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x858;
    constexpr std::ptrdiff_t m_hPlayerPawn         = 0x904;
    constexpr std::ptrdiff_t m_bPawnIsAlive        = 0x90C;
    constexpr std::ptrdiff_t m_hObserverPawn       = 0x908;

    // CCSPlayerController_InventoryServices
    constexpr std::ptrdiff_t m_unMusicID           = 0x58;

    // C_CSPlayerPawn
    constexpr std::ptrdiff_t m_bNeedToReApplyGloves = 0x1695;  // 14152 dump 22-04-26
    constexpr std::ptrdiff_t m_EconGloves          = 0x1698;   // 14152 dump 22-04-26
    // NOTE: post-shift fields below verified against a2x/cs2-dumper 2026-04-22
    // (build 14152). Many were off by +0x40 vs the actual schema.
    constexpr std::ptrdiff_t m_hHudModelArms       = 0x1B58;
    // m_pClippingWeapon: schema field removed in build 14152 (Animgraph 2).
    // Skinchanger features that depended on it should derive the active weapon
    // pointer from m_hActiveWeapon via the entity list instead.
    constexpr std::ptrdiff_t m_pClippingWeapon     = 0x0;
    constexpr std::ptrdiff_t m_nEconGlovesChanged  = 0x1AC8;   // 14153 a2x
    constexpr std::ptrdiff_t m_iShotsFired         = 0x1C5C;
    // 14153: m_aimPunchAngle no longer exists on the pawn directly.
    // The punch QAngle now lives inside CCSPlayer_AimPunchServices, pointed to
    // by m_pAimPunchServices on the pawn. Helper accessors live in features that
    // need it. Kept as legacy alias = 0 so any stale write is a no-op (safe).
    constexpr std::ptrdiff_t m_aimPunchAngle       = 0x0;      // DEPRECATED — use indirection
    constexpr std::ptrdiff_t m_pAimPunchServices   = 0x1490;   // CCSPlayer_AimPunchServices*
    constexpr std::ptrdiff_t m_predictableBaseAngle_inAimPunch = 0x50; // QAngle inside services
    constexpr std::ptrdiff_t m_vecCsViewPunchAngle_inCamSvc    = 0x48; // QAngle inside m_pCameraServices
    // v_angle on C_BasePlayerPawn — final view angles AFTER punch is applied.
    // Per recent reversing notes, useful as an alternative read path on builds
    // where m_angEyeAngles drifts. We keep both available.
    constexpr std::ptrdiff_t v_angle               = 0x1298;
    constexpr std::ptrdiff_t m_angEyeAngles        = 0x3300;
    constexpr std::ptrdiff_t m_ArmorValue          = 0x1C74;
    constexpr std::ptrdiff_t m_bGunGameImmunity    = 0x3278;  // unverified, was 0x32B8 (-0x40)
    constexpr std::ptrdiff_t m_iIDEntIndex         = 0x33DC;
    constexpr std::ptrdiff_t m_flFlashDuration     = 0x1400;
    constexpr std::ptrdiff_t m_flFlashMaxAlpha     = 0x13FC;
    constexpr std::ptrdiff_t m_flLastSmokeOverlayAlpha = 0x1420;
    constexpr std::ptrdiff_t m_bIsScoped           = 0x1C48;
    constexpr std::ptrdiff_t m_entitySpottedState   = 0x1C30;  // was 0x1C70 (-0x40)
    // EntitySpottedState_t::m_bSpottedByMask — uint32[2] bitmask of player
    // slots currently looking at this entity. Anti-detection uses this to
    // soften silent aim when an enemy has us in their PVS / on screen.
    constexpr std::ptrdiff_t m_bSpottedByMask_inSpottedState = 0xC;

    // C_CSGameRules — m_bIsValveDS = true on Valve official matchmaking
    // dedicated servers (the only place Overwatch demos are pulled from
    // and where VAC Live actively scrutinizes input). Anti-detection uses
    // this to throttle silent aim flicks far harder on official servers.
    constexpr std::ptrdiff_t m_bIsValveDS          = 0xA4;
    constexpr std::ptrdiff_t m_bHasMatchStarted    = 0xB0;
    // C_CSGameRules — freeze/warmup gates. Silent aim during freeze time
    // (round-start "armory" where attacks are disabled by the server) is
    // an absolutely free server-side bot-detection signal: the angle write
    // is still serialised even though no shot can fire. Skip rewrites
    // entirely while either flag is set.
    constexpr std::ptrdiff_t m_bFreezePeriod       = 0x40;
    constexpr std::ptrdiff_t m_bWarmupPeriod       = 0x41;

    // C_CSPlayerPawn — m_bWaitForNoAttack is set true by the server
    // immediately after weapon switch / respawn / round-restart and stays
    // true until the player RELEASES attack and re-presses it. Silent
    // firing while this is set produces "fired before client could have
    // pressed M1 again" — pure bot signature, zero false-positive rate.
    constexpr std::ptrdiff_t m_bWaitForNoAttack    = 0x1CA8;

    // C_CSWeaponBaseGun — m_zoomLevel: 0 = unscoped, 1 = first zoom,
    // 2 = second zoom (AWP only). No-scope silent-fire on AWP/SSG
    // is one of the loudest reportable patterns in the game; no human
    // hits no-scopes consistently. Refuse silent-aim rewrites while
    // holding a sniper that isn't currently scoped.
    constexpr std::ptrdiff_t m_zoomLevel           = 0x1CB0;

    // C_EconEntity (parent of C_BasePlayerWeapon) — attribute container
    // chain to the item definition index.  m_iItemDefinitionIndex sits
    // inside m_AttributeManager.m_Item, so the absolute offset on a
    // weapon entity is 0x13B8 (m_AttributeManager) + 0x50 (m_Item) +
    // 0x1BA (m_iItemDefinitionIndex) = 0x15C2.
    // (m_AttributeManager / m_Item / m_iItemDefinitionIndex are defined
    //  later in this file under C_AttributeContainer / C_EconItemView.)
    constexpr std::ptrdiff_t kWeaponItemDefIndexOffset = 0x13B8 + 0x50 + 0x1BA; // 0x15C2

    // CS2 sniper weapon item-definition indices (stable since 2014).
    // Used by anti-detection to refuse silent firing while no-scoped.
    constexpr uint16_t kItemDefAWP    = 9;
    constexpr uint16_t kItemDefSSG08  = 40;
    constexpr uint16_t kItemDefG3SG1  = 11;
    constexpr uint16_t kItemDefSCAR20 = 38;

    // C_BaseModelEntity — alpha property
    constexpr std::ptrdiff_t m_pClientAlphaProperty = 0xF50;

    // CClientAlphaProperty
    constexpr std::ptrdiff_t m_nAlpha              = 0x17;

    // CSmokeGrenadeProjectile
    constexpr std::ptrdiff_t m_nSmokeEffectTickBegin = 0x1250;
    constexpr std::ptrdiff_t m_bDidSmokeEffect     = 0x1254;
    constexpr std::ptrdiff_t m_vSmokeColor         = 0x125C;  // Vector (3 floats)
    constexpr std::ptrdiff_t m_bSmokeEffectSpawned = 0x1299;

    // C_Inferno
    constexpr std::ptrdiff_t m_fireCount           = 0x1958;
    constexpr std::ptrdiff_t m_nFireEffectTickBegin = 0x196C;

    // C_BaseEntity velocity
    constexpr std::ptrdiff_t m_vecVelocity         = 0x430;

    // CPlayer_CameraServices
    constexpr std::ptrdiff_t m_iFOV                = 0x290;   // uint32, NOT float
    constexpr std::ptrdiff_t m_iFOVStart           = 0x294;   // uint32
    constexpr std::ptrdiff_t m_flFOVTime           = 0x298;   // GameTime_t
    constexpr std::ptrdiff_t m_flFOVRate           = 0x29C;   // float32
    constexpr std::ptrdiff_t m_hZoomOwner          = 0x2A0;   // CHandle
    constexpr std::ptrdiff_t m_flLastShotFOV       = 0x2A4;   // float32

    // CBasePlayerController — desired-FOV (lives on the CONTROLLER, not the pawn).
    // Writing this on the pawn corrupts random fields and crashes the game.
    constexpr std::ptrdiff_t m_iDesiredFOV_OnController = 0x784;   // uint32 (build 14152)

    // C_EnvSky (renamed in 14152: m_flSkyBrightnessScale -> m_flBrightnessScale)
    constexpr std::ptrdiff_t m_vTintColor          = 0xFB9;   // Color (RGBA bytes)
    constexpr std::ptrdiff_t m_vTintColorLightingOnly = 0xFBD; // Color
    constexpr std::ptrdiff_t m_flSkyBrightnessScale = 0xFC4;  // float32 (m_flBrightnessScale)
    constexpr std::ptrdiff_t m_bSkyEnabled         = 0xFDC;   // bool (m_bEnabled on C_EnvSky)

    // CGameSceneNode
    constexpr std::ptrdiff_t m_vecAbsOrigin        = 0xC8;

    // C_BaseEntity — viewmodel detection
    constexpr std::ptrdiff_t m_bRenderWithViewModels = 0x56A;

    // C_TonemapController2 — exposure control
    constexpr std::ptrdiff_t m_flAutoExposureMin   = 0x600;
    constexpr std::ptrdiff_t m_flAutoExposureMax   = 0x604;

    // CSkeletonInstance
    constexpr std::ptrdiff_t m_materialGroup       = 0x3C4;
    constexpr std::ptrdiff_t m_modelState          = 0x150;

    // CModelState (m_BoneArray is hand-RE'd, not in schema)
    constexpr std::ptrdiff_t m_MeshGroupMask       = 0x1C8;
    constexpr std::ptrdiff_t m_BoneArray           = 0x80;

    // C_BaseModelEntity (render / visual)
    constexpr std::ptrdiff_t m_nRenderMode         = 0xC78;
    constexpr std::ptrdiff_t m_clrRender           = 0xC98;
    constexpr std::ptrdiff_t m_Glow                = 0xDD8;
    constexpr std::ptrdiff_t m_flGlowBackfaceMult  = 0xE30;
    constexpr std::ptrdiff_t m_ClientOverrideTint  = 0xF58;
    constexpr std::ptrdiff_t m_bUseClientOverrideTint = 0xF5C;

    // CGlowProperty
    constexpr std::ptrdiff_t m_bGlowing            = 0x51;
    constexpr std::ptrdiff_t m_glowColorOverride   = 0x40;
    constexpr std::ptrdiff_t m_iGlowType           = 0x30;
    constexpr std::ptrdiff_t m_nGlowRange          = 0x38;
    constexpr std::ptrdiff_t m_nGlowRangeMin       = 0x3C;
    constexpr std::ptrdiff_t m_flGlowTime          = 0x48;
    constexpr std::ptrdiff_t m_flGlowStartTime     = 0x4C;

    // C_EconEntity
    constexpr std::ptrdiff_t m_AttributeManager    = 0x13B8;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidLow  = 0x1650;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidHigh = 0x1654;
    constexpr std::ptrdiff_t m_nFallbackPaintKit   = 0x1658;
    constexpr std::ptrdiff_t m_nFallbackSeed       = 0x165C;
    constexpr std::ptrdiff_t m_flFallbackWear      = 0x1660;
    constexpr std::ptrdiff_t m_nFallbackStatTrak   = 0x1664;

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

    // C_CSWeaponBase — server-replicated firing state.
    // Used by the seeded triggerbot to predict whether the next shot
    // will land inside the target's hitbox cone before pulling the
    // trigger. Verified against build 14152 schema dump.
    constexpr std::ptrdiff_t m_fAccuracyPenalty    = 0x17D0;  // float — cumulative inaccuracy
    constexpr std::ptrdiff_t m_flRecoilIndex       = 0x17E0;  // float — current recoil step

    // C_PlantedC4
    constexpr std::ptrdiff_t m_bBombTicking        = 0x1160;
    constexpr std::ptrdiff_t m_flC4Blow            = 0x1190;
    constexpr std::ptrdiff_t m_bBombDefused        = 0x11B4;
    constexpr std::ptrdiff_t m_flDefuseCountDown   = 0x11B0;
    constexpr std::ptrdiff_t m_flDefuseLength      = 0x11AC;
}
