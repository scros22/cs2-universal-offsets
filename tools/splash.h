// splash.h — Frameless Direct2D loader window for the Lucid injector.
//
// Mirrors the liquid-glass aesthetic of the in-game menu (render/menu.h):
//   • Squared rounded body (HRGN-clipped to ~12px radius)
//   • Translucent dark base + vertical sheen + horizontal cross-sheen
//   • 1px white inner top highlight + 1px black inner bottom shadow
//   • Vertical accent bar (light red) on the left edge
//   • Big "LUCID v1.0" version text in light pink-red with an inky outline
//   • Subtitle "CS2 Loader" + a single live status line beneath it
//
// Threading model: Show() spawns a dedicated UI thread that owns the
// HWND + D2D resources and pumps the message loop. Status()/Done()/Close()
// are safe to call from any thread; they post messages to the UI thread
// which mutates render state under a SRWLOCK and invalidates.
//
// Auto-close: Done(true)  schedules a 900ms close (success — quick)
//             Done(false) schedules a 4500ms close (failure — readable)
//             Close()     closes immediately.

#pragma once

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <dwmapi.h>
#include <wrl/client.h>
#include <atomic>
#include <cmath>
#include <string>
#include <thread>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdi32.lib")

namespace Splash {

namespace detail {

using Microsoft::WRL::ComPtr;

inline constexpr wchar_t kClassName[] = L"LucidSplashWnd";
inline constexpr int     kWinW         = 480;
inline constexpr int     kWinH         = 180;
inline constexpr int     kRadius       = 14;

inline constexpr UINT WM_SP_STATUS = WM_APP + 1;
inline constexpr UINT WM_SP_DONE   = WM_APP + 2;
inline constexpr UINT WM_SP_CLOSE  = WM_APP + 3;

struct State {
    HWND               hwnd        = nullptr;
    DWORD              uiThreadId  = 0;
    std::thread        uiThread;
    std::atomic<bool>  ready{false};

    // protected by lock — touched from any thread, read on UI thread
    SRWLOCK            lock = SRWLOCK_INIT;
    std::wstring       status      = L"Initializing...";
    int                doneState   = 0;       // 0 = running, 1 = ok, -1 = fail

    // D2D + DWrite (UI thread only)
    ComPtr<ID2D1Factory>          d2d;
    ComPtr<ID2D1HwndRenderTarget> rt;
    ComPtr<IDWriteFactory>        dw;
    ComPtr<IDWriteTextFormat>     fmtVersion;   // big "LUCID v1.0"
    ComPtr<IDWriteTextFormat>     fmtSubtitle;  // "CS2 Loader"
    ComPtr<IDWriteTextFormat>     fmtStatus;    // live status line
    ComPtr<IDWriteTextFormat>     fmtFooter;    // "PRESS ANY KEY" prompt
};

inline State& S() { static State s; return s; }

// ---------- D2D helpers ----------

inline void CreateDeviceResources()
{
    State& s = S();
    if (s.rt) return;

    RECT rc; GetClientRect(s.hwnd, &rc);
    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
    props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                                          D2D1_ALPHA_MODE_PREMULTIPLIED);

    s.d2d->CreateHwndRenderTarget(
        props,
        D2D1::HwndRenderTargetProperties(s.hwnd, size,
            D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &s.rt);
}

inline void DiscardDeviceResources()
{
    S().rt.Reset();
}

// Draw the liquid-glass body: layered fills that mirror the menu rail.
inline void DrawGlass(ID2D1RenderTarget* rt, const D2D1_RECT_F& r)
{
    // 1) translucent dark base
    ComPtr<ID2D1SolidColorBrush> base;
    rt->CreateSolidColorBrush(D2D1::ColorF(0.039f, 0.039f, 0.051f, 0.96f), &base);
    rt->FillRectangle(r, base.Get());

    // 2) vertical sheen (white gradient, very low alpha)
    {
        D2D1_GRADIENT_STOP stops[3] = {
            { 0.00f, D2D1::ColorF(1.f, 1.f, 1.f, 0.018f) },
            { 0.55f, D2D1::ColorF(1.f, 1.f, 1.f, 0.055f) },
            { 1.00f, D2D1::ColorF(1.f, 1.f, 1.f, 0.010f) },
        };
        ComPtr<ID2D1GradientStopCollection> coll;
        rt->CreateGradientStopCollection(stops, 3, &coll);
        ComPtr<ID2D1LinearGradientBrush> lin;
        rt->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties({ r.left, r.top }, { r.left, r.bottom }),
            coll.Get(), &lin);
        rt->FillRectangle(r, lin.Get());
    }

