#pragma once
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "../imgui/imgui.h"

// ============================================
// Enhanced Bullet Tracer — SDK Synchronized
// ============================================

namespace BulletTracer
{
    using Game::Vector3;
    using Game::Vector2;
    using Game::ViewMatrix;

    struct TracerConfig
    {
        bool  enabled     = true;
        float trailLife   = 1.2f;
        float bulletSpeed = 8000.0f;
        float thickness   = 2.0f;
        float rayLength   = 8000.0f;
    };

    inline TracerConfig config;

    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float DEG2RAD = PI / 180.0f;

    inline void AngleToDirection(float pitch, float yaw, float out[3])
    {
        float p = pitch * DEG2RAD;
        float y = yaw * DEG2RAD;
        float cp = cosf(p);
        float sp = sinf(p);
        float cy = cosf(y);
        float sy = sinf(y);
        out[0] = cp * cy;
        out[1] = cp * sy;
        out[2] = -sp;
    }

    struct Trace
    {
        Vector3 startPos;
        Vector3 endPos;
        float   spawnTime;
        float   totalDist;
    };

    inline std::vector<Trace> traces;
    inline std::mutex traceMutex;
    inline int lastShotsFired = -1;

    inline float GetTime()
    {
        return static_cast<float>(GetTickCount64()) / 1000.0f;
    }

    // Standard detection logic
    inline bool DetectShot(uintptr_t localPawn)
    {
        if (!localPawn) return false;

        int currentShots = Game::Read<int>(localPawn + 0x270C); // m_iShotsFired
        if (currentShots <= 0) currentShots = Game::Read<int>(localPawn + 0x271C);

        if (lastShotsFired == -1)
        {
            lastShotsFired = currentShots;
            return false;
        }

        if (currentShots > lastShotsFired)
        {
            lastShotsFired = currentShots;
            return true;
        }
        
        lastShotsFired = currentShots;
        return false;
    }

    inline void AddTraceFromAngles(float eyeX, float eyeY, float eyeZ, float pitch, float yaw)
    {
        if (!config.enabled) return;

        float dir[3];
        AngleToDirection(pitch, yaw, dir);

        Trace t;
        t.startPos = { eyeX, eyeY, eyeZ };
        t.endPos   = { eyeX + dir[0] * config.rayLength, eyeY + dir[1] * config.rayLength, eyeZ + dir[2] * config.rayLength };
        t.spawnTime = GetTime();

        Vector3 delta = t.endPos - t.startPos;
        t.totalDist = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

        std::lock_guard<std::mutex> lock(traceMutex);
        traces.push_back(t);
    }

    // Unified Update call for Combat loop
    inline void Update(uintptr_t lp)
    {
        if (!config.enabled || !lp) return;

        if (DetectShot(lp))
        {
            uintptr_t sceneNode = Game::Read<uintptr_t>(lp + Offsets::m_pGameSceneNode);
            if (sceneNode)
            {
                Vector3 origin = Game::Read<Vector3>(sceneNode + Offsets::m_vecAbsOrigin);
                Vector3 viewOffset = Game::Read<Vector3>(lp + Offsets::m_vecViewOffset);
                Game::QAngle angles = Game::Read<Game::QAngle>(Game::clientBase + Offsets::dwViewAngles);
                
                AddTraceFromAngles(origin.x + viewOffset.x, origin.y + viewOffset.y, origin.z + viewOffset.z, angles.pitch, angles.yaw);
            }
        }
    }

    // Unified Render call for Present hook
    inline void Render()
    {
        if (!config.enabled || !Game::clientBase) return;

        float now = GetTime();
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!draw) return;

        ViewMatrix vm = Game::Read<ViewMatrix>(Game::clientBase + Offsets::dwViewMatrix);
        ImVec2 scrSize = ImGui::GetIO().DisplaySize;

        std::lock_guard<std::mutex> lock(traceMutex);

        traces.erase(
            std::remove_if(traces.begin(), traces.end(),
                [now](const Trace& t) { return (now - t.spawnTime) > config.trailLife; }),
            traces.end());

        for (const auto& t : traces)
        {
            float age = now - t.spawnTime;
            float travelTime = t.totalDist / config.bulletSpeed;
            if (travelTime < 0.01f) travelTime = 0.01f;
            float bulletFrac = age / travelTime;
            if (bulletFrac > 1.0f) bulletFrac = 1.0f;

            float trailAge = age - travelTime;
            float trailAlpha = 1.0f;
            if (trailAge > 0.0f)
            {
                trailAlpha = 1.0f - (trailAge / config.trailLife);
                if (trailAlpha <= 0.0f) continue;
            }

            constexpr int SEGMENTS = 14;
            ImVec2 pts[SEGMENTS + 1];
            bool   ok[SEGMENTS + 1] = {};

            for (int s = 0; s <= SEGMENTS; s++)
            {
                float segFrac = ((float)s / (float)SEGMENTS) * bulletFrac;
                Vector3 pos3d = {
                    t.startPos.x + (t.endPos.x - t.startPos.x) * segFrac,
                    t.startPos.y + (t.endPos.y - t.startPos.y) * segFrac,
                    t.startPos.z + (t.endPos.z - t.startPos.z) * segFrac
                };
                
                float p[3] = { pos3d.x, pos3d.y, pos3d.z };
                ok[s] = Game::WorldToScreen(p, pts[s].x, pts[s].y, scrSize.x, scrSize.y);
            }

            // --- Origin Correction (Prevents Top-Left Screen Artifacts) ---
            // If the start point is behind the camera/off-screen, default to bottom-center (UI-origin)
            if (!ok[0])
            {
                pts[0] = ImVec2(scrSize.x * 0.5f, scrSize.y * 1.0f);
                ok[0] = true;
            }


            for (int s = 0; s < SEGMENTS; s++)
            {
                // Draw segment if at least one point is on screen (or let ImGui handle clipping)
                // WorldToScreen already handles the 'behind camera' check (returns false if w < 0)
                if (!ok[s] && !ok[s+1]) continue;

                float segFrac = (float)s / (float)SEGMENTS;
                float brightness = 0.3f + 0.7f * segFrac;
                int alpha = static_cast<int>(trailAlpha * brightness * 255.0f);
                if (alpha <= 0) continue;

                // Simple safety for off-screen points to prevent crazy ImGui lines
                ImVec2 p1 = pts[s];
                ImVec2 p2 = pts[s+1];
                
                draw->AddLine(p1, p2, IM_COL32(255, 255, 255, alpha), config.thickness);
                draw->AddLine(p1, p2, IM_COL32(0, 255, 255, (int)(alpha * 0.4f)), config.thickness * 2.5f);
            }
        }
    }
}
