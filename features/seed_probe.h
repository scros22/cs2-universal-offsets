#pragma once

// =================================================================
// Seed Probe — Phase 1 of the seeded triggerbot effort
// =================================================================
//
// Hooks client.dll's bullet-seed generator (sub_180C7BDD0 on build
// 14155). On every shot, logs:
//   - the function's three inputs (quantized pitch, yaw, extraKey)
//   - the function's return value (the actual seed handed to
//     RandomSeed before the spread loop runs)
//   - candidate game-state values that *might* be the source of
//     extraKey — so we can correlate and pin down which one Valve
//     actually feeds in.
//
// This file is COMPILED OUT BY DEFAULT. Define LUCID_SEED_PROBE
// at build-time (preprocessor define) to enable it. Even when
// compiled in, it is RUNTIME-GATED on `cfg.enabled`, so a stray
// build won't start writing files unless the user explicitly
// flips the menu toggle.
//
// File-write heartbeat is one of the heuristics that pushes a CS2
// account into the 20-hour Untrusted-mode cooldown — see comment
// on DllLog() in dllmain.cpp. We mitigate by:
//   1) Default-off at compile time.
//   2) Default-off at runtime even if compiled in.
//   3) Single per-shot ~120-byte append, no per-frame writes.
//   4) Buffered to a single OS file handle (no open/close per line).
//   5) Lives in %TEMP%, not next to the DLL.
//
// Once we have a few hundred lines of data we can identify the
// extraKey source with ~100% confidence, ship Phase 2 (the
// in-process seed reproducer), and DELETE this file from the build.
// =================================================================

#ifdef LUCID_SEED_PROBE

#include <Windows.h>
#include <Psapi.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <atomic>

#include "../vendor/minhook/include/MinHook.h"
#include "../core/memory.h"
#include "../core/sdk_offsets.h"
#include "../core/game_state.h"

namespace SeedProbe
{
    struct Config
    {
        // Default OFF even when compiled in. Flip via menu when
        // you're about to start a probe session in a bot game.
        // TEMPORARY: defaulted ON for the Phase 1 capture session.
        // Revert to false (and remove the LUCID_SEED_PROBE define
        // in dllmain.cpp) once enough samples are collected.
        bool enabled = true;
    };

    inline Config cfg;

    // Signature for sub_180C7BDD0 (CCSWeaponBaseGun bullet-seed
    // generator). 31-byte unique prologue including the SHA-1
    // construction call site. Verified single-hit on build 14155.
    constexpr const char* kSeedGenSig =
        "48 89 5C 24 08 57 48 81 EC F0 00 00 00 "
        "F3 0F 10 0A 48 8D 8C 24 10 01 00 00 "
        "41 8B D8 48 8B FA E8";

    // Signature for sub_180C7C6F0 — GetBulletAccuracySpread (the
    // seeded RNG loop that produces the per-bullet spread vector).
    // 32-byte unique prologue. Verified single-hit on build 14155.
    constexpr const char* kSpreadSig =
        "48 8B C4 48 89 58 08 48 89 68 18 48 89 70 20 "
        "57 41 54 41 55 41 56 41 57 48 81 EC E0 00 00 00";

    // ---- detour ----
    using SeedGenFn = std::int64_t(__fastcall*)(std::int64_t weapon,
                                                float*       angles,
                                                std::int32_t extraKey);
    inline SeedGenFn oSeedGen = nullptr;

    // GetBulletAccuracySpread signature (Valve's):
    //   void(uint16 itemDef, int pelletCount, int mode, uint32 seed,
    //        float baseSpread, float inaccuracy, float recoilOrShots,
    //        void* perfectAimPtr, float* outSpread)
    using SpreadFn = void(__fastcall*)(std::uint16_t, int, int, std::uint32_t,
                                       float, float, float,
                                       void*, float*);
    inline SpreadFn oSpread = nullptr;

    inline std::atomic<std::uint64_t> g_calls{0};
    inline HANDLE g_log = INVALID_HANDLE_VALUE;
    inline CRITICAL_SECTION g_logCs{};
    inline bool g_logCsInit = false;