    // 3) horizontal cross-sheen (left dimmer, right brighter — fakes refraction)
    {
        D2D1_GRADIENT_STOP stops[2] = {
            { 0.f, D2D1::ColorF(1.f, 1.f, 1.f, 0.000f) },
            { 1.f, D2D1::ColorF(1.f, 1.f, 1.f, 0.030f) },
        };
        ComPtr<ID2D1GradientStopCollection> coll;
        rt->CreateGradientStopCollection(stops, 2, &coll);
        ComPtr<ID2D1LinearGradientBrush> lin;
        rt->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties({ r.left, r.top }, { r.right, r.top }),
            coll.Get(), &lin);
        rt->FillRectangle(r, lin.Get());
    }

    // 4) inner top highlight (1px) + inner bottom shadow (1px)
    {
        ComPtr<ID2D1SolidColorBrush> hi, lo;
        rt->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 1.f, 0.10f), &hi);
        rt->CreateSolidColorBrush(D2D1::ColorF(0.f, 0.f, 0.f, 0.32f), &lo);
        rt->DrawLine({ r.left + 1.f, r.top + 1.f },
                     { r.right - 1.f, r.top + 1.f }, hi.Get(), 1.f);
        rt->DrawLine({ r.left + 1.f, r.bottom - 1.f },
                     { r.right - 1.f, r.bottom - 1.f }, lo.Get(), 1.f);
    }

    // 5) crisp 1px outer border
    {
        ComPtr<ID2D1SolidColorBrush> bd;
        rt->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 1.f, 0.13f), &bd);
        D2D1_RECT_F br = { r.left + 0.5f, r.top + 0.5f,
                           r.right - 0.5f, r.bottom - 0.5f };
        rt->DrawRectangle(br, bd.Get(), 1.f);
    }

    // 6) accent bar (left edge, light pink-red)
    {
        ComPtr<ID2D1SolidColorBrush> acc;
        rt->CreateSolidColorBrush(D2D1::ColorF(0.898f, 0.224f, 0.208f, 0.95f), &acc);
        D2D1_RECT_F bar = { r.left + 14.f, r.top + 22.f,
                            r.left + 16.f, r.bottom - 22.f };
        rt->FillRectangle(bar, acc.Get());
    }
}

// Draw a string with a 1px 8-direction dark outline, then fill on top.
inline void DrawOutlinedText(ID2D1RenderTarget* rt, IDWriteTextFormat* fmt,
                              const wchar_t* text, D2D1_RECT_F box,
                              D2D1::ColorF fill, float outlineAlpha = 0.85f)
{
    ComPtr<ID2D1SolidColorBrush> ink, body;
    rt->CreateSolidColorBrush(D2D1::ColorF(0.f, 0.f, 0.f, outlineAlpha), &ink);
    rt->CreateSolidColorBrush(fill, &body);

    UINT32 len = (UINT32)wcslen(text);
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            D2D1_RECT_F off = { box.left + dx, box.top + dy,
                                box.right + dx, box.bottom + dy };
            rt->DrawText(text, len, fmt, off, ink.Get(),
                         D2D1_DRAW_TEXT_OPTIONS_NONE);
        }
    rt->DrawText(text, len, fmt, box, body.Get(),
                 D2D1_DRAW_TEXT_OPTIONS_NONE);
}

