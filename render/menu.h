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
#include "../core/stealth.h"   // Stealth::g_pUntrustedFlag for VAC watermark
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
#include "../features/skinchanger_test.h"
#include "../features/inventory_changer.h"
#include "../features/model_changer.h"
#include "../features/triggerbot.h"
#include "../features/backtrack.h"
#include "../features/anti_aim.h"
#include "../features/fake_lag.h"
#include "../features/sound_esp.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <wincodec.h>
// WinHTTP is loaded at runtime (see SteamAvatarThread) — NOT statically linked.
// Windowscodecs.lib likewise avoided; WIC is accessed via COM + local GUIDs.
// Both DLLs are non-boot and may not be mapped at the same VA in cs2.exe as in
// the injector process, so static IAT entries would point to garbage.

namespace Menu
{
    // State
    inline int   activeTab    = 0;
    inline bool  advancedMode = false;  // false = simple, true = advanced
    inline float menuAlpha    = 0.f;
    inline ImFont* fonts[3]   = { nullptr, nullptr, nullptr };
    inline ImFont* espFont    = nullptr;

    // ---- Card grid navigation state (per-tab feature page index, -1 = grid) ----
    inline int   pageStack[5]   = { -1, -1, -1, -1, -1 };
    inline int   prevPage[5]    = { -1, -1, -1, -1, -1 };
    inline float pageAnim       = 1.f;     // 0..1 transition progress
    inline int   pageAnimTab    = 0;       // which tab is animating
    inline int   pageDir        = 1;       // +1 = entering page, -1 = backing out

    // simple animation map: address -> 0..1 progress
    inline std::unordered_map<const void*, float> g_anim;
    inline float& Anim(const void* key) {
        auto it = g_anim.find(key);
        if (it == g_anim.end()) it = g_anim.emplace(key, 0.f).first;
        return it->second;
    }
    inline float AnimStep(const void* key, bool toward, float speed, float dt) {
        float& a = Anim(key);
        float target = toward ? 1.f : 0.f;
        float k = speed * dt; if (k > 1.f) k = 1.f;
        a += (target - a) * k;
        return a;
    }

    // HUD Steam avatar — downloaded once on startup via background thread
    inline ID3D11Device*             g_pDevice      = nullptr;  // assigned from hooks.h
    inline ID3D11ShaderResourceView* hudAvatarSRV    = nullptr;
    inline std::vector<uint8_t>      hudAvatarPixels;
    inline int                       hudAvatarW      = 0, hudAvatarH = 0;
    inline volatile bool             hudAvatarReady  = false;

    // Call from render thread: creates D3D11 SRV from pending pixel data
    inline void CreateAvatarSRV()
    {
        if (!hudAvatarReady || hudAvatarSRV || hudAvatarW == 0 || !g_pDevice) return;
        D3D11_TEXTURE2D_DESC td = {};
        td.Width = hudAvatarW; td.Height = hudAvatarH;
        td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = hudAvatarPixels.data();
        sd.SysMemPitch = hudAvatarW * 4;
        ID3D11Texture2D* tex = nullptr;
        if (SUCCEEDED(g_pDevice->CreateTexture2D(&td, &sd, &tex))) {
            g_pDevice->CreateShaderResourceView(tex, nullptr, &hudAvatarSRV);
            tex->Release();
        }
        hudAvatarPixels.clear(); hudAvatarPixels.shrink_to_fit();
        hudAvatarReady = false;
    }

    // Background thread: fetches Steam profile XML → downloads avatar → decodes via WIC
    inline DWORD WINAPI SteamAvatarThread(LPVOID)
    {
        // 1. Account ID from registry
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Valve\\Steam\\ActiveProcess", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
            return 0;
        DWORD accountID = 0, sz = sizeof(DWORD);
        RegQueryValueExA(hKey, "ActiveUser", nullptr, nullptr, (LPBYTE)&accountID, &sz);
        RegCloseKey(hKey);
        if (!accountID) return 0;

        uint64_t steamID64 = 76561197960265728ULL + (uint64_t)accountID;

        // 2. Load WinHTTP at runtime — avoids static IAT entries.
        //    The injector resolves imports from its own address space; non-boot DLLs
        //    like winhttp.dll may not be mapped in cs2.exe at the same VA, so any
        //    statically-linked import would call garbage addresses and crash.
        HMODULE hWH = LoadLibraryA("winhttp.dll");
        if (!hWH) return 0;

        typedef void* (WINAPI *tWHOpen)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
        typedef void* (WINAPI *tWHConnect)(void*, LPCWSTR, WORD, DWORD);
        typedef void* (WINAPI *tWHOpenReq)(void*, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR*, DWORD);
        typedef BOOL  (WINAPI *tWHSetOpt)(void*, DWORD, LPVOID, DWORD);
        typedef BOOL  (WINAPI *tWHSend)(void*, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR);
        typedef BOOL  (WINAPI *tWHRecv)(void*, LPVOID);
        typedef BOOL  (WINAPI *tWHRead)(void*, LPVOID, DWORD, LPDWORD);
        typedef BOOL  (WINAPI *tWHClose)(void*);

        auto whOpen  = (tWHOpen)  GetProcAddress(hWH, "WinHttpOpen");
        auto whConn  = (tWHConnect)GetProcAddress(hWH, "WinHttpConnect");
        auto whReq   = (tWHOpenReq)GetProcAddress(hWH, "WinHttpOpenRequest");
        auto whOpt   = (tWHSetOpt) GetProcAddress(hWH, "WinHttpSetOption");
        auto whSend  = (tWHSend)   GetProcAddress(hWH, "WinHttpSendRequest");
        auto whRecv  = (tWHRecv)   GetProcAddress(hWH, "WinHttpReceiveResponse");
        auto whRead  = (tWHRead)   GetProcAddress(hWH, "WinHttpReadData");
        auto whClose = (tWHClose)  GetProcAddress(hWH, "WinHttpCloseHandle");
        if (!whOpen || !whConn || !whReq || !whOpt || !whSend || !whRecv || !whRead || !whClose)
            return 0;

        // WinHTTP constants (not including winhttp.h to avoid dllimport declarations)
        constexpr DWORD kAccess   = 0;           // WINHTTP_ACCESS_TYPE_DEFAULT_PROXY
        constexpr WORD  kPortSSL  = 443;         // INTERNET_DEFAULT_HTTPS_PORT
        constexpr DWORD kSecure   = 0x00800000;  // WINHTTP_FLAG_SECURE
        constexpr DWORD kOptSec   = 31;          // WINHTTP_OPTION_SECURITY_FLAGS
        constexpr DWORD kSecFlgs  = 0x00000100 | 0x00002000 | 0x00001000 | 0x00000200;
        // ^ IGNORE_UNKNOWN_CA | IGNORE_CERT_DATE_INVALID | IGNORE_CERT_CN_INVALID | IGNORE_CERT_WRONG_USAGE

        // 3. FetchURL — all WinHTTP calls go through captured function pointers (no IAT)
        auto FetchURL = [&](const wchar_t* host, const wchar_t* path, std::string& out) -> bool {
            void* hs = whOpen(L"LucidHUD/1.0", kAccess, nullptr, nullptr, 0);
            if (!hs) return false;
            void* hc = whConn(hs, host, kPortSSL, 0);
            if (!hc) { whClose(hs); return false; }
            void* hr = whReq(hc, L"GET", path, nullptr, nullptr, nullptr, kSecure);
            if (!hr) { whClose(hc); whClose(hs); return false; }
            DWORD sf = kSecFlgs;
            whOpt(hr, kOptSec, &sf, sizeof(sf));
            if (!whSend(hr, nullptr, 0, nullptr, 0, 0, 0) || !whRecv(hr, nullptr)) {
                whClose(hr); whClose(hc); whClose(hs); return false;
            }
            char buf[4096]; DWORD rd = 0;
            while (whRead(hr, buf, sizeof(buf)-1, &rd) && rd > 0) { buf[rd]=0; out += buf; }
            whClose(hr); whClose(hc); whClose(hs);
            return true;
        };

        wchar_t xmlPath[128];
        swprintf_s(xmlPath, L"/profiles/%llu/?xml=1", (unsigned long long)steamID64);
        std::string xmlData;
        if (!FetchURL(L"steamcommunity.com", xmlPath, xmlData)) return 0;

        // 3. Parse <avatarFull> URL
        const char* tag = "<avatarFull>";
        auto p = xmlData.find(tag);
        if (p == std::string::npos) return 0;
        p += strlen(tag);
        auto e = xmlData.find("</avatarFull>", p);
        if (e == std::string::npos) return 0;
        std::string avatarUrl = xmlData.substr(p, e - p);
        // Strip CDATA
        auto cd = avatarUrl.find("<![CDATA[");
        if (cd != std::string::npos) avatarUrl = avatarUrl.substr(cd + 9, avatarUrl.size() - cd - 12);

        // 4. Parse host + path from avatar URL
        if (avatarUrl.rfind("https://", 0) != 0) return 0;
        std::string tmp = avatarUrl.substr(8);
        auto sl = tmp.find('/');
        if (sl == std::string::npos) return 0;
        std::wstring whost(tmp.begin(), tmp.begin() + sl);
        std::wstring wpath2(tmp.begin() + sl, tmp.end());

        // 5. Download avatar image to %TEMP%\lucid_av.jpg
        char tmpDir[MAX_PATH] = {};
        GetTempPathA(MAX_PATH, tmpDir);
        char tmpFile[MAX_PATH];
        snprintf(tmpFile, sizeof(tmpFile), "%slucid_av.jpg", tmpDir);

        std::string imgData;
        if (!FetchURL(whost.c_str(), wpath2.c_str(), imgData)) return 0;
        if (imgData.size() < 128) return 0;  // too small to be a valid image

        { FILE* f = fopen(tmpFile, "wb"); if (!f) return 0;
          fwrite(imgData.data(), 1, imgData.size(), f); fclose(f); }

        // 6. Decode JPEG via WIC to RGBA
        // GUIDs defined locally — avoids Windowscodecs.lib in the import table
        static const GUID kCLSID_WICFactory = { 0xcacaf262, 0x9370, 0x4615, { 0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a } };
        static const GUID kFmt32bppRGBA     = { 0xf5c7ad2d, 0x6a8d, 0x43dd, { 0xa7, 0xa8, 0xa2, 0x99, 0x35, 0x26, 0x1a, 0xe9 } };
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        IWICImagingFactory* factory = nullptr;
        if (FAILED(CoCreateInstance(kCLSID_WICFactory, nullptr, CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory), (void**)&factory)) || !factory) { CoUninitialize(); return 0; }

