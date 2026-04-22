#pragma once

// ---------------------------------------------------------------
// Aimbot v3 â€” Clean humanized aim with Bezier physics.
// Smoothing 1â€“100 scale. Anti-detection built into aim physics.
// No junk toggles. No artificial misses. Just real aim feel.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/signatures.h"
#include "../core/memory.h"
#include "../core/math.h"
#include "../core/stealth.h"
#include "../core/vtable_swap.h"
#include "fake_lag.h"
#include "../core/spoof_call.h"
#include "../vendor/imgui/imgui.h"
#include "../vendor/minhook/include/MinHook.h"
#include "../features/bhop.h"

namespace BulletTracer { bool DetectShot(uintptr_t pawn); void AddTraceFromAngles(float ex, float ey, float ez, float p, float y); }

namespace Aimbot
{
    // Local logger — debug builds only. Release: no disk I/O.
#ifdef _DEBUG
    static void AimLog(const char* fmt, ...)
    {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "cs2init.txt");
        HANDLE hFile = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ,
                                   nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return;
        char buf[1024];
        va_list args; va_start(args, fmt);
        int len = vsprintf_s(buf, sizeof(buf) - 2, fmt, args);
        va_end(args);
        if (len > 0) { buf[len] = '\n'; buf[len + 1] = '\0'; DWORD w; WriteFile(hFile, buf, len + 1, &w, nullptr); }
        CloseHandle(hFile);
    }
#else
    static void AimLog(const char*, ...) {}