inline void Render()
{
    State& s = S();
    if (!s.rt) CreateDeviceResources();
    if (!s.rt) return;

    s.rt->BeginDraw();
    s.rt->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));

    D2D1_RECT_F body = { 0.f, 0.f, (float)kWinW, (float)kWinH };
    DrawGlass(s.rt.Get(), body);

    // ---- text ----
    // Big version line ("LUCID v1.0") — light pink-red w/ dark outline
    DrawOutlinedText(
        s.rt.Get(), s.fmtVersion.Get(),
        L"LUCID  v1.0",
        D2D1::RectF(34.f, 24.f, (float)kWinW - 24.f, 70.f),
        D2D1::ColorF(1.f, 0.541f, 0.541f, 1.f),  // #FF8A8A
        0.78f);

    // Subtitle — small tracked uppercase muted-red tag (matches menu chips)
    {
        ComPtr<ID2D1SolidColorBrush> sub;
        s.rt->CreateSolidColorBrush(
            D2D1::ColorF(0.898f, 0.224f, 0.208f, 0.85f), &sub);
        const wchar_t* t = L"C S 2   L O A D E R";
        s.rt->DrawText(t, (UINT32)wcslen(t), s.fmtSubtitle.Get(),
                       D2D1::RectF(36.f, 72.f, (float)kWinW - 24.f, 96.f),
                       sub.Get());
    }

    // Header accent stub (matches cfg-panel red strip)
    {
        ComPtr<ID2D1SolidColorBrush> stub;
        s.rt->CreateSolidColorBrush(
            D2D1::ColorF(0.898f, 0.224f, 0.208f, 0.95f), &stub);
        s.rt->FillRectangle(D2D1::RectF(34.f, 100.f, 62.f, 102.f), stub.Get());
    }

    // Hairline separator
    {
        ComPtr<ID2D1SolidColorBrush> sep;
        s.rt->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 1.f, 0.10f), &sep);
        s.rt->DrawLine({ 64.f, 101.f }, { (float)kWinW - 24.f, 101.f },
                       sep.Get(), 1.f);
    }

    // Status line — color tracks doneState
    {
        std::wstring text;
        int state;
        AcquireSRWLockShared(&s.lock);
        text = s.status; state = s.doneState;
        ReleaseSRWLockShared(&s.lock);

        D2D1::ColorF c =
            (state == 1)  ? D2D1::ColorF(0.45f, 0.85f, 0.55f, 1.f)   // green
          : (state == -1) ? D2D1::ColorF(1.f,   0.541f, 0.541f, 1.f) // red
                          : D2D1::ColorF(0.84f, 0.84f, 0.88f, 1.f);  // neutral

        // Status dot
        ComPtr<ID2D1SolidColorBrush> dot;
        s.rt->CreateSolidColorBrush(c, &dot);
        s.rt->FillEllipse(D2D1::Ellipse({ 42.f, 124.f }, 3.5f, 3.5f), dot.Get());

        s.rt->DrawText(text.c_str(), (UINT32)text.size(),
                       s.fmtStatus.Get(),
                       D2D1::RectF(54.f, 115.f, (float)kWinW - 24.f, 139.f),
                       dot.Get());
    }

    // Footer hint — only shown once injection has finished
    {
        int state;
        AcquireSRWLockShared(&s.lock);
        state = s.doneState;
        ReleaseSRWLockShared(&s.lock);

        if (state != 0) {
            // Subtle pulse so the prompt feels alive without being noisy
            DWORD t = GetTickCount();
            float pulse = 0.55f + 0.25f * (float)sin(t * 0.004);
            ComPtr<ID2D1SolidColorBrush> ft;
            s.rt->CreateSolidColorBrush(
                D2D1::ColorF(0.898f, 0.224f, 0.208f, pulse), &ft);
            const wchar_t* t1 = L"PRESS ANY KEY TO CLOSE";
            s.rt->DrawText(t1, (UINT32)wcslen(t1), s.fmtFooter.Get(),
                           D2D1::RectF(34.f, (float)kWinH - 24.f,
                                       (float)kWinW - 24.f, (float)kWinH - 6.f),
                           ft.Get());
        }
    }

    HRESULT hr = s.rt->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) DiscardDeviceResources();
}

inline LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    State& s = S();
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps; BeginPaint(h, &ps);
        Render();
        EndPaint(h, &ps);
        return 0;
    }
    case WM_SIZE:
        if (s.rt) {
            D2D1_SIZE_U sz = D2D1::SizeU(LOWORD(lp), HIWORD(lp));
            s.rt->Resize(sz);
        }
        return 0;
    case WM_DISPLAYCHANGE:
        InvalidateRect(h, nullptr, FALSE);
        return 0;
    case WM_TIMER:
        if (wp == 1) {                 // ~30 fps repaint while running
            InvalidateRect(h, nullptr, FALSE);
        }
        return 0;
    case WM_SP_STATUS:
        InvalidateRect(h, nullptr, FALSE);
        return 0;
    case WM_SP_DONE:
        // Take focus so any key/click can dismiss us
        SetForegroundWindow(h);
        SetFocus(h);
        InvalidateRect(h, nullptr, FALSE);
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN: {
        int state;
        AcquireSRWLockShared(&s.lock);
        state = s.doneState;
        ReleaseSRWLockShared(&s.lock);
        if (state != 0) DestroyWindow(h);
        return 0;
    }
    case WM_SP_CLOSE:
        DestroyWindow(h);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_NCHITTEST:
        return HTCLIENT;   // frameless, non-draggable
    }
    return DefWindowProcW(h, msg, wp, lp);
}

