#pragma once
#include <Windows.h>
#include <vector>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "../imgui/imgui.h"
#include <algorithm>
#include <string>
#include <cctype>

extern ImFont* g_EspFonts[3];

namespace ESP
{
    struct Config
    {
        bool enabled = true;
        bool bBox = true;
        int boxType = 0;
        bool bSkeleton = true;
        bool bHealthBar = true;
        bool bName = true;
        bool bDistance = true;
        bool teamCheck = true;
        float teamColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
        float enemyColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        float skeletonColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Preview Calibration
        bool bShowCalibration = false;
        float previewWidthScale = 0.35f;
        float previewHeightScale = 0.95f;
        float previewYOffset = 0.12f;

        bool bGlow = false;
        float glowColor[4] = { 1.0f, 0.0f, 0.0f, 0.6f };
        bool bChams = false;
        bool chamsVisible = true;
        bool chamsOccluded = true;
        int chamsStyleEnemies = 1; // 0=Flat, 1=Regular, 2=Illumin, 3=Glow, 4=Latex, 5=Glass, 6=Golden, 7=Bubble
        int chamsStyleTeam = 1;
        int chamsStyleLocal = 1;
        float chamsColorVisible[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
        float chamsColorOccluded[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
        bool teamChams = false;
        bool localChams = false;
        bool bBombTimer = false;
        bool bSpectators = false;
        float droppedColor[4] = { 1.0f, 1.0f, 1.0f, 0.7f };
        bool bDroppedWeapons = true;
        bool bWeapon = true;
        bool bWeaponIcon = true;
        bool bHitmarker = false;
        bool bHitsound = false;
        int espFont = 1; // 0=Pixel, 1=Verdana, 2=Hax
        bool bDroppedDistance = true;
        int nKnifeModel = 0;
        int nPlayerModel = 0;
        int nGloveModel = 0;
    };

    inline Config config;
    inline float lastHitTime = 0.0f;

    inline void DrawLine(ImDrawList* draw, ImVec2 a, ImVec2 b, ImU32 col, float thickness)
    {
        draw->AddLine(a, b, col, thickness);
    }

    inline void DrawBox(ImDrawList* draw, float x, float y, float w, float h, ImU32 col)
    {
        // Triple-layer technique for pixel-perfect contrast
        // 1. Black outer
        draw->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), IM_COL32(0, 0, 0, 200), 0.0f, 0, 1.0f);
        // 2. Black inner
        draw->AddRect(ImVec2(x + 1, y + 1), ImVec2(x + w - 1, y + h - 1), IM_COL32(0, 0, 0, 200), 0.0f, 0, 1.0f);
        // 3. Main colored line
        draw->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), col, 0.0f, 0, 1.0f);
    }

    inline void DrawCornerBox(ImDrawList* draw, float x, float y, float w, float h, ImU32 col)
    {
        float lineW = w / 4.0f;
        float lineH = h / 4.0f;

        auto DrawLineBase = [&](ImVec2 p1, ImVec2 p2) {
            // Outline contrast
            draw->AddLine(p1, p2, IM_COL32(0, 0, 0, 200), 3.0f);
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

    inline std::string CleanDesignerName(std::string name)
    {
        if (name.empty()) return "UNKNOWN";
        
        // Remove "weapon_" or "item_" prefix
        size_t off = name.find("_");
        if (off != std::string::npos)
            name = name.substr(off + 1);
            
        // Uppercase everything
        for (auto& c : name) c = (char)toupper((unsigned char)c);
        
        // Specific cleanup for common items
        if (name == "SMOKEGRENADE") return "SMOKE";
        if (name == "FLASHBANG") return "FLASH";
        if (name == "HEGRENADE") return "HE";
        if (name == "MOLOTOV") return "FIRE";
        if (name == "DEAGLE") return "DEAGLE";
        if (name == "USP_SILENCER") return "USP-S";
        if (name == "M4A1_SILENCER") return "M4A1-S";
        
        return name;
    }

    inline const char* GetWeaponName(int id)
    {
        switch (id)
        {
        case 1: return "DEAGLE"; case 2: return "DUALIES"; case 3: return "FIVE-SEVEN"; case 4: return "GLOCK";
        case 7: return "AK-47"; case 8: return "AUG"; case 9: return "AWP"; case 10: return "FAMAS";
        case 11: return "G3SG1"; case 13: return "GALIL"; case 14: return "M249"; case 16: return "M4A4";
        case 17: return "MAC-10"; case 19: return "P90"; case 23: return "MP5-SD"; case 24: return "UMP-45";
        case 25: return "XM1014"; case 26: return "BIZON"; case 27: return "MAG-7"; case 28: return "NEGEV";
        case 29: return "SAWED-OFF"; case 30: return "TEC-9"; case 31: return "ZEUS"; case 32: return "P2000";
        case 33: return "P250"; case 34: return "MP7"; case 35: return "MP9"; case 36: return "NOVA";
        case 38: return "SCAR-20"; case 39: return "SG553"; case 40: return "SSG08"; case 60: return "M4A1-S";
        case 61: return "USP-S"; case 63: return "CZ75-A"; case 64: return "REVOLVER";
        case 43: case 44: case 45: case 46: case 47: case 48: return "GRENADE";
        case 49: return "C4";
        default: return "KNIFE";
        }
    }

    inline const char* GetWeaponIcon(int id)
    {
        switch (id)
        {
        case 1:  return "A"; // deagle
        case 2:  return "B"; // dualies
        case 3:  return "C"; // fiveseven
        case 4:  return "D"; // glock
        case 7:  return "W"; // ak47
        case 8:  return "U"; // aug
        case 9:  return "Z"; // awp
        case 10: return "R"; // famas
        case 11: return "X"; // g3sg1
        case 13: return "Q"; // galil
        case 14: return "g"; // m249
        case 16: return "S"; // m4a4
        case 17: return "K"; // mac10
        case 19: return "P"; // p90
        case 23: return "x"; // mp5sd
        case 24: return "L"; // ump45
        case 25: return "b"; // xm1014
        case 26: return "M"; // bizon
        case 27: return "d"; // mag7
        case 28: return "f"; // negev
        case 29: return "c"; // sawedoff
        case 30: return "H"; // tec9
        case 31: return "h"; // taser
        case 32: return "E"; // p2000
        case 33: return "N"; // mp7
        case 34: return "O"; // mp9
        case 35: return "O"; // mp9
        case 36: return "F"; // p250
        case 38: return "Y"; // scar20
        case 39: return "V"; // sg556
        case 40: return "a"; // ssg08
        case 42: return "["; // knife
        case 60: return "T"; // m4a1s
        case 61: return "G"; // usps
        case 63: return "I"; // cz75
        case 64: return "J"; // r8
        case 500: return "["; // knife
        default: return "W";
        }
    }

    inline const char* GetWeaponIconByDesignerName(const char* name)
    {
        if (!name) return "";
        std::string d = name;
        if (d.find("ak47") != std::string::npos) return "W";
        if (d.find("awp") != std::string::npos) return "Z";
        if (d.find("ssg08") != std::string::npos) return "a";
        if (d.find("m4a1_s") != std::string::npos || d.find("m4a1_silencer") != std::string::npos) return "T";
        if (d.find("m4a1") != std::string::npos) return "S"; // M4A4 in some dumps is called m4a1
        if (d.find("m4a4") != std::string::npos) return "S";
        if (d.find("deagle") != std::string::npos) return "A";
        if (d.find("elite") != std::string::npos) return "B";
        if (d.find("fiveseven") != std::string::npos) return "C";
        if (d.find("glock") != std::string::npos) return "D";
        if (d.find("p2000") != std::string::npos) return "E";
        if (d.find("p250") != std::string::npos) return "F";
        if (d.find("usp_s") != std::string::npos || d.find("usp_silencer") != std::string::npos) return "G";
        if (d.find("usps") != std::string::npos) return "G";
        if (d.find("tec9") != std::string::npos) return "H";
        if (d.find("cz75") != std::string::npos) return "I";
        if (d.find("revolver") != std::string::npos) return "J";
        if (d.find("mac10") != std::string::npos) return "K";
        if (d.find("ump45") != std::string::npos) return "L";
        if (d.find("bizon") != std::string::npos) return "M";
        if (d.find("mp7") != std::string::npos) return "N";
        if (d.find("mp9") != std::string::npos) return "O";
        if (d.find("mp5") != std::string::npos) return "x";
        if (d.find("p90") != std::string::npos) return "P";
        if (d.find("galil") != std::string::npos) return "Q";
        if (d.find("famas") != std::string::npos) return "R";
        if (d.find("aug") != std::string::npos) return "U";
        if (d.find("sg556") != std::string::npos) return "V";
        if (d.find("g3sg1") != std::string::npos) return "X";
        if (d.find("scar20") != std::string::npos) return "Y";
        if (d.find("xm1014") != std::string::npos) return "b";
        if (d.find("sawedoff") != std::string::npos) return "c";
        if (d.find("mag7") != std::string::npos) return "d";
        if (d.find("nova") != std::string::npos) return "e";
        if (d.find("negev") != std::string::npos) return "f";
        if (d.find("m249") != std::string::npos) return "g";
        if (d.find("taser") != std::string::npos) return "h";
        if (d.find("flashbang") != std::string::npos) return "i";
        if (d.find("hegrenade") != std::string::npos) return "j";
        if (d.find("smokegrenade") != std::string::npos) return "k";
        if (d.find("molotov") != std::string::npos) return "l";
        if (d.find("decoy") != std::string::npos) return "m";
        if (d.find("incendiary") != std::string::npos) return "n";
        if (d.find("c4") != std::string::npos) return "o";
        if (d.find("defuse") != std::string::npos) return "r";
        if (d.find("knife") != std::string::npos) return "[";
        return "W"; 
    }

    // --- Bomb Info Window (Draggable) ---
    namespace BombUI 
    {
        inline float bombTimer = 40.0f;
    }

    inline void RenderBombWindow(uintptr_t bomb)
    {
        if (!config.bBombTimer || !bomb) return;

        bool isTicking = Game::Read<bool>(bomb + Offsets::m_bBombTicking);
        if (!isTicking) return;

        uintptr_t localPawn = Game::GetLocalPlayerPawn();
        float currentTime = Game::Read<float>(localPawn + Offsets::m_flSimulationTime);
        float c4BlowTime = Game::Read<float>(bomb + Offsets::m_flC4Blow);
        float remaining = c4BlowTime - currentTime;

        if (remaining <= 0) return;

        // Custom Window for Bomb Status
        ImGui::Begin("BOMB_WINDOW", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
        {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
            
            // Layout matching reference image
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcTextSize("Bomb Timer").x * 0.5f);
            ImGui::Text("Bomb Timer");
            
            // Site Check (Simple heuristic based on bomb position in typical maps or can be zeroed)
            Game::Vector3 bombPos = Game::GetEntityOrigin(bomb);
            const char* siteStr = "A Site"; // Dynamic detection requires GameRules but heuristic is often enough for UI
            
            char infoBuf[64];
            snprintf(infoBuf, sizeof(infoBuf), "%.1fs - %s", remaining, siteStr);
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcTextSize(infoBuf).x * 0.5f);
            ImGui::Text("%s", infoBuf);

            // Bomb Progress Bar
            float bombFrac = std::clamp(remaining / 40.0f, 0.0f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
            ImGui::ProgressBar(bombFrac, ImVec2(180, 12), "");
            ImGui::PopStyleColor();

            // Defuse Bar (If active)
            float defuseRemaining = Game::Read<float>(bomb + Offsets::m_flDefuseCountDown) - currentTime;
            if (defuseRemaining > 0)
            {
                char defuseBuf[32];
                snprintf(defuseBuf, sizeof(defuseBuf), "Defuse in %.1fs", defuseRemaining);
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcTextSize(defuseBuf).x * 0.5f);
                ImGui::Text("%s", defuseBuf);

                float defuseMax = Game::Read<float>(bomb + Offsets::m_flDefuseLength);
                float defuseFrac = std::clamp(defuseRemaining / (defuseMax > 0 ? defuseMax : 10.0f), 0.0f, 1.0f);
                
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
                ImGui::ProgressBar(defuseFrac, ImVec2(180, 12), "");
                ImGui::PopStyleColor();
            }

            // Lethal Indicator
            Game::Vector3 localOrigin = Game::GetEntityOrigin(localPawn);
            float dist = (bombPos - localOrigin).Length();
            bool lethal = (dist < 500.0f); // Approximate blast radius in units
            
            ImGui::Spacing();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcTextSize(lethal ? "Lethal" : "Safe").x * 0.5f);
            if (lethal) ImGui::TextColored(ImVec4(1,1,1,1), "Lethal");
            else        ImGui::TextColored(ImVec4(1,1,1,1), "Safe");

            ImGui::End();
        }
    }

    // --- Visuals Preview System (Precision Silhouette Alignment) ---
    inline void RenderPreview(ImDrawList* draw, ImVec2 pos, ImVec2 size)
    {
        // Use user-defined calibration scales
        float bw = size.x * config.previewWidthScale; 
        float bh = size.y * config.previewHeightScale; 
        float bx = pos.x + (size.x - bw) * 0.5f; // Centered X
        float by = pos.y + (size.y - bh) * config.previewYOffset;

        ImU32 enemyCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)config.enemyColor);
        ImU32 textCol = IM_COL32(255, 255, 255, 255);

        // 1. Box Overlay (Supports Bounding & Corner)
        if (config.bBox) {
            if (config.boxType == 0) // Bounding
                DrawBox(draw, bx, by, bw, bh, enemyCol);
            else // Corner
                DrawCornerBox(draw, bx, by, bw, bh, enemyCol);
        }

        // 2. Multi-Part Skeleton Dummy (Simplified for Preview)
        if (config.bSkeleton) {
            float cx = bx + bw * 0.5f;
            float shY = by + bh * 0.16f; // Calibrated Shoulder Y
            draw->AddLine(ImVec2(cx, by + bh * 0.04f), ImVec2(cx, by + bh * 0.45f), enemyCol, 1.5f); // Spine
            draw->AddLine(ImVec2(bx + bw * 0.10f, shY), ImVec2(bx + bw * 0.90f, shY), enemyCol, 1.5f); // Shoulders
            draw->AddLine(ImVec2(cx, by + bh * 0.45f), ImVec2(bx + bw * 0.30f, by + bh * 0.96f), enemyCol, 1.5f); // Left Leg
            draw->AddLine(ImVec2(cx, by + bh * 0.45f), ImVec2(bx + bw * 0.70f, by + bh * 0.96f), enemyCol, 1.5f); // Right Leg
        }

        // 3. Precision Healthbar
        if (config.bHealthBar) {
            float hpMargin = 6.0f;
            float hpWidth = 2.0f;
            
            // Background
            draw->AddRectFilled(ImVec2(bx - hpMargin - 1, by - 1), ImVec2(bx - hpMargin + hpWidth + 1, by + bh + 1), IM_COL32(0, 0, 0, 180));
            // Bar (Static at 85% for preview)
            draw->AddRectFilled(ImVec2(bx - hpMargin, by + bh * 0.15f), ImVec2(bx - hpMargin + hpWidth, by + bh), IM_COL32(0, 255, 120, 255));
        }

        // 4. labels & Metadata
        float labelOffset = 0.0f;

        if (config.bName) {
            const char* name = "PREVIEW_TARGET";
            ImVec2 sz = ImGui::CalcTextSize(name);
            draw->AddText(ImVec2(bx + bw/2 - sz.x/2, by - sz.y - 4), textCol, name);
        }

        if (config.bWeapon) {
            const char* wp = "AK-47 [ICON]";
            ImVec2 sz = ImGui::CalcTextSize(wp);
            draw->AddText(ImVec2(bx + bw/2 - sz.x/2, by + bh + 2), textCol, wp);
            labelOffset += sz.y + 2;
        }

        if (config.bDistance) {
            const char* dist = "[ 12m ]";
            ImVec2 sz = ImGui::CalcTextSize(dist);
            draw->AddText(ImVec2(bx + bw/2 - sz.x/2, by + bh + 2 + labelOffset), textCol, dist);
        }
    }

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
            if (pBombSystem)
            {
                uintptr_t bomb = Game::Read<uintptr_t>(pBombSystem);
                if (bomb) RenderBombWindow(bomb);
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
                                        
                                        // Profile simulation (appending ID)
                                        uint64_t steamId = Game::Read<uint64_t>(controller + 0x6C0); // m_steamID
                                        char fullInfo[128];
                                        snprintf(fullInfo, sizeof(fullInfo), "%s [%llu]", buf.data, steamId);
                                        
                                        std::string infoStr(fullInfo);
                                        if (infoStr.length() > 0 && std::find(spectatorList.begin(), spectatorList.end(), infoStr) == spectatorList.end())
                                            spectatorList.push_back(infoStr);
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
            
            int team = Game::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
            bool isEnemy = (team != localTeam);

            if (config.teamCheck && !isEnemy) continue;

            // --- Hit Feedback ---
            static std::map<uintptr_t, int> lastHealth;
            if (health < lastHealth[pawn] && health > 0)
            {
                if (isEnemy) 
                {
                    if (config.bHitsound) Beep(800, 30);
                    if (config.bHitmarker) lastHitTime = (float)ImGui::GetTime();
                }
            }
            lastHealth[pawn] = health;

            ImU32 playerCol = isEnemy ? 
                ImGui::ColorConvertFloat4ToU32(*(ImVec4*)config.enemyColor) : 
                ImGui::ColorConvertFloat4ToU32(*(ImVec4*)config.teamColor);

            // Chams logic removed - now handled by DrawObject hook in chams.h
                // Reset Glow state
                if (!config.bGlow)
                    Game::Write<bool>(pawn + Offsets::m_Glow + Offsets::m_bGlowing, false);
            if (config.bGlow && !config.bChams)
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
            ImVec2 boxPos(headX - (boxWidth / 2), headY);
            ImVec2 boxSize(boxWidth, boxHeight);
            float offset = 0.0f;

            if (config.bBox)
            {
                if (config.boxType == 0)
                    DrawBox(draw, boxPos.x, boxPos.y, boxSize.x, boxSize.y, playerCol);
                else
                    DrawCornerBox(draw, boxPos.x, boxPos.y, boxSize.x, boxSize.y, playerCol);
            }

            if (config.bSkeleton)
            {
                ImU32 skelCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)config.skeletonColor);
                ImU32 glowCol = IM_COL32(0, 0, 0, 180); 

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
                        draw->AddLine(ImVec2(s1X, s1Y), ImVec2(s2X, s2Y), glowCol, 2.5f);
                        draw->AddLine(ImVec2(s1X, s1Y), ImVec2(s2X, s2Y), skelCol, 1.0f);
                    }
                }
            }

            if (config.bHealthBar)
            {
                // Vertical health bar on left (Ultra-Clean)
                float hpHeight = boxHeight * (health / 100.0f);
                float hpMargin = 6.0f;
                float hpWidth = 2.0f;
                
                float r = health < 50 ? 255.0f : floorf(255.0f - (health * 2 - 100) * 255.0f / 100.0f);
                float g = health > 50 ? 255.0f : floorf((health * 2) * 255.0f / 100.0f);
                ImU32 hpColor = IM_COL32((int)r, (int)g, 20, 255);

                // Background
                draw->AddRectFilled(ImVec2(boxPos.x - hpMargin - 1, boxPos.y - 1),
                                    ImVec2(boxPos.x - hpMargin + hpWidth + 1, boxPos.y + boxHeight + 1), 
                                    IM_COL32(0, 0, 0, 180));
                
                // Color fill
                draw->AddRectFilled(ImVec2(boxPos.x - hpMargin, boxPos.y + (boxHeight - hpHeight)),
                                    ImVec2(boxPos.x - hpMargin + hpWidth, boxPos.y + boxHeight), 
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
                        // Structure: UPPERCASE + Shadow
                        std::string nameStr(buf.data);
                        for (auto& c : nameStr) c = (char)toupper((unsigned char)c);
                        
                        ImVec2 textSize = ImGui::CalcTextSize(nameStr.c_str());
                        ImVec2 textPos(boxPos.x + (boxSize.x * 0.5f) - (textSize.x / 2.0f), boxPos.y - textSize.y - 4.0f);
                        
                        // 4-way shadowing for max contrast
                        draw->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), nameStr.c_str());
                        draw->AddText(ImVec2(textPos.x - 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), nameStr.c_str());
                        draw->AddText(ImVec2(textPos.x + 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), nameStr.c_str());
                        draw->AddText(ImVec2(textPos.x - 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), nameStr.c_str());
                        draw->AddText(textPos, IM_COL32(255, 255, 255, 255), nameStr.c_str());
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
                    ImVec2 textPos(boxPos.x + (boxSize.x * 0.5f) - (textSize.x / 2.0f), boxPos.y + boxSize.y + offset + 4.0f);

                    draw->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(ImVec2(textPos.x - 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(ImVec2(textPos.x + 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(ImVec2(textPos.x - 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), distBuf);
                    draw->AddText(textPos, IM_COL32(255, 255, 255, 255), distBuf);
                    offset += textSize.y + 4.0f;
                }
            }

            if (config.bWeapon)
            {
                uintptr_t weaponServices = Game::Read<uintptr_t>(pawn + Offsets::m_pWeaponServices);
                if (weaponServices)
                {
                    uint32_t weaponHandle = Game::Read<uint32_t>(weaponServices + Offsets::m_hActiveWeapon);
                    uintptr_t activeWeapon = Game::GetEntityByHandle(weaponHandle);
                    if (activeWeapon)
                    {
                        std::string designerName = Game::GetEntityDesignerName(activeWeapon);
                        
                        if (config.bWeaponIcon && iconFont)
                        {
                            const char* weaponIcon = GetWeaponIconByDesignerName(designerName.c_str());
                            ImGui::PushFont(iconFont);
                            float iconWidth = ImGui::CalcTextSize(weaponIcon).x;
                            draw->AddText(ImVec2(boxPos.x + (boxSize.x * 0.5f) - (iconWidth * 0.5f) + 1, boxPos.y + boxSize.y + offset + 1), IM_COL32(0, 0, 0, 255), weaponIcon);
                            draw->AddText(ImVec2(boxPos.x + (boxSize.x * 0.5f) - (iconWidth * 0.5f), boxPos.y + boxSize.y + offset), IM_COL32(255, 255, 255, 255), weaponIcon);
                            ImGui::PopFont();
                            offset += 18.0f;
                        }
                        
                        std::string cleanName = CleanDesignerName(designerName);
                        ImVec2 textSize = ImGui::CalcTextSize(cleanName.c_str());
                        ImVec2 textPos(boxPos.x + (boxSize.x * 0.5f) - (textSize.x / 2.0f), boxPos.y + boxSize.y + offset);

                        // Professional Shadow (4-way)
                        draw->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), cleanName.c_str());
                        draw->AddText(ImVec2(textPos.x - 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), cleanName.c_str());
                        draw->AddText(ImVec2(textPos.x + 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), cleanName.c_str());
                        draw->AddText(ImVec2(textPos.x - 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), cleanName.c_str());
                        draw->AddText(textPos, IM_COL32(230, 230, 230, 255), cleanName.c_str());
                    }
                }
            }
        }

        // Dropped Weapons Loop
        if (config.bDroppedWeapons)
        {
            Game::Vector3 localOrigin = Game::GetEntityOrigin(localPawn);
            for (int i = 65; i <= 1024; i++) // Fixed iteration range
            {
                uintptr_t listEntry = Game::Read<uintptr_t>(entityList + (8 * (i >> 9)) + 0x10);
                if (!listEntry) continue;

                uintptr_t entity = Game::Read<uintptr_t>(listEntry + Game::ENTITY_IDENTITY_SIZE * (i & 0x1FF));
                if (!entity) continue;

                std::string designerName = Game::GetEntityDesignerName(entity);
                if (designerName.empty()) continue;

                bool isWeapon = designerName.find("weapon_") != std::string::npos;
                bool isItem = designerName.find("item_") != std::string::npos;
                bool isGrenade = designerName.find("grenade") != std::string::npos;

                if (!isWeapon && !isItem && !isGrenade) continue;

                uint32_t owner = Game::Read<uint32_t>(entity + Offsets::m_hOwnerEntity);
                if (owner != 0 && owner != 0xFFFFFFFF) continue;

                // Use GameSceneNode for world-space coordinates (Referenced from FOV_Changer_Source)
                uintptr_t node = Game::Read<uintptr_t>(entity + Offsets::m_pGameSceneNode);
                if (!node) continue;

                Game::Vector3 worldPos = Game::Read<Game::Vector3>(node + Offsets::m_vecAbsOrigin);
                if (worldPos.IsZero()) continue;


                float scrX, scrY;
                float world[3] = { worldPos.x, worldPos.y, worldPos.z };
                if (Game::WorldToScreen(world, scrX, scrY, screenW, screenH))
                {
                    const char* icon = GetWeaponIconByDesignerName(designerName.c_str());
                    ImU32 dCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)config.droppedColor);

                    if (config.bWeaponIcon && iconFont)
                    {
                        ImGui::PushFont(iconFont);
                        float iconWidth = ImGui::CalcTextSize(icon).x;
                        draw->AddText(ImVec2(scrX - (iconWidth * 0.5f) + 1, scrY + 1), IM_COL32(0, 0, 0, 255), icon);
                        draw->AddText(ImVec2(scrX - (iconWidth * 0.5f), scrY), dCol, icon);
                        ImGui::PopFont();

                        if (config.bDroppedDistance)
                        {
                            Game::Vector3 diff = { worldPos.x - localOrigin.x, worldPos.y - localOrigin.y, worldPos.z - localOrigin.z };
                            float distMeters = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z) * 0.0254f;
                            char distBuf[32];
                            sprintf_s(distBuf, "[%.1fm]", distMeters);
                            float distW = ImGui::CalcTextSize(distBuf).x;
                            draw->AddText(ImVec2(scrX - (distW * 0.5f) + 1, scrY + 13), IM_COL32(0, 0, 0, 255), distBuf);
                            draw->AddText(ImVec2(scrX - (distW * 0.5f), scrY + 12), dCol, distBuf);
                        }
                    }
                    else
                    {
                        std::string cleanName = CleanDesignerName(designerName);
                        float nameWidth = ImGui::CalcTextSize(cleanName.c_str()).x;
                        draw->AddText(ImVec2(scrX - (nameWidth * 0.5f) + 1, scrY + 1), IM_COL32(0, 0, 0, 255), cleanName.c_str());
                        draw->AddText(ImVec2(scrX - (nameWidth * 0.5f), scrY), dCol, cleanName.c_str());
                        
                        if (config.bDroppedDistance)
                        {
                            Game::Vector3 diff = { worldPos.x - localOrigin.x, worldPos.y - localOrigin.y, worldPos.z - localOrigin.z };
                            float distMeters = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z) * 0.0254f;
                            char distBuf[32];
                            sprintf_s(distBuf, "[%.1fm]", distMeters);
                            float distW = ImGui::CalcTextSize(distBuf).x;
                            draw->AddText(ImVec2(scrX - (distW * 0.5f) + 1, scrY + 13), IM_COL32(0, 0, 0, 255), distBuf);
                            draw->AddText(ImVec2(scrX - (distW * 0.5f), scrY + 12), dCol, distBuf);
                        }
                    }
                }
            }
        }

        if (config.bSpectators && !spectatorList.empty())
        {
            ImGui::Begin("SPECTATOR_LIST", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
            {
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcTextSize("Spectators").x * 0.5f);
                ImGui::Text("Spectators");
                ImGui::Separator();
                
                for (const auto& speccer : spectatorList)
                {
                    // Clean name (removing the steam ID for visual parity with screenshot)
                    std::string cleanName = speccer;
                    size_t bracket = cleanName.find(" [");
                    if (bracket != std::string::npos)
                        cleanName = cleanName.substr(0, bracket);
                        
                    ImGui::Text("%s", cleanName.c_str());
                }
            }
            ImGui::End();
        }

        if (config.bHitmarker && (ImGui::GetTime() - lastHitTime) < 0.25f)
        {
            float mX = (float)screenW / 2.0f;
            float mY = (float)screenH / 2.0f;
            float size = 6.0f;
            draw->AddLine(ImVec2(mX - size, mY - size), ImVec2(mX - (size/2), mY - (size/2)), IM_COL32(255, 255, 255, 255), 1.5f);
            draw->AddLine(ImVec2(mX + size, mY - size), ImVec2(mX + (size/2), mY - (size/2)), IM_COL32(255, 255, 255, 255), 1.5f);
            draw->AddLine(ImVec2(mX - size, mY + size), ImVec2(mX - (size/2), mY + (size/2)), IM_COL32(255, 255, 255, 255), 1.5f);
            draw->AddLine(ImVec2(mX + size, mY + size), ImVec2(mX + (size/2), mY + (size/2)), IM_COL32(255, 255, 255, 255), 1.5f);
        }

        if (fontPushed) ImGui::PopFont();
    }
}
