#pragma once

// ---------------------------------------------------------------
// Backtrack — exploit lag compensation to aim at players' past
// positions. Stores position history per-player, finds the best
// tick within the valid window, and feeds that position to the
// aimbot as the preferred target.
//
// How it works: CS2's lag compensation rewinds player positions
// on the server when processing your shot. By aiming at where
// an enemy WAS (up to ~200ms ago), the server still registers
// the hit because it rewinds to that tick.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../core/math.h"
#include "../vendor/imgui/imgui.h"

namespace Backtrack
{
    struct Config
    {
        bool  enabled     = false;
        int   maxTicksBack = 12;    // max ticks to look back (1 tick ~= 15.6ms at 64tick)
        bool  drawHistory = true;   // draw dots at backtrack positions
    };

    inline Config cfg;

    // Per-player tick record
    struct TickRecord
    {
        Math::Vec3 headPos;
        Math::Vec3 origin;
        float      simTime;
        bool       valid;
    };

    static constexpr int MAX_PLAYERS = 64;
    static constexpr int MAX_TICKS   = 24;   // ring buffer depth

    // Ring buffer per player
    inline TickRecord records[MAX_PLAYERS][MAX_TICKS] = {};
    inline int         writeIdx[MAX_PLAYERS]          = {};
    inline float       lastRecordTime[MAX_PLAYERS]    = {};

    // Best backtrack result for current frame (consumed by aimbot)
    inline volatile bool      hasBestTarget    = false;
    inline volatile float     bestHeadX        = 0.f;
    inline volatile float     bestHeadY        = 0.f;
    inline volatile float     bestHeadZ        = 0.f;
    inline volatile int       bestPlayerIdx    = -1;
    inline volatile float     bestSimTime      = 0.f;

    // ---------------------------------------------------------------
    // Record tick — called every frame, stores current head position
    // for each alive enemy player.
    // ---------------------------------------------------------------
    inline void RecordTick()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn) return;

            int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            for (int i = 1; i <= 64; ++i)
            {
                __try {
                    uintptr_t ctrl = GameState::GetEntityByIndex(i);
                    if (!ctrl) continue;

                    uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                    if (!pH || pH == 0xFFFFFFFF) continue;
                    uintptr_t pawn = GameState::ResolveHandle(pH);
                    if (!pawn || pawn == localPawn) continue;

                    int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                    if (hp <= 0) continue;

                    int team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                    if (team == localTeam) continue;

                    float simTime = Mem::Read<float>(pawn + Offsets::m_flSimulationTime);

                    // Only record if simulation time changed (new server tick for this player)
                    int idx = i - 1;
                    if (idx < 0 || idx >= MAX_PLAYERS) continue;
                    if (simTime == lastRecordTime[idx]) continue;
                    lastRecordTime[idx] = simTime;

                    Math::Vec3 head = GameState::GetBonePos(pawn, 7);
                    if (head.x == 0.f && head.y == 0.f && head.z == 0.f) continue;

                    Math::Vec3 origin = GameState::GetEntityOrigin(pawn);

                    int wi = writeIdx[idx] % MAX_TICKS;
                    records[idx][wi].headPos = head;
                    records[idx][wi].origin  = origin;
                    records[idx][wi].simTime = simTime;
                    records[idx][wi].valid   = true;
                    writeIdx[idx]++;
                } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Find best backtrack target — closest to crosshair from history
    // ---------------------------------------------------------------
    inline void FindBest()
    {
        if (!cfg.enabled || !GameState::clientBase) { hasBestTarget = false; return; }

        __try {
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn) { hasBestTarget = false; return; }

            Math::QAngle viewAng = Mem::Read<Math::QAngle>(
                GameState::clientBase + GameState::RVA_dwViewAngles());

            Math::Vec3 eyePos = GameState::GetEntityOrigin(localPawn);
            Math::Vec3 viewOff = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecViewOffset);
            eyePos.x += viewOff.x; eyePos.y += viewOff.y; eyePos.z += viewOff.z;

            float bestDist = 999999.f;
            hasBestTarget = false;

            int maxBack = cfg.maxTicksBack;
            if (maxBack > MAX_TICKS) maxBack = MAX_TICKS;

            for (int p = 0; p < MAX_PLAYERS; ++p)
            {
                int totalRecords = writeIdx[p];
                if (totalRecords < 2) continue;

                for (int t = 1; t <= maxBack && t <= totalRecords; ++t)
                {
                    int ri = (totalRecords - t) % MAX_TICKS;
                    if (ri < 0) ri += MAX_TICKS;
                    if (!records[p][ri].valid) continue;

                    Math::Vec3 head = records[p][ri].headPos;
                    float dx = head.x - eyePos.x;
                    float dy = head.y - eyePos.y;
                    float dz = head.z - eyePos.z;
                    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
                    if (dist < 1.f) continue;

                    // Calculate angle to this position
                    float pitchTo = -atan2f(dz, sqrtf(dx*dx + dy*dy)) * 57.2957795f;
                    float yawTo   = atan2f(dy, dx) * 57.2957795f;

                    float dpitch = pitchTo - viewAng.pitch;
                    float dyaw   = yawTo - viewAng.yaw;
                    while (dyaw > 180.f) dyaw -= 360.f;
                    while (dyaw < -180.f) dyaw += 360.f;

                    float angDist = sqrtf(dpitch*dpitch + dyaw*dyaw);

                    if (angDist < bestDist && angDist < 15.f) // within reasonable FOV
                    {
                        bestDist       = angDist;
                        bestHeadX      = head.x;
                        bestHeadY      = head.y;
                        bestHeadZ      = head.z;
                        bestPlayerIdx  = p;
                        bestSimTime    = records[p][ri].simTime;
                        hasBestTarget  = true;
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) { hasBestTarget = false; }
    }

    // ---------------------------------------------------------------
    // Render — draw backtrack position dots
    // ---------------------------------------------------------------
    inline void Render()
    {
        if (!cfg.enabled || !cfg.drawHistory || !GameState::clientBase) return;

        __try {
            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            if (!dl) return;

            ImVec2 disp = ImGui::GetIO().DisplaySize;
            float scrW = disp.x, scrH = disp.y;

            int maxBack = cfg.maxTicksBack;
            if (maxBack > MAX_TICKS) maxBack = MAX_TICKS;

            for (int p = 0; p < MAX_PLAYERS; ++p)
            {
                int total = writeIdx[p];
                if (total < 2) continue;

                for (int t = 1; t <= maxBack && t <= total; ++t)
                {
                    int ri = (total - t) % MAX_TICKS;
                    if (ri < 0) ri += MAX_TICKS;
                    if (!records[p][ri].valid) continue;

                    float sx, sy;
                    float pos[3] = { records[p][ri].headPos.x,
                                     records[p][ri].headPos.y,
                                     records[p][ri].headPos.z };
                    if (GameState::WorldToScreen(pos, sx, sy, scrW, scrH))
                    {
                        float alpha = 1.0f - (float)t / (float)(maxBack + 1);
                        ImU32 col = IM_COL32(255, 100, 100, (int)(alpha * 180));
                        dl->AddCircleFilled(ImVec2(sx, sy), 2.5f, col);
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void Tick()
    {
        if (!cfg.enabled) return;
        RecordTick();
        FindBest();
    }
}
