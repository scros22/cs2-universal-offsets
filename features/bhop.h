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

// Button command offsets from client.dll (from dumper: buttons.hpp)
namespace ButtonOffsets {
    constexpr std::ptrdiff_t jump    = 0x2066C70;
    constexpr std::ptrdiff_t left    = 0x2066AC0;  // +moveleft
    constexpr std::ptrdiff_t right   = 0x2066B50;  // +moveright
    constexpr std::ptrdiff_t forward = 0x20669A0;  // +forward
    constexpr std::ptrdiff_t back    = 0x2066A30;  // +back
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

    // Movement services offsets
    constexpr std::ptrdiff_t m_pMovementServices      = 0x1418;
    constexpr std::ptrdiff_t m_flStamina              = 0x518;
    constexpr std::ptrdiff_t m_flStaminaAtJumpStart   = 0x528;
    constexpr std::ptrdiff_t m_flAccumulatedJumpError = 0x530;
    constexpr std::ptrdiff_t m_flVelocityModifier     = 0x2714;

    inline bool hookInstalled = false;

    // ---------------------------------------------------------------
    // Auto Strafe — proper Source engine air strafing
    //
    // The mechanic: in air, RELEASE forward key entirely, then
    // press A while turning mouse left OR press D while turning
    // mouse right. This is what creates the speed gain.
    //
    // On ground: don't touch any keys — let the player move normally.
    // On transition back to ground: release any strafe keys we held.
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

        if (onGround)
        {
            // Just landed or walking — release any strafe keys we were holding
            if (inAutoStrafe)
            {
                Mem::Write<uint32_t>(base + ButtonOffsets::left, 256);
                Mem::Write<uint32_t>(base + ButtonOffsets::right, 256);
                inAutoStrafe = false;
            }
            yawInit = false;
            return;
        }

        // === IN AIR ===

        // Only auto strafe while space is held (bhop active)
        if (!(GetAsyncKeyState(VK_SPACE) & 0x8000))
        {
            if (inAutoStrafe)
            {
                Mem::Write<uint32_t>(base + ButtonOffsets::left, 256);
                Mem::Write<uint32_t>(base + ButtonOffsets::right, 256);
                inAutoStrafe = false;
            }
            yawInit = false;
            return;
        }

        inAutoStrafe = true;

        // CRITICAL: Release forward key in air — holding W kills air accel
        Mem::Write<uint32_t>(base + ButtonOffsets::forward, 256);
        Mem::Write<uint32_t>(base + ButtonOffsets::back, 256);

        float curYaw = Mem::Read<Math::QAngle>(
            GameState::clientBase + GameState::RVA_dwViewAngles()).yaw;

        if (!yawInit)
        {
            savedYaw = curYaw;
            yawInit = true;
            // First frame in air — press both strafe directions briefly
            Mem::Write<uint32_t>(base + ButtonOffsets::left, 256);
            Mem::Write<uint32_t>(base + ButtonOffsets::right, 256);
            return;
        }

        float delta = curYaw - savedYaw;
        savedYaw = curYaw;

        if (delta > 180.f) delta -= 360.f;
        if (delta < -180.f) delta += 360.f;

        // Low threshold — even small mouse movements should trigger strafing
        if (delta > 0.05f)
        {
            // Turning left → strafe left (A key)
            Mem::Write<uint32_t>(base + ButtonOffsets::left, 65537);
            Mem::Write<uint32_t>(base + ButtonOffsets::right, 256);
        }
        else if (delta < -0.05f)
        {
            // Turning right → strafe right (D key)
            Mem::Write<uint32_t>(base + ButtonOffsets::right, 65537);
            Mem::Write<uint32_t>(base + ButtonOffsets::left, 256);
        }
        else
        {
            // Not turning — release both (no acceleration without turning)
            Mem::Write<uint32_t>(base + ButtonOffsets::left, 256);
            Mem::Write<uint32_t>(base + ButtonOffsets::right, 256);
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

        // Only bhop when space is held
        if (!(GetAsyncKeyState(VK_SPACE) & 0x8000))
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

        Mem::Write<float>(localPawn + m_flVelocityModifier, 1.0f);

        uintptr_t moveSvc = Mem::Read<uintptr_t>(localPawn + m_pMovementServices);
        if (moveSvc)
        {
            Mem::Write<float>(moveSvc + m_flStamina, 0.f);
            Mem::Write<float>(moveSvc + m_flStaminaAtJumpStart, 0.f);
            Mem::Write<float>(moveSvc + m_flAccumulatedJumpError, 0.f);
        }
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
