#pragma once

// =====================================================================
//  Seeded Triggerbot â€” calls Valve's spread RNG functions directly to
//  predict whether the next bullet WILL hit, and only fires on a tick
//  where the predicted impact lands inside the target hitbox.
//
//  Pipeline (each tick, when key held + crosshair on enemy):
//    1. read view angles, weapon, target bone position
//    2. seed = sub_180C7BDD0(weapon, &angles, m_nTickBase)
//    3. spread X,Y = sub_180C7C6F0(itemDef, 1, 0, seed+1, last_args...)
//    4. dir = forward + right_basis*X + up_basis*Y
//    5. ray-test dir vs (bone_pos - eye, hitbox_radius). hit â‡’ click.
//
//  Why bit-exact: we call the engine's own functions. Same SHA-1, same
//  Park-Miller LCG, same Bays-Durham shuffle, same `+1`. Predictions
//  match Valve's own spread output to single-precision float.
//
//  The spread function args (baseSpread / inaccuracy / recoil) are
//  captured passively from real engine calls (HUD-preview also
//  triggers them, so the cache fills before the player ever fires)
//  and keyed by itemDef.
//
//  Anti-detection notes:
//   * No memory writes to game state.
//   * No mouse-input pattern change vs the legacy triggerbot.
//   * Tier0's RandomSeed is called by the engine spread fn itself â€”
//     nothing to restore on our side. Worst-case our prediction
//     perturbs decal RNG by 4 floats. Untraceable.
//   * Skips fire if cached args are stale (engine never called the
//     spread fn for this weapon yet).
//
//  Compatibility shim: keeps namespace `Triggerbot` so menu.h, hooks.h
//  and the rest of the project continue to work unchanged.
// =====================================================================

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <atomic>
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
#include "../../core/memory.h"
#include "../../core/math.h"
#include "../../core/spoof_call.h"
#include "../../core/engine_trace.h"
#include "../../vendor/minhook/include/MinHook.h"

namespace Triggerbot
{
    // -------------------------------------------------------------
    // Public configuration (menu-bound â€” names preserved for compat)
    // -------------------------------------------------------------
    struct Config
    {
        bool  enabled       = false;
        int   key           = 0;        // 0 = always-on, else hold-to-fire VK
        bool  teamCheck     = true;
        bool  smokeCheck    = true;
        int   minDelayMs    = 0;        // pre-fire delay floor (ms)
        int   maxDelayMs    = 5;        // pre-fire delay ceiling â€” effectively zero
        int   burstMin      = 1;
        int   burstMax      = 1;
        bool  scopeOnly     = false;

        // ---- Seeded prediction (the new primary gate) ----
        bool  seededPredict     = true;     // master switch
        bool  predictBothTicks  = true;     // try tick & tick+1 (cmd timing slop)
        float hitboxRadius      = 5.5f;     // units. head sphere ~5u; +0.5 slop for sway
        int   targetBone        = 7;        // CS2: 7=Head, 6=Neck, 23=Chest, 1=Pelvis
        bool  airborneFire      = true;     // allow firing in air (the whole point)
        float pickFov           = 8.0f;     // degrees â€” wide picker; radius check is the real gate
        // ---- Target leading (NEW) ----
        // Strafing enemies move ~250 ups; over 50ms (typical low-ping)
        // that's 12.5 units â€” enough to miss a head sphere. Lead the
        // target by velocity * leadTime so the predicted impact point
        // accounts for where the enemy WILL be when our click lands.
        bool  targetLead        = true;     // enable velocity-lead
        float leadTime          = 0.050f;   // seconds (~3 ticks @ 64hz)
        // Wider tick-prediction window for strafing targets. We try
        // {tick-1, tick, tick+1, tick+2} â€” each candidate gets its own
        // lead-projected target (lead = leadTime + tickOff/64).
        bool  wideTickWindow    = true;
        // ---- Target mode (auto-selects bone + radius) ----
        // 0 = HEAD only      \u2192 bone 7,  radius ~5 (1-tap headshot)
        // 1 = BODY only      \u2192 bone 23, radius ~12 (chest/spine)
        // 2 = AUTO (any hit) \u2192 try head first, fall back to chest
        // 3 = MANUAL         \u2192 use targetBone + hitboxRadius as set
        int   targetMode        = 0;

        // ---- Legacy heuristic fallback (used when prediction
        //      can't run â€” sigs not resolved or args cache empty) ----
        bool  accuracyGate    = true;
        float maxPenalty      = 0.45f;
        bool  speedGate       = false;      // off by default â€” predictor handles it
        float maxSpeed        = 110.f;

        // ---- EngineTrace vischeck (NEW) ----
        // Run a real engine bullet trace eye→target before firing.
        // Eliminates the seed-trigger's biggest failure mode: clicking
        // when crosshair is near a head bone but the line of sight is
        // actually blocked by a wall/teammate. Cheap (one trace per
        // fire-candidate tick).
        bool  visCheck        = true;

        // ---- Force-perfect-shot-on-trigger (NEW) ----
        // When the trigger sends a click, arm a short window during
        // which our passive spread hook zeroes the engine's spread
        // output. The bullet then leaves the barrel exactly along the
        // crosshair regardless of jump/move/turning inaccuracy.
        //
        // ⚠ DESYNC RISK: CS2 is server-authoritative for damage. The
        //   server runs its OWN copy of the spread fn with the same
        //   seed and uses REAL inaccuracy. Zeroing client-side spread
        //   makes the client predict a perfect hit (you hear the dink/
        //   kill sound) while the server bullet still flies wide. Net
        //   result: kill-sound plays, no damage. KEEP THIS OFF for
        //   real damage; it's only useful for visual confirmation
        //   (impact decals on the crosshair) when desync is acceptable.
        bool  perfectShot     = false;
        int   perfectWindowMs = 200;

        // ---- UC-reference consecutive-tick gate ----
        // The reference impl ("premium_seed_sync_trigger") only fires
        // when N consecutive predicted ticks ALL land hit. This kills
        // the 1-tick false-positive (predictor hits a single tick
        // because of view jitter, but the bullet's actual tick lands
        // a frame later when the head bone has moved off-axis).
        // UC-default 1 = fire on the first tick the predicted bullet
        // lands inside the hitbox sphere. Setting >1 is safer against
        // single-tick view-jitter false positives but introduces a
        // race that prevents jump/strafe shots from ever firing
        // (eye-angle drifts each frame, streak resets to 0 every miss).
        int   requireConsecutiveTicks = 1;

        // ---- Strict-window jumpshot mode ----
        // The wide-tick window {-1,0,+1,+2} fires when ANY of those
        // projected positions intersects the hitbox sphere. With
        // strictWindow ON, both ticks the engine could possibly fire
        // on ({0, +1}) must predict hit before we click. Net effect:
        // the bullet lands regardless of which side of the tick
        // boundary the engine processes our +attack on. Applies to
        // BOTH perfect-shot and seeded-only paths.
        bool  strictWindow    = true;

        // ---- Local-eye velocity lead (NEW) ----
        // During a jump the local player's eye position is moving
        // ~250-300 ups (horizontal) + jump arc Z. Engine fires the
        // bullet from eye_pos at fire-tick, NOT from eye_pos when we
        // tested. For an enemy 30 units wide at 800u away, a 6u eye
        // shift between predict and fire offsets the impact by ~6u
        // — enough to clip past a head sphere on a strafing target.
        // Project local eye forward by leadTime so we test from
        // where the engine will actually fire from.
        bool  localEyeLead    = true;

