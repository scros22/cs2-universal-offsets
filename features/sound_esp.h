#pragma once

// ---------------------------------------------------------------
// Sound ESP — draws directional indicators showing where enemy
// players are, even through walls. Uses entity positions and
// distance-based scaling to show nearby enemies as "sound" pings.
//
// Visual approach: draws circle indicators around crosshair
// pointing toward enemy direction, with size based on distance
// (closer = bigger). Also shows footstep icons at enemy feet
// when they're moving fast enough to make noise.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../core/math.h"
#include "../vendor/imgui/imgui.h"

namespace SoundESP
{
    struct Config
    {
        bool  enabled       = false;
        float maxDistance    = 2000.f;  // max distance to show (units)
        float indicatorSize = 40.f;    // base indicator size
        float ringRadius    = 80.f;    // distance of indicator from crosshair
        bool  showFootsteps = true;    // show footstep markers at feet
        float footstepSpeed = 130.f;   // min speed to show footstep marker
    };

    inline Config cfg;

    // Footstep trail record
    struct FootstepMark
    {
        Math::Vec3 pos;
        DWORD      time;
        bool       valid;
    };

    static constexpr int MAX_MARKS    = 128;
    static constexpr DWORD MARK_LIFE  = 2000; // ms

    inline FootstepMark footsteps[MAX_MARKS] = {};
    inline int footstepIdx = 0;

    inline void AddFootstep(Math::Vec3 pos)
    {
        int i = footstepIdx % MAX_MARKS;
        footsteps[i].pos   = pos;
        footsteps[i].time  = GetTickCount();
        footsteps[i].valid = true;
        footstepIdx++;
    }

