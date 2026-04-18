#pragma once

// ---------------------------------------------------------------
// Rank Revealer — reads competitive ranking data from
// CCSPlayerController and renders it on the scoreboard overlay.
// Shows CS Rating (ELO number) and win count for each player.
// ---------------------------------------------------------------

#include <cstdint>
#include <cstdio>
#include "../core/memory.h"
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../vendor/imgui/imgui.h"

namespace RankRevealer
{
    struct Config
    {
        bool enabled = false;
    };
    inline Config cfg;

    // CCSPlayerController offsets (from CCSPlayerController_ActionTrackingServices parent)
    namespace Off
    {
        constexpr std::ptrdiff_t m_iCompetitiveRanking    = 0x880;
        constexpr std::ptrdiff_t m_iCompetitiveWins       = 0x884;
        constexpr std::ptrdiff_t m_iCompetitiveRankType   = 0x888;
    }

    struct PlayerRank
    {
        int32_t rating   = 0;
        int32_t wins     = 0;
        int8_t  rankType = 0;
        bool    valid    = false;
    };

    // CS2 Premier rank tiers (ELO-based)
    inline const char* GetPremierTier(int32_t rating)
    {
        if (rating <= 0)     return "";
        if (rating < 5000)   return "Common";
        if (rating < 10000)  return "Uncommon";
        if (rating < 15000)  return "Rare";
        if (rating < 20000)  return "Mythical";
        if (rating < 25000)  return "Legendary";
        if (rating < 30000)  return "Ancient";
        if (rating < 35000)  return "Epic";
        return "Supreme";
    }

    inline ImU32 GetTierColor(int32_t rating)
    {
        if (rating <= 0)     return IM_COL32(150, 150, 150, 255);
        if (rating < 5000)   return IM_COL32(180, 180, 180, 255); // grey
        if (rating < 10000)  return IM_COL32(100, 200, 100, 255); // green
        if (rating < 15000)  return IM_COL32(80, 150, 255, 255);  // blue
        if (rating < 20000)  return IM_COL32(180, 80, 255, 255);  // purple
        if (rating < 25000)  return IM_COL32(255, 60, 180, 255);  // pink
        if (rating < 30000)  return IM_COL32(255, 180, 40, 255);  // gold
        if (rating < 35000)  return IM_COL32(255, 40, 40, 255);   // red
        return IM_COL32(255, 220, 80, 255);                       // bright gold
    }

    inline PlayerRank GetRank(uintptr_t controller)
    {
        PlayerRank r;
        if (!controller) return r;
        r.rating   = Mem::Read<int32_t>(controller + Off::m_iCompetitiveRanking);
        r.wins     = Mem::Read<int32_t>(controller + Off::m_iCompetitiveWins);
        r.rankType = Mem::Read<int8_t>(controller + Off::m_iCompetitiveRankType);
        r.valid    = (r.rating > 0 || r.wins > 0);
        return r;
    }

    // Renders a compact rank panel in the overlay.
    // Called from ESP render loop if enabled.
    inline void RenderPanel()
    {
        if (!cfg.enabled) return;
        if (!GameState::clientBase) return;

        uintptr_t localCtrl = GameState::GetLocalController();
        uintptr_t entList   = GameState::GetEntityList();
        if (!localCtrl || !entList) return;

        int localTeam = 0;
        {
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (localPawn)
                localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
        }

        struct Entry
        {
            char     name[32];
            int32_t  rating;
            int32_t  wins;
            int      team;
            bool     alive;
        };
        Entry entries[64];
        int count = 0;

        for (int i = 1; i <= 64 && count < 64; ++i)
        {
            __try {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ctrl = Mem::Read<uintptr_t>(chunk + 0x70 * (i & 0x1FF));
                if (!ctrl) continue;

                PlayerRank rk = GetRank(ctrl);
                if (!rk.valid) continue;

                // Get name
                uintptr_t np = Mem::Read<uintptr_t>(ctrl + Offsets::m_sSanitizedPlayerName);
                if (!np) continue;
                struct NB { char d[32]; };
                NB nb = Mem::Read<NB>(np); nb.d[31] = '\0';
                if (!nb.d[0]) continue;

                // Get team
                uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                uintptr_t pawn = GameState::ResolveHandle(pH);
                int team = 0;
                bool alive = Mem::Read<bool>(ctrl + Offsets::m_bPawnIsAlive);
                if (pawn) team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);

                Entry& e = entries[count++];
                memcpy(e.name, nb.d, 32);
                e.rating = rk.rating;
                e.wins   = rk.wins;
                e.team   = team;
                e.alive  = alive;
            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }

        if (count == 0) return;

        // Draw panel
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 disp = ImGui::GetIO().DisplaySize;

        float lineH = 20.f;
        float pw = 280.f;
        float ph = 30.f + count * lineH + 8.f;
        float px = disp.x * 0.5f - pw * 0.5f;
        float py = disp.y * 0.12f;

        // Background
        dl->AddRectFilled(ImVec2(px, py), ImVec2(px + pw, py + ph),
                          IM_COL32(8, 8, 12, 210), 6.f);
        dl->AddRect(ImVec2(px, py), ImVec2(px + pw, py + ph),
                    IM_COL32(255, 255, 255, 25), 6.f, 0, 1.f);

        // Header
        ImFont* font = ImGui::GetFont();
        float fs = 12.f;

        auto DrawText = [&](float x, float y, ImU32 col, const char* txt, bool center = false) {
            ImVec2 sz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, txt);
            if (center) x -= sz.x * 0.5f;
            dl->AddText(font, fs, ImVec2(x, y), col, txt);
        };

        DrawText(px + pw * 0.5f, py + 6, IM_COL32(255, 255, 255, 200), "RANKS", true);

        float ty = py + 28.f;
        for (int i = 0; i < count; ++i)
        {
            const Entry& e = entries[i];
            ImU32 teamCol = (e.team == localTeam)
                ? IM_COL32(100, 180, 255, 200)
                : IM_COL32(255, 100, 80, 200);
            if (!e.alive)
                teamCol = (teamCol & 0x00FFFFFF) | (100 << 24); // dimmed

            // Name
            DrawText(px + 8, ty, teamCol, e.name);

            // Rating
            char rBuf[32];
            ImU32 rCol = GetTierColor(e.rating);
            if (e.rating > 0)
                snprintf(rBuf, sizeof(rBuf), "%d", e.rating);
            else
                snprintf(rBuf, sizeof(rBuf), "---");
            ImVec2 rsz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, rBuf);
            DrawText(px + pw - 80 - rsz.x, ty, rCol, rBuf);

            // Wins
            char wBuf[16];
            snprintf(wBuf, sizeof(wBuf), "%dW", e.wins);
            ImVec2 wsz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, wBuf);
            DrawText(px + pw - 8 - wsz.x, ty, IM_COL32(180, 180, 180, 180), wBuf);

            ty += lineH;
        }
    }
}
