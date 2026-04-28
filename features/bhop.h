#pragma once

// ---------------------------------------------------------------
// Bhop v26 — Full auto bhop + proper air strafe
//
// Fully automatic: just hold space and move your mouse.
// Auto-jump on ground contact, auto-strafe in air based on
// mouse movement direction.
//
// KEY INSIGHT: In Source engine, holding W (forward) in air kills
// air acceleration. Auto strafe MUST release forward and ONLY
// press A/D to gain speed. This is the fundamental mechanic.
//
// Values: 65537 = press (+cmd), 256 = release (-cmd)
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../core/signatures.h"
#include "../vendor/minhook/include/MinHook.h"

// Button command offsets are pulled from sdk/buttons.hpp via
// Offsets::Buttons (refreshed from live dumper, build 14154).
// Pre-fix: bhop hardcoded jump=0x2066C70, drift +0x18D40 from live
// 0x204DF30 — every press/release was hitting unrelated client.dll
// memory and bhop only "worked" because the user holds Space manually.
namespace ButtonOffsets {
    constexpr std::ptrdiff_t jump    = Offsets::Buttons::jump;
    constexpr std::ptrdiff_t left    = Offsets::Buttons::left;     // +moveleft
    constexpr std::ptrdiff_t right   = Offsets::Buttons::right;    // +moveright
    constexpr std::ptrdiff_t forward = Offsets::Buttons::forward;  // +forward
    constexpr std::ptrdiff_t back    = Offsets::Buttons::back;     // +back
    constexpr std::ptrdiff_t duck    = Offsets::Buttons::duck;     // +duck (for jumpshot crouch-air)
}

constexpr std::uint32_t FL_ONGROUND = (1 << 0);

namespace Bhop
{
    struct Config
    {
        bool  enabled       = false;
        int   key           = VK_SPACE;
        float maxSpeed      = 380.f;
        float minSpeed      = 30.f;
        bool  autoStrafe    = true;
        bool  showVelocity  = true;   // HUD speed display
        // Strafe mode: 0 = velocity (smoothest, perfect-strafe physics),
        //              1 = mouse-yaw (legacy, follows mouse turn delta)
        int   strafeMode    = 0;
    };

    inline Config cfg;

    // Internal state
    inline bool  wasOnGround     = true;
    inline int   hopCount        = 0;
    inline DWORD lastHopTime     = 0;
    inline float savedYaw        = 0.f;
    inline bool  yawInit         = false;
    inline bool  jumpToggle      = false;  // alternates jump press/release
    inline bool  inAutoStrafe    = false;  // true while we're controlling strafe keys in air

    // Last value we wrote to each button slot, so AutoStrafe can avoid
    // hammering the same press/release on every tick. Spamming the same
    // button command bytes 64+ times/sec produces visible micro-jitter
    // and increases our memory-write footprint for no gain.
    inline uint32_t lastBtnLeft    = 0;
    inline uint32_t lastBtnRight   = 0;
    inline uint32_t lastBtnForward = 0;
    inline uint32_t lastBtnBack    = 0;

    inline void WriteBtnIfChanged(uintptr_t base, std::ptrdiff_t off,
                                  uint32_t val, uint32_t& cache)
    {
        if (cache == val) return;
        Mem::Write<uint32_t>(base + off, val);
        cache = val;
    }

    // Movement services offsets — canonical, sourced from
    // dumps/latest/offsets/sdk/client_dll.hpp on build 14154.
    // PRE-14154 these were 0x518/0x528/0x530/0x2714 with the services
    // pointer at pawn+0x1418; writing those values now silently lands
    // on unrelated fields and bhop never resets stamina, killing the
    // strafe speed gain.
    using Offsets::m_pMovementServices_pawn;
    using Offsets::m_flStamina_movement;
    using Offsets::m_flStaminaAtJumpStart_movement;
    using Offsets::m_flAccumulatedJumpError_mov;
    using Offsets::m_flLastJumpVelocityZ_movement;
    using Offsets::m_flVelocityModifier_pawn;

