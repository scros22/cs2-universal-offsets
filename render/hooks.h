                                                                                            #pragma once

// ---------------------------------------------------------------
// DX11 hooks — Present and ResizeBuffers via MinHook function
// detours. ImGui init, WndProc interception, third-person patch.
// ---------------------------------------------------------------

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <cstdio>
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/backends/imgui_impl_win32.h"
#include "../vendor/imgui/backends/imgui_impl_dx11.h"
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/signatures.h"
#include "../core/memory.h"
#include "../core/stealth.h"
#include "../core/vtable_swap.h"
#include "../features/visuals/bullet_tracer.h"
#include "../features/visuals/esp.h"
#include "../features/combat/aimbot.h"
#include "../features/visuals/world_effects.h"
#include "../features/visuals/damage_indicator.h"
#include "../features/movement/bhop.h"
#include "../features/visuals/chams.h"
#include "../features/misc/auto_accept.h"
#include "../features/misc/rank_revealer.h"
#include "../features/misc/grenade_prediction.h"
#include "../sdk/icon_font.h"
#include "../features/visuals/weapon_icons.h"
#include "../features/misc/nade_helper.h"
#include "../features/skins/skinchanger.h"
#include "../features/skins/inventory_changer.h"
#include "../features/skins/knife_glove_manager.h"
#include "../features/skins/model_changer.h"
#include "../features/combat/triggerbot.h"
#include "../features/movement/backtrack.h"
#include "../features/movement/anti_aim.h"
#include "../features/movement/fake_lag.h"
#include "../features/misc/sound_esp.h"

// Global weapon icon font (like unix.solutions) - MUST be declared before any namespace
inline ImFont* iconFont = nullptr;

// Wireframe config — declared at file scope so menu.h can reference it
// via forward declaration. Moved here so it resolves before Hooks namespace.
namespace WireframeHands
{
    struct Config
    {
        bool  enabled     = false;
        float color[4]    = { 0.0f, 1.0f, 0.8f, 1.0f }; // teal
        bool  handsOnly   = true;
    };
    inline Config cfg;
}

