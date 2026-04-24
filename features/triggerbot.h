#pragma once

// ---------------------------------------------------------------
// Triggerbot — fires when crosshair rests on an enemy.
// Uses m_iIDEntIndex (server-authoritative entity under crosshair).
// Output via SendInput (same as aimbot — no memory writes).
// Humanized: random pre-fire delay + post-fire cooldown.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../core/stealth.h"
#include "../core/spoof_call.h"

namespace Triggerbot
{
    struct Config
    {
        bool  enabled      = false;
        int   key          = 0;       // 0 = always-on while aimbot active, else hold key
        bool  teamCheck    = true;
        bool  smokeCheck   = true;
        int   minDelayMs   = 30;      // human reaction floor
        int   maxDelayMs   = 120;     // human reaction ceiling
        int   burstMin     = 1;       // min shots per burst
        int   burstMax     = 3;       // max shots per burst
        bool  scopeOnly    = false;   // only fire when scoped (AWP/Scout)

        // ---- Seeded / accuracy-gated firing ----
        // Reads m_fAccuracyPenalty on the active weapon and only fires
        // when the spread cone is small enough that the bullet has a
        // high probability of landing on the target. Stops the trigger
        // from spamming through bloom and missing every shot — and from
        // the shot-pattern ML signature ("fires every tick regardless
        // of accuracy" is one of the strongest VAC heuristics).
        bool  accuracyGate    = true;
        float maxPenalty      = 0.45f;  // skip if m_fAccuracyPenalty above this
        // Don't fire while moving fast on the ground — CS2 inaccuracy
        // scales with horizontal speed and bullets just fly off.
        bool  speedGate       = true;
        float maxSpeed        = 110.f;  // u/s; ~walk speed limit
        // Jump-apex fire: when in the air, only fire near the apex of
        // the jump (vertical velocity near 0) where bullet inaccuracy
        // is at a local minimum. Replaces the old "fire whenever in air"
        // path which was just spraying into space.
        bool  jumpApexFire    = true;
        float jumpApexAbsVz   = 60.f;   // |vel.z| < this counts as "near apex"
    };

    inline Config cfg;

    // Internal state
    inline DWORD  triggerTime   = 0;   // when crosshair first touched enemy
    inline int    delayMs       = 0;   // randomized delay for this engagement
    inline int    burstLeft     = 0;   // shots remaining in current burst
    inline DWORD  lastShotTime  = 0;
    inline int    cooldownMs    = 0;

