#pragma once
#include <Windows.h>
#include <vector>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "../imgui/imgui.h"

extern ImFont* g_EspFonts[3];

namespace ESP
{
    struct Config
    {
        bool enabled = true;
        bool bBox = true;
        int boxMode = 0;
        bool bSkeleton = true;
        bool bHealthBar = true;
        bool bName = true;
        bool bDistance = true;
        bool teamCheck = true;
        float boxColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float skeletonColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        bool bBombTimer = false;
        bool bGlow = false;
        float glowColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        bool bChams = false;
        float chamsColor[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
        bool bSpectators = false;
        int espFont = 1; // 0=Pixel, 1=Verdana, 2=Hax
    };

    inline Config config;

    inline void DrawLine(ImDrawList* draw, ImVec2 a, ImVec2 b, ImU32 col, float thickness)
    {
        draw->AddLine(a, b, col, thickness);
    }

    inline void DrawBox(ImDrawList* draw, float x, float y, float w, float h, ImU32 col)
    {
        draw->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
        draw->AddRect(ImVec2(x + 1, y + 1), ImVec2(x + w - 1, y + h - 1), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
        draw->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), col, 0.0f, 0, 1.0f);
    }

    inline void DrawCornerBox(ImDrawList* draw, float x, float y, float w, float h, ImU32 col)
    {
        float lineW = w / 4.0f;
        float lineH = h / 4.0f;

        auto DrawLineBase = [&](ImVec2 p1, ImVec2 p2) {
            draw->AddLine(p1, p2, IM_COL32(0, 0, 0, 255), 3.0f);
            draw->AddLine(p1, p2, col, 1.0f);
        };

        DrawLineBase(ImVec2(x, y), ImVec2(x + lineW, y));
        DrawLineBase(ImVec2(x, y), ImVec2(x, y + lineH));
        DrawLineBase(ImVec2(x + w - lineW, y), ImVec2(x + w, y));
        DrawLineBase(ImVec2(x + w, y), ImVec2(x + w, y + lineH));
        DrawLineBase(ImVec2(x, y + h - lineH), ImVec2(x, y + h));
        DrawLineBase(ImVec2(x, y + h), ImVec2(x + lineW, y + h));
        DrawLineBase(ImVec2(x + w - lineW, y + h), ImVec2(x + w, y + h));
        DrawLineBase(ImVec2(x + w, y + h - lineH), ImVec2(x + w, y + h));
    }

    struct BoneConnection { int bone1; int bone2; };
    inline const std::vector<BoneConnection> skeletonBones = {
        { 6, 5 }, { 5, 4 }, { 4, 0 },
        { 5, 8 }, { 8, 9 }, { 9, 10 },
        { 5, 13 }, { 13, 14 }, { 14, 15 },
        { 0, 22 }, { 22, 23 }, { 23, 24 },
        { 0, 25 }, { 25, 26 }, { 26, 27 }
    };

    inline void Render()
    {
        if (!config.enabled) return;
        if (!Game::clientBase) return;

        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!draw) return;

        // Push selected ESP font
        int fi = config.espFont;
        bool fontPushed = false;
        if (fi >= 0 && fi < 3 && g_EspFonts[fi]) { ImGui::PushFont(g_EspFonts[fi]); fontPushed = true; }

        uintptr_t localCtrl = Game::Read<uintptr_t>(Game::clientBase + Offsets::dwLocalPlayerController);
        if (!localCtrl) { if (fontPushed) ImGui::PopFont(); return; }

        uint32_t localHandle = Game::Read<uint32_t>(localCtrl + Offsets::m_hPlayerPawn);
        uintptr_t localPawn = Game::GetEntityByHandle(localHandle);
        if (!localPawn) { if (fontPushed) ImGui::PopFont(); return; }