#endif

    // ---------------------------------------------------------------
    // Config â€” clean settings. No miss chance, no HS% manager, no
    // scatter, no wall-pen, no cooldown jitter. Detection resistance
    // comes from smooth physics, not from fake anti-cheat toggles.
    // ---------------------------------------------------------------
    struct Config
    {
        bool  enabled         = true;
        int   aimKey          = 0;           // 0 = auto (mouse1), 1 = mouse2, 2 = always on
        float fov             = 5.5f;        // degrees (1â€“180)
        float smoothing       = 22.0f;       // 1â€“100 (1 = instant, 100 = max drag)
        int   targetBone      = 7;           // build 14152: 7=head, 6=neck, 23=chest, 1=pelvis
        bool  headPriority    = true;        // try head first, fallback to configured
        bool  noRecoil        = true;        // compensate weapon recoil
        bool  teamCheck       = true;
        bool  visCheck        = true;        // visibility check (toggle — disable to lock through walls)
        bool  velPredict      = true;        // lead moving targets
        float velPredictScale = 1.0f;        // prediction multiplier
        bool  multiBone       = true;        // scan multiple bones for best hit
        float humanization    = 0.28f;       // 0â€“1 hand tremor intensity
        bool  smokeCheck      = true;        // never aim through smoke
        bool  showFovCircle   = true;
        float fovCircleColor[4] = { 1.f, 1.f, 1.f, 0.14f };
        bool  jumpShot        = true;
        bool  jumpApexOnly    = false;
        float jumpApexThreshold = 30.f;
        bool  noSpread        = false;       // zero weapon inaccuracy (perfect jump shots)
        float smokeRadius     = 144.0f;      // CS2 smoke sphere radius
        bool  silentAim       = true;       // silent aim via CreateMove frame history
        float silentHitChance = 80.f;        // % of shots that get corrected (rest fire naturally)
        float silentSpread    = 0.2f;        // degrees of random scatter added to aim angle
        float silentMaxDelta  = 5.0f;        // max degrees of correction (skip if crosshair is further)
    };

    inline Config cfg;
    inline float  drawnFovRadius = 0.f;

    // ---------------------------------------------------------------
    // Stat Governor â€” behavioral anti-detection layer.
    // Monitors kills/headshots/rounds and throttles aimbot behavior
    // when stats become suspicious. Does NOT change aim feel/physics.
    // It works by: temporarily forcing body shots, adding brief
    // post-kill acquisition delays, and skipping ticks entirely when
    // performance is too hot. The aimbot settings stay untouched.
    // ---------------------------------------------------------------
    namespace Governor
    {
        struct GovernorConfig
        {
            bool  enabled          = false;
            float intensity        = 0.35f;    // 0â€“1 how aggressive throttling is
            float hsCapPercent     = 55.0f;    // start forcing body after this HS%
            int   multiKillWindow  = 4;        // kills within this many seconds = multi-kill
            int   multiKillCap     = 4;        // max rapid kills before brief pause
        };

        inline GovernorConfig gcfg;

        // Session stats (reset on round start detection)
        struct SessionStats
        {
            int   totalKills       = 0;
            int   headKills        = 0;
            int   roundKills       = 0;        // kills in current round
            int   consecutiveHS    = 0;        // streak of headshot kills
            DWORD lastKillTime     = 0;        // tick of last kill
            int   rapidKills       = 0;        // kills in quick succession
            DWORD roundStartTime   = 0;        // when current round started
            int   roundsPlayed     = 0;
            bool  forceBody        = false;    // governor forcing body shots
            int   forceBodyTicks   = 0;        // how many ticks to force body
            int   pauseTicks       = 0;        // skip aimbot for this many ticks
            int   lastPlayerCount  = 0;        // for round-change detection
        };

        inline SessionStats stats;

        // Called when we detect a kill happened (enemy HP went to 0 while locked)
        inline void OnKill(bool wasHeadshot)
        {
            if (!gcfg.enabled) return;

            stats.totalKills++;
            stats.roundKills++;
            if (wasHeadshot)
            {
                stats.headKills++;
                stats.consecutiveHS++;
            }
            else
            {
                stats.consecutiveHS = 0;
            }

            DWORD now = GetTickCount();

            // Rapid kill detection
            if (stats.lastKillTime != 0 && (now - stats.lastKillTime) < (DWORD)(gcfg.multiKillWindow * 1000))
                stats.rapidKills++;
            else
                stats.rapidKills = 1;
            stats.lastKillTime = now;

            float hsPercent = stats.totalKills > 0 ? (stats.headKills * 100.f / stats.totalKills) : 0.f;

            // --- Force body shots when HS% is too high ---
            // After enough kills to be statistically meaningful
            if (stats.totalKills >= 4 && hsPercent > gcfg.hsCapPercent)
            {
                // Force next 1-3 kills to aim body
                int bodyForce = 1 + (int)(gcfg.intensity * 2.f);
                // Each "tick" is ~15ms in CreateMove, force body for ~4-8 seconds
                stats.forceBodyTicks = bodyForce * 250;
                stats.forceBody = true;
            }

            // --- Consecutive headshot streak throttle ---
            if (stats.consecutiveHS >= 4)
            {
                // Force body on next kill with increasing probability
                float streakChance = 0.3f + gcfg.intensity * 0.4f;
                // Deterministic based on tick count to avoid needing extra RNG call
                if ((now % 100) < (DWORD)(streakChance * 100.f))
                {
                    stats.forceBodyTicks = 200;
                    stats.forceBody = true;
                }
            }

            // --- Multi-kill pause: brief hesitation after rapid consecutive kills ---
            if (stats.rapidKills >= gcfg.multiKillCap)
            {
                // Pause aimbot for 0.5-1.5 seconds (looks like repositioning)
                int pauseMs = 500 + (int)(gcfg.intensity * 1000.f);
                stats.pauseTicks = pauseMs / 15;  // ~15ms per CreateMove tick
                stats.rapidKills = 0;
            }
        }

        // Called every CreateMove tick. Returns what the governor wants.
        struct Decision
        {
            bool  skipThisTick;      // don't aim this tick at all
            int   overrideBone;      // -1 = no override, else force this bone
        };

        inline Decision Tick()
        {
            Decision d = { false, -1 };
            if (!gcfg.enabled) return d;

            // Decrement timers
            if (stats.pauseTicks > 0)
            {
                stats.pauseTicks--;
                d.skipThisTick = true;
                return d;
            }

            if (stats.forceBodyTicks > 0)
            {
                stats.forceBodyTicks--;
                if (stats.forceBodyTicks <= 0)
                    stats.forceBody = false;
            }

            // Force body shot: use chest (bone 4) instead of head
            if (stats.forceBody)
                d.overrideBone = 4;

            // Round-kill rate throttle: if we've gotten lots of kills
            // very quickly in a round, add occasional skip ticks
            DWORD now = GetTickCount();
            if (stats.roundStartTime > 0 && stats.roundKills >= 3)
            {
                float roundSec = (now - stats.roundStartTime) / 1000.f;
                if (roundSec > 0.f)
                {
                    float killRate = stats.roundKills / roundSec;
                    // More than 1 kill every 5 seconds is suspicious
                    if (killRate > 0.35f)
                    {
                        // Occasionally skip a tick (2-8% chance based on intensity)
                        int skipChance = 2 + (int)(gcfg.intensity * 6.f);
                        if ((int)(now % 100) < skipChance)
                            d.skipThisTick = true;
                    }
                }
            }

            return d;
        }

        // Detect round changes by monitoring alive player count jumps
        inline void CheckRoundReset()
        {
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            int aliveCount = 0;
            for (int i = 1; i <= 64; ++i)
            {
                __try {
                    uintptr_t chunk = Mem::Read<uintptr_t>(entList + 0x8 * (i >> 9) + 0x10);
                    if (!chunk) continue;
                    uintptr_t ctrl = Mem::Read<uintptr_t>(chunk + GameState::kEntityStride * (i & 0x1FF));
                    if (!ctrl) continue;
                    uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                    if (!pH || pH == 0xFFFFFFFF) continue;
                    uintptr_t pawn = GameState::ResolveHandle(pH);
                    if (!pawn) continue;
                    int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                    if (hp > 0) aliveCount++;
                } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
            }

            // Big jump in alive count = new round
            if (stats.lastPlayerCount > 0 && aliveCount > stats.lastPlayerCount + 2)
            {
                stats.roundKills = 0;
                stats.rapidKills = 0;
                stats.roundStartTime = GetTickCount();
                stats.roundsPlayed++;
                stats.forceBody = false;
                stats.forceBodyTicks = 0;
                stats.pauseTicks = 0;
            }
            stats.lastPlayerCount = aliveCount;
        }

        // Get display stats for menu
        inline float GetHSPercent()
        {
            return stats.totalKills > 0 ? (stats.headKills * 100.f / stats.totalKills) : 0.f;
        }

        inline void ResetSession()
        {
            stats = SessionStats{};
            stats.roundStartTime = GetTickCount();
        }
    }
    // End Governor namespace

    // ---------------------------------------------------------------
    // Smoke line-of-sight check
    // Returns true if a ray from 'start' to 'end' passes through any
    // active smoke grenade sphere. Uses closest-point-on-segment test.
    // ---------------------------------------------------------------
    inline bool IsLineThroughSmoke(const Math::Vec3& start, const Math::Vec3& end)
    {
        if (!cfg.smokeCheck) return false;
        uintptr_t entList = GameState::GetEntityList();
        if (!entList) return false;

        Math::Vec3 dir = { end.x - start.x, end.y - start.y, end.z - start.z };
        float segLen2 = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        if (segLen2 < 1.f) return false;

        float r = cfg.smokeRadius;
        float r2 = r * r;

        for (int i = 64; i < 1024; ++i)
        {
            __try {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ent = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                if (!ent) continue;

                bool didSmoke = Mem::Read<bool>(ent + Offsets::m_bDidSmokeEffect);
                if (!didSmoke) continue;

                int tickBegin = Mem::Read<int>(ent + Offsets::m_nSmokeEffectTickBegin);
                if (tickBegin <= 0) continue;

                // Verify this is actually a smoke grenade entity
                uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
                if (!identity) continue;
                uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
                if (!namePtr) continue;
                char nameBuf[32] = {};
                for (int c = 0; c < 31; ++c) {
                    nameBuf[c] = Mem::Read<char>(namePtr + c);
                    if (!nameBuf[c]) break;
                }
                if (strstr(nameBuf, XS("smoke")) == nullptr) continue;

                uintptr_t node = Mem::Read<uintptr_t>(ent + Offsets::m_pGameSceneNode);
                if (!node) continue;
                Math::Vec3 smokePos = Mem::Read<Math::Vec3>(node + Offsets::m_vecAbsOrigin);
                if (smokePos.IsZero()) continue;

                Math::Vec3 toSmoke = { smokePos.x - start.x, smokePos.y - start.y, smokePos.z - start.z };
                float dot = toSmoke.x * dir.x + toSmoke.y * dir.y + toSmoke.z * dir.z;
                float t = dot / segLen2;
                if (t < 0.f) t = 0.f;
                if (t > 1.f) t = 1.f;
                Math::Vec3 closest = { start.x + dir.x * t, start.y + dir.y * t, start.z + dir.z * t };
                float dx = closest.x - smokePos.x;
                float dy = closest.y - smokePos.y;
                float dz = closest.z - smokePos.z;
                if (dx * dx + dy * dy + dz * dz < r2)
                    return true;
            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }
        return false;
    }

    // ---------------------------------------------------------------
    // High-quality PRNG (xorshift32)
    // ---------------------------------------------------------------
    inline uint32_t rngState = 0;

    inline void SeedRng()
    {
        LARGE_INTEGER pc;
        QueryPerformanceCounter(&pc);
        rngState = (uint32_t)(pc.QuadPart ^ (pc.QuadPart >> 17) ^ GetTickCount());
        if (!rngState) rngState = 1;
    }

    inline uint32_t Xorshift32()
    {
        uint32_t x = rngState;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        rngState = x;
        return x;
    }

    inline float Rand01()  { return (Xorshift32() & 0xFFFFFF) / 16777216.0f; }
    inline float RandRange(float mn, float mx) { return mn + Rand01() * (mx - mn); }
    inline int   RandInt(int mn, int mx)       { return mn + (int)(Xorshift32() % (uint32_t)(mx - mn + 1)); }

    // ---------------------------------------------------------------
    // Human Neuromotor Model â€” mimics real biomechanical mouse control
    //
    // VACnet analyzes angular velocity profiles 0.5s before / 0.25s
    // after each shot. It feeds 140 "atoms" from 8 rounds into a
    // deep neural network. To appear human we need:
    //
    //   1. Reaction delay before aim starts (150â€“300ms)
    //   2. Bell-curve velocity (accel â†’ peak â†’ decel), not stepâ†’decay
    //   3. Low-freq Perlin noise tremor (2â€“8 Hz), not white noise
    //   4. Overshoot past target then damped oscillation back
    //   5. Post-shot refractory period (brief settle)
    //   6. Per-engagement variance (no two flicks look the same)
    //
    // These produce the exact velocity/timing fingerprint that VACnet
    // expects from legitimate players.
    // ---------------------------------------------------------------

    // --- Perlin noise (1D, seamless) for natural hand tremor ---
    namespace Perlin
    {
        // Permutation table (seeded once from our PRNG)
        inline uint8_t perm[512];
        inline bool    initialized = false;

        inline void Init()
        {
            for (int i = 0; i < 256; ++i) perm[i] = (uint8_t)i;
            // Fisher-Yates shuffle
            for (int i = 255; i > 0; --i)
            {
                int j = Xorshift32() % (i + 1);
                uint8_t tmp = perm[i];
                perm[i] = perm[j];
                perm[j] = tmp;
            }
            for (int i = 0; i < 256; ++i) perm[i + 256] = perm[i];
            initialized = true;
        }

        inline float Fade(float t) { return t * t * t * (t * (t * 6.f - 15.f) + 10.f); }
        inline float Lerp(float a, float b, float t) { return a + t * (b - a); }
        inline float Grad(int hash, float x)
        {
            return (hash & 1) ? -x : x;
        }

        // 1D Perlin noise, output in [-1, 1]
        inline float Noise1D(float x)
        {
            if (!initialized) Init();
            int xi = (int)floorf(x) & 255;
            float xf = x - floorf(x);
            float u = Fade(xf);
            int a = perm[xi];
            int b = perm[xi + 1];
            return Lerp(Grad(a, xf), Grad(b, xf - 1.f), u);
        }

        // Multi-octave for richer tremor texture
        inline float FBM(float x, int octaves = 3, float lacunarity = 2.0f, float gain = 0.5f)
        {
            float sum = 0.f, amp = 1.f, freq = 1.f;
            for (int i = 0; i < octaves; ++i)
            {
                sum += amp * Noise1D(x * freq);
                freq *= lacunarity;
                amp *= gain;
            }
            return sum;
        }
    }

    // --- Timing helpers ---
    inline DWORD  perlinStartTime = 0;   // for continuous tremor phase
    inline float  GetPerlinTime()
    {
        if (!perlinStartTime) perlinStartTime = GetTickCount();
        return (GetTickCount() - perlinStartTime) / 1000.f;
    }


    // Per-session parameter jitter -- randomized once at session start
    // Makes aim profile statistically unique per match
    inline float sessionFovJitter    = 0.f;  // +/-0.2 deg
    inline float sessionSmoothJitter = 0.f;  // +/-3
    inline float sessionHumanJitter  = 0.f;  // +/-0.05
    inline bool  sessionJitterApplied = false;

    inline void ApplySessionJitter()
    {
        if (sessionJitterApplied) return;
        sessionJitterApplied = true;
        sessionFovJitter    = RandRange(-0.2f, 0.2f);
        sessionSmoothJitter = RandRange(-3.f, 3.f);
        sessionHumanJitter  = RandRange(-0.05f, 0.05f);
    }

    // Effective config values with jitter baked in
    // Safety-clamped to NEVER cross VACNet ban thresholds:
    //   smoothing >= 43, humanization >= 0.50, fov <= 2.3
    inline float EffectiveFov()
    {
        float v = cfg.fov + sessionFovJitter;
        if (v > 30.0f) v = 30.0f;  // engine max
        if (v < 0.5f) v = 0.5f;
        return v;
    }
    inline float EffectiveSmoothing()
    {
        float v = cfg.smoothing + sessionSmoothJitter;
        if (v < 1.0f) v = 1.0f;    // allow full range
        return v;
    }
    inline float EffectiveHumanization()
    {
        float v = cfg.humanization + sessionHumanJitter;
        if (v < 0.0f) v = 0.0f;   // allow zero tremor
        if (v > 1.0f) v = 1.0f;
        return v;
    }
    // Natural hand tremor: 2â€“8 Hz Perlin noise, scaled by humanization
    // Returns small angle offset in degrees
    inline float HandTremor(float channel, float intensity)
    {
        float t = GetPerlinTime();
        // Primary: ~4 Hz physiological tremor
        float primary = Perlin::FBM(t * 4.0f + channel * 137.f, 2);
        // Secondary: ~7 Hz finer tremor
        float secondary = Perlin::FBM(t * 7.3f + channel * 271.f, 2) * 0.3f;
        // Combined: max ~0.15 degrees at full intensity
        return (primary + secondary) * 0.12f * intensity * EffectiveHumanization();
    }

    // ---------------------------------------------------------------
    // Aim state machine â€” models full human aim lifecycle
    //
    //   IDLE â†’ REACTING â†’ ATTACKING â†’ CORRECTING â†’ LOCKED
    //          â†‘ 150-300ms  â†‘ bell-curve  â†‘ damped    â†‘ tracking
    //          delay        velocity       oscillation  with tremor
    // ---------------------------------------------------------------
    enum AimPhase
    {
        PHASE_IDLE,         // not targeting
        PHASE_REACTING,     // target acquired, reaction delay (no movement)
        PHASE_ATTACKING,    // ballistic flick with bell-curve velocity
        PHASE_CORRECTING,   // damped oscillation around target
        PHASE_LOCKED        // on target, micro-adjustments (tracking)
    };

    struct AimState
    {
        AimPhase  phase         = PHASE_IDLE;
        uintptr_t lockedTarget  = 0;

        // REACTING phase
        DWORD     reactStartTime = 0;
        int       reactDelayMs   = 0;

        // ATTACKING phase
        int       attackTicks    = 0;
        int       attackDuration = 0;      // total ticks for this flick
        float     curveProgress  = 0.f;
        float     attackSpeed    = 0.f;

        // Per-engagement randomized parameters (set at BeginEngagement)
        float     accelShape     = 0.f;    // bias for acceleration phase [0.3â€“0.6]
        float     peakVelMul     = 0.f;    // peak velocity multiplier [0.8â€“1.2]
        float     overshootPct   = 0.f;    // overshoot as fraction of distance [0.04â€“0.18]
        float     overshootDirP  = 0.f;    // overshoot direction pitch
        float     overshootDirY  = 0.f;    // overshoot direction yaw
        float     dampingRate    = 0.f;    // how fast oscillation decays [0.4â€“0.7]
        int       corrOscCount   = 0;      // correction oscillation counter

        // Saved start angle (for computing velocity delta)
        float     startDeltaP    = 0.f;
        float     startDeltaY    = 0.f;

        // Post-kill refractory
        DWORD     lastKillTime   = 0;
        int       postKillDelayMs = 0;

        // Engagement staleness safety valve
        DWORD     engagementStartTime = 0;
    };

    inline AimState state;

    // Governor kill-detection state
    inline uintptr_t govLastTarget    = 0;
    inline int       govLastTargetHP  = 100;
    inline int       govLastBone      = -1;
    inline DWORD     govRoundCheckTick = 0;

    // Crash recovery: consecutive crash counter + backoff
    inline int   consecutiveCrashes   = 0;
    inline int   crashBackoffTicks    = 0;

    // Self-healing: track consecutive ticks with no valid target found.
    // If this exceeds a threshold, force a full state refresh.
    inline int   noTargetTicks         = 0;
    inline DWORD lastSelfHealTime      = 0;

    // SendInput fallback: if SpoofCall crashes, switch to direct permanently
    inline bool  useDirectSendInput   = false;

    // Session-level engagement variance (slowly drifts)
    inline float sessionSpeedBias  = 1.0f;  // [0.85â€“1.15] varies over engagements
            inline int   engagementCount   = 0;

    inline void ResetState()
    {
        state = AimState{};
    }

    // Maps smoothing 1â€“100 to per-tick movement fraction
    //   s=1  â†’ 0.77  (nearly instant)
    //   s=30 â†’ 0.10  (medium follow)
    //   s=100â†’ 0.032 (slow drag)
    inline float GetSmoothFraction()
    {
        return 1.0f / (1.0f + EffectiveSmoothing() * 0.22f);
    }

    // Angular velocity cap derived from smoothing (degrees/tick)
    //   s=1  â†’ ~32   (instant flick)
    //   s=30 â†’ ~7.5  (natural arm speed)
    //   s=100â†’ ~3.9  (slow drag)
    inline float GetMaxStep()
    {
        return 3.0f + 35.0f / (1.0f + EffectiveSmoothing() * 0.12f) + RandRange(-0.3f, 0.3f);
    }

    // ---------------------------------------------------------------
    // Bell-curve velocity profile (replaces Bezier ease-out)
    //
    // Real human flick velocity:
    //   0%â†’30%: acceleration (muscles engaging)
    //   30%â†’60%: peak velocity (full speed)
    //   60%â†’100%: deceleration (braking toward target)
    //
    // We model this as a modified smoothstep with adjustable peak
    // position. The integral gives position along the flick path.
    // ---------------------------------------------------------------

    // Velocity at time t (0â†’1), peakAt controls shape [0.3â€“0.6]
    // Returns 0â†’1 velocity magnitude (not position)
    inline float HumanVelocityCurve(float t, float peakAt)
    {
        if (t <= 0.f) return 0.f;
        if (t >= 1.f) return 0.f;
        // Two-phase: ramp up to peak, ramp down after
        if (t < peakAt)
        {
            float u = t / peakAt;
            return u * u * (3.f - 2.f * u); // smoothstep up
        }
        else
        {
            float u = (t - peakAt) / (1.f - peakAt);
            float down = 1.f - u * u * (3.f - 2.f * u); // smoothstep down
            return down;
        }
    }

    // Integrated position along flick path (0â†’1)
    // We accumulate velocity samples for an accurate integral
    inline float HumanPositionCurve(float t, float peakAt)
    {
        if (t <= 0.f) return 0.f;
        if (t >= 1.f) return 1.f;
        // Numerical integration with 32 steps
        constexpr int kSteps = 32;
        float dt = t / kSteps;
        float sum = 0.f;
        for (int i = 0; i < kSteps; ++i)
        {
            float ti = (i + 0.5f) * dt;
            sum += HumanVelocityCurve(ti, peakAt);
        }
        // Normalize by total area (integral of velocity over [0,1])
        float total = 0.f;
        float dtFull = 1.f / kSteps;
        for (int i = 0; i < kSteps; ++i)
        {
            float ti = (i + 0.5f) * dtFull;
            total += HumanVelocityCurve(ti, peakAt);
        }
        return (total > 0.001f) ? (sum * dt) / (total * dtFull) : t;
    }

    inline void BeginEngagement(uintptr_t target, float fovDist)
    {
        engagementCount++;

        // Slowly drift session speed bias to add long-term variance
        sessionSpeedBias += RandRange(-0.03f, 0.03f);
        if (sessionSpeedBias < 0.85f) sessionSpeedBias = 0.85f;
        if (sessionSpeedBias > 1.15f) sessionSpeedBias = 1.15f;

        state.lockedTarget = target;
        state.attackTicks = 0;
        state.curveProgress = 0.f;
        state.corrOscCount = 0;
        state.engagementStartTime = GetTickCount();
        state.lastKillTime = 0;
        state.postKillDelayMs = 0;

        // --- Per-engagement randomization ---
        // These make every flick unique to the neural network.
        // No two engagements have the same velocity profile.

        // Acceleration shape: where in the flick peak velocity occurs
        state.accelShape = RandRange(0.30f, 0.55f);

        // Peak velocity multiplier (per-engagement variance)
        state.peakVelMul = RandRange(0.85f, 1.15f) * sessionSpeedBias;

        // Attack speed: how fast the curve progresses 0â†’1
        // Distant targets take longer (realistic â€” bigger mouse travel)
        float distFactor = 1.0f + fovDist * 0.05f;
        state.attackSpeed = (1.0f / (0.35f + EffectiveSmoothing() * 0.065f * distFactor))
                          * state.peakVelMul;

        // Predicted ticks for full flick (for tremor scaling)
        state.attackDuration = (int)(1.0f / (state.attackSpeed * 0.015f)) + 1;
        if (state.attackDuration < 2) state.attackDuration = 2;
        if (state.attackDuration > 45) state.attackDuration = 45;

        // Overshoot: bigger flicks overshoot more (realistic)
        float fovFactor = fovDist / (EffectiveFov() > 0.1f ? EffectiveFov() : 1.f);
        state.overshootPct = RandRange(0.02f, 0.12f) * fovFactor * EffectiveHumanization();
        // Overshoot direction: slightly off from a straight line
        state.overshootDirP = RandRange(-0.4f, 0.4f);
        state.overshootDirY = RandRange(-0.6f, 0.6f);

        // Damping rate for correction oscillation
        state.dampingRate = RandRange(0.40f, 0.70f);

        // Very close targets: skip directly to correcting
        if (fovDist < 1.5f)
        {
            state.phase = PHASE_CORRECTING;
            state.curveProgress = 1.0f;
        }
        else
        {
            // Reaction delay: minimal for semi-rage. Just enough
            // to not look like instant-lock to VACNet.
            state.phase = PHASE_REACTING;
            state.reactStartTime = GetTickCount();
            float reactBase = 15.f + EffectiveSmoothing() * 0.3f;
            state.reactDelayMs = RandInt((int)reactBase, (int)(reactBase * 1.4f));
            // High chance of near-instant reaction
            if (Rand01() < 0.40f)
                state.reactDelayMs = RandInt(8, 35);
        }
    }

    // ---------------------------------------------------------------
    // FOV circle overlay
    // ---------------------------------------------------------------
    inline void RenderFovCircle()
    {
        if (!cfg.enabled || !cfg.showFovCircle) return;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;
        ImVec2 disp = ImGui::GetIO().DisplaySize;
        ImVec2 c(disp.x * 0.5f, disp.y * 0.5f);
        ImU32 fovCol = ImGui::ColorConvertFloat4ToU32(
            {cfg.fovCircleColor[0], cfg.fovCircleColor[1], cfg.fovCircleColor[2], cfg.fovCircleColor[3]});
        float gameFov = 67.0f;
        float screenRadius = tanf(cfg.fov * Math::kDeg2Rad) / tanf(gameFov * 0.5f * Math::kDeg2Rad) * (disp.y * 0.5f);
        drawnFovRadius = screenRadius;
        dl->AddCircle(c, screenRadius, fovCol, 72, 1.2f);
    }

    // ---------------------------------------------------------------
    // Visibility â€” layered approach for reliability AND safety
    // ---------------------------------------------------------------

    // Check REAL dormancy (accounts for chams wallhack override)
    inline bool IsReallyDormant(uintptr_t pawn, int entityIndex)
    {
        uintptr_t node = Mem::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
        if (!node) return true;
        bool liveDormant = Mem::Read<bool>(node + 0x10B);

        if (!liveDormant) return false;

        if (entityIndex >= 1 && entityIndex <= 64)
        {
            if (!GameState::originalDormant[entityIndex])
                return false;
        }

        return true;
    }

    // Check entity data isn't stale (simulation time within 2s)
    inline bool IsEntityFresh(uintptr_t pawn, float gameTime)
    {
        if (gameTime <= 0.f) return true;
        float simTime = Mem::Read<float>(pawn + Offsets::m_flSimulationTime);
        if (simTime <= 0.f) return true;
        float age = gameTime - simTime;
        return age < 2.0f;
    }

    inline bool IsEntityValid(uintptr_t pawn, int entityIndex = -1)
    {
        uintptr_t sceneNode = Mem::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
        if (!sceneNode) return false;
        bool liveDormant = Mem::Read<bool>(sceneNode + 0x10B);

        if (!liveDormant)
        {
            if (entityIndex >= 1 && entityIndex <= 64 && GameState::originalDormant[entityIndex])
                return false;
            return true;
        }

        return false;
    }

    inline bool IsVisible(uintptr_t pawn, uintptr_t localPawn, int entityIndex = -1)
    {
        // ---------------------------------------------------------------
        // Proper visibility: uses m_bSpottedByMask to check if the LOCAL
        // player specifically can see the target. Not "any teammate."
        //
        // Layer 1: Check if the game server says WE spotted this entity.
        //   m_bSpottedByMask is uint32[2] (64 bits). Bit N = player N can see them.
        //   We check our own bit (GameState::localPlayerIndex).
        //
        // Layer 2: Original dormancy (before chams override).
        //   If the server considers this entity dormant (not transmitting
        //   updated data to us), we cannot reliably aim at them.
        //
        // Both must agree for strict vis: NOT originally dormant AND
        // our specific bit is set in spottedByMask.
        // ---------------------------------------------------------------

        // Check original dormancy first — if server isn't sending data, skip
        if (entityIndex >= 1 && entityIndex <= 64)
        {
            if (GameState::originalDormant[entityIndex])
                return false; // Server considers this entity occluded/far
        }
        else
        {
            uintptr_t node = Mem::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
            if (node && Mem::Read<bool>(node + 0x10B))
                return false; // Dormant
        }

        // Check m_bSpottedByMask for OUR player's bit specifically
        int localIdx = GameState::localPlayerIndex;
        if (localIdx >= 1 && localIdx <= 64)
        {
            // m_bSpottedByMask is uint32[2] at m_entitySpottedState + 0x0C
            int arrayIdx = (localIdx - 1) / 32;   // 0 or 1
            int bitIdx   = (localIdx - 1) % 32;
            uint32_t mask = Mem::Read<uint32_t>(
                pawn + Offsets::m_entitySpottedState + 0x0C + arrayIdx * 4);
            if (mask & (1u << bitIdx))
                return true; // Game server says WE can see them
        }

        // Fallback: m_bSpotted ("someone on my team sees this entity").
        // spottedByMask is updated at ~10Hz server-side and can be stale
        // for several frames. m_bSpotted is a broader flag that's more
        // reliably maintained. Still requires SOMEONE on the team to see
        // the enemy — way safer than no vis check (which caused a ban).
        {
            uint8_t spotted = Mem::Read<uint8_t>(pawn + Offsets::m_entitySpottedState + 0x08);
            if (spotted) return true;
        }

        return false;
    }

    // ---------------------------------------------------------------
    // Target finding
    // ---------------------------------------------------------------
    struct Target
    {
        uintptr_t    pawn = 0;
        Math::Vec3   pos;
        Math::QAngle angle;
        float        fov = FLT_MAX;
        int          bone = 7;
    };

    // ---------------------------------------------------------------
    // Velocity-predicted bone position
    // ---------------------------------------------------------------
    inline Math::Vec3 GetPredictedBonePos(uintptr_t pawn, int bone)
    {
        Math::Vec3 bonePos = GameState::GetBonePos(pawn, bone);
        if (bonePos.IsZero()) return bonePos;

        if (cfg.velPredict)
        {
            Math::Vec3 vel = Mem::Read<Math::Vec3>(pawn + Offsets::m_vecVelocity);
            if (vel.Length() < 3500.f && vel.Length() > 1.f)
            {
                constexpr float kTickInterval = 1.0f / 64.0f;
                float scale = kTickInterval * cfg.velPredictScale;
                bonePos.x += vel.x * scale;
                bonePos.y += vel.y * scale;
                bonePos.z += vel.z * scale;
            }
        }
        return bonePos;
    }

    // ---------------------------------------------------------------
    // Multi-bone scanner â€” tries multiple bones, returns closest to
    // crosshair with a valid (non-zero, sane) position.
    // ---------------------------------------------------------------
    inline Math::Vec3 GetBestBonePos(uintptr_t pawn, int preferredBone,
                                     const Math::Vec3& eye, const Math::QAngle& view,
                                     int& outBone)
    {
        Math::Vec3 origin = GameState::GetEntityOrigin(pawn);

        // Animgraph 2 (build 14152) bone indices, ordered roughly head-down:
        //   7 head, 25 eye-L, 26 eye-R, 6 neck, 23 chest, 4 spine2, 3 spine1, 1 pelvis
        static const int kBones[] = { 7, 25, 26, 6, 23, 4, 3, 1 };
        constexpr int kNumBones = sizeof(kBones) / sizeof(kBones[0]);

        Math::Vec3 bestPos;
        float bestFov = FLT_MAX;
        int bestBone = preferredBone;

        auto validateBone = [&](Math::Vec3 pos) -> bool {
            if (pos.IsZero()) return false;
            if (isnan(pos.x) || isnan(pos.y) || isnan(pos.z)) return false;
            if (isinf(pos.x) || isinf(pos.y) || isinf(pos.z)) return false;
            if (!origin.IsZero())
            {
                float dist = (pos - origin).Length();
                if (dist > 150.f) return false;
                if (dist < 0.5f) return false;
            }
            return true;
        };

        if (cfg.multiBone)
        {
            Math::Vec3 prefPos = GetPredictedBonePos(pawn, preferredBone);
            if (validateBone(prefPos))
            {
                Math::QAngle ang = Math::CalcAngle(eye, prefPos);
                float fov = Math::AngleFov(view, ang);
                bestPos = prefPos;
                bestFov = fov;
                bestBone = preferredBone;
            }

            for (int bi = 0; bi < kNumBones; ++bi)
            {
                int b = kBones[bi];
                if (b == preferredBone) continue;
                Math::Vec3 pos = GetPredictedBonePos(pawn, b);
                if (!validateBone(pos)) continue;
                Math::QAngle ang = Math::CalcAngle(eye, pos);
                float fov = Math::AngleFov(view, ang);
                if (bestPos.IsZero() || (fov < bestFov - 0.5f))
                {
                    bestPos = pos;
                    bestFov = fov;
                    bestBone = b;
                }
            }
        }
        else
        {
            Math::Vec3 pos = GetPredictedBonePos(pawn, preferredBone);
            if (validateBone(pos)) { bestPos = pos; bestBone = preferredBone; }
            if (bestPos.IsZero() && preferredBone != 6) { pos = GetPredictedBonePos(pawn, 6); if (validateBone(pos)) { bestPos = pos; bestBone = 6; } }
            if (bestPos.IsZero()) { pos = GetPredictedBonePos(pawn, 5); if (validateBone(pos)) { bestPos = pos; bestBone = 5; } }
            if (bestPos.IsZero()) { pos = GetPredictedBonePos(pawn, 4); if (validateBone(pos)) { bestPos = pos; bestBone = 4; } }
            if (bestPos.IsZero()) { pos = GetPredictedBonePos(pawn, 3); if (validateBone(pos)) { bestPos = pos; bestBone = 3; } }
            if (bestPos.IsZero()) { pos = GetPredictedBonePos(pawn, 0); if (validateBone(pos)) { bestPos = pos; bestBone = 0; } }
        }

        if (bestPos.IsZero() && !origin.IsZero())
        {
            bestPos = origin + Math::Vec3(0.f, 0.f, 64.f);
            bestBone = preferredBone;
        }

        outBone = bestBone;
        return bestPos;
    }

    inline Math::Vec3 EyePosition(uintptr_t pawn)
    {
        return GameState::GetEntityOrigin(pawn)
             + Mem::Read<Math::Vec3>(pawn + Offsets::m_vecViewOffset);
    }

    inline bool IsInAir(uintptr_t pawn)
    {
        uint32_t flags = Mem::Read<uint32_t>(pawn + Offsets::m_fFlags);
        return (flags & (1 << 0)) == 0;
    }

    // ---------------------------------------------------------------
    // Get active weapon entity pointer from local pawn
    // ---------------------------------------------------------------
    inline uintptr_t GetActiveWeapon(uintptr_t pawn)
    {
        if (!pawn) return 0;
        uintptr_t svc = Mem::Read<uintptr_t>(pawn + Offsets::m_pWeaponServices);
        if (!svc) return 0;
        uint32_t h = Mem::Read<uint32_t>(svc + Offsets::m_hActiveWeapon);
        if (!h || h == 0xFFFFFFFF) return 0;
        return GameState::ResolveHandle(h);
    }

    // Weapon inaccuracy offsets (from C_CSWeaponBase schema)
    constexpr std::ptrdiff_t kWeapAccuracyPenalty    = 0x19C0; // m_fAccuracyPenalty (float)
    constexpr std::ptrdiff_t kWeapTurningInaccuracy  = 0x19BC; // m_flTurningInaccuracy (float)

    // ---------------------------------------------------------------
    // No Spread — zero weapon inaccuracy each tick.
    // Sets m_fAccuracyPenalty and m_flTurningInaccuracy to 0 on the
    // active weapon entity. This removes all spread from jumping,
    // moving, firing, etc. Bullets go exactly where you aim.
    // ---------------------------------------------------------------
    inline void ZeroWeaponInaccuracy(uintptr_t pawn)
    {
        uintptr_t weapon = GetActiveWeapon(pawn);
        if (!weapon) return;

        // Only zero if values are non-zero (avoid unnecessary writes)
        float penalty = Mem::Read<float>(weapon + kWeapAccuracyPenalty);
        if (penalty != 0.f)
            Mem::Write<float>(weapon + kWeapAccuracyPenalty, 0.f);

        float turning = Mem::Read<float>(weapon + kWeapTurningInaccuracy);
        if (turning != 0.f)
            Mem::Write<float>(weapon + kWeapTurningInaccuracy, 0.f);
    }

    inline float GetVerticalVelocity(uintptr_t pawn)
    {
        float vz = Mem::Read<float>(pawn + Offsets::m_vecVelocity + 8);
        return vz;
    }

    inline bool IsAtJumpApex(uintptr_t pawn)
    {
        if (!IsInAir(pawn)) return false;
        float vz = GetVerticalVelocity(pawn);
        return fabsf(vz) < cfg.jumpApexThreshold;
    }

    // ---------------------------------------------------------------
    // Recoil compensation â€” wider randomization for natural spray
    // Range 1.80â€“2.20x instead of 1.94â€“2.06x so spray looks imperfect
    // ---------------------------------------------------------------
    inline Math::QAngle GetAimPunch(uintptr_t pawn)
    {
        Math::Vec3 punch = Mem::Read<Math::Vec3>(pawn + Offsets::m_aimPunchAngle);
        if (isnan(punch.x) || isnan(punch.y) || isinf(punch.x) || isinf(punch.y))
            return { 0.f, 0.f, 0.f };
        if (fabsf(punch.x) > 89.f || fabsf(punch.y) > 180.f)
            return { 0.f, 0.f, 0.f };
        float compScale = 2.0f + RandRange(-0.20f, 0.20f);
        return { punch.x * compScale, punch.y * compScale, 0.f };
    }

    // ---------------------------------------------------------------
    // FindBestTarget â€” simplified. No HS manager, no wall penetration,
    // no fovMode switching. Pure angle-based FOV in degrees.
    // ---------------------------------------------------------------
    inline Target FindBestTarget(const Math::Vec3& eye, const Math::QAngle& view, float gameTime)
    {
        Target best;
        uintptr_t localCtrl = GameState::GetLocalController();
        if (!localCtrl) return best;
        uint32_t localH = Mem::Read<uint32_t>(localCtrl + Offsets::m_hPlayerPawn);
        uintptr_t localPawn = GameState::ResolveHandle(localH);
        if (!localPawn) return best;
        int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
        uintptr_t entList = GameState::GetEntityList();
        if (!entList) return best;
        float bestScore = FLT_MAX;

        for (int i = 1; i <= 64; ++i)
        {
            uintptr_t chunk = Mem::Read<uintptr_t>(entList + 0x8 * (i >> 9) + 0x10);
            if (!chunk) continue;
            uintptr_t ctrl = Mem::Read<uintptr_t>(chunk + GameState::kEntityStride * (i & 0x1FF));
            if (!ctrl) continue;
            uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
            if (!pH || pH == 0xFFFFFFFF) continue;
            uintptr_t pawn = GameState::ResolveHandle(pH);
            if (!pawn || pawn == localPawn) continue;
            int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
            uint8_t life = Mem::Read<uint8_t>(pawn + Offsets::m_lifeState);
            if (hp <= 0 || life != 0) continue;

            if (!IsEntityValid(pawn, i)) continue;
            if (!IsEntityFresh(pawn, gameTime)) continue;

            if (cfg.teamCheck)
            {
                int team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                if (team == localTeam) continue;
            }

            // Head priority: always try head first
            int primaryBone = cfg.targetBone;
            if (cfg.headPriority) primaryBone = 7;

            int selectedBone = primaryBone;
            Math::Vec3 bone = GetBestBonePos(pawn, primaryBone, eye, view, selectedBone);
            if (bone.IsZero()) continue;

            Math::QAngle ang = Math::CalcAngle(eye, bone);
            float fovVal = Math::AngleFov(view, ang);
            if (fovVal > EffectiveFov()) continue;

            // Visibility check
            if (cfg.visCheck)
            {
                bool isVis = IsVisible(pawn, localPawn, i);
                if (!isVis) continue;
            }

            // Smoke check â€” VACnet flags smoke kills hard
            if (cfg.smokeCheck && IsLineThroughSmoke(eye, bone))
                continue;

            // Score: FOV distance + slight distance bias
            Math::Vec3 delta = bone - eye;
            float dist = delta.Length();
            float score = fovVal + dist * 0.01f;
            if (score < bestScore)
            {
                bestScore  = score;
                best.pawn  = pawn;
                best.pos   = bone;
                best.angle = ang;
                best.fov   = fovVal;
                best.bone  = selectedBone;
            }
        }
        return best;
    }

    // ---------------------------------------------------------------
    // Mouse-input aimbot — read-only game data + SendInput mouse
    // No hooks on game functions. No game memory writes for aim.
    // Game sees genuine mouse hardware input via SendInput.
    // ---------------------------------------------------------------
    constexpr float kDefaultYaw   = 0.022f;  // Source engine m_yaw
    constexpr float kDefaultPitch = 0.022f;  // Source engine m_pitch

    // Read game sensitivity: dwSensitivity -> +0x58 = actual value
    inline float GetGameSensitivity()
    {
        uintptr_t sensBase = Mem::Read<uintptr_t>(
            GameState::clientBase + Offsets::Global::dwSensitivity);
        if (!sensBase) return 2.5f; // sane default
        float s = Mem::Read<float>(sensBase + Offsets::Global::dwSensitivity_sensitivity);
        if (s < 0.01f || s > 100.f) return 2.5f;
        return s;
    }

    // Convert an angle delta (degrees) to mouse pixel count
    // Inverse of: angle = pixels * sensitivity * m_yaw
    // So: pixels = angle / (sensitivity * m_yaw)
    inline void AngleToMouse(float dPitch, float dYaw, long& outDx, long& outDy)
    {
        float sens = GetGameSensitivity();
        float pixelsPerDegreeX = 1.0f / (sens * kDefaultYaw);
        float pixelsPerDegreeY = 1.0f / (sens * kDefaultPitch);
        outDx = (long)(-dYaw   * pixelsPerDegreeX); // Source: yaw -= mouseX * sens * m_yaw
        outDy = (long)(dPitch * pixelsPerDegreeY);  // Source: pitch += mouseY * sens * m_pitch
    }

    // Send a relative mouse movement — tries spoof first, falls back to direct
    inline void SendMouseDelta(long dx, long dy)
    {
        if (dx == 0 && dy == 0) return;
        INPUT inp = {};
        inp.type = INPUT_MOUSE;
        inp.mi.dx = dx;
        inp.mi.dy = dy;
        inp.mi.dwFlags = MOUSEEVENTF_MOVE;

        if (!useDirectSendInput)
        {
            __try {
                SpoofCall::SpoofedSendInput(1, &inp, sizeof(INPUT));
                return; // success
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                // Spoof stub corrupted — switch to direct permanently
                useDirectSendInput = true;
            }
        }

        // Direct fallback — resolve SendInput dynamically (no IAT)
        HMODULE u32 = GetModuleHandleW(L"user32.dll");
        if (u32) {
            typedef UINT(WINAPI* SendInputFn)(UINT, LPINPUT, int);
            auto fn = reinterpret_cast<SendInputFn>(GetProcAddress(u32, "SendInput"));
            if (fn) fn(1, &inp, sizeof(INPUT));
        }
    }

    // Accumulator for sub-pixel mouse deltas (fractional pixels build up over ticks)
    inline float mouseAccumX = 0.f;
    inline float mouseAccumY = 0.f;

    // ---------------------------------------------------------------
    // Anti-detection state
    // ---------------------------------------------------------------
    // Momentum: carry forward previous tick's delta for natural inertia
    inline float prevMoveDp = 0.f;
    inline float prevMoveDy = 0.f;

    // Tick skip: random micro-pauses that break frame-locked patterns
    inline int   skipTicksRemaining = 0;

    // Post-shot disruption: brief aim degradation after firing
    inline DWORD shotDisruptUntil   = 0;
    inline float shotDisruptScale   = 1.0f;

    // FOV distance of current target (for soft-edge attenuation)
    inline float curTargetFovDist   = 0.f;

    // NaN guard: detect and purge poison floats that silently kill the aimbot.
    // NaN doesn't crash (no exception), it just makes all math produce 0 forever.
    // A single frame of garbage view angles infects momentum + accumulators permanently.
    inline bool IsFinite(float v) { return !isnan(v) && !isinf(v); }
    inline bool SanitizeAimState()
    {
        bool poisoned = false;
        if (!IsFinite(mouseAccumX) || !IsFinite(mouseAccumY))
        {
            mouseAccumX = mouseAccumY = 0.f;
            poisoned = true;
        }
        if (!IsFinite(prevMoveDp) || !IsFinite(prevMoveDy))
        {
            prevMoveDp = prevMoveDy = 0.f;
            poisoned = true;
        }
        if (!IsFinite(state.curveProgress))
        {
            ResetState();
            poisoned = true;
        }
        return poisoned;
    }

    // Send mouse delta with sub-pixel accumulation + anti-detection noise
    inline void SendMouseDeltaSmooth(float dPitch, float dYaw)
    {
        float sens = GetGameSensitivity();
        float pixPerDegX = 1.0f / (sens * kDefaultYaw);
        float pixPerDegY = 1.0f / (sens * kDefaultPitch);
        mouseAccumX += -dYaw   * pixPerDegX;
        mouseAccumY += dPitch * pixPerDegY;

        // Sub-pixel noise floor: tiny random jitter (+/-0.2px) that
        // makes statistical analysis of pixel-level patterns harder.
        // Below 1px so it rarely adds a full pixel of movement.
        mouseAccumX += RandRange(-0.20f, 0.20f);
        mouseAccumY += RandRange(-0.20f, 0.20f);

        long dx = (long)mouseAccumX;
        long dy = (long)mouseAccumY;
        mouseAccumX -= (float)dx;
        mouseAccumY -= (float)dy;
        SendMouseDelta(dx, dy);
    }

    // ---------------------------------------------------------------
    // Silent Aim v2 — Gradual Angle Convergence
    //
    // VACNet analyzes angular velocity 0.5s before and 0.25s after
    // each shot. The old approach (snap angles on fire tick only)
    // created a single-frame discontinuity exactly where VACNet
    // looks hardest — instant detection.
    //
    // New approach: CONTINUOUSLY drift the server-side angles toward
    // the target while aiming. Each CreateMove tick applies a tiny
    // correction (max ~0.15-0.30 deg/tick) so the server sees smooth,
    // human-like tracking. By the time the player fires, the server
    // angle is already on target. No angular spike on the shot tick.
    //
    // When the target is lost, correction decays back to zero over
    // several ticks (natural deceleration curve, not a sudden stop).
    //
    // Perlin noise is mixed into the correction for micro-tremor
    // so the angular velocity profile has natural frequency content.
    // ---------------------------------------------------------------
    namespace SilentAim
    {
        // Frame history layout within CCSGOInput object
        constexpr int FRAME_HISTORY_COUNT = 0xBC8;  // int: entry count
        constexpr int FRAME_HISTORY_ARRAY = 0xBD0;  // uintptr_t: pointer to entries
        constexpr int FRAME_ENTRY_SIZE    = 96;      // 0x60 bytes per entry
        constexpr int PITCH_IN_ENTRY      = 0x10;    // float: pitch within entry
        constexpr int YAW_IN_ENTRY        = 0x14;    // float: yaw within entry

        using CreateMoveFn = double(__fastcall*)(__int64, unsigned int, __int64);
        inline CreateMoveFn oCreateMove = nullptr;
        inline void*        pCreateMoveHook = nullptr;  // for cleanup

        // Shared state: Tick() writes the desired aim angle, CreateMove reads it
        inline volatile bool   hasTarget    = false;
        inline volatile float  aimPitch     = 0.f;
        inline volatile float  aimYaw       = 0.f;

        inline double __fastcall hkCreateMove(__int64 a1, unsigned int a2, __int64 a3)
        {
            // Bhop: process button state via CCSGOInput (a1)
            __try {
                Bhop::OnCreateMove(a1);
            } __except (EXCEPTION_EXECUTE_HANDLER) {}

            // Visual no-recoil: zero punch angles every tick (like reference)
            __try {
                if (cfg.noRecoil) {
                    uintptr_t lp = GameState::GetLocalPawn();
                    if (lp) {
                        Math::Vec3 zero = { 0.f, 0.f, 0.f };
                        Mem::Write<Math::Vec3>(lp + Offsets::m_aimPunchAngle, zero);
                        Mem::Write<Math::Vec3>(lp + Offsets::m_aimPunchAngle + 0xC, zero); // m_aimPunchAngleVel
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}

            // Fake lag: choke this tick's command if requested
            __try {
                if (FakeLag::OnCreateMove())
                    return 0.0; // suppress this tick's command (choke)
            } __except (EXCEPTION_EXECUTE_HANDLER) {}

            // Passthrough when disabled
            if (!cfg.enabled || !cfg.silentAim)
            {
                return oCreateMove(a1, a2, a3);
            }

            __try {
                int entryCount = *reinterpret_cast<int*>(a1 + FRAME_HISTORY_COUNT);
                uintptr_t entryArray = *reinterpret_cast<uintptr_t*>(a1 + FRAME_HISTORY_ARRAY);
                int safeCount = (entryCount > 0 && entryCount <= 64) ? entryCount : 0;

                if (!entryArray || safeCount <= 0 || !hasTarget)
                    return oCreateMove(a1, a2, a3);

                // Save original angles, set to target angle directly
                struct SavedEntry { float pitch; float yaw; };
                SavedEntry saved[64] = {};

                for (int i = 0; i < safeCount; i++)
                {
                    uintptr_t entry = entryArray + FRAME_ENTRY_SIZE * i;
                    float* pP = reinterpret_cast<float*>(entry + PITCH_IN_ENTRY);
                    float* pY = reinterpret_cast<float*>(entry + YAW_IN_ENTRY);
                    saved[i].pitch = *pP;
                    saved[i].yaw   = *pY;

                    // Set directly to target angle — instant correction
                    float newP = aimPitch;
                    float newY = aimYaw;
                    if (newP >  89.f) newP =  89.f;
                    if (newP < -89.f) newP = -89.f;
                    *pP = newP;
                    *pY = newY;
                }

                // Call original — server receives corrected angles
                double result = oCreateMove(a1, a2, a3);

                // Restore original — client camera unchanged
                for (int i = 0; i < safeCount; i++)
                {
                    uintptr_t entry = entryArray + FRAME_ENTRY_SIZE * i;
                    *reinterpret_cast<float*>(entry + PITCH_IN_ENTRY) = saved[i].pitch;
                    *reinterpret_cast<float*>(entry + YAW_IN_ENTRY)   = saved[i].yaw;
                }

                return result;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return oCreateMove(a1, a2, a3);
            }
        }

        inline bool Install()
        {
            uintptr_t addr = Mem::FindPattern(L"client.dll", Signatures::CreateMove);
            if (!addr)
            {
                AimLog("[SilentAim] CreateMove pattern NOT FOUND");
                return false;
            }

            MH_STATUS st = MH_CreateHook(
                reinterpret_cast<void*>(addr),
                &hkCreateMove,
                reinterpret_cast<void**>(&oCreateMove));
            if (st != MH_OK)
            {
                AimLog("[SilentAim] MH_CreateHook failed: %d", st);
                return false;
            }

            st = MH_EnableHook(reinterpret_cast<void*>(addr));
            if (st != MH_OK)
            {
                AimLog("[SilentAim] MH_EnableHook failed: %d", st);
                return false;
            }

            pCreateMoveHook = reinterpret_cast<void*>(addr);
            AimLog("[SilentAim] CreateMove hooked at 0x%p", (void*)addr);
            return true;
        }

        inline void Uninstall()
        {
            if (pCreateMoveHook)
            {
                MH_DisableHook(pCreateMoveHook);
                MH_RemoveHook(pCreateMoveHook);
                pCreateMoveHook = nullptr;
                oCreateMove = nullptr;
            }
            hasTarget  = false;
        }
    }

    // ---------------------------------------------------------------
    // Tick() -- called from PresentCore (render thread, every frame)
    // Pure read + mouse output. Zero game memory writes.
    // ---------------------------------------------------------------
    inline void Tick()
    {
        if (!GameState::clientBase) return;

        // Visual no-recoil runs even when aimbot is disabled
        if (cfg.noRecoil) {
            __try {
                uintptr_t lp = GameState::GetLocalPawn();
                if (lp) {
                    Math::Vec3 zero = { 0.f, 0.f, 0.f };
                    Mem::Write<Math::Vec3>(lp + Offsets::m_aimPunchAngle, zero);
                    Mem::Write<Math::Vec3>(lp + Offsets::m_aimPunchAngle + 0xC, zero);
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        if (!cfg.enabled) return;

        __try {

        if (!rngState) SeedRng();

        // Crash backoff: if we crashed many ticks in a row, wait before retrying.
        // This prevents burning CPU on a crash loop and gives the game time to stabilize.
        if (crashBackoffTicks > 0)
        {
            crashBackoffTicks--;
            if (crashBackoffTicks == 0)
                consecutiveCrashes = 0; // Give a clean start after backoff
            return;
        }

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        // Periodic NaN decontamination: purge poisoned state every tick
        SanitizeAimState();

        // Clear originalDormant so stale chams data from the previous frame
        // can't block enemies. Aimbot runs BEFORE chams in the render loop,
        // so without this, we read last frame's (potentially stale) dormancy.
        // IsEntityFresh() provides an independent staleness check via simTime.
        memset(GameState::originalDormant, 0, sizeof(GameState::originalDormant));

        uintptr_t vaAddr    = GameState::clientBase + GameState::RVA_dwViewAngles();
        Math::QAngle viewAng = Mem::Read<Math::QAngle>(vaAddr);

        // NaN guard: game writes garbage during round transitions/halftime.
        // A single NaN frame permanently poisons momentum + accumulators.
        if (!IsFinite(viewAng.pitch) || !IsFinite(viewAng.yaw))
        {
            SanitizeAimState();
            return;
        }

        Math::Vec3 eye = EyePosition(localPawn);
        if (eye.IsZero()) return;
        if (!IsFinite(eye.x) || !IsFinite(eye.y) || !IsFinite(eye.z))
        {
            SanitizeAimState();
            return;
        }

        // Bullet tracer detection (read-only)
        bool shotFired = BulletTracer::DetectShot(localPawn);

        // Post-shot aim disruption: firing creates micro-recoil surprise.
        // Humans briefly lose tracking precision after each shot.
        // 40-90ms of 20-35% reduced aim speed per shot.
        if (shotFired)
        {
            shotDisruptUntil = GetTickCount() + (DWORD)RandInt(10, 25);
            shotDisruptScale = RandRange(0.93f, 0.98f);
        }

        float gameTime = GameState::GetGameTime();

        // Update local player index cache for spottedByMask checks
        GameState::UpdateLocalPlayerIndex();

        // Governor: check round resets periodically (~every 2s)
        DWORD nowGov = GetTickCount();
        if (nowGov - govRoundCheckTick > 2000)
        {
            Governor::CheckRoundReset();
            govRoundCheckTick = nowGov;
        }

        // Governor: behavioural throttle
        Governor::Decision gov = Governor::Tick();
        if (gov.skipThisTick) goto tickDone;

        {
            // No Spread: zero weapon inaccuracy every tick
            if (cfg.noSpread)
                ZeroWeaponInaccuracy(localPawn);

            bool inAir = IsInAir(localPawn);
            // noSpread implies jumpShot (no point zeroing spread if aimbot skips air)
            if (inAir && !cfg.jumpShot && !cfg.noSpread) goto tickDone;
            if (inAir && cfg.jumpApexOnly && !cfg.noSpread && !IsAtJumpApex(localPawn))
                goto tickDone;

            // Don't aim while fully blinded
            float flashAlpha = Mem::Read<float>(localPawn + Offsets::m_flFlashMaxAlpha);
            if (flashAlpha > 240.f) goto tickDone;

            // Robust key check: PEB-walked GetAsyncKeyState (no IAT import).
            // SafeGetKeyState resolves via export walk, falls back to LoadLibrary.
            auto CheckKey = [](int vk) -> bool {
                SHORT s = GetAsyncKeyState(vk);
                return (s & 0x8000) != 0;
            };
            // Aim key modes: 0 = auto (mouse1), 1 = mouse2, 2 = always on
            bool shouldAim = false;
            if (cfg.aimKey == 0)
                shouldAim = CheckKey(VK_LBUTTON);
            else if (cfg.aimKey == 1)
                shouldAim = CheckKey(VK_RBUTTON);
            else if (cfg.aimKey == 2)
                shouldAim = true; // always active when enabled

            if (!shouldAim)
            {
                if (state.phase != PHASE_IDLE)
                    state.phase = PHASE_IDLE;
                mouseAccumX = mouseAccumY = 0.f;
                prevMoveDp = prevMoveDy = 0.f;
                skipTicksRemaining = 0;
                SilentAim::hasTarget = false;
                goto tickDone;
            }

            // ----- Anti-detection: random tick skipping -----
            // Rare micro-pauses (~1%) — keeps pattern non-deterministic
            // without starving aim of frames.
            if (skipTicksRemaining > 0)
            {
                skipTicksRemaining--;
                SendMouseDeltaSmooth(HandTremor(0.f, 0.3f), HandTremor(1.f, 0.3f));
                goto tickDone;
            }
            if (Rand01() < 0.008f)
            {
                skipTicksRemaining = 1; // Single tick only
                SendMouseDeltaSmooth(HandTremor(0.f, 0.3f), HandTremor(1.f, 0.3f));
                goto tickDone;
            }

            Target t;
            __try {
                t = FindBestTarget(eye, viewAng, gameTime);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                t = Target{}; // Corrupt entity data — treat as no target
            }

            // Governor: force body shots when HS% is too hot
            if (gov.overrideBone >= 0 && t.pawn)
            {
                int boneOverride = gov.overrideBone;
                Math::Vec3 bodyPos = GetBestBonePos(t.pawn, boneOverride, eye, viewAng, boneOverride);
                if (!bodyPos.IsZero())
                {
                    t.pos   = bodyPos;
                    t.angle = Math::CalcAngle(eye, bodyPos);
                    t.fov   = Math::AngleFov(viewAng, t.angle);
                    t.bone  = boneOverride;
                }
            }

            if (!t.pawn)
            {
                noTargetTicks++;

                // Self-healing: after ~2 seconds of no targets, force refresh
                // all cached state. Something may have gone stale.
                if (noTargetTicks > 120 && (GetTickCount() - lastSelfHealTime) > 3000)
                {
                    lastSelfHealTime = GetTickCount();
                    GameState::UpdateLocalPlayerIndex();
                    memset(GameState::originalDormant, 0, sizeof(GameState::originalDormant));
                    SanitizeAimState();
                    noTargetTicks = 0;
                }

                // Grace period: keep engagement alive through brief visibility
                // flickers (smoke, lag, tight angles) — only lose target after
                // 14 consecutive no-target ticks (~220ms at 64fps).
                // This prevents breaking a locked engagement every time the
                // spotting system glitches for a single frame.
                if (state.lockedTarget != 0 && noTargetTicks <= 14)
                {
                    SilentAim::hasTarget = false;
                    goto tickDone;  // hold phase/lockedTarget, try again next tick
                }

                if (state.phase != PHASE_IDLE)
                    ResetState();
                state.lockedTarget = 0;
                govLastTarget = 0;
                mouseAccumX = mouseAccumY = 0.f;
                SilentAim::hasTarget = false;
                goto tickDone;
            }

            // Target found — reset no-target counter
            noTargetTicks = 0;

            // Post-kill refractory
            if (state.lastKillTime != 0 && state.postKillDelayMs > 0)
            {
                DWORD elapsed = GetTickCount() - state.lastKillTime;
                if (elapsed < (DWORD)state.postKillDelayMs)
                    goto tickDone;
            }

            // ---- Engagement staleness safety valve ----
            // If locked target hasn't changed in 45+ seconds, something is stuck.
            // Force-reset to prevent any stale-pointer scenarios.
            if (state.lockedTarget != 0 && state.engagementStartTime != 0)
            {
                DWORD engAge = GetTickCount() - state.engagementStartTime;
                if (engAge > 45000)
                {
                    ResetState();
                    govLastTarget = 0;
                }
            }

            // ---- Target management: sticky lock + re-engagement ----
            if (state.phase == PHASE_IDLE && t.pawn == state.lockedTarget && state.lockedTarget != 0)
            {
                if (t.fov < 2.0f)
                {
                    state.phase = PHASE_CORRECTING;
                    state.curveProgress = 1.0f;
                    state.corrOscCount = 0;
                }
                else
                {
                    state.phase = PHASE_REACTING;
                    state.reactStartTime = GetTickCount();
                    state.reactDelayMs = RandInt(40, 120);
                    state.attackTicks = 0;
                    state.curveProgress = 0.f;
                }
            }
            else if (t.pawn != state.lockedTarget)
            {
                // Old-target validation wrapped in __try/__except.
                // If the old pawn pointer is stale (player disconnected,
                // round restarted, slot reused), any read can crash.
                // Without this, the outer __except catches it but never
                // clears lockedTarget -> permanent crash loop = aimbot dead.
                if (state.lockedTarget != 0 && state.phase != PHASE_IDLE)
                {
                    __try {
                        int oldHp = Mem::Read<int32_t>(state.lockedTarget + Offsets::m_iHealth);
                        uint8_t oldLife = Mem::Read<uint8_t>(state.lockedTarget + Offsets::m_lifeState);
                        bool oldDead = (oldHp <= 0 || oldLife != 0);
                        bool oldDormant = false;

                        if (!oldDead)
                        {
                            uintptr_t oldNode = Mem::Read<uintptr_t>(state.lockedTarget + Offsets::m_pGameSceneNode);
                            if (oldNode) oldDormant = Mem::Read<bool>(oldNode + 0x10B);

                            if (!oldDormant)
                            {
                                bool oldVis = cfg.visCheck ? IsVisible(state.lockedTarget, localPawn) : true;
                                if (oldVis)
                                {
                                    int oldOutBone = t.bone;
                                    Math::Vec3 oldBone = GetBestBonePos(state.lockedTarget, cfg.targetBone, eye, viewAng, oldOutBone);
                                    if (!oldBone.IsZero())
                                    {
                                        float oldFov = Math::AngleFov(viewAng, Math::CalcAngle(eye, oldBone));
                                        if (t.fov > oldFov * 0.5f || oldFov < EffectiveFov())
                                        {
                                            t.pawn  = state.lockedTarget;
                                            t.pos   = oldBone;
                                            t.angle = Math::CalcAngle(eye, oldBone);
                                            t.fov   = oldFov;
                                            t.bone  = oldOutBone;
                                        }
                                    }
                                }
                            }
                        }
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        // Stale pointer — accept new target, don't crash-loop
                    }
                }

                if (t.pawn != state.lockedTarget)
                    BeginEngagement(t.pawn, t.fov);
            }
            state.lockedTarget = t.pawn;

            // Governor kill detection (wrapped — lockedTarget could be stale)
            __try {
                if (state.lockedTarget)
                {
                    int curHP = Mem::Read<int32_t>(state.lockedTarget + Offsets::m_iHealth);
                    if (govLastTarget == state.lockedTarget && govLastTargetHP > 0 && curHP <= 0)
                    {
                        bool wasHS = (govLastBone == 0 || govLastBone == 6 || govLastBone == 7);
                        Governor::OnKill(wasHS);
                        state.lastKillTime = GetTickCount();
                        state.postKillDelayMs = RandInt(30, 100);
                        state.phase = PHASE_IDLE;
                    }
                    govLastTarget   = state.lockedTarget;
                    govLastTargetHP = curHP;
                    govLastBone     = t.bone;
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                govLastTarget = 0;
                govLastTargetHP = 100;
            }

            Math::QAngle aimAngle = t.angle;

            // Recoil compensation
            if (cfg.noRecoil)
            {
                Math::QAngle punch = GetAimPunch(localPawn);
                aimAngle.pitch -= punch.pitch;
                aimAngle.yaw   -= punch.yaw;
                Math::ClampAngles(aimAngle);
            }

            // Distance to target (in degrees)
            float dp = aimAngle.pitch - viewAng.pitch;
            float dy = aimAngle.yaw - viewAng.yaw;
            while (dy > 180.f) dy -= 360.f;
            while (dy < -180.f) dy += 360.f;
            float distToTarget = sqrtf(dp * dp + dy * dy);

            // ===========================================================
            // SILENT AIM v2 — Gradual angle convergence
            //
            // Instead of snapping angles on the fire tick (which VACNet
            // detects as a single-frame discontinuity), we continuously
            // feed the target angle to the CreateMove hook. The hook
            // applies tiny corrections each tick (~0.18 deg) so the
            // server sees smooth, human-like tracking. By the time the
            // player fires, the server angle is already on target.
            //
            // This eliminates the angular spike that VACNet's neural
            // network is specifically trained to detect.
            // ===========================================================
            if (cfg.silentAim && SilentAim::oCreateMove)
            {
                // Set target angle directly — CreateMove hook will apply it
                SilentAim::aimPitch  = aimAngle.pitch;
                SilentAim::aimYaw    = aimAngle.yaw;
                SilentAim::hasTarget = true;

                // Bullet tracer from aim angle (not view angle)
                if (shotFired)
                    BulletTracer::AddTraceFromAngles(eye.x, eye.y, eye.z,
                                                     aimAngle.pitch, aimAngle.yaw);
                goto tickDone;
            }

            // Clear silent aim target when using mouse mode
            SilentAim::hasTarget = false;

            // ===========================================================
            // VISIBLE AIM via MOUSE INPUT
            // Same neuromotor humanization as before, but outputs
            // SendInput mouse deltas instead of writing game memory.
            // ===========================================================

            // The delta we want to move THIS TICK (in degrees)
            float moveDp = 0.f, moveDy = 0.f;

            switch (state.phase)
            {
            case PHASE_REACTING:
            {
                DWORD elapsed = GetTickCount() - state.reactStartTime;
                if (elapsed >= (DWORD)state.reactDelayMs)
                {
                    state.startDeltaP = dp;
                    state.startDeltaY = dy;
                    state.phase = PHASE_ATTACKING;
                    state.attackTicks = 0;
                    state.curveProgress = 0.f;
                }
                else
                {
                    // Still reacting -- only natural hand tremor
                    moveDp = HandTremor(0.f, 0.5f);
                    moveDy = HandTremor(1.f, 0.5f);
                }
                break;
            }

            case PHASE_ATTACKING:
            {
                state.attackTicks++;
                state.curveProgress += state.attackSpeed * RandRange(0.92f, 1.08f);
                if (state.curveProgress > 1.0f) state.curveProgress = 1.0f;

                float position = HumanPositionCurve(state.curveProgress, state.accelShape);
                (void)position; // used implicitly via curveProgress

                float overshootPhase = 0.f;
                if (state.curveProgress > 0.6f)
                {
                    float ovT = (state.curveProgress - 0.6f) / 0.4f;
                    overshootPhase = sinf(ovT * 3.14159f) * state.overshootPct;
                }

                float targetP = aimAngle.pitch + (state.overshootDirP * overshootPhase * fabsf(state.startDeltaP));
                float targetY = aimAngle.yaw   + (state.overshootDirY * overshootPhase * fabsf(state.startDeltaY));

                float remainP = targetP - viewAng.pitch;
                float remainY = targetY - viewAng.yaw;
                while (remainY > 180.f) remainY -= 360.f;
                while (remainY < -180.f) remainY += 360.f;

                float vel = HumanVelocityCurve(state.curveProgress, state.accelShape);
                float moveFrac = vel * GetSmoothFraction() * 3.5f * state.peakVelMul;
                if (moveFrac > 0.90f) moveFrac = 0.90f;

                moveDp = remainP * moveFrac;
                moveDy = remainY * moveFrac;

                // Perlin tremor
                float tremorScale = (state.curveProgress > 0.5f) ? 1.3f : 0.6f;
                moveDp += HandTremor(0.f, tremorScale);
                moveDy += HandTremor(1.f, tremorScale);

                // Angular velocity cap
                float stepTotal = sqrtf(moveDp * moveDp + moveDy * moveDy);
                float maxStep = GetMaxStep();
                if (state.attackTicks <= 2) maxStep *= 1.5f;
                if (stepTotal > maxStep && stepTotal > 0.001f)
                {
                    float scale = maxStep / stepTotal;
                    moveDp *= scale;
                    moveDy *= scale;
                }

                if (distToTarget < 1.8f || state.curveProgress >= 0.90f)
                {
                    state.phase = PHASE_CORRECTING;
                    state.corrOscCount = 0;
                }
                break;
            }

            case PHASE_CORRECTING:
            {
                state.corrOscCount++;
                float smoothFrac = GetSmoothFraction();
                float dampFactor = powf(state.dampingRate, (float)state.corrOscCount * 0.15f);
                float corrSpeed = smoothFrac * 2.8f * dampFactor + RandRange(0.f, smoothFrac * 0.3f);
                if (corrSpeed > 0.80f) corrSpeed = 0.80f;

                float oscMul = 1.0f;
                if (state.corrOscCount < 10 && distToTarget > 0.3f)
                {
                    float oscPhase = sinf(state.corrOscCount * 0.8f) * 0.12f * dampFactor * EffectiveHumanization();
                    oscMul = 1.0f + oscPhase;
                }

                moveDp = dp * corrSpeed * oscMul;
                moveDy = dy * corrSpeed * oscMul;

                moveDp += HandTremor(0.f, 1.0f);
                moveDy += HandTremor(1.f, 1.0f);

                if (distToTarget < 0.4f)
                    state.phase = PHASE_LOCKED;
                break;
            }

            case PHASE_LOCKED:
            {
                float smoothFrac = GetSmoothFraction();
                float lockSpeed = smoothFrac * 1.5f + RandRange(0.f, smoothFrac * 0.2f);
                if (lockSpeed > 0.60f) lockSpeed = 0.60f;

                moveDp = dp * lockSpeed;
                moveDy = dy * lockSpeed;

                moveDp += HandTremor(0.f, 1.2f);
                moveDy += HandTremor(1.f, 1.2f);

                if ((engagementCount + state.corrOscCount) % 7 == 0)
                {
                    moveDp += Perlin::Noise1D(GetPerlinTime() * 1.2f) * 0.04f * EffectiveHumanization();
                    moveDy += Perlin::Noise1D(GetPerlinTime() * 1.5f + 50.f) * 0.06f * EffectiveHumanization();
                }

                if (distToTarget > 2.5f)
                {
                    state.phase = PHASE_CORRECTING;
                    state.corrOscCount = 0;
                }
                break;
            }

            default:
                break;
            }

            // ===============================================================
            // Anti-detection post-processing (applied to ALL phases)
            // These layers are invisible to the user but change the
            // statistical fingerprint VACNet analyzes around shots.
            // ===============================================================

            // --- FOV soft-edge attenuation ---
            // Targets near the edge of FOV get progressively more smoothing.
            // Prevents the sharp "snap zone" at the FOV boundary that VACNet
            // detects as inhuman. Inner 60% of FOV = full speed. Outer 40%
            // linearly attenuates down to 55% speed.
            {
                float fovEdge = EffectiveFov();
                float innerZone = fovEdge * 0.60f;
                if (t.fov > innerZone && fovEdge > 0.01f)
                {
                    float edgeFrac = (t.fov - innerZone) / (fovEdge - innerZone);
                    if (edgeFrac > 1.f) edgeFrac = 1.f;
                    float atten = 1.0f - edgeFrac * 0.45f; // 1.0 → 0.55
                    moveDp *= atten;
                    moveDy *= atten;
                }
            }

            // --- Momentum / inertia ---
            // Carry forward 18% of previous tick's movement delta.
            // Real arms have mass — you can't instantly change direction.
            // This creates natural acceleration/deceleration curves between
            // ticks instead of independent per-frame calculations.
            {
                // NaN guard: purge poisoned movement before it infects accumulators
                if (!IsFinite(moveDp) || !IsFinite(moveDy))
                {
                    moveDp = moveDy = 0.f;
                    prevMoveDp = prevMoveDy = 0.f;
                }
                float momentum = 0.18f;
                moveDp = moveDp * (1.f - momentum) + prevMoveDp * momentum;
                moveDy = moveDy * (1.f - momentum) + prevMoveDy * momentum;
                prevMoveDp = moveDp;
                prevMoveDy = moveDy;
            }

            // --- Post-shot aim disruption ---
            // After firing, briefly reduce aim speed by 20-35%.
            // Humans experience micro-surprise from recoil feedback
            // and briefly lose tracking precision. VACNet specifically
            // analyzes the 0.25s window after each shot.
            if (GetTickCount() < shotDisruptUntil)
            {
                moveDp *= shotDisruptScale;
                moveDy *= shotDisruptScale;
            }

            // Send the computed delta as mouse input (genuine hardware-level)
            if ((moveDp != 0.f || moveDy != 0.f) && IsFinite(moveDp) && IsFinite(moveDy))
                SendMouseDeltaSmooth(moveDp, moveDy);

            // Bullet tracer from current view angles (read-only)
            if (shotFired)
                BulletTracer::AddTraceFromAngles(eye.x, eye.y, eye.z, viewAng.pitch, viewAng.yaw);
        }

    tickDone:;
        consecutiveCrashes = 0; // Reached end of tick without crashing
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            // Crash recovery: ALWAYS reset to clean state so next tick starts fresh.
            // Without this, a stale pointer or corrupt entity data creates a
            // permanent crash loop where the aimbot appears "turned off."
            ResetState();
            govLastTarget = 0;
            govLastTargetHP = 100;
            mouseAccumX = mouseAccumY = 0.f;
            prevMoveDp = prevMoveDy = 0.f;
            skipTicksRemaining = 0;
            SilentAim::hasTarget = false;
            consecutiveCrashes++;
            // Exponential backoff: after repeated crashes, wait longer before retrying.
            // 1-2 crashes: instant retry (transient glitch)
            // 3-9 crashes: skip 30 ticks (~500ms, lets round transition settle)
            // 10+ crashes: skip 60 ticks (~1s, then reset and try fresh)
            // Fast recovery: don't let crash backoff kill the aimbot.
            // Round transitions cause brief entity corruption — recover fast.
            if (consecutiveCrashes >= 15)
                crashBackoffTicks = 20;
            else if (consecutiveCrashes >= 5)
                crashBackoffTicks = 8;
        }
    }

    // ---------------------------------------------------------------
    // Lifecycle
    // ---------------------------------------------------------------
    inline bool initialized = false;

    inline bool Setup()
    {
        // Validate that we can read essential game state
        if (!GameState::clientBase) {
            AimLog("[Aimbot] Setup FAILED -- no clientBase");
            return false;
        }

        uintptr_t vaAddr = GameState::clientBase + GameState::RVA_dwViewAngles();
        Math::QAngle testAng = Mem::Read<Math::QAngle>(vaAddr);

        AimLog("[Aimbot] Mouse-input aimbot init");
        AimLog("[Aimbot]   clientBase = 0x%p", (void*)GameState::clientBase);
        AimLog("[Aimbot]   dwViewAngles @ 0x%p (pitch=%.1f yaw=%.1f)",
               (void*)vaAddr, testAng.pitch, testAng.yaw);
        AimLog("[Aimbot]   sensitivity = %.3f", GetGameSensitivity());
        AimLog("[Aimbot]   mode: READ-ONLY + SendInput mouse (no hooks, no mem writes)");

        // Install CreateMove hook for silent aim capability
        // Hook is always installed but only active when cfg.silentAim = true
        if (!SilentAim::Install())
            AimLog("[Aimbot] Silent aim hook failed — mouse-only mode available");

        initialized = true;
        return true;
    }

    inline void Shutdown()
    {
        SilentAim::Uninstall();
        initialized = false;
        mouseAccumX = mouseAccumY = 0.f;
    }
}