        // ---- Visual debug overlay ----
        // Renders an in-game panel showing live predictor state:
        // current seed, inaccuracy, last predicted spread offset,
        // hit/miss for the consecutive-tick window, counters. Helps
        // diagnose why the trigger isn't firing in any given moment.
        bool  debugVisual     = false;
    };

    inline Config cfg;

    // -------------------------------------------------------------
    // Engine function pointers (resolved at Setup)
    // -------------------------------------------------------------
    using SeedGenFn = std::int64_t(__fastcall*)(std::int64_t weapon,
                                                float*       angles,
                                                std::int32_t extraKey);

    // 9-arg signature confirmed via IDA decompile of sub_180C7BE80:
    //   v81 = seed + 1;
    //   sub_180C7C6F0(v78, v80, v79, v77, v81, &v108, &v111, v114, v117);
    // Last two pointers (v114, v117) are the float[pelletCount]
    // output buffers. v114[0] = X spread, v117[0] = Y spread.
    using SpreadFn = void(__fastcall*)(std::uint16_t itemDef,
                                       int           pellets,
                                       int           mode,
                                       std::uint32_t seedRaw,
                                       std::uint32_t seedPlusOne,
                                       float         baseSpread,
                                       float         inaccuracy,
                                       float*        outPerfectAim,
                                       float*        outSpread);

    inline SeedGenFn pSeedGen = nullptr;
    inline SpreadFn  pSpread  = nullptr;
    inline SpreadFn  oSpread  = nullptr;   // hooked original
    inline std::atomic<int>           g_setupResult{-99};
    inline std::atomic<std::uint64_t> g_spreadCalls{0};
    inline std::atomic<std::uint64_t> g_predictTries{0};
    inline std::atomic<std::uint64_t> g_predictHits{0};
    inline std::atomic<std::uint64_t> g_clicksSent{0};

