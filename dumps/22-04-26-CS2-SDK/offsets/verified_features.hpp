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
    // ----- No Smoke -----
    namespace No_Smoke
    {
        constexpr std::ptrdiff_t C_SmokeGrenadeProjectile__m_nSmokeEffectTickBegin = 0x1250; // int32 — set 0 to skip the puff
        constexpr std::ptrdiff_t C_SmokeGrenadeProjectile__m_bDidSmokeEffect = 0x1254; // bool — set false
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_flLastSmokeOverlayAlpha = 0x1420; // float — local pawn's screen overlay alpha; set 0
    }

    // ----- Smoke Color -----
    namespace Smoke_Color
    {
        constexpr std::ptrdiff_t C_SmokeGrenadeProjectile__m_vSmokeColor = 0x125C; // Vector3 (RGB 0..255) — 3 floats, multiply RGB by 255
    }

    // ----- No Flash -----
    namespace No_Flash
    {
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_flFlashDuration = 0x1400; // float — set 0 to clear flash
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_flFlashMaxAlpha = 0x13FC; // float — set 0 for full removal, 0..255 for partial
    }

    // ----- Skybox Tint -----
    namespace Skybox_Tint
    {
        constexpr std::ptrdiff_t env_sky__m_vTintColor = 0xFB9; // Color32 — DOES NOT WORK at runtime — use the DrawSkyboxArray hook instead
        constexpr std::ptrdiff_t env_sky__m_vTintColorLightingOnly = 0xFBD; // Color32 — Same caveat
        constexpr std::ptrdiff_t env_sky__m_flBrightnessScale = 0xFC4; // float — DOES update live (independent code path)
    }

    // ----- FOV Changer -----
    namespace FOV_Changer
    {
        constexpr std::ptrdiff_t CBasePlayerController__m_iDesiredFOV = 0x784; // uint32 — a2x-named m_iDesiredFOV_OnController
        constexpr std::ptrdiff_t CCSPlayer_CameraServices__m_iFOV = 0x290; // uint32 — current camera FOV
        constexpr std::ptrdiff_t CCSPlayer_CameraServices__m_iFOVStart = 0x294; // uint32 — target camera FOV
        constexpr std::ptrdiff_t C_CSPlayerPawn__m_pCameraServices = 0x1218; // CCSPlayer_CameraServices* — deref to reach m_iFOV/m_iFOVStart
    }

    // ----- Chams (Material Override) -----
    namespace Chams__Material_Override_
    {
    }

    // ----- Fullbright -----
    namespace Fullbright
    {
        constexpr uint32_t convar_mat_fullbright__strip_flags = 0x4400;
        constexpr bool     convar_mat_fullbright__write_both  = true;
    }

    // ----- Third Person -----
    namespace Third_Person
    {
    }

    // ----- Anti-Fog -----
    namespace Anti_Fog
    {
        constexpr std::ptrdiff_t C_FogController__m_fog__params_ = 0x608; // fogparams_t — +0x14 RGB primary, +0x18 RGB secondary, +0x24 start, +0x28 end, +0x30 density, +0x64 enable bool
        constexpr std::ptrdiff_t C_EnvCubemapFog__m_bActive = 0x62C; // bool — set false
        constexpr std::ptrdiff_t C_EnvCubemapFog__m_flFogMaxOpacity = 0x630; // float — set 0
        constexpr std::ptrdiff_t C_EnvVolumetricFogController__m_bActive = 0x64C; // bool — set false
        constexpr std::ptrdiff_t C_EnvVolumetricFogController__m_bStartDisabled = 0x674; // bool — set true
        constexpr std::ptrdiff_t C_EnvVolumetricFogVolume__m_bActive = 0x600; // bool — set false
        constexpr std::ptrdiff_t C_EnvVolumetricFogVolume__m_flStrength = 0x620; // float — set 0
    }

    // ----- No Color Correction -----
    namespace No_Color_Correction
    {
        constexpr std::ptrdiff_t C_ColorCorrection__m_flMaxWeight = 0x61C; // float — set 0
        constexpr std::ptrdiff_t C_ColorCorrection__m_flCurWeight = 0x620; // float — set 0
        constexpr std::ptrdiff_t C_ColorCorrection__m_bEnabled = 0x824; // bool — set false
        constexpr std::ptrdiff_t C_ColorCorrection__m_bEnabledOnClient_0_ = 0x828; // bool — set false
        constexpr std::ptrdiff_t C_ColorCorrection__m_flCurWeightOnClient = 0x82C; // float — set 0
    }

    // ----- Night Mode / Asus Mode (Sky Tint via env_sky) -----
    namespace Night_Mode___Asus_Mode__Sky_Tint_via_env_sky_
    {
        constexpr std::ptrdiff_t env_sky__m_vTintColor = 0xFB9; // Color32 — RGBA 0..255 packed
        constexpr std::ptrdiff_t env_sky__m_vTintColorLightingOnly = 0xFBD; // Color32 — match m_vTintColor for consistency
        constexpr std::ptrdiff_t env_sky__m_flBrightnessScale = 0xFC4; // float — 1.0 = default
    }

} // namespace cs2::verified
