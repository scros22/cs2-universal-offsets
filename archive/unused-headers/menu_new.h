#pragma once

// ═══════════════════════════════════════════════════════════════
// Lucid CS2 Menu  ─  Menu 19 "framework" exact replica
// Dark navy (13,13,18) | Soft-purple accent (142,132,255)
// 840x630 window | 110px LEFT SIDEBAR with stacked tabs
// Toggle-switch checkboxes | Equalizer-bar sliders
// Gradient accent lines top+bottom | Auto-height section boxes
// ═══════════════════════════════════════════════════════════════

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_internal.h"
#include "../features/esp.h"
#include "../features/aimbot.h"
#include "../features/chams.h"
#include "../features/bullet_tracer.h"
#include "../features/world_effects.h"
#include "../features/damage_indicator.h"
#include "../features/bhop.h"
#include "../features/auto_accept.h"
#include "../features/rank_revealer.h"
#include "../features/grenade_prediction.h"
#include "../features/nade_helper.h"
#include "../features/skinchanger.h"
#include "../features/triggerbot.h"
#include "../features/backtrack.h"
#include "../features/anti_aim.h"
#include "../features/fake_lag.h"
#include "../features/sound_esp.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cmath>

namespace Menu
{
    // State
    inline int   activeTab    = 0;
    inline float menuAlpha    = 0.f;
    inline ImFont* fonts[3]   = { nullptr, nullptr, nullptr };
    inline ImFont* espFont    = nullptr;

    inline float primaryColor[4]   = { 142/255.f, 132/255.f, 255/255.f, 1.0f };
    inline float secondaryColor[4] = { 0.09f, 0.09f, 0.09f, 0.70f };
    inline bool  themeApplied      = false;

    enum MenuStyle : int { STYLE_GLASS = 0, STYLE_CYBER = 1, STYLE_MINIMAL = 2 };
    inline MenuStyle activeStyle      = STYLE_GLASS;
    inline int       lastAppliedStyle = -1;

    inline const char* kTabLabels[] = { "AIM", "VIS", "SKN", "WLD", "CFG" };
    inline constexpr int kTabCount  = 5;

    // Colour palette (menu 19 exact values)
    static constexpr ImU32 kWinBg     = IM_COL32(13,  13,  18,  255);
    static constexpr ImU32 kRightBg   = IM_COL32(13,  13,  18,  255);
    static constexpr ImU32 kChildBg   = IM_COL32(16,  16,  22,  255);
    static constexpr ImU32 kElemBg    = IM_COL32(21,  21,  29,  255);
    static constexpr ImU32 kCheckOn   = IM_COL32(37,  36,  53,  255);
    static constexpr ImU32 kCircleOff = IM_COL32(41,  41,  53,  255);
    static constexpr ImU32 kOuterStr  = IM_COL32(21,  23,  26,  255);
    static constexpr ImU32 kInnerStr  = IM_COL32(19,  18,  26,  255);
    static constexpr ImU32 kTextDim   = IM_COL32(49,  49,  61,  255);
    static constexpr ImU32 kTextMid   = IM_COL32(104, 104, 120, 255);
    static constexpr ImU32 kTextBrt   = IM_COL32(255, 255, 255, 255);

    // Colour helpers
    inline ImU32 EvoAccent(int alpha = 255)
    {
        return IM_COL32(
            (int)(primaryColor[0] * 255),
            (int)(primaryColor[1] * 255),
            (int)(primaryColor[2] * 255),
            alpha);
    }
    inline ImVec4 EvoAccentV(float alpha = 1.f)
    { return { primaryColor[0], primaryColor[1], primaryColor[2], alpha }; }

    // Apply ImGui style theme
    inline void ApplyTheme()
    {
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding     = 4.f;
        s.FrameRounding      = 2.f;
        s.GrabRounding       = 2.f;
        s.PopupRounding      = 4.f;
        s.ChildRounding      = 4.f;
        s.ScrollbarRounding  = 2.f;
        s.WindowBorderSize   = 0.f;
        s.ChildBorderSize    = 1.f;
        s.FrameBorderSize    = 0.f;
        s.ScrollbarSize      = 6.f;
        s.GrabMinSize        = 6.f;
        s.WindowPadding      = { 0, 0 };
        s.FramePadding       = { 8, 4 };
        s.ItemSpacing        = { 6, 4 };

        ImVec4* c = s.Colors;
        c[ImGuiCol_WindowBg]             = { 13/255.f,  13/255.f,  18/255.f, 1.f };
        c[ImGuiCol_ChildBg]              = { 16/255.f,  16/255.f,  22/255.f, 1.f };
        c[ImGuiCol_PopupBg]              = { 16/255.f,  16/255.f,  22/255.f, 1.f };
        c[ImGuiCol_Text]                 = { 104/255.f, 104/255.f, 120/255.f, 1.f };
        c[ImGuiCol_TextDisabled]         = { 49/255.f,  49/255.f,  61/255.f, 1.f };
        c[ImGuiCol_Border]               = { 19/255.f,  18/255.f,  26/255.f, 1.f };
        c[ImGuiCol_BorderShadow]         = { 0, 0, 0, 0 };
        c[ImGuiCol_FrameBg]              = { 21/255.f,  21/255.f,  29/255.f, 1.f };
        c[ImGuiCol_FrameBgHovered]       = { 31/255.f,  31/255.f,  41/255.f, 1.f };
        c[ImGuiCol_FrameBgActive]        = { 37/255.f,  36/255.f,  53/255.f, 1.f };
        c[ImGuiCol_TitleBg]              = { 13/255.f,  13/255.f,  18/255.f, 1.f };
        c[ImGuiCol_TitleBgActive]        = { 13/255.f,  13/255.f,  18/255.f, 1.f };
        c[ImGuiCol_ScrollbarBg]          = { 16/255.f,  16/255.f,  22/255.f, 0.5f };
        c[ImGuiCol_ScrollbarGrab]        = { 41/255.f,  41/255.f,  53/255.f, 1.f };
        c[ImGuiCol_ScrollbarGrabHovered] = { 55/255.f,  55/255.f,  70/255.f, 1.f };
        c[ImGuiCol_ScrollbarGrabActive]  = EvoAccentV(0.50f);
        c[ImGuiCol_CheckMark]            = EvoAccentV(1.f);
        c[ImGuiCol_SliderGrab]           = EvoAccentV(0.80f);
        c[ImGuiCol_SliderGrabActive]     = EvoAccentV(1.f);
        c[ImGuiCol_Button]               = { 21/255.f,  21/255.f,  29/255.f, 1.f };
        c[ImGuiCol_ButtonHovered]        = { 31/255.f,  31/255.f,  41/255.f, 1.f };
        c[ImGuiCol_ButtonActive]         = EvoAccentV(0.35f);
        c[ImGuiCol_Header]               = { 21/255.f,  21/255.f,  29/255.f, 1.f };
        c[ImGuiCol_HeaderHovered]        = { 31/255.f,  31/255.f,  41/255.f, 1.f };
        c[ImGuiCol_HeaderActive]         = EvoAccentV(0.30f);
        c[ImGuiCol_Separator]            = { 19/255.f,  18/255.f,  26/255.f, 1.f };
        c[ImGuiCol_SeparatorHovered]     = { 31/255.f,  31/255.f,  41/255.f, 1.f };
        c[ImGuiCol_SeparatorActive]      = EvoAccentV(0.70f);
    }


    // ============================================================
    //  CONTROLS
    // ============================================================

