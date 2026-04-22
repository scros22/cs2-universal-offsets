#pragma once

// ---------------------------------------------------------------
// World Effects — Sky color, no-flash, no-smoke/smoke color,
// molotov/fire color, FOV changer, radar hack.
// FOV: hooks GetWorldFov in client.dll (intercepts the renderer's
//      FOV query — no memory write fighting).
// Sky: hooks DrawSkyboxArray in scenesystem.dll (modifies color
//      in the draw primitive buffer — no fog manipulation).
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <cstring>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../core/signatures.h"
#include "../core/stealth.h"

namespace WorldEffects
{
    struct Config
    {
        // Sky color
        bool  skyEnabled     = false;
        bool  skyRainbow     = false;   // smooth rainbow cycle
        float skyRainbowSpeed = 0.3f;  // cycles per second
        float skyColor[4]    = { 0.4f, 0.0f, 0.6f, 1.0f }; // default purple
        float skyBrightness  = 1.0f;   // brightness scale for env_sky

        // No-flash
        bool  noFlash        = false;
        float maxFlashAlpha  = 0.0f;   // 0 = fully removed, 0-255 for partial

        // Smoke
        bool  noSmoke        = false;
        bool  smokeColor     = false;
        float smokeCol[4]    = { 0.0f, 0.5f, 1.0f, 0.3f }; // blue-ish transparent

        // Molotov/fire
        bool  fireColor      = false;
        float fireCol[4]     = { 1.0f, 0.0f, 0.5f, 0.8f };

        // FOV
        bool  fovEnabled     = false;
        float fovValue       = 110.f;

        // Brightness / Exposure (fullbright-like via tonemap manipulation)
        bool  brightnessEnabled = false;
        float exposureMin       = 1.0f;   // default game: ~0.25
        float exposureMax       = 1.0f;   // default game: ~8.0

        // Night mode — darkens sky, lowers exposure, adds fog tint
        int   nightMode         = 0;       // 0=off, 1=night, 2=midnight, 3=sunset, 4=bloodmoon

        // Fog override
        bool  fogEnabled        = false;
        float fogColor[3]       = { 0.1f, 0.1f, 0.2f };
        float fogStart          = 100.f;
        float fogEnd            = 4000.f;
        float fogDensity        = 0.8f;

        // Third person
        bool  thirdPerson       = false;
        float thirdPersonDist   = 120.f; // camera distance
    };

    inline Config cfg;

