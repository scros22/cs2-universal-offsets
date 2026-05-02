#pragma once

// ---------------------------------------------------------------
// Anti-Aim â€” desync your hitbox from your visual model to make
// you harder to hit. Works by writing to m_angEyeAngles on the
// local pawn (what other players see) while your actual view
// stays unchanged.
//
// Modes:
//   Spin    â€” rapidly rotate yaw (classic spinbot)
//   Jitter  â€” snap between opposite angles each tick
//   Down    â€” pitch down to 89 (makes headshots harder)
//   Desync  â€” offset body yaw from head yaw
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
#include "../../core/memory.h"
#include "../../core/math.h"

namespace AntiAim
{
    enum Mode { MODE_OFF = 0, MODE_SPIN, MODE_JITTER, MODE_DOWN, MODE_DESYNC };

    struct Config
    {
        bool  enabled      = false;
        int   pitchMode    = 0;   // 0=off, 1=down, 2=up, 3=zero
        int   yawMode      = 0;   // 0=off, 1=spin, 2=jitter, 3=desync
        float spinSpeed    = 15.f; // degrees per tick
        float desyncDelta  = 58.f; // body yaw offset from real yaw
    };

    inline Config cfg;

    // Internal state
    inline float spinAngle    = 0.f;
    inline bool  jitterSide   = false;
    inline DWORD lastTickTime = 0;

    inline void Tick()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn) return;

            int32_t hp = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
            if (hp <= 0) return;

            // Don't anti-aim while attacking (let aim work normally)
            if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) return;

            // Read current real view angles
            Math::QAngle viewAng = Mem::Read<Math::QAngle>(
                GameState::clientBase + GameState::RVA_dwViewAngles());

            float fakePitch = viewAng.pitch;
            float fakeYaw   = viewAng.yaw;

            // Pitch manipulation
            switch (cfg.pitchMode)
            {
                case 1: fakePitch =  89.f;  break; // Down
                case 2: fakePitch = -89.f;  break; // Up
                case 3: fakePitch =  0.f;   break; // Zero
                default: break;
            }

            // Yaw manipulation
            switch (cfg.yawMode)
            {
                case 1: // Spin
                    spinAngle += cfg.spinSpeed;
                    if (spinAngle > 360.f) spinAngle -= 360.f;
                    fakeYaw = spinAngle;
                    break;

                case 2: // Jitter
                    jitterSide = !jitterSide;
                    fakeYaw = viewAng.yaw + (jitterSide ? 90.f : -90.f);
                    break;

                case 3: // Desync â€” offset body yaw
                    jitterSide = !jitterSide;
                    fakeYaw = viewAng.yaw + (jitterSide ? cfg.desyncDelta : -cfg.desyncDelta);
                    break;

                default: break;
            }

            // Normalize
            while (fakeYaw > 180.f) fakeYaw -= 360.f;
            while (fakeYaw < -180.f) fakeYaw += 360.f;

            // Write fake angles to m_angEyeAngles (networked â€” other players see this)
            Math::QAngle fakeAng = { fakePitch, fakeYaw, 0.f };
            Mem::Write<Math::QAngle>(localPawn + Offsets::m_angEyeAngles, fakeAng);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}
