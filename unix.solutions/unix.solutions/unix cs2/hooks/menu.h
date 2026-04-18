#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../features/skin_changer.h"
#include "../features/esp.h"
#include "../features/aimbot.h"
#include "../features/triggerbot.h"
#include "../features/bhop.h"
#include "../features/bullet_tracer.h"
#include "../features/paint_kits.h"
#include "../features/spinbot.h"
#include "../features/config.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <map>
#include <wincodec.h>
#include "EspPreview.h"

#pragma comment(lib, "Windowscodecs.lib")

// Forward declare globals from present.h
extern bool g_RequestUnload;
extern bool g_ViewFovEnabled;
extern int  g_ViewFov;

namespace Hooks
{
    inline int M_ICurrentPage = 1;
    inline int selectedWeaponIdx = 0;

    // Color scheme: index into preset or custom
    inline int g_SchemeIdx = 6; // Default to Cyberpunk
    inline ImVec4 g_Accent = ImVec4(1.0f, 0.08f, 0.58f, 1.0f); // DeepPink

    struct ColorScheme { const char* name; ImVec4 color; };
    inline ColorScheme g_Schemes[] = {
        { "White",   ImVec4(1.0f, 1.0f, 1.0f, 1.0f) },
        { "Red",     ImVec4(1.0f, 0.29f, 0.32f, 1.0f) },
        { "Cyan",    ImVec4(0.0f, 0.85f, 1.0f, 1.0f) },
        { "Green",   ImVec4(0.3f, 1.0f, 0.3f, 1.0f) },
        { "Purple",  ImVec4(0.7f, 0.3f, 1.0f, 1.0f) },
        { "Orange",  ImVec4(1.0f, 0.6f, 0.2f, 1.0f) },
        { "Cyberpunk", ImVec4(1.0f, 0.08f, 0.58f, 1.0f) },
    };
    inline constexpr int g_SchemeCount = sizeof(g_Schemes) / sizeof(g_Schemes[0]);

    inline ImU32 AccentU32(float a = 1.0f) {
        return IM_COL32((int)(g_Accent.x*255), (int)(g_Accent.y*255), (int)(g_Accent.z*255), (int)(a*255));
    }

    inline ID3D11ShaderResourceView* g_pEspPreviewTexture = nullptr;
    inline bool g_EspPreviewLoadingFailed = false;

