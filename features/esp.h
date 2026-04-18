#pragma once

// ---------------------------------------------------------------
// ESP — Modern overlay rendering with clean minimal aesthetics.
// Soft corners, gradient health bars, pill-shaped info tags.
// ---------------------------------------------------------------

#include <Windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/math.h"
#include "../core/memory.h"
#include "../core/stealth.h"
#include "../vendor/imgui/imgui.h"
#include "weapon_icons.h"

// Forward declaration for weapon icon font - now global like unix.solutions
extern ImFont* iconFont;

namespace ESP
{
    struct Config
    {
        bool  enabled     = true;  // ENABLED by default so debug output shows
        bool  box         = true;
        int   boxStyle    = 1;  // 0 = full, 1 = corners
        bool  skeleton    = true;
        bool  healthBar   = true;
        bool  name        = true;
        bool  distance    = true;
        bool  weapon      = true;
        bool  weaponIcon  = true;  // Toggle for icon vs text
        bool  teamCheck   = true;
        float boxColor[4]      = { 1.f, 1.f, 1.f, 0.7f };
        float skeletonColor[4] = { 0.85f, 0.85f, 0.85f, 0.6f };
        bool  bombTimer      = true;   // Re-enabled with proper checks
        bool  spectators     = false;
        int   bombTimerStyle = 0;   // 0=Classic  1=Vivid  2=Compact
        int   spectatorStyle = 0;   // 0=Classic  1=Stealth  2=Minimal

        // Visible/hidden color system
        bool  visColorEnabled = true;
        float visibleColor[4] = { 0.2f, 1.0f, 0.2f, 0.8f };  // green when visible
        float hiddenColor[4]  = { 1.0f, 0.2f, 0.2f, 0.8f };  // red when hidden
    };

    inline Config cfg;
    inline bool   previewMode = false;   // true when menu open → show preview panels

    // ---------------------------------------------------------------
    // Modern drawing primitives — clean lines, soft edges
    // ---------------------------------------------------------------
    namespace Draw
    {
        inline void Text(ImDrawList* dl, ImVec2 pos, ImU32 col, const char* txt,
                         bool centered = false, float size = 0.f)
        {
            ImFont* font = ImGui::GetFont();
            float fs = size > 0.f ? size : ImGui::GetFontSize();
            ImVec2 sz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, txt);
            if (centered) pos.x -= sz.x * 0.5f;
            // 8-directional thick outline — pops hard like premium internals
            ImU32 outl = IM_COL32(0, 0, 0, 255);
            constexpr float o = 1.f;  // outline offset
            dl->AddText(font, fs, ImVec2(pos.x - o, pos.y - o), outl, txt);
            dl->AddText(font, fs, ImVec2(pos.x,     pos.y - o), outl, txt);
            dl->AddText(font, fs, ImVec2(pos.x + o, pos.y - o), outl, txt);
            dl->AddText(font, fs, ImVec2(pos.x - o, pos.y),     outl, txt);
            dl->AddText(font, fs, ImVec2(pos.x + o, pos.y),     outl, txt);
            dl->AddText(font, fs, ImVec2(pos.x - o, pos.y + o), outl, txt);
            dl->AddText(font, fs, ImVec2(pos.x,     pos.y + o), outl, txt);
            dl->AddText(font, fs, ImVec2(pos.x + o, pos.y + o), outl, txt);
            dl->AddText(font, fs, pos, col, txt);
        }

        inline void CornerBox(ImDrawList* dl, float x, float y, float w, float h,
                              ImU32 col, float cornerLen = 0.22f, float thick = 2.0f)
        {
            float lw = w * cornerLen, lh = h * cornerLen;
            float x2 = x + w, y2 = y + h;
            // Full black outline behind every corner line
            ImU32 outl = IM_COL32(0, 0, 0, 255);
            float ot = thick + 2.f; // outline thickness
            dl->AddLine({x,y},{x+lw,y},outl,ot);     dl->AddLine({x,y},{x,y+lh},outl,ot);
            dl->AddLine({x2-lw,y},{x2,y},outl,ot);   dl->AddLine({x2,y},{x2,y+lh},outl,ot);
            dl->AddLine({x,y2-lh},{x,y2},outl,ot);   dl->AddLine({x,y2},{x+lw,y2},outl,ot);
            dl->AddLine({x2-lw,y2},{x2,y2},outl,ot); dl->AddLine({x2,y2-lh},{x2,y2},outl,ot);
            // Inner colored lines
            dl->AddLine({x,y},{x+lw,y},col,thick);
            dl->AddLine({x,y},{x,y+lh},col,thick);
            dl->AddLine({x2-lw,y},{x2,y},col,thick);
            dl->AddLine({x2,y},{x2,y+lh},col,thick);
            dl->AddLine({x,y2-lh},{x,y2},col,thick);
            dl->AddLine({x,y2},{x+lw,y2},col,thick);
            dl->AddLine({x2-lw,y2},{x2,y2},col,thick);
            dl->AddLine({x2,y2-lh},{x2,y2},col,thick);
        }

        inline void FullBox(ImDrawList* dl, float x, float y, float w, float h,
                            ImU32 col, float thick = 1.4f)
        {
            dl->AddRect(ImVec2(x-1,y-1), ImVec2(x+w+1,y+h+1), IM_COL32(0,0,0,255), 0.f, 0, thick+2.f);
            dl->AddRect(ImVec2(x,y), ImVec2(x+w,y+h), col, 0.f, 0, thick);
        }