    // ---------------------------------------------------------------
    // Anti-flash: zero out flash duration + alpha on local pawn
    // ---------------------------------------------------------------
    inline void RunNoFlash()
    {
        if (!cfg.noFlash || !GameState::clientBase) return;
        __try {
            uintptr_t lp = GameState::GetLocalPawn();
            if (!lp) return;
            float dur = Mem::Read<float>(lp + Offsets::m_flFlashDuration);
            if (dur > 0.01f)
            {
                Mem::SmartWrite<float>(lp + Offsets::m_flFlashDuration, 0.f);
                Mem::SmartWrite<float>(lp + Offsets::m_flFlashMaxAlpha, cfg.maxFlashAlpha);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // No-smoke: find C_SmokeGrenadeProjectile entities and kill the
    // smoke effect by zeroing m_nSmokeEffectTickBegin.
    // Smoke color: set m_vSmokeColor on the smoke entity.
    //
    // Entity identification: read designerName from CEntityIdentity
    // to confirm it's a smoke projectile before writing.
    // ---------------------------------------------------------------
    inline void RunNoSmoke()
    {
        if (!cfg.noSmoke && !cfg.smokeColor) return;
        if (!GameState::clientBase) return;

        // Throttle — no need to run every frame
        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 100) return;
        lastTick = now;

        __try {
            // Also zero overlay alpha on local pawn
            if (cfg.noSmoke)
            {
                uintptr_t lp = GameState::GetLocalPawn();
                if (lp) Mem::Write<float>(lp + Offsets::m_flLastSmokeOverlayAlpha, 0.f);
            }

            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            // Smoke grenade projectile offsets (from dumper: C_SmokeGrenadeProjectile, build 14152)
            constexpr std::ptrdiff_t kSmokeEffectTickBegin = 0x1250;
            constexpr std::ptrdiff_t kDidSmokeEffect       = 0x1254;
            constexpr std::ptrdiff_t kSmokeColor           = 0x125C; // Vector (3 floats)

            for (int i = 64; i < 1024; ++i) // skip player slots
            {
                __try {
                    uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                    if (!chunk) continue;
                    uintptr_t ent = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                    if (!ent) continue;

                    // Read CEntityIdentity (ent + 0x10)
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
                    if (!identity) continue;

                    // Read designer name pointer (CUtlSymbolLarge = const char*)
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
                    if (!namePtr) continue;

                    // Read first 16 bytes of name — enough to check "smokegrenade_pro"
                    char name[24]{};
                    for (int c = 0; c < 23; ++c) {
                        name[c] = Mem::Read<char>(namePtr + c);
                        if (name[c] == '\0') break;
                    }

                    if (strncmp(name, "smokegrenade_projectile", 23) != 0)
                        continue;

                    // Confirmed smoke entity — apply effects
                    if (cfg.noSmoke)
                    {
                        int32_t tickBegin = Mem::Read<int32_t>(ent + kSmokeEffectTickBegin);
                        if (tickBegin != 0)
                            Mem::Write<int32_t>(ent + kSmokeEffectTickBegin, 0);
                        Mem::Write<bool>(ent + kDidSmokeEffect, false);
                    }

                    if (cfg.smokeColor)
                    {
                        // Write custom RGB to m_vSmokeColor (Vector = 3 floats)
                        Mem::Write<float>(ent + kSmokeColor + 0x0, cfg.smokeCol[0] * 255.f);
                        Mem::Write<float>(ent + kSmokeColor + 0x4, cfg.smokeCol[1] * 255.f);
                        Mem::Write<float>(ent + kSmokeColor + 0x8, cfg.smokeCol[2] * 255.f);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Molotov/fire color — tint inferno entities via m_clrRender
    // ---------------------------------------------------------------
    inline void RunFireColor()
    {
        if (!cfg.fireColor || !GameState::clientBase) return;

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 200) return;
        lastTick = now;

        __try {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 0; i < 2048; ++i)
            {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ent = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                if (!ent) continue;

                __try {
                    // Identify inferno by designer name (cheap pointer checks first)
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
                    if (!namePtr) continue;
                    char name[32]{};
                    for (int c = 0; c < 31; ++c)
                        name[c] = Mem::Read<char>(namePtr + c);
                    if (!strstr(name, XS("inferno"))) continue;

                    // Apply color via m_clrRender (best-effort for particle tinting)
                    uint8_t r = (uint8_t)(cfg.fireCol[0] * 255.f);
                    uint8_t g = (uint8_t)(cfg.fireCol[1] * 255.f);
                    uint8_t b = (uint8_t)(cfg.fireCol[2] * 255.f);
                    uint8_t a = (uint8_t)(cfg.fireCol[3] * 255.f);
                    uint32_t color = (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24);
                    Mem::SmartWrite<uint32_t>(ent + Offsets::m_clrRender, color);
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // GetWorldFov hook — intercepts the engine's FOV query.
    // Returns our desired value instead of the default.
    // ---------------------------------------------------------------
    using GetWorldFovFn = float(__fastcall*)(void*);
    inline GetWorldFovFn oGetWorldFov = nullptr;
    inline bool         fovHooked      = false;
    inline void*        pFovHookTarget = nullptr;

    // Third-person ConVar value addresses (offset +0x40 into CS2 ConVar struct)
    inline uintptr_t pCV_c_thirdpersonshoulder      = 0;
    inline uintptr_t pCV_cam_idealdist              = 0;
    inline uintptr_t pCV_thirdpersonshoulderaimdist = 0;
    inline uintptr_t pCV_thirdpersonshoulderdist    = 0;
    inline uintptr_t pCV_thirdpersonshoulderheight  = 0;
    inline uintptr_t pCV_thirdpersonshoulderoffset  = 0;

    inline float __fastcall hkGetWorldFov(void* rcx)
    {
        float orig = oGetWorldFov ? oGetWorldFov(rcx) : 90.f;
        if (!cfg.fovEnabled) return orig;
        __try {
            if (!GameState::clientBase) return orig;
            uintptr_t lp = GameState::GetLocalPawn();
            if (!lp) return orig;
            bool scoped = Mem::Read<bool>(lp + Offsets::m_bIsScoped);
            if (scoped) return orig;
            return cfg.fovValue;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return orig; }
    }

    // ---------------------------------------------------------------
    // DrawSkyboxArray hook — modifies skybox color in the render
    // primitive buffer before the skybox is drawn. No fog, no entity
    // writes — operates at the rendering level.
    // ---------------------------------------------------------------
    using DrawSkyboxArrayFn = void(__fastcall*)(void*, void*, void*, int, void*, void*, void*);
    inline DrawSkyboxArrayFn oDrawSkyboxArray = nullptr;
    inline bool skyHooked = false;

    // HSV to RGB (h: 0-360, s/v: 0-1) → RGB 0-1
    inline void HsvToRgb(float h, float s, float v, float& r, float& g, float& b)
    {
        float c = v * s;
        float x = c * (1.f - fabsf(fmodf(h / 60.f, 2.f) - 1.f));
        float m = v - c;
        if      (h < 60.f)  { r = c; g = x; b = 0; }
        else if (h < 120.f) { r = x; g = c; b = 0; }
        else if (h < 180.f) { r = 0; g = c; b = x; }
        else if (h < 240.f) { r = 0; g = x; b = c; }
        else if (h < 300.f) { r = x; g = 0; b = c; }
        else                { r = c; g = 0; b = x; }
        r += m; g += m; b += m;
    }

    inline void __fastcall hkDrawSkyboxArray(void* a1, void* a2, void* drawPrimitive,
                                              int count, void* a5, void* a6, void* a7)
    {
        if (cfg.skyEnabled && drawPrimitive && count > 0 && count < 100)
        {
            __try {
                size_t offset = (size_t)(count * 0x68) - 0x50;
                void** skyboxObjPtr = (void**)((char*)drawPrimitive + offset);
                if (skyboxObjPtr && *skyboxObjPtr)
                {
                    float* colorPtr = (float*)((char*)(*skyboxObjPtr) + 0x100);

                    if (cfg.skyRainbow)
                    {
                        // Smooth rainbow: hue cycles based on time
                        float t = (float)GetTickCount64() / 1000.f;
                        float hue = fmodf(t * cfg.skyRainbowSpeed * 360.f, 360.f);
                        HsvToRgb(hue, 1.f, 1.f, colorPtr[0], colorPtr[1], colorPtr[2]);
                    }
                    else
                    {
                        colorPtr[0] = cfg.skyColor[0];
                        colorPtr[1] = cfg.skyColor[1];
                        colorPtr[2] = cfg.skyColor[2];
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
        if (oDrawSkyboxArray) oDrawSkyboxArray(a1, a2, drawPrimitive, count, a5, a6, a7);
    }

    // ---------------------------------------------------------------
    // Sky brightness via direct entity writes to env_sky.
    // m_flBrightnessScale updates in real-time. Color is handled
    // by the DrawSkyboxArray hook (entity writes to m_vTintColor
    // don't work — renderer caches material params at setup).
    // ---------------------------------------------------------------
    inline void RunSkyBrightness()
    {
        if (!cfg.skyEnabled || !GameState::clientBase) return;
        if (cfg.skyBrightness == 1.0f) return; // nothing to do

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 200) return;
        lastTick = now;

        __try {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 0; i < 8192; ++i)
            {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ent = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
                    if (!namePtr) continue;
                    struct NB { char d[64]; };
                    NB nb = Mem::Read<NB>(namePtr);
                    nb.d[63] = '\0';

                    if (strstr(nb.d, "env_sky"))
                        Mem::SmartWrite<float>(ent + Offsets::m_flSkyBrightnessScale, cfg.skyBrightness);
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Night mode — combines sky tint, exposure, and fog for
    // atmospheric presets. Applied via entity writes, no hooks.
    //   0 = Off
    //   1 = Night (dark blue sky, low exposure, light fog)
    //   2 = Midnight (black sky, very dark, thick fog)
    //   3 = Sunset (warm orange/pink sky, golden light)
    //   4 = Blood Moon (deep red sky, eerie fog)
    // ---------------------------------------------------------------
    inline void RunNightMode()
    {
        if (cfg.nightMode == 0 || !GameState::clientBase) return;

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 200) return;
        lastTick = now;

        // Night mode preset values
        struct NightPreset {
            uint8_t skyR, skyG, skyB;
            float   brightness;
            float   expMin, expMax;
            uint8_t fogR, fogG, fogB;
            float   fogStart, fogEnd, fogDensity;
        };

        static const NightPreset presets[] = {
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },             // 0: off (unused)
            { 20, 25, 60, 0.15f, 0.08f, 0.15f, 10, 15, 40, 200.f, 8000.f, 0.4f },   // 1: Night
            { 5, 5, 15, 0.05f, 0.03f, 0.06f, 5, 5, 15, 50.f, 4000.f, 0.7f },         // 2: Midnight
            { 255, 140, 50, 0.6f, 0.5f, 0.8f, 180, 100, 50, 500.f, 12000.f, 0.3f },  // 3: Sunset
            { 120, 10, 10, 0.2f, 0.1f, 0.2f, 80, 10, 10, 100.f, 6000.f, 0.6f },      // 4: Blood Moon
        };

        int idx = cfg.nightMode;
        if (idx < 1 || idx > 4) return;
        const NightPreset& p = presets[idx];

        __try {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 0; i < 8192; ++i)
            {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ent = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
                    if (!namePtr) continue;
                    struct NB { char d[64]; };
                    NB nb = Mem::Read<NB>(namePtr);
                    nb.d[63] = '\0';

                    // Sky entity
                    if (strstr(nb.d, "env_sky"))
                    {
                        uint32_t skyColor = (uint32_t)p.skyR | ((uint32_t)p.skyG << 8)
                                          | ((uint32_t)p.skyB << 16) | (255u << 24);
                        Mem::SmartWrite<uint32_t>(ent + Offsets::m_vTintColor, skyColor);
                        Mem::SmartWrite<uint32_t>(ent + Offsets::m_vTintColorLightingOnly, skyColor);
                        Mem::SmartWrite<float>(ent + Offsets::m_flSkyBrightnessScale, p.brightness);
                    }

                    // Tonemap controller — exposure
                    if (strstr(nb.d, "tonemap_controller") || strstr(nb.d, "env_tonemap"))
                    {
                        Mem::SmartWrite<float>(ent + Offsets::m_flAutoExposureMin, p.expMin);
                        Mem::SmartWrite<float>(ent + Offsets::m_flAutoExposureMax, p.expMax);
                    }

                    // Fog controller — atmospheric fog
                    if (strstr(nb.d, "env_fog_controller"))
                    {
                        constexpr std::ptrdiff_t kFog = 0x608; // fogparams_t offset in C_FogController
                        Mem::SmartWrite<bool>(ent + kFog + 0x64, true);  // enable
                        uint32_t fogColor = (uint32_t)p.fogR | ((uint32_t)p.fogG << 8)
                                          | ((uint32_t)p.fogB << 16) | (255u << 24);
                        Mem::SmartWrite<uint32_t>(ent + kFog + 0x14, fogColor); // colorPrimary
                        Mem::SmartWrite<uint32_t>(ent + kFog + 0x18, fogColor); // colorSecondary
                        Mem::SmartWrite<float>(ent + kFog + 0x24, p.fogStart);  // start
                        Mem::SmartWrite<float>(ent + kFog + 0x28, p.fogEnd);    // end
                        Mem::SmartWrite<float>(ent + kFog + 0x30, p.fogDensity);// maxdensity
                    }

                    // Cubemap fog — atmospheric haze
                    if (strstr(nb.d, "env_cubemap_fog"))
                    {
                        Mem::SmartWrite<bool>(ent + 0x62C, true);   // m_bActive
                        Mem::SmartWrite<float>(ent + 0x630, p.fogDensity); // m_flFogMaxOpacity
                        Mem::SmartWrite<float>(ent + 0x60C, p.fogStart);   // m_flStartDistance
                        Mem::SmartWrite<float>(ent + 0x608, p.fogEnd);     // m_flEndDistance
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Custom fog override — user-configurable fog via fog controller
    // ---------------------------------------------------------------
    inline void RunFogOverride()
    {
        if (!cfg.fogEnabled || !GameState::clientBase) return;

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 200) return;
        lastTick = now;

        __try {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            uint8_t fr = (uint8_t)(cfg.fogColor[0] * 255.f);
            uint8_t fg = (uint8_t)(cfg.fogColor[1] * 255.f);
            uint8_t fb = (uint8_t)(cfg.fogColor[2] * 255.f);
            uint32_t fogColor = (uint32_t)fr | ((uint32_t)fg << 8) | ((uint32_t)fb << 16) | (255u << 24);

            for (int i = 0; i < 2048; ++i)
            {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ent = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
                    if (!namePtr) continue;
                    struct NB { char d[64]; };
                    NB nb = Mem::Read<NB>(namePtr);
                    nb.d[63] = '\0';

                    if (strstr(nb.d, "env_fog_controller"))
                    {
                        constexpr std::ptrdiff_t kFog = 0x608;
                        Mem::SmartWrite<bool>(ent + kFog + 0x64, true);
                        Mem::SmartWrite<uint32_t>(ent + kFog + 0x14, fogColor);
                        Mem::SmartWrite<uint32_t>(ent + kFog + 0x18, fogColor);
                        Mem::SmartWrite<float>(ent + kFog + 0x24, cfg.fogStart);
                        Mem::SmartWrite<float>(ent + kFog + 0x28, cfg.fogEnd);
                        Mem::SmartWrite<float>(ent + kFog + 0x30, cfg.fogDensity);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // FOV changer — uses GetWorldFov hook when available (no fighting
    // with the game's camera/weapon systems). Falls back to memory
    // writes if the hook pattern was not found at init.
    // ---------------------------------------------------------------
    inline void RunFOV()
    {
        if (!cfg.fovEnabled || !GameState::clientBase) return;
        // We always write to m_iFOV / m_iDesiredFOV every tick, even when the
        // GetWorldFov hook is active — the renderer caches FOV from the camera
        // services struct at the START of each frame (before our hook fires),
        // so without these writes the value briefly reverts to default and
        // produces a visible flicker / "reverting to default" feel.
        __try {
            uintptr_t lp = GameState::GetLocalPawn();
            if (!lp) return;
            bool scoped = Mem::Read<bool>(lp + Offsets::m_bIsScoped);
            if (scoped) return; // don't override while scoped
            uint32_t desired = (uint32_t)cfg.fovValue;

            uintptr_t camSvc = Mem::Read<uintptr_t>(lp + Offsets::m_pCameraServices);
            if (camSvc) {
                Mem::SmartWrite<uint32_t>(camSvc + Offsets::m_iFOV,      desired);
                Mem::SmartWrite<uint32_t>(camSvc + Offsets::m_iFOVStart, desired);
            }
            // Pawn-level desired-FOV (the value the game's camera-update
            // path reads back into m_iFOV every tick). Without writing this
            // the camera services value gets clobbered immediately.
            Mem::SmartWrite<uint32_t>(lp + Offsets::m_iDesiredFOV, desired);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Brightness / Exposure — manipulate C_TonemapController2
    // Overrides auto-exposure range to force consistent brightness.
    // Setting both min and max to the same value = locked exposure.
    // Higher values = brighter. 1.0 = fullbright-ish, 0.5 = neutral.
    // ---------------------------------------------------------------
    inline void RunBrightness()
    {
        if (!cfg.brightnessEnabled || !GameState::clientBase) return;

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 200) return;
        lastTick = now;

        __try {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 0; i < 1024; ++i)
            {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ent = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
                    if (!namePtr) continue;
                    struct NB { char d[64]; };
                    NB nb = Mem::Read<NB>(namePtr);
                    nb.d[63] = '\0';

                    if (strstr(nb.d, "tonemap_controller") || strstr(nb.d, "env_tonemap"))
                    {
                        Mem::SmartWrite<float>(ent + Offsets::m_flAutoExposureMin, cfg.exposureMin);
                        Mem::SmartWrite<float>(ent + Offsets::m_flAutoExposureMax, cfg.exposureMax);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Third person — OverrideView hook on CCSGOViewAdviceService.
    // Hooks the camera setup function in client.dll and offsets the
    // camera behind/above the player while also setting the engine's
    // third-person input flag so the local model renders reliably.
    // Toggled via middle mouse click (handled in WndProc).
    // ---------------------------------------------------------------

    // CViewSetup field offsets (Source 2 / CS2 — validated March 2026)
    namespace ViewSetupOffsets
    {
        constexpr size_t Origin = 0x490; // Vec3 (x, y, z)
        constexpr size_t Angles = 0x4A0; // QAngle (pitch, yaw, roll)
        constexpr size_t Fov    = 0x230; // float
    }

    // Hook function type: void __fastcall OverrideView(void* thisPtr, void* viewSetup)
    using OverrideViewFn = void(__fastcall*)(void*, void*);
    inline OverrideViewFn oOverrideView = nullptr;
    inline void*          pOverrideViewTarget = nullptr;
    inline bool           overrideViewHooked = false;

    inline void __fastcall hkOverrideView(void* thisPtr, void* viewSetup)
    {
        // Call original first — let engine set up default camera
        if (oOverrideView)
            oOverrideView(thisPtr, viewSetup);

        if (!GameState::clientBase) return;

        if (cfg.thirdPerson)
        {
            __try {
                uintptr_t lp = GameState::GetLocalPawn();
                if (!lp) return;

                int hp = Mem::Read<int>(lp + Offsets::m_iHealth);
                if (hp <= 0) return;

                // Enable engine third-person input flag
                Mem::SmartWrite<bool>(
                    GameState::clientBase + GameState::RVA_dwCSGOInput() + 0x229, true);

                // Use the game's built-in shoulder camera ConVars.
                // Setting c_thirdpersonshoulder=1 and cam_idealdist lets the engine
                // properly position the camera behind the player with collision.
                if (pCV_c_thirdpersonshoulder)
                    *reinterpret_cast<int*>(pCV_c_thirdpersonshoulder) = 1;
                if (pCV_cam_idealdist)
                    *reinterpret_cast<float*>(pCV_cam_idealdist) = cfg.thirdPersonDist;
                if (pCV_thirdpersonshoulderaimdist)
                    *reinterpret_cast<float*>(pCV_thirdpersonshoulderaimdist) = 0.f;
                if (pCV_thirdpersonshoulderdist)
                    *reinterpret_cast<float*>(pCV_thirdpersonshoulderdist) = 0.f;
                if (pCV_thirdpersonshoulderheight)
                    *reinterpret_cast<float*>(pCV_thirdpersonshoulderheight) = 0.f;
                if (pCV_thirdpersonshoulderoffset)
                    *reinterpret_cast<float*>(pCV_thirdpersonshoulderoffset) = 0.f;

            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }

    // Disable third person flag and reset ConVars when turned off (called from Tick)
    inline void RunThirdPerson()
    {
        if (!cfg.thirdPerson)
        {
            if (GameState::clientBase)
            {
                Mem::SmartWrite<bool>(
                    GameState::clientBase + GameState::RVA_dwCSGOInput() + 0x229, false);
                if (pCV_c_thirdpersonshoulder)
                    *reinterpret_cast<int*>(pCV_c_thirdpersonshoulder) = 0;
                if (pCV_cam_idealdist)
                    *reinterpret_cast<float*>(pCV_cam_idealdist) = 130.f;
            }
        }
    }

    // ---------------------------------------------------------------
    // Main tick — call all sub-features
    // ---------------------------------------------------------------
    inline void Tick()
    {
        RunNoFlash();
        RunNoSmoke();
        RunFireColor();
        RunSkyBrightness();
        RunNightMode();
        RunFogOverride();
        RunBrightness();
        RunFOV();
        RunThirdPerson();
    }

    // ---------------------------------------------------------------
    // Setup / Shutdown — install and remove function hooks.
    // Must be called AFTER MH_Initialize() and module bases are ready.
    // ---------------------------------------------------------------
    // Store hook target so we can disable/remove it in Shutdown
    inline void* pSkyHookTarget = nullptr;

    // ---------------------------------------------------------------
    // ConVar lookup via tier0 CCvar linked list.
    //
    // CS2 ConVars self-register into a central list managed by tier0.dll.
    // The CCvar interface ("VEngineCvar007") holds a CUtlLinkedList of
    // convar* pointers. We traverse it to find a ConVar by name and
    // return the address of its value union (at convar+0x40).
    //
    // CCvar layout (Fatality cs2 SDK):
    //   +0x00-0x3F : vtable / padding
    //   +0x40      : CUtlLinkedList<convar*> cvars
    //     +0x00    : cutl_memory.memory ptr  (T* elem array)
    //     +0x08    : cutl_memory.alloc_count (int)
    //     +0x0C    : cutl_memory.grow_size   (int)
    //     +0x10    : _head  (uint16)
    //
    // Element array stride = 16 bytes each:
    //   +0x00 : convar* elem
    //   +0x08 : uint16 previous
    //   +0x0A : uint16 next
    //   +0x0C : padding
    //
    // convar layout:
    //   +0x00 : const char* name
    //   +0x40 : cvar_value_t value
    // ---------------------------------------------------------------
    inline uintptr_t FindCvarValue(void* pCCvar, const char* name)
    {
        if (!pCCvar || !name) return 0;
        __try {
            uintptr_t lb     = reinterpret_cast<uintptr_t>(pCCvar) + 0x40;
            void*     elems  = *reinterpret_cast<void**>(lb);          // _memory.memory
            uint16_t  head   = *reinterpret_cast<uint16_t*>(lb + 0x10); // _head
            if (!elems || head == 0xFFFF) return 0;

            int guard = 8192; // safety cap
            for (uint16_t i = head; i != 0xFFFF && guard-- > 0; )
            {
                uint8_t* ep = reinterpret_cast<uint8_t*>(elems) + (uintptr_t)i * 16;
                __try {
                    auto* cv = *reinterpret_cast<void**>(ep);           // elem = convar*
                    if (cv)
                    {
                        const char* n = *reinterpret_cast<const char**>(cv); // name at +0
                        if (n && strcmp(n, name) == 0)
                            return reinterpret_cast<uintptr_t>(cv) + 0x40;  // value at +0x40
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
                i = *reinterpret_cast<uint16_t*>(ep + 10); // next index
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return 0;
    }

    inline bool Setup()
    {
        // --- GetWorldFov hook (client.dll) ---
        // Hooks the renderer's FOV query directly so we return our value
        // without fighting the game's per-tick camera/weapon FOV writes.
        uintptr_t callSite = Mem::FindPattern(L"client.dll", Signatures::SetWorldFov);
        if (callSite)
        {
            int32_t rel = *reinterpret_cast<int32_t*>(callSite + 1);
            pFovHookTarget = reinterpret_cast<void*>(callSite + 5 + rel);
            if (MH_CreateHook(pFovHookTarget,
                              reinterpret_cast<void*>(hkGetWorldFov),
                              reinterpret_cast<void**>(&oGetWorldFov)) == MH_OK)
            {
                MH_STATUS st = MH_EnableHook(pFovHookTarget);
                fovHooked = (st == MH_OK || st == MH_ERROR_ENABLED);
            }
        }

        // --- OverrideView hook (client.dll) via MinHook ---
        // Hooks CCSGOViewAdviceService::OverrideView to control the camera
        // for third person. Much more reliable than byte-patching the
        // camera-reset branch since it gives us direct camera control.
        uintptr_t ovAddr = Mem::FindPattern(L"client.dll", Signatures::OverrideView);
        if (ovAddr)
        {
            pOverrideViewTarget = reinterpret_cast<void*>(ovAddr);
            MH_STATUS st = MH_CreateHook(
                pOverrideViewTarget,
                &hkOverrideView,
                reinterpret_cast<void**>(&oOverrideView));
            if (st == MH_OK || st == MH_ERROR_ALREADY_CREATED)
            {
                st = MH_EnableHook(pOverrideViewTarget);
                overrideViewHooked = (st == MH_OK || st == MH_ERROR_ENABLED);
            }
        }

        // --- DrawSkyboxArray hook (scenesystem.dll) via MinHook ---
        // Entity writes to m_vTintColor don't work (renderer caches at setup).
        // This inline hook modifies color in the draw primitive buffer at
        // render time, which IS effective. The VMT transition only removes
        // Present/Present1 hooks, so this survives.
        uintptr_t skyAddr = Mem::FindPattern(L"scenesystem.dll", Signatures::DrawSkyboxArray);
        if (skyAddr)
        {
            pSkyHookTarget = reinterpret_cast<void*>(skyAddr);
            MH_STATUS st = MH_CreateHook(
                pSkyHookTarget,
                &hkDrawSkyboxArray,
                reinterpret_cast<void**>(&oDrawSkyboxArray));
            if (st == MH_OK || st == MH_ERROR_ALREADY_CREATED)
            {
                st = MH_EnableHook(pSkyHookTarget);
                skyHooked = (st == MH_OK || st == MH_ERROR_ENABLED);
            }
        }

        // --- Third-person ConVars ---
        // All CS2 ConVars register into tier0.dll's central CCvar list.
        // Use CreateInterface("VEngineCvar007") to get the CCvar object
        // and traverse its linked list to find each ConVar by name.
        // Fallback: use the known VEngineCvar007 static offset (0x3A33B0)
        // in tier0.dll if CreateInterface is unavailable.
        using CreateInterfaceFn = void*(__cdecl*)(const char*, int*);
        HMODULE hTier0 = GetModuleHandleW(L"tier0.dll");
        void*   pCCvar = nullptr;
        if (hTier0)
        {
            auto pCI = (CreateInterfaceFn)GetProcAddress(hTier0, "CreateInterface");
            if (pCI) pCCvar = pCI("VEngineCvar007", nullptr);
            // Fallback: direct static offset from cs2-dumper (tier0 VEngineCvar007)
            if (!pCCvar)
                pCCvar = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(hTier0) + 0x3A33B0);
        }
        if (pCCvar)
        {
            pCV_c_thirdpersonshoulder      = FindCvarValue(pCCvar, "c_thirdpersonshoulder");
            pCV_cam_idealdist              = FindCvarValue(pCCvar, "cam_idealdist");
            pCV_thirdpersonshoulderaimdist = FindCvarValue(pCCvar, "c_thirdpersonshoulderaimdist");
            pCV_thirdpersonshoulderdist    = FindCvarValue(pCCvar, "c_thirdpersonshoulderdist");
            pCV_thirdpersonshoulderheight  = FindCvarValue(pCCvar, "c_thirdpersonshoulderheight");
            pCV_thirdpersonshoulderoffset  = FindCvarValue(pCCvar, "c_thirdpersonshoulderoffset");
        }

        return overrideViewHooked || skyHooked;
    }

    inline void Shutdown()
    {
        // Disable third person input flag
        if (GameState::clientBase)
        {
            __try {
                Mem::SmartWrite<bool>(
                    GameState::clientBase + GameState::RVA_dwCSGOInput() + 0x229, false);
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        // Remove OverrideView hook
        if (overrideViewHooked && pOverrideViewTarget)
        {
            MH_DisableHook(pOverrideViewTarget);
            MH_RemoveHook(pOverrideViewTarget);
            oOverrideView = nullptr;
            pOverrideViewTarget = nullptr;
            overrideViewHooked = false;
        }

        if (skyHooked && pSkyHookTarget)
        {
            MH_DisableHook(pSkyHookTarget);
            MH_RemoveHook(pSkyHookTarget);
            oDrawSkyboxArray = nullptr;
            pSkyHookTarget = nullptr;
            skyHooked = false;
        }
        if (fovHooked && pFovHookTarget)
        {
            MH_DisableHook(pFovHookTarget);
            MH_RemoveHook(pFovHookTarget);
            oGetWorldFov   = nullptr;
            pFovHookTarget = nullptr;
            fovHooked      = false;
        }
    }
}
