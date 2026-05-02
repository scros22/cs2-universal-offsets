#pragma once

// ---------------------------------------------------------------
// Nade Helper â€” shows guide overlay for common smoke/molotov
// lineups on popular competitive maps. Position-based: when you
// stand near a lineup spot, shows the aim direction and type.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include "../../core/memory.h"
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
#include "../../core/math.h"
#include "../../vendor/imgui/imgui.h"

namespace NadeHelper
{
    struct Config
    {
        bool  enabled      = false;
        float triggerDist  = 120.f;  // units â€” how close to stand to see lineup
        float aimTolerance = 3.f;    // degrees â€” green when within this
    };
    inline Config cfg;

    // Lineup definition
    struct Lineup
    {
        const char* name;         // e.g. "A Site Smoke"
        const char* map;          // e.g. "de_dust2"
        const char* type;         // "smoke", "molotov", "flash", "he"
        float standX, standY, standZ;   // where to stand
        float aimPitch, aimYaw;         // where to aim (angles)
        const char* throwType;    // "throw", "jump_throw", "run_throw"
        const char* desc;         // short tip
    };

    // ---------------------------------------------------------------
    // Lineup database â€” popular competitive lineups
    // ---------------------------------------------------------------
    inline const Lineup kLineups[] = {
        // ===== de_dust2 =====
        // T side
        { "Xbox Smoke",          "de_dust2", "smoke",   -534, 1232, -119,  -50.1f, 126.9f, "jump_throw", "Stand corner, aim sky, jump throw" },
        { "CT Spawn Smoke",      "de_dust2", "smoke",   -400, 1726, -119,  -42.5f, 68.5f,  "throw",      "From T spawn tunnel" },
        { "A Cross Smoke",       "de_dust2", "smoke",   1250, 388, 32,     -9.5f,  -101.4f,"throw",      "Block CT crossing to A" },
        { "B Door Smoke",        "de_dust2", "smoke",   -1377, 2609, 35,   -3.8f,  -131.2f,"throw",      "Stand B tunnel, aim above door" },
        { "B Window Smoke",      "de_dust2", "smoke",   -1618, 2528, 68,   -48.0f, -91.0f, "jump_throw", "Block window from upper tunnel" },
        { "A Long Molotov",      "de_dust2", "molotov", 1300, 744, 3,      -1.0f,  -100.5f,"throw",      "Molly car from A long" },
        // CT side
        { "T Spawn Long Flash",  "de_dust2", "flash",   1734, 544, 3,      -55.0f, -144.0f,"throw",      "Pop flash over doors" },
        { "Lower Tunnel Molotov","de_dust2", "molotov", -1780, 2860, 64,   -4.0f,  -40.0f, "throw",      "Clear lower dark" },

        // ===== de_mirage =====
        // T side
        { "A Site Smoke",        "de_mirage", "smoke",  -1824, 560, -168,  -64.5f, -170.5f,"jump_throw", "Stand T ramp, aim roof edge" },
        { "CT Smoke (A)",        "de_mirage", "smoke",  -1824, 560, -168,  -56.0f, 157.2f, "jump_throw", "Block CT rotation" },
        { "Jungle Smoke",        "de_mirage", "smoke",  -1550, 810, -100,  -57.0f, -172.0f,"jump_throw", "Cover jungle from T" },
        { "Stairs Smoke",        "de_mirage", "smoke",  -1413, -463, -104, -64.0f, -106.0f,"jump_throw", "Block stairs connector" },
        { "Window Smoke",        "de_mirage", "smoke",  -2070, -342, -104, -46.5f, -100.0f,"throw",      "Smoke off window sniper" },
        { "B Short Smoke",       "de_mirage", "smoke",  -417, -978, -184,  -48.5f, -156.8f,"jump_throw", "Smoke short from apps" },
        { "B Site Molotov",      "de_mirage", "molotov",-307, -2000, -168, -28.0f, -72.0f, "throw",      "Clear van/bench" },
        // CT side
        { "A Ramp Flash",        "de_mirage", "flash",  -150, 310, -106,   -58.0f, 140.0f, "throw",      "Pop flash T ramp" },

        // ===== de_inferno =====
        // T side
        { "Coffin Smoke",        "de_inferno", "smoke",  232, 486, 66,     -60.0f, 132.0f, "jump_throw", "Smoke coffins/graveyard" },
        { "CT Smoke (B)",        "de_inferno", "smoke", -1538, 338, 104,   -62.5f, 150.0f, "jump_throw", "Block CT from B" },
        { "A Long Smoke",        "de_inferno", "smoke",  -48, 1436, 128,  -20.0f, -82.0f, "throw",       "Smoke arch/library from T" },
        { "Banana Molotov",      "de_inferno", "molotov",-1090, 2108, 112, -4.0f,  -20.0f, "throw",       "Clear car hold" },
        // CT side
        { "Top Banana Smoke",    "de_inferno", "smoke", -2170, 450, 128,   -36.0f, 46.0f,  "throw",      "Delay banana push" },

        // ===== de_anubis =====
        { "A Main Smoke",        "de_anubis", "smoke",  -1370, 1020, -64,  -50.0f, -78.0f, "jump_throw", "Smoke main entrance" },
        { "B Main Smoke",        "de_anubis", "smoke",   836, 260, 0,      -60.0f, 120.0f, "jump_throw", "Block B main" },

        // ===== de_nuke =====
        { "Outside Smoke",       "de_nuke", "smoke",     1590, -1000, -416, -38.0f, -130.0f, "throw",    "Smoke outside silo" },
        { "A Hut Smoke",         "de_nuke", "smoke",     620, -900, -416,  -55.0f, -170.0f, "jump_throw","Cover hut from lobby" },

        // ===== de_ancient =====
        { "A Main Smoke",        "de_ancient", "smoke", -500, 900, 0,      -60.0f, -90.0f,  "jump_throw","Smoke main A" },
        { "B Ramp Smoke",        "de_ancient", "smoke",  700, 200, 64,     -52.0f, 135.0f,  "jump_throw","Cover B ramp" },
    };
    inline constexpr int kLineupCount = sizeof(kLineups) / sizeof(kLineups[0]);

