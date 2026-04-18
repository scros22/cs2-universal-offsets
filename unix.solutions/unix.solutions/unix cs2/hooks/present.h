#pragma once
#include <d3d11.h>
#include <dxgi.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../sdk/Verdana_Regular.h"
#include "../sdk/Verdana_Bold.h"
#include "../sdk/Tahoma_Bold.h"
#include "../sdk/icon_font.h"

inline ImFont* Verdana = nullptr;
inline ImFont* Verdana_Bold = nullptr;
inline ImFont* Tahoma_BoldXP = nullptr;
inline ImFont* iconFont = nullptr;
inline ImFont* g_EspFonts[3] = { nullptr, nullptr, nullptr }; // 0=Pixel(default), 1=Verdana, 2=Hax(Tahoma)

#include "../features/skin_changer.h"
#include "../features/esp.h"
#include "../features/aimbot.h"
#include "../features/bullet_tracer.h"
#include "../features/spinbot.h"
#include "../features/fov_changer.h"
#include "../features/model_changer.h"
#include "../features/chams.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <cstdlib>
#include <cmath>

// === Global state ===
inline bool g_RequestUnload = false;
inline float g_MenuAlpha = 0.0f;
inline bool g_ViewFovEnabled = false;
inline int g_ViewFov = 90;

// === Snow particle system ===
struct SnowParticle { float x, y, speed, drift, phase; };
inline SnowParticle g_Snow[200];
inline bool g_SnowInit = false;

inline void InitSnow(float w, float h)
{
    for (int i = 0; i < 200; i++)
    {
        g_Snow[i].x = (float)(rand() % (int)w);
        g_Snow[i].y = (float)(rand() % (int)h);
        g_Snow[i].speed = 30.0f + (float)(rand() % 60);
        g_Snow[i].drift = 0.3f + (float)(rand() % 100) / 200.0f;
        g_Snow[i].phase = (float)(rand() % 628) / 100.0f;
    }
    g_SnowInit = true;
}