    // Thin separator line
    inline void SynthSep()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float  w   = ImGui::GetContentRegionAvail().x;
        dl->AddRectFilled(pos, { pos.x + w, pos.y + 1.f }, kInnerStr);
        ImGui::Dummy({ w, 1.f });
    }

    // Toggle-switch checkbox (40x20 pill right-aligned, circle slides)
    inline bool EvoCheckbox(const char* label, bool* v)
    {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float w  = ImGui::GetContentRegionAvail().x;
        const float h  = 28.f;
        ImVec2 pos     = ImGui::GetCursorScreenPos();

        const float pillW = 40.f, pillH = 20.f;
        ImVec2 pillTL = { pos.x + w - pillW, pos.y + (h - pillH) * 0.5f };
        ImVec2 pillBR = { pos.x + w,          pos.y + (h + pillH) * 0.5f };

        ImU32 pillBg = *v ? kCheckOn : kElemBg;
        float cx     = pillTL.x + (*v ? 28.f : 12.f);
        float cy     = (pillTL.y + pillBR.y) * 0.5f;
        ImU32 cirC   = *v ? EvoAccent(220) : kCircleOff;

        float ty = pos.y + (h - ImGui::GetFontSize()) * 0.5f;
        dl->AddText({ pos.x + 2.f, ty }, *v ? kTextBrt : kTextDim, label);

        dl->AddRectFilled(pillTL, pillBR, pillBg, 100.f);
        dl->AddCircleFilled({ cx, cy }, 6.f, cirC);

        ImGui::Dummy({ w, h });
        bool changed = false;
        if (ImGui::IsItemClicked()) { *v = !*v; changed = true; }
        ImGui::PopID();
        return changed;
    }

    // Equalizer-bar slider (5 animated bars, right-aligned 55x18)
    inline bool EvoSliderFloat(const char* label, float* v, float mn, float mx,
                                const char* fmt = "%.1f")
    {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float w  = ImGui::GetContentRegionAvail().x;
        const float h  = 28.f;
        ImVec2 pos     = ImGui::GetCursorScreenPos();

        const float sW = 55.f, sH = 18.f;
        ImVec2 sMin = { pos.x + w - sW, pos.y + (h - sH) * 0.5f };
        ImVec2 sMax = { pos.x + w,       pos.y + (h + sH) * 0.5f };
        float  cy   = (sMin.y + sMax.y) * 0.5f;

        dl->AddRectFilled(sMin, sMax, kElemBg, 2.f);

        float range  = mx - mn;
        float segW   = (sMax.x - sMin.x - 6.f) / 5.f;
        for (int i = 0; i < 5; ++i)
        {
            float bx     = sMin.x + 3.f + segW * i;
            float lower  = mn + range *  (float)i      * 0.25f;
            float upper  = mn + range * ((float)i + 1) * 0.25f;
            bool  active = (*v >= lower && (i < 4 ? *v < upper : *v <= upper));
            float barH   = active ? 6.f : 3.f;
            int   alpha  = active ? 230 : 26;
            dl->AddRectFilled({ bx, cy - barH }, { bx + 3.f, cy + barH },
                               EvoAccent(alpha), 2.f);
        }

        char valStr[32];
        snprintf(valStr, sizeof(valStr), fmt, *v);
        ImVec2 vsz = ImGui::CalcTextSize(valStr);
        float  ty  = pos.y + (h - ImGui::GetFontSize()) * 0.5f;
        dl->AddText({ pos.x + 2.f, ty }, kTextMid, label);
        dl->AddText({ sMin.x - vsz.x - 4.f, ty }, kTextMid, valStr);

        ImGui::SetCursorScreenPos(sMin);
        bool changed = false;
        if (ImGui::InvisibleButton("##s", { sW, sH }) || ImGui::IsItemActive())
        {
            float nt = (ImGui::GetMousePos().x - sMin.x) / sW;
            nt = nt < 0.f ? 0.f : (nt > 1.f ? 1.f : nt);
            *v = mn + (mx - mn) * nt;
            changed = true;
        }
        ImGui::SetCursorScreenPos({ pos.x, pos.y + h });
        ImGui::PopID();
        return changed;
    }

    inline bool EvoSliderInt(const char* label, int* v, int mn, int mx)
    {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float w  = ImGui::GetContentRegionAvail().x;
        const float h  = 28.f;
        ImVec2 pos     = ImGui::GetCursorScreenPos();

        const float sW = 55.f, sH = 18.f;
        ImVec2 sMin = { pos.x + w - sW, pos.y + (h - sH) * 0.5f };
        ImVec2 sMax = { pos.x + w,       pos.y + (h + sH) * 0.5f };
        float  cy   = (sMin.y + sMax.y) * 0.5f;

        dl->AddRectFilled(sMin, sMax, kElemBg, 2.f);

        float range  = (float)(mx - mn);
        float segW   = (sMax.x - sMin.x - 6.f) / 5.f;
        for (int i = 0; i < 5; ++i)
        {
            float bx     = sMin.x + 3.f + segW * i;
            float lower  = (float)mn + range *  (float)i      * 0.25f;
            float upper  = (float)mn + range * ((float)i + 1) * 0.25f;
            bool  active = ((float)*v >= lower && (i < 4 ? (float)*v < upper : (float)*v <= upper));
            float barH   = active ? 6.f : 3.f;
            int   alpha  = active ? 230 : 26;
            dl->AddRectFilled({ bx, cy - barH }, { bx + 3.f, cy + barH },
                               EvoAccent(alpha), 2.f);
        }

        char valStr[32];
        snprintf(valStr, sizeof(valStr), "%d", *v);
        ImVec2 vsz = ImGui::CalcTextSize(valStr);
        float  ty  = pos.y + (h - ImGui::GetFontSize()) * 0.5f;
        dl->AddText({ pos.x + 2.f, ty }, kTextMid, label);
        dl->AddText({ sMin.x - vsz.x - 4.f, ty }, kTextMid, valStr);

        ImGui::SetCursorScreenPos(sMin);
        bool changed = false;
        if (ImGui::InvisibleButton("##s", { sW, sH }) || ImGui::IsItemActive())
        {
            float nt = (ImGui::GetMousePos().x - sMin.x) / sW;
            nt = nt < 0.f ? 0.f : (nt > 1.f ? 1.f : nt);
            *v = mn + (int)(range * nt + 0.5f);
            changed = true;
        }
        ImGui::SetCursorScreenPos({ pos.x, pos.y + h });
        ImGui::PopID();
        return changed;
    }

    // Section/category label
    inline void EvoLabel(const char* text)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos     = ImGui::GetCursorScreenPos();
        float  w       = ImGui::GetContentRegionAvail().x;
        float  fh      = ImGui::GetFontSize();
        dl->AddText(pos, kTextMid, text);
        ImGui::Dummy({ w, fh + 2.f });
    }

    inline void SectionHeader(const char* text) { EvoLabel(text); }

    // Full-width dark button
    inline bool EvoButton(const char* label, ImVec2 sz = { -1.f, 26.f })
    {
        if (sz.x < 0) sz.x = ImGui::GetContentRegionAvail().x;
        ImGui::PushStyleColor(ImGuiCol_Button,        { 21/255.f, 21/255.f, 29/255.f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 31/255.f, 31/255.f, 41/255.f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  EvoAccentV(0.35f));
        bool hit = ImGui::Button(label, sz);
        ImGui::PopStyleColor(3);
        return hit;
    }

    // Styled combo/dropdown
    inline bool EvoCombo(const char* label, int* v, const char* const* items, int cnt)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg,        { 21/255.f, 21/255.f, 29/255.f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, { 31/255.f, 31/255.f, 41/255.f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_PopupBg,        { 16/255.f, 16/255.f, 22/255.f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_Header,         { 31/255.f, 31/255.f, 41/255.f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,  EvoAccentV(0.20f));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool hit = ImGui::Combo(label, v, items, cnt);
        ImGui::PopStyleColor(5);
        return hit;
    }

    // Key picker
    inline int KeyCombo(const char* label, int current)
    {
        const char* names[] = { "Auto", "RClick", "Mouse4", "Mouse5", "Shift", "Alt" };
        const int   codes[] = { 0, VK_RBUTTON, VK_XBUTTON1, VK_XBUTTON2, VK_SHIFT, VK_MENU };
        int idx = 0;
        for (int i = 0; i < 6; ++i) if (codes[i] == current) idx = i;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Combo(label, &idx, names, 6)) return codes[idx];
        return current;
    }

    // Auto-height bordered section box
    inline void SynthBeginSection(const char* id)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, { 16/255.f, 16/255.f, 22/255.f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_Border,  { 19/255.f, 18/255.f, 26/255.f, 1.f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,   { 8.f, 7.f });
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   4.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
        ImGui::BeginChild(id,
            { ImGui::GetContentRegionAvail().x, 0.f },
            ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    inline void SynthEndSection()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        ImGui::Dummy({ 0.f, 4.f });
    }


    // ============================================================
    //  CONFIG SYSTEM
    // ============================================================

    struct SavedConfig
    {
        uint32_t magic    = 0x4C554349;
        uint32_t version  = 15;
        uint32_t dataSize = sizeof(SavedConfig);
        char                    name[32];
        Aimbot::Config          aimbot;
        ESP::Config             esp;
        BulletTracer::Config    tracer;
        WorldEffects::Config    worldEffects;
        DamageIndicator::Config damage;
        WireframeHands::Config  wireframe;
        Chams::Config           chams;
        Bhop::Config            bhop;
        SkinChanger::Config     skinchanger;
        int   menuStyle = 0;
        float priColor[4] = { 142/255.f, 132/255.f, 255/255.f, 1.f };
        float secColor[4] = { 0.09f, 0.09f, 0.09f, 0.70f };
    };

    inline constexpr int kMaxSlots = 5;
    inline char slotNames[kMaxSlots][32] = {"Slot 1","Slot 2","Slot 3","Slot 4","Slot 5"};
    inline bool slotsLoaded = false;

    inline void GetConfigDir(char* out, size_t sz)
    {
        char appdata[MAX_PATH]{};
        GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH);
        snprintf(out, sz, "%s\\LucidCS2", appdata);
        CreateDirectoryA(out, nullptr);
    }

    inline void GetSlotPath(int slot, char* out, size_t sz)
    {
        char dir[MAX_PATH]{};
        GetConfigDir(dir, sizeof(dir));
        snprintf(out, sz, "%s\\slot_%d.dat", dir, slot);
    }

    inline void LoadSlotNames()
    {
        if (slotsLoaded) return;
        slotsLoaded = true;
        char dir[MAX_PATH]{}, path[MAX_PATH]{};
        GetConfigDir(dir, sizeof(dir));
        snprintf(path, sizeof(path), "%s\\names.txt", dir);
        FILE* f = nullptr;
        fopen_s(&f, path, "r");
        if (!f) return;
        for (int i = 0; i < kMaxSlots; ++i)
        {
            char line[64]{};
            if (!fgets(line, sizeof(line), f)) break;
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') line[--len] = '\0';
            if (len > 0 && line[len-1] == '\r') line[--len] = '\0';
            strncpy_s(slotNames[i], line, 31);
        }
        fclose(f);
    }

    inline void SaveSlotNames()
    {
        char dir[MAX_PATH]{}, path[MAX_PATH]{};
        GetConfigDir(dir, sizeof(dir));
        snprintf(path, sizeof(path), "%s\\names.txt", dir);
        FILE* f = nullptr;
        fopen_s(&f, path, "w");
        if (!f) return;
        for (int i = 0; i < kMaxSlots; ++i)
            fprintf(f, "%s\n", slotNames[i]);
        fclose(f);
    }

    inline bool SlotExists(int slot)
    {
        char path[MAX_PATH]{};
        GetSlotPath(slot, path, sizeof(path));
        return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
    }

    inline bool SaveConfig(int slot)
    {
        char path[MAX_PATH]{};
        GetSlotPath(slot, path, sizeof(path));
        FILE* f = nullptr;
        fopen_s(&f, path, "wb");
        if (!f) return false;
        SavedConfig cfg{};
        strncpy_s(cfg.name, slotNames[slot], 31);
        cfg.aimbot       = Aimbot::cfg;
        cfg.esp          = ESP::cfg;
        cfg.tracer       = BulletTracer::cfg;
        cfg.worldEffects = WorldEffects::cfg;
        cfg.damage       = DamageIndicator::cfg;
        cfg.wireframe    = WireframeHands::cfg;
        cfg.chams        = Chams::cfg;
        cfg.bhop         = Bhop::cfg;
        cfg.skinchanger  = SkinChanger::cfg;
        cfg.menuStyle    = (int)Menu::activeStyle;
        memcpy(cfg.priColor, Menu::primaryColor, sizeof(cfg.priColor));
        memcpy(cfg.secColor, Menu::secondaryColor, sizeof(cfg.secColor));
        fwrite(&cfg, sizeof(cfg), 1, f);
        fclose(f);
        return true;
    }

    inline bool LoadConfig(int slot)
    {
        char path[MAX_PATH]{};
        GetSlotPath(slot, path, sizeof(path));
        FILE* f = nullptr;
        fopen_s(&f, path, "rb");
        if (!f) return false;
        uint32_t hdr[3]{};
        if (fread(hdr, sizeof(hdr), 1, f) != 1) { fclose(f); return false; }
        if (hdr[0] != 0x4C554349 || hdr[1] != 15) { fclose(f); return false; }
        fseek(f, 0, SEEK_SET);
        SavedConfig cfg{};
        fseek(f, 0, SEEK_END);
        long fsz = ftell(f);
        fseek(f, 0, SEEK_SET);
        size_t rsz = ((size_t)fsz < sizeof(cfg)) ? (size_t)fsz : sizeof(cfg);
        if (fread(&cfg, 1, rsz, f) < 12) { fclose(f); return false; }
        fclose(f);
        if (cfg.menuStyle < 0 || cfg.menuStyle > 2) cfg.menuStyle = 0;
        if (cfg.aimbot.fov < 0.1f)        cfg.aimbot.fov = 2.2f;
        if (cfg.aimbot.smoothing < 1.f)   cfg.aimbot.smoothing = 45.f;
        Aimbot::cfg          = cfg.aimbot;
        ESP::cfg             = cfg.esp;
        BulletTracer::cfg    = cfg.tracer;
        WorldEffects::cfg    = cfg.worldEffects;
        DamageIndicator::cfg = cfg.damage;
        WireframeHands::cfg  = cfg.wireframe;
        Chams::cfg           = cfg.chams;
        Bhop::cfg            = cfg.bhop;
        SkinChanger::cfg     = cfg.skinchanger;
        Menu::activeStyle    = (Menu::MenuStyle)cfg.menuStyle;
        memcpy(Menu::primaryColor,   cfg.priColor, sizeof(cfg.priColor));
        memcpy(Menu::secondaryColor, cfg.secColor, sizeof(cfg.secColor));
        Menu::themeApplied = false;
        Aimbot::ResetState();
        return true;
    }


    // ============================================================
    //  PRESETS
    // ============================================================

    inline void ApplyPreset_Undetected()
    {
        Aimbot::cfg.enabled       = true;
        Aimbot::cfg.fov           = 1.5f;
        Aimbot::cfg.smoothing     = 65.f;
        Aimbot::cfg.humanization  = 0.65f;
        Aimbot::cfg.targetBone    = 4;
        Aimbot::cfg.aimKey        = 0;
        Aimbot::cfg.teamCheck     = true;
        Aimbot::cfg.visCheck      = true;
        Aimbot::cfg.showFovCircle = false;
        Aimbot::cfg.jumpShot      = false;
        Aimbot::cfg.velPredict    = false;
        Aimbot::cfg.multiBone     = false;
        Aimbot::cfg.headPriority  = false;
        Aimbot::cfg.smokeCheck    = true;
        Aimbot::ResetState();
        ESP::cfg.enabled          = false;
        BulletTracer::cfg.enabled = false;
        Bhop::cfg.enabled         = false;
    }

    inline void ApplyPreset_LegitAim()
    {
        Aimbot::cfg.enabled       = true;
        Aimbot::cfg.fov           = 2.0f;
        Aimbot::cfg.smoothing     = 50.f;
        Aimbot::cfg.humanization  = 0.55f;
        Aimbot::cfg.targetBone    = 6;
        Aimbot::cfg.aimKey        = 0;
        Aimbot::cfg.teamCheck     = true;
        Aimbot::cfg.visCheck      = true;
        Aimbot::cfg.showFovCircle = true;
        Aimbot::cfg.velPredict    = true;
        Aimbot::cfg.multiBone     = true;
        Aimbot::cfg.headPriority  = true;
        Aimbot::cfg.smokeCheck    = true;
        Aimbot::ResetState();
        ESP::cfg.enabled   = true;
        ESP::cfg.box       = true;
        ESP::cfg.healthBar = true;
        ESP::cfg.name      = true;
        ESP::cfg.distance  = true;
        BulletTracer::cfg.enabled = true;
        Bhop::cfg.enabled  = false;
    }

    inline void ApplyPreset_SilentAim()
    {
        Aimbot::cfg.enabled       = true;
        Aimbot::cfg.fov           = 2.3f;
        Aimbot::cfg.smoothing     = 43.f;
        Aimbot::cfg.humanization  = 0.50f;
        Aimbot::cfg.targetBone    = 6;
        Aimbot::cfg.aimKey        = 0;
        Aimbot::cfg.teamCheck     = true;
        Aimbot::cfg.visCheck      = true;
        Aimbot::cfg.showFovCircle = true;
        Aimbot::cfg.velPredict    = true;
        Aimbot::cfg.multiBone     = true;
        Aimbot::cfg.headPriority  = true;
        Aimbot::cfg.smokeCheck    = true;
        Aimbot::ResetState();
        ESP::cfg.enabled   = true;
        ESP::cfg.box       = true;
        ESP::cfg.skeleton  = true;
        ESP::cfg.healthBar = true;
        ESP::cfg.name      = true;
        ESP::cfg.distance  = true;
        ESP::cfg.teamCheck = true;
        BulletTracer::cfg.enabled = true;
        Bhop::cfg.enabled  = true;
    }

    inline void ApplyPreset_Rage()
    {
        Aimbot::cfg.enabled           = true;
        Aimbot::cfg.fov               = 10.f;
        Aimbot::cfg.smoothing         = 8.f;
        Aimbot::cfg.humanization      = 0.15f;
        Aimbot::cfg.targetBone        = 6;
        Aimbot::cfg.aimKey            = 0;
        Aimbot::cfg.teamCheck         = true;
        Aimbot::cfg.visCheck          = true;
        Aimbot::cfg.showFovCircle     = false;
        Aimbot::cfg.jumpShot          = true;
        Aimbot::cfg.jumpApexOnly      = true;
        Aimbot::cfg.jumpApexThreshold = 40.f;
        Aimbot::cfg.velPredict        = true;
        Aimbot::cfg.multiBone         = true;
        Aimbot::cfg.headPriority      = true;
        Aimbot::cfg.smokeCheck        = true;
        Aimbot::ResetState();
        ESP::cfg.enabled   = true;
        ESP::cfg.box       = true;
        ESP::cfg.skeleton  = true;
        ESP::cfg.healthBar = true;
        ESP::cfg.name      = true;
        ESP::cfg.distance  = true;
        ESP::cfg.teamCheck = false;
        BulletTracer::cfg.enabled = true;
        Bhop::cfg.enabled         = true;
        Bhop::cfg.autoStrafe      = true;
    }

    inline void ApplyPreset_Off()
    {
        Aimbot::cfg.enabled       = false;
        ESP::cfg.enabled          = false;
        BulletTracer::cfg.enabled = false;
        Bhop::cfg.enabled         = false;
    }


    // ============================================================
    //  TAB CONTENT
    // ============================================================

    inline void Left_Aim()
    {
        SynthBeginSection("##aim_s1");
        EvoLabel("AIMBOT");
        EvoCheckbox("Enable", &Aimbot::cfg.enabled);
        SynthSep();
        {
            const char* akn[] = { "Auto (Mouse1)", "Right Click", "Always On" };
            EvoCombo("Aim Key##ak", &Aimbot::cfg.aimKey, akn, 3);
        }
        SynthSep();
        EvoCheckbox("Team Check",    &Aimbot::cfg.teamCheck);
        SynthSep();
        EvoCheckbox("Head Priority", &Aimbot::cfg.headPriority);
        SynthSep();
        EvoCheckbox("Smoke Check",   &Aimbot::cfg.smokeCheck);
        SynthSep();
        EvoCheckbox("Silent Aim",    &Aimbot::cfg.silentAim);
        if (Aimbot::cfg.silentAim)
        {
            SynthSep();
            EvoSliderFloat("Max Delta##sd", &Aimbot::cfg.silentMaxDelta, 0.5f, 5.f, "%.1f");
        }
        SynthEndSection();

        SynthBeginSection("##aim_s2");
        EvoLabel("AIM FEEL");
        EvoSliderFloat("FOV",          &Aimbot::cfg.fov,          0.5f, 30.f,  "%.1f");
        SynthSep();
        EvoSliderFloat("Smoothing",    &Aimbot::cfg.smoothing,    1.f,  100.f, "%.0f");
        SynthSep();
        EvoSliderFloat("Humanization", &Aimbot::cfg.humanization, 0.f,  1.f,   "%.2f");
        SynthSep();
        EvoCheckbox("No Recoil",       &Aimbot::cfg.noRecoil);
        SynthEndSection();
    }

    inline void Right_Aim()
    {
        SynthBeginSection("##aim_r1");
        EvoLabel("TARGETING");
        {
            const char* bones[]   = { "Head","Neck","Chest","Pelvis" };
            const int   boneIds[] = { 6, 5, 4, 3 };
            int bIdx = 0;
            for (int i = 0; i < 4; ++i) if (boneIds[i] == Aimbot::cfg.targetBone) bIdx = i;
            if (EvoCombo("Hitbox##hb", &bIdx, bones, 4))
                Aimbot::cfg.targetBone = boneIds[bIdx];
        }
        SynthSep();
        EvoCheckbox("Multi-Bone Scan",  &Aimbot::cfg.multiBone);
        SynthSep();
        EvoCheckbox("Velocity Predict", &Aimbot::cfg.velPredict);
        if (Aimbot::cfg.velPredict)
        {
            SynthSep();
            EvoSliderFloat("Predict Scale##ps", &Aimbot::cfg.velPredictScale, 0.1f, 3.f, "%.2f");
        }
        SynthSep();
        EvoCheckbox("Show FOV Circle",  &Aimbot::cfg.showFovCircle);
        SynthSep();
        EvoCheckbox("Jump Shot",        &Aimbot::cfg.jumpShot);
        if (Aimbot::cfg.jumpShot)
        {
            SynthSep();
            EvoCheckbox("Apex Only##jo", &Aimbot::cfg.jumpApexOnly);
        }
        SynthSep();
        EvoCheckbox("No Spread",        &Aimbot::cfg.noSpread);
        SynthEndSection();

        SynthBeginSection("##aim_r2");
        EvoLabel("STAT GOVERNOR");
        EvoCheckbox("Enable##gov",  &Aimbot::Governor::gcfg.enabled);
        if (Aimbot::Governor::gcfg.enabled)
        {
            SynthSep();
            EvoSliderFloat("Intensity##gi",  &Aimbot::Governor::gcfg.intensity,    0.f,  1.f, "%.2f");
            SynthSep();
            EvoSliderFloat("HS Cap %%##ghc", &Aimbot::Governor::gcfg.hsCapPercent, 30.f, 80.f, "%.0f");
            SynthSep();
            if (EvoButton("Reset Session##gr")) Aimbot::Governor::ResetSession();
        }
        SynthEndSection();

        SynthBeginSection("##aim_r3");
        EvoLabel("BUNNY HOP");
        EvoCheckbox("Enable##bhop",      &Bhop::cfg.enabled);
        if (Bhop::cfg.enabled)
        {
            SynthSep();
            EvoSliderFloat("Max Speed##bms", &Bhop::cfg.maxSpeed, 200.f, 500.f, "%.0f");
            SynthSep();
            EvoSliderFloat("Min Speed##bns", &Bhop::cfg.minSpeed,  10.f, 120.f, "%.0f");
            SynthSep();
            EvoCheckbox("Auto Strafe##bs", &Bhop::cfg.autoStrafe);
        }
        SynthSep();
        EvoCheckbox("Velocity Display##bvd", &Bhop::cfg.showVelocity);
        SynthEndSection();

        SynthBeginSection("##aim_triggerbot");
        EvoLabel("TRIGGERBOT");
        EvoCheckbox("Enable##trig",      &Triggerbot::cfg.enabled);
        if (Triggerbot::cfg.enabled)
        {
            SynthSep();
            EvoCheckbox("Team Check##ttc",   &Triggerbot::cfg.teamCheck);
            SynthSep();
            EvoCheckbox("Smoke Check##tsc",  &Triggerbot::cfg.smokeCheck);
            SynthSep();
            EvoCheckbox("Scope Only##tso",   &Triggerbot::cfg.scopeOnly);
            SynthSep();
            ImGui::SliderInt("Min Delay##tmd", &Triggerbot::cfg.minDelayMs, 0, 200);
            SynthSep();
            ImGui::SliderInt("Max Delay##txd", &Triggerbot::cfg.maxDelayMs, 0, 300);
        }
        SynthEndSection();

        SynthBeginSection("##aim_backtrack");
        EvoLabel("BACKTRACK");
        EvoCheckbox("Enable##bt",         &Backtrack::cfg.enabled);
        if (Backtrack::cfg.enabled)
        {
            SynthSep();
            ImGui::SliderInt("Max Ticks##btmt", &Backtrack::cfg.maxTicksBack, 1, 20);
            SynthSep();
            EvoCheckbox("Draw History##btdh", &Backtrack::cfg.drawHistory);
        }
        SynthEndSection();

        SynthBeginSection("##aim_antiaim");
        EvoLabel("ANTI-AIM");
        EvoCheckbox("Enable##aa",     &AntiAim::cfg.enabled);
        if (AntiAim::cfg.enabled)
        {
            static const char* pitchModes[] = { "Off", "Down", "Up", "Zero" };
            static const char* yawModes[]   = { "Off", "Spin", "Jitter", "Desync" };
            SynthSep();
            ImGui::Combo("Pitch##aap", &AntiAim::cfg.pitchMode, pitchModes, 4);
            SynthSep();
            ImGui::Combo("Yaw##aay",   &AntiAim::cfg.yawMode,   yawModes,   4);
            if (AntiAim::cfg.yawMode == 1)
            {
                SynthSep();
                EvoSliderFloat("Speed##aas", &AntiAim::cfg.spinSpeed, 1.f, 45.f, "%.0f");
            }
            if (AntiAim::cfg.yawMode == 3)
            {
                SynthSep();
                EvoSliderFloat("Delta##aad", &AntiAim::cfg.desyncDelta, 10.f, 58.f, "%.0f");
            }
        }
        SynthEndSection();

        SynthBeginSection("##aim_fakelag");
        EvoLabel("FAKE LAG");
        EvoCheckbox("Enable##fl",     &FakeLag::cfg.enabled);
        if (FakeLag::cfg.enabled)
        {
            static const char* flModes[] = { "Fixed", "Dynamic", "On Key" };
            SynthSep();
            ImGui::Combo("Mode##flm", &FakeLag::cfg.mode, flModes, 3);
            SynthSep();
            ImGui::SliderInt("Max Choke##flmc", &FakeLag::cfg.maxChoke, 1, 14);
        }
        SynthEndSection();
    }

    inline void Left_Vis()
    {
        SynthBeginSection("##vis_s1");
        EvoLabel("ESP");
        EvoCheckbox("Enable ESP", &ESP::cfg.enabled);
        if (ESP::cfg.enabled)
        {
            SynthSep();
            EvoCheckbox("Box",       &ESP::cfg.box);
            if (ESP::cfg.box)
            {
                SynthSep();
                const char* bst[] = { "Normal","Corners" };
                EvoCombo("Style##bs", &ESP::cfg.boxStyle, bst, 2);
            }
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Box Color##bc", ESP::cfg.boxColor,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            SynthSep();
            EvoCheckbox("Skeleton",   &ESP::cfg.skeleton);
            SynthSep();
            EvoCheckbox("Health Bar", &ESP::cfg.healthBar);
            SynthSep();
            EvoCheckbox("Name",       &ESP::cfg.name);
            SynthSep();
            EvoCheckbox("Distance",   &ESP::cfg.distance);
            SynthSep();
            EvoCheckbox("Weapon",     &ESP::cfg.weapon);
            SynthSep();
            EvoCheckbox("Team Check##etm", &ESP::cfg.teamCheck);
            SynthSep();
            EvoCheckbox("Vis Color",  &ESP::cfg.visColorEnabled);
            if (ESP::cfg.visColorEnabled)
            {
                SynthSep();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::ColorEdit4("Visible##vc", ESP::cfg.visibleColor,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                SynthSep();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::ColorEdit4("Hidden##hc",  ESP::cfg.hiddenColor,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }
        }
        SynthEndSection();

        SynthBeginSection("##vis_s2");
        EvoLabel("CHAMS");
        EvoCheckbox("Enable Chams",  &Chams::cfg.enabled);
        if (Chams::cfg.enabled)
        {
            SynthSep();
            EvoCheckbox("Wallhack##cw", &Chams::cfg.wallhack);
            auto SlotW = [](const char* n, Chams::SlotStyle& sl) {
                ImGui::PushID(n);
                if (sl.material < 0 || sl.material >= Chams::MAT_COUNT) sl.material = Chams::MAT_NONE;
                ImGui::Text("  %s", n);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 30);
                if (ImGui::BeginCombo("##m", Chams::MaterialNames[sl.material]))
                {
                    for (int i = 0; i < Chams::MAT_COUNT; ++i)
                    {
                        bool sel = (sl.material == i);
                        if (ImGui::Selectable(Chams::MaterialNames[i], sel)) sl.material = i;
                        if (sel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                if (sl.material != Chams::MAT_NONE)
                {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##cc", sl.color,
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                }
                ImGui::PopID();
            };
            SynthSep();
            SlotW("Visible", Chams::cfg.playerVis);
            if (Chams::cfg.wallhack) { SynthSep(); SlotW("Walls", Chams::cfg.playerHid); }
            SynthSep();
            EvoCheckbox("Hand Chams##hc",   &Chams::cfg.handsEnabled);
            if (Chams::cfg.handsEnabled)  { SynthSep(); SlotW("Hands",   Chams::cfg.hands);   }
            SynthSep();
            EvoCheckbox("Weapon Chams##wc", &Chams::cfg.weaponsEnabled);
            if (Chams::cfg.weaponsEnabled){ SynthSep(); SlotW("Weapons", Chams::cfg.weapons); }
        }
        SynthEndSection();
    }

    inline void Right_Vis()
    {
        SynthBeginSection("##vis_r1");
        EvoLabel("EFFECTS");
        EvoCheckbox("Bullet Tracers",    &BulletTracer::cfg.enabled);
        SynthSep();
        EvoCheckbox("Damage Indicators", &DamageIndicator::cfg.enabled);
        if (DamageIndicator::cfg.enabled)
        {
            SynthSep();
            const char* dp[] = { "Left","Right" };
            const char* ds[] = { "Classic","Minimal","Bold" };
            EvoCombo("Position##dp", &DamageIndicator::cfg.position, dp, 2);
            SynthSep();
            EvoCombo("Style##ds",    &DamageIndicator::cfg.style,    ds, 3);
        }
        SynthEndSection();

        SynthBeginSection("##vis_r2");
        EvoLabel("EXTRAS");
        EvoCheckbox("Bomb Timer",     &ESP::cfg.bombTimer);
        SynthSep();
        EvoCheckbox("Spectator List", &ESP::cfg.spectators);
        SynthSep();
        EvoCheckbox("Rank Revealer",  &RankRevealer::cfg.enabled);
        SynthEndSection();

        SynthBeginSection("##vis_r3");
        EvoLabel("GRENADE PRED.");
        EvoCheckbox("Enable##gp",       &GrenadePrediction::cfg.enabled);
        if (GrenadePrediction::cfg.enabled)
        {
            SynthSep();
            EvoCheckbox("Show Trail##gt",   &GrenadePrediction::cfg.showTrail);
            SynthSep();
            EvoCheckbox("Show Landing##gl", &GrenadePrediction::cfg.showLanding);
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Trail##tc", GrenadePrediction::cfg.trailColor,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        }
        SynthEndSection();

        SynthBeginSection("##vis_r4");
        EvoLabel("NADE HELPER");
        EvoCheckbox("Enable##nh", &NadeHelper::cfg.enabled);
        if (NadeHelper::cfg.enabled)
        {
            SynthSep();
            EvoSliderFloat("Trigger Dist##nt", &NadeHelper::cfg.triggerDist,  50.f, 300.f, "%.0f");
            SynthSep();
            EvoSliderFloat("Aim Tol##na",      &NadeHelper::cfg.aimTolerance,  1.f,  10.f, "%.1f");
        }
        SynthEndSection();

        SynthBeginSection("##vis_r_sound");
        EvoLabel("SOUND ESP");
        EvoCheckbox("Enable##sesp",       &SoundESP::cfg.enabled);
        if (SoundESP::cfg.enabled)
        {
            SynthSep();
            EvoCheckbox("Footstep Marks##sfm", &SoundESP::cfg.showFootsteps);
            SynthSep();
            EvoSliderFloat("Max Dist##smd", &SoundESP::cfg.maxDistance, 500.f, 4000.f, "%.0f");
            SynthSep();
            EvoSliderFloat("Ring Size##srs", &SoundESP::cfg.ringRadius, 30.f, 150.f, "%.0f");
            SynthSep();
            EvoSliderFloat("Arrow Size##sas", &SoundESP::cfg.indicatorSize, 15.f, 80.f, "%.0f");
        }
        SynthEndSection();
    }

    inline void Left_Skn()
    {
        SynthBeginSection("##skn_s1");
        EvoLabel("OPTIONS");
        EvoCheckbox("Enable Skin Changer", &SkinChanger::cfg.enabled);
        if (SkinChanger::cfg.enabled)
        {
            SynthSep();
            if (EvoButton("Randomize All##ra")) SkinChanger::RandomizeAll();
            SynthSep();
            if (EvoButton("Force Update##fu"))
            {
                SkinChanger::ForceFullUpdate();
                SkinChanger::lastKnifeDefIdx    = 0;
                SkinChanger::lastGloveSpawnTime = 0.f;
                SkinChanger::gloveRefreshFrames = 0;
            }
        }
        SynthEndSection();

        if (!SkinChanger::cfg.enabled) return;

        SynthBeginSection("##skn_s2");
        EvoLabel("WEAPON");
        static const char* wn[SkinChanger::kWeaponCount]{};
        static bool wnInit = false;
        if (!wnInit) { for (int i = 0; i < SkinChanger::kWeaponCount; ++i) wn[i] = SkinChanger::kWeapons[i].name; wnInit = true; }
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##wsel", &SkinChanger::cfg.activeWeaponSlot, wn, SkinChanger::kWeaponCount);
        int ws = SkinChanger::cfg.activeWeaponSlot;
        if (ws >= 0 && ws < SkinChanger::kWeaponCount)
        {
            auto& skin = SkinChanger::cfg.weapons[ws];
            SynthSep();
            EvoCheckbox("Enable##wp", &skin.enabled);
            if (skin.enabled)
            {
                SynthSep();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputInt("Paint Kit##wpk", &skin.paintKit);
                if (skin.paintKit < 0) skin.paintKit = 0;
                SynthSep();
                EvoSliderFloat("Wear##ww", &skin.wear, 0.f, 1.f, "%.4f");
                SynthSep();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputInt("StatTrak##wst", &skin.statTrak);
            }
        }
        SynthEndSection();

        SynthBeginSection("##skn_s3");
        EvoLabel("QUICK APPLY");
        auto QS = [](const char* n, int pk) {
            if (EvoButton(n)) {
                int s = SkinChanger::cfg.activeWeaponSlot;
                if (s >= 0 && s < SkinChanger::kWeaponCount)
                { SkinChanger::cfg.weapons[s].enabled = true; SkinChanger::cfg.weapons[s].paintKit = pk; SkinChanger::cfg.weapons[s].wear = 0.0001f; }
            }
        };
        QS("Dragon Lore##ql", 344);
        QS("Asiimov##ql",     279);
        QS("Hyper Beast##ql", 475);
        QS("Fade##ql",         38);
        SynthEndSection();
    }

    inline void Right_Skn()
    {
        SynthBeginSection("##skn_r1");
        EvoLabel("KNIFE CHANGER");
        EvoCheckbox("Enable##kc", &SkinChanger::cfg.knifeEnabled);
        if (SkinChanger::cfg.knifeEnabled)
        {
            SynthSep();
            static const char* kn[SkinChanger::kKnifeCount]{};
            static bool knInit = false;
            if (!knInit) { for (int i = 0; i < SkinChanger::kKnifeCount; ++i) kn[i] = SkinChanger::kKnives[i].name; knInit = true; }
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::Combo("Model##km", &SkinChanger::cfg.knifeModel, kn, SkinChanger::kKnifeCount))
                SkinChanger::ForceFullUpdate();
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputInt("Paint Kit##kpk", &SkinChanger::cfg.knifePaintKit);
            if (SkinChanger::cfg.knifePaintKit < 0) SkinChanger::cfg.knifePaintKit = 0;
            SynthSep();
            EvoSliderFloat("Wear##kw", &SkinChanger::cfg.knifeWear, 0.f, 1.f, "%.4f");
            SynthSep();
            auto KQ = [](const char* n, int pk) {
                if (EvoButton(n)) { SkinChanger::cfg.knifePaintKit = pk; SkinChanger::cfg.knifeWear = 0.0001f; SkinChanger::ForceFullUpdate(); }
            };
            KQ("Fade##kq",         38);
            KQ("Crimson Web##kq",  12);
            KQ("Doppler##kq",     415);
            KQ("Tiger Tooth##kq", 409);
        }
        SynthEndSection();

        SynthBeginSection("##skn_r2");
        EvoLabel("GLOVE CHANGER");
        EvoCheckbox("Enable##gc", &SkinChanger::cfg.gloveEnabled);
        if (SkinChanger::cfg.gloveEnabled)
        {
            SynthSep();
            static const char* gn[SkinChanger::kGloveCount]{};
            static bool gnInit = false;
            if (!gnInit) { for (int i = 0; i < SkinChanger::kGloveCount; ++i) gn[i] = SkinChanger::kGloves[i].name; gnInit = true; }
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::Combo("Model##gm", &SkinChanger::cfg.gloveModel, gn, SkinChanger::kGloveCount))
                SkinChanger::ForceFullUpdate();
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputInt("Paint Kit##gpk", &SkinChanger::cfg.glovePaintKit);
            if (SkinChanger::cfg.glovePaintKit < 0) SkinChanger::cfg.glovePaintKit = 0;
            SynthSep();
            EvoSliderFloat("Wear##gw", &SkinChanger::cfg.gloveWear, 0.f, 1.f, "%.4f");
            SynthSep();
            auto GQ = [](const char* n, int pk) {
                if (EvoButton(n)) { SkinChanger::cfg.glovePaintKit = pk; SkinChanger::cfg.gloveWear = 0.0001f; }
            };
            GQ("Pandora's Box##gq",  10006);
            GQ("Crimson Kimono##gq", 10007);
            GQ("Emerald Web##gq",    10036);
        }
        SynthEndSection();
    }

    inline void Left_Wld()
    {
        SynthBeginSection("##wld_s1");
        EvoLabel("SKY COLOR");
        EvoCheckbox("Sky Override##so", &WorldEffects::cfg.skyEnabled);
        if (WorldEffects::cfg.skyEnabled)
        {
            SynthSep();
            EvoCheckbox("Rainbow Sky##rs", &WorldEffects::cfg.skyRainbow);
            SynthSep();
            if (WorldEffects::cfg.skyRainbow)
                EvoSliderFloat("Speed##rss", &WorldEffects::cfg.skyRainbowSpeed, 0.05f, 2.f, "%.2f");
            else
            {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::ColorEdit4("Sky Color##scc", WorldEffects::cfg.skyColor,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }
            SynthSep();
            EvoSliderFloat("Brightness##sb", &WorldEffects::cfg.skyBrightness, 0.1f, 5.f, "%.1f");
        }
        SynthEndSection();

        SynthBeginSection("##wld_s2");
        EvoLabel("FLASH / SMOKE");
        EvoCheckbox("No Flash##nf",      &WorldEffects::cfg.noFlash);
        if (WorldEffects::cfg.noFlash) { SynthSep(); EvoSliderFloat("Max Alpha##mfa", &WorldEffects::cfg.maxFlashAlpha, 0.f, 255.f, "%.0f"); }
        SynthSep();
        EvoCheckbox("No Smoke##ns",      &WorldEffects::cfg.noSmoke);
        SynthSep();
        EvoCheckbox("Smoke Color##smoC", &WorldEffects::cfg.smokeColor);
        if (WorldEffects::cfg.smokeColor)
        {
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Smoke##st", WorldEffects::cfg.smokeCol,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        }
        SynthEndSection();

        SynthBeginSection("##wld_s3");
        EvoLabel("FIRE");
        EvoCheckbox("Fire Color##fco", &WorldEffects::cfg.fireColor);
        if (WorldEffects::cfg.fireColor)
        {
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Fire##fc", WorldEffects::cfg.fireCol,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        }
        SynthEndSection();

        SynthBeginSection("##wld_s4");
        EvoLabel("FOV CHANGER");
        EvoCheckbox("FOV Override##fovc", &WorldEffects::cfg.fovEnabled);
        if (WorldEffects::cfg.fovEnabled)
        {
            SynthSep();
            EvoSliderFloat("FOV Value##fv", &WorldEffects::cfg.fovValue, 60.f, 150.f, "%.0f");
        }
        SynthEndSection();
    }

    inline void Right_Wld()
    {
        SynthBeginSection("##wld_r1");
        EvoLabel("NIGHT MODE");
        const char* nm[] = { "Off","Night","Midnight","Sunset","Blood Moon" };
        EvoCombo("Mode##nm", &WorldEffects::cfg.nightMode, nm, 5);
        SynthEndSection();

        SynthBeginSection("##wld_r2");
        EvoLabel("FOG");
        EvoCheckbox("Custom Fog##cf", &WorldEffects::cfg.fogEnabled);
        if (WorldEffects::cfg.fogEnabled)
        {
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit3("Fog Color##fgc", WorldEffects::cfg.fogColor, ImGuiColorEditFlags_NoInputs);
            SynthSep();
            EvoSliderFloat("Start##fs",   &WorldEffects::cfg.fogStart,    0.f,    2000.f, "%.0f");
            SynthSep();
            EvoSliderFloat("End##fe",     &WorldEffects::cfg.fogEnd,      500.f, 20000.f, "%.0f");
            SynthSep();
            EvoSliderFloat("Density##fd", &WorldEffects::cfg.fogDensity,   0.f,    1.f,   "%.2f");
        }
        SynthEndSection();

        SynthBeginSection("##wld_r3");
        EvoLabel("EXPOSURE");
        EvoCheckbox("Brightness Override##bo", &WorldEffects::cfg.brightnessEnabled);
        if (WorldEffects::cfg.brightnessEnabled)
        {
            SynthSep();
            EvoSliderFloat("Min Exp##me", &WorldEffects::cfg.exposureMin, 0.1f, 5.f, "%.2f");
            SynthSep();
            EvoSliderFloat("Max Exp##xe", &WorldEffects::cfg.exposureMax, 0.1f, 5.f, "%.2f");
            SynthSep();
            if (EvoButton("Fullbright##fb")) WorldEffects::cfg.exposureMin = WorldEffects::cfg.exposureMax = 3.f;
        }
        SynthEndSection();

        SynthBeginSection("##wld_r4");
        EvoLabel("MISC");
        EvoCheckbox("Third Person##tp",   &WorldEffects::cfg.thirdPerson);
        SynthSep();
        EvoCheckbox("Wireframe View##wv", &WireframeHands::cfg.enabled);
        if (WireframeHands::cfg.enabled)
        {
            SynthSep();
            EvoCheckbox("Hands Only##wh", &WireframeHands::cfg.handsOnly);
            SynthSep();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Wire Color##wc", WireframeHands::cfg.color,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        }
        SynthSep();
        EvoCheckbox("Auto-Accept##aa", &AutoAccept::cfg.enabled);
        if (AutoAccept::cfg.enabled) { SynthSep(); EvoSliderFloat("Delay##aad", &AutoAccept::cfg.delay, 0.1f, 3.f, "%.1f"); }
        SynthEndSection();
    }

    inline void Left_Cfg()
    {
        SynthBeginSection("##cfg_s1");
        EvoLabel("PRESETS");
        ImGui::TextColored({ 0.4f,1.f,0.4f,1.f }, "  Safe");
        ImGui::Dummy({ 0.f, 2.f });
        if (EvoButton("Undetected##pu"))  ApplyPreset_Undetected();
        SynthSep();
        if (EvoButton("Legit Aim##pla"))  ApplyPreset_LegitAim();
        ImGui::Dummy({ 0.f, 4.f });
        ImGui::TextColored({ 1.f,0.7f,0.3f,1.f }, "  Risky");
        ImGui::Dummy({ 0.f, 2.f });
        if (EvoButton("Aggressive##pal")) ApplyPreset_SilentAim();
        ImGui::Dummy({ 0.f, 4.f });
        ImGui::TextColored({ 1.f,0.4f,0.4f,1.f }, "  Danger");
        ImGui::Dummy({ 0.f, 2.f });
        if (EvoButton("Rage##pr"))        ApplyPreset_Rage();
        SynthSep();
        if (EvoButton("All Off##pao"))    ApplyPreset_Off();
        SynthEndSection();

        SynthBeginSection("##cfg_s2");
        EvoLabel("ACCENT COLOR");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::ColorEdit4("##pri", primaryColor,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar
                | ImGuiColorEditFlags_PickerHueWheel))
            themeApplied = false;
        SynthEndSection();
    }

    inline void Right_Cfg()
    {
        SynthBeginSection("##cfg_r1");
        EvoLabel("CONFIG SLOTS");
        LoadSlotNames();
        for (int i = 0; i < kMaxSlots; ++i)
        {
            ImGui::PushID(i);
            bool exists = SlotExists(i);
            ImGui::TextColored(
                exists ? ImVec4{0.80f,0.90f,0.80f,1.f} : ImVec4{0.40f,0.42f,0.44f,1.f},
                "%s%s", slotNames[i], exists ? "" : "  (empty)");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##rn", slotNames[i], sizeof(slotNames[i]));
            float bw = (ImGui::GetContentRegionAvail().x - 6.f) * 0.5f;
            if (EvoButton("Save##sv", { bw, 22.f })) { SaveConfig(i); SaveSlotNames(); }
            ImGui::SameLine(0, 6.f);
            if (exists) { if (EvoButton("Load##ld", { bw, 22.f })) LoadConfig(i); }
            else { ImGui::BeginDisabled(); EvoButton("Load##ld", { bw, 22.f }); ImGui::EndDisabled(); }
            if (i < kMaxSlots - 1) SynthSep();
            ImGui::PopID();
        }
        SynthEndSection();
    }


    // ============================================================
    //  MAIN RENDER  -  Menu 19 "framework" chrome
    // ============================================================
    inline void Render(bool& showMenu)
    {
        ApplyTheme();

        float dt = ImGui::GetIO().DeltaTime;
        if (showMenu  && menuAlpha < 1.f) menuAlpha += dt * 5.f;
        if (!showMenu && menuAlpha > 0.f) menuAlpha -= dt * 7.f;
        if (menuAlpha < 0.f) menuAlpha = 0.f;
        if (menuAlpha > 1.f) menuAlpha = 1.f;
        if (menuAlpha <= 0.001f) return;

        // Layout (menu 19: 840x630, sidebar 110px)
        const float W      = 840.f;
        const float H      = 630.f;
        const float SIDE_W = 110.f;
        const float PAD    = 15.f;
        const float COL_Y  = 20.f;
        const float COL_H  = H - COL_Y - PAD;               // 595
        const float COL_X  = SIDE_W + PAD;                  // 125
        const float COL_W  = (W - COL_X - PAD - 10.f) * 0.5f; // 345

        ImGui::SetNextWindowSize({ W, H }, ImGuiCond_Once);
        ImGui::SetNextWindowPos({ 100.f, 80.f }, ImGuiCond_Once);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    { 0.f, 0.f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,            menuAlpha);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, { 13/255.f, 13/255.f, 18/255.f, 1.f });

        ImGui::Begin("##lucid19", nullptr,
            ImGuiWindowFlags_NoTitleBar    | ImGuiWindowFlags_NoResize          |
            ImGuiWindowFlags_NoScrollbar   | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse    | ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImVec2 wp  = ImGui::GetWindowPos();
        ImVec2 ws  = ImGui::GetWindowSize();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Window background + outer stroke
        dl->AddRectFilled(wp, { wp.x + ws.x, wp.y + ws.y }, kWinBg, 12.f);
        dl->AddRect(wp, { wp.x + ws.x, wp.y + ws.y }, kOuterStr, 12.f);

        // Right content panel
        ImVec2 panTL = { wp.x + SIDE_W, wp.y + PAD };
        ImVec2 panBR = { wp.x + ws.x - PAD, wp.y + ws.y - PAD };
        dl->AddRectFilled(panTL, panBR, kRightBg, 8.f);
        dl->AddRect(panTL, panBR, kInnerStr, 8.f);

        // Gradient accent lines (top + bottom edges, menu 19 exact)
        float midX = wp.x + ws.x * 0.5f;
        ImU32 a20  = EvoAccent(51);
        ImU32 a0   = EvoAccent(0);
        dl->AddRectFilledMultiColor({ midX,  wp.y              }, { wp.x+ws.x, wp.y+1.f        }, a20, a0,  a0,  a20);
        dl->AddRectFilledMultiColor({ wp.x,  wp.y              }, { midX,      wp.y+1.f        }, a0,  a20, a20, a0 );
        dl->AddRectFilledMultiColor({ midX,  wp.y+ws.y - 1.f  }, { wp.x+ws.x, wp.y+ws.y       }, a20, a0,  a0,  a20);
        dl->AddRectFilledMultiColor({ wp.x,  wp.y+ws.y - 1.f  }, { midX,      wp.y+ws.y       }, a0,  a20, a20, a0 );

        // "LUCID" stacked letters (gradient alpha, centred in sidebar)
        static const char lts[] = "LUCID";
        const float lcx = wp.x + 55.f;
        const float lyS = wp.y + 22.f;
        for (int i = 0; i < 5; ++i)
        {
            char ch[2] = { lts[i], '\0' };
            ImVec2 csz = ImGui::CalcTextSize(ch);
            float  d   = fabsf((float)(i - 2));
            int    al  = (int)(255.f * (1.f - d * 0.35f));
            if (al < 70) al = 70;
            dl->AddText({ lcx - csz.x * 0.5f, lyS + (float)i * 18.f }, EvoAccent(al), ch);
        }

        // Tab buttons (5 x 50px, centred vertically in sidebar)
        const float tW    = 50.f, tH = 50.f, tSp = 6.f;
        const float totH  = kTabCount * tH + (kTabCount - 1) * tSp;
        const float tY0   = wp.y + (ws.y - totH) * 0.5f;
        const float tX0   = wp.x + (SIDE_W - tW) * 0.5f; // =30

        for (int i = 0; i < kTabCount; ++i)
        {
            float ty  = tY0 + (float)i * (tH + tSp);
            float tx  = tX0;
            bool  sel = (activeTab == i);

            if (sel)
            {
                dl->AddRectFilled({ tx, ty }, { tx + tW, ty + tH },
                    IM_COL32((int)(primaryColor[0]*255*0.15f),
                             (int)(primaryColor[1]*255*0.15f),
                             (int)(primaryColor[2]*255*0.15f), 180), 4.f);
                // left accent bar
                dl->AddRectFilled({ tx, ty + 10.f }, { tx + 2.5f, ty + tH - 10.f },
                    EvoAccent(200), 1.f);
            }

            ImVec2 tsz = ImGui::CalcTextSize(kTabLabels[i]);
            dl->AddText(
                { tx + (tW - tsz.x) * 0.5f, ty + (tH - ImGui::GetFontSize()) * 0.5f },
                sel ? EvoAccent(230) : kTextDim, kTabLabels[i]);

            ImGui::SetCursorScreenPos({ tx, ty });
            ImGui::PushID(i + 50000);
            if (ImGui::InvisibleButton("##tab", { tW, tH })) activeTab = i;
            ImGui::PopID();
        }

        // Outer border on foreground draw list (always on top)
        ImGui::GetForegroundDrawList()->AddRect(
            wp, { wp.x + ws.x, wp.y + ws.y },
            IM_COL32(21, 23, 26, (int)(200 * menuAlpha)), 12.f, 0, 1.f);

        // Two-column content
        ImGui::PushStyleColor(ImGuiCol_ChildBg,  { 0.f, 0.f, 0.f, 0.f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 6.f, 6.f });
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { 6.f, 4.f });

        // Left column
        ImGui::SetCursorPos({ COL_X, COL_Y });
        if (ImGui::BeginChild("##lc19", { COL_W, COL_H }, false))
        {
            ImGuiErrorRecoveryState rs;
            ImGui::ErrorRecoveryStoreState(&rs);
            __try {
                switch (activeTab)
                {
                case 0: Left_Aim(); break;
                case 1: Left_Vis(); break;
                case 2: Left_Skn(); break;
                case 3: Left_Wld(); break;
                case 4: Left_Cfg(); break;
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
            ImGui::ErrorRecoveryTryToRecoverState(&rs);
        }
        ImGui::EndChild();

        // Right column
        ImGui::SetCursorPos({ COL_X + COL_W + 10.f, COL_Y });
        if (ImGui::BeginChild("##rc19", { COL_W, COL_H }, false))
        {
            ImGuiErrorRecoveryState rs;
            ImGui::ErrorRecoveryStoreState(&rs);
            __try {
                switch (activeTab)
                {
                case 0: Right_Aim(); break;
                case 1: Right_Vis(); break;
                case 2: Right_Skn(); break;
                case 3: Right_Wld(); break;
                case 4: Right_Cfg(); break;
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
            ImGui::ErrorRecoveryTryToRecoverState(&rs);
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(2);   // WindowPadding + ItemSpacing
        ImGui::PopStyleColor();  // ChildBg

        ImGui::End();
        ImGui::PopStyleColor();  // WindowBg
        ImGui::PopStyleVar(3);   // WindowPadding, WindowBorderSize, Alpha
    }

} // namespace Menu