        int localTeam = Game::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);

        uintptr_t entityList = Game::GetEntityList();
        if (!entityList) { if (fontPushed) ImGui::PopFont(); return; }

        int highestIdx = Game::Read<int>(Game::clientBase + Offsets::dwGameEntitySystem_highestEntityIndex);
        if (highestIdx < 0 || highestIdx > 2048) highestIdx = 64;

        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        float screenW = displaySize.x;
        float screenH = displaySize.y;

        std::vector<std::string> spectatorList;

        if (config.bBombTimer)
        {
            uintptr_t pBombSystem = Game::Read<uintptr_t>(Game::clientBase + Offsets::dwPlantedC4);
            uintptr_t bomb = 0;
            if (pBombSystem)
                bomb = Game::Read<uintptr_t>(pBombSystem);

            if (bomb)
            {
                float currentTime = Game::Read<float>(localPawn + Offsets::m_flSimulationTime);
                uintptr_t bombNode = Game::Read<uintptr_t>(bomb + Offsets::m_pGameSceneNode);
                if (bombNode)
                {
                    Game::Vector3 bombOrigin = Game::Read<Game::Vector3>(bombNode + Offsets::m_vecAbsOrigin);
                    if (!bombOrigin.IsZero())
                    {
                        float bX, bY;
                        float bOrg[3] = { bombOrigin.x, bombOrigin.y, bombOrigin.z };
                        if (Game::WorldToScreen(bOrg, bX, bY, screenW, screenH))
                        {
                            bool isTicking = Game::Read<bool>(bomb + Offsets::m_bBombTicking);
                            if (isTicking)
                            {
                                float c4BlowTime = Game::Read<float>(bomb + Offsets::m_flC4Blow);
                                float remaining = c4BlowTime - currentTime;
                                if (remaining > 0)
                                {
                                    char bombText[64];
                                    snprintf(bombText, sizeof(bombText), "C4: %.1f", remaining);
                                    ImVec2 bSize = ImGui::CalcTextSize(bombText);
                                    ImVec2 bPos(bX - (bSize.x / 2.0f), bY);
                                    draw->AddText(ImVec2(bPos.x + 1, bPos.y + 1), IM_COL32(0, 0, 0, 255), bombText);
                                    draw->AddText(bPos, remaining < 10.0f ? IM_COL32(255, 0, 0, 255) : IM_COL32(255, 255, 0, 255), bombText);
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int i = 1; i <= 64; i++)
        {
            uintptr_t listEntry = Game::Read<uintptr_t>(entityList + (8 * (i >> 9)) + 0x10);
            if (!listEntry) continue;

            uintptr_t controller = Game::Read<uintptr_t>(listEntry + Game::ENTITY_IDENTITY_SIZE * (i & 0x1FF));
            if (!controller || controller == localCtrl) continue;

            bool isAlive = Game::Read<bool>(controller + Offsets::m_bPawnIsAlive);

            if (!isAlive)
            {
                if (config.bSpectators)
                {
                    uint32_t obsPawnHandle = Game::Read<uint32_t>(controller + Offsets::m_hObserverPawn);
                    if (obsPawnHandle != 0 && obsPawnHandle != 0xFFFFFFFF)
                    {
                        uintptr_t obsPawn = Game::GetEntityByHandle(obsPawnHandle);
                        if (obsPawn && obsPawn != localPawn)
                        {
                            uintptr_t obsServices = Game::Read<uintptr_t>(obsPawn + Offsets::m_pObserverServices);
                            if (obsServices)
                            {
                                uint32_t targetHandle = Game::Read<uint32_t>(obsServices + Offsets::m_hObserverTarget);
                                if (targetHandle > 0 && targetHandle == localHandle)
                                {
                                    uintptr_t namePtr = Game::Read<uintptr_t>(controller + Offsets::m_sSanitizedPlayerName);
                                    if (namePtr)
                                    {
                                        struct NameBuf { char data[32]; };
                                        NameBuf buf = Game::Read<NameBuf>(namePtr);
                                        buf.data[31] = '\0';
                                        std::string nameStr(buf.data);
                                        if (nameStr.length() > 0 && std::find(spectatorList.begin(), spectatorList.end(), nameStr) == spectatorList.end())
                                            spectatorList.push_back(nameStr);
                                    }
                                }
                            }
                        }
                    }
                }
                continue;
            }

            uint32_t pawnHandle = Game::Read<uint32_t>(controller + Offsets::m_hPlayerPawn);
            if (!pawnHandle) continue;

            uintptr_t pawn = Game::GetEntityByHandle(pawnHandle);
            if (!pawn || pawn == localPawn) continue;

            int health = Game::Read<int32_t>(pawn + Offsets::m_iHealth);
            if (health <= 0) continue;

            if (config.teamCheck)
            {
                int team = Game::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                if (team == localTeam) continue;
            }

            if (config.bChams)
            {
                uint8_t r = (uint8_t)(config.chamsColor[0] * 255.0f);
                uint8_t g = (uint8_t)(config.chamsColor[1] * 255.0f);
                uint8_t b = (uint8_t)(config.chamsColor[2] * 255.0f);
                uint8_t a = (uint8_t)(config.chamsColor[3] * 255.0f);
                uint32_t colorInt = r | (g << 8) | (b << 16) | (a << 24);

                // Render mode 1 = kRenderTransColor — fills model with solid color
                Game::Write<uint8_t>(pawn + Offsets::m_nRenderMode, 1);
                Game::Write<uint8_t>(pawn + Offsets::m_clrRender + 0, r);
                Game::Write<uint8_t>(pawn + Offsets::m_clrRender + 1, g);
                Game::Write<uint8_t>(pawn + Offsets::m_clrRender + 2, b);
                Game::Write<uint8_t>(pawn + Offsets::m_clrRender + 3, a);

                // Also enable glow for outline highlight
                Game::Write<bool>(pawn + Offsets::m_Glow + Offsets::m_bGlowing, true);
                Game::Write<int>(pawn + Offsets::m_Glow + Offsets::m_iGlowType, 3);
                Game::Write<uint32_t>(pawn + Offsets::m_Glow + Offsets::m_glowColorOverride, colorInt);
                Game::Write<float>(pawn + Offsets::m_flGlowBackfaceMult, 1.0f);
            }
            else if (config.bGlow)
            {
                uint8_t r = (uint8_t)(config.glowColor[0] * 255.0f);
                uint8_t g = (uint8_t)(config.glowColor[1] * 255.0f);
                uint8_t b = (uint8_t)(config.glowColor[2] * 255.0f);
                uint8_t a = (uint8_t)(config.glowColor[3] * 255.0f);
                uint32_t colorInt = r | (g << 8) | (b << 16) | (a << 24);

                Game::Write<bool>(pawn + Offsets::m_Glow + Offsets::m_bGlowing, true);
                Game::Write<int>(pawn + Offsets::m_Glow + Offsets::m_iGlowType, 3);
                Game::Write<uint32_t>(pawn + Offsets::m_Glow + Offsets::m_glowColorOverride, colorInt);
            }

            Game::Vector3 origin = Game::GetEntityOrigin(pawn);
            Game::Vector3 headPos = Game::GetBonePosition(pawn, 6);

            if (origin.IsZero() || headPos.IsZero()) continue;

            headPos.z += 8.0f;

            float orgX, orgY, headX, headY;
            float org[3] = { origin.x, origin.y, origin.z };
            float head[3] = { headPos.x, headPos.y, headPos.z };

            if (!Game::WorldToScreen(org, orgX, orgY, screenW, screenH)) continue;
            if (!Game::WorldToScreen(head, headX, headY, screenW, screenH)) continue;

            float boxHeight = orgY - headY;
            float boxWidth = boxHeight * 0.5f;

            if (config.bBox)
            {
                ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(config.boxColor[0], config.boxColor[1], config.boxColor[2], config.boxColor[3]));
                if (config.boxMode == 0)
                    DrawBox(draw, headX - (boxWidth / 2), headY, boxWidth, boxHeight, col);
                else
                    DrawCornerBox(draw, headX - (boxWidth / 2), headY, boxWidth, boxHeight, col);
            }

            if (config.bSkeleton)
            {
                ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(config.skeletonColor[0], config.skeletonColor[1], config.skeletonColor[2], config.skeletonColor[3]));
                
                for (const auto& conn : skeletonBones)
                {
                    Game::Vector3 b1 = Game::GetBonePosition(pawn, conn.bone1);
                    Game::Vector3 b2 = Game::GetBonePosition(pawn, conn.bone2);
                    if (b1.IsZero() || b2.IsZero()) continue;

                    float p1[3] = { b1.x, b1.y, b1.z };
                    float p2[3] = { b2.x, b2.y, b2.z };
                    float s1X, s1Y, s2X, s2Y;

                    if (Game::WorldToScreen(p1, s1X, s1Y, screenW, screenH) &&
                        Game::WorldToScreen(p2, s2X, s2Y, screenW, screenH))
                    {
                        DrawLine(draw, ImVec2(s1X, s1Y), ImVec2(s2X, s2Y), col, 1.0f);
                    }
                }
            }

            if (config.bHealthBar)
            {
                float hpHeight = boxHeight * (health / 100.0f);
                float hpMargin = 4.0f;
                float hpWidth = 2.0f;
                
                float r = health < 50 ? 255.0f : floorf(255.0f - (health * 2 - 100) * 255.0f / 100.0f);
                float g = health > 50 ? 255.0f : floorf((health * 2) * 255.0f / 100.0f);
                ImU32 hpColor = IM_COL32((int)r, (int)g, 0, 255);

                draw->AddRectFilled(ImVec2(headX - (boxWidth / 2) - hpMargin - hpWidth - 1, headY - 1),
                                    ImVec2(headX - (boxWidth / 2) - hpMargin + 1, headY + boxHeight + 1), 
                                    IM_COL32(0, 0, 0, 255));
                draw->AddRectFilled(ImVec2(headX - (boxWidth / 2) - hpMargin - hpWidth, headY + (boxHeight - hpHeight)),
                                    ImVec2(headX - (boxWidth / 2) - hpMargin, headY + boxHeight), 
                                    hpColor);
            }

            if (config.bName)
            {
                uintptr_t namePtr = Game::Read<uintptr_t>(controller + Offsets::m_sSanitizedPlayerName);
                if (namePtr)
                {
                    struct NameBuf { char data[32]; };
                    NameBuf buf = Game::Read<NameBuf>(namePtr);
                    buf.data[31] = '\0';

                    if (buf.data[0] != '\0')
                    {
                        ImVec2 textSize = ImGui::CalcTextSize(buf.data);
                        ImVec2 textPos(headX - (textSize.x / 2.0f), headY - textSize.y - 4.0f);
                        
                        draw->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), buf.data);
                        draw->AddText(ImVec2(textPos.x - 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), buf.data);
                        draw->AddText(ImVec2(textPos.x + 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), buf.data);
                        draw->AddText(ImVec2(textPos.x - 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), buf.data);
                        draw->AddText(textPos, IM_COL32(255, 255, 255, 255), buf.data);
                    }
                }
            }

            if (config.bDistance)
            {
                Game::Vector3 localOrigin = Game::GetEntityOrigin(localPawn);
                if (!localOrigin.IsZero())
                {
                    float distUnits = (origin - localOrigin).Length2D();
                    float distMeters = distUnits * 0.0254f;

                    char distBuf[32];
                    snprintf(distBuf, sizeof(distBuf), "[ %.0fm ]", distMeters);

                    ImVec2 textSize = ImGui::CalcTextSize(distBuf);
                    ImVec2 textPos(headX - (textSize.x / 2.0f), headY + boxHeight + 4.0f);

                    draw->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(ImVec2(textPos.x - 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(ImVec2(textPos.x + 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(ImVec2(textPos.x - 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(textPos, IM_COL32(255, 255, 255, 255), distBuf);
                }
            }
        }

        if (config.bSpectators && !spectatorList.empty())
        {
            float specY = 20.0f;
            for (const auto& speccer : spectatorList)
            {
                char specText[64];
                snprintf(specText, sizeof(specText), "[Spectator] %s", speccer.c_str());
                draw->AddText(ImVec2(10.0f + 1, specY + 1), IM_COL32(0, 0, 0, 255), specText);
                draw->AddText(ImVec2(10.0f, specY), IM_COL32(255, 0, 0, 255), specText);
                specY += 15.0f;
            }
        }

        if (fontPushed) ImGui::PopFont();
    }
}