    // Tiny diagnostic log so we can see WHY no shot fired without
    // a debug build. Append-only at %TEMP%\\lucid_trigger.log.
    inline void DLog(const char* fmt, ...)
    {
        char path[MAX_PATH]; GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "lucid_trigger.log");
        HANDLE h = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ,
                               nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return;
        char buf[512]; va_list a; va_start(a, fmt);
        int n = _vsnprintf_s(buf, sizeof(buf)-2, _TRUNCATE, fmt, a);
        va_end(a);
        if (n > 0) { buf[n]='\n'; buf[n+1]=0; DWORD w; WriteFile(h, buf, n+1, &w, nullptr); }
        CloseHandle(h);
    }

    // Sigs (shared with seed_probe.h â€” duplicated here so the probe
    // can be removed without breaking the trigger).
    constexpr const char* kSeedGenSig =
        "48 89 5C 24 08 57 48 81 EC F0 00 00 00 F3 0F 10 0A "
        "48 8D 8C 24 10 01 00 00 41 8B D8 48 8B FA E8";
    constexpr const char* kSpreadSig =
        "48 8B C4 48 89 58 08 48 89 68 18 48 89 70 20 "
        "57 41 54 41 55 41 56 41 57 48 81 EC E0 00 00 00";

    // -------------------------------------------------------------
    // Per-itemDef args cache â€” populated by passive hook on spread fn.
    // Each entry holds the most recent (mode, baseSpread, inaccuracy)
    // tuple Valve's own caller computed for this weapon. Pellets count
    // we override (always 1 for prediction â€” we only test where the
    // first pellet would land).
    // -------------------------------------------------------------
    struct CachedArgs
    {
        std::atomic<bool> valid{false};
        int               mode{0};
        float             baseSpread{0.f};
        float             inaccuracy{0.f};
    };
    inline CachedArgs g_argCache[1024];

    // Force-perfect-shot deadline (ms timestamp). When > GetTickCount()
    // and cfg.perfectShot is on, the next call(s) to hkSpread will
    // overwrite the perfectAim/spread outputs with 0,0 â€” bullet goes
    // straight regardless of jump/move/turn inaccuracy. Set right
    // before SendClick(), expires automatically.
    inline std::atomic<DWORD> g_perfectUntilMs{0};

    // Public: tick range in which the most recent synthetic LBUTTON
    // click is still observable to GetAsyncKeyState. Aimbot reads this
    // to mask synth events out of its own aim-key gate so triggerbot
    // doesn't toggle aimbot state mid-engagement.
    inline std::atomic<DWORD> g_synthClickUntilMs{0};

    // -------------------------------------------------------------
    // Debug snapshot — last predictor state, sampled atomically by
    // Tick() and read by RenderDebug() on the present thread. Plain
    // POD held in atomics + a seqlock-style version counter so the
    // render thread never tears half-updated values.
    // -------------------------------------------------------------
    struct DebugSnap {
        DWORD     ms;            // GetTickCount when this snap was taken
        int       lastTick;      // m_nTickBase
        unsigned  itemDef;
        float     pitch, yaw;
        float     punchPitch, punchYaw;
        float     baseSpread;
        float     inaccuracy;
        float     spreadX, spreadY;     // last predicted offset (perfectShot=false path)
        float     missDist;             // |target - closest| (perfectShot fast-path)
        float     radius;
        int       hitsInWindow;         // consecutive hits accumulated
        int       neededHits;           // cfg.requireConsecutiveTicks
        int       targetIdx;            // entity index of locked target, 0=none
        int       lastStage;            // last stage(N) we exited at
        unsigned  hit : 1;
        unsigned  perfect : 1;
        unsigned  predicted : 1;        // ran prediction this tick
        unsigned  fired : 1;
        unsigned  cacheHot : 1;
    };
    inline std::atomic<std::uint32_t> g_dbgSeq{0};
    inline DebugSnap                  g_dbgSnap{};

    inline void DbgPublish(const DebugSnap& s)
    {
        // Seqlock writer: bump to odd, write, bump to even. Readers
        // retry while seq is odd or changes mid-read.
        std::uint32_t seq = g_dbgSeq.load(std::memory_order_relaxed);
        g_dbgSeq.store(seq + 1, std::memory_order_release);
        g_dbgSnap = s;
        g_dbgSeq.store(seq + 2, std::memory_order_release);
    }

    // Consecutive-hit accumulator (UC reference's "all_ticks_hit" gate).
    // Tracks predicted-tick count and which target we accumulated against
    // so switching targets resets the counter immediately.
    inline int             g_hitStreak  = 0;
    inline std::uintptr_t  g_streakPawn = 0;

    // -------------------------------------------------------------
    // Passive spread hook â€” captures last-known args per weapon.
    // Always passes through to the real function unchanged.
    // -------------------------------------------------------------
    inline void __fastcall hkSpread(std::uint16_t itemDef,
                                    int           pellets,
                                    int           mode,
                                    std::uint32_t seedRaw,
                                    std::uint32_t seedPlusOne,
                                    float         baseSpread,
                                    float         inaccuracy,
                                    float*        outPerfectAim,
                                    float*        outSpread)
    {
        if (oSpread)
            oSpread(itemDef, pellets, mode, seedRaw, seedPlusOne,
                    baseSpread, inaccuracy, outPerfectAim, outSpread);

        // ---- force-perfect window: if our trigger just clicked,
        //      flatten the engine's spread output so the bullet
        //      leaves the barrel exactly down the crosshair.
        DWORD deadline = g_perfectUntilMs.load(std::memory_order_acquire);
        if (deadline && GetTickCount() < deadline)
        {
            __try {
                int n = (pellets <= 0) ? 1 : pellets;
                if (n > 16) n = 16;     // safety cap
                if (outPerfectAim) for (int i = 0; i < n; ++i) outPerfectAim[i] = 0.f;
                if (outSpread)     for (int i = 0; i < n; ++i) outSpread[i]     = 0.f;
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        if (itemDef < 1024)
        {
            auto& c = g_argCache[itemDef];
            c.mode       = mode;
            c.baseSpread = baseSpread;
            c.inaccuracy = inaccuracy;
            bool first = !c.valid.exchange(true, std::memory_order_acq_rel);
            if (first) DLog("[hkSpread] cached itemDef=%u mode=%d base=%.5f inacc=%.5f",
                            (unsigned)itemDef, mode, baseSpread, inaccuracy);
        }
        g_spreadCalls.fetch_add(1, std::memory_order_relaxed);
    }

    // -------------------------------------------------------------
    // Setup â€” sig-scan client.dll for both engine functions and
    // install the passive spread hook. Returns:
    //   1  full success
    //   0  hooks installed but spread sig missing (predictor disabled)
    //  -1  client.dll not loaded yet
    //  -2  seed-gen sig missing
    // -------------------------------------------------------------
    inline int Setup()
    {
        HMODULE hClient = GetModuleHandleW(L"client.dll");
        if (!hClient) return -1;

        std::uintptr_t addrSeed = Mem::FindPatternInModule(hClient, kSeedGenSig);
        if (!addrSeed) return -2;
        pSeedGen = reinterpret_cast<SeedGenFn>(addrSeed);

        std::uintptr_t addrSpr = Mem::FindPatternInModule(hClient, kSpreadSig);
        if (!addrSpr) return 0;
        pSpread = reinterpret_cast<SpreadFn>(addrSpr);

        if (MH_CreateHook(reinterpret_cast<void*>(addrSpr),
                          reinterpret_cast<void*>(&hkSpread),
                          reinterpret_cast<void**>(&oSpread)) != MH_OK)
            return 0;
        if (MH_EnableHook(reinterpret_cast<void*>(addrSpr)) != MH_OK)
            return 0;

        DLog("[Setup] OK seedGen=%p spread=%p", (void*)addrSeed, (void*)addrSpr);
        g_setupResult.store(1, std::memory_order_release);
        return 1;
    }

    // -------------------------------------------------------------
    // Internal humanization state (carried over from legacy trigger)
    // -------------------------------------------------------------
    inline DWORD    triggerTime  = 0;
    inline int      delayMs      = 0;
    inline int      burstLeft    = 0;
    inline DWORD    lastShotTime = 0;
    inline int      cooldownMs   = 0;

    inline std::uint32_t tRng = 0;
    inline std::uint32_t TRand()
    {
        if (!tRng)
        {
            LARGE_INTEGER pc; QueryPerformanceCounter(&pc);
            tRng = static_cast<std::uint32_t>(pc.QuadPart ^ (pc.QuadPart >> 17));
            if (!tRng) tRng = 1;
        }
        tRng ^= tRng << 13;
        tRng ^= tRng >> 17;
        tRng ^= tRng << 5;
        return tRng;
    }
    inline int TRandInt(int lo, int hi)
    {
        if (lo >= hi) return lo;
        return lo + (int)(TRand() % (hi - lo + 1));
    }

    inline void SendClick()
    {
        // Mark the synthetic-click window BEFORE issuing the input so
        // any aimbot tick that races our SendInput sees the mask.
        // 80ms covers our SendInput round-trip + the down/up message
        // pair landing in WM_INPUT/RawInput queues. Aimbot ignores
        // LBUTTON state during this window.
        g_synthClickUntilMs.store(GetTickCount() + 80, std::memory_order_release);

        INPUT inp[2] = {};
        inp[0].type = INPUT_MOUSE;
        inp[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        inp[1].type = INPUT_MOUSE;
        inp[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SpoofCall::SpoofedSendInput(2, inp, sizeof(INPUT));
    }

    // -------------------------------------------------------------
    // Build forward / right / up basis from view angles.
    // Source-engine convention (matches Valve's AngleVectors):
    //   forward = ( cp*cy,  cp*sy, -sp )
    //   right   = ( -sy,    cy,     0  )      (LH coord, before roll)
    //   up      = forward Ã— right  (so dir = fwd + right*X + up*Y)
    //
    // Final direction transform observed in sub_180C7BE80:
    //   dir.x = up.x*Y + right.x*X + fwd.x
    //   dir.y = up.y*Y + right.y*X + fwd.y
    //   dir.z = up.z*Y - right.x*X + fwd.z   (note the asymmetric z)
    // The asymmetric z is consistent with up = (cp*cy, cp*sy, -sp) and
    // right = (-sy, cy, 0) â€” verified algebraically against the IDA
    // pseudo (v118..v121, v122..v124).
    // -------------------------------------------------------------
    inline void Basis(float pitch, float yaw,
                      Math::Vec3& fwd, Math::Vec3& right, Math::Vec3& up)
    {
        const float cp = cosf(pitch * Math::kDeg2Rad);
        const float sp = sinf(pitch * Math::kDeg2Rad);
        const float cy = cosf(yaw   * Math::kDeg2Rad);
        const float sy = sinf(yaw   * Math::kDeg2Rad);

        fwd   = { cp * cy,  cp * sy, -sp };
        right = { -sy,      cy,       0.f };
        up    = { sp * cy,  sp * sy,  cp };
    }

    // -------------------------------------------------------------
    // PredictHit â€” the heart of the seeded trigger. Runs the engine's
    // own seed generator + spread RNG with the supplied tick value,
    // builds the resulting fire direction, and returns true iff the
    // direction-ray passes within `radius` of `targetWorld` measured
    // from `eye`.
    //
    //   weapon:       weapon entity ptr (passed to seed generator)
    //   itemDef:      cached args lookup key
    //   pitch/yaw:    current view angles (also fed to seed-gen)
    //   tick:         m_nTickBase value to test
    //   eye:          shooter eye world pos
    //   targetWorld:  candidate impact center (e.g. enemy head bone)
    //   radius:       hitbox sphere radius
    // -------------------------------------------------------------
    inline bool PredictHit(std::int64_t weapon,
                           std::uint16_t itemDef,
                           float pitch, float yaw,
                           std::int32_t tick,
                           const Math::Vec3& eye,
                           const Math::Vec3& targetWorld,
                           float radius)
    {
        if (!pSeedGen || !pSpread) return false;
        if (itemDef >= 1024) return false;
        const auto& args = g_argCache[itemDef];
        if (!args.valid.load(std::memory_order_acquire)) return false;

        // Re-quantized angles (the engine's seed-gen does this itself,
        // we just hand it the raw values â€” wrapper sub_180C75F00 is
        // called inside sub_180C7BDD0).
        float angles[3] = { pitch, yaw, 0.f };

        std::uint32_t seed = static_cast<std::uint32_t>(
            pSeedGen(weapon, angles, tick));

        // ---- LIVE inaccuracy override ----
        // The cached `args.inaccuracy` is whatever the engine last
        // passed when it called the spread fn (HUD preview, last shot,
        // etc.). When the player is jumping or moving, real engine
        // inaccuracy spikes 5-10x. If we predict against the cached
        // (stationary) value the cone we test is far too tight and the
        // trigger fires shots that the engine then sprays wide.
        //
        // Read m_fAccuracyPenalty + m_flTurningInaccuracy live from the
        // weapon and feed the engine spread fn the worst-case of the
        // two so the predicted cone matches what's actually about to
        // come out of the barrel â€” fixes jumpshot / move-shot accuracy.
        float liveInaccuracy = args.inaccuracy;
        __try {
            float pen = Mem::Read<float>(
                static_cast<std::uintptr_t>(weapon) + Offsets::m_fAccuracyPenalty);
            float turn = Mem::Read<float>(
                static_cast<std::uintptr_t>(weapon) + Offsets::m_flTurningInaccuracy);
            float live = pen + turn;
            if (live > liveInaccuracy) liveInaccuracy = live;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}

        // NOTE: do NOT zero inaccuracy when perfectShot is on. The
        // server runs the same spread fn with REAL inaccuracy, so a
        // 0-spread client prediction with the bullet actually flying
        // wide on the server is exactly the "kill sound but no damage"
        // bug. Always test against real live inaccuracy.

        float perfectX = 0.f, spreadY = 0.f;
        __try {
            pSpread(itemDef, /*pellets*/1, args.mode,
                    seed, seed + 1u,
                    args.baseSpread, liveInaccuracy,
                    &perfectX, &spreadY);
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }

        Math::Vec3 fwd, right, up;
        Basis(pitch, yaw, fwd, right, up);

        // dir = forward + right*X + up*Y  (basis chosen to match the
        // observed sub_180C7BE80 pseudo: see Basis() comment).
        Math::Vec3 dir = {
            fwd.x + right.x * perfectX + up.x * spreadY,
            fwd.y + right.y * perfectX + up.y * spreadY,
            fwd.z + right.z * perfectX + up.z * spreadY,
        };

        // Distance from infinite ray (origin=eye, dir already â‰ˆ unit
        // since spread offsets are tiny) to point targetWorld.
        Math::Vec3 toTarget = targetWorld - eye;
        float t = toTarget.Dot(dir);     // projection along ray
        if (t <= 0.f) return false;      // behind us
        Math::Vec3 closest = { eye.x + dir.x * t,
                               eye.y + dir.y * t,
                               eye.z + dir.z * t };
        Math::Vec3 miss = targetWorld - closest;
        float dist2 = miss.x*miss.x + miss.y*miss.y + miss.z*miss.z;
        return dist2 <= (radius * radius);
    }

    // -------------------------------------------------------------
    // ReadAimPunch — the bullet's actual fire direction in CS2 is
    // computed from view + aim_punch (recoil-applied angles), not the
    // raw user view. Failing to add the punch is why mid-spray seeded
    // shots silently miss: the predictor tests the unpunched ray, but
    // the engine fires through punched ray, so spread alignment is off
    // by the punch delta. Reference impls ("premium_get_view + aim_punch")
    // confirm this is the missing transform.
    //
    // m_aimPunchAngle moved to CCSPlayer_AimPunchServices in 14153:
    //   pawn + 0x1490 → services
    //   services + 0x50 = QAngle predictableBaseAngle
    // -------------------------------------------------------------
    inline Math::QAngle ReadAimPunch(std::uintptr_t pawn)
    {
        Math::QAngle q{0,0,0};
        __try {
            std::uintptr_t svc = Mem::Read<std::uintptr_t>(
                pawn + Offsets::m_pAimPunchServices);
            if (svc) {
                q = Mem::Read<Math::QAngle>(
                    svc + Offsets::m_predictableBaseAngle);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) { q = {0,0,0}; }
        return q;
    }

    // -------------------------------------------------------------
    // Public per-frame entry point. Called from EndScene hook.
    //
    // 2026-05-04 — FULL REWRITE based on the UC reference
    // ("supercoolundetectedp100seedsynctriggerbot"). Single linear
    // flow, no committed/fast-path/legacy fork, no streak gate by
    // default. Designed so jump + strafe shots fire reliably:
    //   1. key + ready gates
    //   2. pick best enemy by FOV inside cfg.pickFov
    //   3. read live eye+view+aim_punch+velocity, advance bone by
    //      velocity*leadTime to land where the enemy WILL be
    //   4. perfect-shot path  : forward ray vs hitbox sphere only
    //      seeded-predict path: pSeedGen + pSpread, ray vs sphere
    //   5. vis-check head OR chest via EngineTrace
    //   6. SendClick, arm perfect window, weapon-class cooldown
    // -------------------------------------------------------------
    inline void Tick()
    {
        // ----- cfg.enabled edge detector -----
        // When the user toggles trigger off mid-cooldown, the previous
        // version left cooldownMs / lastShotTime / burstLeft / triggerTime
        // / g_synthClickUntilMs / g_perfectUntilMs untouched. Re-enabling
        // before the cooldown elapsed (or while a perfect-shot window was
        // still arming) silently blocked the next click — user reports
        // "trigger stops firing after a few toggles, only fixes itself on
        // next half". Wipe the runtime state on every disable transition
        // so re-enable always starts cold.
        {
            static bool s_prevEnabled = false;
            if (s_prevEnabled && !cfg.enabled) {
                triggerTime  = 0;
                burstLeft    = 0;
                cooldownMs   = 0;
                lastShotTime = 0;
                g_hitStreak  = 0;
                g_synthClickUntilMs.store(0, std::memory_order_relaxed);
                g_perfectUntilMs.store(0, std::memory_order_relaxed);
            }
            s_prevEnabled = cfg.enabled;
        }

        if (!cfg.enabled || !GameState::clientBase) return;

        __try {

        // -------- diagnostic: per-stage exit counters --------
        static std::atomic<std::uint32_t> sStage[16]{};
        static std::atomic<DWORD> sLastDump{0};
        auto stage = [&](int n) { sStage[n].fetch_add(1, std::memory_order_relaxed); };
        DWORD nowMs = GetTickCount();
        if (nowMs - sLastDump.load(std::memory_order_relaxed) > 1000)
        {
            sLastDump.store(nowMs, std::memory_order_relaxed);
            DLog("[stage] key=%u noPawn=%u notReady=%u noScope=%u noTarg=%u "
                 "smoke=%u ground=%u predMiss=%u visBlk=%u fired=%u",
                 sStage[0].load(), sStage[1].load(), sStage[2].load(),
                 sStage[3].load(), sStage[4].load(), sStage[5].load(),
                 sStage[6].load(), sStage[7].load(), sStage[8].load(),
                 sStage[9].load());
            for (auto& s : sStage) s.store(0, std::memory_order_relaxed);
        }

        // ---- 1. KEY GATE ----
        if (cfg.key != 0)
        {
            SHORT s = GetAsyncKeyState(cfg.key);
            if (!(s & 0x8000)) { triggerTime = 0; g_hitStreak = 0; stage(0); return; }
        }

        std::uintptr_t localPawn = GameState::GetLocalPawn();
        std::uintptr_t localCtrl = GameState::GetLocalController();
        if (!localPawn || !localCtrl) { stage(1); return; }

        // Post-shot inter-click cooldown (humanises rapid bursts).
        if (lastShotTime && cooldownMs > 0)
        {
            if (GetTickCount() - lastShotTime < (DWORD)cooldownMs) { return; }
            cooldownMs = 0;
        }

        // ---- 2. WEAPON-READY (m_nNextPrimaryAttackTick) ----
        // Hard gate: engine ignores attack input until tickBase >= nextAtk.
        // Spamming SendInput inside this window decouples our predictor
        // from the bullet that eventually leaves \u2014 root cause of the
        // "deagle keeps shooting after a miss" report.
        std::uintptr_t weapon = GameState::GetActiveWeapon(localPawn);
        if (!weapon) { stage(2); return; }
        {
            __try {
                std::int32_t nextAtk = Mem::Read<std::int32_t>(
                    weapon + Offsets::m_nNextPrimaryAttackTick);
                std::int32_t tBase   = Mem::Read<std::int32_t>(
                    localCtrl + Offsets::m_nTickBase);
                if (nextAtk > 0 && tBase > 0 && tBase < nextAtk) {
                    triggerTime = 0; burstLeft = 0; g_hitStreak = 0;
                    stage(2); return;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        if (cfg.scopeOnly)
        {
            bool scoped = Mem::Read<bool>(localPawn + Offsets::m_bIsScoped);
            if (!scoped) { triggerTime = 0; g_hitStreak = 0; stage(3); return; }
        }

        // ---- 3. PICK BEST ENEMY (smallest FOV inside cfg.pickFov) ----
        // Tight cone (default ~8\u00b0). Same picker grounded or airborne \u2014
        // widening the cone in air was making us fire at heads several
        // degrees off the crosshair (perfectShot zeroed spread, bullet
        // went straight forward, missed entirely).
        Math::QAngle eyeAng = Mem::Read<Math::QAngle>(localPawn + Offsets::m_angEyeAngles);
        Math::Vec3   eyePos = GameState::GetEntityOrigin(localPawn);
        Math::Vec3   vOff   = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecViewOffset);
        eyePos.x += vOff.x; eyePos.y += vOff.y; eyePos.z += vOff.z;

        int   myTeam  = Mem::Read<std::uint8_t>(localPawn + Offsets::m_iTeamNum);
        std::uintptr_t pawn = 0;
        float bestFov = cfg.pickFov;

        for (int i = 1; i <= 64; ++i)
        {
            __try {
                std::uintptr_t cctrl = GameState::GetEntityByIndex(i);
                if (!cctrl) continue;
                bool alive = Mem::Read<bool>(cctrl + Offsets::m_bPawnIsAlive);
                if (!alive) continue;
                std::uint32_t ph = Mem::Read<std::uint32_t>(cctrl + Offsets::m_hPlayerPawn);
                if (!ph || ph == 0xFFFFFFFFu) continue;
                std::uintptr_t cpawn = GameState::ResolveHandle(ph);
                if (!cpawn || cpawn == localPawn) continue;
                int chp = Mem::Read<std::int32_t>(cpawn + Offsets::m_iHealth);
                if (chp <= 0) continue;
                if (cfg.teamCheck) {
                    int tt = Mem::Read<std::uint8_t>(cpawn + Offsets::m_iTeamNum);
                    if (tt == myTeam) continue;
                }
                // Pick on whichever bone the user picked; predictor will
                // try both head + chest later when targetMode=AUTO.
                int pickBone = (cfg.targetMode == 1) ? 23 : 7;
                Math::Vec3 b = GameState::GetBonePos(cpawn, pickBone);
                if (b.IsZero()) continue;
                Math::QAngle a = Math::CalcAngle(eyePos, b);
                float fv = Math::AngleFov(eyeAng, a);
                if (fv < bestFov) { bestFov = fv; pawn = cpawn; }
            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }

        if (!pawn) { triggerTime = 0; burstLeft = 0; g_hitStreak = 0; stage(4); return; }

        if (cfg.smokeCheck)
        {
            float a = Mem::Read<float>(localPawn + Offsets::m_flLastSmokeOverlayAlpha);
            if (a > 0.5f) { triggerTime = 0; g_hitStreak = 0; stage(5); return; }
        }
        if (!cfg.airborneFire)
        {
            std::uint32_t flags = Mem::Read<std::uint32_t>(localPawn + Offsets::m_fFlags);
            if (!(flags & 1u)) { triggerTime = 0; g_hitStreak = 0; stage(6); return; }
        }

        // ---- 4. PREDICT IMPACT ----
        // Build live aiming basis (eye + view + aim_punch). Bullet leaves
        // through PUNCHED angles; using raw view here is the #1 reason
        // mid-spray seeded shots silently miss.
        Math::QAngle ang = Mem::Read<Math::QAngle>(localPawn + Offsets::m_angEyeAngles);
        {
            Math::QAngle pn = ReadAimPunch(localPawn);
            ang.pitch += pn.pitch;
            ang.yaw   += pn.yaw;
        }
        Math::Vec3 eye = GameState::GetEntityOrigin(localPawn);
        {
            Math::Vec3 vo = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecViewOffset);
            eye.x += vo.x; eye.y += vo.y; eye.z += vo.z;
        }

        // ---- Local-eye velocity lead ----
        // Engine fires the bullet from the eye_pos AT FIRE-TICK, not
        // at predict-tick. For airborne shots the local pawn is moving
        // 250-300 ups, so the eye shifts 4-6 units between predict and
        // fire — enough to miss a head sphere. Project local eye by
        // localVel * leadTime so we test from where the engine will
        // actually fire from.
        Math::Vec3 localVel{0,0,0};
        if (cfg.localEyeLead) {
            __try { localVel = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecVelocity); }
            __except (EXCEPTION_EXECUTE_HANDLER) { localVel = {0,0,0}; }
            float lead = cfg.leadTime;
            eye.x += localVel.x * lead;
            eye.y += localVel.y * lead;
            eye.z += localVel.z * lead;
        }

        // Forward unit vector (used for the perfect-shot ray test).
        const float cp = cosf(ang.pitch * Math::kDeg2Rad);
        const float sp = sinf(ang.pitch * Math::kDeg2Rad);
        const float cy = cosf(ang.yaw   * Math::kDeg2Rad);
        const float sy = sinf(ang.yaw   * Math::kDeg2Rad);
        Math::Vec3  fwd = { cp*cy, cp*sy, -sp };

        // Bone(s) to test. Tight radii so we only fire on a true hit:
        // since the predictor uses real spread + real inaccuracy, the
        // ray we test IS the bullet path the server will simulate.
        // Generous radii here cause server-misses on edge cases.
        struct TargetSpec { int bone; float radius; };
        const float headR  = 3.6f;
        const float chestR = 6.0f;
        TargetSpec specs[2] = {{7, headR}, {23, chestR}};
        int nSpecs = 1;
        switch (cfg.targetMode) {
            case 0: specs[0] = {7,  headR};                          nSpecs = 1; break;
            case 1: specs[0] = {23, chestR};                         nSpecs = 1; break;
            case 2: specs[0] = {7,  headR}; specs[1] = {23, chestR}; nSpecs = 2; break;
            default: specs[0] = {cfg.targetBone, cfg.hitboxRadius};  nSpecs = 1; break;
        }

        // Velocity-lead: project bone forward by leadTime so we test
        // where the enemy WILL be when the bullet lands. ~50ms lead at
        // 250 ups strafe = 12.5u \u2014 enough to make/break a head-sphere.
        Math::Vec3 targetVel{0,0,0};
        if (cfg.targetLead) {
            __try { targetVel = Mem::Read<Math::Vec3>(pawn + Offsets::m_vecVelocity); }
            __except (EXCEPTION_EXECUTE_HANDLER) { targetVel = {0,0,0}; }
        }

        // Always run the seeded predictor when the sigs resolved —
        // it's the only thing that matches what the SERVER will
        // compute for the bullet trajectory. Even with perfectShot
        // armed we still want the real-spread prediction (the
        // perfect-shot output zeroing is a separate, optional layer).
        bool predictionWanted = (pSeedGen != nullptr) && (pSpread != nullptr)
                                && cfg.seededPredict;
        std::uint16_t itemDef = 0;
        std::int32_t tick = 0;
        bool cacheHot = false;
        float baseSpread = 0.f, liveInacc = 0.f;
        if (predictionWanted) {
            __try {
                itemDef = Mem::Read<std::uint16_t>(
                    weapon + Offsets::m_AttributeManager
                           + Offsets::m_Item + Offsets::m_iItemDefinitionIndex);
                tick = Mem::Read<std::int32_t>(localCtrl + Offsets::m_nTickBase);
                cacheHot = (itemDef < 1024) &&
                    g_argCache[itemDef].valid.load(std::memory_order_acquire);
                if (cacheHot) {
                    baseSpread = g_argCache[itemDef].baseSpread;
                    float pen = Mem::Read<float>(weapon + Offsets::m_fAccuracyPenalty);
                    float trn = Mem::Read<float>(weapon + Offsets::m_flTurningInaccuracy);
                    liveInacc = (pen + trn);
                    if (liveInacc < g_argCache[itemDef].inaccuracy)
                        liveInacc = g_argCache[itemDef].inaccuracy;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) { cacheHot = false; }
        }

        bool  hit       = false;
        float bestMiss  = 1e9f;
        float bestRad   = headR;
        // Tick offsets the engine could actually fire on:
        //   perfectShot: {0, +1} only — the bullet leaves on tick or
        //              tick+1 depending on which side of the tick
        //              boundary our +attack arrives. Negative or +2
        //              offsets are noise that produce false fires.
        //   seeded:    {0, +1, -1, +2} (legacy wide window)
        // The engine fires the bullet on EITHER tick or tick+1
        // depending on which side of the tick boundary our +attack
        // arrives on. Test only the offsets the server can actually
        // fire on — testing -1 or +2 produces false positives that
        // don't survive server replay (= kill-sound, no-damage).
        const int  tickOffsStrict[2] = {0, 1};
        const int  tickOffsWide[4]   = {0, 1, -1, 2};
        const bool useStable = cfg.strictWindow;
        const int* tickOffs  = useStable ? tickOffsStrict : tickOffsWide;
        const int  nOffs     = useStable                                  ? 2
                             : (predictionWanted && cfg.wideTickWindow)   ? 4
                             : (predictionWanted && cfg.predictBothTicks) ? 2 : 1;

        for (int ti = 0; ti < nSpecs && !hit; ++ti)
        {
            Math::Vec3 boneBase = GameState::GetBonePos(pawn, specs[ti].bone);
            if (boneBase.IsZero()) continue;

            // Stable mode: ALL tested offsets must hit (no false fires
            // from a single off-tick alignment). Otherwise: ANY hit fires.
            int  hitsThisSpec   = 0;
            int  testedThisSpec = 0;
            float specBestMiss  = 1e9f;
            for (int oi = 0; oi < nOffs; ++oi)
            {
                int  tOff = tickOffs[oi];
                float lead = cfg.targetLead ? (cfg.leadTime + tOff * (1.f/64.f)) : 0.f;
                if (lead < 0.f) lead = 0.f;
                Math::Vec3 target = boneBase;
                target.x += targetVel.x * lead;
                target.y += targetVel.y * lead;
                target.z += targetVel.z * lead;

                // Eye position projected to THIS offset's fire moment
                // (on top of the base leadTime already applied above).
                Math::Vec3 eyeAtFire = eye;
                if (cfg.localEyeLead && tOff != 0) {
                    float dt = tOff * (1.f / 64.f);
                    eyeAtFire.x += localVel.x * dt;
                    eyeAtFire.y += localVel.y * dt;
                    eyeAtFire.z += localVel.z * dt;
                }

                Math::Vec3 dir = fwd;
                if (predictionWanted && cacheHot) {
                    float angles[3] = { ang.pitch, ang.yaw, 0.f };
                    std::uint32_t seed = (std::uint32_t)pSeedGen(
                        (std::int64_t)weapon, angles, tick + tOff);
                    float perfectX = 0.f, spreadY = 0.f;
                    __try {
                        pSpread(itemDef, 1, g_argCache[itemDef].mode,
                                seed, seed + 1u, baseSpread, liveInacc,
                                &perfectX, &spreadY);
                    } __except (EXCEPTION_EXECUTE_HANDLER) {}
                    Math::Vec3 right, up;
                    Basis(ang.pitch, ang.yaw, dir, right, up);
                    dir.x += right.x * perfectX + up.x * spreadY;
                    dir.y += right.y * perfectX + up.y * spreadY;
                    dir.z += right.z * perfectX + up.z * spreadY;
                }

                Math::Vec3 toT = target - eyeAtFire;
                float t = toT.Dot(dir);
                if (t <= 0.f) {
                    if (useStable) { hitsThisSpec = -1; break; }
                    continue;
                }
                Math::Vec3 closest = { eyeAtFire.x + dir.x*t,
                                       eyeAtFire.y + dir.y*t,
                                       eyeAtFire.z + dir.z*t };
                Math::Vec3 miss = target - closest;
                float d2 = miss.x*miss.x + miss.y*miss.y + miss.z*miss.z;
                float md = sqrtf(d2);
                if (md < specBestMiss) specBestMiss = md;
                ++testedThisSpec;
                bool offHit = (d2 <= specs[ti].radius * specs[ti].radius);
                if (offHit) ++hitsThisSpec;
                else if (useStable) { hitsThisSpec = -1; break; }
                if (!useStable && offHit) { hit = true; break; }
            }

            if (specBestMiss < bestMiss) { bestMiss = specBestMiss; bestRad = specs[ti].radius; }
            if (useStable && hitsThisSpec > 0 && hitsThisSpec == testedThisSpec) hit = true;
        }

        g_predictTries.fetch_add(1, std::memory_order_relaxed);
        if (hit) g_predictHits.fetch_add(1, std::memory_order_relaxed);

        // ---- Optional consecutive-tick gate ----
        // Default = 1 (no gate). Set higher only if you want the UC-style
        // anti-jitter confirmation \u2014 it WILL prevent some jump shots
        // from firing because the eye-angle drifts each frame.
        int needed = cfg.requireConsecutiveTicks;
        if (needed < 1) needed = 1;
        if (needed > 5) needed = 5;
        if (g_streakPawn != pawn) { g_streakPawn = pawn; g_hitStreak = 0; }
        if (hit) g_hitStreak++; else g_hitStreak = 0;

        if (cfg.debugVisual) {
            DebugSnap d{};
            d.ms          = GetTickCount();
            d.lastTick    = tick;
            d.itemDef     = itemDef;
            d.pitch       = ang.pitch;
            d.yaw         = ang.yaw;
            Math::QAngle pn2 = ReadAimPunch(localPawn);
            d.punchPitch  = pn2.pitch;
            d.punchYaw    = pn2.yaw;
            d.baseSpread  = baseSpread;
            d.inaccuracy  = liveInacc;
            d.missDist    = bestMiss;
            d.radius      = bestRad;
            d.hitsInWindow= g_hitStreak;
            d.neededHits  = needed;
            d.targetIdx   = (int)((pawn >> 4) & 0xFFFF);
            d.lastStage   = 12;
            d.hit         = hit ? 1u : 0u;
            d.perfect     = cfg.perfectShot ? 1u : 0u;
            d.predicted   = 1;
            d.fired       = 0;
            d.cacheHot    = cacheHot ? 1u : 0u;
            DbgPublish(d);
        }

        if (!hit) { stage(7); return; }
        if (g_hitStreak < needed) { return; }

        // ---- 5. VIS CHECK (EngineTrace head OR chest) ----
        if (cfg.visCheck && EngineTrace::Ready())
        {
            bool clear = false;
            __try {
                Math::Vec3 h = GameState::GetBonePos(pawn, 7);
                if (!h.IsZero() && EngineTrace::IsVisible(pawn, h)) clear = true;
                if (!clear) {
                    Math::Vec3 c = GameState::GetBonePos(pawn, 23);
                    if (!c.IsZero() && EngineTrace::IsVisible(pawn, c)) clear = true;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
            if (!clear) { triggerTime = 0; g_hitStreak = 0; stage(8); return; }
        }

        // ---- 6. FIRE (with optional micro-delay humanisation) ----
        DWORD now = GetTickCount();
        if (triggerTime == 0)
        {
            triggerTime = now;
            // Bell-curve bias: average two rolls so the distribution
            // peaks in the middle of [min,max] instead of being flat.
            // Flat distributions produce a uniform delay spread which
            // is itself a behavioral fingerprint -- humans cluster
            // toward a personal mean.
            int a = TRandInt(cfg.minDelayMs, cfg.maxDelayMs);
            int b = TRandInt(cfg.minDelayMs, cfg.maxDelayMs);
            delayMs = (a + b) / 2;
            burstLeft  = TRandInt(cfg.burstMin, cfg.burstMax);
            if (delayMs <= 0) { /* fall through and fire this tick */ }
            else return;
        }
        if (now - triggerTime < (DWORD)delayMs) return;

        if (burstLeft > 0)
        {
            if (cfg.perfectShot)
                g_perfectUntilMs.store(GetTickCount() + (DWORD)cfg.perfectWindowMs,
                                       std::memory_order_release);
            SendClick();
            g_clicksSent.fetch_add(1, std::memory_order_relaxed);
            stage(9);
            g_hitStreak = 0;
            if (cfg.debugVisual) {
                DebugSnap d = g_dbgSnap; d.fired = 1; d.ms = GetTickCount();
                DbgPublish(d);
            }
            burstLeft--;
            lastShotTime = now;

            // Per-weapon cooldown approximating engine refire interval.
            int cooldown = 90;
            __try {
                std::uint16_t id = Mem::Read<std::uint16_t>(
                    weapon + Offsets::m_AttributeManager
                           + Offsets::m_Item + Offsets::m_iItemDefinitionIndex);
                switch (id) {
                    case 9:   cooldown = TRandInt(1450, 1600); break;   // AWP
                    case 11:  cooldown = TRandInt(1100, 1250); break;   // SCAR-20
                    case 38:  cooldown = TRandInt(1100, 1250); break;   // G3SG1
                    case 40:  cooldown = TRandInt(1050, 1200); break;   // SSG08
                    case 1:   cooldown = TRandInt( 240,  320); break;   // Deagle
                    case 64:  cooldown = TRandInt( 200,  280); break;   // Revolver
                    case 2:   cooldown = TRandInt( 130,  180); break;   // Dual Berettas
                    case 3:   cooldown = TRandInt( 110,  150); break;   // Five-Seven
                    case 4:   cooldown = TRandInt(  90,  130); break;   // Glock
                    case 32:  cooldown = TRandInt(  90,  130); break;   // P2000
                    case 36:  cooldown = TRandInt(  85,  120); break;   // P250
                    case 61:  cooldown = TRandInt(  80,  110); break;   // USP-S
                    case 63:  cooldown = TRandInt(  90,  130); break;   // CZ75
                    case 60:  cooldown = TRandInt(  90,  130); break;   // M4A1-S
                    case 16:  cooldown = TRandInt(  90,  130); break;   // M4A4
                    case 7:   cooldown = TRandInt(  90,  130); break;   // AK-47
                    default:  cooldown = TRandInt(  85,  130); break;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
            cooldownMs = cooldown;
            if (burstLeft <= 0) triggerTime = 0;
        }

        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // -------------------------------------------------------------
    // RenderDebug \u2014 in-game visual diagnostic for the seeded trigger.
    // Draws a top-right panel with live predictor state plus a
    // crosshair miss-circle showing how close the predicted bullet
    // landed to the target hitbox, and the consecutive-hit streak
    // bar (UC reference \"all_ticks_hit\" gate visualisation).
    //
    // Reads the seqlock-published g_dbgSnap, retrying on torn reads.
    // Called from PresentCore on the present thread.
    // -------------------------------------------------------------
    inline void RenderDebug()
    {
        if (!cfg.enabled || !cfg.debugVisual) return;

        // Seqlock read
        DebugSnap s{};
        for (int i = 0; i < 4; ++i) {
            std::uint32_t a = g_dbgSeq.load(std::memory_order_acquire);
            if (a & 1u) continue;
            s = g_dbgSnap;
            std::uint32_t b = g_dbgSeq.load(std::memory_order_acquire);
            if (a == b) break;
        }

        ImDrawList* dl = ImGui::GetForegroundDrawList();
        if (!dl) return;

        ImVec2 ds = ImGui::GetIO().DisplaySize;
        const float scrW = ds.x, scrH = ds.y;

        // -------------------------------------------------------
        // PER-ENEMY BONE / HITBOX OVERLAY
        // Iterates living enemy controllers, projects every relevant
        // bone to screen, draws red dots + spine line. The bone the
        // current predictor snapshot scored a HIT on (or the closest
        // miss) is drawn larger and green/yellow so debugging a
        // jumpshot is one glance: green dot inside crosshair = next
        // tick should fire.
        // -------------------------------------------------------
        std::uintptr_t localPawn = GameState::GetLocalPawn();
        std::uintptr_t localCtrl = GameState::GetLocalController();
        if (localPawn && localCtrl)
        {
            int localTeam = 0;
            __try { localTeam = Mem::Read<std::uint8_t>(localPawn + Offsets::m_iTeamNum); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}

            // Bones to plot: head, neck, spine chain, pelvis, hands, feet
            // Animgraph 2 (build 14152+): 0 pelvis | 3-5 spine | 6 neck
            //   7 head | 10/15 hands (approx) | 17/22 feet (approx)
            struct BoneDef { int idx; float r; bool primary; };
            static const BoneDef kBones[] = {
                {7,  4.5f, true },   // head
                {6,  3.5f, false},   // neck
                {5,  3.5f, true },   // upper spine
                {4,  3.0f, false},   // mid spine
                {3,  3.0f, false},   // lower spine
                {0,  4.0f, true },   // pelvis
                {10, 2.5f, false},   // l hand
                {15, 2.5f, false},   // r hand
            };
            const int kBoneCount = (int)(sizeof(kBones)/sizeof(kBones[0]));

            for (int idx = 1; idx <= 64; ++idx)
            {
                __try {
                    std::uintptr_t ctrl = GameState::GetEntityByIndex(idx);
                    if (!ctrl || ctrl == localCtrl) continue;
                    std::uint32_t pawnH = Mem::Read<std::uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                    if (!pawnH) continue;
                    std::uintptr_t pawn = GameState::ResolveHandle(pawnH);
                    if (!pawn || pawn == localPawn) continue;
                    int hp = Mem::Read<std::int32_t>(pawn + Offsets::m_iHealth);
                    if (hp <= 0) continue;
                    int team = Mem::Read<std::uint8_t>(pawn + Offsets::m_iTeamNum);
                    if (cfg.teamCheck && team == localTeam) continue;

                    // Project all bones first
                    ImVec2 scr[16];
                    bool   ok [16] = {};
                    int n = kBoneCount; if (n > 16) n = 16;
                    Math::Vec3 bp[16];
                    for (int b = 0; b < n; ++b) {
                        bp[b] = GameState::GetBonePos(pawn, kBones[b].idx);
                        if (bp[b].IsZero()) { ok[b] = false; continue; }
                        float sx, sy;
                        ok[b] = GameState::WorldToScreen(&bp[b].x, sx, sy, scrW, scrH);
                        scr[b] = ImVec2(sx, sy);
                    }

                    // Spine chain line: pelvis(0) -> spine(3,4,5) -> neck(6) -> head(7)
                    auto findIdx = [&](int boneId)->int{
                        for (int b = 0; b < n; ++b) if (kBones[b].idx == boneId) return b;
                        return -1;
                    };
                    int chain[] = { 0, 3, 4, 5, 6, 7 };
                    ImU32 lineCol = IM_COL32(220, 60, 70, 200);
                    for (int c = 0; c + 1 < (int)(sizeof(chain)/sizeof(chain[0])); ++c) {
                        int a = findIdx(chain[c]), bb = findIdx(chain[c+1]);
                        if (a < 0 || bb < 0 || !ok[a] || !ok[bb]) continue;
                        dl->AddLine(scr[a], scr[bb], lineCol, 1.5f);
                    }

                    // Dots
                    for (int b = 0; b < n; ++b) {
                        if (!ok[b]) continue;
                        // Primary (target-eligible) bone = solid bigger red.
                        // Secondary = smaller dim red.
                        float r = kBones[b].primary ? 4.5f : 3.0f;
                        ImU32 col = kBones[b].primary
                            ? IM_COL32(255, 60, 70, 235)
                            : IM_COL32(220, 80, 90, 200);
                        // If predictor said HIT this snap and the
                        // target-idx matches, paint the head bone green.
                        if (s.hit && (int)((pawn >> 4) & 0xFFFF) == s.targetIdx
                            && kBones[b].idx == 7) {
                            col = IM_COL32(120, 240, 130, 255);
                            r += 1.5f;
                        }
                        dl->AddCircleFilled(scr[b], r, col, 12);
                        dl->AddCircle(scr[b], r + 0.5f, IM_COL32(0, 0, 0, 200), 12, 1.f);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }
        }
        const float pw = 320.f, ph = 240.f;
        ImVec2 p0(ds.x - pw - 12.f, 80.f);
        ImVec2 p1(p0.x + pw, p0.y + ph);

        // Background
        dl->AddRectFilled(p0, p1, IM_COL32(8, 10, 14, 220), 6.f);
        dl->AddRect(p0, p1, IM_COL32(255, 70, 90, 220), 6.f, 0, 1.5f);

        char buf[160];
        float x = p0.x + 10.f, y = p0.y + 8.f;
        ImU32 white = IM_COL32(235, 235, 235, 255);
        ImU32 dim   = IM_COL32(170, 170, 175, 255);
        ImU32 ok    = IM_COL32(120, 240, 130, 255);
        ImU32 bad   = IM_COL32(255, 90, 90, 255);
        ImU32 acc   = IM_COL32(255, 200, 80, 255);

        dl->AddText(ImVec2(x, y), acc, "SEEDED TRIGGER \u2014 LIVE"); y += 16.f;
        dl->AddText(ImVec2(x, y), dim,  "------------------------"); y += 14.f;

        DWORD age = GetTickCount() - s.ms;
        snprintf(buf, sizeof(buf), "snapshot age   : %u ms", (unsigned)age);
        dl->AddText(ImVec2(x, y), age < 300 ? white : bad, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "tick           : %d", s.lastTick);
        dl->AddText(ImVec2(x, y), white, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "itemDef        : %u   %s",
                 s.itemDef, s.cacheHot ? "(HOT)" : "(cold)");
        dl->AddText(ImVec2(x, y), s.cacheHot ? white : bad, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "view yaw/pitch : %.2f / %.2f",
                 s.yaw, s.pitch);
        dl->AddText(ImVec2(x, y), white, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "aim_punch p/y  : %.3f / %.3f",
                 s.punchPitch, s.punchYaw);
        dl->AddText(ImVec2(x, y), dim, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "baseSpread     : %.5f", s.baseSpread);
        dl->AddText(ImVec2(x, y), dim, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "inaccuracy     : %.5f", s.inaccuracy);
        dl->AddText(ImVec2(x, y), dim, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "miss / radius  : %.2f u  /  %.2f u",
                 s.missDist, s.radius);
        dl->AddText(ImVec2(x, y),
                    (s.missDist <= s.radius) ? ok : bad, buf); y += 14.f;

        snprintf(buf, sizeof(buf), "hit streak     : %d / %d %s",
                 s.hitsInWindow, s.neededHits,
                 s.hit ? "HIT" : "miss");
        dl->AddText(ImVec2(x, y),
                    (s.hitsInWindow >= s.neededHits) ? ok : (s.hit ? acc : bad),
                    buf); y += 14.f;

        snprintf(buf, sizeof(buf), "target idx     : %d", s.targetIdx);
        dl->AddText(ImVec2(x, y), s.targetIdx ? white : dim, buf); y += 14.f;

        // Streak bar
        y += 4.f;
        const float bw = pw - 20.f;
        ImVec2 b0(x, y), b1(x + bw, y + 8.f);
        dl->AddRectFilled(b0, b1, IM_COL32(40, 40, 48, 255), 3.f);
        if (s.neededHits > 0) {
            float frac = (float)s.hitsInWindow / (float)s.neededHits;
            if (frac > 1.f) frac = 1.f;
            ImVec2 f1(x + bw * frac, y + 8.f);
            dl->AddRectFilled(b0, f1,
                              (frac >= 1.f) ? IM_COL32(120, 240, 130, 255)
                                            : IM_COL32(255, 200, 80, 255),
                              3.f);
        }
        y += 14.f;

        // Counters
        std::uint64_t tries  = g_predictTries.load();
        std::uint64_t hits   = g_predictHits.load();
        std::uint64_t clicks = g_clicksSent.load();
        snprintf(buf, sizeof(buf), "tries/hits/fire: %llu / %llu / %llu",
                 (unsigned long long)tries,
                 (unsigned long long)hits,
                 (unsigned long long)clicks);
        dl->AddText(ImVec2(x, y), white, buf); y += 14.f;

        // Recent fire flash
        DWORD perf = g_perfectUntilMs.load();
        DWORD synth = g_synthClickUntilMs.load();
        DWORD nowM = GetTickCount();
        snprintf(buf, sizeof(buf), "perfect win    : %s  synth: %s",
                 (perf > nowM)  ? "ARMED" : "off",
                 (synth > nowM) ? "ARMED" : "off");
        dl->AddText(ImVec2(x, y),
                    (perf > nowM) ? ok : dim, buf); y += 14.f;

        // ---- Crosshair miss-circle ----
        // Shows the closest miss distance vs the hitbox radius as
        // concentric circles around screen centre. Inner = miss,
        // outer = radius. When inner <= outer the predictor said HIT.
        if (s.radius > 0.f) {
            ImVec2 cc(ds.x * 0.5f, ds.y * 0.5f);
            // Pixels per unit. Calibrate so a 5u headbox is ~30px.
            const float ppu = 6.f;
            float rPx = s.radius * ppu;
            float mPx = s.missDist * ppu;
            if (mPx > rPx * 4.f) mPx = rPx * 4.f;     // clamp out-of-frame

            dl->AddCircle(cc, rPx,
                          (s.hit ? IM_COL32(120, 240, 130, 200)
                                 : IM_COL32(180, 180, 180, 160)),
                          48, 1.5f);
            dl->AddCircle(cc, mPx,
                          (s.hit ? IM_COL32(120, 240, 130, 220)
                                 : IM_COL32(255, 90, 90, 220)),
                          48, 1.5f);
            // small marker dot at miss radius (north)
            dl->AddCircleFilled(ImVec2(cc.x, cc.y - mPx), 2.5f,
                                IM_COL32(255, 255, 255, 255));
            // fired flash ring
            if (s.fired && age < 300) {
                float fAlpha = 1.f - (age / 300.f);
                dl->AddCircle(cc, rPx + 6.f + age * 0.05f,
                              IM_COL32(120, 240, 130, (int)(fAlpha * 220)),
                              48, 2.f);
            }
        }
    }
}
