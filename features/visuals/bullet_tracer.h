#pragma once

// ---------------------------------------------------------------
// Bullet tracer â€” renders animated trace lines along shot paths.
// ---------------------------------------------------------------

#include <Windows.h>
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
#include "../../core/math.h"
#include "../../core/memory.h"
#include "../../vendor/imgui/imgui.h"

namespace BulletTracer
{
    struct Config
    {
        bool  enabled     = true;
        float trailLife   = 2.5f;
        float bulletSpeed = 8000.f;
        float thickness   = 2.f;
        float rayLength   = 8000.f;
    };

    inline Config cfg;

    struct Trace
    {
        float start[3], end[3];
        float spawnTime, totalDist;
    };

    inline std::vector<Trace> traces;
    inline std::mutex traceMtx;
    inline int prevShotsFired = 0;

    inline float Now() { return static_cast<float>(GetTickCount64()) / 1000.f; }

    inline void AddTraceFromAngles(float ex, float ey, float ez, float pitch, float yaw)
    {
        if (!cfg.enabled) return;
        float dir[3];
        Math::AngleToDirection(pitch, yaw, dir);

        Trace t;
        t.start[0] = ex; t.start[1] = ey; t.start[2] = ez;
        t.end[0] = ex + dir[0] * cfg.rayLength;
        t.end[1] = ey + dir[1] * cfg.rayLength;
        t.end[2] = ez + dir[2] * cfg.rayLength;
        t.spawnTime = Now();

        float dx = t.end[0] - ex, dy = t.end[1] - ey, dz = t.end[2] - ez;
        t.totalDist = sqrtf(dx * dx + dy * dy + dz * dz);

        std::lock_guard<std::mutex> lk(traceMtx);
        traces.push_back(t);
    }

    inline bool DetectShot(uintptr_t pawn)
    {
        if (!pawn) return false;
        int cur = Mem::Read<int>(pawn + Offsets::m_iShotsFired);
        if (cur > prevShotsFired && prevShotsFired >= 0)
        {
            prevShotsFired = cur;
            return true;
        }
        prevShotsFired = cur;
        return false;
    }

    inline void Render()
    {
        if (!cfg.enabled || !GameState::clientBase) return;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        float now = Now();
        ImVec2 disp = ImGui::GetIO().DisplaySize;
        std::lock_guard<std::mutex> lk(traceMtx);

        float maxAge = cfg.trailLife + 2.f;
        traces.erase(std::remove_if(traces.begin(), traces.end(),
            [now, maxAge](const Trace& t) { return now - t.spawnTime > maxAge; }), traces.end());

        for (const auto& t : traces)
        {
            float age = now - t.spawnTime;
            float travelT = t.totalDist / cfg.bulletSpeed;
            if (travelT < 0.01f) travelT = 0.01f;
            float bulletFrac = (age / travelT < 1.f) ? age / travelT : 1.f;

            float trailAge = age - travelT;
            float trailAlpha = 1.f;
            if (trailAge > 0.f)
            {
                trailAlpha = 1.f - trailAge / cfg.trailLife;
                if (trailAlpha <= 0.f) continue;
                trailAlpha *= trailAlpha;
            }

            constexpr int SEG = 16;
            ImVec2 pts[SEG + 1];
            bool   ok[SEG + 1] = {};

            for (int s = 0; s <= SEG; ++s)
            {
                float f = (float)s / SEG * bulletFrac;
                float p[3] = {
                    t.start[0] + (t.end[0] - t.start[0]) * f,
                    t.start[1] + (t.end[1] - t.start[1]) * f,
                    t.start[2] + (t.end[2] - t.start[2]) * f
                };
                ok[s] = GameState::WorldToScreen(p, pts[s].x, pts[s].y, disp.x, disp.y);
            }

            for (int s = 0; s < SEG; ++s)
            {
                if (!ok[s] || !ok[s + 1]) continue;
                float bright = 0.2f + 0.8f * ((float)s / SEG);
                int a = (int)(trailAlpha * bright * 220.f);
                if (a <= 0) continue;
                if (a > 255) a = 255;
                dl->AddLine(pts[s], pts[s + 1], IM_COL32(255, 255, 255, a), cfg.thickness);
                int ga = (int)(trailAlpha * bright * 40.f);
                if (ga > 255) ga = 255;
                dl->AddLine(pts[s], pts[s + 1], IM_COL32(180, 200, 255, ga), cfg.thickness * 3.5f);
            }

            if (bulletFrac < 1.f)
            {
                float hp[3] = {
                    t.start[0] + (t.end[0] - t.start[0]) * bulletFrac,
                    t.start[1] + (t.end[1] - t.start[1]) * bulletFrac,
                    t.start[2] + (t.end[2] - t.start[2]) * bulletFrac
                };
                float hx, hy;
                if (GameState::WorldToScreen(hp, hx, hy, disp.x, disp.y))
                    dl->AddCircleFilled({hx, hy}, 3.f, IM_COL32(255, 255, 255, (int)(trailAlpha * 255)));
            }
        }
    }
}
