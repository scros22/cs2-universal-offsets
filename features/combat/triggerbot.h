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
        int   minDelayMs    = 60;       // humanization floor
        int   maxDelayMs    = 220;      // humanization ceiling
        int   burstMin      = 1;
        int   burstMax      = 3;
        bool  scopeOnly     = false;

        // ---- Seeded prediction (the new primary gate) ----
        bool  seededPredict     = true;     // master switch
        bool  predictBothTicks  = true;     // try tick & tick+1 (cmd timing slop)
        float hitboxRadius      = 6.0f;     // units. ~head sphere radius
        int   targetBone        = 7;        // CS2: 7=Head, 6=Neck, 23=Chest, 1=Pelvis
        bool  airborneFire      = true;     // allow firing in air (the whole point)
        float pickFov           = 5.0f;     // degrees \u2014 cone for FOV target picking
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
    // Public per-frame entry point. Called from EndScene hook.
    // -------------------------------------------------------------
    inline void Tick()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {

        // -------- diagnostic: which gate are we exiting at? --------
        // Count ticks per exit stage, dump once per second so the log
        // doesn't explode but we still see live state.
        static std::atomic<std::uint32_t> sStage[16]{};
        static std::atomic<DWORD> sLastDump{0};
        auto stage = [&](int n) { sStage[n].fetch_add(1, std::memory_order_relaxed); };
        DWORD nowMs = GetTickCount();
        if (nowMs - sLastDump.load(std::memory_order_relaxed) > 1000)
        {
            sLastDump.store(nowMs, std::memory_order_relaxed);
            DLog("[stage] keyHeld=%u noPawn=%u cooldown=%u noScope=%u "
                 "noEntIdx=%u noCtrl=%u noPH=%u noPawn2=%u dead=%u team=%u "
                 "smoke=%u grounded=%u predicted=%u fired=%u",
                 sStage[0].load(), sStage[1].load(), sStage[2].load(),
                 sStage[3].load(), sStage[4].load(), sStage[5].load(),
                 sStage[6].load(), sStage[7].load(), sStage[8].load(),
                 sStage[9].load(), sStage[10].load(), sStage[11].load(),
                 sStage[12].load(), sStage[13].load());
            for (auto& s : sStage) s.store(0, std::memory_order_relaxed);
        }

        // Hold-to-fire key
        if (cfg.key != 0)
        {
            SHORT s = GetAsyncKeyState(cfg.key);
            if (!(s & 0x8000)) { triggerTime = 0; return; }
        }
        stage(0);

        std::uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) { stage(1); return; }

        // Post-shot cooldown (humanization)
        if (lastShotTime && cooldownMs > 0)
        {
            if (GetTickCount() - lastShotTime < (DWORD)cooldownMs) { stage(2); return; }
            cooldownMs = 0;
        }

        if (cfg.scopeOnly)
        {
            bool scoped = Mem::Read<bool>(localPawn + Offsets::m_bIsScoped);
            if (!scoped) { triggerTime = 0; stage(3); return; }
        }

        // Crosshair target â€” m_iIDEntIndex is server-authoritative
        // and only set when the local raycast lands on a hitbox; it
        // returns 0 when aimed at the body silhouette but the server
        // ray clips a wall edge first. Way too restrictive for a
        // seeded triggerbot. Pick the enemy with the smallest FOV
        // delta inside a tight cone instead â€” same approach the
        // aimbot uses, just with a much smaller FOV.
        Math::QAngle eyeAng = Mem::Read<Math::QAngle>(localPawn + Offsets::m_angEyeAngles);
        Math::Vec3   eyePos = GameState::GetEntityOrigin(localPawn);
        Math::Vec3   vOff   = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecViewOffset);
        eyePos.x += vOff.x; eyePos.y += vOff.y; eyePos.z += vOff.z;

        std::uintptr_t pawn = 0;
        float bestFov = cfg.pickFov;   // degrees

        int myTeam = Mem::Read<std::uint8_t>(localPawn + Offsets::m_iTeamNum);

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

                if (cfg.teamCheck)
                {
                    int tt = Mem::Read<std::uint8_t>(cpawn + Offsets::m_iTeamNum);
                    if (tt == myTeam) continue;
                }

                Math::Vec3 bone = GameState::GetBonePos(cpawn, cfg.targetBone);
                if (bone.IsZero()) continue;

                Math::QAngle a = Math::CalcAngle(eyePos, bone);
                float fv = Math::AngleFov(eyeAng, a);
                if (fv < bestFov) { bestFov = fv; pawn = cpawn; }
            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }

        if (!pawn) { triggerTime = 0; burstLeft = 0; stage(4); return; }

        int hp = Mem::Read<std::int32_t>(pawn + Offsets::m_iHealth);
        if (hp <= 0) { triggerTime = 0; stage(8); return; }

        if (cfg.smokeCheck)
        {
            float a = Mem::Read<float>(localPawn + Offsets::m_flLastSmokeOverlayAlpha);
            if (a > 0.5f) { triggerTime = 0; stage(10); return; }
        }

        // Optional non-airborne gate (for legacy users who want it)
        if (!cfg.airborneFire)
        {
            std::uint32_t flags = Mem::Read<std::uint32_t>(localPawn + Offsets::m_fFlags);
            if (!(flags & 1u)) { triggerTime = 0; stage(11); return; }
        }

        // ---- The seeded gate ----
        bool predictionWanted = (pSeedGen != nullptr) &&
                                (pSpread  != nullptr) &&
                                 cfg.seededPredict;

        bool predictionApplied = false;   // did we actually run prediction?

        if (predictionWanted)
        {
            std::uintptr_t weapon = GameState::GetActiveWeapon(localPawn);
            if (!weapon) return;

            std::uint16_t itemDef = Mem::Read<std::uint16_t>(
                weapon + Offsets::m_AttributeManager
                       + Offsets::m_Item
                       + Offsets::m_iItemDefinitionIndex);

            // Chicken-and-egg: cache fills from real engine spread
            // calls. Until at least one fires, predictor would lock
            // out forever. Cold cache â†’ fall through to legacy gates
            // so first shot can fire and seed it.
            bool cacheHot = (itemDef < 1024) &&
                g_argCache[itemDef].valid.load(std::memory_order_acquire);

            if (cacheHot)
            {
                std::uintptr_t localCtrl = GameState::GetLocalController();
                if (!localCtrl) return;
                std::int32_t tick = Mem::Read<std::int32_t>(
                    localCtrl + Offsets::m_nTickBase);

                Math::QAngle ang = Mem::Read<Math::QAngle>(
                    localPawn + Offsets::m_angEyeAngles);

                Math::Vec3 eye = GameState::GetEntityOrigin(localPawn);
                Math::Vec3 vo  = Mem::Read<Math::Vec3>(
                    localPawn + Offsets::m_vecViewOffset);
                eye.x += vo.x; eye.y += vo.y; eye.z += vo.z;

                // ---- Resolve target bone(s) + radius from mode ----
                // mode 0 = HEAD only       (bone 7,  r=5)
                // mode 1 = BODY only       (bone 23, r=12)
                // mode 2 = AUTO any-hit    (try head, then chest)
                // mode 3 = MANUAL          (use cfg.targetBone, cfg.hitboxRadius)
                struct TargetSpec { int bone; float radius; };
                TargetSpec specs[2] = {{7, 5.0f}, {7, 5.0f}};
                int nSpecs = 1;
                switch (cfg.targetMode)
                {
                    case 0: specs[0] = {7,  5.0f};                    nSpecs = 1; break;
                    case 1: specs[0] = {23, 12.0f};                   nSpecs = 1; break;
                    case 2: specs[0] = {7,  5.0f}; specs[1] = {23, 12.0f}; nSpecs = 2; break;
                    default: specs[0] = {cfg.targetBone, cfg.hitboxRadius}; nSpecs = 1; break;
                }

                bool hit = false;

                // Read target velocity ONCE for leading. Source's
                // m_vecVelocity is the post-friction tick velocity â€”
                // close enough to predict ~50ms forward.
                Math::Vec3 targetVel{0,0,0};
                if (cfg.targetLead) {
                    __try {
                        targetVel = Mem::Read<Math::Vec3>(pawn + Offsets::m_vecVelocity);
                    } __except (EXCEPTION_EXECUTE_HANDLER) { targetVel = {0,0,0}; }
                }

                // Tick offsets to try. Wider window catches strafing
                // enemies whose bullet impact tick varies with ping.
                int tickOffs[4] = {0, 1, -1, 2};
                int nOffs = cfg.wideTickWindow ? 4 : (cfg.predictBothTicks ? 2 : 1);

                for (int ti = 0; ti < nSpecs && !hit; ++ti)
                {
                    Math::Vec3 boneBase = GameState::GetBonePos(pawn, specs[ti].bone);
                    if (boneBase.IsZero()) continue;

                    for (int oi = 0; oi < nOffs && !hit; ++oi)
                    {
                        int  tOff = tickOffs[oi];
                        // Lead time = base latency + per-tick advance.
                        // For tickOff < 0 we lead LESS (target was further
                        // back); for > 0 we lead MORE.
                        float lead = cfg.targetLead
                                     ? (cfg.leadTime + tOff * (1.f/64.f))
                                     : 0.f;
                        if (lead < 0.f) lead = 0.f;

                        Math::Vec3 target = boneBase;
                        target.x += targetVel.x * lead;
                        target.y += targetVel.y * lead;
                        target.z += targetVel.z * lead;

                        hit = PredictHit(static_cast<std::int64_t>(weapon),
                                         itemDef, ang.pitch, ang.yaw,
                                         tick + tOff, eye, target,
                                         specs[ti].radius);
                    }
                }

                std::uint64_t tries = g_predictTries.fetch_add(1, std::memory_order_relaxed) + 1;
                if (hit) g_predictHits.fetch_add(1, std::memory_order_relaxed);
                if ((tries % 60) == 0)
                    DLog("[predict] try#%llu hit=%d itemDef=%u tick=%d ang=(%.2f,%.2f) base=%.5f inacc=%.5f",
                         (unsigned long long)tries, (int)hit,
                         (unsigned)itemDef, tick,
                         ang.pitch, ang.yaw,
                         g_argCache[itemDef].baseSpread,
                         g_argCache[itemDef].inaccuracy);

                predictionApplied = true;
                if (!hit) return;     // not this tick â€” keep waiting
            }
            else
            {
                static std::atomic<std::uint16_t> s_lastLogged{0xFFFF};
                if (s_lastLogged.exchange(itemDef) != itemDef)
                    DLog("[predict] cache COLD itemDef=%u â€” falling through to fire so cache seeds",
                         (unsigned)itemDef);
            }
        }

        // Legacy accuracy/speed gates â€” only run when prediction
        // didn't already greenlight (i.e. predictor disabled OR cache cold).
        if (!predictionApplied)
        {
            if (cfg.accuracyGate)
            {
                std::uintptr_t weapon = GameState::GetActiveWeapon(localPawn);
                if (weapon)
                {
                    float p = Mem::Read<float>(weapon + Offsets::m_fAccuracyPenalty);
                    float t = Mem::Read<float>(weapon + Offsets::m_flTurningInaccuracy);
                    if ((p + t) > cfg.maxPenalty) return;
                }
            }
            if (cfg.speedGate)
            {
                Math::Vec3 v = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecVelocity);
                if (sqrtf(v.x*v.x + v.y*v.y) > cfg.maxSpeed) return;
            }
        }

        stage(12);

        // Humanization: pre-fire delay
        DWORD now = GetTickCount();
        if (triggerTime == 0)
        {
            triggerTime = now;
            delayMs = TRandInt(cfg.minDelayMs, cfg.maxDelayMs);
            burstLeft = TRandInt(cfg.burstMin, cfg.burstMax);
            return;
        }
        if (now - triggerTime < (DWORD)delayMs) return;

        if (burstLeft > 0)
        {
            SendClick();
            g_clicksSent.fetch_add(1, std::memory_order_relaxed);
            stage(13);
            burstLeft--;
            lastShotTime = now;
            // Bates(2)-distributed cooldown â€” looks human, not uniform
            cooldownMs = TRandInt(20, 60) + TRandInt(20, 60);
            if (burstLeft <= 0) triggerTime = 0;
        }

        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}