        std::wstring wTmp(tmpFile, tmpFile + strlen(tmpFile));
        IWICBitmapDecoder* decoder = nullptr;
        factory->CreateDecoderFromFilename(wTmp.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
        if (!decoder) { factory->Release(); CoUninitialize(); return 0; }

        IWICBitmapFrameDecode* frame = nullptr;
        if (FAILED(decoder->GetFrame(0, &frame)) || !frame)
            { decoder->Release(); factory->Release(); CoUninitialize(); return 0; }

        IWICFormatConverter* conv = nullptr;
        if (FAILED(factory->CreateFormatConverter(&conv)) || !conv)
            { frame->Release(); decoder->Release(); factory->Release(); CoUninitialize(); return 0; }

        if (FAILED(conv->Initialize(frame, kFmt32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
            { conv->Release(); frame->Release(); decoder->Release(); factory->Release(); CoUninitialize(); return 0; }

        UINT w = 0, h = 0;
        conv->GetSize(&w, &h);
        if (w == 0 || h == 0)
            { conv->Release(); frame->Release(); decoder->Release(); factory->Release(); CoUninitialize(); return 0; }

        std::vector<uint8_t> pixels(w * h * 4);
        conv->CopyPixels(nullptr, w * 4, (UINT)pixels.size(), pixels.data());

        conv->Release(); frame->Release(); decoder->Release(); factory->Release();
        CoUninitialize();

        // 7. Signal main thread
        hudAvatarPixels = std::move(pixels);
        hudAvatarW = (int)w; hudAvatarH = (int)h;
        hudAvatarReady = true;

        return 0;
    }

    inline void InitSteamAvatar() {
        HANDLE t = CreateThread(nullptr, 0, SteamAvatarThread, nullptr, 0, nullptr);
        if (t) CloseHandle(t);
    }

    inline float primaryColor[4]   = { 142/255.f, 132/255.f, 255/255.f, 1.0f };
    inline float secondaryColor[4] = { 0.09f, 0.09f, 0.09f, 0.70f };
    inline bool  themeApplied      = false;

    enum MenuStyle : int { STYLE_GLASS = 0, STYLE_CYBER = 1, STYLE_MINIMAL = 2 };
    inline MenuStyle activeStyle      = STYLE_GLASS;
    inline int       lastAppliedStyle = -1;
    inline int       hudStyle         = 0;   // 0=Pill  1=Clean  2=Ghost

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

    // Strip ##id suffix from a label string before DrawList text rendering
    inline const char* StripLabel(const char* s, char* buf, int n)
    {
        const char* h = strstr(s, "##");
        if (!h) return s;
        int len = (int)(h - s); if (len >= n) len = n - 1;
        memcpy(buf, s, len); buf[len] = '\0'; return buf;
    }

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
        const float h  = 24.f;
        ImVec2 pos     = ImGui::GetCursorScreenPos();

        const float pillW = 36.f, pillH = 18.f;
        ImVec2 pillTL = { pos.x + w - pillW, pos.y + (h - pillH) * 0.5f };
        ImVec2 pillBR = { pos.x + w,          pos.y + (h + pillH) * 0.5f };

        ImU32 pillBg = *v ? kCheckOn : kElemBg;
        float cx     = pillTL.x + (*v ? 26.f : 10.f);
        float cy     = (pillTL.y + pillBR.y) * 0.5f;
        ImU32 cirC   = *v ? EvoAccent(220) : kCircleOff;

        float ty = pos.y + (h - ImGui::GetFontSize()) * 0.5f;
        char _lbChk[64]; const char* lblChk = StripLabel(label, _lbChk, sizeof(_lbChk));
        dl->AddText({ pos.x + 2.f, ty }, *v ? kTextBrt : kTextDim, lblChk);

        dl->AddRectFilled(pillTL, pillBR, pillBg, 100.f);
        dl->AddCircleFilled({ cx, cy }, 5.5f, cirC);

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
        const float h  = 24.f;
        ImVec2 pos     = ImGui::GetCursorScreenPos();

        const float sW = 50.f, sH = 16.f;
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
        char _lbF[64]; const char* lblF = StripLabel(label, _lbF, sizeof(_lbF));
        dl->AddText({ pos.x + 2.f, ty }, kTextMid, lblF);
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
        ImGui::Dummy({ w, 0.f });
        ImGui::PopID();
        return changed;
    }

    inline bool EvoSliderInt(const char* label, int* v, int mn, int mx)
    {
        ImGui::PushID(label);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float w  = ImGui::GetContentRegionAvail().x;
        const float h  = 24.f;
        ImVec2 pos     = ImGui::GetCursorScreenPos();

        const float sW = 50.f, sH = 16.f;
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
        char _lbI[64]; const char* lblI = StripLabel(label, _lbI, sizeof(_lbI));
        dl->AddText({ pos.x + 2.f, ty }, kTextMid, lblI);
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
        ImGui::Dummy({ w, 0.f });
        ImGui::PopID();
        return changed;
    }

    // Section/category label — accent bar + title + subtle rule line
    inline void EvoLabel(const char* text)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos     = ImGui::GetCursorScreenPos();
        float  w       = ImGui::GetContentRegionAvail().x;
        float  fh      = ImGui::GetFontSize();
        ImVec2 tsz     = ImGui::CalcTextSize(text);
        // Small accent bar on the left
        dl->AddRectFilled({ pos.x, pos.y + fh*0.2f }, { pos.x + 2.5f, pos.y + fh*0.85f }, EvoAccent(180), 2.f);
        // Section title text
        dl->AddText({ pos.x + 7.f, pos.y }, IM_COL32(185, 182, 210, 255), text);
        // Subtle horizontal rule to the right
        float lx = pos.x + 7.f + tsz.x + 8.f;
        float ly = pos.y + fh * 0.5f;
        dl->AddRectFilled({ lx, ly }, { pos.x + w, ly + 1.f }, IM_COL32(42, 40, 62, 100));
        ImGui::Dummy({ w, fh + 5.f });
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
        ImGui::PushID(label);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool hit = ImGui::Combo("##c", v, items, cnt);
        ImGui::PopStyleColor(5);
        ImGui::PopID();
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

    // Auto-height section — wraps content in a subtle "card":
    //   * 1px hairline outline so each section visibly separates from the
    //     next without heavy backgrounds,
    //   * a thin accent stripe down the left edge for a hierarchical feel,
    //   * a very low-alpha fill so labels still pop against the menu bg.
    inline void SynthBeginSection(const char* id)
    {
        // Section sub-cards: borderless, single-tone fill so AIMBOT/BUNNY
        // HOP etc. read as soft groupings without a hard top hairline.
        // Accent stripe on the left provides hierarchy.
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(16, 14, 26, 130));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 11.f, 7.f });
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 9.f);
        ImGui::BeginChild(id,
            { ImGui::GetContentRegionAvail().x, 0.f },
            ImGuiChildFlags_AutoResizeY,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // (no accent stripe — keep sections clean)
    }
    inline void SynthEndSection()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(1);
        ImGui::Dummy({ 0.f, 6.f });
    }


    // ============================================================
    //  CONFIG SYSTEM
    // ============================================================

    struct SavedConfig
    {
        uint32_t magic    = 0x4C554349;
        uint32_t version  = 16; // bumped: Bhop::Config gained `strafeMode`
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
        int   hudStyle  = 0;
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
        cfg.hudStyle     = Menu::hudStyle;
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
        // hdr[0] = magic, hdr[1] = version, hdr[2] = dataSize at write time.
        // CRITICAL: dataSize MUST equal sizeof(SavedConfig) at load time.
        // SavedConfig is read with raw fread of the full struct — every
        // member after Aimbot::Config (ESP/chams/colors/...) is positionally
        // bound to sizeof(Aimbot::Config). A struct-size mismatch silently
        // shifts every field after the change point, producing exactly the
        // "weapon ? / wrong toggles / black menu / silent toggle off"
        // corruption seen in builds that briefly removed cfg fields without
        // a version bump (commit 6743f1b). Reject + fall back to defaults
        // is far cleaner than reading 8 bytes of garbage into bools/floats.
        if (hdr[0] != 0x4C554349 ||
            hdr[1] != 16 ||
            hdr[2] != (uint32_t)sizeof(SavedConfig))
        {
            fclose(f);
            return false;
        }
        fseek(f, 0, SEEK_SET);
        SavedConfig cfg{};
        fseek(f, 0, SEEK_END);
        long fsz = ftell(f);
        fseek(f, 0, SEEK_SET);
        // dataSize already validated above, so fsz must equal sizeof(cfg).
        size_t rsz = ((size_t)fsz < sizeof(cfg)) ? (size_t)fsz : sizeof(cfg);
        if (fread(&cfg, 1, rsz, f) < 12) { fclose(f); return false; }
        fclose(f);
        if (cfg.menuStyle < 0 || cfg.menuStyle > 2) cfg.menuStyle = 0;
        if (cfg.aimbot.fov < 0.1f)        cfg.aimbot.fov = 2.2f;
        if (cfg.aimbot.smoothing < 1.f)   cfg.aimbot.smoothing = 45.f;
        Aimbot::cfg          = cfg.aimbot;
        // Hard-pin: silent-aim path is disabled build-wide. Never honor
        // a previously-saved silentAim=true value from disk.
        Aimbot::cfg.silentAim = false;
        ESP::cfg             = cfg.esp;
        BulletTracer::cfg    = cfg.tracer;
        WorldEffects::cfg    = cfg.worldEffects;
        DamageIndicator::cfg = cfg.damage;
        WireframeHands::cfg  = cfg.wireframe;
        Chams::cfg           = cfg.chams;
        Bhop::cfg            = cfg.bhop;
        SkinChanger::cfg     = cfg.skinchanger;
        Menu::activeStyle    = (Menu::MenuStyle)cfg.menuStyle;
        Menu::hudStyle       = (cfg.hudStyle >= 0 && cfg.hudStyle <= 2) ? cfg.hudStyle : 0;
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
        Aimbot::cfg.targetBone    = 23;
        Aimbot::cfg.aimKey        = 0;
        Aimbot::cfg.teamCheck     = true;
        Aimbot::cfg.visCheck      = false;  // 14153: trace plumbing unreliable
        Aimbot::cfg.showFovCircle = false;
        Aimbot::cfg.jumpShot      = false;
        Aimbot::cfg.velPredict    = false;
        Aimbot::cfg.multiBone     = false;
        Aimbot::cfg.headPriority  = false;
        Aimbot::cfg.smokeCheck    = false;  // 14153: smoke detection unreliable
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
        Aimbot::cfg.targetBone    =  7;
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
        Aimbot::cfg.silentAim     = false;  // <-- silent-aim path disabled across the build
        Aimbot::cfg.fov           = 2.3f;
        // Semi-rage baseline: ~10 points snappier than old preset
        // while keeping movement human-looking.
        Aimbot::cfg.smoothing     = 33.f;
        Aimbot::cfg.humanization  = 0.40f;
        Aimbot::cfg.targetBone    =  7;
        Aimbot::cfg.aimKey        = 0;
        Aimbot::cfg.teamCheck     = true;
        Aimbot::cfg.visCheck      = false;  // 14153: trace plumbing unreliable
        Aimbot::cfg.showFovCircle = true;
        Aimbot::cfg.velPredict    = true;
        Aimbot::cfg.multiBone     = true;
        Aimbot::cfg.headPriority  = true;
        Aimbot::cfg.smokeCheck    = false;  // 14153: smoke detection unreliable
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
        // Engage rage-mode toggles. ApplyTransition (called next aimbot
        // tick) snapshots the current legit knobs, then clobbers
        // silentAim/aimKey/smoothing/visCheck for the session.
        Aimbot::Rage::cfg.enabled         = true;
        Aimbot::Rage::cfg.alwaysOn        = true;
        Aimbot::Rage::cfg.silentForce     = true;
        Aimbot::Rage::cfg.instant         = true;
        Aimbot::Rage::cfg.forceWallbang   = true;
        Aimbot::Rage::cfg.lowestHpFirst   = true;
        Aimbot::Rage::cfg.forceBaim       = false;
        Aimbot::Rage::cfg.prioritizeArmored = false;
        Aimbot::cfg.silentAim         = false;  // silent-aim path disabled build-wide
        Aimbot::cfg.fov               = 10.f;
        Aimbot::cfg.smoothing         = 8.f;
        Aimbot::cfg.humanization      = 0.15f;
        Aimbot::cfg.targetBone        =  7;
        Aimbot::cfg.aimKey            = 2;   // Rage = always-on, no key required
        Aimbot::cfg.teamCheck         = true;
        Aimbot::cfg.visCheck          = false;  // 14153: trace plumbing unreliable
        Aimbot::cfg.showFovCircle     = false;
        Aimbot::cfg.jumpShot          = true;
        Aimbot::cfg.jumpApexOnly      = true;
        Aimbot::cfg.jumpApexThreshold = 40.f;
        Aimbot::cfg.velPredict        = true;
        Aimbot::cfg.multiBone         = true;
        Aimbot::cfg.headPriority      = true;
        Aimbot::cfg.smokeCheck        = false;  // 14153: smoke detection unreliable
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
        if (advancedMode)
        {
            SynthSep();
            EvoCheckbox("Head Priority", &Aimbot::cfg.headPriority);
            SynthSep();
            EvoCheckbox("Smoke Check",   &Aimbot::cfg.smokeCheck);
            SynthSep();
            EvoCheckbox("Vis Check",     &Aimbot::cfg.visCheck);
        }
        // "Silent Aim" toggle removed — the WriteSubtick path proved
        // unreliable in the field (would silently no-op for some users).
        // Aimbot::cfg.silentAim is hard-pinned to false at startup.
        SynthEndSection();

        SynthBeginSection("##aim_s2");
        EvoLabel("AIM FEEL");
        EvoSliderFloat("FOV",          &Aimbot::cfg.fov,          0.5f, 30.f,  "%.1f");
        SynthSep();
        EvoSliderFloat("Smoothing",    &Aimbot::cfg.smoothing,    1.f,  100.f, "%.0f");
        if (advancedMode)
        {
            SynthSep();
            EvoSliderFloat("Humanization", &Aimbot::cfg.humanization, 0.f,  1.f,   "%.2f");
        }
        SynthSep();
        EvoCheckbox("No Recoil",       &Aimbot::cfg.noRecoil);
        SynthEndSection();
    }

    inline void Right_Aim()
    {
        if (advancedMode)
        {
            SynthBeginSection("##aim_r1");
            EvoLabel("TARGETING");
            {
                const char* bones[]   = { "Head","Neck","Chest","Pelvis" };
                const int   boneIds[] = { 7, 6, 23, 1 };
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
        }

        SynthBeginSection("##aim_r3");
        EvoLabel("BUNNY HOP");
        EvoCheckbox("Enable##bhop",      &Bhop::cfg.enabled);
        if (Bhop::cfg.enabled)
        {
            if (advancedMode)
            {
                SynthSep();
                EvoSliderFloat("Max Speed##bms", &Bhop::cfg.maxSpeed, 200.f, 500.f, "%.0f");
                SynthSep();
                EvoSliderFloat("Min Speed##bns", &Bhop::cfg.minSpeed,  10.f, 120.f, "%.0f");
            }
            SynthSep();
            EvoCheckbox("Auto Strafe##bs", &Bhop::cfg.autoStrafe);
        }
        SynthSep();
        EvoCheckbox("Velocity Display##bvd", &Bhop::cfg.showVelocity);
        SynthEndSection();

        SynthBeginSection("##aim_triggerbot");
        EvoLabel("TRIGGERBOT");
        EvoCheckbox("Enable##trig",      &Triggerbot::cfg.enabled);
        if (Triggerbot::cfg.enabled && advancedMode)
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

        if (advancedMode)
        {
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
                if (AntiAim::cfg.yawMode == 1) // Spin
                {
                    SynthSep();
                    EvoSliderFloat("Speed##aas", &AntiAim::cfg.spinSpeed, 1.f, 45.f, "%.0f");
                }
                if (AntiAim::cfg.yawMode == 3) // Desync
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
            if (advancedMode && ESP::cfg.box)
            {
                SynthSep();
                const char* bst[] = { "Normal","Corners" };
                EvoCombo("Style##bs", &ESP::cfg.boxStyle, bst, 2);
            }
            if (advancedMode)
            {
                SynthSep();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::ColorEdit4("Box Color##bc", ESP::cfg.boxColor,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }
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
            if (advancedMode && ESP::cfg.weapon)
            {
                SynthSep();
                EvoCheckbox("Use Weapon Icons##wi", &ESP::cfg.weaponIcon);
            }
            SynthSep();
            EvoCheckbox("Team Check##etm", &ESP::cfg.teamCheck);
            SynthSep();
            EvoCheckbox("Bomb Timer", &ESP::cfg.bombTimer);
            if (advancedMode && ESP::cfg.bombTimer)
            {
                SynthSep();
                const char* bts[] = { "Classic", "Vivid", "Compact" };
                EvoCombo("Bomb Style##bts", &ESP::cfg.bombTimerStyle, bts, 3);
            }
            if (advancedMode)
            {
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
        if (advancedMode && DamageIndicator::cfg.enabled)
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
        EvoCheckbox("Spectator List", &ESP::cfg.spectators);
        if (advancedMode && ESP::cfg.spectators)
        {
            SynthSep();
            const char* sps[] = { "Classic", "Stealth", "Minimal" };
            EvoCombo("List Style##sls", &ESP::cfg.spectatorStyle, sps, 3);
        }
        SynthSep();
        EvoCheckbox("Rank Revealer",  &RankRevealer::cfg.enabled);
        SynthSep();
        EvoCheckbox("Model Changer", &ModelChanger::cfg.enabled);
        if (ModelChanger::cfg.enabled)
        {
            SynthSep();
            // Build display-name array each frame (cheap, < 20 entries).
            const char* names[ModelChanger::kAgentCount];
            for (int i = 0; i < ModelChanger::kAgentCount; ++i)
                names[i] = ModelChanger::kAgents[i].displayName;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::Combo("##mcAgent", &ModelChanger::cfg.selectedAgent,
                         names, ModelChanger::kAgentCount);
        }
        SynthEndSection();

        if (advancedMode)
        {
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
        }

        SynthBeginSection("##vis_r_sound");
        EvoLabel("SOUND ESP");
        EvoCheckbox("Enable##sesp",       &SoundESP::cfg.enabled);
        if (SoundESP::cfg.enabled)
        {
            SynthSep();
            EvoCheckbox("Footstep Marks##sfm", &SoundESP::cfg.showFootsteps);
            if (advancedMode)
            {
                SynthSep();
                EvoSliderFloat("Max Dist##smd", &SoundESP::cfg.maxDistance, 500.f, 4000.f, "%.0f");
                SynthSep();
                EvoSliderFloat("Ring Size##srs", &SoundESP::cfg.ringRadius, 30.f, 150.f, "%.0f");
                SynthSep();
                EvoSliderFloat("Arrow Size##sas", &SoundESP::cfg.indicatorSize, 15.f, 80.f, "%.0f");
            }
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
            SynthSep();
            if (EvoButton("Inject Locker Items##ili"))
            {
                InventoryChanger::ResetAutoInject();
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
        const char* nm[] = { "Off","Night","Midnight","Sunset","Blood Moon","Aurora","Cyberpunk","Vaporwave","Hellfire" };
        EvoCombo("Mode##nm", &WorldEffects::cfg.nightMode, nm, IM_ARRAYSIZE(nm));
        SynthSep();
        EvoLabel("ASUS MODE");
        const char* am[] = { "Off","Lime","Hot Pink","Cyan","Red","Yellow" };
        EvoCombo("Color##am", &WorldEffects::cfg.asusMode, am, IM_ARRAYSIZE(am));
        SynthEndSection();

        SynthBeginSection("##wld_visibility");
        EvoLabel("VISIBILITY");
        EvoCheckbox("Fullbright##fbm",         &WorldEffects::cfg.fullbright);
        SynthSep();
        EvoCheckbox("Anti-Fog##af",            &WorldEffects::cfg.antiFog);
        SynthSep();
        EvoCheckbox("No Shadows##nsh",         &WorldEffects::cfg.noShadows);
        SynthSep();
        EvoCheckbox("No Color Correction##ncc",&WorldEffects::cfg.noColorCorrection);
        SynthEndSection();

        if (advancedMode)
        {
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
        }

        SynthBeginSection("##wld_r4");
        EvoLabel("MISC");
        EvoCheckbox("Third Person##tp",   &WorldEffects::cfg.thirdPerson);
        if (advancedMode && WorldEffects::cfg.thirdPerson)
        {
            SynthSep();
            EvoSliderFloat("3P Distance##tpd", &WorldEffects::cfg.thirdPersonDist, 50.f, 600.f, "%.0f");
        }
        if (advancedMode)
        {
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
        }
        SynthSep();
        EvoCheckbox("Auto-Accept##aa", &AutoAccept::cfg.enabled);
        if (advancedMode && AutoAccept::cfg.enabled) { SynthSep(); EvoSliderFloat("Delay##aad", &AutoAccept::cfg.delay, 0.1f, 3.f, "%.1f"); }
        SynthEndSection();
    }

    inline void Left_Cfg()
    {
        SynthBeginSection("##cfg_mode");
        EvoLabel("UI MODE");
        EvoCheckbox("Advanced Mode", &advancedMode);
        SynthEndSection();

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

        SynthBeginSection("##cfg_s3");
        EvoLabel("HUD STYLE");
        {
            const char* huds[] = { "Pill", "Clean", "Ghost" };
            EvoCombo("Style##huds", &Menu::hudStyle, huds, 3);
        }
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
    //  LUCIDE-STYLE TAB ICONS  (pure DrawList, no textures)
    // ============================================================
    inline void DrawTabIcon(ImDrawList* dl, int tab, ImVec2 c, ImU32 col)
    {
        const float t = 1.4f;
        switch (tab)
        {
        case 0: // Crosshair — Aim
            dl->AddCircle(c, 7.f, col, 32, t);
            dl->AddCircleFilled(c, 1.8f, col);
            dl->AddLine({c.x,        c.y - 11.f}, {c.x,        c.y -  9.f}, col, t);
            dl->AddLine({c.x,        c.y +  9.f}, {c.x,        c.y + 11.f}, col, t);
            dl->AddLine({c.x - 11.f, c.y       }, {c.x -  9.f, c.y       }, col, t);
            dl->AddLine({c.x +  9.f, c.y       }, {c.x + 11.f, c.y       }, col, t);
            break;
        case 1: // Eye — Vis
        {
            const float ew = 11.f, ctrl = 7.5f;
            dl->AddBezierCubic(
                {c.x - ew, c.y}, {c.x - ew * 0.5f, c.y - ctrl},
                {c.x + ew * 0.5f, c.y - ctrl}, {c.x + ew, c.y}, col, t);
            dl->AddBezierCubic(
                {c.x + ew, c.y}, {c.x + ew * 0.5f, c.y + ctrl},
                {c.x - ew * 0.5f, c.y + ctrl}, {c.x - ew, c.y}, col, t);
            dl->AddCircle(c, 3.2f, col, 12, t);
            break;
        }
        case 2: // Layers / chevrons — Skin
        {
            const float offsets[3] = { -5.f, 0.f, 5.f };
            for (int j = 0; j < 3; ++j)
            {
                float yo = offsets[j];
                dl->AddLine({c.x - 10.f, c.y + yo}, {c.x,       c.y + yo - 4.f}, col, t);
                dl->AddLine({c.x,        c.y + yo - 4.f}, {c.x + 10.f, c.y + yo}, col, t);
            }
            break;
        }
        case 3: // Globe — World
        {
            dl->AddCircle(c, 9.f, col, 32, t);
            dl->AddLine({c.x, c.y - 9.f}, {c.x, c.y + 9.f}, col, t);
            const float ex = 9.f, ctrl2 = 5.f;
            dl->AddBezierCubic(
                {c.x - ex, c.y}, {c.x - ex, c.y - ctrl2},
                {c.x + ex, c.y - ctrl2}, {c.x + ex, c.y}, col, t);
            dl->AddBezierCubic(
                {c.x + ex, c.y}, {c.x + ex, c.y + ctrl2},
                {c.x - ex, c.y + ctrl2}, {c.x - ex, c.y}, col, t);
            break;
        }
        case 4: // Sliders — Config
        {
            const float lw = 18.f;
            const float ly[3] = {c.y - 5.5f, c.y, c.y + 5.5f};
            const float hx[3] = {c.x - 3.f, c.x + 4.f, c.x - 1.f};
            const float hr = 2.3f;
            for (int j = 0; j < 3; ++j)
            {
                float x0 = c.x - lw * 0.5f, x1 = c.x + lw * 0.5f;
                dl->AddLine({x0,         ly[j]}, {hx[j] - hr, ly[j]}, col, t);
                dl->AddLine({hx[j] + hr, ly[j]}, {x1,         ly[j]}, col, t);
                dl->AddCircle({hx[j], ly[j]}, hr, col, 8, t);
            }
            break;
        }
        }
    }

    // ============================================================
    //  TOP-RIGHT HUD OVERLAY  (always visible: LUCID | name | time | FPS)
    //  Three styles:
    //    0 Pill  — premium dark pill with accent bar, hairline dividers,
    //              soft drop shadow, gradient backdrop, avatar accent ring
    //    1 Clean — accent-forward outline, no dividers
    //    2 Ghost — ultra-minimal, barely visible
    // ============================================================
    inline void RenderHUD()
    {
        // Upload pending avatar on the render thread
        if (hudAvatarReady) CreateAvatarSRV();

        ImGuiIO&    io = ImGui::GetIO();
        ImDrawList* fl = ImGui::GetForegroundDrawList();

        // ---- data --------------------------------------------------------
        char fpsBuf[8], timeBuf[12], name[64] = "User";
        snprintf(fpsBuf,  sizeof(fpsBuf),  "%d", (int)(io.Framerate + 0.5f));
        SYSTEMTIME st; GetLocalTime(&st);
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
        GetEnvironmentVariableA("USERNAME", name, sizeof(name));
        name[15] = '\0';

        // ---- geometry ----------------------------------------------------
        const float fh   = ImGui::GetFontSize();
        const float bh   = fh + 18.f;                    // bar height
        const float avD  = bh - 10.f;                    // avatar diameter
        const float avR  = avD * 0.5f;
        const float padX = 12.f;                          // horizontal padding inside pill
        const float gap  = 9.f;                           // gap between text segments
        // Hairline divider footprint = gap + 1px line + gap. The math used to
        // fall short by 9px per divider which clipped the FPS value off the
        // right edge — keep this in sync with HairDivider() below.
        const float divW = gap + 1.f + gap;
        const float radius = bh * 0.5f;                   // fully rounded ends → capsule

        ImVec2 szL  = ImGui::CalcTextSize("LUCID");
        ImVec2 szN  = ImGui::CalcTextSize(name);
        ImVec2 szT  = ImGui::CalcTextSize(timeBuf);
        ImVec2 szFL = ImGui::CalcTextSize("FPS");
        ImVec2 szFV = ImGui::CalcTextSize(fpsBuf);

        const float avSpace  = hudAvatarSRV ? (avD + 8.f) : 0.f;
        const float fpsInner = 5.f; // spacing between "FPS" label and value

        // Left inset is style-dependent: pill/clean draw a 2px accent bar at
        // bx+4..bx+6 so content starts at bx+6+padX; ghost has no accent bar
        // and starts at bx+padX. Compute totalW per style.
        const float pillInset  = 6.f;     // accent bar width
        const float ghostInset = 0.f;
        const float leftInset  = (hudStyle == 2) ? ghostInset : pillInset;

        // Trailing margin so FPS value doesn't kiss the rounded right edge.
        const float trailPad = padX;
        // Note: ghost has 3 dividers (between every segment), pill/clean
        // currently render either 3 (pill) or 0 (clean). Compute per style.
        int dividers = 0;
        if (hudStyle == 0 || hudStyle == 2) dividers = 3;  // pill/ghost: hairlines
        // clean style: no dividers, but we add small inter-segment gaps
        const float cleanGap = (hudStyle == 1) ? (gap + 4.f) : 0.f;

        float totalW = leftInset + padX + avSpace
                     + szL.x  + (dividers ? divW : cleanGap)
                     + szN.x  + (dividers ? divW : cleanGap)
                     + szT.x  + (dividers ? divW : cleanGap)
                     + szFL.x + fpsInner + szFV.x
                     + trailPad;

        const float bx = io.DisplaySize.x - totalW - 14.f;
        const float by = 12.f;
        const float cy = by + bh * 0.5f;
        const float ty = by + (bh - fh) * 0.5f;

        // Hairline divider helper — shared across styles, advances cx
        auto HairDivider = [&](float& cx, ImU32 col, float topPad = 6.f) {
            cx += gap;
            fl->AddRectFilled({ cx, by + topPad }, { cx + 1.f, by + bh - topPad }, col);
            cx += 1.f + gap;
        };

        // ------------------------------------------------------------------
        //  STYLE 2 — GHOST  (truly minimal: low-alpha capsule, no chrome,
        //  no dividers, dot separators only)
        // ------------------------------------------------------------------
        if (hudStyle == 2)
        {
            // Very subtle backdrop — barely there, no border, no accent bar.
            fl->AddRectFilled({ bx, by }, { bx + totalW, by + bh }, IM_COL32(8, 8, 14, 110), radius);

            float cx = bx + padX;
            if (hudAvatarSRV) {
                fl->AddImageRounded((ImTextureID)(intptr_t)hudAvatarSRV,
                    { cx, cy - avR }, { cx + avD, cy + avR },
                    { 0.f, 0.f }, { 1.f, 1.f },
                    IM_COL32(255, 255, 255, 165), avR);
                cx += avD + 8.f;
            }

            // Dot separator instead of vertical hairline — softer, more "ghost"
            const ImU32 cDot = IM_COL32(150, 150, 165, 110);
            auto Dot = [&](float& x) {
                x += gap;
                fl->AddCircleFilled({ x + 1.f, cy }, 1.4f, cDot, 8);
                x += 2.f + gap;
            };

            fl->AddText({ cx, ty }, EvoAccent(195),                "LUCID");   cx += szL.x;
            Dot(cx);
            fl->AddText({ cx, ty }, IM_COL32(190, 190, 200, 180),  name);      cx += szN.x;
            Dot(cx);
            fl->AddText({ cx, ty }, IM_COL32(150, 150, 160, 155),  timeBuf);   cx += szT.x;
            Dot(cx);
            fl->AddText({ cx, ty }, IM_COL32(120, 120, 130, 145),  "FPS");     cx += szFL.x + fpsInner;
            fl->AddText({ cx, ty }, EvoAccent(180),                fpsBuf);
            return;
        }

        // ------------------------------------------------------------------
        //  STYLE 1 — CLEAN  (accent-outline, no dividers)
        // ------------------------------------------------------------------
        if (hudStyle == 1)
        {
            // soft outer glow
            for (int i = 4; i > 0; --i) {
                fl->AddRect({ bx - i, by - i }, { bx + totalW + i, by + bh + i },
                            EvoAccent(8 + (4 - i) * 4), radius + i, 0, 1.f);
            }
            fl->AddRectFilled({ bx, by }, { bx + totalW, by + bh }, IM_COL32(11, 9, 18, 215), radius);
            fl->AddRect      ({ bx, by }, { bx + totalW, by + bh }, EvoAccent(110), radius, 0, 1.2f);
            fl->AddRectFilled({ bx + 4.f, by + 6.f }, { bx + 6.f, by + bh - 6.f }, EvoAccent(235), 2.f);

            float cx = bx + 6.f + padX;
            if (hudAvatarSRV) {
                fl->AddImageRounded((ImTextureID)(intptr_t)hudAvatarSRV,
                    { cx, cy - avR }, { cx + avD, cy + avR },
                    { 0.f, 0.f }, { 1.f, 1.f },
                    IM_COL32(255, 255, 255, 255), avR);
                fl->AddCircle({ cx + avR, cy }, avR + 0.8f, EvoAccent(140), 36, 1.4f);
                cx += avD + 8.f;
            }
            fl->AddText({ cx, ty }, EvoAccent(255),               "LUCID");   cx += szL.x + gap + 4.f;
            fl->AddText({ cx, ty }, kTextBrt,                     name);      cx += szN.x + gap + 4.f;
            fl->AddText({ cx, ty }, EvoAccent(155),               timeBuf);   cx += szT.x + gap + 4.f;
            fl->AddText({ cx, ty }, IM_COL32(105,105,118,170),    "FPS");     cx += szFL.x + fpsInner;
            fl->AddText({ cx, ty }, EvoAccent(235),               fpsBuf);
            return;
        }

        // ------------------------------------------------------------------
        //  STYLE 0 — PILL  (default, premium)
        //  - 4-layer drop shadow
        //  - Two-tone gradient backdrop (top slightly lighter)
        //  - 1px hairline outline + 1px inner highlight
        //  - Accent bar on left edge
        //  - Avatar with double accent ring
        //  - Hairline vertical dividers between segments
        // ------------------------------------------------------------------
        // 1) Drop shadow (4 progressively softer layers, offset down 2px)
        for (int i = 5; i > 0; --i) {
            fl->AddRectFilled({ bx - i, by - i + 2 },
                              { bx + totalW + i, by + bh + i + 2 },
                              IM_COL32(0, 0, 0, 9 + (5 - i) * 6), radius + i);
        }

        // 2) Backdrop — ImGui supports per-corner colors via AddRectFilledMultiColor
        //    Top: slightly lifted; bottom: deep ink. Gives a glass-panel feel.
        const ImU32 cTop  = IM_COL32(20, 18, 30, 235);
        const ImU32 cBot  = IM_COL32(8,  7, 14,  240);
        // Rounded mask is faked: draw rounded base in cBot, then a top-half
        // gradient strip that doesn't quite reach the edges (radius makes
        // hard rectangle inside acceptable since the rounded base shows through).
        fl->AddRectFilled({ bx, by }, { bx + totalW, by + bh }, cBot, radius);
        fl->AddRectFilledMultiColor(
            { bx + 1.f, by + 1.f },
            { bx + totalW - 1.f, by + bh * 0.55f },
            cTop, cTop, cBot, cBot);

        // 3) Outline + inner highlight
        fl->AddRect({ bx, by }, { bx + totalW, by + bh }, IM_COL32(255, 255, 255, 18), radius, 0, 1.f);
        fl->AddRect({ bx + 1.f, by + 1.f }, { bx + totalW - 1.f, by + bh - 1.f },
                    EvoAccent(55), radius - 1.f, 0, 1.f);

        // 4) Accent bar on left edge — vertical capsule
        fl->AddRectFilled({ bx + 4.f, by + 5.f }, { bx + 6.f, by + bh - 5.f }, EvoAccent(235), 2.f);

        // 5) Content
        float cx = bx + 6.f + padX;
        if (hudAvatarSRV) {
            fl->AddImageRounded((ImTextureID)(intptr_t)hudAvatarSRV,
                { cx, cy - avR }, { cx + avD, cy + avR },
                { 0.f, 0.f }, { 1.f, 1.f },
                IM_COL32(255, 255, 255, 255), avR);
            // double ring: thin accent + outer faint
            fl->AddCircle({ cx + avR, cy }, avR + 0.5f, EvoAccent(160), 36, 1.2f);
            fl->AddCircle({ cx + avR, cy }, avR + 1.8f, EvoAccent(45),  36, 1.0f);
            cx += avD + 8.f;
        }

        // Clean separators — small soft dot between segments instead of
        // hairline rules (those were reading as defects in light maps).
        auto Sep = [&](float& cx) {
            fl->AddCircleFilled({ cx + divW * 0.5f, cy + 0.5f }, 1.3f,
                                IM_COL32(150, 145, 175, 150), 8);
            cx += divW;
        };

        fl->AddText({ cx, ty }, EvoAccent(245), "LUCID");   cx += szL.x;  Sep(cx);
        fl->AddText({ cx, ty }, kTextBrt,        name);      cx += szN.x;  Sep(cx);
        fl->AddText({ cx, ty }, kTextMid,        timeBuf);   cx += szT.x;  Sep(cx);
        fl->AddText({ cx, ty }, IM_COL32(108,108,122,200), "FPS"); cx += szFL.x + fpsInner;
        fl->AddText({ cx, ty }, EvoAccent(225),  fpsBuf);
    }

    // ============================================================
    //  TOP-LEFT VAC HEARTBEAT WATERMARK
    //  Mirrors the right-side pill style. Shows the live state of
    //  the client.dll untrusted-mode byte we hold down to defeat
    //  the 20-hour MM cooldown.
    //
    //  States (status dot / value text):
    //    GRAY    "----"  — sigscan not run yet (very first frames)
    //    AMBER   "MISS"  — sigscan ran but couldn't find the setter
    //                       (game updated, pattern broke — bug me)
    //    GREEN   "00"    — flag is being held at 0 (we're invisible)
    //    RED     "01"    — game just set it; we'll zero it next beat
    // ============================================================
    inline void RenderVacWatermark()
    {
        ImGuiIO&    io = ImGui::GetIO();
        ImDrawList* fl = ImGui::GetForegroundDrawList();

        // ---- read live state (single byte deref, totally safe) ---------
        const volatile uint8_t* pFlag = Stealth::g_pUntrustedFlag;
        const bool tried              = Stealth::g_untrustedFlagResolveTried;

        const char* valTxt;
        ImU32       dotCol;
        ImU32       valCol;
        if (!pFlag) {
            if (!tried) { valTxt = "----"; dotCol = IM_COL32(150,150,160,200); valCol = IM_COL32(150,150,160,220); }
            else        { valTxt = "MISS"; dotCol = IM_COL32(245,180, 60,235); valCol = IM_COL32(245,180, 60,235); }
        } else {
            uint8_t v = *pFlag;
            if (v == 0) { valTxt = "00"; dotCol = IM_COL32( 90,225,130,235); valCol = IM_COL32(180,235,200,235); }
            else        { valTxt = "01"; dotCol = IM_COL32(235, 80, 90,235); valCol = IM_COL32(235,150,160,235); }
        }

        // ---- geometry (mirrors RenderHUD pill) -------------------------
        const float fh     = ImGui::GetFontSize();
        const float bh     = fh + 18.f;
        const float padX   = 12.f;
        const float gap    = 9.f;
        const float divW   = gap + 1.f + gap;
        const float radius = bh * 0.5f;
        const float pillInset = 6.f;       // accent bar width

        ImVec2 szL = ImGui::CalcTextSize("VAC");
        ImVec2 szH = ImGui::CalcTextSize("Heartbeat");
        ImVec2 szV = ImGui::CalcTextSize(valTxt);
        const float dotW = 12.f; // status dot footprint

        const float trailPad = padX;
        // Layout: [accent bar] [pad] [VAC] [sep] [Heartbeat] [sep] [dot] [val] [pad]
        float totalW = pillInset + padX
                     + szL.x + divW
                     + szH.x + divW
                     + dotW + 4.f
                     + szV.x
                     + trailPad;

        const float bx = 14.f;             // 14px from left edge (mirror of HUD's 14 from right)
        const float by = 12.f;
        const float cy = by + bh * 0.5f;
        const float ty = by + (bh - fh) * 0.5f;

        // ---- 1) drop shadow -------------------------------------------
        for (int i = 5; i > 0; --i) {
            fl->AddRectFilled({ bx - i, by - i + 2 },
                              { bx + totalW + i, by + bh + i + 2 },
                              IM_COL32(0, 0, 0, 9 + (5 - i) * 6), radius + i);
        }

        // ---- 2) backdrop with gradient --------------------------------
        const ImU32 cTop = IM_COL32(20, 18, 30, 235);
        const ImU32 cBot = IM_COL32( 8,  7, 14, 240);
        fl->AddRectFilled({ bx, by }, { bx + totalW, by + bh }, cBot, radius);
        fl->AddRectFilledMultiColor(
            { bx + 1.f, by + 1.f },
            { bx + totalW - 1.f, by + bh * 0.55f },
            cTop, cTop, cBot, cBot);

        // ---- 3) outline + inner highlight -----------------------------
        fl->AddRect({ bx, by }, { bx + totalW, by + bh }, IM_COL32(255,255,255,18), radius, 0, 1.f);
        fl->AddRect({ bx + 1.f, by + 1.f }, { bx + totalW - 1.f, by + bh - 1.f },
                    EvoAccent(55), radius - 1.f, 0, 1.f);

        // ---- 4) accent bar on left edge -------------------------------
        fl->AddRectFilled({ bx + 4.f, by + 5.f }, { bx + 6.f, by + bh - 5.f }, EvoAccent(235), 2.f);

        // ---- 5) content -----------------------------------------------
        auto Sep = [&](float& cx) {
            fl->AddCircleFilled({ cx + divW * 0.5f, cy + 0.5f }, 1.3f,
                                IM_COL32(150, 145, 175, 150), 8);
            cx += divW;
        };

        float cx = bx + pillInset + padX;
        fl->AddText({ cx, ty }, EvoAccent(245), "VAC");        cx += szL.x;  Sep(cx);
        fl->AddText({ cx, ty }, kTextBrt,        "Heartbeat"); cx += szH.x;  Sep(cx);

        // status dot — pulsing soft halo + crisp center
        const float dotR = 3.6f;
        const float dotCx = cx + dotW * 0.5f - 2.f;
        // soft halo (constant, no time-based pulse to avoid churn)
        fl->AddCircleFilled({ dotCx, cy }, dotR + 2.4f,
                            (dotCol & 0x00FFFFFF) | (40u << 24), 24);
        fl->AddCircleFilled({ dotCx, cy }, dotR, dotCol, 24);
        cx += dotW + 4.f;

        fl->AddText({ cx, ty }, valCol, valTxt);
    }

    // ============================================================
    //  FEATURE CARD GRID  ─  iOS-style mod menu
    //  Each tab shows a 3-col grid of cards. Click OPTIONS on a
    //  card to navigate into that feature's page.
    // ============================================================

    enum FeatureIcon {
        FI_CROSSHAIR, FI_TRIGGER, FI_JUMP, FI_REWIND, FI_ROTATE, FI_BOLT,
        FI_BOX, FI_SILHOUETTE, FI_TRACER, FI_DROP, FI_EYE, FI_BADGE,
        FI_GRENADE, FI_TARGET, FI_SPEAKER,
        FI_PAINT, FI_KNIFE, FI_GLOVE,
        FI_SUN, FI_FLAME, FI_FOV, FI_MOON, FI_BULB, FI_CAMERA, FI_CHECK,
        FI_GEAR, FI_WAND, FI_PALETTE, FI_HUD, FI_FLOPPY,
        FI_NONE
    };

    // ============================================================
    //  LUCIDE-STYLE ICONS  ─  24x24 grid, ~1.7px stroke, rounded
    //  Drawn around center `c` with logical radius `s`.
    // ============================================================
    inline void DrawFeatureIcon(ImDrawList* dl, int id, ImVec2 c, float scale, ImU32 col)
    {
        const float s = scale;                       // logical "12" = half of 24
        const float t = scale * 0.13f + 0.4f;        // ~1.7 px stroke at scale 14
        auto Cap = [&](ImVec2 p) { dl->AddCircleFilled(p, t * 0.5f, col, 8); };
        auto Ln  = [&](ImVec2 a, ImVec2 b) { dl->AddLine(a, b, col, t); Cap(a); Cap(b); };

        switch (id)
        {
        // ── crosshair (lucide: crosshair) ──
        case FI_CROSSHAIR: {
            dl->AddCircle(c, s * 0.78f, col, 32, t);
            Ln({c.x, c.y - s*1.05f}, {c.x, c.y - s*0.55f});
            Ln({c.x, c.y + s*0.55f}, {c.x, c.y + s*1.05f});
            Ln({c.x - s*1.05f, c.y}, {c.x - s*0.55f, c.y});
            Ln({c.x + s*0.55f, c.y}, {c.x + s*1.05f, c.y});
            dl->AddCircleFilled(c, s * 0.10f, col, 12);
            break; }

        // ── triggerbot (lucide: target with center) ──
        case FI_TRIGGER: {
            dl->AddCircle(c, s * 0.95f, col, 32, t);
            dl->AddCircle(c, s * 0.55f, col, 28, t);
            dl->AddCircleFilled(c, s * 0.18f, col, 16);
            break; }

        // ── jump (lucide: arrow-up) ──
        case FI_JUMP: {
            Ln({c.x, c.y + s*0.95f}, {c.x, c.y - s*0.95f});
            Ln({c.x, c.y - s*0.95f}, {c.x - s*0.55f, c.y - s*0.4f});
            Ln({c.x, c.y - s*0.95f}, {c.x + s*0.55f, c.y - s*0.4f});
            break; }

        // ── rewind (lucide: rotate-ccw) ──
        case FI_REWIND: {
            int seg = 24; float r = s * 0.85f;
            for (int i = 0; i < seg; ++i) {
                float a0 = -2.6f + ((float)i / seg) * 5.0f;
                float a1 = -2.6f + ((float)(i+1) / seg) * 5.0f;
                dl->AddLine(
                    { c.x + cosf(a0)*r, c.y + sinf(a0)*r },
                    { c.x + cosf(a1)*r, c.y + sinf(a1)*r }, col, t);
            }
            // arrowhead at start (top-left)
            float ea = -2.6f;
            ImVec2 ep{ c.x + cosf(ea)*r, c.y + sinf(ea)*r };
            Ln(ep, { ep.x - s*0.05f, ep.y + s*0.55f });
            Ln(ep, { ep.x + s*0.55f, ep.y + s*0.05f });
            break; }

        // ── rotate (lucide: refresh-cw / two arrows) ──
        case FI_ROTATE: {
            float r = s * 0.78f;
            int seg = 18;
            for (int half = 0; half < 2; ++half) {
                float base = half ? 0.f : 3.14159f;
                for (int i = 0; i < seg; ++i) {
                    float a0 = base + 0.3f + ((float)i / seg) * 2.2f;
                    float a1 = base + 0.3f + ((float)(i+1) / seg) * 2.2f;
                    dl->AddLine(
                        { c.x + cosf(a0)*r, c.y + sinf(a0)*r },
                        { c.x + cosf(a1)*r, c.y + sinf(a1)*r }, col, t);
                }
                float ea = base + 0.3f + 2.2f;
                ImVec2 ep{ c.x + cosf(ea)*r, c.y + sinf(ea)*r };
                ImVec2 perp{ -sinf(ea), cosf(ea) };
                Ln(ep, { ep.x + perp.x * s*0.45f, ep.y + perp.y * s*0.45f });
                Ln(ep, { ep.x + cosf(ea) * s*0.4f, ep.y + sinf(ea) * s*0.4f });
            }
            break; }

        // ── bolt (lucide: zap, filled) ──
        case FI_BOLT: {
            ImVec2 pts[6] = {
                { c.x - s*0.05f, c.y - s*1.0f },
                { c.x + s*0.55f, c.y - s*1.0f },
                { c.x + s*0.10f, c.y - s*0.05f },
                { c.x + s*0.55f, c.y - s*0.05f },
                { c.x - s*0.30f, c.y + s*1.0f },
                { c.x + s*0.05f, c.y + s*0.10f },
            };
            dl->PathClear();
            for (int i = 0; i < 6; ++i) dl->PathLineTo(pts[i]);
            dl->PathFillConvex(col);
            break; }

        // ── box (lucide: square) ──
        case FI_BOX: {
            float r = s * 0.85f;
            dl->AddRect({ c.x - r, c.y - r }, { c.x + r, c.y + r }, col, 3.f, 0, t);
            break; }

        // ── silhouette (lucide: user) ──
        case FI_SILHOUETTE: {
            dl->AddCircle({ c.x, c.y - s*0.4f }, s * 0.34f, col, 24, t);
            int seg = 14;
            float w = s * 0.7f;
            // shoulders arc
            for (int i = 0; i <= seg; ++i) {
                float u0 = (float)i / seg, u1 = (float)(i+1) / seg;
                if (i == seg) break;
                float a0 = 3.14159f + u0 * 3.14159f;
                float a1 = 3.14159f + u1 * 3.14159f;
                dl->AddLine(
                    { c.x + cosf(a0)*w, c.y + s*0.85f + sinf(a0)*s*0.55f },
                    { c.x + cosf(a1)*w, c.y + s*0.85f + sinf(a1)*s*0.55f }, col, t);
            }
            break; }

        // ── tracer (lucide: move-up-right diag) ──
        case FI_TRACER: {
            ImVec2 a{ c.x - s*0.85f, c.y + s*0.85f };
            ImVec2 b{ c.x + s*0.85f, c.y - s*0.85f };
            Ln(a, b);
            // arrowhead at b
            Ln(b, { b.x - s*0.55f, b.y });
            Ln(b, { b.x, b.y + s*0.55f });
            break; }

        // ── droplet (lucide: droplet) ──
        case FI_DROP: {
            int seg = 26;
            dl->PathClear();
            dl->PathLineTo({ c.x, c.y - s*0.95f });
            for (int i = 0; i <= seg; ++i) {
                float u = (float)i / seg;
                float a = -1.5708f + u * 6.2832f * 0.62f - 0.6f;
                dl->PathLineTo({ c.x + cosf(a)*s*0.65f, c.y + s*0.25f + sinf(a)*s*0.65f });
            }
            dl->PathStroke(col, ImDrawFlags_Closed, t);
            break; }

        // ── eye (lucide: eye) ──
        case FI_EYE: {
            const float w = s * 1.0f, h = s * 0.55f;
            dl->PathClear();
            dl->PathLineTo({ c.x - w, c.y });
            dl->PathBezierCubicCurveTo({ c.x - w*0.6f, c.y - h }, { c.x + w*0.6f, c.y - h }, { c.x + w, c.y }, 18);
            dl->PathStroke(col, 0, t);
            dl->PathClear();
            dl->PathLineTo({ c.x + w, c.y });
            dl->PathBezierCubicCurveTo({ c.x + w*0.6f, c.y + h }, { c.x - w*0.6f, c.y + h }, { c.x - w, c.y }, 18);
            dl->PathStroke(col, 0, t);
            dl->AddCircle(c, s * 0.32f, col, 20, t);
            dl->AddCircleFilled(c, s * 0.12f, col, 12);
            break; }

        // ── badge / award (lucide: award) ──
        case FI_BADGE: {
            dl->AddCircle({ c.x, c.y - s*0.15f }, s * 0.55f, col, 28, t);
            // ribbons
            Ln({ c.x - s*0.32f, c.y + s*0.30f }, { c.x - s*0.55f, c.y + s*1.0f });
            Ln({ c.x - s*0.55f, c.y + s*1.0f }, { c.x - s*0.18f, c.y + s*0.78f });
            Ln({ c.x - s*0.18f, c.y + s*0.78f }, { c.x + s*0.05f, c.y + s*1.0f });
            Ln({ c.x + s*0.05f, c.y + s*1.0f }, { c.x + s*0.42f, c.y + s*0.30f });
            break; }

        // ── grenade (circle + safety lever) ──
        case FI_GRENADE: {
            dl->AddCircle({ c.x, c.y + s*0.18f }, s * 0.62f, col, 28, t);
            // top cap
            Ln({ c.x - s*0.18f, c.y - s*0.55f }, { c.x + s*0.18f, c.y - s*0.55f });
            Ln({ c.x - s*0.18f, c.y - s*0.55f }, { c.x - s*0.18f, c.y - s*0.40f });
            Ln({ c.x + s*0.18f, c.y - s*0.55f }, { c.x + s*0.18f, c.y - s*0.40f });
            // pin ring
            dl->AddCircle({ c.x + s*0.45f, c.y - s*0.55f }, s * 0.12f, col, 12, t);
            Ln({ c.x + s*0.18f, c.y - s*0.48f }, { c.x + s*0.36f, c.y - s*0.50f });
            break; }

        // ── target / focus (lucide: focus) ──
        case FI_TARGET: {
            dl->AddCircleFilled(c, s * 0.18f, col, 16);
            // L-corners
            float r = s * 0.85f;
            Ln({ c.x - r, c.y - r*0.55f }, { c.x - r, c.y - r });
            Ln({ c.x - r, c.y - r }, { c.x - r*0.55f, c.y - r });
            Ln({ c.x + r*0.55f, c.y - r }, { c.x + r, c.y - r });
            Ln({ c.x + r, c.y - r }, { c.x + r, c.y - r*0.55f });
            Ln({ c.x + r, c.y + r*0.55f }, { c.x + r, c.y + r });
            Ln({ c.x + r, c.y + r }, { c.x + r*0.55f, c.y + r });
            Ln({ c.x - r*0.55f, c.y + r }, { c.x - r, c.y + r });
            Ln({ c.x - r, c.y + r }, { c.x - r, c.y + r*0.55f });
            break; }

        // ── speaker (lucide: volume-2) ──
        case FI_SPEAKER: {
            ImVec2 pts[5] = {
                { c.x - s*0.85f, c.y - s*0.30f },
                { c.x - s*0.30f, c.y - s*0.30f },
                { c.x + s*0.20f, c.y - s*0.75f },
                { c.x + s*0.20f, c.y + s*0.75f },
                { c.x - s*0.30f, c.y + s*0.30f },
            };
            for (int i = 0; i < 5; ++i) Ln(pts[i], pts[(i+1)%5]);
            // sound waves
            int seg = 10;
            for (int w = 0; w < 2; ++w) {
                float r = s * (0.40f + w * 0.35f);
                for (int i = 0; i < seg; ++i) {
                    float u0 = (float)i / seg, u1 = (float)(i+1) / seg;
                    float a0 = -0.9f + u0 * 1.8f, a1 = -0.9f + u1 * 1.8f;
                    dl->AddLine(
                        { c.x + s*0.35f + cosf(a0)*r, c.y + sinf(a0)*r },
                        { c.x + s*0.35f + cosf(a1)*r, c.y + sinf(a1)*r }, col, t);
                }
            }
            break; }

        // ── paint / brush (lucide: paintbrush) ──
        case FI_PAINT: {
            // brush head
            dl->AddRect({ c.x - s*0.35f, c.y - s*0.95f }, { c.x + s*0.55f, c.y - s*0.30f }, col, 3.f, 0, t);
            // ferrule
            Ln({ c.x - s*0.20f, c.y - s*0.30f }, { c.x + s*0.40f, c.y - s*0.30f });
            Ln({ c.x - s*0.20f, c.y - s*0.30f }, { c.x - s*0.20f, c.y - s*0.10f });
            Ln({ c.x + s*0.40f, c.y - s*0.30f }, { c.x + s*0.40f, c.y - s*0.10f });
            // handle taper
            Ln({ c.x - s*0.20f, c.y - s*0.10f }, { c.x - s*0.55f, c.y + s*0.95f });
            Ln({ c.x + s*0.40f, c.y - s*0.10f }, { c.x + s*0.05f, c.y + s*0.95f });
            Ln({ c.x - s*0.55f, c.y + s*0.95f }, { c.x + s*0.05f, c.y + s*0.95f });
            break; }

        // ── knife / sword (lucide: sword) ──
        case FI_KNIFE: {
            // blade
            Ln({ c.x - s*0.95f, c.y + s*0.30f }, { c.x + s*0.20f, c.y - s*0.85f });
            Ln({ c.x + s*0.20f, c.y - s*0.85f }, { c.x + s*0.55f, c.y - s*0.85f });
            Ln({ c.x + s*0.55f, c.y - s*0.85f }, { c.x + s*0.55f, c.y - s*0.50f });
            Ln({ c.x + s*0.55f, c.y - s*0.50f }, { c.x - s*0.60f, c.y + s*0.60f });
            Ln({ c.x - s*0.60f, c.y + s*0.60f }, { c.x - s*0.95f, c.y + s*0.30f });
            // guard
            Ln({ c.x - s*0.75f, c.y + s*0.45f }, { c.x - s*0.45f, c.y + s*0.75f });
            // hilt
            Ln({ c.x - s*0.45f, c.y + s*0.75f }, { c.x - s*0.85f, c.y + s*0.95f });
            Ln({ c.x - s*0.95f, c.y + s*0.45f }, { c.x - s*0.75f, c.y + s*0.85f });
            break; }

        // ── glove / hand (lucide: hand) ──
        case FI_GLOVE: {
            // palm
            float pl = c.x - s*0.45f, pr = c.x + s*0.45f;
            float pt = c.y - s*0.30f, pb = c.y + s*0.65f;
            dl->AddRect({ pl, pt }, { pr, pb }, col, s*0.25f, 0, t);
            // fingers (3 rounded)
            for (int i = 0; i < 3; ++i) {
                float fx = pl + s*0.18f + i * s*0.30f;
                Ln({ fx - s*0.10f, pt }, { fx - s*0.10f, c.y - s*0.85f });
                Ln({ fx + s*0.10f, pt }, { fx + s*0.10f, c.y - s*0.85f });
                int seg = 6;
                for (int k = 0; k < seg; ++k) {
                    float u0 = (float)k / seg, u1 = (float)(k+1) / seg;
                    float a0 = 3.14159f + u0 * 3.14159f, a1 = 3.14159f + u1 * 3.14159f;
                    dl->AddLine(
                        { fx + cosf(a0)*s*0.10f, c.y - s*0.85f + sinf(a0)*s*0.10f },
                        { fx + cosf(a1)*s*0.10f, c.y - s*0.85f + sinf(a1)*s*0.10f }, col, t);
                }
            }
            // thumb on right
            Ln({ pr, c.y - s*0.10f }, { c.x + s*0.85f, c.y + s*0.20f });
            Ln({ pr, c.y + s*0.30f }, { c.x + s*0.85f, c.y + s*0.45f });
            int seg = 8;
            for (int k = 0; k < seg; ++k) {
                float u0 = (float)k / seg, u1 = (float)(k+1) / seg;
                float a0 = -1.5708f + u0 * 3.14159f, a1 = -1.5708f + u1 * 3.14159f;
                dl->AddLine(
                    { c.x + s*0.85f + cosf(a0)*s*0.13f, c.y + s*0.32f + sinf(a0)*s*0.13f },
                    { c.x + s*0.85f + cosf(a1)*s*0.13f, c.y + s*0.32f + sinf(a1)*s*0.13f }, col, t);
            }
            break; }

        // ── sun (lucide: sun) ──
        case FI_SUN: {
            dl->AddCircle(c, s * 0.42f, col, 24, t);
            for (int i = 0; i < 8; ++i) {
                float a = (float)i * 0.7854f;
                Ln({ c.x + cosf(a)*s*0.65f, c.y + sinf(a)*s*0.65f },
                   { c.x + cosf(a)*s*0.95f, c.y + sinf(a)*s*0.95f });
            }
            break; }

        // ── flame (lucide: flame) ──
        case FI_FLAME: {
            dl->PathClear();
            dl->PathLineTo({ c.x, c.y - s*0.95f });
            dl->PathBezierCubicCurveTo(
                { c.x + s*0.85f, c.y - s*0.30f },
                { c.x + s*0.95f, c.y + s*0.55f },
                { c.x, c.y + s*0.95f }, 16);
            dl->PathBezierCubicCurveTo(
                { c.x - s*0.95f, c.y + s*0.55f },
                { c.x - s*0.55f, c.y + s*0.05f },
                { c.x - s*0.20f, c.y - s*0.10f }, 16);
            dl->PathBezierCubicCurveTo(
                { c.x - s*0.05f, c.y - s*0.30f },
                { c.x - s*0.30f, c.y - s*0.65f },
                { c.x, c.y - s*0.95f }, 16);
            dl->PathStroke(col, ImDrawFlags_Closed, t);
            break; }

        // ── FOV (lucide: scan / corners) ──
        case FI_FOV: {
            float r = s * 0.95f;
            // four corner brackets
            Ln({ c.x - r, c.y - r*0.45f }, { c.x - r, c.y - r });
            Ln({ c.x - r, c.y - r },       { c.x - r*0.45f, c.y - r });
            Ln({ c.x + r*0.45f, c.y - r }, { c.x + r, c.y - r });
            Ln({ c.x + r, c.y - r },       { c.x + r, c.y - r*0.45f });
            Ln({ c.x + r, c.y + r*0.45f }, { c.x + r, c.y + r });
            Ln({ c.x + r, c.y + r },       { c.x + r*0.45f, c.y + r });
            Ln({ c.x - r*0.45f, c.y + r }, { c.x - r, c.y + r });
            Ln({ c.x - r, c.y + r },       { c.x - r, c.y + r*0.45f });
            // center scan line
            Ln({ c.x - r*0.55f, c.y }, { c.x + r*0.55f, c.y });
            break; }

        // ── moon (lucide: moon) ──
        case FI_MOON: {
            // crescent via two-arc closed path
            int seg = 28;
            dl->PathClear();
            for (int i = 0; i <= seg; ++i) {
                float a = -1.4f + (float)i / seg * 3.5f;
                dl->PathLineTo({ c.x + cosf(a) * s*0.92f, c.y + sinf(a) * s*0.92f });
            }
            for (int i = seg; i >= 0; --i) {
                float a = -1.4f + (float)i / seg * 3.5f;
                dl->PathLineTo({ c.x + s*0.35f + cosf(a) * s*0.78f, c.y - s*0.05f + sinf(a) * s*0.78f });
            }
            dl->PathStroke(col, ImDrawFlags_Closed, t);
            break; }

        // ── lightbulb (lucide: lightbulb) ──
        case FI_BULB: {
            // bulb (circle with bottom flat)
            int seg = 22;
            dl->PathClear();
            for (int i = 0; i <= seg; ++i) {
                float a = -2.6f + (float)i / seg * 5.2f;
                dl->PathLineTo({ c.x + cosf(a) * s*0.55f, c.y - s*0.20f + sinf(a) * s*0.55f });
            }
            dl->PathStroke(col, 0, t);
            // base lines
            Ln({ c.x - s*0.30f, c.y + s*0.40f }, { c.x + s*0.30f, c.y + s*0.40f });
            Ln({ c.x - s*0.25f, c.y + s*0.60f }, { c.x + s*0.25f, c.y + s*0.60f });
            Ln({ c.x - s*0.18f, c.y + s*0.80f }, { c.x + s*0.18f, c.y + s*0.80f });
            // filament hint
            Ln({ c.x - s*0.20f, c.y - s*0.10f }, { c.x + s*0.20f, c.y - s*0.10f });
            break; }

        // ── camera (lucide: camera) ──
        case FI_CAMERA: {
            dl->AddRect({ c.x - s*0.95f, c.y - s*0.45f }, { c.x + s*0.95f, c.y + s*0.65f }, col, 3.f, 0, t);
            // top notch
            Ln({ c.x - s*0.30f, c.y - s*0.45f }, { c.x - s*0.20f, c.y - s*0.70f });
            Ln({ c.x + s*0.30f, c.y - s*0.45f }, { c.x + s*0.20f, c.y - s*0.70f });
            Ln({ c.x - s*0.20f, c.y - s*0.70f }, { c.x + s*0.20f, c.y - s*0.70f });
            // lens
            dl->AddCircle({ c.x, c.y + s*0.10f }, s * 0.32f, col, 24, t);
            break; }

        // ── check (lucide: check-circle) ──
        case FI_CHECK: {
            dl->AddCircle(c, s * 0.95f, col, 32, t);
            Ln({ c.x - s*0.42f, c.y + s*0.05f }, { c.x - s*0.05f, c.y + s*0.42f });
            Ln({ c.x - s*0.05f, c.y + s*0.42f }, { c.x + s*0.50f, c.y - s*0.30f });
            break; }

        // ── gear (lucide: settings) ──
        case FI_GEAR: {
            int teeth = 8;
            float rO = s * 0.95f, rI = s * 0.70f;
            for (int i = 0; i < teeth; ++i) {
                float a   = (float)i * 6.2832f / teeth;
                float ax  = cosf(a), ay = sinf(a);
                float pX  = -ay * s * 0.13f, pY = ax * s * 0.13f;
                ImVec2 q[4] = {
                    { c.x + ax*rI + pX, c.y + ay*rI + pY },
                    { c.x + ax*rO + pX, c.y + ay*rO + pY },
                    { c.x + ax*rO - pX, c.y + ay*rO - pY },
                    { c.x + ax*rI - pX, c.y + ay*rI - pY },
                };
                for (int k = 0; k < 4; ++k) dl->AddLine(q[k], q[(k+1)%4], col, t);
            }
            // body ring
            dl->AddCircle(c, rI, col, 32, t);
            // center hole
            dl->AddCircle(c, s * 0.28f, col, 18, t);
            break; }

        // ── wand / sparkles (lucide: sparkles) ──
        case FI_WAND: {
            // big star
            auto Star = [&](ImVec2 cc, float sz) {
                Ln({ cc.x, cc.y - sz }, { cc.x, cc.y + sz });
                Ln({ cc.x - sz, cc.y }, { cc.x + sz, cc.y });
                Ln({ cc.x - sz*0.65f, cc.y - sz*0.65f }, { cc.x + sz*0.65f, cc.y + sz*0.65f });
                Ln({ cc.x + sz*0.65f, cc.y - sz*0.65f }, { cc.x - sz*0.65f, cc.y + sz*0.65f });
            };
            Star({ c.x - s*0.10f, c.y }, s * 0.55f);
            Star({ c.x + s*0.65f, c.y - s*0.55f }, s * 0.25f);
            Star({ c.x + s*0.55f, c.y + s*0.65f }, s * 0.20f);
            break; }

        // ── palette (lucide: palette) ──
        case FI_PALETTE: {
            // outline blob
            int seg = 28;
            dl->PathClear();
            for (int i = 0; i <= seg; ++i) {
                float a = (float)i / seg * 6.2832f;
                float r = s * 0.92f;
                dl->PathLineTo({ c.x + cosf(a)*r, c.y + sinf(a)*r });
            }
            dl->PathStroke(col, ImDrawFlags_Closed, t);
            // notch on right
            dl->AddCircle({ c.x + s*0.55f, c.y + s*0.10f }, s * 0.14f, col, 14, t);
            // paint dots
            dl->AddCircleFilled({ c.x - s*0.45f, c.y - s*0.20f }, s * 0.10f, col, 12);
            dl->AddCircleFilled({ c.x - s*0.05f, c.y - s*0.55f }, s * 0.10f, col, 12);
            dl->AddCircleFilled({ c.x + s*0.30f, c.y - s*0.40f }, s * 0.10f, col, 12);
            dl->AddCircleFilled({ c.x - s*0.40f, c.y + s*0.35f }, s * 0.10f, col, 12);
            break; }

        // ── HUD / layout (lucide: layout-dashboard) ──
        case FI_HUD: {
            float r = s * 0.92f;
            // big cell top-left
            dl->AddRect({ c.x - r, c.y - r }, { c.x - s*0.10f, c.y - s*0.10f }, col, 2.5f, 0, t);
            // tall cell right
            dl->AddRect({ c.x + s*0.10f, c.y - r }, { c.x + r, c.y + s*0.20f }, col, 2.5f, 0, t);
            // bar bottom-left
            dl->AddRect({ c.x - r, c.y + s*0.10f }, { c.x - s*0.10f, c.y + r }, col, 2.5f, 0, t);
            // small bottom-right
            dl->AddRect({ c.x + s*0.10f, c.y + s*0.40f }, { c.x + r, c.y + r }, col, 2.5f, 0, t);
            break; }

        // ── floppy / save (lucide: save) ──
        case FI_FLOPPY: {
            float r = s * 0.92f;
            dl->AddRect({ c.x - r, c.y - r }, { c.x + r, c.y + r }, col, 3.f, 0, t);
            // top metal slider
            Ln({ c.x - r*0.55f, c.y - r }, { c.x - r*0.55f, c.y - s*0.30f });
            Ln({ c.x + r*0.55f, c.y - r }, { c.x + r*0.55f, c.y - s*0.30f });
            Ln({ c.x - r*0.55f, c.y - s*0.30f }, { c.x + r*0.55f, c.y - s*0.30f });
            Ln({ c.x + r*0.30f, c.y - r }, { c.x + r*0.30f, c.y - s*0.55f });
            // label area
            dl->AddRect({ c.x - r*0.65f, c.y + s*0.05f }, { c.x + r*0.65f, c.y + r*0.85f }, col, 1.5f, 0, t);
            break; }

        default: break;
        }
    }

    // ============================================================
    //  PAGE RENDERERS  (one per existing section; called from grid)
    // ============================================================

    // ── AIM tab ────────────────────────────────────────────────
    inline void Pg_Aimbot()
    {
        EvoCheckbox("Safety Governor##gov", &Aimbot::Governor::gcfg.enabled);
        if (Aimbot::Governor::gcfg.enabled) {
            char hsBuf[32]; sprintf(hsBuf, "HS %%: %.1f", Aimbot::Governor::GetHSPercent());
            SynthSep();
            EvoSliderFloat("Max HS %%##ghc", &Aimbot::Governor::gcfg.hsCapPercent, 25.f, 65.f, "%.0f");
            SynthSep();
            ImGui::TextDisabled("  Current %s", hsBuf);
        }
        SynthSep();
        const char* akn[] = { "Auto (Mouse1)", "Right Click", "Always On" };
        EvoCombo("Aim Key##ak", &Aimbot::cfg.aimKey, akn, 3);
        SynthSep();
        EvoSliderFloat("FOV",          &Aimbot::cfg.fov,          0.5f, 15.f,  "%.1f");
        SynthSep();
        EvoSliderFloat("Smoothing",    &Aimbot::cfg.smoothing,    20.f, 100.f, "%.0f");
        SynthSep();
        EvoSliderFloat("Humanization", &Aimbot::cfg.humanization, 0.30f, 1.f,   "%.2f");
        SynthSep();
        // "Silent Aim" toggle removed — see Pg_Aimbot for context.
        EvoCheckbox("Team Check",    &Aimbot::cfg.teamCheck);
        SynthSep();
        EvoCheckbox("No Recoil",       &Aimbot::cfg.noRecoil);
        SynthSep();
        const char* bones[]   = { "Head","Neck","Chest","Pelvis" };
        const int   boneIds[] = { 7, 6, 23, 1 };
        int bIdx = 0;
        for (int i = 0; i < 4; ++i) if (boneIds[i] == Aimbot::cfg.targetBone) bIdx = i;
        if (EvoCombo("Hitbox##hb", &bIdx, bones, 4)) Aimbot::cfg.targetBone = boneIds[bIdx];
        SynthSep();
        EvoCheckbox("Multi-Bone Scan",  &Aimbot::cfg.multiBone);
        SynthSep();
        EvoCheckbox("Velocity Predict", &Aimbot::cfg.velPredict);
        SynthSep();
        EvoCheckbox("Vis Check",     &Aimbot::cfg.visCheck);
        SynthSep();
        EvoCheckbox("Smoke Check",   &Aimbot::cfg.smokeCheck);
        SynthSep();
        EvoCheckbox("Jump Shot",        &Aimbot::cfg.jumpShot);
        SynthSep();
        EvoCheckbox("No Spread",        &Aimbot::cfg.noSpread);
    }
    inline void Pg_Triggerbot()
    {
        EvoCheckbox("Team Check##ttc",   &Triggerbot::cfg.teamCheck);
        SynthSep(); EvoCheckbox("Smoke Check##tsc",  &Triggerbot::cfg.smokeCheck);
        SynthSep(); EvoCheckbox("Scope Only##tso",   &Triggerbot::cfg.scopeOnly);
        SynthSep(); ImGui::SliderInt("Min Delay##tmd", &Triggerbot::cfg.minDelayMs, 0, 200);
        SynthSep(); ImGui::SliderInt("Max Delay##txd", &Triggerbot::cfg.maxDelayMs, 0, 300);
    }
    inline void Pg_Bhop()
    {
        EvoSliderFloat("Max Speed##bms", &Bhop::cfg.maxSpeed, 200.f, 500.f, "%.0f");
        SynthSep(); EvoSliderFloat("Min Speed##bns", &Bhop::cfg.minSpeed,  10.f, 120.f, "%.0f");
        SynthSep(); EvoCheckbox("Auto Strafe##bs",   &Bhop::cfg.autoStrafe);
        SynthSep();
        const char* sm[] = { "Velocity (smooth)", "Mouse Yaw (manual)" };
        EvoCombo("Strafe Mode##bsm", &Bhop::cfg.strafeMode, sm, 2);
        SynthSep(); EvoCheckbox("Velocity Display##bvd", &Bhop::cfg.showVelocity);
    }

    // Rage Mode page — danger-tier toggles (Neverlose / Memesense style).
    // Flipping `Enable` clobbers aim physics (silent / always-on / no
    // smoothing / no vis-check) for the session. Toggle off to get your
    // legit settings back exactly as they were before.
    inline void Pg_Rage()
    {
        ImGui::TextColored({ 1.f, 0.4f, 0.4f, 1.f }, "  ⚠ Danger — visible to spectators");
        ImGui::Dummy({ 0.f, 4.f });
        EvoCheckbox("Enable Rage##rge", &Aimbot::Rage::cfg.enabled);
        if (!Aimbot::Rage::cfg.enabled) return;
        SynthSep(); EvoCheckbox("Always On##rgao",       &Aimbot::Rage::cfg.alwaysOn);
        // "Force Silent" removed \u2014 silent-aim path disabled build-wide.
        SynthSep(); EvoCheckbox("Instant Aim##rgi",      &Aimbot::Rage::cfg.instant);
        SynthSep(); EvoCheckbox("Wallbang (no vis)##rgw",&Aimbot::Rage::cfg.forceWallbang);
        SynthSep(); EvoCheckbox("Body Aim##rgb",         &Aimbot::Rage::cfg.forceBaim);
        SynthSep(); EvoCheckbox("Lowest HP First##rgh",  &Aimbot::Rage::cfg.lowestHpFirst);
        SynthSep(); EvoCheckbox("Skip Armored##rga",     &Aimbot::Rage::cfg.prioritizeArmored);
        SynthSep(); ImGui::SliderInt("Min Damage##rgd",  &Aimbot::Rage::cfg.minDamage, 0, 100);
    }
    inline void Pg_Backtrack()
    {
        ImGui::SliderInt("Max Ticks##btmt", &Backtrack::cfg.maxTicksBack, 1, 20);
        SynthSep(); EvoCheckbox("Draw History##btdh", &Backtrack::cfg.drawHistory);
    }
    inline void Pg_AntiAim()
    {
        static const char* pitchModes[] = { "Off", "Down", "Up", "Zero" };
        static const char* yawModes[]   = { "Off", "Spin", "Jitter", "Desync" };
        ImGui::Combo("Pitch##aap", &AntiAim::cfg.pitchMode, pitchModes, 4);
        SynthSep(); ImGui::Combo("Yaw##aay",   &AntiAim::cfg.yawMode,   yawModes,   4);
        if (AntiAim::cfg.yawMode == 1) { SynthSep(); EvoSliderFloat("Speed##aas", &AntiAim::cfg.spinSpeed, 1.f, 45.f, "%.0f"); }
        if (AntiAim::cfg.yawMode == 3) { SynthSep(); EvoSliderFloat("Delta##aad", &AntiAim::cfg.desyncDelta, 10.f, 58.f, "%.0f"); }
    }
    inline void Pg_FakeLag()
    {
        static const char* flModes[] = { "Fixed", "Dynamic", "On Key" };
        ImGui::Combo("Mode##flm", &FakeLag::cfg.mode, flModes, 3);
        SynthSep(); ImGui::SliderInt("Max Choke##flmc", &FakeLag::cfg.maxChoke, 1, 14);
    }

    // ── VIS tab ────────────────────────────────────────────────
    inline void Pg_ESP()
    {
        EvoCheckbox("Box",       &ESP::cfg.box);
        if (ESP::cfg.box) { SynthSep(); const char* bst[]={"Normal","Corners"}; EvoCombo("Style##bs",&ESP::cfg.boxStyle,bst,2); }
        SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::ColorEdit4("Box Color##bc", ESP::cfg.boxColor, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
        SynthSep(); EvoCheckbox("Skeleton",   &ESP::cfg.skeleton);
        SynthSep(); EvoCheckbox("Health Bar", &ESP::cfg.healthBar);
        SynthSep(); EvoCheckbox("Name",       &ESP::cfg.name);
        SynthSep(); EvoCheckbox("Distance",   &ESP::cfg.distance);
        SynthSep(); EvoCheckbox("Weapon",     &ESP::cfg.weapon);
        if (ESP::cfg.weapon) { SynthSep(); EvoCheckbox("Use Weapon Icons##wi",&ESP::cfg.weaponIcon); }
        SynthSep(); EvoCheckbox("Team Check##etm", &ESP::cfg.teamCheck);
        SynthSep(); EvoCheckbox("Bomb Timer", &ESP::cfg.bombTimer);
        if (ESP::cfg.bombTimer) { SynthSep(); const char* bts[]={"Classic","Vivid","Compact"}; EvoCombo("Bomb Style##bts",&ESP::cfg.bombTimerStyle,bts,3); }
        SynthSep(); EvoCheckbox("Vis Color",  &ESP::cfg.visColorEnabled);
        if (ESP::cfg.visColorEnabled) {
            SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Visible##vc", ESP::cfg.visibleColor, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
            SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Hidden##hc",  ESP::cfg.hiddenColor,  ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
        }
    }
    inline void Pg_Chams()
    {
        EvoCheckbox("Wallhack##cw", &Chams::cfg.wallhack);
        auto SlotW = [](const char* n, Chams::SlotStyle& sl) {
            ImGui::PushID(n);
            if (sl.material < 0 || sl.material >= Chams::MAT_COUNT) sl.material = Chams::MAT_NONE;
            ImGui::Text("  %s", n); ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 30);
            if (ImGui::BeginCombo("##m", Chams::MaterialNames[sl.material])) {
                for (int i = 0; i < Chams::MAT_COUNT; ++i) {
                    bool sel = (sl.material == i);
                    if (ImGui::Selectable(Chams::MaterialNames[i], sel)) sl.material = i;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (sl.material != Chams::MAT_NONE) {
                ImGui::SameLine();
                ImGui::ColorEdit4("##cc", sl.color, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
            }
            ImGui::PopID();
        };
        SynthSep(); SlotW("Visible", Chams::cfg.playerVis);
        if (Chams::cfg.wallhack) { SynthSep(); SlotW("Walls", Chams::cfg.playerHid); }
        SynthSep(); EvoCheckbox("Hand Chams##hc", &Chams::cfg.handsEnabled);
        if (Chams::cfg.handsEnabled) { SynthSep(); SlotW("Hands", Chams::cfg.hands); }
        SynthSep(); EvoCheckbox("Weapon Chams##wc", &Chams::cfg.weaponsEnabled);
        if (Chams::cfg.weaponsEnabled) { SynthSep(); SlotW("Weapons", Chams::cfg.weapons); }
    }
    inline void Pg_Tracers()
    {
        ImGui::TextColored({0.7f,0.7f,0.8f,0.85f}, "No additional options.");
        ImGui::TextColored({0.55f,0.55f,0.65f,0.7f}, "Use the toggle on the card to enable.");
    }
    inline void Pg_DamageInd()
    {
        const char* dp[]={"Left","Right"}; const char* ds[]={"Classic","Minimal","Bold"};
        EvoCombo("Position##dp", &DamageIndicator::cfg.position, dp, 2);
        SynthSep(); EvoCombo("Style##ds",    &DamageIndicator::cfg.style,    ds, 3);
    }
    inline void Pg_Spectators()
    {
        EvoCheckbox("Spectator List", &ESP::cfg.spectators);
        if (ESP::cfg.spectators) { SynthSep(); const char* sps[]={"Classic","Stealth","Minimal"};
            EvoCombo("List Style##sls", &ESP::cfg.spectatorStyle, sps, 3); }
    }
    inline void Pg_RankReveal()
    {
        ImGui::TextColored({0.7f,0.7f,0.8f,0.85f}, "No additional options.");
        ImGui::TextColored({0.55f,0.55f,0.65f,0.7f}, "Use the toggle on the card to enable.");
    }
    inline void Pg_NadePred()
    {
        EvoCheckbox("Show Trail##gt",   &GrenadePrediction::cfg.showTrail);
        SynthSep(); EvoCheckbox("Show Landing##gl", &GrenadePrediction::cfg.showLanding);
        SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::ColorEdit4("Trail##tc", GrenadePrediction::cfg.trailColor, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
    }
    inline void Pg_NadeHelper()
    {
        EvoSliderFloat("Trigger Dist##nt", &NadeHelper::cfg.triggerDist,  50.f, 300.f, "%.0f");
        SynthSep(); EvoSliderFloat("Aim Tol##na",      &NadeHelper::cfg.aimTolerance,  1.f,  10.f, "%.1f");
    }
    inline void Pg_SoundESP()
    {
        EvoCheckbox("Footstep Marks##sfm", &SoundESP::cfg.showFootsteps);
        SynthSep(); EvoSliderFloat("Max Dist##smd",  &SoundESP::cfg.maxDistance, 500.f, 4000.f, "%.0f");
        SynthSep(); EvoSliderFloat("Ring Size##srs", &SoundESP::cfg.ringRadius,   30.f, 150.f, "%.0f");
        SynthSep(); EvoSliderFloat("Arrow Size##sas",&SoundESP::cfg.indicatorSize,15.f,  80.f, "%.0f");
    }

    // ── SKN tab ────────────────────────────────────────────────
    inline void Pg_SkinChanger()
    {
        if (EvoButton("Randomize All##ra")) SkinChanger::RandomizeAll();
        SynthSep(); if (EvoButton("Force Update##fu")) {
            SkinChanger::ForceFullUpdate();
            SkinChanger::lastKnifeDefIdx = 0;
            SkinChanger::lastGloveSpawnTime = 0.f;
            SkinChanger::gloveRefreshFrames = 0;
        }
        SynthSep(); if (EvoButton("Inject Locker Items##ili")) InventoryChanger::ResetAutoInject();

        SynthSep();
        static const char* wn[SkinChanger::kWeaponCount]{};
        static bool wnInit = false;
        if (!wnInit) { for (int i = 0; i < SkinChanger::kWeaponCount; ++i) wn[i] = SkinChanger::kWeapons[i].name; wnInit = true; }
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##wsel", &SkinChanger::cfg.activeWeaponSlot, wn, SkinChanger::kWeaponCount);
        int ws = SkinChanger::cfg.activeWeaponSlot;
        if (ws >= 0 && ws < SkinChanger::kWeaponCount) {
            auto& skin = SkinChanger::cfg.weapons[ws];
            SynthSep(); EvoCheckbox("Enable##wp", &skin.enabled);
            if (skin.enabled) {
                SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputInt("Paint Kit##wpk", &skin.paintKit);
                if (skin.paintKit < 0) skin.paintKit = 0;
                SynthSep(); EvoSliderFloat("Wear##ww", &skin.wear, 0.f, 1.f, "%.4f");
                SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputInt("StatTrak##wst", &skin.statTrak);
            }
        }
    }
    inline void Pg_Knife()
    {
        static const char* kn[SkinChanger::kKnifeCount]{};
        static bool knInit = false;
        if (!knInit) { for (int i = 0; i < SkinChanger::kKnifeCount; ++i) kn[i] = SkinChanger::kKnives[i].name; knInit = true; }
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Combo("Model##km", &SkinChanger::cfg.knifeModel, kn, SkinChanger::kKnifeCount))
            SkinChanger::ForceFullUpdate();
        SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputInt("Paint Kit##kpk", &SkinChanger::cfg.knifePaintKit);
        if (SkinChanger::cfg.knifePaintKit < 0) SkinChanger::cfg.knifePaintKit = 0;
        SynthSep(); EvoSliderFloat("Wear##kw", &SkinChanger::cfg.knifeWear, 0.f, 1.f, "%.4f");
    }
    inline void Pg_Glove()
    {
        static const char* gn[SkinChanger::kGloveCount]{};
        static bool gnInit = false;
        if (!gnInit) { for (int i = 0; i < SkinChanger::kGloveCount; ++i) gn[i] = SkinChanger::kGloves[i].name; gnInit = true; }
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Combo("Model##gm", &SkinChanger::cfg.gloveModel, gn, SkinChanger::kGloveCount))
            SkinChanger::ForceFullUpdate();
        SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputInt("Paint Kit##gpk", &SkinChanger::cfg.glovePaintKit);
        if (SkinChanger::cfg.glovePaintKit < 0) SkinChanger::cfg.glovePaintKit = 0;
        SynthSep(); EvoSliderFloat("Wear##gw", &SkinChanger::cfg.gloveWear, 0.f, 1.f, "%.4f");
    }

    // ── WLD tab ────────────────────────────────────────────────
    inline void Pg_Sky()
    {
        EvoCheckbox("Sky Override##so", &WorldEffects::cfg.skyEnabled);
        if (WorldEffects::cfg.skyEnabled) {
            SynthSep(); EvoCheckbox("Rainbow Sky##rs", &WorldEffects::cfg.skyRainbow);
            SynthSep();
            if (WorldEffects::cfg.skyRainbow)
                EvoSliderFloat("Speed##rss", &WorldEffects::cfg.skyRainbowSpeed, 0.05f, 2.f, "%.2f");
            else {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::ColorEdit4("Sky Color##scc", WorldEffects::cfg.skyColor, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
            }
            SynthSep(); EvoSliderFloat("Brightness##sb", &WorldEffects::cfg.skyBrightness, 0.1f, 5.f, "%.1f");
        }
    }
    inline void Pg_FlashSmoke()
    {
        EvoCheckbox("No Flash##nf", &WorldEffects::cfg.noFlash);
        if (WorldEffects::cfg.noFlash) { SynthSep(); EvoSliderFloat("Max Alpha##mfa", &WorldEffects::cfg.maxFlashAlpha, 0.f, 255.f, "%.0f"); }
        SynthSep(); EvoCheckbox("No Smoke##ns", &WorldEffects::cfg.noSmoke);
        SynthSep(); EvoCheckbox("Smoke Color##smoC", &WorldEffects::cfg.smokeColor);
        if (WorldEffects::cfg.smokeColor) {
            SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Smoke##st", WorldEffects::cfg.smokeCol, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
        }
    }
    inline void Pg_Fire()
    {
        EvoCheckbox("Fire Color##fco", &WorldEffects::cfg.fireColor);
        if (WorldEffects::cfg.fireColor) {
            SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit4("Fire##fc", WorldEffects::cfg.fireCol, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
        }
    }
    inline void Pg_FOV()
    {
        EvoCheckbox("FOV Override##fovc", &WorldEffects::cfg.fovEnabled);
        if (WorldEffects::cfg.fovEnabled) { SynthSep(); EvoSliderFloat("FOV Value##fv", &WorldEffects::cfg.fovValue, 60.f, 150.f, "%.0f"); }
    }
    inline void Pg_NightAsus()
    {
        const char* nm[] = { "Off","Night","Midnight","Sunset","Blood Moon","Aurora","Cyberpunk","Vaporwave","Hellfire" };
        EvoCombo("Night Mode##nm", &WorldEffects::cfg.nightMode, nm, IM_ARRAYSIZE(nm));
        SynthSep();
        const char* am[] = { "Off","Lime","Hot Pink","Cyan","Red","Yellow" };
        EvoCombo("Asus Mode##am", &WorldEffects::cfg.asusMode, am, IM_ARRAYSIZE(am));
    }
    inline void Pg_Visibility()
    {
        EvoCheckbox("Fullbright##fbm",          &WorldEffects::cfg.fullbright);
        SynthSep(); EvoCheckbox("Anti-Fog##af", &WorldEffects::cfg.antiFog);
        SynthSep(); EvoCheckbox("No Shadows##nsh", &WorldEffects::cfg.noShadows);
        SynthSep(); EvoCheckbox("No Color Correction##ncc", &WorldEffects::cfg.noColorCorrection);
        SynthSep(); EvoCheckbox("Custom Fog##cf", &WorldEffects::cfg.fogEnabled);
        if (WorldEffects::cfg.fogEnabled) {
            SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit3("Fog Color##fgc", WorldEffects::cfg.fogColor, ImGuiColorEditFlags_NoInputs);
            SynthSep(); EvoSliderFloat("Start##fs",   &WorldEffects::cfg.fogStart,    0.f,    2000.f, "%.0f");
            SynthSep(); EvoSliderFloat("End##fe",     &WorldEffects::cfg.fogEnd,    500.f,   20000.f, "%.0f");
            SynthSep(); EvoSliderFloat("Density##fd", &WorldEffects::cfg.fogDensity,  0.f,       1.f, "%.2f");
        }
        SynthSep(); EvoCheckbox("Brightness Override##bo", &WorldEffects::cfg.brightnessEnabled);
        if (WorldEffects::cfg.brightnessEnabled) {
            SynthSep(); EvoSliderFloat("Min Exp##me", &WorldEffects::cfg.exposureMin, 0.1f, 5.f, "%.2f");
            SynthSep(); EvoSliderFloat("Max Exp##xe", &WorldEffects::cfg.exposureMax, 0.1f, 5.f, "%.2f");
            SynthSep(); if (EvoButton("Fullbright##fb")) WorldEffects::cfg.exposureMin = WorldEffects::cfg.exposureMax = 3.f;
        }
    }
    inline void Pg_ThirdPerson()
    {
        EvoSliderFloat("Distance##tpd", &WorldEffects::cfg.thirdPersonDist, 50.f, 600.f, "%.0f");
    }
    inline void Pg_Wireframe()
    {
        EvoCheckbox("Hands Only##wh", &WireframeHands::cfg.handsOnly);
        SynthSep(); ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::ColorEdit4("Wire Color##wc", WireframeHands::cfg.color, ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar);
    }
    inline void Pg_AutoAccept()
    {
        EvoSliderFloat("Delay##aad", &AutoAccept::cfg.delay, 0.1f, 3.f, "%.1f");
    }

    // ── CFG tab ────────────────────────────────────────────────
    inline void Pg_UIMode()  { EvoCheckbox("Advanced Mode", &advancedMode); }
    inline void Pg_Presets()
    {
        ImGui::TextColored({ 0.4f,1.f,0.4f,1.f }, "  Safe");
        ImGui::Dummy({ 0.f, 2.f });
        if (EvoButton("Undetected##pu"))  ApplyPreset_Undetected();
        SynthSep(); if (EvoButton("Legit Aim##pla"))  ApplyPreset_LegitAim();
        ImGui::Dummy({ 0.f, 4.f }); ImGui::TextColored({ 1.f,0.7f,0.3f,1.f }, "  Risky"); ImGui::Dummy({ 0.f, 2.f });
        if (EvoButton("Aggressive##pal")) ApplyPreset_SilentAim();
        ImGui::Dummy({ 0.f, 4.f }); ImGui::TextColored({ 1.f,0.4f,0.4f,1.f }, "  Danger"); ImGui::Dummy({ 0.f, 2.f });
        if (EvoButton("Rage##pr"))        ApplyPreset_Rage();
        SynthSep(); if (EvoButton("All Off##pao")) ApplyPreset_Off();
    }
    inline void Pg_Accent()
    {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::ColorEdit4("##pri", primaryColor,
                ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar|ImGuiColorEditFlags_PickerHueWheel))
            themeApplied = false;
    }
    inline void Pg_HUDStyle()
    {
        const char* huds[] = { "Pill", "Clean", "Ghost" };
        EvoCombo("Style##huds", &Menu::hudStyle, huds, 3);
    }
    inline void Pg_Configs()
    {
        LoadSlotNames();
        for (int i = 0; i < kMaxSlots; ++i) {
            ImGui::PushID(i);
            bool exists = SlotExists(i);
            ImGui::TextColored(exists ? ImVec4{0.80f,0.90f,0.80f,1.f} : ImVec4{0.40f,0.42f,0.44f,1.f},
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
    }

    // ── Feature catalog ───────────────────────────────────────
    struct FeatureDef {
        const char* name;
        const char* subtitle;
        int         icon;
        bool*       enabled;       // nullptr = no toggle
        void      (*render)();
    };

    inline FeatureDef kFeat_Aim[] = {
        { "Aimbot",     "Targeting & feel",     FI_CROSSHAIR, &Aimbot::cfg.enabled,        Pg_Aimbot     },
        { "Rage Mode",  "Wallbang / silent / lowHP", FI_BOLT, &Aimbot::Rage::cfg.enabled,  Pg_Rage       },
        { "Triggerbot", "Auto-fire on target",  FI_TRIGGER,   &Triggerbot::cfg.enabled,    Pg_Triggerbot },
        { "Bunny Hop",  "Auto bhop & strafe",   FI_JUMP,      &Bhop::cfg.enabled,          Pg_Bhop       },
        { "Backtrack",  "Rewind enemy ticks",   FI_REWIND,    &Backtrack::cfg.enabled,     Pg_Backtrack  },
        { "Anti-Aim",   "Pitch & yaw modes",    FI_ROTATE,    &AntiAim::cfg.enabled,       Pg_AntiAim    },
        { "Fake Lag",   "Choke outgoing pkts",  FI_BOLT,      &FakeLag::cfg.enabled,       Pg_FakeLag    },
    };
    inline FeatureDef kFeat_Vis[] = {
        { "ESP",         "Player visuals",       FI_BOX,        &ESP::cfg.enabled,             Pg_ESP         },
        { "Chams",       "Material overrides",   FI_SILHOUETTE, &Chams::cfg.enabled,           Pg_Chams       },
        { "Tracers",     "Bullet trails",        FI_TRACER,     &BulletTracer::cfg.enabled,    Pg_Tracers     },
        { "Damage Ind.", "Hitmarker indicator",  FI_DROP,       &DamageIndicator::cfg.enabled, Pg_DamageInd   },
        { "Spectators",  "Who's watching you",   FI_EYE,        &ESP::cfg.spectators,          Pg_Spectators  },
        { "Rank Reveal", "Show enemy ranks",     FI_BADGE,      &RankRevealer::cfg.enabled,    Pg_RankReveal  },
        { "Nade Predict","Trajectory preview",   FI_GRENADE,    &GrenadePrediction::cfg.enabled,Pg_NadePred  },
        { "Nade Helper", "Auto-aim grenades",    FI_TARGET,     &NadeHelper::cfg.enabled,      Pg_NadeHelper  },
        { "Sound ESP",   "Footstep markers",     FI_SPEAKER,    &SoundESP::cfg.enabled,        Pg_SoundESP    },
    };
    inline FeatureDef kFeat_Skn[] = {
        { "Skin Changer","Weapon paintkits",     FI_PAINT, &SkinChanger::cfg.enabled,      Pg_SkinChanger },
        { "Knife",       "Custom knife model",   FI_KNIFE, &SkinChanger::cfg.knifeEnabled, Pg_Knife       },
        { "Gloves",      "Custom glove model",   FI_GLOVE, &SkinChanger::cfg.gloveEnabled, Pg_Glove       },
    };
    inline FeatureDef kFeat_Wld[] = {
        { "Sky",         "Sky color & rainbow",  FI_SUN,    &WorldEffects::cfg.skyEnabled,    Pg_Sky         },
        { "Flash/Smoke", "No flash & smoke",     FI_BULB,   &WorldEffects::cfg.noFlash,       Pg_FlashSmoke  },
        { "Fire",        "Molotov color",        FI_FLAME,  &WorldEffects::cfg.fireColor,     Pg_Fire        },
        { "FOV",         "Field of view",        FI_FOV,    &WorldEffects::cfg.fovEnabled,    Pg_FOV         },
        { "Atmosphere",  "Night & ASUS modes",   FI_MOON,   nullptr,                          Pg_NightAsus   },
        { "Visibility",  "Fullbright, fog, exp", FI_EYE,    &WorldEffects::cfg.fullbright,    Pg_Visibility  },
        { "Third Person","3P camera view",       FI_CAMERA, &WorldEffects::cfg.thirdPerson,   Pg_ThirdPerson },
        { "Wireframe",   "Hands wireframe view", FI_BOX,    &WireframeHands::cfg.enabled,     Pg_Wireframe   },
        { "Auto-Accept", "Auto match accept",    FI_CHECK,  &AutoAccept::cfg.enabled,         Pg_AutoAccept  },
    };
    inline FeatureDef kFeat_Cfg[] = {
        { "UI Mode",     "Advanced controls",    FI_GEAR,    &advancedMode, Pg_UIMode   },
        { "Presets",     "Quick-apply configs",  FI_WAND,    nullptr,       Pg_Presets  },
        { "Accent",      "Theme accent color",   FI_PALETTE, nullptr,       Pg_Accent   },
        { "HUD Style",   "Top-right HUD layout", FI_HUD,     nullptr,       Pg_HUDStyle },
        { "Configs",     "Save & load slots",    FI_FLOPPY,  nullptr,       Pg_Configs  },
    };

    inline FeatureDef* GetTabFeatures(int tab, int& count)
    {
        switch (tab) {
            case 0: count = (int)IM_ARRAYSIZE(kFeat_Aim); return kFeat_Aim;
            case 1: count = (int)IM_ARRAYSIZE(kFeat_Vis); return kFeat_Vis;
            case 2: count = (int)IM_ARRAYSIZE(kFeat_Skn); return kFeat_Skn;
            case 3: count = (int)IM_ARRAYSIZE(kFeat_Wld); return kFeat_Wld;
            case 4: count = (int)IM_ARRAYSIZE(kFeat_Cfg); return kFeat_Cfg;
        }
        count = 0; return nullptr;
    }

    // ── Card renderer ────────────────────────────────────────
    //  Submits OPTIONS + toggle as REAL items (so clicks register).
    //  Hover for the whole card uses a non-blocking rect-test.
    inline bool DrawFeatureCard(const FeatureDef& f, ImVec2 sz, float dt)
    {
        ImDrawList* dl   = ImGui::GetWindowDrawList();
        const ImVec2 p0  = ImGui::GetCursorScreenPos();
        const ImVec2 p1  = { p0.x + sz.x, p0.y + sz.y };
        const float  R   = 13.f;
        const bool   isOn = (f.enabled && *f.enabled);

        // Non-blocking hover detection (does NOT consume clicks)
        const bool hovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(p0, p1);
        const float hAnim  = AnimStep(&f, hovered, 16.f, dt);

        // ── shadow (lifts on hover) ──
        for (int s = 0; s < 4; ++s) {
            float ofs   = 2.0f + (float)s * 2.4f + hAnim * 2.5f;
            int   alpha = (int)((26 - s * 6 + hAnim * 12) * menuAlpha);
            if (alpha <= 0) continue;
            dl->AddRectFilled(
                { p0.x - 1.f, p0.y + ofs },
                { p1.x + 1.f, p1.y + ofs },
                IM_COL32(0, 0, 0, alpha), R + 2.f);
        }

        // ── base fill ──
        ImU32 baseFill = IM_COL32(
            19 + (int)(hAnim * 5),
            17 + (int)(hAnim * 5),
            29 + (int)(hAnim * 7),
            (int)((220 + hAnim * 25) * menuAlpha));
        dl->AddRectFilled(p0, p1, baseFill, R);

        // ── enabled accent wash (smooth radial centred on icon, no hard line) ──
        if (isOn) {
            float cx = (p0.x + p1.x) * 0.5f;
            float cy = p0.y + 44.f;
            float maxR = sz.x * 0.85f;
            // Many faint concentric rings → smooth gradient fall-off
            for (int g = 0; g < 14; ++g) {
                float t  = (float)g / 13.f;          // 0..1
                float rr = maxR * (0.18f + t * 0.82f);
                int   ga = (int)((28.f * (1.f - t) * (1.f - t)) * menuAlpha);
                if (ga <= 0) continue;
                dl->AddCircleFilled({ cx, cy }, rr, EvoAccent(ga), 40);
            }
        }

        // ── glossy top sliver ── two halves that fade INTO the center,
        //  brighter near the corners, dimmer in the middle (≈25% less alpha).
        {
            const int   midA = (int)(58 * menuAlpha);   // was 78 (≈ -25%)
            const float yA   = p0.y + 1.f;
            const float yB   = p0.y + 2.4f;
            const float xL   = p0.x + 16.f;
            const float xR   = p1.x - 16.f;
            const float xMid = (xL + xR) * 0.5f;
            // left half: 0 → midA → 0
            dl->AddRectFilledMultiColor({ xL, yA }, { xMid, yB },
                IM_COL32(255,255,255,0),       IM_COL32(255,255,255,midA),
                IM_COL32(255,255,255,midA),    IM_COL32(255,255,255,0));
            // right half: midA → 0 → 0 → midA  (mirror)
            dl->AddRectFilledMultiColor({ xMid, yA }, { xR, yB },
                IM_COL32(255,255,255,midA),    IM_COL32(255,255,255,0),
                IM_COL32(255,255,255,0),       IM_COL32(255,255,255,midA));
        }

        // ── outline ──
        ImU32 outline = isOn
            ? EvoAccent((int)((140 + hAnim * 60) * menuAlpha))
            : IM_COL32(50 + (int)(hAnim * 28),
                       48 + (int)(hAnim * 28),
                       76 + (int)(hAnim * 44),
                       (int)((180 + hAnim * 50) * menuAlpha));
        dl->AddRect(p0, p1, outline, R, 0, 1.f);
        // hairline
        dl->AddRect({ p0.x + 0.5f, p0.y + 0.5f }, { p1.x - 0.5f, p1.y - 0.5f },
            IM_COL32(255, 255, 255, (int)(menuAlpha * (10 + hAnim * 6))), R - 0.5f, 0, 1.f);

        // ── icon plate (centred more — sits a bit lower) ──
        const float iconR = 23.f;
        ImVec2 iconC{ (p0.x + p1.x) * 0.5f, p0.y + 44.f };

        if (isOn) {
            // Smooth pulse halo (many faint rings, no banding)
            float pulse = 0.5f + 0.5f * sinf((float)ImGui::GetTime() * 2.4f);
            for (int g = 0; g < 8; ++g) {
                float t  = (float)g / 7.f;
                float gr = iconR + 2.f + t * (14.f + pulse * 4.f);
                int   ga = (int)((34.f * (1.f - t) * (1.f - t)) * menuAlpha);
                if (ga <= 0) continue;
                dl->AddCircleFilled(iconC, gr, EvoAccent(ga), 36);
            }
        }
        ImU32 plateFill = isOn
            ? EvoAccent((int)((58 + hAnim * 22) * menuAlpha))
            : IM_COL32(28, 26, 42, (int)((190 + hAnim * 30) * menuAlpha));
        dl->AddCircleFilled(iconC, iconR, plateFill, 36);
        dl->AddCircle(iconC, iconR,
            isOn ? EvoAccent((int)(225 * menuAlpha))
                 : IM_COL32(64, 60, 92, (int)(210 * menuAlpha)),
            36, 1.f);

        ImU32 iconCol = isOn
            ? IM_COL32(255, 255, 255, (int)(248 * menuAlpha))
            : IM_COL32(178, 178, 200, (int)((220 + hAnim * 30) * menuAlpha));
        DrawFeatureIcon(dl, f.icon, iconC, 13.5f, iconCol);

        // ── name + subtitle (subtitle wrapped in a soft pill badge) ──
        ImVec2 nmSz   = ImGui::CalcTextSize(f.name);
        const float nameY = iconC.y + iconR + 11.f;
        dl->AddText({ (p0.x + p1.x - nmSz.x) * 0.5f, nameY },
            IM_COL32(238, 238, 248, (int)(248 * menuAlpha)), f.name);
        if (f.subtitle && *f.subtitle) {
            ImVec2 stSz = ImGui::CalcTextSize(f.subtitle);
            const float padX = 7.f, padY = 1.f;             // ~10% smaller chip
            float chipY0 = nameY + nmSz.y + 4.f;
            float chipY1 = chipY0 + stSz.y + padY * 2.f;
            float chipX0 = (p0.x + p1.x - stSz.x) * 0.5f - padX;
            float chipX1 = (p0.x + p1.x + stSz.x) * 0.5f + padX;
            float chipR  = (chipY1 - chipY0) * 0.5f;
            // chip background
            ImU32 chipFill = isOn
                ? EvoAccent((int)((26 + hAnim * 8) * menuAlpha))
                : IM_COL32(30, 28, 46, (int)((155 + hAnim * 25) * menuAlpha));
            dl->AddRectFilled({ chipX0, chipY0 }, { chipX1, chipY1 }, chipFill, chipR);
            // chip border
            ImU32 chipEdge = isOn
                ? EvoAccent((int)((110 + hAnim * 40) * menuAlpha))
                : IM_COL32(70, 66, 100, (int)((150 + hAnim * 50) * menuAlpha));
            dl->AddRect({ chipX0, chipY0 }, { chipX1, chipY1 }, chipEdge, chipR, 0, 1.f);
            // text — nudged up 2px so it sits visually centred (font baseline bias)
            dl->AddText({ chipX0 + padX, chipY0 + padY - 2.f },
                isOn ? IM_COL32(232, 230, 245, (int)(232 * menuAlpha))
                     : IM_COL32(150, 150, 170, (int)(218 * menuAlpha)),
                f.subtitle);
        }

        // ── bottom action row ──
        const float rowH  = 28.f;
        const float rowY0 = p1.y - rowH - 9.f;
        const float rowY1 = p1.y - 9.f;
        const float rowMx = (p0.x + p1.x) * 0.5f;
        const bool  hasToggle = (f.enabled != nullptr);
        bool clickedOptions = false;

        // Use a stable ID scope per-card so children don't collide across cards
        ImGui::PushID((const void*)&f);

        // OPTIONS button — left half if there's also a toggle; centred-wide otherwise
        {
            ImVec2 bTL, bBR;
            if (hasToggle) {
                bTL = { p0.x + 9.f, rowY0 };
                bBR = { rowMx - 4.f, rowY1 };
            } else {
                float w = (p1.x - p0.x) * 0.6f;
                float cx = (p0.x + p1.x) * 0.5f;
                bTL = { cx - w * 0.5f, rowY0 };
                bBR = { cx + w * 0.5f, rowY1 };
            }
            ImGui::SetCursorScreenPos(bTL);
            bool clicked = ImGui::InvisibleButton("opt", { bBR.x - bTL.x, bBR.y - bTL.y });
            bool bh      = ImGui::IsItemHovered();
            bool bd      = ImGui::IsItemActive();
            float bA     = AnimStep((const void*)((const char*)&f + 1), bh, 18.f, dt);
            float pressShift = bd ? 1.f : 0.f;
            ImVec2 dTL{ bTL.x, bTL.y + pressShift };
            ImVec2 dBR{ bBR.x, bBR.y + pressShift };
            ImU32 bFill = IM_COL32(30 + (int)(bA * 10), 28 + (int)(bA * 10),
                46 + (int)(bA * 16), (int)((210 + bA * 30) * menuAlpha));
            dl->AddRectFilled(dTL, dBR, bFill, 7.f);
            dl->AddRect(dTL, dBR,
                IM_COL32(64, 60, 92, (int)((180 + bA * 60) * menuAlpha)), 7.f, 0, 1.f);
            const char* lbl = "OPTIONS";
            ImVec2 lSz = ImGui::CalcTextSize(lbl);
            // No chevron — clean centred label only
            dl->AddText({ (dTL.x + dBR.x - lSz.x) * 0.5f, (dTL.y + dBR.y - lSz.y) * 0.5f },
                IM_COL32(220, 220, 235, (int)(228 * menuAlpha)), lbl);
            if (clicked) clickedOptions = true;
        }

        // Toggle pill (right half)
        if (f.enabled)
        {
            ImVec2 tTL{ rowMx + 4.f, rowY0 };
            ImVec2 tBR{ p1.x - 9.f, rowY1 };
            ImGui::SetCursorScreenPos(tTL);
            bool clicked = ImGui::InvisibleButton("tog", { tBR.x - tTL.x, tBR.y - tTL.y });
            bool bh      = ImGui::IsItemHovered();
            bool bd      = ImGui::IsItemActive();
            if (clicked) *f.enabled = !*f.enabled;

            float onA = AnimStep((const void*)((const char*)&f + 2), *f.enabled, 14.f, dt);
            float pressShift = bd ? 1.f : 0.f;
            ImVec2 dTL{ tTL.x, tTL.y + pressShift };
            ImVec2 dBR{ tBR.x, tBR.y + pressShift };
            // off → muted red, on → primaryColor
            int rR = 132 + (int)((((primaryColor[0] * 255) - 132) * onA));
            int gG =  42 + (int)((((primaryColor[1] * 255) -  42) * onA));
            int bB =  54 + (int)((((primaryColor[2] * 255) -  54) * onA));
            int aA = (int)((215 + (bh ? 25 : 0)) * menuAlpha);
            ImU32 fill = IM_COL32(rR, gG, bB, aA);
            dl->AddRectFilled(dTL, dBR, fill, 7.f);
            // glossy sliver on top
            dl->AddRectFilledMultiColor(
                { dTL.x + 8.f, dTL.y + 1.f }, { dBR.x - 8.f, dTL.y + 2.4f },
                IM_COL32(255,255,255,0),
                IM_COL32(255,255,255,(int)(90 * menuAlpha)),
                IM_COL32(255,255,255,(int)(90 * menuAlpha)),
                IM_COL32(255,255,255,0));
            dl->AddRect(dTL, dBR,
                IM_COL32(255, 255, 255, (int)((34 + bh * 30) * menuAlpha)), 7.f, 0, 1.f);

            // small dot indicator (slides from left=off to right=on)
            float dotR  = 4.f;
            float padX  = 9.f;
            float dotX  = dTL.x + padX + dotR + (dBR.x - dTL.x - padX*2.f - dotR*2.f) * onA;
            float dotY  = (dTL.y + dBR.y) * 0.5f;
            dl->AddCircleFilled({ dotX, dotY }, dotR, IM_COL32(255,255,255,(int)(245*menuAlpha)), 14);
            dl->AddCircle      ({ dotX, dotY }, dotR + 0.6f, IM_COL32(0,0,0,(int)(60*menuAlpha)), 14, 1.f);

            const char* lbl = *f.enabled ? "ON" : "OFF";
            ImVec2 lSz = ImGui::CalcTextSize(lbl);
            // place label opposite to dot
            float lblX = *f.enabled
                ? dTL.x + padX + 4.f
                : dBR.x - padX - 4.f - lSz.x;
            dl->AddText({ lblX, (dTL.y + dBR.y - lSz.y) * 0.5f },
                IM_COL32(255, 255, 255, (int)(245 * menuAlpha)), lbl);
        }

        ImGui::PopID();

        // restore cursor to bottom of card (so layout flows)
        ImGui::SetCursorScreenPos({ p0.x, p1.y });
        return clickedOptions;
    }

    // ── Grid renderer (with slide-in animation) ──────────────
    inline void DrawFeatureGrid(int tab, float dt, ImVec2 areaSz)
    {
        int count = 0;
        FeatureDef* feats = GetTabFeatures(tab, count);
        if (!feats || count == 0) return;

        const int   COLS    = 3;
        const float COL_GAP = 11.f;
        const float ROW_GAP = 11.f;
        const float CARD_H  = 168.f;
        const float CARD_W  = (areaSz.x - COL_GAP * (COLS - 1)) / (float)COLS;

        // Slide offset: when leaving page (pageDir=-1), grid slides in from left.
        // When loading tab (no transition needed) anim is 1.
        float slide = 0.f;
        if (pageAnim < 1.f && tab == pageAnimTab && pageDir < 0) {
            float u = 1.f - pageAnim;
            // ease-out
            u = 1.f - (1.f - u) * (1.f - u);
            slide = -u * areaSz.x * 0.35f;
        }

        ImVec2 origin = ImGui::GetCursorScreenPos();
        for (int i = 0; i < count; ++i) {
            int col = i % COLS;
            int row = i / COLS;
            // staggered fade: each card fades in slightly later
            float cardDelay = (float)(row * 0.04f + col * 0.02f);
            float cardA = pageAnim - cardDelay;
            if (cardA < 0.f) cardA = 0.f;
            if (cardA > 1.f) cardA = 1.f;
            ImVec2 cp{ origin.x + col * (CARD_W + COL_GAP) + slide,
                       origin.y + row * (CARD_H + ROW_GAP) };
            ImGui::SetCursorScreenPos(cp);
            if (DrawFeatureCard(feats[i], { CARD_W, CARD_H }, dt)) {
                prevPage[tab]  = pageStack[tab];
                pageStack[tab] = i;
                pageAnim       = 0.f;
                pageAnimTab    = tab;
                pageDir        = +1;
            }
        }
        // Tell parent window the real content extent (we used SetCursorScreenPos
        // to lay out cards absolutely; without this ImGui logs a debug warning).
        int rows = (count + COLS - 1) / COLS;
        ImGui::SetCursorScreenPos(origin);
        ImGui::Dummy({ areaSz.x, rows * CARD_H + (rows - 1) * ROW_GAP });
    }

    // ── Feature page renderer ────────────────────────────────
    inline void DrawFeaturePage(int tab, float dt, ImVec2 areaSz)
    {
        int count = 0;
        FeatureDef* feats = GetTabFeatures(tab, count);
        if (!feats || pageStack[tab] < 0 || pageStack[tab] >= count) {
            pageStack[tab] = -1; return;
        }
        const FeatureDef& f = feats[pageStack[tab]];
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Slide-in offset (page enters from right)
        float slide = 0.f;
        if (pageAnim < 1.f && tab == pageAnimTab && pageDir > 0) {
            float u = 1.f - pageAnim;
            u = 1.f - (1.f - u) * (1.f - u);
            slide = u * areaSz.x * 0.35f;
        }
        ImVec2 origin = ImGui::GetCursorScreenPos();
        ImVec2 baseOrigin = origin;
        origin.x += slide;
        ImGui::SetCursorScreenPos(origin);

        // ── header card ──
        const float HDR = 46.f;
        ImVec2 hTL = origin;
        ImVec2 hBR = { origin.x + areaSz.x, origin.y + HDR };
        // header background
        dl->AddRectFilled(hTL, hBR,
            IM_COL32(20, 18, 32, (int)(220 * menuAlpha)), 12.f);
        dl->AddRect(hTL, hBR,
            IM_COL32(54, 50, 80, (int)(180 * menuAlpha)), 12.f, 0, 1.f);
        // glossy sliver
        dl->AddRectFilledMultiColor(
            { hTL.x + 16.f, hTL.y + 1.f }, { hBR.x - 16.f, hTL.y + 2.6f },
            IM_COL32(255,255,255,0),
            IM_COL32(255,255,255,(int)(75 * menuAlpha)),
            IM_COL32(255,255,255,(int)(75 * menuAlpha)),
            IM_COL32(255,255,255,0));

        // back button
        ImVec2 bTL{ hTL.x + 8.f, hTL.y + 8.f };
        ImVec2 bBR{ hTL.x + 78.f, hBR.y - 8.f };
        ImGui::SetCursorScreenPos(bTL);
        bool backClicked = ImGui::InvisibleButton("##back", { bBR.x - bTL.x, bBR.y - bTL.y });
        bool bh = ImGui::IsItemHovered();
        bool bd = ImGui::IsItemActive();
        float bA = AnimStep((const void*)"##back", bh, 18.f, dt);
        float ps = bd ? 1.f : 0.f;
        ImVec2 dTL{ bTL.x, bTL.y + ps }, dBR{ bBR.x, bBR.y + ps };
        dl->AddRectFilled(dTL, dBR,
            IM_COL32(30 + (int)(bA*10), 28 + (int)(bA*10), 46 + (int)(bA*16),
                (int)((210 + bA*30) * menuAlpha)), 7.f);
        dl->AddRect(dTL, dBR,
            IM_COL32(64, 60, 92, (int)((180 + bA*60) * menuAlpha)), 7.f, 0, 1.f);
        // chevron + label
        float ax = dTL.x + 14.f, ay = (dTL.y + dBR.y) * 0.5f;
        dl->AddLine({ ax + 5.f, ay - 4.5f }, { ax, ay }, IM_COL32(220,220,235,(int)(230*menuAlpha)), 1.6f);
        dl->AddLine({ ax + 5.f, ay + 4.5f }, { ax, ay }, IM_COL32(220,220,235,(int)(230*menuAlpha)), 1.6f);
        dl->AddText({ ax + 14.f, ay - ImGui::GetFontSize() * 0.5f },
            IM_COL32(220, 220, 235, (int)(232 * menuAlpha)), "BACK");

        if (backClicked) {
            prevPage[tab] = pageStack[tab];
            pageStack[tab] = -1;
            pageAnim = 0.f;
            pageAnimTab = tab;
            pageDir = -1;
        }

        // mini icon plate + name (next to back button)
        bool isOn = (f.enabled && *f.enabled);
        float ix  = bBR.x + 14.f;
        float iy  = (hTL.y + hBR.y) * 0.5f;
        float pR  = 13.f;
        ImU32 plateFill = isOn
            ? EvoAccent((int)(58 * menuAlpha))
            : IM_COL32(28, 26, 42, (int)(200 * menuAlpha));
        dl->AddCircleFilled({ ix + pR, iy }, pR, plateFill, 24);
        dl->AddCircle({ ix + pR, iy }, pR,
            isOn ? EvoAccent((int)(220 * menuAlpha))
                 : IM_COL32(64, 60, 92, (int)(200 * menuAlpha)), 24, 1.f);
        DrawFeatureIcon(dl, f.icon, { ix + pR, iy }, 8.f,
            isOn ? IM_COL32(255,255,255,(int)(245*menuAlpha))
                 : IM_COL32(180,180,200,(int)(225*menuAlpha)));

        // name + subtitle
        float txtX = ix + pR * 2.f + 12.f;
        dl->AddText({ txtX, iy - ImGui::GetFontSize() - 1.f },
            IM_COL32(238, 238, 248, (int)(248 * menuAlpha)), f.name);
        if (f.subtitle && *f.subtitle) {
            dl->AddText({ txtX, iy + 2.f },
                IM_COL32(126, 126, 146, (int)(210 * menuAlpha)), f.subtitle);
        }

        // status pill on right
        if (f.enabled) {
            const char* st = *f.enabled ? "ENABLED" : "DISABLED";
            ImVec2 sSz = ImGui::CalcTextSize(st);
            float sw = sSz.x + 18.f, sh = 22.f;
            ImVec2 sTL{ hBR.x - sw - 10.f, iy - sh * 0.5f };
            ImVec2 sBR{ hBR.x - 10.f, iy + sh * 0.5f };
            int rR2 = *f.enabled ? (int)(primaryColor[0] * 255) : 132;
            int gG2 = *f.enabled ? (int)(primaryColor[1] * 255) :  42;
            int bB2 = *f.enabled ? (int)(primaryColor[2] * 255) :  54;
            dl->AddRectFilled(sTL, sBR, IM_COL32(rR2, gG2, bB2, (int)(220 * menuAlpha)), 6.f);
            dl->AddRect(sTL, sBR, IM_COL32(255,255,255,(int)(34*menuAlpha)), 6.f, 0, 1.f);
            dl->AddText({ (sTL.x + sBR.x - sSz.x) * 0.5f, (sTL.y + sBR.y - sSz.y) * 0.5f },
                IM_COL32(255,255,255,(int)(245*menuAlpha)), st);
        }

        // ── content card ──
        ImVec2 cTL{ origin.x, hBR.y + 10.f };
        ImVec2 cBR{ origin.x + areaSz.x, origin.y + areaSz.y };
        // bg
        dl->AddRectFilled(cTL, cBR,
            IM_COL32(18, 16, 28, (int)(180 * menuAlpha)), 12.f);
        dl->AddRect(cTL, cBR,
            IM_COL32(50, 46, 74, (int)(150 * menuAlpha)), 12.f, 0, 1.f);
        dl->AddRectFilledMultiColor(
            { cTL.x + 16.f, cTL.y + 1.f }, { cBR.x - 16.f, cTL.y + 2.4f },
            IM_COL32(255,255,255,0),
            IM_COL32(255,255,255,(int)(60 * menuAlpha)),
            IM_COL32(255,255,255,(int)(60 * menuAlpha)),
            IM_COL32(255,255,255,0));

        ImGui::SetCursorScreenPos({ cTL.x + 4.f, cTL.y + 4.f });
        ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 14.f, 12.f });
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { 8.f, 6.f });
        if (ImGui::BeginChild("##fpgcnt",
                { areaSz.x - 8.f, areaSz.y - HDR - 18.f }, false))
        {
            ImGuiErrorRecoveryState rs;
            ImGui::ErrorRecoveryStoreState(&rs);
            __try { f.render(); } __except (EXCEPTION_EXECUTE_HANDLER) {}
            ImGui::ErrorRecoveryTryToRecoverState(&rs);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();

        // Reserve the full page area in the parent so SetCursorScreenPos
        // calls above don't trip ImGui's "extending window boundaries" assert.
        ImGui::SetCursorScreenPos(baseOrigin);
        ImGui::Dummy({ areaSz.x, areaSz.y });
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

        // ----------------------------------------------------------------
        //  LAYOUT  (v2 — bottom dock)
        //  Whole menu scaled ~7%. Sidebar is gone; tab strip lives in a
        //  centered "iOS island" dock floating below the content card.
        // ----------------------------------------------------------------
        const float W        = 880.f;            // wider for 3-col grid
        const float DOCK_H   = 48.f;
        const float DOCK_GAP = 14.f;
        const float PAD      = 12.f;             // 15 → 12
        const float HDR_H    = 34.f;             // header strip height
        const float COL_Y    = HDR_H + 6.f;      // content starts below header
        // content height stops above the dock; full window height adds room for it
        const float CONTENT_H = 560.f;           // taller for 3-row grid
        const float H        = COL_Y + CONTENT_H + DOCK_GAP + DOCK_H + PAD + 6.f;
        const float COL_X    = PAD;
        const float COL_W    = (W - PAD * 2.f - 8.f) * 0.5f;
        const float COL_H    = CONTENT_H;

        ImGui::SetNextWindowSize({ W, H }, ImGuiCond_Once);
        ImGui::SetNextWindowPos({ 120.f, 90.f }, ImGuiCond_Once);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    { 0.f, 0.f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,            menuAlpha);

        ImGui::Begin("##lucid19", nullptr,
            ImGuiWindowFlags_NoTitleBar    | ImGuiWindowFlags_NoResize          |
            ImGuiWindowFlags_NoScrollbar   | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoCollapse    | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoBackground);

        ImVec2 wp  = ImGui::GetWindowPos();
        ImVec2 ws  = ImGui::GetWindowSize();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        const int panA = (int)(menuAlpha * 210.f);
        const int sidA = (int)(menuAlpha * 222.f);
        const int borA = (int)(menuAlpha * 175.f);

        // ----------------------------------------------------------------
        //  CONTENT CARD  — redesigned upper surface.
        //   - soft drop shadow underneath (3 stacked rounded rects)
        //   - flat tinted base (no multicolor gradient → no corner bleed)
        //   - inset top "glass" sliver
        //   - header strip with tab name + breadcrumb + accent dot
        //   - hairline divider between header and content
        // ----------------------------------------------------------------
        const float CARD_R = 16.f;
        ImVec2 panTL = { wp.x,           wp.y                     };
        ImVec2 panBR = { wp.x + ws.x,    wp.y + COL_Y + CONTENT_H + 6.f };

        // drop shadow stack
        for (int s = 0; s < 4; ++s)
        {
            float ofs   = 2.f + (float)s * 2.5f;
            int   alpha = (int)((30 - s * 6) * menuAlpha);
            if (alpha <= 0) continue;
            dl->AddRectFilled(
                { panTL.x - 1.f, panTL.y + ofs },
                { panBR.x + 1.f, panBR.y + ofs },
                IM_COL32(0, 0, 0, alpha), CARD_R + 2.f);
        }

        // base fill (single tone — no multicolor bleed past rounded corners)
        dl->AddRectFilled(panTL, panBR, IM_COL32(13, 12, 20, sidA), CARD_R);

        // inner subtle vertical lift: a slightly lighter rounded rect inset by 1px
        dl->AddRect(
            { panTL.x + 0.5f, panTL.y + 0.5f },
            { panBR.x - 0.5f, panBR.y - 0.5f },
            IM_COL32(255, 255, 255, (int)(menuAlpha * 10.f)), CARD_R - 0.5f, 0, 1.f);

        // glossy top sliver (well inside rounding, no bleed)
        dl->AddRectFilledMultiColor(
            { panTL.x + 22, panTL.y + 1.5f },
            { panBR.x - 22, panTL.y + 3.f },
            IM_COL32(255, 255, 255, 0),
            IM_COL32(255, 255, 255, (int)(78 * menuAlpha)),
            IM_COL32(255, 255, 255, (int)(78 * menuAlpha)),
            IM_COL32(255, 255, 255, 0));

        // 1px outline
        dl->AddRect(panTL, panBR, IM_COL32(46, 44, 68, borA), CARD_R, 0, 1.f);

        // ── OUTER RGB illuminating strip ABOVE the panel (subtle glow bar) ──
        {
            const float t      = (float)ImGui::GetTime();
            const float cycle  = 0.16f;
            const float inset  = CARD_R * 1.4f;
            const float swX0   = panTL.x + inset;
            const float swX1   = panBR.x - inset;
            const float swY    = panTL.y - 4.f;     // sits above the panel
            const int   segs   = 110;
            const float swW    = swX1 - swX0;
            for (int s = 0; s < segs; ++s)
            {
                float u0 = (float)s / segs, u1 = (float)(s + 1) / segs;
                float hue = fmodf(t * cycle + u0 * 0.5f, 1.f);
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.85f, 1.f, r, g, b);
                float edgeFade = 1.f;
                if (u0 < 0.18f)        edgeFade = u0 / 0.18f;
                else if (u0 > 0.82f)   edgeFade = (1.f - u0) / 0.18f;
                int aCore = (int)(180 * menuAlpha * edgeFade);
                int aGlow = (int)( 55 * menuAlpha * edgeFade);
                ImU32 cCore = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), aCore);
                ImU32 cGlow = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), aGlow);
                // soft halo above
                dl->AddRectFilled({ swX0 + u0 * swW, swY - 4.f },
                                  { swX0 + u1 * swW, swY        }, cGlow);
                // crisp 1px line
                dl->AddRectFilled({ swX0 + u0 * swW, swY        },
                                  { swX0 + u1 * swW, swY + 1.2f }, cCore);
            }
        }

        // RGB sweep along the top inner edge — mirrors the v1.5 sweep
        {
            const float t      = (float)ImGui::GetTime();
            const float cycle  = 0.16f;
            const float inset  = CARD_R * 0.55f;       // hug the corners
            const float swX0   = panTL.x + inset;
            const float swX1   = panBR.x - inset;
            const float swY    = panTL.y + 1.2f;
            const int   segs   = 96;
            const float swW    = swX1 - swX0;
            for (int s = 0; s < segs; ++s)
            {
                float u0 = (float)s / segs, u1 = (float)(s + 1) / segs;
                float hue = fmodf(t * cycle + u0 * 0.5f, 1.f);
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.85f, 1.f, r, g, b);
                // wider edge fade so corners blend rather than clip
                float edgeFade = 1.f;
                if (u0 < 0.12f)        edgeFade = u0 / 0.12f;
                else if (u0 > 0.88f)   edgeFade = (1.f - u0) / 0.12f;
                int a = (int)(170 * menuAlpha * edgeFade);
                dl->AddRectFilled(
                    { swX0 + u0 * swW, swY },
                    { swX0 + u1 * swW, swY + 1.f },
                    IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), a));
            }
        }

        // ---------- HEADER STRIP ----------
        {
            const float hX0 = panTL.x + 18.f;
            const float hX1 = panBR.x - 18.f;
            const float hY  = panTL.y + 10.f;
            const float hYB = panTL.y + HDR_H;

            // accent dot + tab name (large)
            const char* tabName = kTabLabels[activeTab];
            ImGui::PushFont(nullptr);
            ImVec2 nmSz = ImGui::CalcTextSize(tabName);
            ImGui::PopFont();

            float dotR = 3.5f;
            float dotX = hX0 + dotR;
            float dotY = hY + nmSz.y * 0.5f;
            // halo + dot
            dl->AddCircleFilled({ dotX, dotY }, dotR + 2.f, EvoAccent((int)(60 * menuAlpha)));
            dl->AddCircleFilled({ dotX, dotY }, dotR,        EvoAccent((int)(245 * menuAlpha)));

            // tab name
            float nmX = dotX + dotR + 8.f;
            dl->AddText({ nmX, hY }, IM_COL32(235, 235, 245, (int)(245 * menuAlpha)), tabName);

            // breadcrumb — small dim subtitle right after
            const char* sub = nullptr;
            switch (activeTab) {
                case 0: sub = "Aim assist & feel";        break;
                case 1: sub = "World visuals";            break;
                case 2: sub = "Skin loadout";             break;
                case 3: sub = "Movement & utility";       break;
                case 4: sub = "Profiles & system";        break;
                default: sub = "";                         break;
            }
            if (sub && *sub)
            {
                ImVec2 subSz = ImGui::CalcTextSize(sub);
                float sepX = nmX + nmSz.x + 9.f;
                // tiny vertical separator
                dl->AddRectFilled({ sepX, hY + 3.f }, { sepX + 1.f, hY + nmSz.y - 2.f },
                    IM_COL32(70, 66, 96, (int)(150 * menuAlpha)));
                dl->AddText({ sepX + 7.f, hY }, IM_COL32(130, 130, 150, (int)(200 * menuAlpha)), sub);
            }

            // (status badge removed)

            // (no divider beneath header — keep upper area clean)
        }

        // ----------------------------------------------------------------
        //  BOTTOM DOCK — iOS-style floating island with LUCID brand,
        //  horizontal tab row, and the RGB v1.5 tag.
        // ----------------------------------------------------------------
        ImGui::PushFont(nullptr); // ensure default font for size measure
        const float dockFs   = ImGui::GetFontSize();
        const ImVec2 lucSz   = ImGui::CalcTextSize("LUCID");
        const ImVec2 verSz   = ImGui::CalcTextSize("v1.5");
        ImGui::PopFont();

        const float tabSz   = 36.f;
        const float tabGap  = 4.f;
        const float dockPad = 14.f;
        const float dockSep = 12.f;     // gap between brand / tabs / version
        const float tabsW   = kTabCount * tabSz + (kTabCount - 1) * tabGap;
        const float dockW   = dockPad + lucSz.x + dockSep + tabsW + dockSep + verSz.x + dockPad;
        const float dockX   = wp.x + (ws.x - dockW) * 0.5f;
        const float dockY   = panBR.y + DOCK_GAP;
        ImVec2 dockTL = { dockX,         dockY };
        ImVec2 dockBR = { dockX + dockW, dockY + DOCK_H };

        // dock backdrop — single-tone fill (no multicolor bleed), pill-rounded
        const float dockR = DOCK_H * 0.5f;
        // shadow under the dock
        for (int s = 0; s < 3; ++s)
        {
            float ofs   = 2.f + (float)s * 2.5f;
            int   alpha = (int)((28 - s * 7) * menuAlpha);
            if (alpha <= 0) continue;
            dl->AddRectFilled(
                { dockTL.x - 1.f, dockTL.y + ofs },
                { dockBR.x + 1.f, dockBR.y + ofs },
                IM_COL32(0, 0, 0, alpha), dockR + 2.f);
        }
        dl->AddRectFilled(dockTL, dockBR, IM_COL32(13, 12, 20, sidA), dockR);
        dl->AddRect(
            { dockTL.x + 0.5f, dockTL.y + 0.5f },
            { dockBR.x - 0.5f, dockBR.y - 0.5f },
            IM_COL32(255, 255, 255, (int)(menuAlpha * 12.f)), dockR - 0.5f, 0, 1.f);
        dl->AddRectFilledMultiColor(
            { dockTL.x + 18, dockTL.y + 1.5f },
            { dockBR.x - 18, dockTL.y + 3.f },
            IM_COL32(255, 255, 255, 0),
            IM_COL32(255, 255, 255, (int)(78 * menuAlpha)),
            IM_COL32(255, 255, 255, (int)(78 * menuAlpha)),
            IM_COL32(255, 255, 255, 0));
        dl->AddRect(dockTL, dockBR, IM_COL32(46, 44, 68, borA), dockR, 0, 1.f);

        // RGB sweep along the BOTTOM inner edge of the dock — mirror image
        // of the content card’s top sweep so the two islands bracket the UI.
        {
            const float t      = (float)ImGui::GetTime();
            const float cycle  = 0.16f;
            const float inset  = dockR * 0.55f;
            const float swX0   = dockTL.x + inset;
            const float swX1   = dockBR.x - inset;
            const float swY    = dockBR.y - 2.2f;
            const int   segs   = 80;
            const float swW    = swX1 - swX0;
            for (int s = 0; s < segs; ++s)
            {
                float u0 = (float)s / segs, u1 = (float)(s + 1) / segs;
                float hue = fmodf(t * cycle + u0 * 0.5f, 1.f);
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.85f, 1.f, r, g, b);
                float edgeFade = 1.f;
                if (u0 < 0.14f)        edgeFade = u0 / 0.14f;
                else if (u0 > 0.86f)   edgeFade = (1.f - u0) / 0.14f;
                int a = (int)(170 * menuAlpha * edgeFade);
                dl->AddRectFilled(
                    { swX0 + u0 * swW, swY },
                    { swX0 + u1 * swW, swY + 1.f },
                    IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), a));
            }
        }

        // ── OUTER RGB illuminating strip BELOW the dock ──
        {
            const float t      = (float)ImGui::GetTime();
            const float cycle  = 0.16f;
            const float inset  = dockR * 1.2f;
            const float swX0   = dockTL.x + inset;
            const float swX1   = dockBR.x - inset;
            const float swY    = dockBR.y + 4.f;    // sits below the dock
            const int   segs   = 90;
            const float swW    = swX1 - swX0;
            for (int s = 0; s < segs; ++s)
            {
                float u0 = (float)s / segs, u1 = (float)(s + 1) / segs;
                float hue = fmodf(-t * cycle + u0 * 0.5f, 1.f);
                if (hue < 0.f) hue += 1.f;
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.85f, 1.f, r, g, b);
                float edgeFade = 1.f;
                if (u0 < 0.20f)        edgeFade = u0 / 0.20f;
                else if (u0 > 0.80f)   edgeFade = (1.f - u0) / 0.20f;
                int aCore = (int)(180 * menuAlpha * edgeFade);
                int aGlow = (int)( 55 * menuAlpha * edgeFade);
                ImU32 cCore = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), aCore);
                ImU32 cGlow = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), aGlow);
                // crisp line
                dl->AddRectFilled({ swX0 + u0 * swW, swY        },
                                  { swX0 + u1 * swW, swY + 1.2f }, cCore);
                // soft halo below
                dl->AddRectFilled({ swX0 + u0 * swW, swY + 1.2f },
                                  { swX0 + u1 * swW, swY + 5.2f }, cGlow);
            }
        }

        const float dockMidY = (dockTL.y + dockBR.y) * 0.5f;

        // ---- LUCID brand on the left (with halo) ----
        {
            float lx = dockTL.x + dockPad;
            float ly = dockMidY - lucSz.y * 0.5f;
            const ImU32 col  = EvoAccent((int)(250 * menuAlpha));
            const ImU32 halo = EvoAccent((int)(75  * menuAlpha));
            dl->AddText({ lx - 1.f, ly      }, halo, "LUCID");
            dl->AddText({ lx + 1.f, ly      }, halo, "LUCID");
            dl->AddText({ lx,       ly - 1.f}, halo, "LUCID");
            dl->AddText({ lx,       ly + 1.f}, halo, "LUCID");
            dl->AddText({ lx, ly }, col, "LUCID");
        }

        // ---- vertical hairline separator ----
        {
            float sx = dockTL.x + dockPad + lucSz.x + dockSep * 0.5f;
            dl->AddRectFilled({ sx, dockTL.y + 12.f }, { sx + 1.f, dockBR.y - 12.f },
                IM_COL32(60, 56, 86, (int)(160 * menuAlpha)));
        }

        // ---- horizontal tab row ----
        const float tabsX = dockTL.x + dockPad + lucSz.x + dockSep;
        const float tabsY = dockMidY - tabSz * 0.5f;
        for (int i = 0; i < kTabCount; ++i)
        {
            float tx = tabsX + (float)i * (tabSz + tabGap);
            float ty = tabsY;
            bool  sel = (activeTab == i);

            ImGui::SetCursorScreenPos({ tx, ty });
            ImGui::PushID(i + 50000);
            const bool clicked = ImGui::InvisibleButton("##tab", { tabSz, tabSz });
            const bool hovered = ImGui::IsItemHovered();
            ImGui::PopID();
            if (clicked) {
                if (activeTab != i) { pageAnim = 0.f; pageAnimTab = i; }
                activeTab = i;
            }

            if (sel)
            {
                const ImU32 selTop = IM_COL32(
                    (int)(primaryColor[0] * 255 * 0.32f),
                    (int)(primaryColor[1] * 255 * 0.32f),
                    (int)(primaryColor[2] * 255 * 0.32f),
                    (int)(220 * menuAlpha));
                const ImU32 selBot = IM_COL32(
                    (int)(primaryColor[0] * 255 * 0.16f),
                    (int)(primaryColor[1] * 255 * 0.16f),
                    (int)(primaryColor[2] * 255 * 0.16f),
                    (int)(180 * menuAlpha));
                dl->AddRectFilledMultiColor(
                    { tx, ty }, { tx + tabSz, ty + tabSz },
                    selTop, selTop, selBot, selBot);
                dl->AddRect({ tx, ty }, { tx + tabSz, ty + tabSz },
                    EvoAccent((int)(180 * menuAlpha)), 8.f, 0, 1.f);
            }
            else if (hovered)
            {
                dl->AddRectFilled({ tx, ty }, { tx + tabSz, ty + tabSz },
                    IM_COL32(255, 255, 255, (int)(16 * menuAlpha)), 8.f);
            }

            ImU32 iconCol = sel
                ? EvoAccent((int)(245 * menuAlpha))
                : (hovered ? IM_COL32(190, 190, 205, (int)(235 * menuAlpha))
                           : IM_COL32(115, 115, 135, (int)(220 * menuAlpha)));
            DrawTabIcon(dl, i, { tx + tabSz * 0.5f, ty + tabSz * 0.5f }, iconCol);
        }

        // ---- right hairline separator ----
        {
            float sx = tabsX + tabsW + dockSep * 0.5f;
            dl->AddRectFilled({ sx, dockTL.y + 12.f }, { sx + 1.f, dockBR.y - 12.f },
                IM_COL32(60, 56, 86, (int)(160 * menuAlpha)));
        }

        // ---- v1.5 RGB tag on the right (illuminated, no chip) ----
        {
            const char* ver = "v1.5";
            float vx = tabsX + tabsW + dockSep;
            float vy = dockMidY - verSz.y * 0.5f;

            const float t      = (float)ImGui::GetTime();
            const float cycle  = 0.16f;
            const float spread = 0.09f;

            float cx = vx;
            for (int i = 0; ver[i]; ++i)
            {
                char ch[2] = { ver[i], '\0' };
                ImVec2 chs = ImGui::CalcTextSize(ch);
                float hue = fmodf(t * cycle + (float)i * spread, 1.f);
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.85f, 1.f, r, g, b);
                ImU32 col  = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), (int)(255 * menuAlpha));
                ImU32 glow = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), (int)(110 * menuAlpha));
                ImVec2 cp{ cx, vy };
                dl->AddText({ cp.x - 1.f, cp.y      }, glow, ch);
                dl->AddText({ cp.x + 1.f, cp.y      }, glow, ch);
                dl->AddText({ cp.x,       cp.y - 1.f}, glow, ch);
                dl->AddText({ cp.x,       cp.y + 1.f}, glow, ch);
                dl->AddText(cp, col, ch);
                cx += chs.x;
            }
            // sweep underline beneath the version
            const int segs = 14;
            for (int s = 0; s < segs; ++s)
            {
                float u0 = (float)s / segs, u1 = (float)(s + 1) / segs;
                float hue = fmodf(t * cycle + u0 * 0.4f, 1.f);
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(hue, 0.85f, 1.f, r, g, b);
                dl->AddRectFilled(
                    { vx + u0 * verSz.x, vy + verSz.y + 1.5f },
                    { vx + u1 * verSz.x, vy + verSz.y + 2.7f },
                    IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), (int)(195 * menuAlpha)));
            }
        }

        // (no outer window border — cards stand on their own)

        // ----------------------------------------------------------------
        //  CONTENT AREA  ─  feature card grid OR feature page (back-able)
        // ----------------------------------------------------------------
        // page transition fade
        if (pageAnim < 1.f) { pageAnim += dt * 7.f; if (pageAnim > 1.f) pageAnim = 1.f; }
        ImGui::PushStyleColor(ImGuiCol_ChildBg,  { 0.f, 0.f, 0.f, 0.f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.f, 6.f });
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { 8.f, 5.f });

        // outer scroll container spans both former columns
        const float CONTENT_X = PAD;
        const float CONTENT_W = ws.x - PAD * 2.f;
        ImGui::SetCursorPos({ CONTENT_X, COL_Y });
        if (ImGui::BeginChild("##content19", { CONTENT_W, COL_H }, false,
                ImGuiWindowFlags_NoScrollbar))
        {
            ImVec2 areaSz{ CONTENT_W - 8.f, COL_H - 4.f };
            ImGuiErrorRecoveryState rs;
            ImGui::ErrorRecoveryStoreState(&rs);
            __try {
                if (pageStack[activeTab] < 0)
                    DrawFeatureGrid(activeTab, dt, areaSz);
                else
                    DrawFeaturePage(activeTab, dt, areaSz);
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
            ImGui::ErrorRecoveryTryToRecoverState(&rs);
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(2);   // WindowPadding + ItemSpacing
        ImGui::PopStyleColor();  // ChildBg

        ImGui::End();
        ImGui::PopStyleVar(3);   // WindowPadding, WindowBorderSize, Alpha
    }

} // namespace Menu