        inline void HealthBar(ImDrawList* dl, float x, float y, float h,
                              int hp, float barW = 4.5f, float gap = 6.f)
        {
            float frac = (float)hp / 100.f;
            if (frac > 1.f) frac = 1.f;
            float barH = h * frac;
            float bx = x - gap - barW;
            float rnd = 2.5f;

            // Outer dark backdrop with subtle glow
            dl->AddRectFilled(ImVec2(bx - 1.5f, y - 1.5f),
                              ImVec2(bx + barW + 1.5f, y + h + 1.5f),
                              IM_COL32(0, 0, 0, 180), rnd + 1.f);

            // Inner glass trough — translucent dark with faint border
            dl->AddRectFilled(ImVec2(bx, y), ImVec2(bx + barW, y + h),
                              IM_COL32(12, 14, 20, 160), rnd);
            dl->AddRect(ImVec2(bx, y), ImVec2(bx + barW, y + h),
                        IM_COL32(255, 255, 255, 18), rnd, 0, 0.8f);

            // Health colour gradient — brighter green→yellow→red
            float cr, cg;
            if (hp > 50) { float t = (hp - 50) / 50.f; cr = 1.f - t; cg = 0.55f + t * 0.45f; }
            else { float t = hp / 50.f; cr = 1.f; cg = t * 0.55f; }

            ImU32 topC = IM_COL32((int)(cr * 255), (int)(cg * 255), 15, 230);
            ImU32 botC = IM_COL32((int)(cr * 200), (int)(cg * 200), 5, 200);

            float fillTop = y + (h - barH);
            // Main health fill
            dl->AddRectFilled(ImVec2(bx + 0.5f, fillTop),
                              ImVec2(bx + barW - 0.5f, y + h - 0.5f),
                              botC, rnd - 0.5f);
            dl->AddRectFilledMultiColor(ImVec2(bx + 0.5f, fillTop),
                                        ImVec2(bx + barW - 0.5f, y + h - 0.5f),
                                        topC, topC, botC, botC);

            // Liquid-glass specular highlight — bright strip on left 40%
            if (barH > 4.f)
            {
                float hlW = barW * 0.38f;
                dl->AddRectFilled(
                    ImVec2(bx + 1.f, fillTop + 1.f),
                    ImVec2(bx + 1.f + hlW, y + h - 1.f),
                    IM_COL32(255, 255, 255, 55), rnd - 1.f);
            }

            // Top gloss arc — small bright line at very top of fill
            if (barH > 2.f)
            {
                dl->AddLine(ImVec2(bx + 1.5f, fillTop + 0.8f),
                            ImVec2(bx + barW - 1.5f, fillTop + 0.8f),
                            IM_COL32(255, 255, 255, 70), 1.f);
            }

            // HP text when not full
            if (hp < 100) {
                char buf[8]; snprintf(buf, sizeof(buf), "%d", hp);
                ImVec2 sz = ImGui::CalcTextSize(buf);
                Text(dl, ImVec2(bx + barW * 0.5f, fillTop - sz.y - 2), IM_COL32(255, 255, 255, 210), buf, true, 9.f);
            }
        }

