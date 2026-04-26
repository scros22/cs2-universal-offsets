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
#include "world_effects.h"   // for WorldEffects::*Hooked diagnostic flags
#include "aimbot.h"          // for Aimbot::diag_* live counters
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

            // Two-ring "halo" outline:
            //   * inner ring (4 cardinal directions, 1px, full-alpha) gives a
            //     crisp readable edge,
            //   * outer ring (4 diagonal directions, ~1.4px, half-alpha) plus
            //     4 cardinal directions at 1.4px low-alpha softens the corners
            //     so the outline blends rather than "stair-steps".
            // Net effect: a subtle drop-shadow / glyph border that reads as
            // smooth at any zoom instead of the chunky 8-dir 1px halo.
            const ImU32 outlInner = IM_COL32(0, 0, 0, 235);
            const ImU32 outlOuter = IM_COL32(0, 0, 0, 110);
            constexpr float oi = 1.0f;   // inner ring offset
            constexpr float oo = 1.6f;   // outer ring offset

            // Outer soft halo (8 directions, low alpha → looks like a blurred
            // border once stacked under the inner ring).
            dl->AddText(font, fs, ImVec2(pos.x - oo, pos.y      ), outlOuter, txt);
            dl->AddText(font, fs, ImVec2(pos.x + oo, pos.y      ), outlOuter, txt);
            dl->AddText(font, fs, ImVec2(pos.x,      pos.y - oo), outlOuter, txt);
            dl->AddText(font, fs, ImVec2(pos.x,      pos.y + oo), outlOuter, txt);
            dl->AddText(font, fs, ImVec2(pos.x - oo, pos.y - oo), outlOuter, txt);
            dl->AddText(font, fs, ImVec2(pos.x + oo, pos.y - oo), outlOuter, txt);
            dl->AddText(font, fs, ImVec2(pos.x - oo, pos.y + oo), outlOuter, txt);
            dl->AddText(font, fs, ImVec2(pos.x + oo, pos.y + oo), outlOuter, txt);

            // Crisp inner ring (4 cardinal, full alpha).
            dl->AddText(font, fs, ImVec2(pos.x - oi, pos.y      ), outlInner, txt);
            dl->AddText(font, fs, ImVec2(pos.x + oi, pos.y      ), outlInner, txt);
            dl->AddText(font, fs, ImVec2(pos.x,      pos.y - oi), outlInner, txt);
            dl->AddText(font, fs, ImVec2(pos.x,      pos.y + oi), outlInner, txt);

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
                              int hp, float barW = 2.4f, float gap = 5.f)
        {
            // Silk redesign — thin (2.4 px), full rounding, gradient sheen,
            // soft outer halo for separation against bright pixels. No hard
            // outline. HP number floats as a tiny chip above the fill cap.
            float frac = (float)hp / 100.f;
            if (frac > 1.f) frac = 1.f;
            if (frac < 0.f) frac = 0.f;
            const float barH  = h * frac;
            const float bx    = x - gap - barW;
            const float rnd   = barW * 0.5f;     // capsule

            // Soft outer halo (no hard edge)
            dl->AddRectFilled(ImVec2(bx - 1.2f, y - 1.2f),
                              ImVec2(bx + barW + 1.2f, y + h + 1.2f),
                              IM_COL32(0, 0, 0, 110), rnd + 1.2f);

            // Trough — translucent slate
            dl->AddRectFilled(ImVec2(bx, y), ImVec2(bx + barW, y + h),
                              IM_COL32(14, 16, 22, 175), rnd);

            // Health colour — green→yellow→red, slightly desaturated for "silk"
            float cr, cg;
            if (hp > 50) { float t = (hp - 50) / 50.f; cr = 1.f - t * 0.95f; cg = 0.78f + t * 0.18f; }
            else         { float t = hp / 50.f;        cr = 1.f;             cg = 0.18f + t * 0.60f; }

            const ImU32 fillTopC = IM_COL32((int)(cr * 255), (int)(cg * 255), 60, 245);
            const ImU32 fillBotC = IM_COL32((int)(cr * 195), (int)(cg * 195), 30, 230);

            const float fillY = y + (h - barH);
            if (barH > 0.5f)
            {
                // Vertical gradient fill
                dl->AddRectFilledMultiColor(
                    ImVec2(bx, fillY), ImVec2(bx + barW, y + h),
                    fillTopC, fillTopC, fillBotC, fillBotC);
                // Re-round the top cap
                dl->AddRectFilled(ImVec2(bx, fillY),
                                  ImVec2(bx + barW, fillY + rnd * 2.f),
                                  fillTopC, rnd, ImDrawFlags_RoundCornersTop);
                // Re-round the bottom cap
                dl->AddRectFilled(ImVec2(bx, y + h - rnd * 2.f),
                                  ImVec2(bx + barW, y + h),
                                  fillBotC, rnd, ImDrawFlags_RoundCornersBottom);

                // Silk highlight — 1 px sheen down the left of the fill
                if (barH > 3.f)
                {
                    dl->AddLine(
                        ImVec2(bx + 0.6f, fillY + 1.2f),
                        ImVec2(bx + 0.6f, y + h - 1.2f),
                        IM_COL32(255, 255, 255, 90), 0.8f);
                }

                // Tiny bright cap dot at the top of the current fill — gives
                // the bar a "filled to here" focal point.
                dl->AddCircleFilled(
                    ImVec2(bx + barW * 0.5f, fillY + 0.6f),
                    rnd + 0.4f,
                    IM_COL32(255, 255, 255, 150), 8);
            }

            // HP number when not full — small dim chip above cap
            if (hp < 100)
            {
                char buf[8]; snprintf(buf, sizeof(buf), "%d", hp);
                ImVec2 sz = ImGui::CalcTextSize(buf);
                const float tx = bx + barW * 0.5f;
                const float ty = fillY - sz.y - 1.f;
                Text(dl, ImVec2(tx, ty),
                     IM_COL32(245, 245, 250, 220), buf, true, 9.f);
            }
        }

        inline void BombPanel(ImDrawList* dl, float scrW, float remaining,
                              bool defusing, float defuseRem, int style = 0)
        {
            char buf[32];
            const bool urgent = remaining < 10.f;
            const bool danger = remaining < 5.f;

            // Shared palette
            const ImU32 cText      = urgent ? IM_COL32(255, 90, 80, 255) : IM_COL32(240, 240, 245, 240);
            const ImU32 cTextDim   = urgent ? IM_COL32(255, 130, 110, 220) : IM_COL32(170, 170, 180, 220);
            const ImU32 cAccent    = urgent ? IM_COL32(255, 70, 50, 255)  : IM_COL32(80, 200, 130, 255);
            const ImU32 cBg        = IM_COL32(10, 11, 16, 220);
            const ImU32 cBgDeep    = IM_COL32(6,  7,  11, 235);
            const ImU32 cOutline   = IM_COL32(255, 255, 255, 22);

            const float frac = (remaining > 0.f && remaining <= 40.f) ? (remaining / 40.f) : 0.f;

            // ----------------------------------------------------------
            //  STYLE 1 — VIVID  (compact accent capsule, glow halo)
            // ----------------------------------------------------------
            if (style == 1)
            {
                const float pw = 144.f, ph = 32.f;
                const float px = scrW * 0.5f - pw * 0.5f;
                const float py = 56.f;
                const float r  = ph * 0.5f;  // capsule radius

                // soft glow halo (pulses harder when urgent)
                const int glowAlpha = danger ? 55 : (urgent ? 38 : 18);
                for (int i = 4; i > 0; --i) {
                    dl->AddRect({px - i, py - i}, {px + pw + i, py + ph + i},
                                IM_COL32((cAccent>>IM_COL32_R_SHIFT)&0xFF,
                                         (cAccent>>IM_COL32_G_SHIFT)&0xFF,
                                         (cAccent>>IM_COL32_B_SHIFT)&0xFF,
                                         glowAlpha / i),
                                r + i, 0, 1.f);
                }
                dl->AddRectFilled({px, py}, {px + pw, py + ph}, cBgDeep, r);
                dl->AddRect      ({px, py}, {px + pw, py + ph}, IM_COL32(255,255,255,28), r, 0, 1.f);
                // accent bar left edge
                dl->AddRectFilled({px + 4.f, py + 5.f}, {px + 6.f, py + ph - 5.f}, cAccent, 2.f);

                snprintf(buf, sizeof(buf), "C4  %.1fs", remaining);
                Text(dl, ImVec2(px + pw * 0.5f + 3.f, py + 4.f), cText, buf, true, 13.f);

                // hairline progress under text
                const float bX = px + 14.f, bY = py + ph - 6.f, bW = pw - 28.f, bH = 2.f;
                dl->AddRectFilled({bX, bY}, {bX + bW, bY + bH}, IM_COL32(40, 42, 56, 200), 1.f);
                dl->AddRectFilled({bX, bY}, {bX + bW * frac, bY + bH}, cAccent, 1.f);

                if (defusing && defuseRem > 0.f) {
                    snprintf(buf, sizeof(buf), "DEFUSE %.1fs", defuseRem);
                    Text(dl, ImVec2(px + pw * 0.5f, py + ph + 4.f),
                         IM_COL32(120, 190, 255, 255), buf, true, 11.f);
                }
                return;
            }

            // ----------------------------------------------------------
            //  STYLE 2 — COMPACT  (small left-aligned chip)
            // ----------------------------------------------------------
            if (style == 2)
            {
                const float pw = 96.f, ph = 22.f;
                const float px = 14.f, py = 72.f;
                const float r  = 4.f;

                dl->AddRectFilled({px, py}, {px + pw, py + ph}, cBg, r);
                dl->AddRect      ({px, py}, {px + pw, py + ph}, cOutline, r, 0, 1.f);
                dl->AddRectFilled({px + 3.f, py + 4.f}, {px + 4.5f, py + ph - 4.f}, cAccent, 1.5f);

                snprintf(buf, sizeof(buf), "C4  %.1fs", remaining);
                Text(dl, ImVec2(px + 9.f, py + 4.f), cText, buf, false, 11.f);

                // mini progress bar across bottom
                const float bX = px + 1.f, bY = py + ph - 1.5f, bW = pw - 2.f;
                dl->AddRectFilled({bX, bY}, {bX + bW * frac, bY + 1.5f}, cAccent);

                if (defusing && defuseRem > 0.f) {
                    snprintf(buf, sizeof(buf), "DEF %.1fs", defuseRem);
                    Text(dl, ImVec2(px + pw * 0.5f, py + ph + 3.f),
                         IM_COL32(120, 190, 255, 255), buf, true, 10.f);
                }
                return;
            }

            // ----------------------------------------------------------
            //  STYLE 0 — CLASSIC  (refined dark pill, centred)
            // ----------------------------------------------------------
            const float pw = 124.f, ph = 30.f;
            const float px = scrW * 0.5f - pw * 0.5f;
            const float py = 64.f;
            const float r  = 6.f;

            // subtle drop shadow
            for (int i = 3; i > 0; --i) {
                dl->AddRectFilled({px - i, py - i + 1}, {px + pw + i, py + ph + i + 1},
                                  IM_COL32(0, 0, 0, 12 + (3 - i) * 6), r + i);
            }
            dl->AddRectFilled({px, py}, {px + pw, py + ph}, cBgDeep, r);
            dl->AddRect      ({px, py}, {px + pw, py + ph}, cOutline, r, 0, 1.f);
            dl->AddRect      ({px + 1, py + 1}, {px + pw - 1, py + ph - 1},
                              IM_COL32(255, 255, 255, 10), r - 1.f, 0, 1.f);

            // tiny accent dot left
            dl->AddCircleFilled({px + 11.f, py + ph * 0.5f}, 2.4f, cAccent, 12);

            snprintf(buf, sizeof(buf), "C4  %.1fs", remaining);
            Text(dl, ImVec2(px + pw * 0.5f + 4.f, py + 4.f), cText, buf, true, 12.f);

            // hairline progress bar across the bottom
            const float bX = px + 10.f, bY = py + ph - 5.f, bW = pw - 20.f, bH = 2.f;
            dl->AddRectFilled({bX, bY}, {bX + bW, bY + bH}, IM_COL32(38, 40, 52, 180), 1.f);
            dl->AddRectFilled({bX, bY}, {bX + bW * frac, bY + bH}, cAccent, 1.f);

            if (defusing && defuseRem > 0.f) {
                snprintf(buf, sizeof(buf), "DEFUSE %.1fs", defuseRem);
                Text(dl, ImVec2(px + pw * 0.5f, py + ph + 3.f),
                     IM_COL32(120, 190, 255, 255), buf, true, 11.f);
            }
            (void)cTextDim;
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
    // Skeleton bones (CS2 build 14152 — Animgraph 2 indices)
    //   0  ORIGIN     1  PELVIS    2  SPINE0    3  SPINE1   4  SPINE2
    //   6  NECK       7  HEAD      8  CLAV_L    9  SHO_L   10  ELB_L
    //  11  HAND_L    12  CLAV_R   13  SHO_R    14  ELB_R   15  HAND_R
    //  17  HIP_L     18  KNEE_L   19  HEEL_L   20  HIP_R   21  KNEE_R
    //  22  HEEL_R    23  CHEST    24  GUN      25  EYE_L   26  EYE_R
    //  27  VIEW_ANGLE (per UC — tracks aim direction; useful for resolver)
    // ---------------------------------------------------------------
    struct BonePair { int a, b; };
    inline const BonePair kSkeleton[] = {
        // Spine
        {1, 3}, {3, 4}, {4, 23}, {23, 6}, {6, 7},
        // Left arm
        {6, 9}, {9, 10}, {10, 11},
        // Right arm
        {6, 13}, {13, 14}, {14, 15},
        // Left leg
        {1, 17}, {17, 18}, {18, 19},
        // Right leg
        {1, 20}, {20, 21}, {21, 22},
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
                GameState::clientBase + GameState::RVA_dwPlantedC4());
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
                    GameState::clientBase + GameState::RVA_dwPlantedC4());
                uintptr_t dataPtr = Mem::Read<uintptr_t>(
                    GameState::clientBase + GameState::RVA_dwPlantedC4() + 0x8);
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

        // ---- DIAGNOSTIC overlay (top-left). Always-on while we debug 14152. ----
        // Shows exactly where the ESP loop bails so we can fix offsets blind.
        int dbg_chunks = 0, dbg_ctrls = 0, dbg_alive = 0, dbg_enemy = 0, dbg_w2s_ok = 0, dbg_w2s_fail = 0;
        Math::ViewMatrix dbgVM = GameState::GetViewMatrix();
        bool dbgVMok = dbgVM.m[3][0] != 0.f || dbgVM.m[3][1] != 0.f || dbgVM.m[3][2] != 0.f;

        auto drawDbg = [&]() {
            (void)dbg_chunks; (void)dbg_ctrls; (void)dbg_alive; (void)dbg_enemy;
            (void)dbg_w2s_ok; (void)dbg_w2s_fail; (void)dbgVMok; (void)dl;
            if (false) {
            char buf[480];
            snprintf(buf, sizeof(buf),
                "ESP DIAG: cb=%llx | resLP=%llx resLC=%llx resEL=%llx resVM=%llx resVA=%llx | lp=%llx lc=%llx el=%llx | chunks=%d ctrls=%d alive=%d enemy=%d | w2s ok=%d fail=%d | VM ok=%d",
                (unsigned long long)GameState::clientBase,
                (unsigned long long)GameState::resolved_dwLocalPlayerPawn,
                (unsigned long long)GameState::resolved_dwLocalPlayerController,
                (unsigned long long)GameState::resolved_dwEntityList,
                (unsigned long long)GameState::resolved_dwViewMatrix,
                (unsigned long long)GameState::resolved_dwViewAngles,
                (unsigned long long)localPawn, (unsigned long long)localCtrl, (unsigned long long)entList,
                dbg_chunks, dbg_ctrls, dbg_alive, dbg_enemy, dbg_w2s_ok, dbg_w2s_fail, dbgVMok ? 1 : 0);
            dl->AddText({10.f, 10.f}, IM_COL32(255,255,0,255), buf);

            // Second diagnostic line: hooks installed + live view-angles.
            Math::QAngle va{0,0,0};
            __try {
                if (GameState::clientBase)
                    va = Mem::Read<Math::QAngle>(GameState::clientBase + GameState::RVA_dwViewAngles());
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
            char buf2[400];
            snprintf(buf2, sizeof(buf2),
                "  HOOKS: fov=%d sky=%d 3p=%d | viewAng pitch=%.1f yaw=%.1f roll=%.1f",
                WorldEffects::fovHooked ? 1 : 0,
                WorldEffects::skyHooked ? 1 : 0,
                (WorldEffects::pThirdPersonOn && WorldEffects::pThirdPersonOff) ? 1 : 0,
                va.pitch, va.yaw, va.roll);
            dl->AddText({10.f, 26.f}, IM_COL32(0,255,0,255), buf2);

            // Aimbot live state — diagnoses why the bot isn't moving the mouse.
            DWORD now = GetTickCount();
            auto agoMs = [&](DWORD t)->int { return t == 0 ? -1 : (int)(now - t); };
            char buf3[480];
            snprintf(buf3, sizeof(buf3),
                "  AIM: en=%d slt=%d ak=%d cm=%d ins=%d/err=%d insAddr=%p tickAgo=%d keyAgo=%d tgtAgo=%d sentAgo=%d ent=%d fov=%.1f bail=%d lastDx=%ld lastDy=%ld direct=%d  crash=%d reachedAgo=%d",
                Aimbot::cfg.enabled ? 1 : 0,
                Aimbot::cfg.silentAim ? 1 : 0,
                Aimbot::cfg.aimKey,
                Aimbot::SilentAim::pCreateMoveHook ? 1 : 0,
                Aimbot::diag_installResult,
                Aimbot::diag_installSubErr,
                (void*)Aimbot::diag_installAddr,
                agoMs(Aimbot::diag_lastTick),
                agoMs(Aimbot::diag_lastKeyDown),
                agoMs(Aimbot::diag_lastTarget),
                agoMs(Aimbot::diag_lastSent),
                Aimbot::diag_targetEnt,
                Aimbot::diag_lastFov,
                Aimbot::diag_lastBail,
                Aimbot::diag_lastDx, Aimbot::diag_lastDy,
                Aimbot::useDirectSendInput ? 1 : 0,
                Aimbot::diag_lastCrash,
                agoMs(Aimbot::diag_lastReached));
            dl->AddText({10.f, 42.f}, IM_COL32(255,160,80,255), buf3);

            // Why no target? Per-reject breakdown from FindBestTarget.
            char buf4[400];
            snprintf(buf4, sizeof(buf4),
                "  TGT-REJ: seen=%d dead=%d team=%d valid=%d bone=%d fov=%d vis=%d smoke=%d  cfg(team=%d vis=%d smoke=%d fov=%.1f bone=%d hp=%d) bArrOff=0x%X",
                Aimbot::diag_seen, Aimbot::diag_rDead, Aimbot::diag_rTeam, Aimbot::diag_rValid,
                Aimbot::diag_rBone, Aimbot::diag_rFov, Aimbot::diag_rVis, Aimbot::diag_rSmoke,
                Aimbot::cfg.teamCheck?1:0, Aimbot::cfg.visCheck?1:0, Aimbot::cfg.smokeCheck?1:0,
                Aimbot::cfg.fov, Aimbot::cfg.targetBone, Aimbot::cfg.headPriority?1:0,
                GameState::detectedBoneArrayOffset);
            dl->AddText({10.f, 58.f}, IM_COL32(255,80,80,255), buf4);

            // SilentAim hook stage counters — pinpoints where the hook bails.
            // enter=hook called, punch=passed punch block, lag=passed fakelag,
            // armed=hasTarget+entries valid, applied=actually wrote angles,
            // bail=last bail reason (1=disabled,2=noArr,3=cnt<=0,4=!hasTgt,5=SEH).
            char buf5[300];
            snprintf(buf5, sizeof(buf5),
                "  SLT-HK: enter=%lu punch=%lu lag=%lu armed=%lu applied=%lu bail=%lu  hasTgt=%d aimP=%.1f aimY=%.1f",
                (unsigned long)Aimbot::diag_silentEnter,
                (unsigned long)Aimbot::diag_silentPunch,
                (unsigned long)Aimbot::diag_silentLag,
                (unsigned long)Aimbot::diag_silentArmed,
                (unsigned long)Aimbot::diag_silentApplied,
                (unsigned long)Aimbot::diag_silentBailReason,
                Aimbot::SilentAim::hasTarget ? 1 : 0,
                Aimbot::SilentAim::aimPitch,
                Aimbot::SilentAim::aimYaw);
            dl->AddText({10.f, 74.f}, IM_COL32(120,200,255,255), buf5);

            // WriteSubtick hook (per-subtick cmd writer @ sub_180C54450)
            // wsIns: 1=hooked, -1=no sig, -2=CreateHook fail, -3=EnableHook fail
            // wsCalls = total invocations (any), wsRedir = ticks where angles actually rewritten
            char buf6[300];
            snprintf(buf6, sizeof(buf6),
                "  WS-HK: ins=%d/err=%d addr=%p calls=%lu redir=%lu",
                Aimbot::diag_wsInstall, Aimbot::diag_wsSubErr,
                (void*)Aimbot::diag_wsAddr,
                (unsigned long)Aimbot::diag_wsCalls,
                (unsigned long)Aimbot::diag_wsRedirected);
            dl->AddText({10.f, 90.f}, IM_COL32(180,255,160,255), buf6);
            } // end if(false) — debug overlay disabled for clean release build
        };

        if (!localPawn || !localCtrl || !entList) { drawDbg(); return; }

        uint32_t localH = Mem::Read<uint32_t>(localCtrl + Offsets::m_hPlayerPawn);
        int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);

        std::vector<std::string> spectatorNames;
        Math::Vec3 localPos = GameState::GetEntityOrigin(localPawn);

        for (int i = 1; i <= 64; ++i)
        {
            uintptr_t ctrl = GameState::GetEntityByIndex(i);
            if (!ctrl) continue;
            ++dbg_chunks;
            ++dbg_ctrls;
            if (ctrl == localCtrl) continue;

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
            ++dbg_alive;

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
            ++dbg_enemy;

            Math::Vec3 feet = GameState::GetEntityOrigin(pawn);
            if (feet.IsZero()) continue;
            // Animgraph 2 (build 14152): HEAD bone is index 7, NECK is 6.
            Math::Vec3 head = GameState::GetBonePos(pawn, 7);
            // Bone array offset can drift between builds; if the head bone
            // is unavailable, estimate it from feet so boxes still draw.
            if (head.IsZero())
                head = { feet.x, feet.y, feet.z + 72.f };
            else
                head.z += 8.f;

            float feetX, feetY, headX, headY;
            float fp[3] = {feet.x,feet.y,feet.z};
            float hp3[3] = {head.x,head.y,head.z};
            if (!GameState::WorldToScreen(fp, feetX, feetY, scrW, scrH)) { ++dbg_w2s_fail; continue; }
            if (!GameState::WorldToScreen(hp3, headX, headY, scrW, scrH)) { ++dbg_w2s_fail; continue; }
            ++dbg_w2s_ok;

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
                            const char* iconChar = WeaponIcons::GetWeaponIcon(weaponName.c_str());

                            // Tighter scale (was 0.75) — icons were eating
                            // vertical space over short pawns. 0.60 keeps
                            // glyphs readable while staying out of the bar.
                            constexpr float scale    = 0.60f;
                            const float     baseFs   = ImGui::GetFontSize();
                            const float     glyphFs  = baseFs * scale;

                            ImGui::PushFont(iconFont);
                            ImVec2 iconSize = ImGui::CalcTextSize(iconChar);
                            iconSize.x *= scale;
                            iconSize.y *= scale;
                            const float iconX = headX - iconSize.x * 0.5f;

                            // Soft 4-direction shadow (much smoother than the
                            // single hard offset; same cost as 4 AddText).
                            constexpr float so = 1.0f;
                            const ImU32 shadow = IM_COL32(0, 0, 0, 170);
                            dl->AddText(iconFont, glyphFs, ImVec2(iconX - so, yOff     ), shadow, iconChar);
                            dl->AddText(iconFont, glyphFs, ImVec2(iconX + so, yOff     ), shadow, iconChar);
                            dl->AddText(iconFont, glyphFs, ImVec2(iconX,      yOff - so), shadow, iconChar);
                            dl->AddText(iconFont, glyphFs, ImVec2(iconX,      yOff + so), shadow, iconChar);

                            // Main glyph
                            dl->AddText(iconFont, glyphFs, ImVec2(iconX, yOff),
                                        IM_COL32(245, 245, 250, 245), iconChar);
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
                uintptr_t entity = GameState::GetEntityByIndex(i);
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

                // Tighter scale (was 0.9) — dropped icons no longer dominate
                // the screen when several pieces of kit are stacked.
                constexpr float scale  = 0.62f;
                const float baseFs     = ImGui::GetFontSize();
                const float glyphFs    = baseFs * scale;

                ImGui::PushFont(iconFont);
                ImVec2 iconSize = ImGui::CalcTextSize(iconChar);
                iconSize.x *= scale;
                iconSize.y *= scale;

                const float iconX = screenX - iconSize.x * 0.5f;
                const float iconY = screenY - iconSize.y * 0.5f;

                // Type-tinted icon — desaturated a notch from the old neon
                // palette so several drops in a frame don't clash.
                ImU32 iconColor;
                const bool isBomb = (className == "weapon_c4");
                if      (isBomb)    iconColor = IM_COL32(255,  92,  82, 250); // C4 red
                else if (isGrenade) iconColor = IM_COL32(110, 200, 255, 245); // grenade blue
                else                iconColor = IM_COL32(255, 220, 110, 245); // weapon amber

                // Subtle pill backdrop — gives the glyph a clean reading
                // surface without a hard rectangle. Only painted when the
                // item is reasonably close (avoid clutter at long range).
                float dist = 0.f;
                if (!localPos.IsZero())
                    dist = (origin - localPos).Length() * 0.0254f;
                const bool drawChip = (dist > 0.f && dist < 35.f);

                if (drawChip) {
                    const float pad   = 3.5f;
                    const float chipR = (iconSize.y * 0.5f) + pad;
                    const ImVec2 cMin{iconX - pad,            iconY - pad * 0.5f};
                    const ImVec2 cMax{iconX + iconSize.x + pad, iconY + iconSize.y + pad * 0.5f};
                    dl->AddRectFilled(cMin, cMax, IM_COL32(8, 9, 14, 165), chipR);
                    dl->AddRect      (cMin, cMax, IM_COL32(255, 255, 255, 18), chipR, 0, 1.f);
                }

                // Soft 4-direction shadow halo for off-chip readability.
                constexpr float so = 1.0f;
                const ImU32 shadow = IM_COL32(0, 0, 0, 175);
                dl->AddText(iconFont, glyphFs, ImVec2(iconX - so, iconY     ), shadow, iconChar);
                dl->AddText(iconFont, glyphFs, ImVec2(iconX + so, iconY     ), shadow, iconChar);
                dl->AddText(iconFont, glyphFs, ImVec2(iconX,      iconY - so), shadow, iconChar);
                dl->AddText(iconFont, glyphFs, ImVec2(iconX,      iconY + so), shadow, iconChar);

                // Main icon
                dl->AddText(iconFont, glyphFs, ImVec2(iconX, iconY), iconColor, iconChar);

                ImGui::PopFont();

                // Distance label below the icon for nearby items only.
                if (drawChip) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.0fm", dist);
                    Draw::Text(dl,
                               ImVec2(screenX, iconY + iconSize.y + 3.f),
                               IM_COL32(225, 225, 230, 235), buf, true, 9.f);
                }
            }
        }

        Draw::SpectatorPanel(dl, scrW, spectatorNames, cfg.spectatorStyle);

        // Diagnostic readout — top-left, yellow.
        drawDbg();
    }
}
