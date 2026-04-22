// =====================================================================
// verified_features.hpp — hand-curated working CS2 cheat features
// =====================================================================
//
// Each entry is a known-good offset / hook / ConVar trick that has been
// verified live against the current CS2 build. Constants are static
// constexpr so they can be used at compile time.
//
// CS2_BUILD: 14153
// =====================================================================

#pragma once

#include <cstdint>

namespace cs2::verified
{
    // ----- No Smoke [working] -----
    namespace No_Smoke
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_SmokeGrenadeProjectile__m_nSmokeEffectTickBegin = 0x1250; // int32 — set 0 to skip the puff
        constexpr std::ptrdiff_t C_SmokeGrenadeProjectile__m_bDidSmokeEffect = 0x1254; // bool — set false
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_flLastSmokeOverlayAlpha = 0x1420; // float — local pawn's screen overlay alpha; set 0
    }

    // ----- Smoke Color [working] -----
    namespace Smoke_Color
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_SmokeGrenadeProjectile__m_vSmokeColor = 0x125C; // Vector3 (RGB 0..255) — 3 floats, multiply RGB by 255
    }

    // ----- No Flash [working] -----
    namespace No_Flash
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_flFlashDuration = 0x1400; // float — set 0 to clear flash
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_flFlashMaxAlpha = 0x13FC; // float — set 0 for full removal, 0..255 for partial
    }

    // ----- Skybox Tint [working] -----
    namespace Skybox_Tint
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t env_sky__m_vTintColor = 0xFB9; // Color32 — DOES NOT WORK at runtime — use the DrawSkyboxArray hook instead
        constexpr std::ptrdiff_t env_sky__m_vTintColorLightingOnly = 0xFBD; // Color32 — Same caveat
        constexpr std::ptrdiff_t env_sky__m_flBrightnessScale = 0xFC4; // float — DOES update live (independent code path)
    }

    // ----- FOV Changer [working] -----
    namespace FOV_Changer
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t CBasePlayerController__m_iDesiredFOV = 0x784; // uint32 — a2x-named m_iDesiredFOV_OnController
        constexpr std::ptrdiff_t CCSPlayer_CameraServices__m_iFOV = 0x290; // uint32 — current camera FOV
        constexpr std::ptrdiff_t CCSPlayer_CameraServices__m_iFOVStart = 0x294; // uint32 — target camera FOV
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_pCameraServices = 0x1218; // CCSPlayer_CameraServices* — deref to reach m_iFOV/m_iFOVStart
    }

    // ----- Chams (Material Override) [working] -----
    namespace Chams__Material_Override_
    {
        constexpr const char* status = "working";
    }

    // ----- Fullbright [broken — does not toggle in build 14153 even with both slots written; suspect engine reads a 3rd location or the cvar object pointer moved. Re-IDA scenesystem.dll for the new value-slot offset.] -----
    namespace Fullbright
    {
        constexpr const char* status = "broken — does not toggle in build 14153 even with both slots written; suspect engine reads a 3rd location or the cvar object pointer moved. Re-IDA scenesystem.dll for the new value-slot offset.";
        constexpr uint32_t convar_mat_fullbright__strip_flags = 0x4400;
        constexpr bool     convar_mat_fullbright__write_both  = true;
    }

    // ----- Third Person [working] -----
    namespace Third_Person
    {
        constexpr const char* status = "working";
    }

    // ----- Anti-Fog [working] -----
    namespace Anti_Fog
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_FogController__m_fog__params_ = 0x608; // fogparams_t — +0x14 RGB primary, +0x18 RGB secondary, +0x24 start, +0x28 end, +0x30 density, +0x64 enable bool
        constexpr std::ptrdiff_t C_EnvCubemapFog__m_bActive = 0x62C; // bool — set false
        constexpr std::ptrdiff_t C_EnvCubemapFog__m_flFogMaxOpacity = 0x630; // float — set 0
        constexpr std::ptrdiff_t C_EnvVolumetricFogController__m_bActive = 0x64C; // bool — set false
        constexpr std::ptrdiff_t C_EnvVolumetricFogController__m_bStartDisabled = 0x674; // bool — set true
        constexpr std::ptrdiff_t C_EnvVolumetricFogVolume__m_bActive = 0x600; // bool — set false
        constexpr std::ptrdiff_t C_EnvVolumetricFogVolume__m_flStrength = 0x620; // float — set 0
    }

    // ----- No Color Correction [working] -----
    namespace No_Color_Correction
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_ColorCorrection__m_flMaxWeight = 0x61C; // float — set 0
        constexpr std::ptrdiff_t C_ColorCorrection__m_flCurWeight = 0x620; // float — set 0
        constexpr std::ptrdiff_t C_ColorCorrection__m_bEnabled = 0x824; // bool — set false
        constexpr std::ptrdiff_t C_ColorCorrection__m_bEnabledOnClient_0_ = 0x828; // bool — set false
        constexpr std::ptrdiff_t C_ColorCorrection__m_flCurWeightOnClient = 0x82C; // float — set 0
    }

    // ----- Night Mode / Asus Mode (Sky Tint via env_sky) [working] -----
    namespace Night_Mode___Asus_Mode__Sky_Tint_via_env_sky_
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t env_sky__m_vTintColor = 0xFB9; // Color32 — RGBA 0..255 packed
        constexpr std::ptrdiff_t env_sky__m_vTintColorLightingOnly = 0xFBD; // Color32 — match m_vTintColor for consistency
        constexpr std::ptrdiff_t env_sky__m_flBrightnessScale = 0xFC4; // float — 1.0 = default
    }

    // ----- ESP — Player Pawn Core [working] -----
    namespace ESP___Player_Pawn_Core
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_BaseEntity__m_pGameSceneNode = 0x330; // CGameSceneNode* — deref → bone matrix + abs origin
        constexpr std::ptrdiff_t C_BaseEntity__m_iHealth = 0x34C; // int32 — 0 == dead
        constexpr std::ptrdiff_t C_BaseEntity__m_lifeState = 0x354; // uint8 — 0 == ALIVE
        constexpr std::ptrdiff_t C_BaseEntity__m_iTeamNum = 0x3EB; // uint8 — 2=T, 3=CT
        constexpr std::ptrdiff_t CGameSceneNode__m_vecAbsOrigin = 0xC8; // Vector3 — world position (read for ESP root)
        constexpr std::ptrdiff_t C_CSPlayerPawnBase__m_pWeaponServices = 0x11E0; // ptr — pawn-side weapon services
        constexpr std::ptrdiff_t C_CSPlayerPawnBase__m_pObserverServices = 0x11F8; // ptr — spectator target via +0x4C m_hObserverTarget
        constexpr std::ptrdiff_t CCSPlayerController__m_iszPlayerName = 0x6F0; // char[128] — UTF-8 nickname
        constexpr std::ptrdiff_t CCSPlayerController__m_hPawn = 0x6BC; // CHandle — handle → pawn entity
        constexpr std::ptrdiff_t C_BasePlayerWeapon__m_iItemDefinitionIndex = 0x1BA; // uint16 — weapon definition index (CSWeaponID)
        constexpr std::ptrdiff_t C_BasePlayerWeapon__m_iClip1 = 0x16D8; // int32 — current magazine count
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_iIDEntIndex = 0x33DC; // CEntityIndex — entity the local pawn is looking at (handy for triggerbot)
        constexpr std::ptrdiff_t C_CSObserverPawn__m_hObserverTarget = 0x4C; // CHandle — current spectated entity (use when local is spectating)
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_vOldOrigin = 0x1390; // Vector — previous-tick origin — useful for prediction / extrapolation
    }

    // ----- ESP — Skeleton / Bones [working] -----
    namespace ESP___Skeleton___Bones
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_BaseEntity__m_pGameSceneNode = 0x330; // CSkeletonInstance* — actually CSkeletonInstance for animated entities
        constexpr std::ptrdiff_t CSkeletonInstance__m_modelState = 0x150; // CModelState — embedded; bone array pointer is inside
    }

    // ----- Silent Aim / Aim Punch / No Recoil [working] -----
    namespace Silent_Aim___Aim_Punch___No_Recoil
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_pAimPunchServices = 0x1490; // CCSPlayer_AimPunchServices* — deref for punch arrays
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_iShotsFired = 0x1C5C; // int32 — consecutive shots fired this trigger pull (drives spread)
        constexpr std::ptrdiff_t C_CSWeaponBase__m_flRecoilIndex = 0x17E0; // float — recoil pattern index
        constexpr std::ptrdiff_t C_CSWeaponBase__m_flNextPrimaryAttackTickRatio = 0x16CC; // float — rate-of-fire gate
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_pWeaponServices = 0x11E0; // ptr — owns FireBullet (recoil applied here)
    }

    // ----- Money / Armor / Helmet / Score [working] -----
    namespace Money___Armor___Helmet___Score
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t CCSPlayerController_InGameMoneyServices__m_iAccount = 0x40; // int32 — current cash
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_ArmorValue = 0x1C74; // int32 — armor 0..100
        constexpr std::ptrdiff_t CCSPlayer_ItemServices__m_bHasDefuser = 0x48; // bool — CT defuser flag
        constexpr std::ptrdiff_t CCSPlayer_ItemServices__m_bHasHelmet = 0x49; // bool — kevlar+helmet flag
        constexpr std::ptrdiff_t CCSPlayerController_ActionTrackingServices__m_iKills = 0x30; // int32 — scoreboard kills
        constexpr std::ptrdiff_t CCSPlayerController_ActionTrackingServices__m_iDeaths = 0x34; // int32 — scoreboard deaths
        constexpr std::ptrdiff_t CCSPlayerController_ActionTrackingServices__m_iAssists = 0x38; // int32 — scoreboard assists
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_unCurrentEquipmentValue = 0x1C78; // uint16 — rounded loadout $$
    }

    // ----- Spotted / Glow (Radar Hack) [working] -----
    namespace Spotted___Glow__Radar_Hack_
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t EntitySpottedState_t__m_bSpotted = 0x8; // bool — force true to reveal on radar
        constexpr std::ptrdiff_t EntitySpottedState_t__m_bSpottedByMask = 0xC; // uint32[2] — bit per spotter; OR with 0xFFFFFFFF
    }

    // ----- Rank Reveal (Premier MMR) [working] -----
    namespace Rank_Reveal__Premier_MMR_
    {
        constexpr const char* status = "working";
        constexpr std::ptrdiff_t CCSPlayerController__m_iCompetitiveRanking = 0x878; // int32 — Premier rating
        constexpr std::ptrdiff_t CCSPlayerController__m_iCompetitiveRankingPredicted_Win = 0x884; // int32 — +rating on win
        constexpr std::ptrdiff_t CCSPlayerController__m_iCompetitiveRankingPredicted_Loss = 0x888; // int32 — -rating on loss
        constexpr std::ptrdiff_t CCSPlayerController__m_iCompetitiveRankingPredicted_Tie = 0x88C; // int32 — +/- rating on draw
    }

    // ----- Globals — RVAs in client.dll [working] -----
    namespace Globals___RVAs_in_client_dll
    {
        constexpr const char* status = "working";
    }

} // namespace cs2::verified
