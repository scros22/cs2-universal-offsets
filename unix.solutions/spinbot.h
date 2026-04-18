#pragma once
#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "../sdk/game.h"
#include "../sdk/offsets.h"

namespace Spinbot
{
    struct SpinConfig
    {
        bool enabled = false;
        float yawSpeed = 15.0f;   // degrees per tick
        float pitch = 0.0f;       // fixed pitch (-89 to 89)
        bool antiAim = false;     // jitter instead of smooth spin
    };

    inline SpinConfig config;
    inline float currentYaw = 0.0f;

    // Called from CreateMove BEFORE original — modifies server-side angles only
    // Returns the yaw to use for the server; caller restores visual angles after
    inline float GetSpinYaw()
    {
        if (!config.enabled) return -999.0f;

        if (config.antiAim)
        {
            // Jitter: alternate between two extremes
            static bool flip = false;
            flip = !flip;
            currentYaw = flip ? 180.0f : -180.0f;
        }
        else
        {
            currentYaw += config.yawSpeed;
            if (currentYaw > 180.0f) currentYaw -= 360.0f;
            if (currentYaw < -180.0f) currentYaw += 360.0f;
        }

        return currentYaw;
    }

    inline float GetSpinPitch()
    {
        return config.pitch;
    }
}
