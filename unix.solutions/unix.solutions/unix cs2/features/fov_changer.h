#pragma once
#include <Windows.h>
#include <thread>
#include <atomic>
#include "../sdk/game.h"
#include "../sdk/offsets.h"

// Externs declared in present.h but managed here/linked to menu
extern bool g_ViewFovEnabled;
extern int  g_ViewFov;

namespace FOVChanger
{
    inline std::atomic<bool> running = false;

    inline void UpdateThread()
    {
        while (running.load())
        {
            // Run at high frequency to "Hard-Lock" the FOV
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            // Direct Pointer Method (Matches FOV_Changer_Source logic)
            uintptr_t localPawn = Game::Read<uintptr_t>(Game::clientBase + Offsets::dwLocalPlayerPawn);
            if (!localPawn)
                continue;

            // SKIP OVERRIDE IF SCOPED (Restores Sniper Zoom)
            bool isScoped = Game::Read<bool>(localPawn + Offsets::m_bIsScoped);
            if (isScoped)
                continue;

            uintptr_t cameraServices = Game::Read<uintptr_t>(localPawn + Offsets::m_pCameraServices);

            if (cameraServices)
            {
                // Overwriting these prevents engine resets (e.g. during zooming or scope-out)
                Game::Write<int>(cameraServices + Offsets::m_iFOV, g_ViewFov);
                Game::Write<int>(cameraServices + Offsets::m_iFOVStart, g_ViewFov);
                Game::Write<float>(cameraServices + Offsets::m_flFOVRate, 0.0f); // Disable smoothing
                Game::Write<float>(cameraServices + Offsets::m_flFOVTime, 0.0f); // Disable timer
            }

            // 2. Viewmodel FOV Logic
            // Scales arms and weapon model using the April 2026 offset (0x2424)
            Game::Write<float>(localPawn + Offsets::m_flViewmodelFOV, (float)g_ViewFov);

        }
    }

    inline void Init()
    {
        if (running.load()) return;
        running.store(true);
        std::thread(UpdateThread).detach();
    }

    inline void Shutdown()
    {
        running.store(false);
    }
}
