#pragma once
#include <Windows.h>
#include <atomic>
#include <thread>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "bullet_tracer.h"

namespace Combat
{
    struct Config
    {
        bool noRecoil = false;
        bool noSpread = false;
        bool fakeGround = false;
        bool noFlash = false;
        bool noMovementShake = false;
        bool noFireShake = false;
    };

    inline Config config;
    inline std::atomic<bool> running{ false };

    inline void Tick()
    {
        uintptr_t localPawn = Game::GetLocalPlayerPawn();
        if (!localPawn) localPawn = Game::Read<uintptr_t>(Game::clientBase + Offsets::dwLocalPlayerPawn);
        if (!localPawn) return;

        // --- Bullet Tracer Support ---
        BulletTracer::Update(localPawn);

        // 4. Hardened Anti-Flash
        if (config.noFlash)
        {
            Game::Write<float>(localPawn + Offsets::m_flFlashMaxAlpha, 0.0f);
            Game::Write<float>(localPawn + Offsets::m_flFlashDuration, 0.0f);
        }

        // 5. Anti-Movement / Fire Shake
        if (config.noMovementShake || config.noFireShake)
        {
            Game::Write<Game::Vector3>(localPawn + Offsets::m_vViewPunchAngle, { 0, 0, 0 });
        }

        // 1. Recoil Lock (Moved to Aimbot hook for synchronization)
        /*if (config.noRecoil)
        {
            Game::Write<Game::Vector3>(localPawn + Offsets::m_aimPunchAngle, { 0, 0, 0 });
            Game::Write<Game::Vector3>(localPawn + Offsets::m_aimPunchAngleVel, { 0, 0, 0 });
        }*/

        uintptr_t weaponServices = Game::Read<uintptr_t>(localPawn + Offsets::m_pWeaponServices);
        if (weaponServices)
        {
            uint32_t weaponHandle = Game::Read<uint32_t>(weaponServices + Offsets::m_hActiveWeapon);
            uintptr_t activeWeapon = Game::GetEntityByHandle(weaponHandle);

            if (activeWeapon)
            {
                // 2. Accuracy Lock (Moved to Aimbot hook for synchronization)
                /*if (config.noSpread)
                {
                    Game::Write<float>(activeWeapon + Offsets::m_fAccuracyPenalty, 0.0f);
                    Game::Write<float>(activeWeapon + Offsets::m_flRecoilIndex, 0.0f);
                    Game::Write<int>(activeWeapon + Offsets::m_iRecoilIndex, 0);
                }*/
            }
        }

        // 3. Fake Ground State
        if (config.fakeGround)
        {
            if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
            {
                uint32_t flags = Game::Read<uint32_t>(localPawn + Offsets::m_fFlags);
                Game::Write<uint32_t>(localPawn + Offsets::m_fFlags, flags | (1 << 0));
            }
        }
    }

    inline void Init()
    {
        running.store(true);
        std::thread([]() {
            while (running.load())
            {
                Tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }).detach();
    }

    inline void Shutdown()
    {
        running.store(false);
    }
}