inline void UpdateAndDrawSnow(ImDrawList* dl, float w, float h, float alpha, float dt)
{
    if (!g_SnowInit) InitSnow(w, h);
    ImU32 col = IM_COL32(255, 255, 255, (int)(alpha * 180));
    for (int i = 0; i < 200; i++)
    {
        g_Snow[i].y += g_Snow[i].speed * dt;
        g_Snow[i].x += sinf(g_Snow[i].phase + g_Snow[i].y * 0.01f) * g_Snow[i].drift;
        if (g_Snow[i].y > h) { g_Snow[i].y = -5.0f; g_Snow[i].x = (float)(rand() % (int)w); }
        if (g_Snow[i].x < 0) g_Snow[i].x = w;
        if (g_Snow[i].x > w) g_Snow[i].x = 0;
        dl->AddText(ImVec2(g_Snow[i].x, g_Snow[i].y), col, ".");
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks
{
    using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
    using ResizeBuffersFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

    inline PresentFn oPresent = nullptr;
    inline ResizeBuffersFn oResizeBuffers = nullptr;
    inline WNDPROC oWndProc = nullptr;

    void Shutdown();

    inline uintptr_t* pVTablePresentEntry = nullptr;
    inline uintptr_t* pVTableResizeEntry = nullptr;
    inline uintptr_t* pVTableDrawEntry   = nullptr;

    inline ID3D11Device* pDevice = nullptr;
    inline ID3D11DeviceContext* pContext = nullptr;
    inline ID3D11RenderTargetView* pRenderTargetView = nullptr;
    inline HWND gameWindow = nullptr;
    inline bool initialized = false;
    inline bool showMenu = true;

    inline void CleanupRenderTarget()
    {
        if (pRenderTargetView)
        {
            pRenderTargetView->Release();
            pRenderTargetView = nullptr;
        }
    }

    inline void CreateRenderTarget(IDXGISwapChain* pSwapChain)
    {
        ID3D11Texture2D* pBackBuffer = nullptr;
        if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer)))
        {
            pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
            pBackBuffer->Release();
        }
    }

    inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_KEYDOWN && wParam == VK_INSERT)
        {
            showMenu = !showMenu;
            return 0;
        }

        if (showMenu)
        {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                return true;

            switch (msg)
            {
            case WM_LBUTTONDOWN: case WM_LBUTTONUP:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP:
            case WM_MOUSEWHEEL:
            case WM_MOUSEMOVE:
                if (ImGui::GetIO().WantCaptureMouse)
                    return 0;
                break;
            case WM_KEYDOWN: case WM_KEYUP:
            case WM_CHAR:
                if (ImGui::GetIO().WantCaptureKeyboard)
                    return 0;
                break;
            }
        }

        return CallWindowProcW(oWndProc, hWnd, msg, wParam, lParam);
    }

    void RenderMenu();

    inline HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
    {
        // --- STABILITY GUARD ---
        // Check if the device has changed or been released (map transition)
        ID3D11Device* currentDevice = nullptr;
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&currentDevice)))
        {
            if (initialized && pDevice != currentDevice)
            {
                printf("[!] Graphics Device Change Detected - Resetting Session Safely...\n");
                Shutdown();
            }
            currentDevice->Release();
        }

        if (!initialized)
        {
            if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
                return oPresent(pSwapChain, SyncInterval, Flags);

            pDevice->GetImmediateContext(&pContext);

            D3D11_DEPTH_STENCIL_DESC dd = { FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, 0, 0 };
            pDevice->CreateDepthStencilState(&dd, &Chams::dssOff);
            printf("[+] Chams System Initialized (Shaders Compiled, Buffers Ready)\n");
            Chams::ready = true;

            DXGI_SWAP_CHAIN_DESC desc;
            pSwapChain->GetDesc(&desc);
            gameWindow = desc.OutputWindow;

            CreateRenderTarget(pSwapChain);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.IniFilename = nullptr;

            ImFontConfig fontCfg{};
            fontCfg.PixelSnapH = true;
            fontCfg.OversampleH = 2;
            fontCfg.OversampleV = 1;
            fontCfg.RasterizerMultiply = 1.05f;
            fontCfg.FontDataOwnedByAtlas = false;

            io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(Verdana_Regular), sizeof(Verdana_Regular), 13.0f, &fontCfg);
            Tahoma_BoldXP = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(Tahoma_Bold), sizeof(Tahoma_Bold), 13.0f, &fontCfg);
            Verdana = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(Verdana_Regular), sizeof(Verdana_Regular), 13.0f, &fontCfg);
            Verdana_Bold = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(Verdana_Bold2), sizeof(Verdana_Bold2), 14.0f, &fontCfg);
            iconFont = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(weapons_font), sizeof(weapons_font), 18.0f, &fontCfg);

            // Set up ESP font array: 0=Pixel(default), 1=Verdana, 2=Hax(Tahoma)
            g_EspFonts[0] = io.Fonts->Fonts[0]; // default font = pixel
            g_EspFonts[1] = Verdana;
            g_EspFonts[2] = Tahoma_BoldXP;

            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 0;
            style.ChildRounding = 0;
            style.FrameRounding = 0;
            style.ScrollbarRounding = 0;
            style.GrabRounding = 0;
            style.PopupRounding = 0;
            style.TabRounding = 0;

            style.WindowBorderSize = 1;
            style.FrameBorderSize = 1;
            style.PopupBorderSize = 1;

            style.WindowPadding = ImVec2(8, 8);
            style.FramePadding = ImVec2(4, 3);
            style.ItemSpacing = ImVec2(8, 6);
            style.ItemInnerSpacing = ImVec2(4, 4);
            style.CellPadding = ImVec2(2, 1);
            style.GrabMinSize = 10;
            style.ScrollbarSize = 12;

            ImVec4* colors = style.Colors;
            colors[ImGuiCol_WindowBg] = ImColor(16, 16, 16);
            colors[ImGuiCol_ChildBg] = ImColor(24, 24, 24);
            colors[ImGuiCol_PopupBg] = ImColor(16, 16, 16);
            colors[ImGuiCol_MenuBarBg] = ImColor(20, 20, 20);
            colors[ImGuiCol_Border] = ImColor(0, 0, 0, 255);
            colors[ImGuiCol_Separator] = ImColor(58, 58, 58);
            colors[ImGuiCol_Text] = ImColor(255, 255, 255);
            colors[ImGuiCol_TextDisabled] = ImColor(146, 146, 146);
            colors[ImGuiCol_FrameBg] = ImColor(13, 13, 13);
            colors[ImGuiCol_FrameBgHovered] = ImColor(16, 16, 16);
            colors[ImGuiCol_FrameBgActive] = ImColor(11, 11, 11);
            colors[ImGuiCol_Button] = ImColor(23, 23, 23);
            colors[ImGuiCol_ButtonHovered] = ImColor(27, 27, 27);
            colors[ImGuiCol_ButtonActive] = ImColor(20, 20, 20);
            colors[ImGuiCol_SliderGrab] = ImColor(255, 74, 82);
            colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.29f, 0.32f, 0.5f);
            colors[ImGuiCol_CheckMark] = ImColor(255, 74, 82);
            colors[ImGuiCol_Header] = ImColor(23, 23, 23);
            colors[ImGuiCol_HeaderHovered] = ImColor(27, 27, 27);
            colors[ImGuiCol_HeaderActive] = ImColor(20, 20, 20);
            colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
            colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.0f, 0.29f, 0.32f, 0.5f);
            colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0f, 0.29f, 0.32f, 0.5f);
            colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.29f, 0.32f, 0.5f);
            colors[ImGuiCol_Tab] = ImColor(13, 13, 13);
            colors[ImGuiCol_TabHovered] = ImColor(27, 27, 27);
            colors[ImGuiCol_TitleBg] = ImColor(16, 16, 16);
            colors[ImGuiCol_TitleBgActive] = ImColor(16, 16, 16);
            colors[ImGuiCol_TitleBgCollapsed] = ImColor(16, 16, 16);

            ImGui_ImplWin32_Init(gameWindow);
            ImGui_ImplDX11_Init(pDevice, pContext);

            oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

            printf("[+] Captured D3D11 Device: 0x%IX | Context: 0x%IX\n", (uintptr_t)pDevice, (uintptr_t)pContext);
            Chams::Init(pDevice);
            initialized = true;
        }

        SkinChanger::Tick();
        ModelChanger::Run();
        Chams::Tick();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        float dt = io.DeltaTime;

        // Smooth menu alpha fade
        float targetAlpha = showMenu ? 1.0f : 0.0f;
        g_MenuAlpha += (targetAlpha - g_MenuAlpha) * (dt * 8.0f);
        if (g_MenuAlpha < 0.01f) g_MenuAlpha = 0.0f;
        if (g_MenuAlpha > 0.99f) g_MenuAlpha = 1.0f;

        // Blur overlay + snow when menu is visible
        if (g_MenuAlpha > 0.01f)
        {
            ImDrawList* bgDraw = ImGui::GetBackgroundDrawList();
            ImU32 blurCol = IM_COL32(0, 0, 0, (int)(g_MenuAlpha * 120));
            bgDraw->AddRectFilled(ImVec2(0, 0), io.DisplaySize, blurCol);
            UpdateAndDrawSnow(bgDraw, io.DisplaySize.x, io.DisplaySize.y, g_MenuAlpha, dt);
        }

        if (showMenu)
            RenderMenu();

        // === DEBUG OVERLAY ===