    // ---------------------------------------------------------------
    // Map detection â€” read current map name from CGlobalVars
    // ---------------------------------------------------------------
    inline std::string GetCurrentMap()
    {
        uintptr_t gv = Mem::Read<uintptr_t>(
            GameState::clientBase + GameState::RVA_dwGlobalVars());
        if (!gv) return "";

        // CGlobalVarsBase::m_pMapName is a const char* at offset 0x188
        uintptr_t namePtr = Mem::Read<uintptr_t>(gv + 0x188);
        if (!namePtr) return "";

        struct Buf { char d[64]; };
        Buf b = Mem::Read<Buf>(namePtr);
        b.d[63] = '\0';
        return std::string(b.d);
    }

    inline ImU32 GetNadeColor(const char* type)
    {
        if (strcmp(type, "smoke") == 0)   return IM_COL32(180, 180, 180, 220);
        if (strcmp(type, "molotov") == 0) return IM_COL32(255, 120, 30, 220);
        if (strcmp(type, "flash") == 0)   return IM_COL32(255, 255, 80, 220);
        if (strcmp(type, "he") == 0)      return IM_COL32(255, 60, 40, 220);
        return IM_COL32(200, 200, 200, 220);
    }

    // ---------------------------------------------------------------
    // Render â€” shows nearby lineups when player stands near spot
    // ---------------------------------------------------------------
    inline void Render()
    {
        if (!cfg.enabled) return;
        if (!GameState::clientBase) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        Math::Vec3 myPos = GameState::GetEntityOrigin(localPawn);
        if (myPos.IsZero()) return;

        // Get current map
        std::string mapFull = GetCurrentMap();
        if (mapFull.empty()) return;

        // Normalize: strip path prefix (maps/xxx/xxx or just xxx)
        std::string mapName = mapFull;
        {
            size_t slash = mapName.rfind('/');
            if (slash != std::string::npos) mapName = mapName.substr(slash + 1);
            slash = mapName.rfind('\\');
            if (slash != std::string::npos) mapName = mapName.substr(slash + 1);
        }

        // Read view angles for aim comparison
        float viewPitch = 0.f, viewYaw = 0.f;
        {
            uintptr_t vaAddr = GameState::clientBase + GameState::RVA_dwViewAngles();
            viewPitch = Mem::Read<float>(vaAddr);
            viewYaw   = Mem::Read<float>(vaAddr + 4);
        }

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 disp = ImGui::GetIO().DisplaySize;
        float scrW = disp.x, scrH = disp.y;
        float trigDist = cfg.triggerDist;

        int panelCount = 0;

        for (int i = 0; i < kLineupCount; ++i)
        {
            const Lineup& L = kLineups[i];

            // Map filter
            if (mapName.find(L.map) == std::string::npos &&
                mapFull.find(L.map) == std::string::npos) continue;

            // Distance check
            float dx = myPos.x - L.standX;
            float dy = myPos.y - L.standY;
            float dz = myPos.z - L.standZ;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist > trigDist) continue;

            // This lineup is active â€” render guide

            // Calculate aim delta
            auto NormAngle = [](float a) -> float {
                while (a > 180.f) a -= 360.f;
                while (a < -180.f) a += 360.f;
                return a;
            };
            float dPitch = NormAngle(viewPitch - L.aimPitch);
            float dYaw   = NormAngle(viewYaw - L.aimYaw);
            float aimDelta = sqrtf(dPitch * dPitch + dYaw * dYaw);
            bool onTarget = (aimDelta <= cfg.aimTolerance);

            ImU32 nadeCol = GetNadeColor(L.type);

            // Draw world marker at stand position
            float sp[3] = { L.standX, L.standY, L.standZ + 10.f };
            float sx, sy;
            if (GameState::WorldToScreen(sp, sx, sy, scrW, scrH))
            {
                // Circle marker on ground
                ImU32 circleCol = onTarget ? IM_COL32(80, 255, 80, 200)
                                          : IM_COL32(255, 255, 255, 120);
                dl->AddCircleFilled(ImVec2(sx, sy), 8.f, IM_COL32(0, 0, 0, 150), 16);
                dl->AddCircle(ImVec2(sx, sy), 8.f, circleCol, 16, 2.f);
                // Small nade type icon
                dl->AddCircleFilled(ImVec2(sx, sy), 4.f, nadeCol, 12);
            }

            // HUD panel â€” aim guide
            float panelW = 220.f;
            float panelH = 70.f;
            float panelX = scrW * 0.5f - panelW * 0.5f;
            float panelY = scrH * 0.72f + panelCount * (panelH + 8.f);

            // Background
            dl->AddRectFilled(ImVec2(panelX, panelY),
                              ImVec2(panelX + panelW, panelY + panelH),
                              IM_COL32(8, 8, 12, 200), 6.f);
            dl->AddRect(ImVec2(panelX, panelY),
                        ImVec2(panelX + panelW, panelY + panelH),
                        nadeCol & 0x40FFFFFF, 6.f, 0, 1.f);

            // Nade type + name
            ImFont* font = ImGui::GetFont();
            float fs = 13.f;
            char header[64];
            snprintf(header, sizeof(header), "[%s] %s", L.type, L.name);
            ImVec2 hsz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, header);
            dl->AddText(font, fs, ImVec2(panelX + 8, panelY + 4), nadeCol, header);

