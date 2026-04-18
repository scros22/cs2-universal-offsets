#pragma once

// ---------------------------------------------------------------
// Auto-Accept — automatically accepts matchmaking when queue pops.
// Monitors the accept button state in Panorama UI via
// the game's IsLocalPlayerWaitingForAccept pattern and simulates
// the accept action by writing to the force button address.
//
// Approach: Poll CCSGOMatchmakingQueueState from dwGameTypes in
// matchmaking.dll. When ready-up detected, send keypress to
// accept (Enter key) via SendInput.
// Alternative: Direct Panorama JS dispatch via concommand.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include "../core/memory.h"
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"

namespace AutoAccept
{
    struct Config
    {
        bool enabled = false;
        float delay  = 0.5f;   // seconds delay before accepting (looks human)
    };
    inline Config cfg;

    // Internal state
    inline bool   wasAccepting    = false;
    inline DWORD  acceptDetectTime = 0;
    inline bool   accepted        = false;

    // Detect queue pop by monitoring the HUD accept overlay.
    // CS2 sets a specific byte in the matchmaking module when the
    // accept dialog is shown. We also watch for the distinct
    // flash overlay alpha changing pattern that indicates queue pop.
    //
    // Simpler approach: The game's accept button is handled by
    // Panorama. We use the `gameui_activate` + Enter key approach
    // which is widely used and undetected since it's identical to
    // a real user pressing Enter.

    inline void Tick()
    {
        if (!cfg.enabled) return;

        // Check if we're in a matchmaking queue that popped
        // Method: Read SignOnState from NetworkGameClient
        // SignOnState 0 = not connected, waiting in lobby
        // When accept dialog shows, game is still SignOnState 0
        // but the accept Panorama overlay is visible
        //
        // We detect via matchmaking.dll dwGameTypes state
        uintptr_t mmBase = Mem::GetModBase(L"matchmaking.dll");
        if (!mmBase) { wasAccepting = false; accepted = false; return; }

        // dwGameTypes + 0x8 contains a pointer to the queue manager state
        // When accept dialog is shown, a specific flag at a known offset is set
        // We use a simpler heuristic: if engine is not connected (signOnState < 6)
        // AND the accept overlay panel is displayed
        uintptr_t ngc = Mem::Read<uintptr_t>(
            GameState::engine2Base + Offsets::Global::dwNetworkGameClient);
        if (!ngc) { wasAccepting = false; accepted = false; return; }

        int signOnState = Mem::Read<int>(ngc + Offsets::Global::dwNetworkGameClient_signOnState);

        // When in full match, signOnState >= 6. Don't try to accept.
        if (signOnState >= 6) { wasAccepting = false; accepted = false; return; }

        // Check if the matchmaking ready-up is active
        // The game sets a specific state in matchmaking.dll when accept dialog appears
        // We read the queue state from dwGameTypes
        uintptr_t gameTypes = mmBase + 0x1B8000; // dwGameTypes
        if (!gameTypes) return;

        // At dwGameTypes + 0x1D0 is m_bMatchmakingRunning (bool)
        // At dwGameTypes + 0x2B8 is the accept/ready state
        // When the accept dialog shows, the "ready up" state flips
        bool mmRunning = Mem::Read<bool>(gameTypes + 0x1D0);

        // More reliable: scan for the Panorama accept panel being visible
        // by checking if we're queued (mmRunning) AND not yet connected
        // AND the "accept" window has appeared (indicated by state change)
        //
        // Simplest reliable method: Use ConCommand approach
        // When accept dialog shows, the command "cl_mm_accept" works
        // We detect it by polling mmRunning && signOnState==0 pattern
        // then fire the accept after delay

        if (!mmRunning)
        {
            wasAccepting = false;
            accepted = false;
            return;
        }

        // Queue is running and we're not connected — potential accept state
        // The accept dialog typically shows when we transition from
        // searching to found state. We detect this by checking if
        // deltaTick is -1 (not connected to game server)
        int deltaTick = Mem::Read<int>(ngc + Offsets::Global::dwNetworkGameClient_deltaTick);
        bool notConnected = (deltaTick <= 0);

        if (mmRunning && notConnected)
        {
            // This combination suggests the accept dialog might be up
            if (!wasAccepting)
            {
                // First detection — start timer
                wasAccepting    = true;
                acceptDetectTime = GetTickCount();
                accepted        = false;
            }

            if (!accepted)
            {
                DWORD elapsed = GetTickCount() - acceptDetectTime;
                float delaySec = cfg.delay;
                if (delaySec < 0.1f) delaySec = 0.1f;

                if (elapsed >= (DWORD)(delaySec * 1000.f))
                {
                    // Fire accept — press Enter which accepts the Panorama prompt
                    INPUT inputs[2] = {};
                    inputs[0].type = INPUT_KEYBOARD;
                    inputs[0].ki.wVk = VK_RETURN;
                    inputs[0].ki.dwFlags = 0; // keydown

                    inputs[1].type = INPUT_KEYBOARD;
                    inputs[1].ki.wVk = VK_RETURN;
                    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

                    SendInput(2, inputs, sizeof(INPUT));
                    accepted = true;
                }
            }
        }
        else
        {
            wasAccepting = false;
            accepted     = false;
        }
    }
}