    inline bool hookInstalled = false;

    // ---------------------------------------------------------------
    // Auto Strafe — proper Source engine air strafing.
    //
    // The mechanic: in air, RELEASE forward/back, then press A while
    // turning left of velocity OR D while turning right of velocity.
    // This generates the speed gain.
    //
    // We support TWO modes:
    //   strafeMode = 0 (default): velocity-based. Compute the angle
    //     between the player's view yaw and current horizontal velocity
    //     vector. Strafe in the direction that points the velocity
    //     toward the view ("perfect strafe") — produces buttery, fluid
    //     turns regardless of mouse speed.
    //   strafeMode = 1: legacy mouse-yaw delta. Follows the user's mouse
    //     turn rate frame-by-frame. Less optimal, more "manual feel".
    //
    // All button writes go through WriteBtnIfChanged so we don't spam
    // the same value over and over (which produces stutter + bloats
    // our memory-write footprint that anti-cheat heuristics scan for).
    // ---------------------------------------------------------------
    inline void AutoStrafe()
    {
        if (!cfg.autoStrafe) return;
        if (!GameState::clientBase) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        uint32_t flags = Mem::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        bool onGround = (flags & FL_ONGROUND) != 0;
        uintptr_t base = GameState::clientBase;

        auto releaseStrafe = [&]() {
            WriteBtnIfChanged(base, ButtonOffsets::left,    256, lastBtnLeft);
            WriteBtnIfChanged(base, ButtonOffsets::right,   256, lastBtnRight);
            // NOTE: we do NOT release forward/back here — those are
            // user controls and on the ground we leave them alone.
        };

        if (onGround)
        {
            if (inAutoStrafe)
            {
                releaseStrafe();
                // Ground frame: also release any forward/back override
                // we held so the player gets normal walking control back.
                WriteBtnIfChanged(base, ButtonOffsets::forward, 256, lastBtnForward);
                WriteBtnIfChanged(base, ButtonOffsets::back,    256, lastBtnBack);
                inAutoStrafe = false;
            }
            yawInit = false;
            return;
        }

        // === IN AIR ===

        // Only auto strafe while the bhop key is held (don't surprise the
        // user when they're just falling normally).
        if (!(GetAsyncKeyState(cfg.key) & 0x8000))
        {
            if (inAutoStrafe)
            {
                releaseStrafe();
                inAutoStrafe = false;
            }
            yawInit = false;
            return;
        }

        inAutoStrafe = true;

        // CRITICAL: in Source, holding W/S in air kills air acceleration.
        // Force them released so A/D can do their job.
        WriteBtnIfChanged(base, ButtonOffsets::forward, 256, lastBtnForward);
        WriteBtnIfChanged(base, ButtonOffsets::back,    256, lastBtnBack);

        float curYaw = Mem::Read<Math::QAngle>(
            GameState::clientBase + GameState::RVA_dwViewAngles()).yaw;

        // ---- direction = sign of "turn into velocity" ----
        // dir < 0  → strafe LEFT (press A)
        // dir > 0  → strafe RIGHT (press D)
        // dir == 0 → no input (hold current speed)
        float dir = 0.f;

        if (cfg.strafeMode == 0)
        {
            // Velocity-vector mode. Compute angle from velocity vector.
            Math::Vec3 vel = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecVelocity);
            float speed2D = sqrtf(vel.x * vel.x + vel.y * vel.y);

            if (speed2D < 30.f)
            {
                // No real horizontal motion yet — kickstart by alternating.
                static bool kickRight = false;
                kickRight = !kickRight;
                dir = kickRight ? 1.f : -1.f;
            }
            else
            {
                // Angle between view yaw and velocity heading. Source
                // engine yaw uses degrees with east=0, north=90.
                float velYawDeg = atan2f(vel.y, vel.x) * (180.f / 3.14159265f);
                float diff = curYaw - velYawDeg;
                while (diff >  180.f) diff -= 360.f;
                while (diff < -180.f) diff += 360.f;

                // diff > 0 → view is to the LEFT of velocity heading
                //             so to keep velocity rotating into view we
                //             press A (strafe left, turn velocity left).
                // diff < 0 → press D (strafe right).
                if      (diff >  1.0f) dir = -1.f;
                else if (diff < -1.0f) dir =  1.f;
                // tiny residual diff → coast, no input.
            }
        }
        else
        {
            // Legacy mouse-yaw delta mode.
            if (!yawInit) { savedYaw = curYaw; yawInit = true; return; }
            float delta = curYaw - savedYaw;
            savedYaw = curYaw;
            if (delta >  180.f) delta -= 360.f;
            if (delta < -180.f) delta += 360.f;

            if      (delta >  0.10f) dir = -1.f;  // turning left
            else if (delta < -0.10f) dir =  1.f;  // turning right
        }