        inline void BombPanel(ImDrawList* dl, float scrW, float remaining,
                              bool defusing, float defuseRem, int style = 0)
        {
            char buf[32];
            bool urgent = remaining < 10.f;
            ImU32 tc = urgent ? IM_COL32(255, 60, 50, 255) : IM_COL32(255, 255, 255, 230);

            if (style == 1) // Vivid — glowing accent border, more prominent
            {
                ImU32 glowC = urgent ? IM_COL32(255, 40, 20, 45) : IM_COL32(60, 180, 100, 28);
                ImU32 lineC = urgent ? IM_COL32(255, 70, 50, 255) : IM_COL32(70, 210, 130, 255);
                float pw = 190.f, ph = 44.f;
                float px = scrW * 0.5f - pw * 0.5f, py = 62.f;
                dl->AddRectFilled(ImVec2(px-6, py-6), ImVec2(px+pw+6, py+ph+6), glowC, 10.f);
                dl->AddRectFilled(ImVec2(px, py), ImVec2(px+pw, py+ph), IM_COL32(8, 10, 14, 235), 7.f);
                dl->AddRect(ImVec2(px, py), ImVec2(px+pw, py+ph), lineC, 7.f, 0, 1.3f);
                dl->AddRectFilled(ImVec2(px+2.f, py+8.f), ImVec2(px+4.f, py+ph-8.f), lineC, 2.f);
                snprintf(buf, sizeof(buf), "C4  %.1fs", remaining);
                Text(dl, ImVec2(px+pw*0.5f, py+6), tc, buf, true, 16.f);
                float bx=px+10.f, by2=py+29.f, bw=pw-20.f, bh=4.f;
                float frac = remaining / 40.f;
                if (frac > 1.f) frac = 1.f; if (frac < 0.f) frac = 0.f;
                dl->AddRectFilled(ImVec2(bx, by2), ImVec2(bx+bw, by2+bh), IM_COL32(30,32,40,200), 2.f);
                dl->AddRectFilled(ImVec2(bx, by2), ImVec2(bx+bw*frac, by2+bh), lineC, 2.f);
                if (defusing && defuseRem > 0.f) {
                    snprintf(buf, sizeof(buf), "DEFUSE %.1fs", defuseRem);
                    Text(dl, ImVec2(px+pw*0.5f, py+ph+4), IM_COL32(100,180,255,255), buf, true, 11.f);
                }
            }
            else if (style == 2) // Compact — small badge, left edge
            {
                snprintf(buf, sizeof(buf), "C4 %.1fs", remaining);
                float pw = 110.f, ph = 24.f;
                float px = 14.f, py = 72.f;
                dl->AddRectFilled(ImVec2(px, py), ImVec2(px+pw, py+ph), IM_COL32(6,6,8,185), 4.f);
                dl->AddRect(ImVec2(px, py), ImVec2(px+pw, py+ph), tc, 4.f, 0, 0.8f);
                Text(dl, ImVec2(px+pw*0.5f, py+5), tc, buf, true, 12.f);
                if (defusing && defuseRem > 0.f) {
                    snprintf(buf, sizeof(buf), "DEF %.1fs", defuseRem);
                    Text(dl, ImVec2(px+pw*0.5f, py+ph+2), IM_COL32(100,180,255,255), buf, true, 10.f);
                }
            }
            else // Classic (style == 0) — centered minimal dark pill
            {
                float pw = 160.f, ph = 38.f;
                float px = scrW * 0.5f - pw * 0.5f, py = 70.f;
                dl->AddRectFilled(ImVec2(px, py), ImVec2(px+pw, py+ph), IM_COL32(12,12,12,210), 6.f);
                dl->AddRect(ImVec2(px, py), ImVec2(px+pw, py+ph), IM_COL32(255,255,255,35), 6.f, 0, 1.f);
                snprintf(buf, sizeof(buf), "C4  %.1fs", remaining);
                Text(dl, ImVec2(px+pw*0.5f, py+4), tc, buf, true, 15.f);
                float bx=px+10.f, by2=py+26.f, bw=pw-20.f, bh=3.f;
                float frac = remaining / 40.f;
                if (frac > 1.f) frac = 1.f; if (frac < 0.f) frac = 0.f;
                dl->AddRectFilled(ImVec2(bx, by2), ImVec2(bx+bw, by2+bh), IM_COL32(40,40,40,180), 1.5f);
                dl->AddRectFilled(ImVec2(bx, by2), ImVec2(bx+bw*frac, by2+bh), tc, 1.5f);
                if (defusing && defuseRem > 0.f) {
                    snprintf(buf, sizeof(buf), "DEFUSE %.1fs", defuseRem);
                    Text(dl, ImVec2(px+pw*0.5f, py+ph+3), IM_COL32(100,180,255,255), buf, true, 11.f);
                }
            }
        }

        inline void SpectatorPanel(ImDrawList* dl, float scrW, const std::vector<std::string>& names, int style = 0)
        {
            if (names.empty()) return;
            float pw = 180.f, lh = 18.f;
            float ph = 28.f + (float)names.size() * lh;
            float px = scrW - pw - 15.f, py = 80.f;

            if (style == 1) // Stealth — dark navy, menu accent border
            {
                dl->AddRectFilled(ImVec2(px,py), ImVec2(px+pw,py+ph), IM_COL32(13,13,18,215), 6.f);
                dl->AddRect(ImVec2(px,py), ImVec2(px+pw,py+ph), IM_COL32(42,40,62,200), 6.f, 0, 1.f);
                dl->AddRectFilled(ImVec2(px+2.f,py+7.f), ImVec2(px+4.f,py+ph-7.f), IM_COL32(142,132,255,200), 2.f);
                Text(dl, ImVec2(px+pw*0.5f, py+5), IM_COL32(142,132,255,255), "SPECTATING", true, 11.f);
                float ty = py + 24.f;
                for (const auto& n : names) { Text(dl, ImVec2(px+10, ty), IM_COL32(200,200,220,240), n.c_str()); ty += lh; }
            }
            else if (style == 2) // Minimal — barely-there translucent chip
            {
                dl->AddRectFilled(ImVec2(px,py), ImVec2(px+pw,py+ph), IM_COL32(4,4,6,155), 4.f);
                Text(dl, ImVec2(px+pw*0.5f, py+5), IM_COL32(165,165,175,200), "SPECTATORS", true, 10.f);
                float ty = py + 22.f;
                for (const auto& n : names) { Text(dl, ImVec2(px+8, ty), IM_COL32(200,200,200,185), n.c_str()); ty += lh; }
            }
            else // Classic (style == 0) — red-accent dark panel
            {
                dl->AddRectFilled(ImVec2(px,py), ImVec2(px+pw,py+ph), IM_COL32(10,10,10,200), 6.f);
                dl->AddRect(ImVec2(px,py), ImVec2(px+pw,py+ph), IM_COL32(255,70,70,120), 6.f, 0, 1.f);
                Text(dl, ImVec2(px+pw*0.5f, py+5), IM_COL32(255,100,100,255), "SPECTATORS", true, 11.f);
                float ty = py + 24.f;
                for (const auto& n : names) { Text(dl, ImVec2(px+10, ty), IM_COL32(220,220,220,230), n.c_str()); ty += lh; }
            }
        }

