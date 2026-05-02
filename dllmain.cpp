// dllmain.cpp â€” CS2 DLL entry
//
// Uses a CUSTOM ENTRY POINT (RawDllEntry) set via /ENTRY linker flag.
// This runs BEFORE CRT init â€” critical for manual-map thread hijack
// injectors that may wipe PE headers while _DllMainCRTStartup is still
// reading them. We cache what we need, init CRT ourselves, then proceed.
//
#include <Windows.h>
#include <Psapi.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <atomic>
#include <process.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "vendor/minhook/include/MinHook.h"
#include "core/xorstr.h"
#include "core/stealth.h"
#include "core/vac_watchdog.h"
#include "core/syscall.h"
#include "core/spoof_call.h"
#include "core/game_state.h"
#include "features/combat/aimbot.h"
#include "features/visuals/world_effects.h"
#include "features/visuals/chams.h"
#include "features/misc/kill_sound.h"
#include "features/movement/bhop.h"
#include "features/skins/skinchanger.h"
#include "features/skins/inventory_changer.h"
#include "features/skins/model_changer.h"
#include "features/misc/crosshair.h"
// Seeded-triggerbot research probe â€” superseded by Triggerbot::Setup
// (features/triggerbot.h) which owns the spread hook directly. Kept
// in tree (disabled) for future re-enable when investigating new
// Valve changes to the spread RNG path.
//#define LUCID_SEED_PROBE 1
//#include "features/seed_probe.h"
#include "render/hooks.h"

#pragma comment(lib, "Psapi.lib")

// Forward declare CRT init (available with static /MT CRT)
extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

static HMODULE g_hModule = nullptr;
static SIZE_T  g_cachedImageSize = 0; // forward â€” set in RawDllEntry
static std::atomic<bool> g_running{false};

// ---------------------------------------------------------------
// File logger â€” DEBUG ONLY.
//
// Anti-cheat heuristics weight "injected process writes named files
// into %TEMP% on a heartbeat" pretty heavily toward the untrusted-mode
// (20-hour) cooldown. Release builds therefore never touch disk; the
// function still exists and accepts variadic args so call sites need
// no #ifdef churn, it just no-ops.
// ---------------------------------------------------------------
static void DllLog(const char* fmt, ...)
{
#ifdef _DEBUG
    char path[MAX_PATH];
    GetTempPathA(MAX_PATH, path);
    lstrcatA(path, "cs2init.txt");
    HANDLE hFile = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ,
                               nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return;
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    int len = vsprintf_s(buf, sizeof(buf) - 2, fmt, args);
    va_end(args);
    if (len > 0)
    {
        buf[len] = '\n';
        buf[len + 1] = '\0';
        DWORD written;
        WriteFile(hFile, buf, len + 1, &written, nullptr);
    }
    CloseHandle(hFile);
#else
    (void)fmt;
#endif
}

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep)
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    // Skip harmless debug output exceptions + first-chance C++ exceptions
    if (code == 0x40010006 || code == 0x4001000A || code == 0x406D1388 ||
        code == 0xE06D7363) // C++ exception
        return EXCEPTION_CONTINUE_SEARCH;

#ifdef _DEBUG
    // Crash log to %TEMP%\cs2crash.txt â€” DEBUG ONLY (named temp files in
    // a release build are an untrusted-mode signal). Release silently
    // continues and lets SEH unwind normally.
    char path[MAX_PATH];
    GetTempPathA(MAX_PATH, path);
    lstrcatA(path, "cs2crash.txt");
    HANDLE hFile = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ,
                               nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        HMODULE hMod = nullptr;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)ep->ContextRecord->Rip, &hMod);
        uintptr_t rip = ep->ContextRecord->Rip;
        uintptr_t ourBase = reinterpret_cast<uintptr_t>(g_hModule);
        bool inOurImage = ourBase && g_cachedImageSize &&
                          rip >= ourBase && rip < (ourBase + g_cachedImageSize);
        char modName[128] = "<unmapped>";
        if (hMod)
            GetModuleFileNameA(hMod, modName, sizeof(modName));
        else if (inOurImage)
            lstrcpyA(modName, "<lucid.dll>");
        char buf[512];
        int len = wsprintfA(buf, "CRASH: code=0x%08X RIP=0x%p RSP=0x%p mod=%s",
            code, (void*)ep->ContextRecord->Rip,
            (void*)ep->ContextRecord->Rsp, modName);
        if (code == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2)
        {
            const char* op = "???";
            switch (ep->ExceptionRecord->ExceptionInformation[0]) {
                case 0: op = "READ"; break;
                case 1: op = "WRITE"; break;
                case 8: op = "DEP/EXEC"; break;
            }
            len += wsprintfA(buf + len, " AV=%s@0x%p", op,
                (void*)ep->ExceptionRecord->ExceptionInformation[1]);
        }
        buf[len++] = '\r'; buf[len++] = '\n';
        DWORD written;
        WriteFile(hFile, buf, len, &written, nullptr);
        CloseHandle(hFile);
    }