#ifdef _DEBUG
        {
            ImDrawList* dbgDraw = ImGui::GetBackgroundDrawList();
            if (dbgDraw)
            {
                float dy = 40.0f;
                char buf[256];

                uintptr_t cb = Game::clientBase;
                snprintf(buf, sizeof(buf), "clientBase: 0x%IX", cb);
                dbgDraw->AddText(ImVec2(10, dy), IM_COL32(0, 255, 0, 255), buf); dy += 14;

                uintptr_t localCtrl = cb ? Game::Read<uintptr_t>(cb + Offsets::dwLocalPlayerController) : 0;
                snprintf(buf, sizeof(buf), "localCtrl: 0x%IX", localCtrl);
                dbgDraw->AddText(ImVec2(10, dy), IM_COL32(0, 255, 0, 255), buf); dy += 14;

                uintptr_t localPawn = cb ? Game::Read<uintptr_t>(cb + Offsets::dwLocalPlayerPawn) : 0;
                snprintf(buf, sizeof(buf), "localPawn: 0x%IX", localPawn);
                dbgDraw->AddText(ImVec2(10, dy), IM_COL32(0, 255, 0, 255), buf); dy += 14;

                uintptr_t entList = cb ? Game::Read<uintptr_t>(cb + Offsets::dwEntityList) : 0;
                snprintf(buf, sizeof(buf), "entityList: 0x%IX", entList);
                dbgDraw->AddText(ImVec2(10, dy), IM_COL32(0, 255, 0, 255), buf); dy += 14;

                if (localPawn)
                {
                    int hp = Game::Read<int32_t>(localPawn + Offsets::m_iHealth);
                    int team = Game::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
                    snprintf(buf, sizeof(buf), "HP: %d  Team: %d", hp, team);
                    dbgDraw->AddText(ImVec2(10, dy), IM_COL32(0, 255, 0, 255), buf); dy += 14;
                }

                Game::ViewMatrix vm = cb ? Game::Read<Game::ViewMatrix>(cb + Offsets::dwViewMatrix) : Game::ViewMatrix{};
                snprintf(buf, sizeof(buf), "viewMatrix[0][0]: %.3f  [3][3]: %.3f", vm.m[0][0], vm.m[3][3]);
                dbgDraw->AddText(ImVec2(10, dy), IM_COL32(0, 255, 0, 255), buf); dy += 14;

                snprintf(buf, sizeof(buf), "ESP enabled: %d  bBox: %d", ESP::config.enabled, ESP::config.bBox);
                dbgDraw->AddText(ImVec2(10, dy), IM_COL32(0, 255, 0, 255), buf); dy += 14;

                // Test draw — if this line appears, background draw list works
                dbgDraw->AddLine(ImVec2(10, dy + 2), ImVec2(200, dy + 2), IM_COL32(255, 0, 0, 255), 2.0f);
                dy += 6;
                dbgDraw->AddText(ImVec2(10, dy), IM_COL32(255, 0, 0, 255), "^ test line (if visible, drawlist OK)");
            }
        }