        inline ImU32 WeaponAccent(uint16_t defIdx)
        {
            switch (defIdx)
            {
            case 1: case 2: case 3: case 4: case 30: case 32: case 36: case 61: case 63: case 64:
                return IM_COL32(255, 176, 64, 235);   // pistols
            case 7: case 8: case 10: case 13: case 16: case 39: case 60:
                return IM_COL32(48, 214, 255, 235);   // rifles
            case 9: case 11: case 38: case 40:
                return IM_COL32(255, 92, 92, 235);    // snipers / autosnipers
            case 17: case 19: case 23: case 24: case 26: case 33: case 34:
                return IM_COL32(182, 126, 255, 235);  // smgs
            case 14: case 25: case 27: case 28: case 29: case 35:
                return IM_COL32(118, 232, 156, 235);  // heavy / shotguns
            case 42: case 43: case 44: case 45: case 46: case 47: case 48:
                return IM_COL32(255, 130, 82, 235);   // nades / c4
            default:
                return IM_COL32(210, 216, 225, 225);
            }
        }

        inline const char* WeaponBadgeText(uint16_t defIdx)
        {
            switch (defIdx)
            {
            case 1: return "DEAGLE";
            case 2: return "DUALIES";
            case 3: return "57";
            case 4: return "GLOCK";
            case 7: return "AK-47";
            case 8: return "AUG";
            case 9: return "AWP";
            case 10: return "FAMAS";
            case 11: return "G3SG1";
            case 13: return "GALIL";
            case 14: return "M249";
            case 16: return "M4A4";
            case 17: return "MAC-10";
            case 19: return "P90";
            case 23: return "MP5";
            case 24: return "UMP-45";
            case 25: return "XM1014";
            case 26: return "BIZON";
            case 27: return "MAG-7";
            case 28: return "NEGEV";
            case 29: return "SAWED";
            case 30: return "TEC-9";
            case 31: return "ZEUS";
            case 32: return "P2000";
            case 33: return "MP7";
            case 34: return "MP9";
            case 35: return "NOVA";
            case 36: return "P250";
            case 38: return "SCAR-20";
            case 39: return "SG 553";
            case 40: return "SSG 08";
            case 41: case 42: case 59: return "KNIFE";
            case 43: return "FLASH";
            case 44: return "HE";
            case 45: return "SMOKE";
            case 46: return "MOLOTOV";
            case 47: return "DECOY";
            case 48: return "INCEND";
            case 60: return "M4A1-S";
            case 61: return "USP-S";
            case 63: return "CZ75";
            case 64: return "R8";
            default:
                if (defIdx >= 500 && defIdx < 600) return "KNIFE";
                return "?";
            }
        }

        inline void WeaponBadge(ImDrawList* dl, ImVec2 centerTop, uint16_t defIdx)
        {
            const char* tag = WeaponBadgeText(defIdx);
            if (!tag || !tag[0]) return;

            ImFont* font = ImGui::GetFont();
            const float fontSize = 11.f;
            ImVec2 textSz = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, tag);
            const float padX = 9.f;
            const float padY = 3.f;
            const float badgeW = textSz.x + padX * 2.f;
            const float badgeH = textSz.y + padY * 2.f;
            const float round = 5.f;

            ImVec2 badgeMin(centerTop.x - badgeW * 0.5f, centerTop.y);
            ImVec2 badgeMax(centerTop.x + badgeW * 0.5f, centerTop.y + badgeH);
            ImU32 accent = WeaponAccent(defIdx);