#endif

    // Let SEH (__try/__except) handle exceptions in our code normally.
    // NEVER attempt stack manipulation recovery â€” it corrupts the stack
    // for manual-mapped code that isn't in the PEB module list.
    return EXCEPTION_CONTINUE_SEARCH;
}

static void EntryThread(HMODULE hModule)
{
#ifdef _DEBUG
    // Truncate previous session's debug logs for clean diagnostics
    {
        char tmp[MAX_PATH];
        GetTempPathA(MAX_PATH, tmp);
        const char* names[] = { "cs2init.txt", "cs2dbg.txt", "cs2crash.txt" };
        for (auto n : names) {
            char p[MAX_PATH]; lstrcpyA(p, tmp); lstrcatA(p, n);
            HANDLE h = CreateFileA(p, GENERIC_WRITE, 0, nullptr,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        }
    }
#endif

    DllLog("[EntryThread] Started. hModule=%p", hModule);

    // Spoof thread start address â€” scanners see kernel32 instead of unbacked memory
    using NtSetInfoThread = NTSTATUS(NTAPI*)(HANDLE, ULONG, PVOID, ULONG);
    auto ntSit = reinterpret_cast<NtSetInfoThread>(
        GetProcAddress(GetModuleHandleW(XW(L"ntdll.dll")), XS("NtSetInformationThread")));
    if (ntSit) {
        PVOID fake = reinterpret_cast<PVOID>(
            GetProcAddress(GetModuleHandleW(XW(L"kernel32.dll")), XS("BaseThreadInitThunk")));
        ntSit(GetCurrentThread(), 9 /*ThreadQuerySetWin32StartAddress*/, &fake, sizeof(fake));
    }
    // Hide thread from debugger/scanner enumeration (ThreadHideFromDebugger)
    if (ntSit)
        ntSit(GetCurrentThread(), 0x11 /*ThreadHideFromDebugger*/, nullptr, 0);
    DllLog("[EntryThread] Thread start addr spoofed + HideFromDebugger set");

    // Random jitter before init â€” avoids timing-based signatures
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    int jitterMs = 200 + static_cast<int>(pc.QuadPart % 800);
    DllLog("[EntryThread] Jitter sleep %d ms", jitterMs);
    std::this_thread::sleep_for(std::chrono::milliseconds(jitterMs));

    // ---- Stealth init â€” must be first ----
    // Install crash handler
    AddVectoredExceptionHandler(1, CrashHandler);
    DllLog("[EntryThread] Crash handler installed");

    // Register exception function table for x64 table-based SEH.
    // Without this, __try/__except blocks in manual-mapped code are
    // NON-FUNCTIONAL â€” the OS can't find RUNTIME_FUNCTION entries to
    // unwind our stack frames, so ALL exceptions are "unhandled" and
    // the unwinder corrupts the stack walking through garbage addresses.
    {
        auto base = reinterpret_cast<uint8_t*>(g_hModule);
        auto dos  = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        if (dos && dos->e_magic == IMAGE_DOS_SIGNATURE)
        {
            auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
            auto& excDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
            if (excDir.VirtualAddress && excDir.Size)
            {
                auto pFuncs = reinterpret_cast<PRUNTIME_FUNCTION>(base + excDir.VirtualAddress);
                DWORD count = excDir.Size / (DWORD)sizeof(RUNTIME_FUNCTION);
                if (RtlAddFunctionTable(pFuncs, count, (DWORD64)base))
                    DllLog("[EntryThread] Exception table registered (%u entries) â€” SEH enabled", count);
                else
                    DllLog("[EntryThread] RtlAddFunctionTable FAILED â€” SEH will NOT work");
            }
            else
                DllLog("[EntryThread] No .pdata section â€” SEH unavailable");
        }
    }

    DllLog("[EntryThread] Calling Stealth::Init");
    if (!Stealth::Init(hModule))
    {
        DllLog("[EntryThread] Stealth::Init FAILED â€” aborting");
        return;
    }
    DllLog("[EntryThread] Stealth::Init OK");

    // Init direct syscall stubs (bypass ntdll hooks)
    if (DirectSyscall::Init())
        DllLog("[EntryThread] Direct syscalls ready");
    else
        DllLog("[EntryThread] Direct syscalls FAILED â€” using fallback APIs");

    // Init return address spoofing (gadget trampoline in ntdll)
    if (SpoofCall::Init())
    {
        SpoofCall::BuildAPIs();
        SpoofCall::Seal();
        DllLog("[EntryThread] Return-address spoof ready");
    }
    else
        DllLog("[EntryThread] Return-address spoof FAILED â€” using direct calls");

#ifdef _DEBUG
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
#endif

    // Wait for critical modules (PEB walk, never touches GetModuleHandle)
    DllLog("[EntryThread] Waiting for client.dll + engine2.dll");
    int waitTicks = 0;
    while (!Stealth::FindModule(XW(L"client.dll")) ||
           !Stealth::FindModule(XW(L"engine2.dll")))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (++waitTicks > 300)
        {
            DllLog("[EntryThread] Module wait TIMED OUT (30s) â€” aborting");
            return;
        }
    }
    DllLog("[EntryThread] Modules found after %d ticks", waitTicks);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    DllLog("[EntryThread] Calling GameState::Init");
    {
        bool gsOk = false;
        for (int gsTry = 0; gsTry < 10; ++gsTry)
        {
            if (GameState::Init()) { gsOk = true; break; }
            DllLog("[EntryThread] GameState::Init attempt %d failed, retrying...", gsTry + 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        if (!gsOk)
        {
            DllLog("[EntryThread] GameState::Init FAILED after 10 attempts â€” aborting");
            return;
        }
    }
    DllLog("[EntryThread] GameState OK: client=%p engine2=%p",
           (void*)GameState::clientBase, (void*)GameState::engine2Base);

    // MinHook must init before DX hooks (which now use MH_CreateHook)
    DllLog("[EntryThread] MH_Initialize");
    {
        MH_STATUS mhStatus = MH_ERROR_NOT_INITIALIZED;
        for (int mhTry = 0; mhTry < 5; ++mhTry)
        {
            mhStatus = MH_Initialize();
            if (mhStatus == MH_OK || mhStatus == MH_ERROR_ALREADY_INITIALIZED) break;
            DllLog("[EntryThread] MH_Initialize attempt %d failed: %d", mhTry + 1, mhStatus);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        if (mhStatus != MH_OK && mhStatus != MH_ERROR_ALREADY_INITIALIZED)
        {
            DllLog("[EntryThread] MH_Initialize FAILED after 5 attempts: %d â€” aborting", mhStatus);
            return;
        }
    }
    DllLog("[EntryThread] MinHook OK");

    // DX11 Present/ResizeBuffers detour via MinHook
    DllLog("[EntryThread] Calling SetupDX");
    {
        bool dxOk = false;
        for (int dxTry = 0; dxTry < 5; ++dxTry)
        {
            if (Hooks::SetupDX()) { dxOk = true; break; }
            DllLog("[EntryThread] SetupDX attempt %d failed, retrying...", dxTry + 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        if (!dxOk)
        {
            DllLog("[EntryThread] SetupDX FAILED after 5 attempts â€” aborting");
            MH_Uninitialize();
            return;
        }
    }
    DllLog("[EntryThread] DX hooks installed");

    // Register emergency cleanup so Heartbeat can restore hooks on detection
    Stealth::SetCleanupCallback([]() {
        InventoryChanger::Shutdown();
        Bhop::Shutdown();
        ModelChanger::Shutdown();
        WorldEffects::Shutdown();
        Hooks::Shutdown();
        VacWatchdog::Shutdown();
        MH_Uninitialize();
        DirectSyscall::Shutdown();
        SpoofCall::Shutdown();
    });

    // Arm VAC untrusted-event watchdog (IDA-verified hook on
    // ClientModeCSNormal::OnEvent â€” sub_180C5A0B0 build 14154).  If
    // tier0.dll / client.dll aren't fully resolved yet, the heartbeat
    // will retry every ~2s.
    if (VacWatchdog::Install())
        DllLog("[EntryThread] VacWatchdog armed");
    else
        DllLog("[EntryThread] VacWatchdog deferred (will retry on heartbeat)");

    DllLog("[EntryThread] Calling Aimbot::Setup");
    if (Aimbot::Setup())
        DllLog("[EntryThread] Aimbot::Setup OK (mouse-input mode, no hooks)");
    else
        DllLog("[EntryThread] Aimbot::Setup FAILED");

    WorldEffects::Setup();

    // Material chams — hooks scenesystem CSceneAnimatableObject::
    // GeneratePrimitives and substitutes KV3-built materials per style.
    if (Chams::Setup())
        DllLog("[EntryThread] Chams::Setup OK (GeneratePrimitives hooked)");
    else
        DllLog("[EntryThread] Chams::Setup FAILED (sig miss or scenesystem/materialsystem2/tier0 not loaded)");

    // Custom kill ding â€” mutes Valve's HS/body AttackerFeedback ding
    // and plays our own short metallic chirp on every confirmed kill.
    if (KillSound::Setup())
        DllLog("[EntryThread] KillSound::Setup OK (custom ding active, Valve ding muted)");
    else
        DllLog("[EntryThread] KillSound::Setup FAILED (sig miss or hook install failed)");

    // Seeded triggerbot â€” resolves the engine seed-gen + spread
    // functions and installs a passive hook on the spread fn to
    // capture per-weapon arg templates. Predictor lives in
    // features/triggerbot.h (Triggerbot::PredictHit).
    {
        int tsr = Triggerbot::Setup();
        if (tsr == 1)
            DllLog("[EntryThread] Triggerbot::Setup OK (seeded predictor armed)");
        else if (tsr == 0)
            DllLog("[EntryThread] Triggerbot::Setup PARTIAL (seed-gen ok, spread hook missed) â€” falling back to accuracy-gate");
        else
            DllLog("[EntryThread] Triggerbot::Setup FAILED (%d) â€” falling back to accuracy-gate", tsr);
    }

    SkinChanger::Init();
    DllLog("[EntryThread] SkinChanger::Init OK");

    // Inventory locker injector (ARCHILIX-style)
    InventoryChanger::Init();
    DllLog("[EntryThread] InventoryChanger::Init done");
    
    // KNIFE/GLOVE CHANGER - DISABLED (ARCHITECTURALLY IMPOSSIBLE)
    // After extensive IDA Pro analysis and testing, knife changing is confirmed IMPOSSIBLE in CS2.
    // Reason: CS2 uses server-authoritative inventory with no client-side fallback for knives.
    // GetItemInLoadout slot 2 returns PISTOLS (defIndex 32 = P2000), not knives.
    // Knives are NOT in the loadout system and are cached at spawn based on Steam inventory.
    // Modifying CEconItemView structures causes crashes (access violations).
    // See FINAL_KNIFE_ANALYSIS.md for full details.
    // 
    // What DOES work:
    // âœ… Weapon skins (AK-47, AWP, M4A4, etc.) - uses fallback system
    // âœ… Glove skins - uses fallback system
    // âŒ Knife models - NO fallback system, server-authoritative
    DllLog("[EntryThread] Knife changer DISABLED - architecturally impossible in CS2");
    DllLog("[EntryThread] Weapon skins and glove skins work perfectly!");
    DllLog("[EntryThread] See FINAL_KNIFE_ANALYSIS.md for full analysis");
    
    Aimbot::Governor::ResetSession();
    Aimbot::ApplySessionJitter();
    DllLog("[EntryThread] Features initialized (jitter: FOV%.2f Smooth%.1f Human%.3f)",
           Aimbot::sessionFovJitter, Aimbot::sessionSmoothJitter, Aimbot::sessionHumanJitter);

    // NOTE: RWXâ†’RX protection removed â€” manual mappers put code+data
    // in a single RWX allocation, so flipping to RX kills all global
    // variable writes and crashes on the next render/game thread frame.

    // Success chime â€” debug builds only (PlaySoundA from unbacked memory is fingerprinted)
#ifdef _DEBUG
    // Soft success chime (3-note ascending, WAV in memory)
    {
        constexpr int sampleRate = 22050;
        constexpr int bitsPerSample = 16;
        constexpr int channels = 1;

        // 3 notes: C5(523Hz), E5(659Hz), G5(784Hz), each ~80ms + 40ms fade gap
        struct Note { float freq; int ms; };
        Note notes[] = { {523.f, 80}, {659.f, 80}, {784.f, 120} };
        int gap = 25; // silence between notes in ms

        // Calculate total samples
        int totalSamples = 0;
        for (auto& n : notes) totalSamples += (sampleRate * n.ms / 1000) + (sampleRate * gap / 1000);

        int dataSize = totalSamples * (bitsPerSample / 8);
        int wavSize = 44 + dataSize;
        auto* wav = (uint8_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, wavSize);
        if (wav)
        {
            // WAV header
            memcpy(wav, "RIFF", 4);
            *(uint32_t*)(wav + 4)  = wavSize - 8;
            memcpy(wav + 8, "WAVE", 4);
            memcpy(wav + 12, "fmt ", 4);
            *(uint32_t*)(wav + 16) = 16;
            *(uint16_t*)(wav + 20) = 1; // PCM
            *(uint16_t*)(wav + 22) = channels;
            *(uint32_t*)(wav + 24) = sampleRate;
            *(uint32_t*)(wav + 28) = sampleRate * channels * bitsPerSample / 8;
            *(uint16_t*)(wav + 32) = channels * bitsPerSample / 8;
            *(uint16_t*)(wav + 34) = bitsPerSample;
            memcpy(wav + 36, "data", 4);
            *(uint32_t*)(wav + 40) = dataSize;

            auto* samples = (int16_t*)(wav + 44);
            int pos = 0;
            for (auto& n : notes)
            {
                int noteSamples = sampleRate * n.ms / 1000;
                for (int i = 0; i < noteSamples; ++i)
                {
                    float t = (float)i / sampleRate;
                    float env = 1.f;
                    float fadeIn = 0.010f, fadeOut = 0.020f;
                    if (t < fadeIn) env = t / fadeIn;
                    float rem = (float)(noteSamples - i) / sampleRate;
                    if (rem < fadeOut) env *= rem / fadeOut;

                    float val = sinf(2.f * 3.14159265f * n.freq * t) * env * 0.25f;
                    samples[pos++] = (int16_t)(val * 32767.f);
                }
                int gapSamples = sampleRate * gap / 1000;
                for (int i = 0; i < gapSamples; ++i)
                    samples[pos++] = 0;
            }

            PlaySoundA((LPCSTR)wav, nullptr, SND_MEMORY | SND_ASYNC);
            Sleep(400);
            HeapFree(GetProcessHeap(), 0, wav);
        }
    }
#endif
    DllLog("[EntryThread] READY â€” menu should be visible, INSERT to toggle");

    // NOTE: PE header wipe REMOVED â€” it breaks RtlAddFunctionTable's
    // RUNTIME_FUNCTION entries and causes DEP crashes at tiny addresses.
    // Manual mapping already provides stealth (not in PEB module list).
    // Anti-cheat doesn't scan arbitrary heap allocations for PE sigs.

    // Log cleanup disabled for debugging â€” check %TEMP%\cs2init.txt

#ifdef _DEBUG
    printf("[+] Ready â€” INSERT=menu, END=unload\n");
#endif

    g_running.store(true);
    DllLog("[EntryThread] Entering main loop");

    // Flush stale key states â€” GetAsyncKeyState(VK_END) & 1 can be
    // set from before injection; clear it before first real check.
    GetAsyncKeyState(VK_END);

    int loopCount = 0;
    UINT64 loopStartTime = GetTickCount64();
    bool warnedNoPresent = false;
    while (g_running.load())
    {
        DllLog("[MainLoop] Loop %d start", loopCount);
        
        // VK_END = unload. Use & 0x8000 (key held) instead of & 1
        // (pressed since last call) to avoid stale state false positives.
        if (GetAsyncKeyState(VK_END) & 0x8000)
        {
            DllLog("[EntryThread] EXIT: VK_END held (loop=%d)", loopCount);
            break;
        }

        // Watchdog: warn if Present hook hasn't fired after 8 seconds
        if (!warnedNoPresent && !Hooks::inited &&
            (GetTickCount64() - loopStartTime) > 8000)
        {
            DllLog("[EntryThread] WARNING: Present hook not fired after 8s "
                   "(oPresent=%p oPresent1=%p pHkPresent=%p pHkPresent1=%p)",
                   (void*)Hooks::oPresent, (void*)Hooks::oPresent1,
                   Hooks::pHkPresent, Hooks::pHkPresent1);
            warnedNoPresent = true;
        }

        DllLog("[MainLoop] Calling Heartbeat");
        VacWatchdog::Tick();
        if (VacWatchdog::tripped.load())
        {
            DllLog("[EntryThread] EXIT: VacWatchdog tripped reason=%d (loop=%d)",
                   VacWatchdog::tripReason.load(), loopCount);
            break;
        }

        // ---- Untrusted-mode (20-hour cooldown) flag suppression --------
        // Lazy-resolve the address once via sigscan, then Stealth::Heartbeat
        // stamps the byte to 0 every beat. The byte lives at
        // client.dll + 0x2198858 in build 14152. Setter sigscan in
        // sub_1801569B0 is "jz +0x26 ; mov [rip+rel32],1 ; xor eax,eax ; cmp eax,1".
        // See /memories/repo/cs2_untrusted_mode_chain.md for the full chain.
        if (!Stealth::g_pUntrustedFlag && !Stealth::g_untrustedFlagResolveTried)
        {
            constexpr const char* kSetterSig =
                "74 26 C6 05 ? ? ? ? 01 33 C0 83 F8 01";
            uintptr_t hit = Mem::FindPattern(L"client.dll", kSetterSig);
            if (hit)
            {
                // Layout: 74 26  C6 05 [rel32] 01 ...
                // The mov inst starts at hit+2; rel32 at hit+4; the mov is
                // 7 bytes long so RIP after it is hit+2+7 = hit+9.
                int32_t rel = *reinterpret_cast<const int32_t*>(hit + 4);
                uintptr_t flagAddr = (hit + 9) + static_cast<intptr_t>(rel);

                // Sanity-bound to client.dll image so a misfiring sig can't
                // splatter random memory.
                HMODULE hClient = GetModuleHandleW(L"client.dll");
                if (hClient)
                {
                    MODULEINFO mi{};
                    if (GetModuleInformation(GetCurrentProcess(), hClient, &mi, sizeof(mi)))
                    {
                        uintptr_t modBase = reinterpret_cast<uintptr_t>(hClient);
                        uintptr_t modEnd  = modBase + mi.SizeOfImage;
                        if (flagAddr >= modBase && flagAddr < modEnd)
                            Stealth::g_pUntrustedFlag =
                                reinterpret_cast<volatile uint8_t*>(flagAddr);
                    }
                }
            }
            Stealth::g_untrustedFlagResolveTried = true;
        }

        // ---- Belt-and-suspenders: kill the GC-report emitter ------------
        // Even with the byte held at 0, a single race-window read could
        // still trip sub_180F23200 -> sub_180C4A6C0 ("init") which is the
        // ONLY function that sends Game::ChatReportError(InsecureBlocked)
        // to Steam GC. Hooking the emitter to early-return makes the GC
        // path unreachable regardless of the byte's transient state.
        // Sigscan pattern verified unique against client.dll @ 14152.
        if (!Stealth::g_insecureEmitterHookTried)
        {
            constexpr const char* kEmitterSig =
                "48 89 5C 24 20 56 48 83 EC 20 48 8B D9 48 89 6C 24 30 "
                "48 8B E9 48 8B 0D ? ? ? ? 48 8B 01";
            uintptr_t addr = Mem::FindPattern(L"client.dll", kEmitterSig);
            if (addr)
            {
                Stealth::g_pInsecureEmitterAddr = reinterpret_cast<void*>(addr);
                MH_STATUS s = MH_CreateHook(
                    reinterpret_cast<void*>(addr),
                    reinterpret_cast<void*>(&Stealth::InsecureEmitter_Detour),
                    &Stealth::g_pInsecureEmitterTramp);
                if (s == MH_OK &&
                    MH_EnableHook(reinterpret_cast<void*>(addr)) == MH_OK)
                {
                    Stealth::g_insecureEmitterHookOk = true;
                    DllLog("[EntryThread] InsecureEmitter hook installed @ %p", (void*)addr);
                }
                else
                {
                    DllLog("[EntryThread] InsecureEmitter hook FAILED status=%d", (int)s);
                }
            }
            Stealth::g_insecureEmitterHookTried = true;
        }

        if (!Stealth::Heartbeat())
        {
            DllLog("[EntryThread] EXIT: Heartbeat failed (loop=%d cleanupDone=%d)", 
                   loopCount, (int)Stealth::cleanupDone);
            break;
        }

        DllLog("[MainLoop] Heartbeat OK, sleeping");
        loopCount++;
        SpoofCall::SpoofedSleep(80);
        DllLog("[MainLoop] Sleep done, loop %d complete", loopCount);
    }
    
    if (!g_running.load())
        DllLog("[EntryThread] EXIT: running flag cleared externally (loop=%d)", loopCount);

    DllLog("[EntryThread] Main loop exited â€” tearing down");

    // Teardown
    g_running.store(false);
    InventoryChanger::Shutdown();
    Bhop::Shutdown();
    ModelChanger::Shutdown();
    WorldEffects::Shutdown();
    Aimbot::Shutdown();
    Hooks::Shutdown();
    VacWatchdog::Shutdown();
    MH_Uninitialize();
    SpoofCall::SpoofedSleep(200);  // wait for hook unhooks to propagate
    DirectSyscall::Shutdown();
    SpoofCall::Shutdown();

    DllLog("[EntryThread] Shutdown complete");

#ifdef _DEBUG
    FreeConsole();
#endif
}

// ---------------------------------------------------------------
// Entry â€” CreateThread + thread start spoof already done above.
// ---------------------------------------------------------------
static unsigned WINAPI ThreadEntry(void* param)
{
    EntryThread(reinterpret_cast<HMODULE>(param));
    return 0;
}

// CRT-free marker log â€” debug builds only
#ifdef _DEBUG
static void RawMarker(const char* msg)
{
    char path[MAX_PATH] = "C:\\cs2log.txt";
    char tmp[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tmp)) {
        lstrcpyA(path, tmp);
        lstrcatA(path, "cs2init.txt");
    }
    HANDLE hFile = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ,
                               nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return;
    DWORD len = (DWORD)lstrlenA(msg);
    DWORD written;
    WriteFile(hFile, msg, len, &written, nullptr);
    WriteFile(hFile, "\r\n", 2, &written, nullptr);
    CloseHandle(hFile);
}
#else
static void RawMarker(const char*) {}
#endif

static std::atomic<bool> g_initStarted{false};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    // Log EVERY call regardless of reason â€” thread hijack mappers may
    // pass garbage in 'reason' (rcx=base, rdx=reason, r8=reserved)
    {
        char buf[128];
        wsprintfA(buf, "[DllMain] reason=%u hModule=%p reserved=%p", reason, hModule, reserved);
        RawMarker(buf);
    }

    // Accept DLL_PROCESS_ATTACH (1), DLL_THREAD_ATTACH (2), OR reason=0 (thread hijack with no args)
    // Also guard against double-init
    if ((reason == DLL_PROCESS_ATTACH || reason == DLL_THREAD_ATTACH || reason == 0) && !g_initStarted.exchange(true))
    {
        RawMarker("[DllMain] Starting init");

        // If hModule looks invalid (thread hijack didn't set it), find our own base
        if (!hModule || (reinterpret_cast<uintptr_t>(hModule) & 0xFFF) != 0)
        {
            // Walk back from our own function to find the allocation base
            MEMORY_BASIC_INFORMATION mbi{};
            VirtualQuery((void*)&DllMain, &mbi, sizeof(mbi));
            hModule = reinterpret_cast<HMODULE>(mbi.AllocationBase);
            char buf[128];
            wsprintfA(buf, "[DllMain] Fixed hModule via VirtualQuery: %p", hModule);
            RawMarker(buf);
        }

        __try
        {
            g_hModule = hModule;
            // Try _beginthreadex first (proper CRT init per-thread)
            uintptr_t t = _beginthreadex(nullptr, 0, ThreadEntry,
                                         hModule, 0, nullptr);
            if (t) {
                RawMarker("[DllMain] _beginthreadex OK");
                CloseHandle(reinterpret_cast<HANDLE>(t));
            } else {
                char buf[128];
                wsprintfA(buf, "[DllMain] _beginthreadex FAILED err=%lu", GetLastError());
                RawMarker(buf);
                HANDLE h = CreateThread(nullptr, 0,
                    [](LPVOID p) -> DWORD { EntryThread(reinterpret_cast<HMODULE>(p)); return 0; },
                    hModule, 0, nullptr);
                if (h) {
                    RawMarker("[DllMain] CreateThread OK");
                    CloseHandle(h);
                } else {
                    char buf2[128];
                    wsprintfA(buf2, "[DllMain] CreateThread FAILED err=%lu", GetLastError());
                    RawMarker(buf2);
                    // Last resort: run directly on this thread
                    EntryThread(hModule);
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            RawMarker("[DllMain] EXCEPTION caught");
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        g_running.store(false);
    }
    return TRUE;
}

// ---------------------------------------------------------------
// RAW ENTRY POINT â€” runs before CRT init.
// Set via /ENTRY:RawDllEntry in linker settings.
//
// The BYOVD thread-hijack injector sets RIP to our entry and the
// CRT's _DllMainCRTStartup normally reads PE headers during init.
// But the injector wipes headers right after resuming the thread,
// causing a race condition crash inside CRT init.
//
// Solution: this entry runs first (no CRT dependency), caches the
// PE image size from headers before they're wiped, then inits CRT
// and calls DllMain normally.
// ---------------------------------------------------------------

// g_cachedImageSize is defined near g_hModule at file scope (line ~33).
// Read from PE before headers can be wiped.

// Atomic guard â€” prevents RawDllEntry from running twice.
// InterlockedCompareExchange is safe even before CRT init.
static volatile LONG g_rawEntryGuard = 0;

// Allow memory.h to use our cached size instead of reading PE headers
extern "C" SIZE_T GetCachedImageSize() { return g_cachedImageSize; }

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter

extern "C" BOOL WINAPI RawDllEntry(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    // QueueUserAPC only passes 1 arg (the DLL base in rcx).
    // fdwReason (rdx) and lpReserved (r8) will be GARBAGE.
    // The atomic guard protects against double-init, so we accept
    // ANY fdwReason for the init path. Only skip DLL_PROCESS_DETACH(0)
    // if it actually looks like a legitimate detach (guard already set).
    if (fdwReason == DLL_PROCESS_DETACH && g_rawEntryGuard == 1)
    {
        // Legitimate detach â€” pass through to CRT + DllMain
    }
    else
    {
        // Atomic guard: only the FIRST caller proceeds. Any duplicate
        // calls (APC, hijack race, etc.) return immediately.
        if (InterlockedCompareExchange(&g_rawEntryGuard, 1, 0) != 0)
        {
            RawMarker("[RawDllEntry] BLOCKED â€” already running on another thread");
            return TRUE;
        }

        // Always normalize to DLL_PROCESS_ATTACH â€” thread hijack may pass 0,
        // some injectors may pass DLL_THREAD_ATTACH (2)
        fdwReason = DLL_PROCESS_ATTACH;

        // === PRE-CRT: Cache PE info before headers can be wiped ===
        __try {
            auto base = reinterpret_cast<uint8_t*>(hinstDLL);
            auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
            if (dos && dos->e_magic == IMAGE_DOS_SIGNATURE)
            {
                auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
                g_cachedImageSize = nt->OptionalHeader.SizeOfImage;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}

        // If hinstDLL is garbage (thread hijack with no args), discover it
        if (!hinstDLL || (reinterpret_cast<uintptr_t>(hinstDLL) & 0xFFF) != 0)
        {
            MEMORY_BASIC_INFORMATION mbi{};
            VirtualQuery((void*)&RawDllEntry, &mbi, sizeof(mbi));
            hinstDLL = reinterpret_cast<HINSTANCE>(mbi.AllocationBase);

            // Try to cache image size from the corrected base
            if (!g_cachedImageSize)
            {
                __try {
                    auto base = reinterpret_cast<uint8_t*>(hinstDLL);
                    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
                    if (dos && dos->e_magic == IMAGE_DOS_SIGNATURE)
                    {
                        auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
                        g_cachedImageSize = nt->OptionalHeader.SizeOfImage;
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {}
            }

            // For thread hijack: hinstDLL was garbage, now fixed
        }

        // Log â€” pure Win32, no CRT needed
        RawMarker("[RawDllEntry] Reached entry point");
        {
            char buf[128];
            wsprintfA(buf, "[RawDllEntry] hInst=%p reason=%u imgSize=0x%X",
                      hinstDLL, fdwReason, (unsigned)g_cachedImageSize);
            RawMarker(buf);
        }

        // Sanitize lpReserved â€” APC may leave garbage in r8
        // _CRT_INIT checks this to distinguish static vs dynamic load
        lpReserved = nullptr;
    }

    // === Initialize CRT (with retries) ===
    BOOL crtOk = FALSE;
    for (int crtTry = 0; crtTry < 3; ++crtTry)
    {
        crtOk = _CRT_INIT(hinstDLL, fdwReason, lpReserved);
        if (crtOk) break;
        RawMarker("[RawDllEntry] _CRT_INIT failed, retrying...");
        Sleep(300);
    }
    if (!crtOk)
    {
        RawMarker("[RawDllEntry] _CRT_INIT FAILED after 3 attempts â€” trying DllMain anyway");
        // Don't bail â€” DllMain uses raw Win32 and can recover.
        // _beginthreadex will fail but CreateThread fallback works.
    }

    if (fdwReason == DLL_PROCESS_ATTACH)
        RawMarker(crtOk ? "[RawDllEntry] CRT init OK, calling DllMain"
                        : "[RawDllEntry] CRT partial, calling DllMain");

    // === Call our DllMain ===
    return DllMain(reinterpret_cast<HMODULE>(hinstDLL), fdwReason, lpReserved);
}

#pragma warning(pop)