#endif
        // === END DEBUG ===

        ESP::Render();
        Aimbot::RenderFOV();
        BulletTracer::Render();

        ImGui::Render();

        pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Third person camera (direct offset — tested working)
        if (Game::clientBase && SkinChanger::thirdPerson)
        {
            Game::Write<bool>(Game::clientBase + Offsets::dwCSGOInput + 0x229, true);
        }
        else if (Game::clientBase)
        {
            Game::Write<bool>(Game::clientBase + Offsets::dwCSGOInput + 0x229, false);
        }

        // View FOV override — logic moved to FOVChanger high-frequency thread

        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    inline HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
    {
        CleanupRenderTarget();
        HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        CreateRenderTarget(pSwapChain);
        return hr;
    }

    inline void SwapVTableEntry(uintptr_t* pEntry, void* pNewFn, void** ppOriginal)
    {
        DWORD oldProtect;
        VirtualProtect(pEntry, sizeof(uintptr_t), PAGE_READWRITE, &oldProtect);
        *ppOriginal = reinterpret_cast<void*>(*pEntry);
        *pEntry = reinterpret_cast<uintptr_t>(pNewFn);
        VirtualProtect(pEntry, sizeof(uintptr_t), oldProtect, &oldProtect);
    }

    inline void RestoreVTableEntry(uintptr_t* pEntry, void* pOriginalFn)
    {
        if (!pEntry || !pOriginalFn) return;
        DWORD oldProtect;
        VirtualProtect(pEntry, sizeof(uintptr_t), PAGE_READWRITE, &oldProtect);
        *pEntry = reinterpret_cast<uintptr_t>(pOriginalFn);
        VirtualProtect(pEntry, sizeof(uintptr_t), oldProtect, &oldProtect);
    }

    inline bool SetupDXHooks()
    {
        WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, DefWindowProcA, 0, 0, GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr, "UnixDummy", nullptr };
        RegisterClassExA(&wc);
        HWND hWnd = CreateWindowExA(0, wc.lpszClassName, "UnixDummy", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);

        if (!hWnd)
        {
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
            return false;
        }

        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        IDXGISwapChain* pDummy = nullptr;
        ID3D11Device* pDev = nullptr;
        ID3D11DeviceContext* pCtx = nullptr;
        D3D_FEATURE_LEVEL fl;

        if (FAILED(D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION,
            &sd, &pDummy, &pDev, &fl, &pCtx)))
        {
            DestroyWindow(hWnd);
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
            return false;
        }

        uintptr_t* vtable = *reinterpret_cast<uintptr_t**>(pDummy);

        pVTablePresentEntry = &vtable[8];
        pVTableResizeEntry  = &vtable[13];

        SwapVTableEntry(pVTablePresentEntry, &hkPresent, reinterpret_cast<void**>(&oPresent));
        SwapVTableEntry(pVTableResizeEntry, &hkResizeBuffers, reinterpret_cast<void**>(&oResizeBuffers));

        uintptr_t* vtableCtx = *reinterpret_cast<uintptr_t**>(pCtx);
        pVTableDrawEntry = reinterpret_cast<uintptr_t*>(vtableCtx[20]); // Store function address

        // --- MinHook for DrawIndexedInstanced (Reliable) ---
        void* pDrawFn = reinterpret_cast<void*>(vtableCtx[20]);
        if (MH_CreateHook(pDrawFn, &Chams::hkDrawIndexedInstanced, reinterpret_cast<void**>(&Chams::oDrawIndexedInstanced)) == MH_OK)
        {
            if (MH_EnableHook(pDrawFn) == MH_OK)
                printf("[+] Chams Hook enabled (MinHook)\n");
            else
                printf("[-] FAILED to enable Chams Hook\n");
        }
        else
        {
            printf("[-] FAILED to create Chams Hook\n");
        }

        pDummy->Release();
        pDev->Release();
        pCtx->Release();
        
        DestroyWindow(hWnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);

        return true;
    }

    inline void Shutdown()
    {
        if (initialized)
        {
            if (oWndProc && gameWindow)
                SetWindowLongPtrW(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));

            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            CleanupRenderTarget();

            if (pContext) { pContext->Release(); pContext = nullptr; }
            if (pDevice) { pDevice->Release(); pDevice = nullptr; }

            initialized = false;
        }

        RestoreVTableEntry(pVTablePresentEntry, oPresent);
        RestoreVTableEntry(pVTableResizeEntry, oResizeBuffers);
        
        MH_DisableHook(MH_ALL_HOOKS); 

        Chams::Shutdown();

        oPresent = nullptr;
        oResizeBuffers = nullptr;
        pDevice = nullptr;
        pContext = nullptr;
        initialized = false;
    }
}
