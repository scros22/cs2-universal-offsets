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
#include "../core/math.h"
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
        int   nightMode         = 0;       // 0=off, 1=night, 2=midnight, 3=sunset, 4=bloodmoon,
                                           // 5=aurora, 6=cyberpunk, 7=vaporwave, 8=hellfire

        // Asus Mode — solid bright sky color for max enemy silhouette
        // (overrides skyEnabled when on; uses fixed presets, not free color)
        int   asusMode          = 0;       // 0=off, 1=lime, 2=hot pink, 3=cyan, 4=red, 5=yellow

        // Anti-Fog — disables ALL map fog (huge visibility gain)
        bool  antiFog           = false;

        // No Shadows — kills env_global_light cascade shadows
        bool  noShadows         = false;

        // No Color Correction — disables map's mood color grading
        // (often dramatic visibility improvement on dark/dusty maps)
        bool  noColorCorrection = false;

        // Fullbright — mat_fullbright ConVar (scenesystem.dll). Disables
        // all lighting calculation, shows everything at base albedo.
        // Devastating visibility upgrade on dark maps.
        bool  fullbright        = false;

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
                    uintptr_t ent = GameState::GetEntityByIndex(i);
                    if (!ent) continue;

                    // Read CEntityIdentity
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;

                    // Read designer name pointer (CUtlSymbolLarge = const char*)
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
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
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                __try {
                    // Identify inferno by designer name (cheap pointer checks first)
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
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
                    // Build 14152+: skybox tint Vector3 moved from +0x100 → +0xE8.
                    // The old +0x100 slot is now a sun-angle float fed to V_sinf();
                    // writing RGB there poisons the renderer with NaN and crashes
                    // after ~60s. Layout (verified via scenesystem.dll decompile of
                    // sub_18014FB90 a.k.a. DrawSkyboxArray):
                    //   +0xE8 .. +0xF0  vec3 tint  (RGB, 3 floats)
                    //   +0xF4           int   mode  (1 or 2)
                    //   +0xF8 .. +0x104 four sun-angle floats (V_sinf inputs)
                    float* colorPtr = (float*)((char*)(*skyboxObjPtr) + 0xE8);

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
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
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
            { 10, 30, 50, 0.25f, 0.15f, 0.3f, 20, 90, 60, 200.f, 9000.f, 0.4f },     // 5: Aurora — deep blue + green
            { 80, 20, 100, 0.3f, 0.2f, 0.4f, 150, 30, 180, 150.f, 7000.f, 0.5f },    // 6: Cyberpunk — purple + magenta
            { 230, 90, 200, 0.45f, 0.3f, 0.55f, 90, 200, 220, 300.f, 10000.f, 0.4f },// 7: Vaporwave — pink + cyan
            { 200, 50, 5, 0.55f, 0.4f, 0.7f, 220, 90, 20, 200.f, 8000.f, 0.5f },     // 8: Hellfire — red + orange
        };

        int idx = cfg.nightMode;
        if (idx < 1 || idx > 8) return;
        const NightPreset& p = presets[idx];

        __try {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 0; i < 8192; ++i)
            {
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
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
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
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
        // Per a2x dumper (14152): m_iDesiredFOV is on CBasePlayerController at 0x784.
        // Writing both camera-services FOV and the controller's desired-FOV every
        // tick prevents the game's per-frame reset from clobbering our value.
        __try {
            uintptr_t lp = GameState::GetLocalPawn();
            uintptr_t lc = GameState::GetLocalController();
            if (!lp) return;
            bool scoped = Mem::Read<bool>(lp + Offsets::m_bIsScoped);
            if (scoped) return; // don't override while scoped
            uint32_t desired = (uint32_t)cfg.fovValue;

            uintptr_t camSvc = Mem::Read<uintptr_t>(lp + Offsets::m_pCameraServices);
            if (camSvc) {
                Mem::SmartWrite<uint32_t>(camSvc + Offsets::m_iFOV,      desired);
                Mem::SmartWrite<uint32_t>(camSvc + Offsets::m_iFOVStart, desired);
            }
            // Controller-level desired-FOV (the canonical source the renderer
            // reads from at the start of each frame). This is the field that
            // gets reset back to default when missing.
            if (lc) {
                Mem::SmartWrite<uint32_t>(lc + Offsets::m_iDesiredFOV_OnController, desired);
            }
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
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
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
        // Call original first — let engine set up default first-person camera
        if (oOverrideView)
            oOverrideView(thisPtr, viewSetup);

        if (!cfg.thirdPerson || !viewSetup || !GameState::clientBase) return;

        __try {
            uintptr_t lp = GameState::GetLocalPawn();
            if (!lp) return;

            int hp = Mem::Read<int>(lp + Offsets::m_iHealth);
            if (hp <= 0) return;

            // Enable engine third-person input flag — required so the local
            // model actually renders (otherwise we see headless camera floating).
            Mem::SmartWrite<bool>(
                GameState::clientBase + GameState::RVA_dwCSGOInput() + 0x229, true);

            // Read the eye position the engine just wrote into CViewSetup.
            uint8_t* vs = reinterpret_cast<uint8_t*>(viewSetup);
            Math::Vec3   eyePos   = *reinterpret_cast<Math::Vec3*>(vs + ViewSetupOffsets::Origin);
            Math::QAngle viewAng  = *reinterpret_cast<Math::QAngle*>(vs + ViewSetupOffsets::Angles);

            // Build forward vector from view angles. Source uses
            //   pitch = X (down +), yaw = Y, roll = Z (degrees)
            constexpr float kDeg2Rad = 3.14159265358979323846f / 180.f;
            float p  = viewAng.pitch * kDeg2Rad;
            float y  = viewAng.yaw   * kDeg2Rad;
            float cp = cosf(p), sp = sinf(p);
            float cy = cosf(y), sy = sinf(y);

            Math::Vec3 forward{ cp * cy, cp * sy, -sp };

            // Push camera backward from the eye position by user-configured
            // distance, then nudge it slightly upward so we look over the
            // shoulder/head instead of through it.
            float dist = cfg.thirdPersonDist;
            Math::Vec3 newOrigin{
                eyePos.x - forward.x * dist,
                eyePos.y - forward.y * dist,
                eyePos.z - forward.z * dist + 8.f
            };

            *reinterpret_cast<Math::Vec3*>(vs + ViewSetupOffsets::Origin) = newOrigin;
            // Keep angles unchanged — we still want to look the same direction.
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Disable third person flag when turned off (called from Tick)
    inline void RunThirdPerson()
    {
        static bool wasOn = false;
        if (!cfg.thirdPerson)
        {
            if (wasOn && GameState::clientBase)
            {
                __try {
                    Mem::SmartWrite<bool>(
                        GameState::clientBase + GameState::RVA_dwCSGOInput() + 0x229, false);
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
            wasOn = false;
        }
        else
        {
            wasOn = true;
        }
    }

    // ---------------------------------------------------------------
    // Anti-Fog — disables every env_fog_controller and env_cubemap_fog
    // entity in the world. Massive visibility upgrade on foggy maps
    // (Ancient, Anubis, Vertigo). Cheaper than custom fog because we
    // only flip one bool per ent.
    // ---------------------------------------------------------------
    inline void RunAntiFog()
    {
        if (!cfg.antiFog || !GameState::clientBase) return;

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 250) return;
        lastTick = now;

        __try {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 0; i < 2048; ++i)
            {
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
                    if (!namePtr) continue;
                    struct NB { char d[64]; };
                    NB nb = Mem::Read<NB>(namePtr);
                    nb.d[63] = '\0';

                    if (strstr(nb.d, "env_fog_controller"))
                    {
                        constexpr std::ptrdiff_t kFog = 0x608;
                        Mem::SmartWrite<bool>(ent + kFog + 0x64, false);   // m_bEnable = false
                        Mem::SmartWrite<float>(ent + kFog + 0x30, 0.f);    // m_flMaxDensity = 0
                        Mem::SmartWrite<float>(ent + kFog + 0x28, 99999.f);// m_flEnd huge
                    }
                    else if (strstr(nb.d, "env_cubemap_fog"))
                    {
                        Mem::SmartWrite<bool>(ent + 0x62C, false); // m_bActive = false
                        Mem::SmartWrite<float>(ent + 0x630, 0.f);  // m_flFogMaxOpacity = 0
                    }
                    else if (strstr(nb.d, "env_volumetric_fog_controller"))
                    {
                        // C_EnvVolumetricFogController (cs2-dumper schema):
                        //   m_flScattering   = 0x600
                        //   m_flDrawDistance = 0x610
                        //   m_bActive        = 0x64C
                        //   m_bStartDisabled = 0x674
                        Mem::SmartWrite<bool>(ent + 0x64C, false);  // m_bActive
                        Mem::SmartWrite<bool>(ent + 0x674, true);   // m_bStartDisabled
                        Mem::SmartWrite<float>(ent + 0x600, 0.f);   // m_flScattering
                        Mem::SmartWrite<float>(ent + 0x610, 0.f);   // m_flDrawDistance
                    }
                    else if (strstr(nb.d, "env_volumetric_fog_volume"))
                    {
                        // C_EnvVolumetricFogVolume:
                        //   m_bActive   = 0x600
                        //   m_flStrength= 0x620
                        Mem::SmartWrite<bool>(ent + 0x600, false);
                        Mem::SmartWrite<float>(ent + 0x620, 0.f);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // No Shadows — toggles known shadow ConVars via the global CCvar
    // value pointers cached at Setup(). ConVar route is safer than
    // schema poking on C_GlobalLight (whose embedded CLightComponent
    // offset is not reliably exposed by the public schema).
    // Cached at Setup(); harmless no-op if the ConVar isn't found.
    // ---------------------------------------------------------------
    inline uintptr_t pCV_r_shadows           = 0;
    inline uintptr_t pCV_cl_csm_enabled      = 0;
    inline uintptr_t pCV_cl_csm_world_shadows= 0;
    inline uintptr_t pCV_cl_csm_static_props = 0;
    inline uintptr_t pCV_cl_csm_rope_shadows = 0;
    inline uintptr_t pCV_cl_csm_sprite_shadows = 0;
    inline uintptr_t pCV_mat_fullbright      = 0;

    inline void RunNoShadows()
    {
        if (!cfg.noShadows) return;
        // Throttled — ConVars are sticky once set, but a few sub-systems
        // re-read them every frame, so periodically re-stamp.
        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 500) return;
        lastTick = now;

        __try {
            auto setInt = [](uintptr_t p, int v) {
                if (p) Mem::SmartWrite<int>(p, v);
            };
            setInt(pCV_r_shadows,            0);
            setInt(pCV_cl_csm_enabled,       0);
            setInt(pCV_cl_csm_world_shadows, 0);
            setInt(pCV_cl_csm_static_props,  0);
            setInt(pCV_cl_csm_rope_shadows,  0);
            setInt(pCV_cl_csm_sprite_shadows,0);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Fullbright — toggles mat_fullbright ConVar (lives in
    // scenesystem.dll, registered in the global tier0 CCvar). When 1
    // the renderer skips lighting and draws all surfaces at their
    // unlit base color. Single int write per ~500ms.
    // ---------------------------------------------------------------
    inline void RunFullbright()
    {
        // Always run while enabled (engine may re-stamp flags). When the
        // user toggles OFF, we explicitly write 0 back ONCE.
        static int lastDesired = -1;
        int desired = cfg.fullbright ? 1 : 0;

        // Skip entirely when we've already restored to 0 and stayed there.
        if (desired == 0 && lastDesired == 0) return;

        __try {
            if (pCV_mat_fullbright) {
                // CS2 Source-2 ConVar layout (verified IDA scenesystem.dll
                // sub_1804ACB70):
                //   cvar+0x30 : flags (DWORD) — FCVAR_CHEAT=0x400, PERTHREAD=0x8000
                //   cvar+0x58 : value storage (returned by sub_1804ACB70)
                // Our FindCvarValue returns cvar+0x40 which is the *legacy*
                // value union; modern ConVar<T> uses +0x58. Write BOTH so we
                // cover every read path the engine might take.
                uintptr_t cvarBase   = pCV_mat_fullbright - 0x40;
                uintptr_t flagsAddr  = cvarBase + 0x30;
                uintptr_t valueAddr  = cvarBase + 0x58;

                __try {
                    uint32_t flags = Mem::Read<uint32_t>(flagsAddr);
                    // Clear FCVAR_CHEAT (0x400) and FCVAR_DEVELOPMENTONLY (0x4000)
                    uint32_t clean = flags & ~0x4400u;
                    if (clean != flags)
                        Mem::SmartWrite<uint32_t>(flagsAddr, clean);
                } __except (EXCEPTION_EXECUTE_HANDLER) {}

                Mem::SmartWrite<int>(pCV_mat_fullbright, desired); // legacy slot
                Mem::SmartWrite<int>(valueAddr,         desired); // modern slot
            }
            lastDesired = desired;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // No Color Correction — most CS2 maps ship with one or more
    // color_correction entities that apply a LUT (warm dust on
    // Mirage, blue-grey on Anubis, etc). Disabling them is a
    // dramatic free visibility upgrade. We zero m_flMaxWeight,
    // clear m_bEnabled, and toggle m_bEnabledOnClient[0].
    // ---------------------------------------------------------------
    inline void RunNoColorCorrection()
    {
        if (!cfg.noColorCorrection || !GameState::clientBase) return;

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 300) return;
        lastTick = now;

        __try {
            for (int i = 0; i < 2048; ++i)
            {
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
                    if (!namePtr) continue;
                    struct NB { char d[64]; };
                    NB nb = Mem::Read<NB>(namePtr);
                    nb.d[63] = '\0';

                    if (strstr(nb.d, "color_correction"))
                    {
                        // C_ColorCorrection (offsets from sdk):
                        //   m_flMaxWeight        = 0x61C
                        //   m_flCurWeight        = 0x620
                        //   m_bEnabled           = 0x824
                        //   m_bEnabledOnClient[0]= 0x828
                        //   m_flCurWeightOnClient= 0x82C
                        Mem::SmartWrite<float>(ent + 0x61C, 0.f);
                        Mem::SmartWrite<float>(ent + 0x620, 0.f);
                        Mem::SmartWrite<bool>(ent + 0x824, false);
                        Mem::SmartWrite<bool>(ent + 0x828, false);
                        Mem::SmartWrite<float>(ent + 0x82C, 0.f);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Asus Mode — locks the sky to one of a few high-contrast solid
    // colors classically used to make enemy silhouettes pop. Mutually
    // exclusive with skyEnabled (this one wins) — applies via the
    // exact same env_sky path so it's stable.
    // ---------------------------------------------------------------
    inline void RunAsusMode()
    {
        if (cfg.asusMode == 0 || !GameState::clientBase) return;

        static UINT64 lastTick = 0;
        UINT64 now = GetTickCount64();
        if (now - lastTick < 250) return;
        lastTick = now;

        struct AsusPreset { uint8_t r, g, b; float bright; };
        static const AsusPreset presets[] = {
            { 0,   0,   0,   0.0f }, // 0 unused
            { 80,  255, 30,  1.5f }, // 1 Lime
            { 255, 30,  200, 1.5f }, // 2 Hot Pink
            { 30,  220, 255, 1.5f }, // 3 Cyan
            { 255, 30,  30,  1.5f }, // 4 Red
            { 255, 240, 50,  1.6f }, // 5 Yellow
        };
        int idx = cfg.asusMode;
        if (idx < 1 || idx > 5) return;
        const AsusPreset& p = presets[idx];

        __try {
            for (int i = 0; i < 1024; ++i)
            {
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;
                __try {
                    uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
                    if (!identity) continue;
                    uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
                    if (!namePtr) continue;
                    struct NB { char d[64]; };
                    NB nb = Mem::Read<NB>(namePtr);
                    nb.d[63] = '\0';

                    if (strstr(nb.d, "env_sky"))
                    {
                        uint32_t c = (uint32_t)p.r | ((uint32_t)p.g << 8)
                                   | ((uint32_t)p.b << 16) | (255u << 24);
                        Mem::SmartWrite<uint32_t>(ent + Offsets::m_vTintColor, c);
                        Mem::SmartWrite<uint32_t>(ent + Offsets::m_vTintColorLightingOnly, c);
                        Mem::SmartWrite<float>(ent + Offsets::m_flSkyBrightnessScale, p.bright);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
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
        RunAsusMode();
        RunFogOverride();
        RunAntiFog();
        RunNoShadows();
        RunNoColorCorrection();
        RunFullbright();
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

            // Shadow ConVars (No Shadows feature). Any/all may not exist on
            // every build; FindCvarValue returns 0 in that case which is fine.
            pCV_r_shadows             = FindCvarValue(pCCvar, "r_shadows");
            pCV_cl_csm_enabled        = FindCvarValue(pCCvar, "cl_csm_enabled");
            pCV_cl_csm_world_shadows  = FindCvarValue(pCCvar, "cl_csm_world_shadows");
            pCV_cl_csm_static_props   = FindCvarValue(pCCvar, "cl_csm_static_props");
            pCV_cl_csm_rope_shadows   = FindCvarValue(pCCvar, "cl_csm_rope_shadows");
            pCV_cl_csm_sprite_shadows = FindCvarValue(pCCvar, "cl_csm_sprite_shadows");
            pCV_mat_fullbright        = FindCvarValue(pCCvar, "mat_fullbright");
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
