#pragma once
#include <Windows.h>
#include <cstdint>
#include <chrono>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "../sdk/offsets.hpp"
#include "../sdk/buttons.hpp"

namespace Triggerbot
{
    struct TriggerConfig
    {
        bool enabled = false;
        bool alwaysOn = false;
        int key = VK_MENU; // Default ALT key
        bool teamCheck = true;
        int delayMs = 15;
    };

    inline TriggerConfig config;
    inline std::chrono::steady_clock::time_point lastActionTime;
    inline bool wasShooting = false;

    inline void Tick(uintptr_t localPawn)
    {
        if (!config.enabled) return;
        if (!localPawn) return;
        if (!Game::clientBase) return;

        bool shouldFire = false;
        bool keyIsDown = (config.key == 0) || (GetAsyncKeyState(config.key) & 0x8000);

        // Check key or always on
        if (config.alwaysOn || keyIsDown)
        {
            // crosshairId is an EntIndex handle
            int crosshairId = Game::Read<int>(localPawn + 0x3EAC);

            if (crosshairId > 0)
            {
                uintptr_t crosshairEnt = Game::GetEntityByHandle(crosshairId);
                
                if (crosshairEnt)
                {
                    uintptr_t crosshairPawn = crosshairEnt;
                    
                    // m_hPlayerPawn is 0x90C in CCSPlayerController
                    uint32_t pawnHandle = Game::Read<uint32_t>(crosshairEnt + 0x90C);

                    if (pawnHandle != 0 && pawnHandle != 0xFFFFFFFF)
                    {
                        uintptr_t actualPawn = Game::GetEntityByHandle(pawnHandle);
                        if (actualPawn) crosshairPawn = actualPawn;
                    }

                    int health = Game::Read<int32_t>(crosshairPawn + Offsets::m_iHealth);
                    if (health > 0)
                    {
                        bool isEnemy = true;
                        if (config.teamCheck)
                        {
                            int myTeam = Game::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
                            int theirTeam = Game::Read<uint8_t>(crosshairPawn + Offsets::m_iTeamNum);
                            if (myTeam == theirTeam) isEnemy = false;
                        }

                        if (isEnemy)
                        {
                            shouldFire = true;
                        }
                    }
                }
            }
        }

        uintptr_t forceAttack = Game::clientBase + cs2_dumper::buttons::attack;
        auto now = std::chrono::steady_clock::now();

        if (shouldFire)
        {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActionTime).count() > config.delayMs)
            {
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                
                lastActionTime = now;
            }
        }
    }
}