    // ---------------------------------------------------------------
    // Tick — record footstep positions for moving enemies
    // ---------------------------------------------------------------
    inline void Tick()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn) return;

            int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
            Math::Vec3 localPos = GameState::GetEntityOrigin(localPawn);

            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 1; i <= 64; ++i)
            {
                __try {
                    uintptr_t chunk = Mem::Read<uintptr_t>(entList + 0x8 * (i >> 9) + 0x10);
                    if (!chunk) continue;
                    uintptr_t ctrl = Mem::Read<uintptr_t>(chunk + GameState::kEntityStride * (i & 0x1FF));
                    if (!ctrl) continue;

                    uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                    if (!pH || pH == 0xFFFFFFFF) continue;
                    uintptr_t pawn = GameState::ResolveHandle(pH);
                    if (!pawn || pawn == localPawn) continue;

                    int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                    if (hp <= 0) continue;

                    int team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                    if (team == localTeam) continue;

                    Math::Vec3 pos = GameState::GetEntityOrigin(pawn);
                    float dx = pos.x - localPos.x;
                    float dy = pos.y - localPos.y;
                    float dz = pos.z - localPos.z;
                    float dist = sqrtf(dx*dx + dy*dy + dz*dz);

                    if (dist > cfg.maxDistance) continue;

                    // Check if moving (footstep sounds)
                    if (cfg.showFootsteps)
                    {
                        Math::Vec3 vel = Mem::Read<Math::Vec3>(pawn + Offsets::m_vecVelocity);
                        float speed = sqrtf(vel.x*vel.x + vel.y*vel.y);
                        if (speed > cfg.footstepSpeed)
                        {
                            AddFootstep(pos);
                        }
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Render — directional radar + footstep markers
    // ---------------------------------------------------------------
    inline void Render()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {
            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            if (!dl) return;

            ImVec2 disp = ImGui::GetIO().DisplaySize;
            float scrW = disp.x, scrH = disp.y;
            float cx = scrW * 0.5f;
            float cy = scrH * 0.5f;

            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn) return;

            int32_t hp = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
            if (hp <= 0) return;

            int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
            Math::Vec3 localPos = GameState::GetEntityOrigin(localPawn);

            // Get view angles for direction calculation
            Math::QAngle viewAng = Mem::Read<Math::QAngle>(
                GameState::clientBase + GameState::RVA_dwViewAngles());
            float viewYawRad = viewAng.yaw * 0.01745329f; // deg to rad

            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            // --- Directional indicators ---
            for (int i = 1; i <= 64; ++i)
            {
                __try {
                    uintptr_t chunk = Mem::Read<uintptr_t>(entList + 0x8 * (i >> 9) + 0x10);
                    if (!chunk) continue;
                    uintptr_t ctrl = Mem::Read<uintptr_t>(chunk + GameState::kEntityStride * (i & 0x1FF));
                    if (!ctrl) continue;

                    uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                    if (!pH || pH == 0xFFFFFFFF) continue;
                    uintptr_t pawn = GameState::ResolveHandle(pH);
                    if (!pawn || pawn == localPawn) continue;

                    int enemyHp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                    if (enemyHp <= 0) continue;

                    int team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                    if (team == localTeam) continue;

                    Math::Vec3 pos = GameState::GetEntityOrigin(pawn);
                    float dx = pos.x - localPos.x;
                    float dy = pos.y - localPos.y;
                    float dist = sqrtf(dx*dx + dy*dy);

                    if (dist > cfg.maxDistance || dist < 1.f) continue;

                    // Angle from local to enemy (world space)
                    float worldAngle = atan2f(dy, dx);
                    // Relative to view direction (screen space)
                    float relAngle = worldAngle - viewYawRad;

                    // Arrow position on ring around crosshair
                    float radius = cfg.ringRadius;
                    float ax = cx + cosf(relAngle) * radius;
                    float ay = cy - sinf(relAngle) * radius;

                    // Size/alpha based on distance (closer = bigger/brighter)
                    float t = 1.0f - (dist / cfg.maxDistance);
                    float sz = cfg.indicatorSize * (0.3f + 0.7f * t);
                    int alpha = (int)(80.f + 175.f * t);
                    if (alpha > 255) alpha = 255;

                    // Draw arrow/triangle pointing toward enemy
                    float dirX = cosf(relAngle);
                    float dirY = -sinf(relAngle);
                    float perpX = -dirY;
                    float perpY = dirX;

                    ImVec2 tip(ax + dirX * sz * 0.5f, ay + dirY * sz * 0.5f);
                    ImVec2 bl(ax - dirX * sz * 0.3f + perpX * sz * 0.25f,
                              ay - dirY * sz * 0.3f + perpY * sz * 0.25f);
                    ImVec2 br(ax - dirX * sz * 0.3f - perpX * sz * 0.25f,
                              ay - dirY * sz * 0.3f - perpY * sz * 0.25f);

                    // Color: yellow for far, orange/red for close
                    ImU32 col;
                    if (t > 0.7f)
                        col = IM_COL32(255, 60, 60, alpha);    // close = red
                    else if (t > 0.4f)
                        col = IM_COL32(255, 160, 40, alpha);   // mid = orange
                    else
                        col = IM_COL32(255, 220, 80, alpha);   // far = yellow

                    dl->AddTriangleFilled(tip, bl, br, col);
                    dl->AddTriangle(tip, bl, br, IM_COL32(0, 0, 0, alpha / 2), 1.0f);
                } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
            }

            // --- Footstep markers (3D in world) ---
            if (cfg.showFootsteps)
            {
                DWORD now = GetTickCount();
                for (int i = 0; i < MAX_MARKS; ++i)
                {
                    if (!footsteps[i].valid) continue;
                    if (now - footsteps[i].time > MARK_LIFE)
                    {
                        footsteps[i].valid = false;
                        continue;
                    }

                    float sx, sy;
                    float pos[3] = { footsteps[i].pos.x,
                                     footsteps[i].pos.y,
                                     footsteps[i].pos.z };
                    if (GameState::WorldToScreen(pos, sx, sy, scrW, scrH))
                    {
                        float age = (float)(now - footsteps[i].time) / (float)MARK_LIFE;
                        int alpha = (int)(200.f * (1.0f - age));
                        if (alpha < 10) continue;

                        ImU32 col = IM_COL32(255, 180, 50, alpha);
                        // Small circle = footstep
                        dl->AddCircle(ImVec2(sx, sy), 4.f, col, 8, 1.5f);
                        dl->AddCircle(ImVec2(sx, sy), 8.f * (0.5f + age * 0.5f),
                                      IM_COL32(255, 180, 50, alpha / 3), 12, 1.0f);
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}
