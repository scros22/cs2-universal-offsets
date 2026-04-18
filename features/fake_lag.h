#pragma once

// ---------------------------------------------------------------
// Fake Lag — choke outgoing network packets to make your movement
// appear laggy/teleporting to enemies. Works by manipulating
// m_nTickBase to desync the client from the server.
//
// When choking, the server doesn't receive your updated position
// for several ticks. When you "unchoke," all the positions are
// sent at once, making you appear to teleport.
//
// Modes:
//   Fixed   — always choke N ticks then send
//   Dynamic — choke more when moving, less when still
//   OnKey   — only fake lag while holding a key
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../core/math.h"

namespace FakeLag
{
    enum Mode { MODE_FIXED = 0, MODE_DYNAMIC, MODE_ONKEY };

    struct Config
    {
        bool  enabled     = false;
        int   mode        = MODE_FIXED;
        int   maxChoke    = 8;    // 1-14 ticks to choke
        int   key         = 0;    // key for MODE_ONKEY (0 = always)
    };

    inline Config cfg;

    // Internal state
    inline int  chokedTicks  = 0;
    inline bool isChoking    = false;

    // ---------------------------------------------------------------
    // ShouldChoke — decides whether to choke this tick.
    // Called from CreateMove hook.
    // ---------------------------------------------------------------
    inline bool ShouldChoke()
    {
        if (!cfg.enabled) return false;

        switch (cfg.mode)
        {
            case MODE_FIXED:
                return chokedTicks < cfg.maxChoke;

            case MODE_DYNAMIC:
            {
                // Choke more when moving fast
                __try {
                    uintptr_t lp = GameState::GetLocalPawn();
                    if (!lp) return false;
                    Math::Vec3 vel = Mem::Read<Math::Vec3>(lp + Offsets::m_vecVelocity);
                    float speed = sqrtf(vel.x*vel.x + vel.y*vel.y);
                    int dynamicMax = (speed > 200.f) ? cfg.maxChoke :
                                     (speed > 50.f)  ? cfg.maxChoke / 2 : 2;
                    return chokedTicks < dynamicMax;
                } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
            }

            case MODE_ONKEY:
                if (cfg.key && !(GetAsyncKeyState(cfg.key) & 0x8000))
                    return false;
                return chokedTicks < cfg.maxChoke;

            default:
                return false;
        }
    }

    // ---------------------------------------------------------------
    // OnCreateMove — called from the CreateMove hook.
    // When choking, we skip the original CreateMove to suppress
    // the outgoing command. We track choked ticks and flush when
    // we reach the limit.
    //
    // Returns true if we should SKIP calling oCreateMove (choking).
    // Returns false if we should call oCreateMove normally (flushing).
    // ---------------------------------------------------------------
    inline bool OnCreateMove()
    {
        if (!cfg.enabled)
        {
            chokedTicks = 0;
            isChoking   = false;
            return false; // don't choke
        }

        // Don't choke while shooting (let shots through)
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
        {
            chokedTicks = 0;
            isChoking   = false;
            return false;
        }

        if (ShouldChoke())
        {
            chokedTicks++;
            isChoking = true;
            return true; // choke: skip oCreateMove
        }
        else
        {
            chokedTicks = 0;
            isChoking   = false;
            return false; // flush: call oCreateMove normally
        }
    }
}