            // Throw type
            char throwLine[32];
            snprintf(throwLine, sizeof(throwLine), "%s", L.throwType);
            dl->AddText(font, 11.f, ImVec2(panelX + 8, panelY + 22),
                        IM_COL32(180, 180, 180, 220), throwLine);

            // Aim direction indicator
            ImU32 aimCol;
            char aimTxt[64];
            if (onTarget)
            {
                aimCol = IM_COL32(80, 255, 80, 255);
                snprintf(aimTxt, sizeof(aimTxt), "ON TARGET");
            }
            else
            {
                aimCol = IM_COL32(255, 200, 80, 255);
                // Show direction hints
                const char* vDir = (dPitch > 0.5f) ? "DOWN" : (dPitch < -0.5f) ? "UP" : "";
                const char* hDir = (dYaw > 0.5f) ? "LEFT" : (dYaw < -0.5f) ? "RIGHT" : "";
                snprintf(aimTxt, sizeof(aimTxt), "Aim: %s %s (%.1fÂ°)", vDir, hDir, aimDelta);
            }
            dl->AddText(font, 12.f, ImVec2(panelX + 8, panelY + 38), aimCol, aimTxt);

            // Tip
            if (L.desc && L.desc[0])
            {
                dl->AddText(font, 10.f, ImVec2(panelX + 8, panelY + 54),
                            IM_COL32(140, 140, 140, 200), L.desc);
            }

            // Crosshair aim point â€” draw a small diamond at the target pitch/yaw
            // Convert target angles to a screen-ish position
            // (exact would require inverse viewmatrix â€” approximate with delta)
            if (!onTarget)
            {
                // Draw arrow at crosshair pointing toward target
                float cx = scrW * 0.5f, cy = scrH * 0.5f;
                float arrAngle = atan2f(-dPitch, -dYaw);
                float arrLen = 30.f;
                float ax = cx + cosf(arrAngle) * arrLen;
                float ay = cy + sinf(arrAngle) * arrLen;
                dl->AddLine(ImVec2(cx + cosf(arrAngle) * 15.f, cy + sinf(arrAngle) * 15.f),
                            ImVec2(ax, ay), aimCol, 2.f);
                // Arrow head
                float headAngle = 0.4f;
                float headLen = 8.f;
                dl->AddLine(ImVec2(ax, ay),
                    ImVec2(ax - cosf(arrAngle - headAngle) * headLen,
                           ay - sinf(arrAngle - headAngle) * headLen), aimCol, 2.f);
                dl->AddLine(ImVec2(ax, ay),
                    ImVec2(ax - cosf(arrAngle + headAngle) * headLen,
                           ay - sinf(arrAngle + headAngle) * headLen), aimCol, 2.f);
            }
            else
            {
                // Green ring around crosshair
                float cx = scrW * 0.5f, cy = scrH * 0.5f;
                dl->AddCircle(ImVec2(cx, cy), 20.f, IM_COL32(80, 255, 80, 180), 24, 2.f);
            }

            panelCount++;
        }
    }
}