            dl->AddRectFilled(badgeMin, badgeMax, IM_COL32(8, 12, 18, 205), round);
            dl->AddRect(badgeMin, badgeMax, IM_COL32(255, 255, 255, 28), round, 0, 1.f);
            dl->AddRectFilled(ImVec2(badgeMin.x + 1.f, badgeMin.y + 1.f),
                              ImVec2(badgeMin.x + 3.5f, badgeMax.y - 1.f),
                              accent, round);
            Text(dl, ImVec2(centerTop.x, badgeMin.y + padY), IM_COL32(244, 248, 255, 255), tag, true, fontSize);
        }
    }

    // ---------------------------------------------------------------
    // Skeleton bones
    // ---------------------------------------------------------------
    struct BonePair { int a, b; };
    inline const BonePair kSkeleton[] = {
        {6,5},{5,4},{4,0},
        {5,8},{8,9},{9,10},
        {5,13},{13,14},{14,15},
        {0,22},{22,23},{23,24},
        {0,25},{25,26},{26,27}
    };

    // ---------------------------------------------------------------
    // Weapon definition-index to display name
    // ---------------------------------------------------------------
    inline const char* GetWeaponName(uint16_t defIdx)
    {
        switch (defIdx) {
            case 1: return "Deagle"; case 2: return "Dualies"; case 3: return "Five-SeveN";
            case 4: return "Glock"; case 7: return "AK-47"; case 8: return "AUG";
            case 9: return "AWP"; case 10: return "FAMAS"; case 11: return "G3SG1";
            case 13: return "Galil"; case 14: return "M249"; case 16: return "M4A4";
            case 17: return "MAC-10"; case 19: return "P90"; case 23: return "MP5-SD";
            case 24: return "UMP-45"; case 25: return "XM1014"; case 26: return "Bizon";
            case 27: return "MAG-7"; case 28: return "Negev"; case 29: return "Sawed-Off";
            case 30: return "Tec-9"; case 31: return "C4"; case 32: return "P2000";
            case 33: return "MP7"; case 34: return "MP9"; case 35: return "Nova";
            case 36: return "P250"; case 38: return "SCAR-20"; case 39: return "SG 553";
            case 40: return "SSG 08"; case 42: return "Knife"; case 59: return "Knife";
            case 60: return "M4A1-S"; case 61: return "USP-S"; case 63: return "CZ75";
            case 64: return "R8"; default: if (defIdx >= 500 && defIdx < 600) return "Knife";
            return "?";
        }
    }

    // ---------------------------------------------------------------
    // Visibility check — uses spotted state for ESP color switching
    // ---------------------------------------------------------------
    inline bool IsSpotted(uintptr_t pawn)
    {
        return Mem::Read<uint8_t>(pawn + Offsets::m_entitySpottedState + 0x08) != 0;
    }

    // ---------------------------------------------------------------
    // Main render — clean, optimized, no debug overlays
    // ---------------------------------------------------------------
    inline void Render()
    {
        if (!cfg.enabled) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        ImVec2 disp = ImGui::GetIO().DisplaySize;
        float scrW = disp.x, scrH = disp.y;

        // ---- Preview overlays — shown in lobby when menu is open ----
        bool inGame = GameState::clientBase && GameState::GetLocalPawn();
        if (previewMode && !inGame)
        {
            // Bomb timer preview disabled - causes crashes
            // if (cfg.bombTimer)
            //     Draw::BombPanel(dl, scrW, 27.3f, false, 0.f, cfg.bombTimerStyle);
            if (cfg.spectators)
            {
                static const std::vector<std::string> fakeSpec{ "Ghost.1", "Shadow_2", "Watcher3" };
                Draw::SpectatorPanel(dl, scrW, fakeSpec, cfg.spectatorStyle);
            }
        }

        if (!GameState::clientBase) return;

        // ---- C4 Bomb Timer - World-space display on planted bomb ----
        if (cfg.bombTimer)
        {
            uintptr_t bomb = 0;

            // Method A: dwPlantedC4 → pointer → entity pointer (raw pointer chain)
            uintptr_t bombGlobal = Mem::Read<uintptr_t>(
                GameState::clientBase + Offsets::Global::dwPlantedC4);
            if (bombGlobal > 0x10000)
            {
                uintptr_t candidate = Mem::Read<uintptr_t>(bombGlobal);
                if (candidate > 0x10000)
                    bomb = candidate;
            }

            // Method B: dwPlantedC4 is CNetworkUtlVectorBase<CEntityHandle>
            if (!bomb && bombGlobal)
            {
                int32_t count = Mem::Read<int32_t>(
                    GameState::clientBase + Offsets::Global::dwPlantedC4);
                uintptr_t dataPtr = Mem::Read<uintptr_t>(
                    GameState::clientBase + Offsets::Global::dwPlantedC4 + 0x8);
                if (count > 0 && count < 8 && dataPtr > 0x10000)
                {
                    uint32_t handle = Mem::Read<uint32_t>(dataPtr);
                    if (handle && handle != 0xFFFFFFFF)
                    {
                        uintptr_t resolved = GameState::ResolveHandle(handle);
                        if (resolved > 0x10000)
                            bomb = resolved;
                    }
                }
            }

            if (bomb)
            {
                bool ticking = Mem::Read<bool>(bomb + Offsets::m_bBombTicking);
                bool defused = Mem::Read<bool>(bomb + Offsets::m_bBombDefused);
                if (ticking && !defused)
                {
                    uintptr_t localPawnBomb = GameState::GetLocalPawn();
                    if (localPawnBomb)
                    {
                        float curTime = Mem::Read<float>(localPawnBomb + Offsets::m_flSimulationTime);
                        if (curTime <= 0.1f)
                            curTime = GameState::GetGameTime();

                        float blowTime = Mem::Read<float>(bomb + Offsets::m_flC4Blow);
                        float remaining = blowTime - curTime;
                        if (remaining > 0.f && remaining < 60.f)
                        {
                            float defEnd = Mem::Read<float>(bomb + Offsets::m_flDefuseCountDown);
                            bool isDef = defEnd > curTime;
                            float defRem = isDef ? defEnd - curTime : 0.f;

                            // World-space bomb display - clean pill with progress bar
                            uintptr_t bNode = Mem::Read<uintptr_t>(bomb + Offsets::m_pGameSceneNode);
                            if (bNode)
                            {
                                Math::Vec3 bOrg = Mem::Read<Math::Vec3>(bNode + Offsets::m_vecAbsOrigin);
                                float bx, by, wp[3] = { bOrg.x, bOrg.y, bOrg.z + 12.f };
                                if (GameState::WorldToScreen(wp, bx, by, scrW, scrH))
                                {
                                    bool urgent = remaining < 10.f;
                                    bool critical = remaining < 5.f;
                                    
                                    // Color coordination based on time
                                    ImU32 timerCol;
                                    ImU32 barCol;
                                    if (critical) {
                                        timerCol = IM_COL32(255, 50, 50, 255);
                                        barCol = IM_COL32(255, 30, 30, 255);
                                    } else if (urgent) {
                                        timerCol = IM_COL32(255, 140, 50, 255);
                                        barCol = IM_COL32(255, 120, 30, 255);
                                    } else {
                                        timerCol = IM_COL32(100, 220, 100, 255);
                                        barCol = IM_COL32(80, 200, 80, 255);
                                    }

                                    // Timer text
                                    char buf[32];
                                    snprintf(buf, sizeof(buf), "C4  %.1fs", remaining);
                                    ImVec2 tsz = ImGui::CalcTextSize(buf);
                                    float fontSize = 14.f;
                                    tsz.x *= (fontSize / ImGui::GetFontSize());
                                    tsz.y *= (fontSize / ImGui::GetFontSize());

                                    float padX = 10.f, padY = 5.f;
                                    float pw = tsz.x + padX * 2;
                                    float barH = 3.f;
                                    float ph = tsz.y + padY * 2 + barH + 4.f;

                                    // Defuse text if being defused
                                    char defBuf[32] = {};
                                    float defH = 0.f;
                                    if (isDef && defRem > 0.f)
                                    {
                                        snprintf(defBuf, sizeof(defBuf), "DEFUSE %.1fs", defRem);
                                        ImVec2 dsz = ImGui::CalcTextSize(defBuf);
                                        float defFontSize = 12.f;
                                        dsz.x *= (defFontSize / ImGui::GetFontSize());
                                        dsz.y *= (defFontSize / ImGui::GetFontSize());
                                        defH = dsz.y + 3.f;
                                        float dw = dsz.x + padX * 2;
                                        if (dw > pw) pw = dw;
                                        ph += defH;
                                    }

                                    float px = bx - pw * 0.5f;
                                    float py = by - ph - 8.f;

                                    // Background pill with glow
                                    dl->AddRectFilled(ImVec2(px - 2, py - 2), ImVec2(px + pw + 2, py + ph + 2),
                                                      IM_COL32(0, 0, 0, 100), 8.f);
                                    dl->AddRectFilled(ImVec2(px, py), ImVec2(px + pw, py + ph),
                                                      IM_COL32(10, 10, 12, 230), 6.f);
                                    dl->AddRect(ImVec2(px, py), ImVec2(px + pw, py + ph),
                                                IM_COL32(255, 255, 255, 30), 6.f, 0, 1.2f);

                                    // Timer text (centered)
                                    Draw::Text(dl, ImVec2(px + pw * 0.5f, py + padY),
                                               timerCol, buf, true, fontSize);

                                    // Progress bar
                                    float barY = py + padY + tsz.y + 3.f;
                                    float barW = pw - padX * 2;
                                    float frac = remaining / 40.f;
                                    if (frac > 1.f) frac = 1.f;
                                    if (frac < 0.f) frac = 0.f;
                                    
                                    // Bar background
                                    dl->AddRectFilled(ImVec2(px + padX, barY),
                                                      ImVec2(px + padX + barW, barY + barH),
                                                      IM_COL32(30, 30, 35, 200), 1.5f);
                                    // Bar fill
                                    dl->AddRectFilled(ImVec2(px + padX, barY),
                                                      ImVec2(px + padX + barW * frac, barY + barH),
                                                      barCol, 1.5f);

                                    // Defuse text (blue, below timer)
                                    if (isDef && defRem > 0.f)
                                    {
                                        float dy = barY + barH + 3.f;
                                        Draw::Text(dl, ImVec2(px + pw * 0.5f, dy),
                                                   IM_COL32(80, 180, 255, 255), defBuf, true, 12.f);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ---- Everything below requires ESP enabled ----
        if (!cfg.enabled) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        uintptr_t localCtrl = GameState::GetLocalController();
        uintptr_t entList   = GameState::GetEntityList();
        if (!localPawn || !localCtrl || !entList) return;

        uint32_t localH = Mem::Read<uint32_t>(localCtrl + Offsets::m_hPlayerPawn);
        int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);

        std::vector<std::string> spectatorNames;
        Math::Vec3 localPos = GameState::GetEntityOrigin(localPawn);

        for (int i = 1; i <= 64; ++i)
        {
            uintptr_t chunkPtr = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
            if (!chunkPtr) continue;
            uintptr_t ctrl = Mem::Read<uintptr_t>(chunkPtr + 0x70 * (i & 0x1FF));
            if (!ctrl || ctrl == localCtrl) continue;

            bool alive = Mem::Read<bool>(ctrl + Offsets::m_bPawnIsAlive);

            if (!alive && cfg.spectators)
            {
                uint32_t obH = Mem::Read<uint32_t>(ctrl + Offsets::m_hObserverPawn);
                if (obH && obH != 0xFFFFFFFF)
                {
                    uintptr_t obPawn = GameState::ResolveHandle(obH);
                    if (obPawn)
                    {
                        uintptr_t obSvc = Mem::Read<uintptr_t>(obPawn + Offsets::m_pObserverServices);
                        if (obSvc)
                        {
                            uint32_t tgtH = Mem::Read<uint32_t>(obSvc + Offsets::m_hObserverTarget);
                            if (tgtH == localH)
                            {
                                uintptr_t np = Mem::Read<uintptr_t>(ctrl + Offsets::m_sSanitizedPlayerName);
                                if (np) {
                                    struct NB { char d[32]; };
                                    NB nb = Mem::Read<NB>(np); nb.d[31] = '\0';
                                    if (nb.d[0]) {
                                        std::string s(nb.d);
                                        if (std::find(spectatorNames.begin(), spectatorNames.end(), s) == spectatorNames.end())
                                            spectatorNames.push_back(s);
                                    }
                                }
                            }
                        }
                    }
                }
                continue;
            }
            if (!alive) continue;

            uint32_t pawnH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
            if (!pawnH) continue;
            uintptr_t pawn = GameState::ResolveHandle(pawnH);
            if (!pawn || pawn == localPawn) continue;

            int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
            if (hp <= 0) continue;

            if (cfg.teamCheck) {
                int team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                if (team == localTeam) continue;
            }

            Math::Vec3 feet = GameState::GetEntityOrigin(pawn);
            Math::Vec3 head = GameState::GetBonePos(pawn, 6);
            if (feet.IsZero() || head.IsZero()) continue;
            head.z += 8.f;

            float feetX, feetY, headX, headY;
            float fp[3] = {feet.x,feet.y,feet.z};
            float hp3[3] = {head.x,head.y,head.z};
            if (!GameState::WorldToScreen(fp, feetX, feetY, scrW, scrH)) continue;
            if (!GameState::WorldToScreen(hp3, headX, headY, scrW, scrH)) continue;

            float boxH = feetY - headY;
            float boxW = boxH * 0.45f;
            float boxX = headX - boxW * 0.5f;

            // Determine ESP color: visible/hidden aware or static
            bool spotted = IsSpotted(pawn);
            ImU32 espCol;
            if (cfg.visColorEnabled)
            {
                float* c = spotted ? cfg.visibleColor : cfg.hiddenColor;
                espCol = ImGui::ColorConvertFloat4ToU32({c[0], c[1], c[2], c[3]});
            }
            else
            {
                espCol = ImGui::ColorConvertFloat4ToU32(
                    {cfg.boxColor[0], cfg.boxColor[1], cfg.boxColor[2], cfg.boxColor[3]});
            }

            if (cfg.box) {
                if (cfg.boxStyle == 1)
                    Draw::CornerBox(dl, boxX, headY, boxW, boxH, espCol);
                else
                    Draw::FullBox(dl, boxX, headY, boxW, boxH, espCol);
            }

            if (cfg.skeleton) {
                ImU32 skelCol = ImGui::ColorConvertFloat4ToU32(
                    {cfg.skeletonColor[0],cfg.skeletonColor[1],cfg.skeletonColor[2],cfg.skeletonColor[3]});
                for (const auto& bp : kSkeleton) {
                    Math::Vec3 b1 = GameState::GetBonePos(pawn, bp.a);
                    Math::Vec3 b2 = GameState::GetBonePos(pawn, bp.b);
                    if (b1.IsZero() || b2.IsZero()) continue;
                    float p1[3]={b1.x,b1.y,b1.z}, p2[3]={b2.x,b2.y,b2.z};
                    float s1x,s1y,s2x,s2y;
                    if (GameState::WorldToScreen(p1,s1x,s1y,scrW,scrH) &&
                        GameState::WorldToScreen(p2,s2x,s2y,scrW,scrH)) {
                        dl->AddLine({s1x,s1y},{s2x,s2y}, IM_COL32(0,0,0,255), 3.4f);
                        dl->AddLine({s1x,s1y},{s2x,s2y}, skelCol, 1.6f);
                    }
                }
            }

            if (cfg.healthBar)
                Draw::HealthBar(dl, boxX, headY, boxH, hp);

            if (cfg.name) {
                uintptr_t np = Mem::Read<uintptr_t>(ctrl + Offsets::m_sSanitizedPlayerName);
                if (np) {
                    struct NB { char d[32]; };
                    NB nb = Mem::Read<NB>(np); nb.d[31] = '\0';
                    if (nb.d[0])
                        Draw::Text(dl, ImVec2(headX, headY-18.f), IM_COL32(255,255,255,255), nb.d, true, 12.f);
                }
            }

            if (cfg.distance && !localPos.IsZero()) {
                float dist = (feet-localPos).Length2D() * 0.0254f;
                char buf[16]; snprintf(buf,sizeof(buf),"%.0fm",dist);
                Draw::Text(dl, ImVec2(headX, feetY+4.f), IM_COL32(200,200,200,255), buf, true, 11.f);
            }

            // Weapon icon (font-based) or text badge fallback - optimized rendering
            if (cfg.weapon) {
                uintptr_t weapSvc = Mem::Read<uintptr_t>(pawn + Offsets::m_pWeaponServices);
                if (weapSvc) {
                    uint32_t activeH = Mem::Read<uint32_t>(weapSvc + Offsets::m_hActiveWeapon);
                    uintptr_t activeW = GameState::ResolveHandle(activeH);
                    if (activeW) {
                        float yOff = (cfg.distance && !localPos.IsZero()) ? feetY + 18.f : feetY + 4.f;
                        std::string weaponName = GameState::GetDesignerName(activeW);
                        
                        // Render weapon icon if enabled and font loaded, otherwise show text badge
                        if (cfg.weaponIcon && iconFont && !weaponName.empty())
                        {
                            // Cache icon character lookup to avoid repeated string operations
                            const char* iconChar = WeaponIcons::GetWeaponIcon(weaponName.c_str());
                            
                            // Use smaller font size (14px instead of 18px) for better scaling
                            ImGui::PushFont(iconFont);
                            
                            // Scale down the icon slightly for cleaner look
                            ImVec2 iconSize = ImGui::CalcTextSize(iconChar);
                            float scale = 0.75f; // 25% smaller
                            iconSize.x *= scale;
                            iconSize.y *= scale;
                            
                            float iconX = headX - (iconSize.x * 0.5f);
                            
                            // Improved shadow - single offset for performance
                            dl->AddText(iconFont, ImGui::GetFontSize() * scale, 
                                       ImVec2(iconX + 1, yOff + 1), 
                                       IM_COL32(0, 0, 0, 200), iconChar);
                            
                            // Main icon with slight transparency for softer look
                            dl->AddText(iconFont, ImGui::GetFontSize() * scale, 
                                       ImVec2(iconX, yOff), 
                                       IM_COL32(255, 255, 255, 240), iconChar);
                            
                            ImGui::PopFont();
                        }
                        else
                        {
                            // Fallback to text badge
                            uintptr_t wItem = activeW + Offsets::m_AttributeManager + Offsets::m_Item;
                            uint16_t wDef = Mem::Read<uint16_t>(wItem + Offsets::m_iItemDefinitionIndex);
                            Draw::WeaponBadge(dl, ImVec2(headX, yOff), wDef);
                        }
                    }
                }
            }
        }

        // Dropped weapons and equipment ESP - shows icons above dropped items
        if (cfg.weapon && iconFont)
        {
            for (int i = 65; i < 2048; ++i)
            {
                uintptr_t chunkPtr = Mem::Read<uintptr_t>(entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                if (!chunkPtr) continue;
                
                uintptr_t entity = Mem::Read<uintptr_t>(chunkPtr + 0x70 * (i & 0x1FF));
                if (!entity) continue;

                // Get entity class name using GameState helper
                std::string className = GameState::GetDesignerName(entity);
                if (className.empty()) continue;

                // Check if it's a weapon or equipment
                bool isWeapon = (className.find("weapon_") != std::string::npos);
                bool isGrenade = (className.find("grenade") != std::string::npos || 
                                 className.find("molotov") != std::string::npos ||
                                 className.find("incgrenade") != std::string::npos ||
                                 className.find("decoy") != std::string::npos ||
                                 className.find("flashbang") != std::string::npos ||
                                 className.find("smokegrenade") != std::string::npos);
                
                if (!isWeapon && !isGrenade) continue;

                // Check if weapon has no owner (dropped)
                // Owner handle: 0 or 0xFFFFFFFF means no owner
                uint32_t ownerHandle = Mem::Read<uint32_t>(entity + Offsets::m_hOwnerEntity);
                if (ownerHandle != 0 && ownerHandle != 0xFFFFFFFF) continue; // Has owner, skip

                // Get position - add height offset so icon appears above the item
                Math::Vec3 origin = GameState::GetEntityOrigin(entity);
                if (origin.IsZero()) continue;
                
                origin.z += 15.f; // Lift icon above the weapon model
                
                float screenX, screenY;
                float worldPos[3] = {origin.x, origin.y, origin.z};
                if (!GameState::WorldToScreen(worldPos, screenX, screenY, scrW, scrH)) continue;

                // Get weapon icon character
                const char* iconChar = WeaponIcons::GetWeaponIcon(className.c_str());
                
                // Render icon
                ImGui::PushFont(iconFont);
                
                float scale = 0.9f; // Good visibility
                ImVec2 iconSize = ImGui::CalcTextSize(iconChar);
                iconSize.x *= scale;
                iconSize.y *= scale;
                
                float iconX = screenX - (iconSize.x * 0.5f);
                float iconY = screenY - (iconSize.y * 0.5f);
                
                // Color based on type - bright colors for visibility
                ImU32 iconColor;
                bool isBomb = (className == "weapon_c4");
                if (isBomb) {
                    iconColor = IM_COL32(255, 60, 60, 255); // Bright red for C4
                } else if (isGrenade) {
                    iconColor = IM_COL32(80, 200, 255, 255); // Bright blue for grenades
                } else {
                    iconColor = IM_COL32(255, 230, 80, 255); // Bright yellow for weapons
                }
                
                // Strong shadow for visibility
                dl->AddText(iconFont, ImGui::GetFontSize() * scale, 
                           ImVec2(iconX + 1.5f, iconY + 1.5f), 
                           IM_COL32(0, 0, 0, 220), iconChar);
                
                // Main icon
                dl->AddText(iconFont, ImGui::GetFontSize() * scale, 
                           ImVec2(iconX, iconY), 
                           iconColor, iconChar);
                
                ImGui::PopFont();
                
                // Show distance for dropped items if not too far
                if (!localPos.IsZero()) {
                    float dist = (origin - localPos).Length() * 0.0254f;
                    if (dist < 50.f) {
                        char buf[16];
                        snprintf(buf, sizeof(buf), "%.0fm", dist);
                        Draw::Text(dl, ImVec2(screenX, screenY + iconSize.y * 0.5f + 3), 
                                  IM_COL32(220, 220, 220, 230), buf, true, 10.f);
                    }
                }
            }
        }

        Draw::SpectatorPanel(dl, scrW, spectatorNames, cfg.spectatorStyle);
    }
}