        if (dir < 0.f)
        {
            WriteBtnIfChanged(base, ButtonOffsets::left,  65537, lastBtnLeft);
            WriteBtnIfChanged(base, ButtonOffsets::right,   256, lastBtnRight);
        }
        else if (dir > 0.f)
        {
            WriteBtnIfChanged(base, ButtonOffsets::right, 65537, lastBtnRight);
            WriteBtnIfChanged(base, ButtonOffsets::left,    256, lastBtnLeft);
        }
        else
        {
            WriteBtnIfChanged(base, ButtonOffsets::left,    256, lastBtnLeft);
            WriteBtnIfChanged(base, ButtonOffsets::right,   256, lastBtnRight);
        }
    }

    // ---------------------------------------------------------------
    // Auto Bhop — fully automatic jump on ground contact
    // Alternates between +jump and -jump each call to ensure
    // the game registers the input properly.
    // ---------------------------------------------------------------
    inline void DoBhop()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        // Only bhop when the configured key is held
        if (!(GetAsyncKeyState(cfg.key) & 0x8000))
        {
            jumpToggle = false;
            return;
        }

        int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
        if (health <= 0) return;

        uint32_t flags = Mem::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        uintptr_t forceJump = GameState::clientBase + ButtonOffsets::jump;

        if (flags & FL_ONGROUND)
        {
            // On ground — alternate between press and release
            // This ensures the game sees a fresh key press each landing
            if (!jumpToggle)
            {
                Mem::Write<uint32_t>(forceJump, 65537); // +jump
                jumpToggle = true;
            }
            else
            {
                Mem::Write<uint32_t>(forceJump, 256);   // -jump
                jumpToggle = false;
            }
            hopCount++;
            lastHopTime = GetTickCount();
        }
        else
        {
            // In air — always release jump so we can re-press on landing
            Mem::Write<uint32_t>(forceJump, 256);
            jumpToggle = false;
        }
    }

    // ---------------------------------------------------------------
    // Speed preservation — zero stamina/velocity penalties
    // ---------------------------------------------------------------
    inline void PreserveSpeed()
    {
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        Mem::Write<float>(localPawn + m_flVelocityModifier_pawn, 1.0f);

        uintptr_t moveSvc = Mem::Read<uintptr_t>(localPawn + m_pMovementServices_pawn);
        if (moveSvc)
        {
            Mem::Write<float>(moveSvc + m_flStamina_movement,           0.f);
            Mem::Write<float>(moveSvc + m_flStaminaAtJumpStart_movement, 0.f);
            Mem::Write<float>(moveSvc + m_flAccumulatedJumpError_mov,    0.f);
        }
    }

    // ---------------------------------------------------------------
    // GetLastJumpVelZ — reads m_flLastJumpVelocityZ from the movement
    // services block.  Aimbot's jumpshot/triggerbot use this to find
    // the actual apex of the current jump rather than guessing from
    // the noisy live m_vecVelocity.z (which oscillates because the
    // physics integrator updates between client+server ticks).
    // ---------------------------------------------------------------
    inline float GetLastJumpVelZ()
    {
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return 0.f;
        uintptr_t moveSvc = Mem::Read<uintptr_t>(localPawn + m_pMovementServices_pawn);
        if (!moveSvc) return 0.f;
        return Mem::Read<float>(moveSvc + m_flLastJumpVelocityZ_movement);
    }

    // ---------------------------------------------------------------
    // Called from SilentAim::hkCreateMove (tick-synced)
    // ---------------------------------------------------------------
    inline void OnCreateMove(__int64 /*inputPtr*/)
    {
        if (!cfg.enabled) return;
        __try {
            DoBhop();
            AutoStrafe();
            PreserveSpeed();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Legacy API compatibility
    inline void Init() { hookInstalled = true; }
    inline void Shutdown() { hookInstalled = false; }

    // ---------------------------------------------------------------
    // Tick — called from PresentCore (render loop, every frame)
    // Runs full auto bhop + auto strafe + speed preservation.
    // This ensures bhop works even if CreateMove hook is not installed.
    // ---------------------------------------------------------------
    inline void Tick()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {

        DoBhop();
        AutoStrafe();
        PreserveSpeed();

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        uint32_t flags = Mem::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        bool onGround = (flags & FL_ONGROUND) != 0;

        if (onGround && !wasOnGround)
        {
            hopCount++;
            lastHopTime = GetTickCount();
        }
        wasOnGround = onGround;

        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Velocity Display — rendered as HUD overlay showing current
    // horizontal speed, peak speed, and hop count.
    // ---------------------------------------------------------------
    inline float currentSpeed  = 0.f;
    inline float peakSpeed     = 0.f;
    inline DWORD peakResetTime = 0;

    inline void RenderVelocity()
    {
        if (!cfg.showVelocity) return;
        if (!GameState::clientBase) return;

        __try {
            uintptr_t lp = GameState::GetLocalPawn();
            if (!lp) return;

            int32_t hp = Mem::Read<int32_t>(lp + Offsets::m_iHealth);
            if (hp <= 0) return;

            // Read velocity (horizontal only: x,y)
            Math::Vec3 vel = Mem::Read<Math::Vec3>(lp + Offsets::m_vecVelocity);
            currentSpeed = sqrtf(vel.x * vel.x + vel.y * vel.y);

            // Track peak speed (reset after 3 seconds of no new peak)
            DWORD now = GetTickCount();
            if (currentSpeed > peakSpeed)
            {
                peakSpeed = currentSpeed;
                peakResetTime = now;
            }
            if (now - peakResetTime > 3000)
                peakSpeed = currentSpeed;

            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            if (!dl) return;

            ImVec2 disp = ImGui::GetIO().DisplaySize;
            float cx = disp.x * 0.5f;
            float cy = disp.y * 0.82f;

            // Speed color: green < 250, yellow 250-300, red > 300
            ImU32 speedCol;
            if (currentSpeed > 300.f)
                speedCol = IM_COL32(255, 60, 60, 255);
            else if (currentSpeed > 250.f)
                speedCol = IM_COL32(255, 200, 60, 255);
            else
                speedCol = IM_COL32(180, 255, 180, 255);

            // Main speed number
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0f u/s", currentSpeed);
            ImVec2 sz = ImGui::CalcTextSize(buf);
            dl->AddText(ImVec2(cx - sz.x * 0.5f + 1, cy + 1), IM_COL32(0,0,0,180), buf);
            dl->AddText(ImVec2(cx - sz.x * 0.5f, cy), speedCol, buf);

            // Peak + hops (smaller, below)
            if (cfg.enabled && hopCount > 0)
            {
                snprintf(buf, sizeof(buf), "peak: %.0f | hops: %d", peakSpeed, hopCount);
                ImVec2 sz2 = ImGui::CalcTextSize(buf);
                float y2 = cy + sz.y + 2;
                dl->AddText(ImVec2(cx - sz2.x * 0.5f + 1, y2 + 1), IM_COL32(0,0,0,120), buf);
                dl->AddText(ImVec2(cx - sz2.x * 0.5f, y2), IM_COL32(200,200,200,200), buf);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}