#include "menu.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace Hooks
{
    using PresentFn        = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
    using Present1Fn       = HRESULT(__stdcall*)(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
    using ResizeBuffersFn  = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
    using DrawIndexedFn    = void(__stdcall*)(ID3D11DeviceContext*, UINT, UINT, INT);

    inline PresentFn       oPresent       = nullptr;
    inline Present1Fn      oPresent1      = nullptr;
    inline ResizeBuffersFn oResizeBuffers = nullptr;
    inline DrawIndexedFn   oDrawIndexed   = nullptr;
    inline WNDPROC         oWndProc       = nullptr;

    inline void*           pHkPresent     = nullptr;
    inline void*           pHkPresent1    = nullptr;
    inline void*           pHkResize      = nullptr;
    inline void*           pHkFSN         = nullptr;
    inline void*           pHkDrawIndexed = nullptr;

    inline ID3D11Device*            pDevice     = nullptr;
    inline ID3D11DeviceContext*     pContext    = nullptr;
    inline ID3D11RenderTargetView*  pRTV        = nullptr;
    inline HWND                     hwndGame    = nullptr;
    inline bool                     inited      = false;
    inline bool                     showMenu    = true;

    inline ID3D11RasterizerState* wireframeRS = nullptr;
    inline ID3D11RasterizerState* normalRS    = nullptr;
    
    // VMT shadow-copy hooks — replaces MinHook inline detours.
    // VMT swaps only modify heap-allocated vtable copies; they
    // never patch game or system DLL .text sections that VAC checksums.
    inline VTableSwap vmtSwapChain;
    inline VTableSwap vmtContext;
    inline VTableSwap vmtSource2Client;
    inline bool       vmtActive = false;   // true once VMT hooks replace MinHook bootstrap
    inline IDXGISwapChain* vmtSwapChainObj = nullptr; // game's real swap chain

    // Debug log — writes to %TEMP%\cs2dbg.txt (debug builds only)
#ifdef _DEBUG
    inline void Log(const char* fmt, ...)
    {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        strcat_s(path, "cs2dbg.txt");
        FILE* f = nullptr;
        fopen_s(&f, path, "a");
        if (f) {
            va_list args;
            va_start(args, fmt);
            vfprintf(f, fmt, args);
            va_end(args);
            fprintf(f, "\n");
            fclose(f);
        }
    }
#else
    inline void Log(const char*, ...) {} // no-op in release
#endif

    // ---------------------------------------------------------------
    // Render target management
    // ---------------------------------------------------------------
    inline void DestroyRT()
    {
        if (pRTV) { pRTV->Release(); pRTV = nullptr; }
    }

    inline void CreateRT(IDXGISwapChain* sc)
    {
        ID3D11Texture2D* bb = nullptr;
        if (SUCCEEDED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb)))
        {
            pDevice->CreateRenderTargetView(bb, nullptr, &pRTV);
            bb->Release();
        }
    }

    // ---------------------------------------------------------------
    // WndProc hook — menu toggle + ImGui input forwarding
    // ---------------------------------------------------------------
    inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        if (msg == WM_KEYDOWN && wp == VK_INSERT)
        {
            showMenu = !showMenu;
            // Track how many times we bumped the OS cursor counter so we
            // can put it back exactly where we found it on close — never
            // hide CS2's own cursor (pause menu / scoreboard / buy menu).
            static int cursorBumps = 0;
            if (showMenu)
            {
                while (ShowCursor(TRUE) < 0) { ++cursorBumps; }
                ClipCursor(nullptr);
            }
            else
            {
                while (cursorBumps > 0) { ShowCursor(FALSE); --cursorBumps; }
            }
            return 0;
        }

        // Middle mouse click toggles third person (works even with menu closed)
        if (msg == WM_MBUTTONDOWN)
        {
            WorldEffects::cfg.thirdPerson = !WorldEffects::cfg.thirdPerson;
        }

        if (showMenu)
        {
            // CS2 reads mouse movement via raw input (WM_INPUT), bypassing
            // WantCaptureMouse. Eat raw input + standard mouse messages so
            // moving the cursor across the menu doesn't rotate the camera.
            if (msg == WM_INPUT) return 0;

            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
                return true;

            switch (msg)
            {
            case WM_LBUTTONDOWN: case WM_LBUTTONUP:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP:
            case WM_MOUSEWHEEL:  case WM_MOUSEMOVE:
            case WM_NCMOUSEMOVE: case WM_MOUSEACTIVATE:
                return 0;  // never let game see mouse events while menu is up
            case WM_KEYDOWN: case WM_KEYUP: case WM_CHAR:
            case WM_SYSKEYDOWN: case WM_SYSKEYUP:
                if (ImGui::GetIO().WantCaptureKeyboard) return 0;
                break;
            }
        }
        return CallWindowProcW(oWndProc, hWnd, msg, wp, lp);
    }

    // ---------------------------------------------------------------
    // DrawIndexed hook — wireframe viewmodel rendering
    // Viewmodel pass uses DepthWriteMask = 0 (renders on top of scene).
    // When detected, swaps rasterizer to wireframe and optionally
    // tints the viewmodel via m_clrRender each frame.
    // ---------------------------------------------------------------
    inline void __stdcall hkDrawIndexed(ID3D11DeviceContext* ctx, UINT indexCount, UINT startIndex, INT baseVertex)
    {
        if (WireframeHands::cfg.enabled)
        {
            bool applyWireframe = false;
            if (WireframeHands::cfg.handsOnly)
            {
                // Check depth stencil state — viewmodel renders with DepthWriteMask = 0
                ID3D11DepthStencilState* dss = nullptr;
                UINT stencilRef = 0;
                ctx->OMGetDepthStencilState(&dss, &stencilRef);
                if (dss)
                {
                    D3D11_DEPTH_STENCIL_DESC desc;
                    dss->GetDesc(&desc);
                    dss->Release();
                    // Viewmodel: depth test ON but depth write OFF
                    if (desc.DepthEnable && desc.DepthWriteMask == D3D11_DEPTH_WRITE_MASK_ZERO)
                        applyWireframe = true;
                }
            }
            else
            {
                // Wireframe everything
                applyWireframe = true;
            }

            if (applyWireframe && wireframeRS)
            {
                ID3D11RasterizerState* prevRS = nullptr;
                ctx->RSGetState(&prevRS);
                ctx->RSSetState(wireframeRS);
                oDrawIndexed(ctx, indexCount, startIndex, baseVertex);
                ctx->RSSetState(prevRS);
                if (prevRS) prevRS->Release();
                return;
            }
        }
        oDrawIndexed(ctx, indexCount, startIndex, baseVertex);
    }

    // ---------------------------------------------------------------
    // DrawIndexedInstanced hook removed.
    // Chams now operates at the scenesystem layer via
    // CSceneAnimatableObject::GeneratePrimitives (features/visuals/chams.h).
    // ---------------------------------------------------------------

    // ---------------------------------------------------------------
    // Present hook — shared rendering logic
    // Called by both hkPresent (vtable[8]) and hkPresent1 (vtable[22])
    // ---------------------------------------------------------------

    // Forward-declare hook functions (defined below PresentCore) so
    // the VMT transition inside PresentCore can reference them.
    inline HRESULT __stdcall hkPresent(IDXGISwapChain*, UINT, UINT);
    inline HRESULT __stdcall hkPresent1(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
    inline HRESULT __stdcall hkResizeBuffers(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

    inline void PresentCore(IDXGISwapChain* sc)
    {
        if (!inited)
        {
            Log("[PresentCore] First call — initializing ImGui");

            __try {
            if (FAILED(sc->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
            {
                Log("[-] GetDevice failed");
                return;
            }
            pDevice->GetImmediateContext(&pContext);

            DXGI_SWAP_CHAIN_DESC desc;
            sc->GetDesc(&desc);
            hwndGame = desc.OutputWindow;
            CreateRT(sc);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.IniFilename = nullptr;

            // Shared font config for all fonts (like unix.solutions)
            ImFontConfig fontCfg{};
            fontCfg.PixelSnapH = true;
            fontCfg.OversampleH = 2;
            fontCfg.OversampleV = 1;
            fontCfg.RasterizerMultiply = 1.05f;
            fontCfg.FontDataOwnedByAtlas = false;

            // Shared glyph ranges
            static const ImWchar mdlRanges[] = { 0xE700, 0xF900, 0 };   // Segoe MDL2 icons
            static const ImWchar geoRanges[] = { 0x2500, 0x27FF, 0 };   // Geometric shapes + misc symbols

            // Font 0: Glass — Verdana Bold @ 13px, pixel-snapped, no oversample.
            // Same crisp competitive-HUD look as the on-screen watermark so
            // the menu and HUD share a single visual language.
            ImFontConfig fc0;
            fc0.OversampleH        = 1;
            fc0.OversampleV        = 1;
            fc0.PixelSnapH         = true;
            fc0.RasterizerMultiply = 1.05f;
            Menu::fonts[0] = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdanab.ttf", 13.f, &fc0);
            if (!Menu::fonts[0])
                Menu::fonts[0] = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 13.f, &fc0);
            {
                ImFontConfig mc; mc.MergeMode = true; mc.GlyphMinAdvanceX = 14.f; mc.PixelSnapH = true;
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segmdl2.ttf", 14.f, &mc, mdlRanges);
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 14.f, &mc, geoRanges);
            }

            // Font 1: Cyber — Consolas, 14px (monospace, tech/hacker aesthetic)
            Menu::fonts[1] = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.f);
            {
                ImFontConfig mc; mc.MergeMode = true; mc.GlyphMinAdvanceX = 14.f; mc.PixelSnapH = true;
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segmdl2.ttf", 14.f, &mc, mdlRanges);
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 14.f, &mc, geoRanges);
            }

            // Font 2: Minimal — Segoe UI Semibold, 14px (clean, bold readability)
            Menu::fonts[2] = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 14.f);
            {
                ImFontConfig mc; mc.MergeMode = true; mc.GlyphMinAdvanceX = 14.f; mc.PixelSnapH = true;
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segmdl2.ttf", 14.f, &mc, mdlRanges);
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisym.ttf", 14.f, &mc, geoRanges);
            }

            // Crisp HUD font — Verdana Bold @ 12px with no oversampling +
            // pixel snap.  Produces the sharp, slightly pixelated bitmap
            // look used by classic competitive HUDs.  Falls back to
            // Tahoma Bold if Verdana isn't installed.
            {
                ImFontConfig hc;
                hc.OversampleH    = 1;
                hc.OversampleV    = 1;
                hc.PixelSnapH     = true;
                hc.RasterizerMultiply = 1.10f; // slightly thicker strokes
                Menu::hudFont = io.Fonts->AddFontFromFileTTF(
                    "C:\\Windows\\Fonts\\verdanab.ttf", 12.f, &hc);
                if (!Menu::hudFont)
                    Menu::hudFont = io.Fonts->AddFontFromFileTTF(
                        "C:\\Windows\\Fonts\\tahomabd.ttf", 12.f, &hc);
            }

            // Load weapon icon font LAST so it doesn't become the default font
            // This font is ONLY used explicitly in ESP rendering
            // Using 14px instead of 18px for better performance and scaling
            iconFont = io.Fonts->AddFontFromMemoryTTF(
                const_cast<unsigned char*>(weapons_font), 
                sizeof(weapons_font), 
                14.0f,  // Smaller base size for better performance
                &fontCfg,
                io.Fonts->GetGlyphRangesDefault());

            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(hwndGame);
            ImGui_ImplDX11_Init(pDevice, pContext);

            // Pass device reference to menu for avatar texture creation
            Menu::g_pDevice = pDevice;
            // Kick off Steam avatar download in background - DISABLED (causes crashes)
            // Menu::InitSteamAvatar();

            oWndProc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtrW(hwndGame, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
            inited = true;
            Log("[PresentCore] ImGui init COMPLETE — menu should render now");

            // Create wireframe rasterizer states (now that we have the real device)
            if (pDevice && !wireframeRS)
            {
                D3D11_RASTERIZER_DESC wfDesc = {};
                wfDesc.FillMode = D3D11_FILL_WIREFRAME;
                wfDesc.CullMode = D3D11_CULL_NONE;
                wfDesc.DepthClipEnable = TRUE;
                wfDesc.FrontCounterClockwise = FALSE;
                wfDesc.DepthBias = 0;
                wfDesc.DepthBiasClamp = 0.f;
                wfDesc.SlopeScaledDepthBias = 0.f;
                wfDesc.ScissorEnable = FALSE;
                wfDesc.MultisampleEnable = FALSE;
                wfDesc.AntialiasedLineEnable = TRUE; // smoother wireframe edges
                if (SUCCEEDED(pDevice->CreateRasterizerState(&wfDesc, &wireframeRS)))
                    Log("[+] Wireframe rasterizer state created");
                else
                    Log("[-] Failed to create wireframe RS");
            }

            // Init chams D3D resources — chams now hooks scenesystem
            // GeneratePrimitives directly (features/visuals/chams.h::Setup),
            // installed from the entry thread. Nothing to do here.

            // Pre-load weapon SVG icons via Panorama UI - DISABLED (needs CUtlBuffer implementation)
            // if (WeaponIcons::Init(pDevice))
            //     Log("[+] Weapon icon loader initialized (Panorama method)");
            // else
            //     Log("[-] Weapon icon loader failed to initialize");

            } __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[-] PresentCore INIT EXCEPTION — ImGui/DX setup failed");
                return;
            }
        }

        // ---- VMT Hook Transition ----
        // On first frame with valid device/context, transition all DX
        // hooks from the MinHook bootstrap to VMT shadow-copy swap.
        // After this completes, ZERO inline code patches remain in
        // dxgi.dll, d3d11.dll, or any other module.
        if (!vmtActive && pDevice && pContext)
        {
            bool vmtOk = true;
            __try {
                // VMT swap the game's real swap chain
                if (vmtSwapChain.Init(sc, 80))
                {
                    vmtSwapChainObj = sc;
                    // Present  = vtable[8]
                    oPresent = vmtSwapChain.Original<PresentFn>(8);
                    vmtSwapChain.Replace(8, &hkPresent);
                    // ResizeBuffers = vtable[13]
                    oResizeBuffers = vmtSwapChain.Original<ResizeBuffersFn>(13);
                    vmtSwapChain.Replace(13, &hkResizeBuffers);
                    // Present1 = vtable[22] (if different from Present)
                    if (pHkPresent1)
                    {
                        oPresent1 = vmtSwapChain.Original<Present1Fn>(22);
                        vmtSwapChain.Replace(22, &hkPresent1);
                    }
                    Log("[+] SwapChain VMT hooks (Present/Resize/Present1)");
                }
                else { Log("[-] SwapChain VMT init failed"); vmtOk = false; }

                // VMT swap device context (large vtable: 200+ entries)
                if (vmtContext.Init(pContext, 200))
                {
                    oDrawIndexed = vmtContext.Original<DrawIndexedFn>(12);
                    vmtContext.Replace(12, &hkDrawIndexed);
                    Log("[+] DeviceContext VMT hook (DrawIndexed)");
                }
                else { Log("[-] DeviceContext VMT init failed"); vmtOk = false; }

                if (vmtOk)
                {
                    // Remove MinHook bootstrap entirely — free trampoline memory
                    if (pHkPresent)  { MH_DisableHook(pHkPresent);  MH_RemoveHook(pHkPresent); }
                    if (pHkPresent1) { MH_DisableHook(pHkPresent1); MH_RemoveHook(pHkPresent1); }
                    Log("[+] MinHook bootstrap REMOVED — VMT-only mode, zero inline patches");
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[-] VMT hook transition EXCEPTION");
            }
            vmtActive = true;
        }

        // Game logic ticks — all wrapped in SEH to survive loading transitions
        __try {
            Aimbot::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            Bhop::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            WorldEffects::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[PresentCore] WorldEffects::Tick EXCEPTION");
        }
        __try {
            DamageIndicator::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[PresentCore] DamageIndicator::Tick EXCEPTION");
        }
        __try {
            AutoAccept::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            SkinChanger::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            InventoryChanger::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            ModelChanger::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            Triggerbot::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            Backtrack::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            AntiAim::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        __try {
            SoundESP::Tick();
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        // KnifeGloveManager intentionally disabled — SkinChanger::Tick already
        // performs the full ARCHILIX-style knife/glove swap (SetModel +
        // SetMeshGroupMask + UpdateSubclass).  Running both racing each other
        // causes the model to revert.

        // Wireframe hands: tint viewmodel entity via m_clrRender each frame
        if (WireframeHands::cfg.enabled)
        {
            __try {
                uintptr_t lp = GameState::GetLocalPawn();
                if (lp)
                {
                    uint8_t r = (uint8_t)(WireframeHands::cfg.color[0] * 255.f);
                    uint8_t g = (uint8_t)(WireframeHands::cfg.color[1] * 255.f);
                    uint8_t b = (uint8_t)(WireframeHands::cfg.color[2] * 255.f);
                    uint8_t a = (uint8_t)(WireframeHands::cfg.color[3] * 255.f);
                    uint32_t col = (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24);
                    uint32_t vmH = Mem::Read<uint32_t>(lp + 0x1880);
                    if (vmH && vmH != 0xFFFFFFFF)
                    {
                        uintptr_t vmEnt = GameState::ResolveHandle(vmH);
                        if (vmEnt)
                            Mem::Write<uint32_t>(vmEnt + Offsets::m_clrRender, col);
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        // ImGui frame — protected against device/context loss
        __try {
            if (!pDevice || !pContext) return; // device lost, skip frame
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            // Force a software cursor while menu is open: in fullscreen
            // exclusive mode the OS cursor is suppressed, so without this
            // the user sees no pointer to aim at controls.
            ImGui::GetIO().MouseDrawCursor = showMenu;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[PresentCore] ImGui NewFrame EXCEPTION");
            return;
        }

        __try {
            if (showMenu)
            {
                ImDrawList* bgDl = ImGui::GetBackgroundDrawList();
                ImVec2 ds = ImGui::GetIO().DisplaySize;
                float t = (float)ImGui::GetTime();
                float alpha = Menu::menuAlpha; // synced with menu fade

                // --- Animated dark overlay behind the menu ---

                // 1. Base darkening (fades in with menu)
                int baseA = (int)(alpha * 110.f);
                bgDl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(0, 0, 0, baseA));

                // 2. Vignette — darken edges more, lighter center
                //    Uses a few layered rects with gradient alpha
                {
                    float vigStr = alpha * 80.f;
                    int vEdge = (int)(vigStr);
                    // Top
                    bgDl->AddRectFilledMultiColor(
                        ImVec2(0, 0), ImVec2(ds.x, ds.y * 0.18f),
                        IM_COL32(0, 0, 0, vEdge), IM_COL32(0, 0, 0, vEdge),
                        IM_COL32(0, 0, 0, 0),     IM_COL32(0, 0, 0, 0));
                    // Bottom
                    bgDl->AddRectFilledMultiColor(
                        ImVec2(0, ds.y * 0.82f), ds,
                        IM_COL32(0, 0, 0, 0),     IM_COL32(0, 0, 0, 0),
                        IM_COL32(0, 0, 0, vEdge), IM_COL32(0, 0, 0, vEdge));
                    // Left
                    bgDl->AddRectFilledMultiColor(
                        ImVec2(0, 0), ImVec2(ds.x * 0.12f, ds.y),
                        IM_COL32(0, 0, 0, vEdge), IM_COL32(0, 0, 0, 0),
                        IM_COL32(0, 0, 0, 0),     IM_COL32(0, 0, 0, vEdge));
                    // Right
                    bgDl->AddRectFilledMultiColor(
                        ImVec2(ds.x * 0.88f, 0), ds,
                        IM_COL32(0, 0, 0, 0),     IM_COL32(0, 0, 0, vEdge),
                        IM_COL32(0, 0, 0, vEdge), IM_COL32(0, 0, 0, 0));
                }

                // 3. Slow scanlines — subtle horizontal lines scrolling down
                {
                    float scanA = alpha * 12.f; // very subtle
                    float scanSpeed = 30.f;     // pixels per second
                    float lineSpacing = 4.f;
                    float offset = fmodf(t * scanSpeed, lineSpacing * 2.f);
                    for (float y = -lineSpacing + offset; y < ds.y; y += lineSpacing * 2.f)
                    {
                        bgDl->AddRectFilled(
                            ImVec2(0, y), ImVec2(ds.x, y + 1.f),
                            IM_COL32(0, 0, 0, (int)scanA));
                    }
                }

                // 4. Gentle accent glow — soft radial pulse near screen center
                {
                    float pr = Menu::primaryColor[0];
                    float pg = Menu::primaryColor[1];
                    float pb = Menu::primaryColor[2];
                    float pulse = 0.03f + sinf(t * 0.8f) * 0.015f;
                    int gR = (int)(pr * 255.f * pulse * alpha);
                    int gG = (int)(pg * 255.f * pulse * alpha);
                    int gB = (int)(pb * 255.f * pulse * alpha);
                    float cx = ds.x * 0.5f, cy = ds.y * 0.45f;
                    float rx = ds.x * 0.35f, ry = ds.y * 0.35f;
                    // Approximate radial glow with a large elliptical rect
                    bgDl->AddRectFilledMultiColor(
                        ImVec2(cx - rx, cy - ry), ImVec2(cx + rx, cy + ry),
                        IM_COL32(gR, gG, gB, 0),
                        IM_COL32(gR, gG, gB, 0),
                        IM_COL32(gR, gG, gB, 0),
                        IM_COL32(gR, gG, gB, 0));
                    // Inner brighter zone
                    float ir = rx * 0.4f, iy = ry * 0.4f;
                    int iA = (int)(alpha * 18.f * (0.7f + sinf(t * 0.8f) * 0.3f));
                    bgDl->AddRectFilledMultiColor(
                        ImVec2(cx - ir, cy - iy), ImVec2(cx + ir, cy + iy),
                        IM_COL32(gR, gG, gB, iA),
                        IM_COL32(gR, gG, gB, iA),
                        IM_COL32(gR, gG, gB, iA),
                        IM_COL32(gR, gG, gB, iA));
                }

                Log("[Render] Starting Menu::Render");
                Menu::Render(showMenu);
                Log("[Render] Menu::Render done");
            }

            Log("[Render] Starting Menu::RenderHUD");
            Menu::RenderHUD();  // always-on HUD: visible in-game even when menu is closed
            // RenderVacWatermark() removed per UI redesign (top-left "VAC Heartbeat" was distracting)
            Log("[Render] Menu::RenderHUD done");

            // Push Tahoma Bold for all ESP/world overlays
            if (Menu::espFont) ImGui::PushFont(Menu::espFont);

            Log("[Render] Starting ESP");
            ESP::previewMode = showMenu;
            ESP::Render();
            Log("[Render] ESP done, starting Aimbot");
            Aimbot::RenderFovCircle();
            Log("[Render] Aimbot done, starting BulletTracer");
            BulletTracer::Render();
            Log("[Render] BulletTracer done, starting DamageIndicator");
            DamageIndicator::Render();
            Log("[Render] DamageIndicator done, starting GrenadePrediction");
            GrenadePrediction::Render();
            Log("[Render] GrenadePrediction done, starting NadeHelper");
            NadeHelper::Render();
            Log("[Render] NadeHelper done, starting RankRevealer");
            RankRevealer::RenderPanel();
            Log("[Render] RankRevealer done, starting new renders");
            Backtrack::Render();
            SoundESP::Render();
            Bhop::RenderVelocity();
            Crosshair::Render();
            Log("[Render] All renders complete");

            if (Menu::espFont) ImGui::PopFont();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[PresentCore] Render EXCEPTION");
        }

        __try {
            ImGui::Render();
            if (pRTV && pContext)
            {
                pContext->OMSetRenderTargets(1, &pRTV, nullptr);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            }
            else
            {
                // Render target lost (resize race, device reset, etc.)
                // Try to recreate it for the next frame
                if (!pRTV) CreateRT(sc);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[PresentCore] ImGui render EXCEPTION — recreating RT");
            DestroyRT();
            CreateRT(sc);
        }
    }

    inline HRESULT __stdcall hkPresent(IDXGISwapChain* sc, UINT sync, UINT flags)
    {
        __try { PresentCore(sc); }
        __except (EXCEPTION_EXECUTE_HANDLER) { Log("[hkPresent] EXCEPTION in PresentCore"); }
        return oPresent(sc, sync, flags);
    }

    inline HRESULT __stdcall hkPresent1(IDXGISwapChain1* sc1, UINT sync, UINT flags,
                                         const DXGI_PRESENT_PARAMETERS* params)
    {
        __try { PresentCore(static_cast<IDXGISwapChain*>(sc1)); }
        __except (EXCEPTION_EXECUTE_HANDLER) { Log("[hkPresent1] EXCEPTION in PresentCore"); }
        return oPresent1(sc1, sync, flags, params);
    }

    // ---------------------------------------------------------------
    // ResizeBuffers hook
    // ---------------------------------------------------------------
    inline HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* sc, UINT bc, UINT w, UINT h,
                                              DXGI_FORMAT fmt, UINT fl)
    {
        DestroyRT();
        HRESULT hr = oResizeBuffers(sc, bc, w, h, fmt, fl);
        CreateRT(sc);
        return hr;
    }

    // ---------------------------------------------------------------
    // Setup — dummy device to grab Present/ResizeBuffers addresses,
    // then MinHook detours the shared function code in dxgi.dll.
    // ---------------------------------------------------------------
    inline bool SetupDX()
    {
        static volatile bool dxSetupDone = false;
        if (dxSetupDone) { Log("[*] SetupDX already completed — skipping"); return true; }

        Log("[*] SetupDX starting");

        WNDCLASSEXW wc{};
        wc.cbSize        = sizeof(wc);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = DefWindowProcW;
        wc.hInstance      = reinterpret_cast<HINSTANCE>(
                                Stealth::FindModule(L"cs2.exe"));
        if (!wc.hInstance)
            wc.hInstance  = GetModuleHandleW(nullptr);
        // Random class name each session so it's not a static signature
        wchar_t className[16];
        {
            LARGE_INTEGER pc; QueryPerformanceCounter(&pc);
            uint32_t r = (uint32_t)(pc.QuadPart ^ GetTickCount());
            for (int i = 0; i < 8; ++i) {
                r = r * 1664525u + 1013904223u;
                className[i] = L'a' + (r >> 16) % 26;
            }
            className[8] = 0;
        }
        wc.lpszClassName = className;
        
        Log("[*] hInstance=%p", (void*)wc.hInstance);
        ATOM atom = RegisterClassExW(&wc);
        Log("[*] RegisterClassExW atom=%u err=%lu", (unsigned)atom, atom ? 0 : GetLastError());

        HWND tmpWnd = CreateWindowExW(
            0, className, L"", WS_OVERLAPPEDWINDOW,
            0, 0, 2, 2, nullptr, nullptr, wc.hInstance, nullptr);
        if (!tmpWnd)
        {
            Log("[-] CreateWindowExW failed: err=%lu", GetLastError());
            UnregisterClassW(className, wc.hInstance);
            return false;
        }
        Log("[*] tmpWnd=%p", (void*)tmpWnd);

        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount       = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow      = tmpWnd;
        sd.SampleDesc.Count  = 1;
        sd.Windowed          = TRUE;
        sd.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        IDXGISwapChain*      tmpSC  = nullptr;
        ID3D11Device*        tmpDev = nullptr;
        ID3D11DeviceContext* tmpCtx = nullptr;
        D3D_FEATURE_LEVEL    fl;

        Log("[*] Calling D3D11CreateDeviceAndSwapChain (FLIP_DISCARD)");
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION,
            &sd, &tmpSC, &tmpDev, &fl, &tmpCtx);
        if (FAILED(hr))
        {
            Log("[*] FLIP_DISCARD failed (0x%08lX), falling back to DISCARD", hr);
            sd.BufferCount = 1;
            sd.SwapEffect  = DXGI_SWAP_EFFECT_DISCARD;
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                nullptr, 0, D3D11_SDK_VERSION,
                &sd, &tmpSC, &tmpDev, &fl, &tmpCtx);
        }
        if (FAILED(hr))
        {
            char buf[64];
            sprintf_s(buf, "[-] D3D11Create failed: 0x%08lX", hr);
            Log(buf);
            DestroyWindow(tmpWnd);
            UnregisterClassW(className, wc.hInstance);
            return false;
        }
        Log("[*] D3D11 device created OK");

        // Read function pointers from the dummy's vtable.
        // Present = index 8, ResizeBuffers = index 13.
        void** scVT = *reinterpret_cast<void***>(tmpSC);
        pHkPresent = scVT[8];
        pHkResize  = scVT[13];
        Log("[*] Present=%p Resize=%p", pHkPresent, pHkResize);

        // Present1 = IDXGISwapChain1 vtable index 22
        // CS2 uses DXGI 1.2+ flip model and may call Present1 instead of Present
        IDXGISwapChain1* tmpSC1 = nullptr;
        hr = tmpSC->QueryInterface(__uuidof(IDXGISwapChain1), (void**)&tmpSC1);
        if (SUCCEEDED(hr) && tmpSC1)
        {
            void** sc1VT = *reinterpret_cast<void***>(tmpSC1);
            pHkPresent1 = sc1VT[22];
            Log("[*] Present1=%p", pHkPresent1);
            // If Present and Present1 are the same address, only hook once
            if (pHkPresent1 == pHkPresent)
            {
                Log("[*] Present1 == Present (same function), skipping duplicate hook");
                pHkPresent1 = nullptr;
            }
            tmpSC1->Release();
        }
        else
        {
            Log("[*] IDXGISwapChain1 not available (pre-DXGI 1.2)");
        }

        // DrawIndexed = device context vtable index 12
        void** ctxVT = *reinterpret_cast<void***>(tmpCtx);
        pHkDrawIndexed = ctxVT[12];
        Log("[*] DrawIndexed=%p", pHkDrawIndexed);

        tmpSC->Release();
        tmpDev->Release();
        tmpCtx->Release();
        DestroyWindow(tmpWnd);
        UnregisterClassW(className, wc.hInstance);

        // ---------------------------------------------------------
        // STEALTH: Only MinHook Present/Present1 as a 1-frame
        // bootstrap. All other hooks use VMT shadow-copy swap,
        // which never modifies executable code sections.
        // ---------------------------------------------------------
        Log("[*] Creating MinHook bootstrap (Present only)");
        MH_STATUS st;
        st = MH_CreateHook(pHkPresent, &hkPresent,
                          reinterpret_cast<void**>(&oPresent));
        if (st != MH_OK && st != MH_ERROR_ALREADY_CREATED)
        {
            Log("[-] MH_CreateHook(Present) failed: %d", st);
            return false;
        }

        // Present1 bootstrap (CS2 may call Present1 instead)
        if (pHkPresent1)
        {
            st = MH_CreateHook(pHkPresent1, &hkPresent1,
                              reinterpret_cast<void**>(&oPresent1));
            if (st == MH_OK || st == MH_ERROR_ALREADY_CREATED)
                Log("[+] Present1 bootstrap hook created");
            else
                Log("[-] MH_CreateHook(Present1) failed: %d", st);
        }

        // DrawIndexed, DrawIndexedInstanced, ResizeBuffers
        // — hooked via VMT swap once we get the real swap chain & context.

        st = MH_EnableHook(MH_ALL_HOOKS);
        if (st != MH_OK && st != MH_ERROR_ENABLED)
        {
            Log("[-] MH_EnableHook failed: %d", st);
            return false;
        }
        Log("[+] Present bootstrap hook installed (MinHook)");

        // Create wireframe rasterizer state (deferred until we have the real device)
        // Will be created on first Present call when pDevice is available.

        dxSetupDone = true;
        return true;
    }

    // ---------------------------------------------------------------
    // Cleanup
    // ---------------------------------------------------------------
    inline void Shutdown()
    {
        // Restore VMT shadow-copy hooks first (before releasing COM objects)
        vmtSwapChain.Restore();
        vmtContext.Restore();
        vmtSource2Client.Restore();
        vmtActive = false;

        if (inited)
        {
            if (oWndProc && hwndGame)
                SetWindowLongPtrW(hwndGame, GWLP_WNDPROC,
                                  reinterpret_cast<LONG_PTR>(oWndProc));
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            DestroyRT();
            if (wireframeRS) { wireframeRS->Release(); wireframeRS = nullptr; }
            if (normalRS)    { normalRS->Release();    normalRS = nullptr; }
            Chams::Shutdown();
            WeaponIcons::Shutdown();
            if (pContext) { pContext->Release(); pContext = nullptr; }
            if (pDevice)  { pDevice->Release();  pDevice = nullptr; }
            inited = false;
        }

        // Clean up MinHook bootstrap remnants (safe even if already disabled)
        if (pHkPresent)  { MH_DisableHook(pHkPresent);  MH_RemoveHook(pHkPresent);  }
        if (pHkPresent1) { MH_DisableHook(pHkPresent1); MH_RemoveHook(pHkPresent1); }
        oPresent          = nullptr;
        oPresent1         = nullptr;
        oResizeBuffers    = nullptr;
        oDrawIndexed      = nullptr;
    }
}