    inline void LoadEspPreviewTexture(ID3D11Device* pDevice)
    {
        if (g_pEspPreviewTexture || !pDevice) return;

        // Diagnostic: Start loading
        static bool bLoggedAttempt = false;
        if (!bLoggedAttempt) { bLoggedAttempt = true; }

        CoInitialize(nullptr);

        IWICImagingFactory* pFactory = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr)) { g_EspPreviewLoadingFailed = true; Beep(400, 100); return; }

        IWICStream* pStream = nullptr;
        hr = pFactory->CreateStream(&pStream);
        if (SUCCEEDED(hr)) hr = pStream->InitializeFromMemory((BYTE*)EspPreview, sizeof(EspPreview));
        
        IWICBitmapDecoder* pDecoder = nullptr;
        if (SUCCEEDED(hr)) hr = pFactory->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnDemand, &pDecoder);
        
        IWICBitmapFrameDecode* pFrame = nullptr;
        if (SUCCEEDED(hr)) hr = pDecoder->GetFrame(0, &pFrame);
        
        IWICFormatConverter* pConverter = nullptr;
        if (SUCCEEDED(hr)) hr = pFactory->CreateFormatConverter(&pConverter);
        if (SUCCEEDED(hr)) hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
        
        UINT width, height;
        if (SUCCEEDED(hr)) hr = pConverter->GetSize(&width, &height);
        
        if (SUCCEEDED(hr))
        {
            std::vector<unsigned char> buffer(width * height * 4);
            hr = pConverter->CopyPixels(nullptr, width * 4, (UINT)buffer.size(), buffer.data());
            
            if (SUCCEEDED(hr))
            {
                D3D11_TEXTURE2D_DESC desc{};
                desc.Width = width;
                desc.Height = height;
                desc.MipLevels = 1;
                desc.ArraySize = 1;
                desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                desc.SampleDesc.Count = 1;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                
                D3D11_SUBRESOURCE_DATA sub{};
                sub.pSysMem = buffer.data();
                sub.SysMemPitch = width * 4;
                
                ID3D11Texture2D* pTexture = nullptr;
                hr = pDevice->CreateTexture2D(&desc, &sub, &pTexture);
                if (SUCCEEDED(hr))
                {
                    hr = pDevice->CreateShaderResourceView(pTexture, nullptr, &g_pEspPreviewTexture);
                    pTexture->Release();
                }
            }
        }

        if (SUCCEEDED(hr) && g_pEspPreviewTexture) {
            Beep(2000, 100); 
        } else {
            g_EspPreviewLoadingFailed = true;
            Beep(500, 100);
        }

        if (pConverter) pConverter->Release();
        if (pFrame) pFrame->Release();
        if (pDecoder) pDecoder->Release();
        if (pStream) pStream->Release();
        if (pFactory) pFactory->Release();
    }

    inline const char* Tabs[] = { "Rage", "Visuals", "Chams", "Misc", "Skins", "Settings" };
    inline constexpr int TabCount = 6;

    struct WeaponEntry { int defIndex; const char* name; };
    inline WeaponEntry weaponList[] = {
        { 7,  "AK-47" }, { 8,  "AUG" }, { 9,  "AWP" },
        { 1,  "Desert Eagle" }, { 2,  "Dual Berettas" },
        { 10, "FAMAS" }, { 3,  "Five-SeveN" }, { 13, "Galil AR" },
        { 11, "G3SG1" }, { 4,  "Glock-18" }, { 14, "M249" },
        { 16, "M4A4" }, { 60, "M4A1-S" }, { 17, "MAC-10" },
        { 27, "MAG-7" }, { 33, "MP7" }, { 34, "MP9" },
        { 23, "MP5-SD" }, { 28, "Negev" }, { 35, "Nova" },
        { 19, "P90" }, { 32, "P2000" }, { 36, "P250" },
        { 26, "PP-Bizon" }, { 64, "R8 Revolver" },
        { 29, "Sawed-Off" }, { 38, "SCAR-20" }, { 39, "SG 553" },
        { 40, "SSG 08" }, { 30, "Tec-9" }, { 24, "UMP-45" },
        { 61, "USP-S" }, { 25, "XM1014" }, { 63, "CZ75-Auto" },
    };
    inline constexpr int weaponListCount = sizeof(weaponList) / sizeof(weaponList[0]);

    inline WeaponEntry knifeModels[] = {
        { 500, "Bayonet" }, { 505, "Flip Knife" }, { 506, "Gut Knife" },
        { 507, "Karambit" }, { 508, "M9 Bayonet" }, { 509, "Huntsman Knife" },
        { 512, "Falchion Knife" }, { 514, "Bowie Knife" }, { 515, "Butterfly Knife" },
        { 516, "Shadow Daggers" }, { 517, "Paracord Knife" }, { 519, "Ursus Knife" },
        { 520, "Navaja Knife" }, { 521, "Nomad Knife" }, { 522, "Stiletto Knife" },
        { 523, "Talon Knife" }, { 525, "Skeleton Knife" }, { 526, "Kukri Knife" },
    };
    inline constexpr int knifeModelCount = sizeof(knifeModels) / sizeof(knifeModels[0]);
    inline int selectedKnifeIdx = 0;
    inline bool knifeSelected = false; // true when "Knife" entry is selected in inventory

    inline SkinChanger::SkinConfig& GetSkinConfig(int defIndex) { return SkinChanger::weaponSkins[defIndex]; }

    // === Animation state for toggles ===
    inline std::map<ImGuiID, float> g_ToggleAnim;

    // Custom filled box toggle — checkbox style, fills with accent color instead of checkmark
    inline bool FilledToggle(const char* label, bool* v)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 labelSize = ImGui::CalcTextSize(label, NULL, true);

        const float boxSz = ImGui::GetFontSize() + 2.0f;
        const float pad = style.FramePadding.y;

        const ImVec2 pos = window->DC.CursorPos;
        const ImRect totalBB(pos, ImVec2(pos.x + boxSz + style.ItemInnerSpacing.x + labelSize.x, pos.y + boxSz + pad * 2));
        const ImRect boxBB(ImVec2(pos.x, pos.y + pad), ImVec2(pos.x + boxSz, pos.y + pad + boxSz));

        ImGui::ItemSize(totalBB, pad);
        if (!ImGui::ItemAdd(totalBB, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(totalBB, id, &hovered, &held);
        if (pressed) *v = !*v;

        // Animation
        float& animT = g_ToggleAnim[id];
        float target = *v ? 1.0f : 0.0f;
        animT += (target - animT) * ImMin(g.IO.DeltaTime * 12.0f, 1.0f);

        ImDrawList* dl = window->DrawList;

        // Box outline
        ImU32 bgCol = hovered ? IM_COL32(55, 55, 55, 255) : IM_COL32(35, 35, 35, 255);
        ImU32 borderCol = hovered ? IM_COL32(90, 90, 90, 255) : IM_COL32(65, 65, 65, 255);
        dl->AddRectFilled(boxBB.Min, boxBB.Max, bgCol, 2.0f);
        dl->AddRect(boxBB.Min, boxBB.Max, borderCol, 2.0f);

        // Inner fill with accent color (animated)
        if (animT > 0.01f)
        {
            ImVec2 innerPad(2.5f, 2.5f);
            ImU32 fillCol = AccentU32(animT);
            dl->AddRectFilled(boxBB.Min + innerPad, boxBB.Max - innerPad, fillCol, 1.0f);
        }

        // Label
        ImVec2 labelPos(boxBB.Max.x + style.ItemInnerSpacing.x, pos.y + pad + (boxSz - labelSize.y) * 0.5f);
        dl->AddText(labelPos, IM_COL32(220, 220, 220, 255), label);

        return pressed;
    }

    // Draw crescent moon icon using two overlapping circles
    inline void DrawMoon(ImDrawList* dl, ImVec2 center, float radius, ImU32 col, ImU32 bgCol)
    {
        dl->AddCircleFilled(center, radius, col, 32);
        dl->AddCircleFilled(ImVec2(center.x + radius * 0.4f, center.y - radius * 0.15f), radius * 0.78f, bgCol, 32);
    }

    inline void DrawLabelShadow(ImDrawList* dl, ImVec2 pos, ImU32 col, const char* text)
    {
        dl->AddText(pos + ImVec2(1, 1), IM_COL32(0, 0, 0, 200), text);
        dl->AddText(pos, col, text);
    }

    // Helper for keybind selection
    static int g_WaitingKey = -1; // Index or ID of the control waiting for a key
    inline bool KeySelector(const char* label, int& key, int uniqueId)
    {
        bool waiting = (g_WaitingKey == uniqueId);
        char buf[64];
        
        if (waiting) {
            strcpy(buf, "... [Click to Cancel]");
            for (int i = 0; i < 512; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i != VK_LBUTTON && i != VK_INSERT) {
                        key = i;
                        g_WaitingKey = -1;
                        break;
                    }
                }
            }
        } else {
            const char* keyName = "NONE";
            if (key == VK_XBUTTON1) keyName = "MOUSE4";
            else if (key == VK_XBUTTON2) keyName = "MOUSE5";
            else if (key == VK_MENU) keyName = "ALT";
            else if (key == VK_SHIFT) keyName = "SHIFT";
            else if (key == VK_CONTROL) keyName = "CTRL";
            else if (key == VK_LBUTTON) keyName = "MB1";
            else if (key == VK_RBUTTON) keyName = "MB2";
            else if (key >= 'A' && key <= 'Z') { buf[0] = (char)key; buf[1] = '\0'; keyName = buf; }
            else if (key >= 0x30 && key <= 0x39) { buf[0] = (char)key; buf[1] = '\0'; keyName = buf; }
            else if (key != 0) { sprintf(buf, "0x%X", key); keyName = buf; }

            sprintf(buf, "%s: [%s]", label, keyName);
        }

        if (ImGui::Button(buf, ImVec2(-1, 0))) {
            if (waiting) g_WaitingKey = -1;
            else g_WaitingKey = uniqueId;
        }
        return waiting;
    }

    inline void DrawChildHeader(ImDrawList* dl, ImVec2 pos, float width, float height, const char* label)
    {
        dl->AddRectFilledMultiColor(pos, pos + ImVec2(width, height),
            IM_COL32(32, 32, 32, 255), IM_COL32(32, 32, 32, 255),
            IM_COL32(20, 20, 20, 255), IM_COL32(20, 20, 20, 255));
        dl->AddLine(pos + ImVec2(2, height), pos + ImVec2(width - 2, height), IM_COL32(0, 0, 0, 255));
        DrawLabelShadow(dl, pos + ImVec2(8, (height - ImGui::GetFontSize()) * 0.5f), AccentU32(), label);
    }

    inline bool BeginMiseryChild(const char* label, ImVec2 size)
    {
        float headerH = ImGui::GetFontSize() + 12.0f;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->AddRectFilled(pos, pos + size, IM_COL32(18, 18, 18, 255));
        dl->AddRect(pos, pos + size, IM_COL32(0, 0, 0, 255));
        dl->AddRect(pos + ImVec2(1, 1), pos + size - ImVec2(1, 1), IM_COL32(38, 38, 38, 255));

        DrawChildHeader(dl, pos + ImVec2(2, 2), size.x - 4, headerH, label);

        ImGui::SetCursorScreenPos(pos + ImVec2(3, 3 + headerH + 2));
        return ImGui::BeginChild(label, ImVec2(size.x - 6, size.y - 6 - headerH - 2), false, ImGuiWindowFlags_NoBackground);
    }

    inline void EndMiseryChild() { ImGui::EndChild(); }

    inline void RenderMenu()
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowSize(ImVec2(780, 470), ImGuiCond_Once);
        ImGui::SetNextWindowPos(io.DisplaySize * 0.5f, ImGuiCond_Once, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.07f, 0.07f, 0.97f));

        bool open = ImGui::Begin("##UnixMainWindow", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        if (!open) { ImGui::End(); return; }

        ImVec2 wPos = ImGui::GetWindowPos();
        ImVec2 wSize = ImGui::GetWindowSize();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->AddRectFilled(wPos, wPos + wSize, IM_COL32(18, 18, 20, 245));

        // Push accent-synced style colors for all ImGui widgets
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.17f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(g_Accent.x*0.25f, g_Accent.y*0.25f, g_Accent.z*0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(g_Accent.x*0.4f, g_Accent.y*0.4f, g_Accent.z*0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(g_Accent.x, g_Accent.y, g_Accent.z, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(g_Accent.x, g_Accent.y, g_Accent.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(g_Accent.x*0.2f, g_Accent.y*0.2f, g_Accent.z*0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(g_Accent.x*0.35f, g_Accent.y*0.35f, g_Accent.z*0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(g_Accent.x*0.5f, g_Accent.y*0.5f, g_Accent.z*0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(g_Accent.x*0.2f, g_Accent.y*0.2f, g_Accent.z*0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(g_Accent.x*0.35f, g_Accent.y*0.35f, g_Accent.z*0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(g_Accent.x*0.5f, g_Accent.y*0.5f, g_Accent.z*0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(g_Accent.x, g_Accent.y, g_Accent.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(g_Accent.x*0.3f, g_Accent.y*0.3f, g_Accent.z*0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(g_Accent.x*0.5f, g_Accent.y*0.5f, g_Accent.z*0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(g_Accent.x*0.7f, g_Accent.y*0.7f, g_Accent.z*0.7f, 1.0f));
        constexpr int accentStyleCount = 15;

        // === Title bar ===
        float tabBarH = 42.0f;
        ImVec2 tabPos(wPos.x, wPos.y);
        ImVec2 tabSize(wSize.x, tabBarH);
        dl->AddRectFilled(tabPos, tabPos + tabSize, IM_COL32(14, 14, 16, 255));
        dl->AddLine(tabPos + ImVec2(0, tabBarH), tabPos + ImVec2(wSize.x, tabBarH), AccentU32(0.5f));

        // Crescent moon icon + Title
        float moonR = 7.0f;
        ImVec2 moonCenter = tabPos + ImVec2(22, tabBarH * 0.5f);
        DrawMoon(dl, moonCenter, moonR, AccentU32(), IM_COL32(14, 14, 16, 255));
        if (Verdana_Bold) ImGui::PushFont(Verdana_Bold);
        DrawLabelShadow(dl, tabPos + ImVec2(38, (tabBarH - ImGui::GetFontSize()) * 0.5f), AccentU32(), "Unix Internal");
        if (Verdana_Bold) ImGui::PopFont();

        // Tabs
        float tabW = 72.0f;
        float tabStartX = tabPos.x + wSize.x - (tabW * TabCount) - 10.0f;
        for (int i = 0; i < TabCount; i++)
        {
            ImVec2 tPos(tabStartX + tabW * i, tabPos.y + 4);
            ImVec2 tSize(tabW - 4, tabBarH - 8);
            ImVec2 tEnd = tPos + tSize;

            bool hovered = ImGui::IsMouseHoveringRect(tPos, tEnd);
            if (hovered && ImGui::IsMouseClicked(0)) M_ICurrentPage = i;

            ImU32 textCol;
            if (i == M_ICurrentPage) textCol = AccentU32();
            else if (hovered)        textCol = IM_COL32(220, 220, 220, 255);
            else                     textCol = IM_COL32(130, 130, 130, 255);

            if (i == M_ICurrentPage && Verdana_Bold) ImGui::PushFont(Verdana_Bold);
            ImVec2 txtSz = ImGui::CalcTextSize(Tabs[i]);
            ImVec2 txtPos(tPos.x + (tSize.x - txtSz.x) * 0.5f, tPos.y + (tSize.y - txtSz.y) * 0.5f);
            DrawLabelShadow(dl, txtPos, textCol, Tabs[i]);
            if (i == M_ICurrentPage && Verdana_Bold) ImGui::PopFont();

            if (i == M_ICurrentPage)
                dl->AddLine(ImVec2(tPos.x + 4, tabPos.y + tabBarH - 1), ImVec2(tEnd.x - 4, tabPos.y + tabBarH - 1), AccentU32(), 2.0f);
        }

        // Content area
        float contentTop = tabBarH + 10.0f;
        float contentPad = 10.0f;
        ImVec2 contentMin = wPos + ImVec2(contentPad, contentTop);
        ImVec2 contentMax = wPos + wSize - ImVec2(contentPad, contentPad);
        float availW = contentMax.x - contentMin.x;
        float availH = contentMax.y - contentMin.y;
        float halfW = (availW - 8.0f) * 0.5f;

        ImGui::SetCursorScreenPos(contentMin);

        // ===================== RAGE =====================
        if (M_ICurrentPage == 0)
        {
            BeginMiseryChild("Aimbot", ImVec2(halfW, availH));
            FilledToggle("Enabled", &Aimbot::config.enabled);
            FilledToggle("Silent Aim", &Aimbot::config.silentAim);
            FilledToggle("Always Active", &Aimbot::config.alwaysActive);
            FilledToggle("Auto Shoot", &Aimbot::config.autoShoot);
            FilledToggle("Team Check", &Aimbot::config.teamCheck);
            ImGui::Spacing();
            const char* fovTypes[] = { "Angle (Degrees)", "Screen (Pixels)" };
            ImGui::Combo("FOV Mode", &Aimbot::config.fovType, fovTypes, IM_ARRAYSIZE(fovTypes));
            if (Aimbot::config.fovType == 0)
                ImGui::SliderFloat("FOV", &Aimbot::config.fov, 1.0f, 180.0f, "%.1f deg");
            else
                ImGui::SliderFloat("FOV", &Aimbot::config.screenFov, 10.0f, 1000.0f, "%.0f px");

            ImGui::SliderFloat("Smoothing", &Aimbot::config.smoothing, 1.0f, 25.0f, "%.1f");

            const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
            int boneIdx = 0;
            if (Aimbot::config.targetBone == 5) boneIdx = 1;
            else if (Aimbot::config.targetBone == 4) boneIdx = 2;
            else if (Aimbot::config.targetBone == 3) boneIdx = 3;
            if (ImGui::Combo("Hitbox", &boneIdx, bones, IM_ARRAYSIZE(bones)))
            {
                const int boneIds[] = { 6, 5, 4, 3 };
                Aimbot::config.targetBone = boneIds[boneIdx];
            }
            KeySelector("Aim Key", Aimbot::config.aimKey, 102);
            ImGui::Spacing();
            FilledToggle("Visible Only (Wall Check)", &Aimbot::config.visCheck);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Only target enemies that are not hidden behind walls.");
            FilledToggle("Show FOV Circle", &Aimbot::config.showFovCircle);
            EndMiseryChild();

            ImGui::SameLine(0, 8.0f);

            ImGui::BeginGroup();
            BeginMiseryChild("Triggerbot", ImVec2(halfW, availH * 0.45f));
            FilledToggle("Enabled", &Triggerbot::config.enabled);
            KeySelector("Bind", Triggerbot::config.key, 101);
            FilledToggle("Always On", &Triggerbot::config.alwaysOn);
            FilledToggle("Team Check", &Triggerbot::config.teamCheck);
            ImGui::SliderInt("Delay (ms)", &Triggerbot::config.delayMs, 0, 200);
            EndMiseryChild();

            ImGui::Spacing();

            BeginMiseryChild("Combat", ImVec2(halfW, availH * 0.5f));
            FilledToggle("No Recoil", &Combat::config.noRecoil);
            FilledToggle("No Spread", &Combat::config.noSpread);
            FilledToggle("Fake Ground", &Combat::config.fakeGround);
            ImGui::Separator();
            FilledToggle("Anti-Flash", &Combat::config.noFlash);
            FilledToggle("No Movement Shake", &Combat::config.noMovementShake);
            FilledToggle("No Fire Shake", &Combat::config.noFireShake);
            EndMiseryChild();
            ImGui::EndGroup();
        }

        // ===================== CHAMS =====================
        if (M_ICurrentPage == 2)
        {
            BeginMiseryChild("Chams Global", ImVec2(halfW, availH));
            FilledToggle("Master Switch", &Chams::cfg.enabled);
            FilledToggle("Occluded Pass (Wallhack)", &Chams::cfg.wallhack);
            
            ImGui::Spacing();
            ImGui::SeparatorText("Players (Visible)");
            ImGui::Combo("Material##vis", &Chams::cfg.playerVis.material, Chams::MaterialNames, Chams::MAT_COUNT);
            ImGui::ColorEdit4("Color##vis", Chams::cfg.playerVis.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

            if (Chams::cfg.wallhack)
            {
                ImGui::Spacing();
                ImGui::SeparatorText("Players (Hidden)");
                ImGui::Combo("Material##hid", &Chams::cfg.playerHid.material, Chams::MaterialNames, Chams::MAT_COUNT);
                ImGui::ColorEdit4("Color##hid", Chams::cfg.playerHid.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }
            EndMiseryChild();

            ImGui::SameLine(0, 8.0f);

            ImGui::BeginGroup();
            BeginMiseryChild("Local & World", ImVec2(halfW, availH));
            FilledToggle("Hand / Glove Chams", &Chams::cfg.handsEnabled);
            if (Chams::cfg.handsEnabled)
            {
                ImGui::Combo("Material##hands", &Chams::cfg.hands.material, Chams::MaterialNames, Chams::MAT_COUNT);
                ImGui::ColorEdit4("Color##hands", Chams::cfg.hands.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            FilledToggle("Dropped Weapon Chams", &Chams::cfg.weaponsEnabled);
            if (Chams::cfg.weaponsEnabled)
            {
                ImGui::Combo("Material##weap", &Chams::cfg.weapons.material, Chams::MaterialNames, Chams::MAT_COUNT);
                ImGui::ColorEdit4("Color##weap", Chams::cfg.weapons.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }
            EndMiseryChild();
            ImGui::EndGroup();
        }

        // ===================== VISUALS (Index 1) =====================
        if (M_ICurrentPage == 1)
        {
            ImGui::BeginGroup();
                       // Left Column: Settings
            ImGui::BeginChild("##LeftColumnScroll", ImVec2(halfW, availH), false, ImGuiWindowFlags_NoBackground);
            {
                BeginMiseryChild("ESP Settings", ImVec2(halfW - 0, availH * 0.55f));
                FilledToggle("Enabled", &ESP::config.enabled);
                FilledToggle("Box", &ESP::config.bBox);
                FilledToggle("Team Check", &ESP::config.teamCheck);
                if (ESP::config.bBox)
                {
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 45);
                    if (ImGui::Button("COL", ImVec2(40, 0))) ImGui::OpenPopup("BoxColPopup");
                    if (ImGui::BeginPopup("BoxColPopup"))
                    {
                        ImGui::ColorEdit4("Enemy", ESP::config.enemyColor, ImGuiColorEditFlags_AlphaBar);
                        ImGui::ColorEdit4("Team", ESP::config.teamColor, ImGuiColorEditFlags_AlphaBar);
                        ImGui::EndPopup();
                    }
                    const char* boxModes[] = { "Bounding", "Corner" };
                    ImGui::Combo("Box Type", &ESP::config.boxType, boxModes, IM_ARRAYSIZE(boxModes));
                }
                FilledToggle("Healthbar", &ESP::config.bHealthBar);
                FilledToggle("Name", &ESP::config.bName);
                FilledToggle("Distance", &ESP::config.bDistance);
                FilledToggle("Skeleton", &ESP::config.bSkeleton);
                if (ESP::config.bSkeleton)
                {
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 45);
                    if (ImGui::Button("COL##Skel", ImVec2(40, 0))) ImGui::OpenPopup("SkelColPopup");
                    if (ImGui::BeginPopup("SkelColPopup"))
                    {
                        ImGui::ColorEdit4("Skeleton", ESP::config.skeletonColor, ImGuiColorEditFlags_AlphaBar);
                        ImGui::EndPopup();
                    }
                }
                ImGui::Separator();
                FilledToggle("Dropped Weapons", &ESP::config.bDroppedWeapons);
                if (ESP::config.bDroppedWeapons)
                {
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 45);
                    if (ImGui::Button("COL##Dropped", ImVec2(40, 0))) ImGui::OpenPopup("DroppedColPopup");
                    if (ImGui::BeginPopup("DroppedColPopup"))
                    {
                        ImGui::ColorEdit4("Dropped", ESP::config.droppedColor, ImGuiColorEditFlags_AlphaBar);
                        ImGui::EndPopup();
                    }
                    FilledToggle("Show Distance", &ESP::config.bDroppedDistance);
                }
                EndMiseryChild();

                ImGui::Spacing();

                BeginMiseryChild("Extras", ImVec2(halfW - 0, availH * 0.42f));
                FilledToggle("Glow ESP", &ESP::config.bGlow);
                if (ESP::config.bGlow)
                {
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 45);
                    if (ImGui::Button("COL##Glow", ImVec2(40, 0))) ImGui::OpenPopup("GlowColPopup");
                    if (ImGui::BeginPopup("GlowColPopup"))
                    {
                        ImGui::ColorEdit4("Glow", ESP::config.glowColor, ImGuiColorEditFlags_AlphaBar);
                        ImGui::EndPopup();
                    }
                }
                FilledToggle("Bomb Info Window", &ESP::config.bBombTimer);
                FilledToggle("Spectator List", &ESP::config.bSpectators);
                EndMiseryChild();
            }
            ImGui::EndChild();
            
            ImGui::EndGroup();

            ImGui::SameLine(0, 8.0f);

            // Right Column: Preview
            BeginMiseryChild("Preview", ImVec2(halfW, availH));
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 curPos = ImGui::GetCursorScreenPos();
            ImVec2 avail = ImGui::GetContentRegionAvail();
            
            // Draw a slightly darkened background for the preview area
            draw->AddRectFilled(curPos, curPos + avail, IM_COL32(20, 20, 20, 255), 5.0f);
            draw->AddRect(curPos, curPos + avail, IM_COL32(45, 45, 45, 255), 5.0f);

            // Center the preview area
            ImVec2 previewCenter = curPos + avail * 0.5f;
            ImVec2 imgSize = ImVec2(350, 350); // Scale 500x500 down slightly to fit better
            ImVec2 imgPos = previewCenter - imgSize * 0.5f;

            if (pDevice && !g_pEspPreviewTexture && !g_EspPreviewLoadingFailed)
                LoadEspPreviewTexture(pDevice);

            if (g_pEspPreviewTexture)
            {
                draw->AddImage(g_pEspPreviewTexture, imgPos, imgPos + imgSize);
                ESP::RenderPreview(draw, imgPos, imgSize); // Overlay options on image

                // Calibration Toggle Button (Top-Right of preview)
                ImGui::SetCursorScreenPos(curPos + ImVec2(avail.x - 30, 5));
                if (ImGui::Button(ESP::config.bShowCalibration ? "X" : "C", ImVec2(25, 25)))
                    ESP::config.bShowCalibration = !ESP::config.bShowCalibration;
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Manual Calibration Settings");

                // Manual Calibration Sliders (Only if toggled on)
                if (ESP::config.bShowCalibration)
                {
                    ImGui::SetCursorScreenPos(curPos + ImVec2(10, avail.y - 85));
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 200));
                    ImGui::BeginChild("##CalibArea", ImVec2(avail.x - 20, 75), true);
                    ImGui::TextDisabled("Calibration:");
                    ImGui::PushItemWidth(-1);
                    ImGui::SliderFloat("##W", &ESP::config.previewWidthScale, 0.1f, 1.0f, "Width: %.2f");
                    ImGui::SliderFloat("##H", &ESP::config.previewHeightScale, 0.1f, 1.0f, "Height: %.2f");
                    ImGui::SliderFloat("##Y", &ESP::config.previewYOffset, 0.0f, 1.0f, "Y-Offset: %.2f");
                    ImGui::PopItemWidth();
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                }
            }
            else
            {
                // Fallback to dummy if image fails to load
                ImVec2 dummySize = ImVec2(avail.x * 0.6f, avail.y * 0.7f);
                ImVec2 dummyPos = previewCenter - dummySize * 0.5f;
                ESP::RenderPreview(draw, dummyPos, dummySize);
            }
            
            EndMiseryChild();
        }

        // ===================== MISC =====================
        if (M_ICurrentPage == 3)
        {
            BeginMiseryChild("Movement", ImVec2(halfW, availH));
            FilledToggle("Bunny Hop", &Bhop::config.enabled);
            FilledToggle("Third Person", &SkinChanger::thirdPerson);
            ImGui::Spacing();
            FilledToggle("View FOV Override", &g_ViewFovEnabled);
            if (g_ViewFovEnabled)
                ImGui::SliderInt("View FOV", &g_ViewFov, 60, 140);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            FilledToggle("Spinbot", &Spinbot::config.enabled);
            if (Spinbot::config.enabled)
            {
                ImGui::SliderFloat("Yaw Speed", &Spinbot::config.yawSpeed, 1.0f, 50.0f, "%.1f deg/tick");
                ImGui::SliderFloat("Pitch", &Spinbot::config.pitch, -89.0f, 89.0f, "%.1f");
                FilledToggle("Anti-Aim Jitter", &Spinbot::config.antiAim);
            }
            EndMiseryChild();

            ImGui::SameLine(0, 8.0f);

            BeginMiseryChild("Effects", ImVec2(halfW, availH));
            FilledToggle("Bullet Tracers", &BulletTracer::config.enabled);
            if (BulletTracer::config.enabled)
            {
                ImGui::SliderFloat("Trail Life", &BulletTracer::config.trailLife, 0.5f, 5.0f, "%.1f s");
                ImGui::SliderFloat("Thickness", &BulletTracer::config.thickness, 1.0f, 5.0f, "%.1f");
                ImGui::SliderFloat("Bullet Speed", &BulletTracer::config.bulletSpeed, 2000.0f, 20000.0f, "%.0f");
                
                ImGui::Separator();
                std::lock_guard<std::mutex> lock(BulletTracer::traceMutex);
                ImGui::Text("Active Traces: %d", (int)BulletTracer::traces.size());
                ImGui::Text("Shot Count: %d", BulletTracer::lastShotsFired);
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            BeginMiseryChild("Model Changer", ImVec2(halfW, availH));
            const char* knifeNames[] = { "None", "Karambit (Blue Gem)", "Butterfly", "M9 Bayonet", "Bowie" };
            if (ImGui::Combo("Knife Model", &ESP::config.nKnifeModel, knifeNames, IM_ARRAYSIZE(knifeNames)))
            {
                 // Seed 387 is hardcoded for Karambit in ModelChanger::Run logic
            }

            const char* agentNames[] = { "None", "Professional", "Phoenix", "FBI", "Ricksaw", "Darryl", "Ava" };
            ImGui::Combo("Player Model", &ESP::config.nPlayerModel, agentNames, IM_ARRAYSIZE(agentNames));

            const char* gloveNames[] = { "None", "Cobalt Skulls", "Marble Fade", "Emerald" };
            ImGui::Combo("Glove Model", &ESP::config.nGloveModel, gloveNames, IM_ARRAYSIZE(gloveNames));
            
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("Force Refresh Models", ImVec2(-1, 0)))
                SkinChanger::forceUpdate.store(true);

            EndMiseryChild();

            EndMiseryChild();
        }

        // ===================== SKINS =====================
        if (M_ICurrentPage == 4)
        {
            BeginMiseryChild("Inventory", ImVec2(halfW, availH));
            if (ImGui::Button("Force Update", ImVec2(-1, 0)))
                SkinChanger::forceUpdate.store(true);
            if (ImGui::Button("Clear All Skins", ImVec2(-1, 0)))
            {
                std::lock_guard<std::mutex> lock(SkinChanger::configMutex);
                SkinChanger::weaponSkins.clear();
                SkinChanger::forceUpdate.store(true);
            }
            if (ImGui::Button("Randomize All", ImVec2(-1, 0)))
            {
                std::lock_guard<std::mutex> lock(SkinChanger::configMutex);
                if (!g_PaintKits.empty()) {
                    srand((unsigned int)time(NULL));
                    for (int wi = 0; wi < weaponListCount; wi++) {
                        int defIdx = weaponList[wi].defIndex;
                        SkinChanger::SkinConfig& cfg = SkinChanger::weaponSkins[defIdx];
                        cfg.enabled = true;
                        int randomIdx = rand() % g_PaintKits.size();
                        cfg.paintKit = g_PaintKits[randomIdx].id;
                        cfg.wear = 0.001f; cfg.seed = 0; cfg.statTrak = -1;
                    }
                    SkinChanger::forceUpdate.store(true);
                }
            }
            ImGui::Spacing();
            for (int i = 0; i < weaponListCount; i++)
            {
                bool hasConfig = SkinChanger::weaponSkins.count(weaponList[i].defIndex) > 0 && SkinChanger::weaponSkins[weaponList[i].defIndex].enabled;
                if (hasConfig) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(g_Accent.x, g_Accent.y, g_Accent.z, 1.0f));
                if (ImGui::Selectable(weaponList[i].name, !knifeSelected && selectedWeaponIdx == i))
                {
                    selectedWeaponIdx = i;
                    knifeSelected = false;
                }
                if (hasConfig) ImGui::PopStyleColor();
            }
            // Single "Knife" entry at the bottom
            {
                int kDefIdx = knifeModels[selectedKnifeIdx].defIndex;
                bool hasKnife = SkinChanger::weaponSkins.count(kDefIdx) > 0 && SkinChanger::weaponSkins[kDefIdx].enabled;
                if (hasKnife) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(g_Accent.x, g_Accent.y, g_Accent.z, 1.0f));
                if (ImGui::Selectable("Knife", knifeSelected))
                    knifeSelected = true;
                if (hasKnife) ImGui::PopStyleColor();
            }
            EndMiseryChild();

            ImGui::SameLine(0, 8.0f);

            BeginMiseryChild("Configure", ImVec2(halfW, availH));
            if (knifeSelected)
            {
                ImGui::Text("Knife");
                ImGui::Separator();
                ImGui::Spacing();
                // Knife model dropdown
                if (ImGui::BeginCombo("Knife Model", knifeModels[selectedKnifeIdx].name))
                {
                    for (int k = 0; k < knifeModelCount; k++)
                    {
                        bool isSel = (selectedKnifeIdx == k);
                        if (ImGui::Selectable(knifeModels[k].name, isSel))
                            selectedKnifeIdx = k;
                        if (isSel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                int defIdx = knifeModels[selectedKnifeIdx].defIndex;
                SkinChanger::SkinConfig& cfg = GetSkinConfig(defIdx);
                FilledToggle("Enabled", &cfg.enabled);
                // Paint kit selector (shared with weapons below)
                static int selectedRarity = 0;
                const char* rarityNames[] = { "All", "Consumer", "Industrial", "Mil-Spec", "Restricted", "Classified", "Covert", "Contraband" };
                ImGui::Combo("Filter Rarity", &selectedRarity, rarityNames, IM_ARRAYSIZE(rarityNames));
                std::string previewName = "Custom ID: " + std::to_string(cfg.paintKit);
                for (const auto& pk : g_PaintKits) {
                    if (pk.id == cfg.paintKit) { previewName = pk.name; break; }
                }
                if (ImGui::BeginCombo("Paint Kit", previewName.c_str())) {
                    for (const auto& pk : g_PaintKits) {
                        if (selectedRarity != 0 && (int)pk.rarity != selectedRarity - 1) continue;
                        bool isSelected = (cfg.paintKit == pk.id);
                        ImGui::PushStyleColor(ImGuiCol_Text, GetRarityColor(pk.rarity));
                        if (ImGui::Selectable(pk.name, isSelected)) cfg.paintKit = pk.id;
                        ImGui::PopStyleColor();
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::InputInt("Custom Paint Kit ID", &cfg.paintKit);
                if (cfg.paintKit < 0) cfg.paintKit = 0;
                ImGui::SliderFloat("Wear", &cfg.wear, 0.0f, 1.0f, "%.4f");
                ImGui::InputInt("Seed", &cfg.seed);
                bool useStatTrak = cfg.statTrak >= 0;
                if (FilledToggle("StatTrak", &useStatTrak)) cfg.statTrak = useStatTrak ? 0 : -1;
                if (useStatTrak) { ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::InputInt("##statval", &cfg.statTrak); }
            }
            else if (selectedWeaponIdx >= 0 && selectedWeaponIdx < weaponListCount)
            {
                int defIdx = weaponList[selectedWeaponIdx].defIndex;
                SkinChanger::SkinConfig& cfg = GetSkinConfig(defIdx);
                ImGui::Text("%s", weaponList[selectedWeaponIdx].name);
                ImGui::Separator();
                ImGui::Spacing();
                FilledToggle("Enabled", &cfg.enabled);

                static int selectedRarity = 0;
                const char* rarityNames[] = { "All", "Consumer", "Industrial", "Mil-Spec", "Restricted", "Classified", "Covert", "Contraband" };
                ImGui::Combo("Filter Rarity", &selectedRarity, rarityNames, IM_ARRAYSIZE(rarityNames));

                std::string previewName = "Custom ID: " + std::to_string(cfg.paintKit);
                for (const auto& pk : g_PaintKits) {
                    if (pk.id == cfg.paintKit) { previewName = pk.name; break; }
                }
                if (ImGui::BeginCombo("Paint Kit", previewName.c_str())) {
                    for (const auto& pk : g_PaintKits) {
                        if (selectedRarity != 0 && (int)pk.rarity != selectedRarity - 1) continue;
                        bool isSelected = (cfg.paintKit == pk.id);
                        ImGui::PushStyleColor(ImGuiCol_Text, GetRarityColor(pk.rarity));
                        if (ImGui::Selectable(pk.name, isSelected)) cfg.paintKit = pk.id;
                        ImGui::PopStyleColor();
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::InputInt("Custom Paint Kit ID", &cfg.paintKit);
                if (cfg.paintKit < 0) cfg.paintKit = 0;
                ImGui::SliderFloat("Wear", &cfg.wear, 0.0f, 1.0f, "%.4f");
                ImGui::InputInt("Seed", &cfg.seed);
                bool useStatTrak = cfg.statTrak >= 0;
                if (FilledToggle("StatTrak", &useStatTrak)) cfg.statTrak = useStatTrak ? 0 : -1;
                if (useStatTrak) { ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::InputInt("##statval", &cfg.statTrak); }
            }
            EndMiseryChild();
        }

        // ===================== SETTINGS =====================
        if (M_ICurrentPage == 5)
        {
            BeginMiseryChild("Appearance", ImVec2(halfW, availH));
            ImGui::Text("Color Scheme");
            for (int i = 0; i < g_SchemeCount; i++)
            {
                ImVec4 c = g_Schemes[i].color;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(c.x * 0.4f, c.y * 0.4f, c.z * 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(c.x * 0.6f, c.y * 0.6f, c.z * 0.6f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(c.x, c.y, c.z, 1.0f));
                bool selected = (g_SchemeIdx == i);
                if (selected) {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(c.x, c.y, c.z, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                }
                if (ImGui::Button(g_Schemes[i].name, ImVec2(80, 24)))
                {
                    g_SchemeIdx = i;
                    g_Accent = g_Schemes[i].color;
                }
                if (selected) { ImGui::PopStyleVar(); ImGui::PopStyleColor(); }
                ImGui::PopStyleColor(3);
                if (i < g_SchemeCount - 1) ImGui::SameLine();
            }
            ImGui::Spacing();
            ImGui::ColorEdit4("Custom Accent", (float*)&g_Accent, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("FPS: %.0f", io.Framerate);
            ImGui::TextDisabled("INSERT - Toggle Menu");
            EndMiseryChild();

            ImGui::SameLine(0, 8.0f);

            BeginMiseryChild("Config", ImVec2(halfW, availH));
            static char cfgName[64] = "default";
            ImGui::InputText("Name", cfgName, sizeof(cfgName));
            if (ImGui::Button("Save Config", ImVec2(-1, 0)))
                Config::Save(cfgName);
            if (ImGui::Button("Load Config", ImVec2(-1, 0)))
                Config::Load(cfgName);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Saved Configs:");
            auto configs = Config::GetConfigs();
            for (auto& name : configs)
            {
                bool isCurrent = (name == Config::currentConfig);
                if (isCurrent) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(g_Accent.x, g_Accent.y, g_Accent.z, 1.0f));
                if (ImGui::Selectable(name.c_str(), isCurrent))
                {
                    Config::Load(name);
                    strncpy_s(cfgName, name.c_str(), sizeof(cfgName) - 1);
                }
                if (isCurrent) ImGui::PopStyleColor();
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.15f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Unload Cheat", ImVec2(-1, 30)))
                g_RequestUnload = true;
            ImGui::PopStyleColor(3);
            EndMiseryChild();
        }

        // Pop accent-synced style colors
        ImGui::PopStyleColor(accentStyleCount);

        // Window border
        dl->AddRect(wPos, wPos + wSize, IM_COL32(0, 0, 0, 255));
        dl->AddRect(wPos + ImVec2(1, 1), wPos + wSize - ImVec2(1, 1), IM_COL32(38, 38, 38, 255));

        ImGui::End();
    }
}
