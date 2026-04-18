#pragma once
#include <Windows.h>
#include <cstdint>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "../sdk/offsets.hpp"
#include "../sdk/buttons.hpp"

namespace Bhop
{
    struct BhopConfig
    {
        bool enabled = false;
    };

    inline BhopConfig config;

    inline void Tick(uintptr_t localPawn)
    {
        if (!config.enabled) return;
        if (!localPawn) return;
        if (!Game::clientBase) return;

        if (!(GetAsyncKeyState(VK_SPACE) & 0x8000)) return;

        int health = Game::Read<int32_t>(localPawn + Offsets::m_iHealth);
        if (health <= 0) return;

        uint32_t flags = Game::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        uintptr_t forceJump = Game::clientBase + cs2_dumper::buttons::jump;
        bool onGround = (flags & 1) != 0;

        if (onGround)
        {
            Game::Write<uint32_t>(forceJump, 65537); // +jump
        }
        else
        {
            Game::Write<uint32_t>(forceJump, 256); // -jump
        }
    }
}