    // Simple RNG (reuses aimbot's pattern)
    inline uint32_t tRng = 0;
    inline uint32_t TRand()
    {
        if (!tRng)
        {
            LARGE_INTEGER pc;
            QueryPerformanceCounter(&pc);
            tRng = static_cast<uint32_t>(pc.QuadPart ^ (pc.QuadPart >> 17));
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

    inline void Tick()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {

        // Key check
        if (cfg.key != 0)
        {
            SHORT s = GetAsyncKeyState(cfg.key);
            if (!(s & 0x8000))
            {
                triggerTime = 0;
                return;
            }
        }

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        // Post-shot cooldown
        if (lastShotTime && cooldownMs > 0)
        {
            if (GetTickCount() - lastShotTime < (DWORD)cooldownMs)
                return;
            cooldownMs = 0;
        }

        // Scope check
        if (cfg.scopeOnly)
        {
            bool scoped = Mem::Read<bool>(localPawn + Offsets::m_bIsScoped);
            if (!scoped) { triggerTime = 0; return; }
        }

        // Read entity under crosshair (server-side, reliable)
        int entIdx = Mem::Read<int32_t>(localPawn + Offsets::m_iIDEntIndex);
        if (entIdx <= 0 || entIdx > 64)
        {
            triggerTime = 0;
            burstLeft = 0;
            return;
        }

        // Resolve entity
        uintptr_t ctrl = GameState::GetEntityByIndex(entIdx);
        if (!ctrl) return;

        uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
        if (!pH || pH == 0xFFFFFFFF) return;
        uintptr_t pawn = GameState::ResolveHandle(pH);
        if (!pawn) return;

        // Alive check
        int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
        if (hp <= 0) { triggerTime = 0; return; }

        // Team check
        if (cfg.teamCheck)
        {
            int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
            int enemyTeam = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
            if (localTeam == enemyTeam) { triggerTime = 0; return; }
        }

        // Smoke check
        if (cfg.smokeCheck)
        {
            float smokeAlpha = Mem::Read<float>(localPawn + Offsets::m_flLastSmokeOverlayAlpha);
            if (smokeAlpha > 0.5f) { triggerTime = 0; return; }
        }

        // ---- Speed / jump-apex gate (anti-detection + actual hit rate) ----
        // Use FL_ONGROUND from m_fFlags rather than guessing from vel.z
        // — vel.z bounces between -0.5 and +0.5 on slopes/stairs even
        // when the player is grounded, which made the old heuristic
        // mis-classify and skip legitimate shots.
        Math::Vec3 vel = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecVelocity);
        float horizSpeed = sqrtf(vel.x * vel.x + vel.y * vel.y);
        uint32_t flFlags = Mem::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        bool airborne = (flFlags & (1u << 0)) == 0;  // FL_ONGROUND = 1<<0

        if (airborne)
        {
            if (cfg.jumpApexFire)
            {
                // Only fire near the apex of the jump where spread is lowest.
                // CS2 inaccuracy_jump kicks in proportional to |vz| / 250.
                if (fabsf(vel.z) > cfg.jumpApexAbsVz) {
                    triggerTime = 0; return;
                }
            }
            else
            {
                // Don't fire while in air at all
                triggerTime = 0; return;
            }
        }
        else if (cfg.speedGate)
        {
            // On ground: walk speed only.
            if (horizSpeed > cfg.maxSpeed) {
                triggerTime = 0; return;
            }
        }

        // ---- Accuracy-gated (a.k.a. seeded) trigger ----
        // Server uses m_fAccuracyPenalty for the bullet spread cone
        // and m_flTurningInaccuracy as a separate additive cone applied
        // when the view angles change quickly (snap-aim spike).  Both
        // must be small for the shot to land.  This pair is what the
        // community calls "seeded" trigger — we don't predict the
        // server-side RNG seed, we just refuse to fire until the
        // resolved cone width is small enough that the bullet is
        // virtually guaranteed to hit.  Net effect matches a seed
        // predictor without the detection signature of one.
        if (cfg.accuracyGate)
        {
            uintptr_t weapon = GameState::GetActiveWeapon(localPawn);
            if (weapon)
            {
                float penalty = Mem::Read<float>(weapon + Offsets::m_fAccuracyPenalty);
                float turning = Mem::Read<float>(weapon + Offsets::m_flTurningInaccuracy);
                // Combined cone proxy.  Empirically, sum > maxPenalty
                // means > 1° spread which is enough to miss a body box
                // at mid-range.
                if ((penalty + turning) > cfg.maxPenalty) {
                    return;
                }
            }
        }

        // First frame on target — start delay timer
        DWORD now = GetTickCount();
        if (triggerTime == 0)
        {
            triggerTime = now;
            delayMs = TRandInt(cfg.minDelayMs, cfg.maxDelayMs);
            burstLeft = TRandInt(cfg.burstMin, cfg.burstMax);
            return;
        }

        // Wait for humanized delay
        if (now - triggerTime < (DWORD)delayMs)
            return;

        // Fire
        if (burstLeft > 0)
        {
            SendClick();
            burstLeft--;
            lastShotTime = now;
            // Anti-detection: gaussian-ish intershot delay.
            // Uniform 40-100ms is a strong VAC signature ("inhuman
            // perfectly-bounded distribution"). Sum of two uniforms
            // gives a Bates(2) triangular distribution centered on
            // the midpoint, which closely matches human variance.
            int g1 = TRandInt(20, 60);
            int g2 = TRandInt(20, 60);
            cooldownMs = g1 + g2;

            if (burstLeft <= 0)
            {
                // Burst complete — reset for next engagement
                triggerTime = 0;
            }
        }

        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}