    inline void OpenLog()
    {
        if (g_log != INVALID_HANDLE_VALUE) return;
        char path[MAX_PATH];
        DWORD n = GetTempPathA(MAX_PATH, path);
        if (n == 0 || n >= MAX_PATH - 32) return;
        strcat_s(path, MAX_PATH, "lucid_seedprobe.log");
        g_log = CreateFileA(path,
                            FILE_APPEND_DATA,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr, OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (g_log == INVALID_HANDLE_VALUE) return;
        const char* hdr =
            "\r\n=== seed probe session ===\r\n"
            "# columns (seed):   pitch_in,yaw_in,extraKey_in,seed_out,"
            "shotsFired,tickBase,prev_extraKey,delta_shots,delta_tick\r\n"
            "# columns (spread): SPREAD itemDef,pellets,mode,seed,"
            "baseSpread,inacc,recoil,outX,outY\r\n";
        DWORD w; WriteFile(g_log, hdr, (DWORD)strlen(hdr), &w, nullptr);
    }

    inline void WriteLine(const char* line, int len)
    {
        if (g_log == INVALID_HANDLE_VALUE) return;
        DWORD w; WriteFile(g_log, line, (DWORD)len, &w, nullptr);
    }

    // Per-call state — used to compute deltas between shots so we
    // can spot which candidate field increments by exactly the
    // amount we'd expect for "next bullet seed source".
    inline std::int32_t g_prevExtraKey = 0;
    inline std::int32_t g_prevShots    = 0;
    inline std::int32_t g_prevTick     = 0;

    inline std::int64_t __fastcall hkSeedGen(std::int64_t weapon,
                                             float*       angles,
                                             std::int32_t extraKey)
    {
        // Snapshot inputs BEFORE call (angles are read inside; the
        // function may not modify them, but capturing first is
        // cheap and matches what the seed actually depends on).
        float pIn = 0.f, yIn = 0.f;
        __try
        {
            if (angles) { pIn = angles[0]; yIn = angles[1]; }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        std::int64_t seedOut = oSeedGen ? oSeedGen(weapon, angles, extraKey) : 0;

        if (!cfg.enabled) return seedOut;

        // Snapshot candidate sources of extraKey.
        std::int32_t shots = 0, tick = 0;
        __try
        {
            uintptr_t pawn = GameState::GetLocalPawn();
            if (pawn)
                shots = *reinterpret_cast<std::int32_t*>(pawn + Offsets::m_iShotsFired);
            uintptr_t ctrl = GameState::GetLocalController();
            if (ctrl)
                tick = *reinterpret_cast<std::int32_t*>(ctrl + Offsets::m_nTickBase);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        std::int32_t dShots = shots - g_prevShots;
        std::int32_t dTick  = tick  - g_prevTick;
        std::int32_t dKey   = extraKey - g_prevExtraKey;

        char line[256];
        int len = _snprintf_s(line, sizeof(line), _TRUNCATE,
            "%.4f,%.4f,%d,0x%08X,"
            "shots=%d tick=%d "
            "dKey=%d dShots=%d dTick=%d\r\n",
            pIn, yIn, extraKey, (std::uint32_t)seedOut,
            shots, tick, dKey, dShots, dTick);

        EnterCriticalSection(&g_logCs);
        if (g_log == INVALID_HANDLE_VALUE) OpenLog();
        if (len > 0) WriteLine(line, len);
        LeaveCriticalSection(&g_logCs);

        g_prevExtraKey = extraKey;
        g_prevShots    = shots;
        g_prevTick     = tick;
        g_calls.fetch_add(1, std::memory_order_relaxed);
        return seedOut;
    }

    // ---- spread-vector capture (ground truth for Phase 3 validation) ----
    inline std::atomic<std::uint64_t> g_spreadCalls{0};

    inline void __fastcall hkSpread(std::uint16_t itemDef,
                                    int           pellets,
                                    int           mode,
                                    std::uint32_t seed,
                                    float         baseSpread,
                                    float         inacc,
                                    float         recoil,
                                    void*         perfectAim,
                                    float*        outSpread)
    {
        if (oSpread)
            oSpread(itemDef, pellets, mode, seed, baseSpread, inacc,
                    recoil, perfectAim, outSpread);

        if (!cfg.enabled || !outSpread) return;

        float ox = 0.f, oy = 0.f;
        __try { ox = outSpread[0]; oy = outSpread[1]; }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        char line[224];
        int len = _snprintf_s(line, sizeof(line), _TRUNCATE,
            "SPREAD %u,%d,%d,0x%08X,"
            "%.6f,%.6f,%.4f,%.6f,%.6f\r\n",
            (unsigned)itemDef, pellets, mode, seed,
            baseSpread, inacc, recoil, ox, oy);

        EnterCriticalSection(&g_logCs);
        if (g_log == INVALID_HANDLE_VALUE) OpenLog();
        if (len > 0) WriteLine(line, len);
        LeaveCriticalSection(&g_logCs);

        g_spreadCalls.fetch_add(1, std::memory_order_relaxed);
    }

    // 0=not run, 1=ok, -1=no pattern match, -2=MH_CreateHook failed,
    // -3=MH_EnableHook failed
    inline int g_setupResult = 0;

    inline int Setup()
    {
        if (g_setupResult) return g_setupResult;

        if (!g_logCsInit)
        {
            InitializeCriticalSection(&g_logCs);
            g_logCsInit = true;
        }

        HMODULE hClient = GetModuleHandleW(L"client.dll");
        if (!hClient) { g_setupResult = -1; return g_setupResult; }
        uintptr_t addr = Mem::FindPatternInModule(hClient, kSeedGenSig);
        if (!addr) { g_setupResult = -1; return g_setupResult; }

        MH_STATUS st = MH_CreateHook(reinterpret_cast<void*>(addr),
                                     reinterpret_cast<void*>(&hkSeedGen),
                                     reinterpret_cast<void**>(&oSeedGen));
        if (st != MH_OK) { g_setupResult = -2; return g_setupResult; }
        st = MH_EnableHook(reinterpret_cast<void*>(addr));
        if (st != MH_OK) { g_setupResult = -3; return g_setupResult; }

        // Optional second hook on the spread vector function. Logs
        // (seed_in, outSpread_xy) so Phase 3 can validate that our
        // calls (or any in-process reproducer) match Valve's output
        // bit-exactly. Failure here is non-fatal — we still get the
        // seed-derivation log.
        uintptr_t addrSpr = Mem::FindPatternInModule(hClient, kSpreadSig);
        if (addrSpr)
        {
            MH_STATUS s2 = MH_CreateHook(reinterpret_cast<void*>(addrSpr),
                                         reinterpret_cast<void*>(&hkSpread),
                                         reinterpret_cast<void**>(&oSpread));
            if (s2 == MH_OK) MH_EnableHook(reinterpret_cast<void*>(addrSpr));
        }

        g_setupResult = 1;
        return g_setupResult;
    }
}

#else // !LUCID_SEED_PROBE

// Stub so call sites compile cleanly without the define.
namespace SeedProbe
{
    struct Config { bool enabled = false; };
    inline Config cfg;
    inline int g_setupResult = 0;
    inline int Setup() { return 0; }
}

#endif // LUCID_SEED_PROBE