inline void UiThreadMain()
{
    State& s = S();
    s.uiThreadId = GetCurrentThreadId();

    HINSTANCE hi = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.lpszClassName = kClassName;
    wc.hCursor       = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = nullptr;       // we paint everything via D2D
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    RegisterClassExW(&wc);

    int sx = GetSystemMetrics(SM_CXSCREEN);
    int sy = GetSystemMetrics(SM_CYSCREEN);
    int x = (sx - kWinW) / 2;
    int y = (sy - kWinH) / 2;

    HWND h = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        kClassName, L"LUCID Loader",
        WS_POPUP,
        x, y, kWinW, kWinH,
        nullptr, nullptr, hi, nullptr);
    if (!h) { s.ready = true; return; }

    // rounded corners (matches menu chrome)
    HRGN rgn = CreateRoundRectRgn(0, 0, kWinW + 1, kWinH + 1,
                                   kRadius * 2, kRadius * 2);
    SetWindowRgn(h, rgn, FALSE);

    // soft drop shadow behind us (DWM)
    {
        BOOL dwmOk = TRUE;
        DwmSetWindowAttribute(h, DWMWA_TRANSITIONS_FORCEDISABLED, &dwmOk, sizeof(dwmOk));
        MARGINS m = { 0, 0, 0, 1 };
        DwmExtendFrameIntoClientArea(h, &m);
    }

    s.hwnd = h;

    // D2D / DWrite factories
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                      __uuidof(ID2D1Factory),
                      (void**)s.d2d.GetAddressOf());

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                        __uuidof(IDWriteFactory),
                        (IUnknown**)s.dw.GetAddressOf());

    if (s.dw) {
        s.dw->CreateTextFormat(L"Segoe UI", nullptr,
            DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 28.f, L"en-us",
            &s.fmtVersion);
        s.dw->CreateTextFormat(L"Segoe UI", nullptr,
            DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 13.f, L"en-us",
            &s.fmtSubtitle);
        s.dw->CreateTextFormat(L"Segoe UI", nullptr,
            DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 12.f, L"en-us",
            &s.fmtStatus);
        s.dw->CreateTextFormat(L"Segoe UI", nullptr,
            DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 10.f, L"en-us",
            &s.fmtFooter);
        if (s.fmtFooter) {
            s.fmtFooter->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        }
    }

    ShowWindow(h, SW_SHOWNORMAL);
    SetForegroundWindow(h);
    SetFocus(h);
    SetTimer(h, 1, 33, nullptr);   // ~30 fps repaint
    s.ready = true;

    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }

    DiscardDeviceResources();
    s.dw.Reset();
    s.d2d.Reset();
    s.fmtVersion.Reset();
    s.fmtSubtitle.Reset();
    s.fmtStatus.Reset();
    s.fmtFooter.Reset();
    s.hwnd = nullptr;
}

} // namespace detail

// ---------- Public API ----------

inline void Show()
{
    auto& s = detail::S();
    if (s.uiThread.joinable()) return;
    s.uiThread = std::thread(detail::UiThreadMain);
    // wait until window exists
    for (int i = 0; i < 200 && !s.ready.load(); ++i) Sleep(5);
}

inline void Status(const wchar_t* text)
{
    auto& s = detail::S();
    if (!text) return;
    AcquireSRWLockExclusive(&s.lock);
    s.status = text;
    ReleaseSRWLockExclusive(&s.lock);
    if (s.hwnd) PostMessageW(s.hwnd, detail::WM_SP_STATUS, 0, 0);
}

inline void Status(const char* text)
{
    if (!text) return;
    int n = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (n <= 0) return;
    std::wstring w(n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, &w[0], n);
    Status(w.c_str());
}

inline void Done(bool success)
{
    auto& s = detail::S();
    AcquireSRWLockExclusive(&s.lock);
    s.doneState = success ? 1 : -1;
    if (success && s.status == L"Initializing...") s.status = L"Injection complete.";
    ReleaseSRWLockExclusive(&s.lock);
    if (s.hwnd) PostMessageW(s.hwnd, detail::WM_SP_DONE, success ? 1 : 0, 0);
}

inline void Close()
{
    auto& s = detail::S();
    if (s.hwnd) PostMessageW(s.hwnd, detail::WM_SP_CLOSE, 0, 0);
}

inline void Wait()
{
    auto& s = detail::S();
    if (s.uiThread.joinable()) s.uiThread.join();
}

} // namespace Splash
