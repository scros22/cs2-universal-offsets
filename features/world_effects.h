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
        float thirdPersonDist   = 200.f; // camera distance
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

    // Re-entrancy guard. The engine's DrawSkyboxArray internally queries
    // GetWorldFov to build the sky projection; if we return our high
    // user-FOV from inside that path the sun-angle floats at +0xF8..+0x104
    // (right next to where we patch RGB at +0xE8) compute out-of-range
    // and the skybox object NaN-poisons after a few seconds. Setting this
    // flag for the duration of the skybox call forces hkGetWorldFov to
    // pass through to the original so the skybox math sees the real FOV
    // while gameplay still sees our overridden value.
    //
    // DrawSkyboxArray and the FOV query both run on the render thread, so
    // a plain inline bool is sufficient (no thread_local — that path can
    // trip TLS-callback issues in some manually-mapped injection setups).
    inline bool g_inSkyboxDraw = false;

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
        // Never override FOV during skybox setup — see g_inSkyboxDraw note.
        if (g_inSkyboxDraw) return orig;
        __try {
            if (!GameState::clientBase) return orig;
            uintptr_t lp = GameState::GetLocalPawn();
            if (!lp) return orig;
            bool scoped = Mem::Read<bool>(lp + Offsets::m_bIsScoped);
            if (scoped) return orig;
            // Clamp to sane range. Going past ~160 widens the frustum enough
            // that scenesystem culls in light/particle objects that aren't
            // expected on the render path and occasionally crashes.
            float v = cfg.fovValue;
            if (v < 1.f)   v = 1.f;
            if (v > 160.f) v = 160.f;
            return v;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return orig; }
    }

    // ---------------------------------------------------------------
    // RenderDecals hook (client.dll, sub_1810EA0E0) — always-on.
    //
    // Returning nullptr from this per-view decal-render dispatcher
    // skips ALL decal submission for that view: no blood splatters,
    // no bullet impacts, no scorch marks, no sprays. Cleanest visual
    // upgrade in the project (zero perf cost — we LITERALLY don't
    // submit the decal pass).
    //
    // Forced on for the lifetime of the cheat. No menu toggle: this
    // is consistent with how internals usually treat decals (always
    // off — they only ever obscure player models or fake hits onto
    // walls, never an information win for the user).
    //
    // Verified single match in client.dll on build 14155.
    // Args (from IDA decompile): __int64 ctx, __int64** view, char
    // pass_a, char pass_b. Returns _BYTE* (vanilla returns the render
    // list head; nullptr is treated by the engine as "this pass
    // produced nothing" and skipped).
    // ---------------------------------------------------------------
    using RenderDecalsFn = void*(__fastcall*)(__int64, __int64**, char, char);
    inline RenderDecalsFn oRenderDecals = nullptr;
    inline bool  decalsHooked          = false;
    inline void* pRenderDecalsTarget   = nullptr;

    inline void* __fastcall hkRenderDecals(
        __int64 ctx, __int64** view, char pa, char pb)
    {
        // Hard-skip: every decal pass returns nullptr unconditionally.
        // If we ever want a runtime toggle, gate this on a cfg bool
        // and fall through to oRenderDecals when off.
        (void)ctx; (void)view; (void)pa; (void)pb;
        return nullptr;
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
        // Tighter bound: real skybox draws have count <= 8. The previous
        // < 100 cap was permissive enough to scribble into unrelated
        // primitive slots on cubemap-rebuild calls.
        if (cfg.skyEnabled && drawPrimitive && count > 0 && count <= 16)
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
        // Hold the re-entrancy guard across the original call — the engine
        // queries FOV from inside this function for sky projection setup.
        // Wrap with SEH so a fault inside the original can't leave the flag
        // stuck true (which would permanently disable our FOV override).
        g_inSkyboxDraw = true;
        __try {
            if (oDrawSkyboxArray) oDrawSkyboxArray(a1, a2, drawPrimitive, count, a5, a6, a7);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        g_inSkyboxDraw = false;
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
    // Third person — invoke the game's native `thirdperson` /
    // `firstperson` ConCommand handlers directly. This routes through
    // the engine's own camera pipeline (sub_180AC6CA0 / sub_180AC6EC0)
    // so we get smooth interpolation, world collision, view-model
    // hiding, and proper local-pawn rendering for free — no hook,
    // no signature, no per-frame origin math.
    //
    // Reverse engineered (current build, IDA-validated 2026-04-25):
    //   sub_180AC8C30 ("thirdperson" cmd handler)  RVA 0x00AC8C30
    //     - sets *(pInput + 0x229) = 1            // enable flag
    //     - saves eye origin into pInput[+0x230]  // camera anchor
    //     - writes 30.0f to pInput[+0x238]
    //     - clears pInput[+0x6A8] transition flag
    //     - calls localPawn->vtable[+0x9C8](true) // render local pawn
    //   sub_180AC8B50 ("firstperson" cmd handler) RVA 0x00AC8B50
    //     - clears *(pInput + 0x229) = 0
    //     - clears pInput[+0x6A8]
    //     - calls localPawn->vtable[+0x9C8](false) // hide local pawn
    //     - broadcasts cleanup
    // Build 14154 RVAs were 0xAC8BD0 / 0xAC8AF0 (drift +0x60).
    // Both addresses are validated by prologue check; if the cached RVA
    // points elsewhere, ResolveThirdPersonHandlers falls back to the
    // signature scan (sigs in core/signatures.h are still valid).
    //
    // The CInput global (`off_1820613C0` in IDA) lives at
    //   *(uintptr_t*)(clientBase + 0x20613C0)
    // i.e. it is a POINTER, not the embedded struct (the cs2-dumper
    // `dwCSGOInput = 0x23386E0` value is unrelated — points to a
    // C_Item entity reservoir).
    // ---------------------------------------------------------------

    using ThirdPersonCmdFn = void(__fastcall*)();
    inline ThirdPersonCmdFn pThirdPersonOn  = nullptr;
    inline ThirdPersonCmdFn pThirdPersonOff = nullptr;

    constexpr std::ptrdiff_t kThirdPersonOn_RVA  = 0xAC8C30;   // updated 2026-04-25
    constexpr std::ptrdiff_t kThirdPersonOff_RVA = 0xAC8B50;   // updated 2026-04-25
    constexpr std::ptrdiff_t kCInputPtr_RVA      = 0x20613C0;  // off_1820613C0 — unchanged
    constexpr std::ptrdiff_t kCInput_ThirdPerson  = 0x229;  // enable gate  (byte)
    constexpr std::ptrdiff_t kCInput_ThirdActive  = 0x228;  // active flag  (byte) — set by camera-init path
    constexpr std::ptrdiff_t kCInput_ThirdInited  = 0x22A;  // initialized  (byte) — prevents re-init each frame
    constexpr std::ptrdiff_t kCInput_TransitionA  = 0x6A8;  // transition   (dword) — cleared on toggle
    constexpr std::ptrdiff_t kCInput_CurrentSlotDword = 0xB50; // dword index used by camera code
    constexpr std::ptrdiff_t kCInput_SlotStride = 0x928; // 2344 bytes per slot

    // Get the live CInput struct (deref the global pointer). Returns 0 if
    // the global hasn't been populated yet (early in process startup) or
    // if the dereferenced pointer obviously points at unmapped memory.
    inline uintptr_t GetCInput()
    {
        if (!GameState::clientBase) return 0;
        __try {
            uintptr_t p = Mem::Read<uintptr_t>(GameState::clientBase + kCInputPtr_RVA);
            // Cheap sanity: user-mode allocations on x64 sit between
            // 0x00010000 and 0x00007FFFFFFFFFFF. Reject obvious garbage
            // (NULL, low canonical, kernel-half) before the caller pokes it.
            if (p < 0x10000ULL || p > 0x00007FFFFFFFFFFFULL) return 0;
            return p;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
    }

    // Resolve the engine's thirdperson/firstperson handlers (idempotent).
    // Strategy:
    //   1) Try the known RVA and validate the prologue is `48 83 EC ??`
    //      (sub rsp, imm8) — both handlers start that way.
    //   2) Fall back to a unique signature scan on client.dll if the RVA
    //      moves on a future patch. Sig matches the precondition gate +
    //      flag write, which is structurally stable across builds.
    inline void ResolveThirdPersonHandlers()
    {
        if (pThirdPersonOn && pThirdPersonOff) return;
        if (!GameState::clientBase) return;

        auto validatePrologue = [](uintptr_t ea) -> bool {
            __try {
                uint32_t hdr = Mem::Read<uint32_t>(ea);
                // 48 83 EC ??  (sub rsp, imm8)  — low 24 bits = 00 EC 83 48 LE
                return (hdr & 0x00FFFFFF) == 0x00EC8348;
            } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
        };

        // --- ON handler ---
        if (!pThirdPersonOn)
        {
            uintptr_t ea = GameState::clientBase + kThirdPersonOn_RVA;
            if (validatePrologue(ea))
                pThirdPersonOn = reinterpret_cast<ThirdPersonCmdFn>(ea);
            else
            {
                uintptr_t found = Mem::FindPattern(L"client.dll", Signatures::ThirdPersonOnHandler);
                if (found && validatePrologue(found))
                    pThirdPersonOn = reinterpret_cast<ThirdPersonCmdFn>(found);
            }
        }

        // --- OFF handler ---
        if (!pThirdPersonOff)
        {
            uintptr_t ea = GameState::clientBase + kThirdPersonOff_RVA;
            if (validatePrologue(ea))
                pThirdPersonOff = reinterpret_cast<ThirdPersonCmdFn>(ea);
            else
            {
                uintptr_t found = Mem::FindPattern(L"client.dll", Signatures::ThirdPersonOffHandler);
                if (found && validatePrologue(found))
                    pThirdPersonOff = reinterpret_cast<ThirdPersonCmdFn>(found);
            }
        }
    }

    // Forward declare ConVar value pointer (defined above for shoulder cvars)
    // so RunThirdPerson can write the live camera distance each tick.
    extern uintptr_t pCV_cam_idealdist;

    // Called from Tick. Drives the toggle through the engine's own path
    // (smooth interp, collision, viewmodel hide), keeps cam_idealdist in
    // sync with the slider, AND re-stamps the enable flag every tick when
    // on so engine-side resets (round restart, respawn, map change, kill)
    // can't silently turn 3p back off without us noticing.
    inline void RunThirdPerson()
    {
        static bool wasOn = false;
        static UINT64 cvarTick = 0;
        if (!GameState::clientBase) {
            wasOn = false;
            return;
        }

        const bool wantOn = cfg.thirdPerson;

        // Hard safety gate: never run third-person camera logic unless the
        // local pawn is valid and alive. This avoids lobby/menu crashes when
        // users enable third person before spawning in-game.
        bool liveLocalPawn = false;
        __try {
            uintptr_t lp = GameState::GetLocalPawn();
            if (lp)
            {
                int hp = Mem::Read<int>(lp + Offsets::m_iHealth);
                liveLocalPawn = (hp > 0 && hp <= 200);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            liveLocalPawn = false;
        }

        if (!liveLocalPawn)
        {
            // If we left in-game, forget transition state and avoid calling
            // engine camera handlers on invalid globals.
            wasOn = false;
            return;
        }

        ResolveThirdPersonHandlers();
        uintptr_t pInput = GetCInput();
        if (!pInput) { wasOn = false; return; }

        auto stampThirdPersonFlag = [&](uint8_t v)
        {
            // Engine camera path reads +0x229 with a per-slot stride:
            // *(pInput + 0x229 + 0x928 * slot). Keep both base and
            // active slot latched so the gate can't mismatch/flicker.
            Mem::SmartWrite<uint8_t>(pInput + kCInput_ThirdPerson, v);
            int slot = Mem::Read<int>(pInput + kCInput_CurrentSlotDword);
            if (slot > 0 && slot < 8)
                Mem::SmartWrite<uint8_t>(
                    pInput + kCInput_ThirdPerson + (kCInput_SlotStride * slot), v);
        };

        if (!wantOn)
        {
            if (!wasOn) return;
            __try {
                if (pThirdPersonOff) pThirdPersonOff();
                stampThirdPersonFlag(0);
                Mem::SmartWrite<uint32_t>(pInput + kCInput_TransitionA, 0);
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
            wasOn = false;
            return;
        }

        // === wantOn ===

        // Slider clamp: matches menu range (50..600). Anything outside
        // these bounds risks NaN-ing the camera lerp on ConVar write.
        float safeDist = cfg.thirdPersonDist;
        if (safeDist <  50.f) safeDist =  50.f;
        if (safeDist > 600.f) safeDist = 600.f;

        // Low-frequency shoulder cvar latching (200 ms).
        UINT64 now = GetTickCount64();
        if (now - cvarTick > 200)
        {
            cvarTick = now;
            __try {
                if (pCV_c_thirdpersonshoulder)
                    Mem::SmartWrite<int>(pCV_c_thirdpersonshoulder, 1);
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        // Push ideal distance every tick so the slider is always live.
        if (pCV_cam_idealdist)
        {
            __try { Mem::SmartWrite<float>(pCV_cam_idealdist, safeDist); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        if (!wasOn)
        {
            // Edge: invoke engine handler ONCE on the off→on transition.
            // The handler does the heavy lifting (camera anchor, viewmodel
            // hide, localpawn render-self toggle). After this we just
            // latch the enable flag every tick so engine-side resets
            // (round restart, respawn, map change) can't silently flip
            // us back to first-person.
            __try { if (pThirdPersonOn) pThirdPersonOn(); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
            wasOn = true;
        }

        // Steady-state maintenance: keep enable gate latched + clear the
        // engine's transition flag so it doesn't try to fade us back to
        // first-person after a round event. Do NOT force the engine
        // "active" byte at +0x228 — that's owned by the camera processor
        // and writing it desyncs input/mouse handling.
        __try {
            stampThirdPersonFlag(1);
            Mem::SmartWrite<uint32_t>(pInput + kCInput_TransitionA, 0);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
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
    inline uintptr_t pCV_mat_fullbright      = 0;  // legacy +0x40 slot (kept for compat with old code paths)
    inline uintptr_t pCV_mat_fullbright_obj  = 0;  // cvar object pointer (for WriteCvarInt)
    inline int       lastFullbrightWritten   = -1; // throttle to avoid per-frame writes

    // Forward declaration: used by RunFullbright before helper definition below.
    inline void WriteCvarInt(uintptr_t cv, int value);

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
    // Fullbright — RE-ENABLED on build 14154 (2026-04-25).
    //
    // Root cause of the prior breakage (now understood): the previous
    // implementation wrote to cv+0x40 only, but the renderer reads from
    // cv+0x58 via Source 2's canonical resolver `sub_1804ACB70`. mat_
    // fullbright is also registered FCVAR_CHEAT (0x400), so the gate
    // `(flags & 0x400) == 0` in scenesystem.dll sub_180187150 skipped the
    // fullbright branch even when our value landed.
    //
    // Verified in IDA on scenesystem.dll build 14154:
    //   - Reader: sub_180187150 (xref to "mat_fullbright" string).
    //     Gate: `if (cv && (cv[+0x30] & 0x400) == 0) { v=resolve(cv); ... }`
    //   - Resolver sub_1804ACB70: returns `cv + 88` (= cv + 0x58) for
    //     non-indexed cvars (mat_fullbright has no 0x8000 bit).
    //
    // WriteCvarInt strips both FCVAR_CHEAT and FCVAR_DEVELOPMENTONLY at
    // cv+0x30, then writes cv+0x58 (modern) AND cv+0x40 (legacy mirror).
    // ---------------------------------------------------------------
    inline void RunFullbright()
    {
        if (!pCV_mat_fullbright_obj) return;
        const int desired = cfg.fullbright ? 1 : 0;
        if (desired == lastFullbrightWritten) return; // throttle
        WriteCvarInt(pCV_mat_fullbright_obj, desired);
        lastFullbrightWritten = desired;
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
                            return reinterpret_cast<uintptr_t>(cv) + 0x40;  // value at +0x40 (legacy mirror; works for bool cvars)
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
                i = *reinterpret_cast<uint16_t*>(ep + 10); // next index
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return 0;
    }

    // ---------------------------------------------------------------
    // FindCvar — returns the ConVar object pointer (NOT the value slot).
    //
    // Reverse-engineered from scenesystem.dll build 14154 sub_1804ACB70
    // (the canonical Source 2 ConVar value resolver):
    //   cv+0x00  const char*  name
    //   cv+0x08  void*        default-value-block (resolver fallback)
    //   cv+0x28  int16        indexed stride table key
    //   cv+0x30  uint32       flags  (FCVAR_CHEAT=0x400, DEVONLY=0x4000,
    //                                  per-user/indexed=0x8000)
    //   cv+0x58  T            primary value slot for non-indexed cvars
    //                          (resolver: return v2 + 88 when 0x8000 unset)
    //
    // For FCVAR_CHEAT cvars we MUST strip the cheat flag at cv+0x30
    // before writing or the renderer's gate `(flags & 0x400) == 0`
    // skips our value entirely — this is exactly why mat_fullbright
    // appeared not to work even after both +0x40 and +0x58 writes.
    // ---------------------------------------------------------------
    inline uintptr_t FindCvar(void* pCCvar, const char* name)
    {
        if (!pCCvar || !name) return 0;
        __try {
            uintptr_t lb    = reinterpret_cast<uintptr_t>(pCCvar) + 0x40;
            void*     elems = *reinterpret_cast<void**>(lb);
            uint16_t  head  = *reinterpret_cast<uint16_t*>(lb + 0x10);
            if (!elems || head == 0xFFFF) return 0;

            int guard = 8192;
            for (uint16_t i = head; i != 0xFFFF && guard-- > 0; )
            {
                uint8_t* ep = reinterpret_cast<uint8_t*>(elems) + (uintptr_t)i * 16;
                __try {
                    auto* cv = *reinterpret_cast<void**>(ep);
                    if (cv)
                    {
                        const char* n = *reinterpret_cast<const char**>(cv);
                        if (n && strcmp(n, name) == 0)
                            return reinterpret_cast<uintptr_t>(cv);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
                i = *reinterpret_cast<uint16_t*>(ep + 10);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return 0;
    }

    // Write an int to a ConVar object (NOT a value slot). Strips
    // FCVAR_CHEAT (0x400) and FCVAR_DEVELOPMENTONLY (0x4000) at cv+0x30
    // first so the renderer's flag gate doesn't skip us, then writes the
    // value to BOTH the modern slot (cv+0x58 — what sub_1804ACB70 returns)
    // AND the legacy slot (cv+0x40 — some bool cvars still read here).
    inline void WriteCvarInt(uintptr_t cv, int value)
    {
        if (!cv) return;
        __try {
            // Strip cheat-gate bits so the renderer's `(flags & 0x400) == 0`
            // check passes. Leave all other bits intact.
            uint32_t* pFlags = reinterpret_cast<uint32_t*>(cv + 0x30);
            uint32_t  cur    = *pFlags;
            uint32_t  next   = cur & ~(uint32_t)(0x400 | 0x4000);
            if (next != cur) Mem::SmartWrite<uint32_t>(reinterpret_cast<uintptr_t>(pFlags), next);

            // Modern Source 2 value slot (primary).
            Mem::SmartWrite<int>(cv + 0x58, value);
            // Legacy union mirror (bool cvars, some reader paths).
            Mem::SmartWrite<int>(cv + 0x40, value);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Disable PVS / vis-cluster culling so chams + ESP render through
    // walls at any distance.
    //
    // Pattern hits one site inside CRenderingWorldSession::OnLoopActivate
    // (engine2.dll @ 0x18023D3D2 in build 14154):
    //   lea rcx, [g_visMgrPtr]   ; +3 holds rel32 to singleton-of-singleton
    //   xor edx, edx
    //   call qword ptr [rax+30h] ; vtable[6]
    // The g_visMgrPtr is a `T**` — first deref gives the singleton, second
    // gives its vtable. vtable[6] is a recursive routine that stamps every
    // leaf in the visibility tree to the supplied bool. Passing `false`
    // turns PVS culling off (every leaf reads as visible).
    //
    // Result tracked in g_pvsDisabled so the menu / watermark can reflect
    // status if we ever want to expose it.
    // ---------------------------------------------------------------
    inline bool g_pvsDisableTried = false;
    inline bool g_pvsDisabled     = false;

    inline bool TryDisablePvs()
    {
        if (g_pvsDisableTried) return g_pvsDisabled;
        g_pvsDisableTried = true;

        __try {
            uintptr_t hit = Mem::FindPattern(L"engine2.dll", Signatures::DisablePvsAccessor);
            if (!hit) return false;

            // lea rcx, [rip+rel32] : opcode at hit, rel32 at hit+3, next inst at hit+7
            int32_t rel = *reinterpret_cast<const int32_t*>(hit + 3);
            uintptr_t pSingletonHolder = (hit + 7) + static_cast<intptr_t>(rel);

            // Sanity-bound to engine2.dll image — a misfiring sig must not
            // splatter random memory.
            HMODULE hEng = GetModuleHandleW(L"engine2.dll");
            if (!hEng) return false;
            MODULEINFO mi{};
            if (!GetModuleInformation(GetCurrentProcess(), hEng, &mi, sizeof(mi)))
                return false;
            uintptr_t modBase = reinterpret_cast<uintptr_t>(hEng);
            uintptr_t modEnd  = modBase + mi.SizeOfImage;
            if (pSingletonHolder < modBase || pSingletonHolder >= modEnd) return false;

            // First deref: g_visMgrPtr -> singleton instance
            void* pMgr = *reinterpret_cast<void**>(pSingletonHolder);
            if (!pMgr) return false;

            // Second deref: instance -> vtable
            void** vtbl = *reinterpret_cast<void***>(pMgr);
            if (!vtbl) return false;

            // vtable[6] expected inside engine2 .text — guard against a
            // stale/uninitialized object.
            uintptr_t fn = reinterpret_cast<uintptr_t>(vtbl[6]);
            if (fn < modBase || fn >= modEnd) return false;

            using SetVisibilityFn = void(__fastcall*)(void*, bool);
            reinterpret_cast<SetVisibilityFn>(fn)(pMgr, false);

            g_pvsDisabled = true;
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline bool Setup()
    {
        // --- GetWorldFov hook (client.dll) ---
        // Hook the FOV resolver (sub_18080BE50) directly — it returns the
        // final float the renderer uses, after fov_cs_debug + weapon zoom
        // math. We read its value (so weapon-zoom transitions still work)
        // and clamp/override only when not scoped. The legacy E8-call
        // SetWorldFov sig is dead on 14154; GetWorldFovResolver is the
        // current entry point.
        uintptr_t fovAddr = Mem::FindPattern(L"client.dll", Signatures::GetWorldFovResolver);
        if (fovAddr)
        {
            pFovHookTarget = reinterpret_cast<void*>(fovAddr);
            if (MH_CreateHook(pFovHookTarget,
                              reinterpret_cast<void*>(hkGetWorldFov),
                              reinterpret_cast<void**>(&oGetWorldFov)) == MH_OK)
            {
                MH_STATUS st = MH_EnableHook(pFovHookTarget);
                fovHooked = (st == MH_OK || st == MH_ERROR_ENABLED);
            }
        }

        // --- Third person handlers (RVA-bound, no signature scan) ---
        // Resolve the engine's `thirdperson` / `firstperson` ConCommand
        // handlers so RunThirdPerson() can call them directly. Failure is
        // non-fatal — RunThirdPerson falls back to a manual flag flip.
        ResolveThirdPersonHandlers();

        // --- Disable PVS / vis-cluster culling (engine2.dll) ----------
        // Walk to the visibility manager singleton via the setter sigscan
        // inside CRenderingWorldSession::OnLoopActivate, then call
        // vtable[6](mgr, false). vtable[6] is a recursive walk over the
        // vis tree that stamps every leaf as visible (decompiled at
        // engine2.dll!sub_180235950). After this, PVS leaf culling is
        // dead — chams, ESP, and any rendered geometry stays drawn at
        // any distance through any wall, regardless of which leaf the
        // camera is in. We don't hook anything; we just call the
        // existing engine vfunc once at startup. Idempotent — calling
        // again with `false` writes the same byte.
        TryDisablePvs();

        // --- No-decals (client.dll) ---
        // Force-skip every per-view decal-render pass for the lifetime
        // of the cheat. Detour returns nullptr unconditionally so the
        // engine treats each pass as having produced no work. Removes
        // blood, bullet impacts, scorch marks, sprays.
        uintptr_t decalsAddr = Mem::FindPattern(L"client.dll", Signatures::RenderDecals);
        if (decalsAddr)
        {
            pRenderDecalsTarget = reinterpret_cast<void*>(decalsAddr);
            MH_STATUS st = MH_CreateHook(
                pRenderDecalsTarget,
                reinterpret_cast<void*>(&hkRenderDecals),
                reinterpret_cast<void**>(&oRenderDecals));
            if (st == MH_OK || st == MH_ERROR_ALREADY_CREATED)
            {
                st = MH_EnableHook(pRenderDecalsTarget);
                decalsHooked = (st == MH_OK || st == MH_ERROR_ENABLED);
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
            pCV_mat_fullbright_obj    = FindCvar(pCCvar, "mat_fullbright");
        }

        return skyHooked;
    }

    inline void Shutdown()
    {
        // Disable third person — call the engine's native firstperson
        // handler so the local pawn render flag and broadcast cleanup
        // happen the same way the game does it.
        if (cfg.thirdPerson && pThirdPersonOff)
        {
            __try { pThirdPersonOff(); } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
        else if (GameState::clientBase)
        {
            // Fallback flag clear via the deref'd CInput pointer.
            __try {
                uintptr_t pInput = GetCInput();
                if (pInput) Mem::SmartWrite<uint8_t>(pInput + kCInput_ThirdPerson, 0);
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
        cfg.thirdPerson = false;

        if (skyHooked && pSkyHookTarget)
        {
            MH_DisableHook(pSkyHookTarget);
            MH_RemoveHook(pSkyHookTarget);
            oDrawSkyboxArray = nullptr;
            pSkyHookTarget = nullptr;
            skyHooked = false;
        }
        if (decalsHooked && pRenderDecalsTarget)
        {
            MH_DisableHook(pRenderDecalsTarget);
            MH_RemoveHook(pRenderDecalsTarget);
            oRenderDecals        = nullptr;
            pRenderDecalsTarget  = nullptr;
            decalsHooked         = false;
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
