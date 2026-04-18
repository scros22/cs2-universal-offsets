// injector.cpp — BYOVD Manual-mapping DLL injector
// v5.0: CorMem.sys default — WHQL signed kernel R/W via physical memory.
//
// Supported drivers:
//   --driver cormem   → CorMem.sys (default — physical memory map/unmap, WHQL)
//   --driver siv      → SIVX64.sys (WHQL signed, HVCI safe)
//   --driver wdt      → WDTKernel.sys (WHQL, Dell WDT — needs Dell hardware)
//   --driver rtcore   → RTCore64.sys (CVE-2019-16098
//
// Pipeline:
//   1. Load selected BYOVD driver
//   2. Initialize kernel context (resolve ntoskrnl exports)
//   3. Manual map DLL into cs2.exe via kernel memory R/W
//   4. Execute via thread hijack (no NtCreateThreadEx)
//   5. Clean up: wipe headers, unload driver, delete service
//
// Falls back to standard user-mode injection if BYOVD fails.
//
// Compile: cl /EHsc /O2 /Fe:inject.exe injector.cpp

#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>

#include "byovd/driver_provider.h"
#include "byovd/rtcore.h"
#include "byovd/cormem.h"
#include "byovd/wdtkernel.h"
#include "byovd/sivdriver.h"
#include "byovd/echodriver.h"
#include "byovd/kernel_ctx.h"
#include "byovd/mapper.h"

#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>

#pragma comment(lib, "Psapi.lib")

// ---- NT types (fallback path) ----
using NtOpenProcessFn           = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, PVOID, PVOID);
using NtAllocateVirtualMemoryFn = NTSTATUS(NTAPI*)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
using NtWriteVirtualMemoryFn    = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
using NtProtectVirtualMemoryFn  = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
using NtCreateThreadExFn        = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, PVOID, HANDLE,
                                                    PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PVOID);
using NtFreeVirtualMemoryFn     = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG);
using NtCloseFn                 = NTSTATUS(NTAPI*)(HANDLE);

struct OBJ_ATTR { ULONG Length; HANDLE Root; PVOID Name; ULONG Attr; PVOID Sec; PVOID Qos; };
struct CID      { HANDLE Pid; HANDLE Tid; };

// ---- Helpers ----
static DWORD FindProcess(const wchar_t* name)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    DWORD pid = 0;

    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, name) == 0) { pid = pe.th32ProcessID; break; }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

static std::vector<uint8_t> ReadFileToVec(const char* path)
{
    std::vector<uint8_t> buf;
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return buf;
    DWORD sz = GetFileSize(h, nullptr);
    buf.resize(sz);
    DWORD rd = 0;
    ReadFile(h, buf.data(), sz, &rd, nullptr);
    CloseHandle(h);
    return buf;
}

// ---- Embedded resource helpers ----
// Resource IDs must match app.rc:  101 = CS2.dll, 102 = RTCore64.sys, 103 = CORMEM.SYS
#define RES_ID_DLL    101
#define RES_ID_DRIVER 102
#define RES_ID_CORMEM 103

static std::vector<uint8_t> LoadEmbeddedResource(int resId)
{
    std::vector<uint8_t> buf;
    HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(resId), MAKEINTRESOURCEW(10));
    if (!hRes) return buf;
    HGLOBAL hData = LoadResource(nullptr, hRes);
    if (!hData) return buf;
    DWORD sz = SizeofResource(nullptr, hRes);
    auto* ptr = static_cast<const uint8_t*>(LockResource(hData));
    if (ptr && sz) buf.assign(ptr, ptr + sz);
    return buf;
}

static bool ExtractResourceToFile(int resId, const wchar_t* outPath)
{
    auto data = LoadEmbeddedResource(resId);
    if (data.empty()) return false;
    HANDLE h = CreateFileW(outPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    BOOL ok = WriteFile(h, data.data(), (DWORD)data.size(), &written, nullptr);
    CloseHandle(h);
    return ok && written == data.size();
}

// ---- Forward declarations ----
static bool InjectLegacy(DWORD pid, const std::vector<uint8_t>& fileData);

// ---- Manual Mapper ----

// ANSI escape helpers for styled console output
#define ESC     "\x1b["
#define RST     ESC "0m"
#define BOLD    ESC "1m"
#define DIM     ESC "2m"
#define ITAL    ESC "3m"
#define ULINE   ESC "4m"

// Colors
#define FG_BLK  ESC "30m"
#define FG_RED  ESC "31m"
#define FG_GRN  ESC "32m"
#define FG_YEL  ESC "33m"
#define FG_BLU  ESC "34m"
#define FG_MAG  ESC "35m"
#define FG_CYN  ESC "36m"
#define FG_WHT  ESC "37m"

// Bright colors
#define FG_BRED ESC "91m"
#define FG_BGRN ESC "92m"
#define FG_BYEL ESC "93m"
#define FG_BBLU ESC "94m"
#define FG_BMAG ESC "95m"
#define FG_BCYN ESC "96m"
#define FG_BWHT ESC "97m"

// Background
#define BG_BLK  ESC "40m"

// 256-color
#define FG256(n) ESC "38;5;" #n "m"
#define BG256(n) ESC "48;5;" #n "m"

static void EnableAnsi()
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(65001);
    SetConsoleTitleA("LUCID Injector");
}

static void PrintBanner()
{
    printf("\n");
    printf(BOLD FG256(69)  "    ╷   ╷ ╷ ╶─╮ ╶╴ ╶─╮\n" RST);
    printf(BOLD FG256(105) "    │   │ │ │    │  │  │\n" RST);
    printf(BOLD FG256(141) "    ╰── ╰─╯ ╰── ╰╴ ╰──╯\n" RST);
    printf("\n");
    printf(DIM FG256(245) "    BYOVD Manual Map Injector v5.0\n" RST);
    printf(DIM FG256(240) "    ─────────────────────────────────\n\n" RST);
}

static void PrintSection(const char* title)
{
    printf(BOLD FG256(69) "\n  ┌─ " RST BOLD FG_BWHT "%s" RST BOLD FG256(69) "\n" RST, title);
    printf(DIM FG256(240) "  │\n" RST);
}

static void PrintOK(const char* fmt, ...)
{
    printf(DIM FG256(240) "  │  " RST FG_BGRN "✓ " RST);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(RST "\n");
}

static void PrintInfo(const char* fmt, ...)
{
    printf(DIM FG256(240) "  │  " RST FG256(69) "◆ " RST DIM);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(RST "\n");
}

static void PrintWarn(const char* fmt, ...)
{
    printf(DIM FG256(240) "  │  " RST FG_BYEL "⚠ " RST FG_YEL);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(RST "\n");
}

static void PrintFail(const char* fmt, ...)
{
    printf(DIM FG256(240) "  │  " RST FG_BRED "✗ " RST FG_RED);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(RST "\n");
}

static void PrintSectionEnd()
{
    printf(DIM FG256(240) "  │\n  └────\n" RST);
}

// Forward declarations (defined after wmain)
static bool InjectLegacy(DWORD pid, const std::vector<uint8_t>& fileData);
static bool InjectLoadLibrary(DWORD pid, const char* dllFullPath);

int wmain(int argc, wchar_t* argv[])
{
    EnableAnsi();
    PrintBanner();

    // --- Load DLL payload ---
    // Priority: 1) embedded resource  2) CLI arg  3) next to exe  4) dev layout
    std::vector<uint8_t> fileData;
    const char* dllSource = "embedded";

    // Try embedded resource first (single-EXE dist)
    fileData = LoadEmbeddedResource(RES_ID_DLL);

    if (fileData.empty())
    {
        // Fallback: read from disk (dev / loose-file layout)
        char dllPath[MAX_PATH] = {};
        if (argc >= 2) {
            WideCharToMultiByte(CP_ACP, 0, argv[1], -1, dllPath, MAX_PATH, nullptr, nullptr);
        } else {
            char self[MAX_PATH];
            GetModuleFileNameA(nullptr, self, MAX_PATH);
            char* sl = strrchr(self, '\\');
            if (sl) *(sl + 1) = '\0';

            // Check for CS2.dll next to the exe first (dist layout)
            char localDll[MAX_PATH];
            strcpy_s(localDll, self);
            strcat_s(localDll, "CS2.dll");
            if (GetFileAttributesA(localDll) != INVALID_FILE_ATTRIBUTES) {
                strcpy_s(dllPath, localDll);
            } else {
                // Dev layout: tools/ -> x64/Release/
                strcat_s(self, "..\\x64\\Release\\CS2.dll");
                GetFullPathNameA(self, MAX_PATH, dllPath, nullptr);
            }
        }

        if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES) {
            PrintFail("DLL not found (no embedded resource, no file on disk)");
            return 1;
        }

        fileData = ReadFileToVec(dllPath);
        dllSource = dllPath;
    }

    PrintSection("PAYLOAD");
    PrintOK("Source: " DIM "%s" RST " (%u KB)", dllSource, (DWORD)(fileData.size() / 1024));

    if (fileData.size() < sizeof(IMAGE_DOS_HEADER)) {
        PrintFail("Failed to read DLL or file too small");
        return 1;
    }

    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(fileData.data());
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        PrintFail("Invalid DOS signature");
        return 1;
    }

    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(fileData.data() + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        PrintFail("Invalid NT signature");
        return 1;
    }
    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        PrintFail("Not an x64 DLL");
        return 1;
    }

    DWORD imageSize = nt->OptionalHeader.SizeOfImage;
    PrintOK("PE valid: %u sections, image " DIM "0x%X (%u KB)",
           nt->FileHeader.NumberOfSections, imageSize, imageSize / 1024);

    // --- Find cs2.exe ---
    DWORD pid = FindProcess(L"cs2.exe");
    if (!pid) {
        PrintFail("cs2.exe not found — is the game running?");
        PrintSectionEnd();
        return 1;
    }
    PrintOK("cs2.exe PID: " BOLD "%lu", pid);
    PrintSectionEnd();

    // =============================================================
    // BYOVD Injection Path (Primary — Kernel-Assisted)
    // =============================================================
    bool byovdSuccess = false;
    bool forceUsermode = false;
    BYOVD::DriverType selectedDriver = BYOVD::DriverType::CorMem; // default — WHQL signed, physical memory R/W

    // Parse flags: --usermode / -u, --driver <name>
    for (int i = 1; i < argc; ++i)
    {
        if (_wcsicmp(argv[i], L"--usermode") == 0 || _wcsicmp(argv[i], L"-u") == 0)
        {
            forceUsermode = true;
        }
        else if (_wcsicmp(argv[i], L"--driver") == 0 || _wcsicmp(argv[i], L"-d") == 0)
        {
            if (i + 1 < argc)
            {
                ++i;
                if (_wcsicmp(argv[i], L"siv") == 0)
                    selectedDriver = BYOVD::DriverType::SIVDriver;
                else if (_wcsicmp(argv[i], L"rtcore") == 0)
                    selectedDriver = BYOVD::DriverType::RTCore64;
                else if (_wcsicmp(argv[i], L"cormem") == 0)
                    selectedDriver = BYOVD::DriverType::CorMem;
                else if (_wcsicmp(argv[i], L"wdt") == 0)
                    selectedDriver = BYOVD::DriverType::WDTKernel;
                else
                {
                    printf("[!] Unknown driver: %ls (use 'siv', 'cormem', 'rtcore', or 'wdt')\n", argv[i]);
                    return 1;
                }
            }
        }
    }

    if (!forceUsermode)
    {
        PrintSection("BYOVD KERNEL INJECTION");

        // Create the selected driver provider
        std::unique_ptr<BYOVD::IDriverProvider> driver;
        switch (selectedDriver)
        {
        case BYOVD::DriverType::CorMem:
            PrintInfo("Driver: " BOLD FG256(69) "CorMem.sys" RST DIM " (physical memory R/W, WHQL)");
            driver = std::make_unique<BYOVD::CorMemProvider>();
            break;
        case BYOVD::DriverType::RTCore64:
            PrintInfo("Driver: " BOLD FG256(69) "RTCore64.sys" RST DIM " (CVE-2019-16098)");
            driver = std::make_unique<BYOVD::RTCoreProvider>();
            break;
        case BYOVD::DriverType::WDTKernel:
            PrintInfo("Driver: " BOLD FG256(69) "WDTKernel.sys" RST DIM " (Dell WDT, WHQL)");
            driver = std::make_unique<BYOVD::WDTKernelProvider>();
            break;
        case BYOVD::DriverType::SIVDriver:
            PrintInfo("Driver: " BOLD FG256(69) "SIVX64.sys" RST DIM " (SIV, WHQL signed)");
            driver = std::make_unique<BYOVD::SIVDriverProvider>();
            break;
        }

        // Suppress verbose driver/kernel debug output via fd redirect
        int savedFd = _dup(1);       // save stdout fd
        int nulFd = -1;
        _sopen_s(&nulFd, "NUL", _O_WRONLY, _SH_DENYNO, 0);
        auto suppressOn  = [&]() { if (nulFd >= 0) { fflush(stdout); _dup2(nulFd, 1); } };
        auto suppressOff = [&]() { if (savedFd >= 0) { fflush(stdout); _dup2(savedFd, 1); } };

        // Separate log fd for mapper diagnostics (kept on failure)
        wchar_t mapperLogPath[MAX_PATH] = {};
        GetTempPathW(MAX_PATH, mapperLogPath);
        wcscat_s(mapperLogPath, L"lucid_mapper.log");
        int logFd = -1;
        _wsopen_s(&logFd, mapperLogPath, _O_WRONLY | _O_CREAT | _O_TRUNC, _SH_DENYNO, _S_IREAD | _S_IWRITE);
        auto logOn  = [&]() { if (logFd >= 0) { fflush(stdout); _dup2(logFd, 1); } };

        logOn();    // all kernel/driver output → log file (not NUL)

        if (driver->Open())
        {
            suppressOff();
            PrintOK("Driver loaded");

            logOn();
            BYOVD::KernelContext kctx(*driver);
            bool kInit = kctx.Init();
            suppressOff();

            if (kInit)
            {
                PrintOK("Kernel context ready");

                logOn();   // mapper output → log file (not NUL)
                BYOVD::KernelMapper mapper(kctx);
                byovdSuccess = mapper.MapDLL(pid, fileData);
                suppressOff();

                if (byovdSuccess)
                {
                    PrintOK("DLL mapped via " BOLD "physical memory R/W");
                    PrintOK("Init " BOLD FG_BGRN "verified" RST " — DLL is running");
                }
                else
                {
                    PrintWarn("Kernel mapper failed — falling back to usermode");
                    // Dump mapper diagnostics so user/developer can see what went wrong
                    if (logFd >= 0) { _close(logFd); logFd = -1; }
                    FILE* lf = _wfopen(mapperLogPath, L"r");
                    if (lf)
                    {
                        printf(DIM);
                        char line[512];
                        while (fgets(line, sizeof(line), lf))
                            printf("  %s", line);
                        printf(RST "\n");
                        fclose(lf);
                    }
                }
            }
            else
            {
                PrintWarn("Kernel context failed — falling back to usermode");
                // Dump diagnostics from driver/kernel init phase
                if (logFd >= 0) { _close(logFd); logFd = -1; }
                FILE* lf = _wfopen(mapperLogPath, L"r");
                if (lf)
                {
                    printf(DIM);
                    char line[512];
                    while (fgets(line, sizeof(line), lf))
                        printf("  %s", line);
                    printf(RST "\n");
                    fclose(lf);
                }
            }

            logOn();
            driver->Close();

            switch (selectedDriver)
            {
            case BYOVD::DriverType::CorMem:
                BYOVD::UnloadDriverService(L"CORMEMcs2", L"CORMEM.SYS");
                break;
            case BYOVD::DriverType::RTCore64:
                BYOVD::UnloadRTCoreDriver();
                break;
            case BYOVD::DriverType::WDTKernel:
                BYOVD::UnloadWDTKernelDriver();
                break;
            case BYOVD::DriverType::SIVDriver:
                BYOVD::UnloadSIVDriver();
                break;
            }
            suppressOff();

            if (byovdSuccess) PrintOK("Driver cleaned up");
        }
        else
        {
            suppressOff();
            PrintWarn("Driver failed to load — falling back to usermode");
            // Dump driver load diagnostics
            if (logFd >= 0) { _close(logFd); logFd = -1; }
            FILE* lf2 = _wfopen(mapperLogPath, L"r");
            if (lf2)
            {
                printf(DIM);
                char line[512];
                while (fgets(line, sizeof(line), lf2))
                    printf("  %s", line);
                printf(RST "\n");
                fclose(lf2);
            }
        }

        if (nulFd >= 0) _close(nulFd);
        if (logFd >= 0) _close(logFd);
        if (savedFd >= 0) _close(savedFd);
        DeleteFileW(mapperLogPath);
        PrintSectionEnd();
    }

    if (byovdSuccess)
    {
        printf("\n");
        PrintSection("COMPLETE");
        PrintOK("Injection " BOLD FG_BGRN "successful" RST " via " BOLD "BYOVD");
        printf(DIM FG256(240) "  │\n" RST);
        PrintInfo(FG256(69) "INSERT" RST DIM " — toggle menu in-game");
        PrintInfo(FG256(69) "END" RST DIM "    — unload cheat");
        PrintSectionEnd();
        // Success chime
        Beep(523, 100); Beep(659, 100); Beep(784, 150);
        printf("\n" DIM "  Press Enter to exit..." RST "\n");
        getchar();
        return 0;
    }

    // =============================================================
    // Legacy Injection Path (Fallback — User-Mode NtAPI)
    // =============================================================
    PrintSection("LEGACY USER-MODE INJECTION");
    bool legacyOk = InjectLegacy(pid, fileData);
    if (!legacyOk)
    {
        PrintFail("Legacy manual-map injection failed");
        PrintSectionEnd();

        // =============================================================
        // LoadLibrary Fallback (3rd path — most compatible)
        // =============================================================
        PrintSection("LOADLIBRARY FALLBACK");
        PrintInfo("Using Win32 CreateRemoteThread + LoadLibraryA...");
        PrintInfo("This is the most compatible path (Win11 Canary safe)");

        // Need the DLL on disk for LoadLibrary — extract to temp if embedded
        char absPath[MAX_PATH] = {};
        wchar_t tmpDll[MAX_PATH];
        GetTempPathW(MAX_PATH, tmpDll);
        wcscat_s(tmpDll, L"cs2_payload.dll");

        HANDLE hTmp = CreateFileW(tmpDll, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hTmp != INVALID_HANDLE_VALUE) {
            DWORD wr = 0;
            WriteFile(hTmp, fileData.data(), (DWORD)fileData.size(), &wr, nullptr);
            CloseHandle(hTmp);
            WideCharToMultiByte(CP_ACP, 0, tmpDll, -1, absPath, MAX_PATH, nullptr, nullptr);
        }

        bool llOk = false;
        if (absPath[0]) {
            llOk = InjectLoadLibrary(pid, absPath);
            DeleteFileW(tmpDll); // Clean up temp DLL
        }

        if (!llOk)
        {
            PrintFail("All injection methods failed");
            PrintSectionEnd();
            printf("\n" DIM "  Press Enter to exit..." RST "\n");
            getchar();
            return 1;
        }
        PrintSectionEnd();
    }
    else
    {
        PrintSectionEnd();
    }

    printf("\n");
    PrintSection("COMPLETE");
    PrintOK("Injection " BOLD FG_BGRN "successful" RST " via %s", legacyOk ? "Legacy" : "LoadLibrary");
    printf(DIM FG256(240) "  │\n" RST);
    PrintInfo(FG256(69) "INSERT" RST DIM " — toggle menu in-game");
    PrintInfo(FG256(69) "END" RST DIM "    — unload cheat");

    // Success chime
    Beep(523, 100); Beep(659, 100); Beep(784, 150);

    // Check debug log
    char logPath[MAX_PATH];
    GetTempPathA(MAX_PATH, logPath);
    strcat_s(logPath, "cs2dbg.txt");

    Sleep(3000);

    if (GetFileAttributesA(logPath) != INVALID_FILE_ATTRIBUTES) {
        PrintInfo("Debug log: " DIM "%s", logPath);
        HANDLE hLog = CreateFileA(logPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  nullptr, OPEN_EXISTING, 0, nullptr);
        if (hLog != INVALID_HANDLE_VALUE) {
            DWORD sz = GetFileSize(hLog, nullptr);
            DWORD start = (sz > 4096) ? sz - 4096 : 0;
            SetFilePointer(hLog, start, nullptr, FILE_BEGIN);
            char buf[4097] = {};
            DWORD rd = 0;
            ReadFile(hLog, buf, 4096, &rd, nullptr);
            CloseHandle(hLog);
            printf(DIM FG256(240) "\n%s\n" RST, buf);
        }
    } else {
        PrintWarn("No debug log — DllMain may have crashed before logging");
    }

    PrintSectionEnd();
    printf("\n" DIM "  Press Enter to exit..." RST "\n");
    getchar();
    return 0;
}

// =============================================================
// InjectLegacy: Full user-mode manual map (original code path)
// =============================================================
static bool InjectLegacy(DWORD pid, const std::vector<uint8_t>& fileData)
{
    auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(fileData.data());
    auto* nt  = reinterpret_cast<const IMAGE_NT_HEADERS64*>(fileData.data() + dos->e_lfanew);
    DWORD imageSize = nt->OptionalHeader.SizeOfImage;

    // --- Resolve NT functions ---
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    auto pOpen    = (NtOpenProcessFn)GetProcAddress(ntdll, "NtOpenProcess");
    auto pAlloc   = (NtAllocateVirtualMemoryFn)GetProcAddress(ntdll, "NtAllocateVirtualMemory");
    auto pWrite   = (NtWriteVirtualMemoryFn)GetProcAddress(ntdll, "NtWriteVirtualMemory");
    auto pProtect = (NtProtectVirtualMemoryFn)GetProcAddress(ntdll, "NtProtectVirtualMemory");
    auto pThread  = (NtCreateThreadExFn)GetProcAddress(ntdll, "NtCreateThreadEx");
    auto pFree    = (NtFreeVirtualMemoryFn)GetProcAddress(ntdll, "NtFreeVirtualMemory");
    auto pClose   = (NtCloseFn)GetProcAddress(ntdll, "NtClose");

    if (!pOpen || !pAlloc || !pWrite || !pProtect || !pThread) {
        PrintFail("Failed to resolve NT functions");
        return false;
    }

    // --- Open target ---
    HANDLE hProc = nullptr;
    OBJ_ATTR oa{}; oa.Length = sizeof(oa);
    CID cid{}; cid.Pid = reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(pid));

    NTSTATUS st = pOpen(&hProc,
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        &oa, &cid);
    if (st != 0 || !hProc) {
        PrintFail("NtOpenProcess failed: 0x%08lX (run as Admin)", st);
        return false;
    }
    PrintOK("Process opened");

    // --- Allocate image in target ---
    // Use PAGE_READWRITE first (HVCI on Win11 Canary blocks RWX).
    // Per-section protections are applied after writing.
    SIZE_T allocSz = imageSize;
    PVOID remoteBase = nullptr;
    st = pAlloc(hProc, &remoteBase, 0, &allocSz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (st != 0 || !remoteBase) {
        PrintFail("NtAllocateVirtualMemory failed: 0x%08lX", st);
        if (st == 0xC0000022L)
            PrintInfo("ACCESS_DENIED — HVCI or ACG may be blocking allocation");
        pClose(hProc);
        return false;
    }
    PrintOK("Remote image at " DIM "0x%p (0x%X bytes)", remoteBase, imageSize);

    // --- Build local image copy ---
    std::vector<uint8_t> mapped(imageSize, 0);

    // Copy headers
    memcpy(mapped.data(), fileData.data(), nt->OptionalHeader.SizeOfHeaders);

    // Copy sections
    auto* secArr = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        auto& sec = secArr[i];
        if (sec.SizeOfRawData == 0) continue;
        if (sec.PointerToRawData + sec.SizeOfRawData > fileData.size()) {
            PrintWarn("Section %.8s raw data out of bounds, skipping", sec.Name);
            continue;
        }
        memcpy(mapped.data() + sec.VirtualAddress,
               fileData.data() + sec.PointerToRawData,
               sec.SizeOfRawData);
    }
    PrintOK("%u sections mapped", nt->FileHeader.NumberOfSections);

    // --- Process relocations ---
    uintptr_t preferredBase = nt->OptionalHeader.ImageBase;
    uintptr_t actualBase    = reinterpret_cast<uintptr_t>(remoteBase);
    intptr_t  delta         = static_cast<intptr_t>(actualBase - preferredBase);

    auto& relocDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (delta != 0 && relocDir.VirtualAddress != 0 && relocDir.Size != 0)
    {
        DWORD relocOffset = 0;
        int relocCount = 0;

        while (relocOffset < relocDir.Size)
        {
            auto* block = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                mapped.data() + relocDir.VirtualAddress + relocOffset);

            if (block->SizeOfBlock == 0) break;

            DWORD entryCount = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            auto* entries = reinterpret_cast<WORD*>(
                reinterpret_cast<uint8_t*>(block) + sizeof(IMAGE_BASE_RELOCATION));

            for (DWORD e = 0; e < entryCount; e++)
            {
                WORD type   = entries[e] >> 12;
                WORD offset = entries[e] & 0xFFF;
                DWORD rva   = block->VirtualAddress + offset;

                if (rva >= imageSize) continue;

                if (type == IMAGE_REL_BASED_DIR64) {
                    auto* patch = reinterpret_cast<uint64_t*>(mapped.data() + rva);
                    *patch += delta;
                    relocCount++;
                }
                else if (type == IMAGE_REL_BASED_HIGHLOW) {
                    auto* patch = reinterpret_cast<uint32_t*>(mapped.data() + rva);
                    *patch += static_cast<uint32_t>(delta);
                    relocCount++;
                }
            }
            relocOffset += block->SizeOfBlock;
        }
        PrintOK("%d relocations applied " DIM "(delta: 0x%llX)", relocCount, (uint64_t)delta);
    }
    else if (delta == 0) {
        PrintOK("Loaded at preferred base, no relocations needed");
    }

    // --- Resolve imports ---
    // System DLLs (kernel32, user32, d3d11) are mapped at the same virtual
    // address in all processes of the same boot session, so resolving locally
    // gives addresses valid in cs2.exe.
    auto& importDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (importDir.VirtualAddress != 0 && importDir.Size != 0)
    {
        auto* desc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
            mapped.data() + importDir.VirtualAddress);

        int modCount = 0, funcCount = 0;

        while (desc->Name != 0)
        {
            const char* modName = reinterpret_cast<const char*>(mapped.data() + desc->Name);
            HMODULE hMod = GetModuleHandleA(modName);

            if (!hMod) {
                hMod = LoadLibraryA(modName);
                if (!hMod) {
                    PrintWarn("Import module not found: %s", modName);
                    desc++;
                    continue;
                }
            }

            auto* thunk = reinterpret_cast<IMAGE_THUNK_DATA64*>(
                mapped.data() + desc->FirstThunk);
            auto* origThunk = desc->OriginalFirstThunk
                ? reinterpret_cast<IMAGE_THUNK_DATA64*>(mapped.data() + desc->OriginalFirstThunk)
                : thunk;

            while (origThunk->u1.AddressOfData != 0)
            {
                FARPROC func = nullptr;
                if (IMAGE_SNAP_BY_ORDINAL64(origThunk->u1.Ordinal)) {
                    func = GetProcAddress(hMod, MAKEINTRESOURCEA(IMAGE_ORDINAL64(origThunk->u1.Ordinal)));
                } else {
                    auto* nameEntry = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                        mapped.data() + origThunk->u1.AddressOfData);
                    func = GetProcAddress(hMod, nameEntry->Name);
                }

                if (func) {
                    thunk->u1.Function = reinterpret_cast<uint64_t>(func);
                    funcCount++;
                } else {
                    PrintWarn("Unresolved import from %s", modName);
                }

                thunk++;
                origThunk++;
            }

            modCount++;
            desc++;
        }
        PrintOK("Imports: %d modules, %d functions resolved", modCount, funcCount);
    }

    // Update image base in mapped headers
    auto* mappedNt = reinterpret_cast<IMAGE_NT_HEADERS64*>(mapped.data() + dos->e_lfanew);
    mappedNt->OptionalHeader.ImageBase = actualBase;

    // --- Write image to target ---
    st = pWrite(hProc, remoteBase, mapped.data(), imageSize, nullptr);
    if (st != 0) {
        PrintFail("Failed to write image: 0x%08lX", st);
        pFree(hProc, &remoteBase, &allocSz, MEM_RELEASE);
        pClose(hProc);
        return false;
    }
    PrintOK("Image written to cs2.exe");

    // --- Set proper section protections ---
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        auto& sec = secArr[i];
        DWORD prot = PAGE_READONLY;
        bool exec  = (sec.Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        bool write = (sec.Characteristics & IMAGE_SCN_MEM_WRITE) != 0;

        if (exec && write)       prot = PAGE_EXECUTE_READWRITE;
        else if (exec)           prot = PAGE_EXECUTE_READ;
        else if (write)          prot = PAGE_READWRITE;

        PVOID secAddr = reinterpret_cast<uint8_t*>(remoteBase) + sec.VirtualAddress;
        SIZE_T secSize = sec.Misc.VirtualSize ? sec.Misc.VirtualSize : sec.SizeOfRawData;
        ULONG oldProt = 0;

        pProtect(hProc, &secAddr, &secSize, prot, &oldProt);
    }
    PrintOK("Section protections applied");

    // --- Process TLS directory ---
    // The static CRT (/MT) uses _tls_index which the Windows loader normally
    // populates via TlsAlloc. Without this, _tls_index stays 0 (cs2.exe's
    // own TLS slot), and _CRT_INIT corrupts the game's thread-local data.
    auto& tlsDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    uintptr_t tlsAddrOfIndex = 0;
    bool hasTLS = (tlsDir.VirtualAddress != 0 && tlsDir.Size != 0);

    if (hasTLS)
    {
        auto* tls = reinterpret_cast<IMAGE_TLS_DIRECTORY64*>(
            mapped.data() + tlsDir.VirtualAddress);
        // AddressOfIndex is already a relocated VA in the remote image
        tlsAddrOfIndex = tls->AddressOfIndex;
        PrintOK("TLS directory, AddressOfIndex at " DIM "0x%llX",
               (uint64_t)tlsAddrOfIndex);
    }
    else
    {
        PrintInfo("No TLS directory");
    }

    // --- Prepare exception table registration ---
    // x64 SEH is table-based (.pdata). Without registering via
    // RtlAddFunctionTable, any exception (including CRT internal ones) crashes.
    auto& excDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    uintptr_t pdataAddr = 0;
    DWORD pdataEntries = 0;
    bool hasExc = (excDir.VirtualAddress != 0 && excDir.Size != 0);

    if (hasExc)
    {
        pdataAddr   = actualBase + excDir.VirtualAddress;
        pdataEntries = excDir.Size / sizeof(RUNTIME_FUNCTION);
        PrintOK("Exception table: %lu entries at " DIM "0x%llX",
               pdataEntries, (uint64_t)pdataAddr);
    }

    // Resolve helper functions — same address in all processes (per-boot ASLR)
    HMODULE hK32 = GetModuleHandleA("kernel32.dll");
    uintptr_t pTlsAlloc = (uintptr_t)GetProcAddress(hK32, "TlsAlloc");
    uintptr_t pRtlAddFT = (uintptr_t)GetProcAddress(hK32, "RtlAddFunctionTable");
    uintptr_t entryPoint = actualBase + nt->OptionalHeader.AddressOfEntryPoint;

    if (!pTlsAlloc) {
        PrintFail("Failed to resolve TlsAlloc");
        pFree(hProc, &remoteBase, &allocSz, MEM_RELEASE);
        pClose(hProc);
        return false;
    }
    if (!pRtlAddFT) {
        PrintFail("Failed to resolve RtlAddFunctionTable");
        pFree(hProc, &remoteBase, &allocSz, MEM_RELEASE);
        pClose(hProc);
        return false;
    }

    PrintOK("Helpers resolved " DIM "(TlsAlloc, RtlAddFunctionTable)");
    PrintOK("Entry point at " DIM "0x%llX", (uint64_t)entryPoint);

    // --- Build shellcode ---
    // This shellcode runs inside cs2.exe and does three things:
    //   1. TlsAlloc() → write result to _tls_index in our image
    //   2. RtlAddFunctionTable(.pdata, count, base) → register SEH
    //   3. Call _DllMainCRTStartup(hinstDLL, DLL_PROCESS_ATTACH, NULL)
    //
    // x64 calling convention: rcx, rdx, r8, r9 + shadow space
    unsigned char sc[] = {
        // sub rsp, 0x38
        0x48, 0x83, 0xEC, 0x38,

        // --- 1. TLS: call TlsAlloc() ---
        // mov rax, <TlsAlloc>
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // offset 6
        // call rax
        0xFF, 0xD0,                                                     // offset 14
        // mov rcx, <AddressOfIndex VA>
        0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // offset 18
        // mov [rcx], eax  (write TLS index)
        0x89, 0x01,                                                     // offset 26

        // --- 2. Exception: RtlAddFunctionTable(Table, Count, Base) ---
        // mov rcx, <.pdata remote addr>
        0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // offset 30
        // mov edx, <entry count>
        0xBA, 0x00, 0x00, 0x00, 0x00,                                  // offset 38
        // mov r8, <image base>
        0x49, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // offset 45
        // mov rax, <RtlAddFunctionTable>
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // offset 53
        // call rax
        0xFF, 0xD0,                                                     // offset 63

        // --- 3. Call _DllMainCRTStartup(hinstDLL, DLL_PROCESS_ATTACH, NULL) ---
        // mov rcx, <image base>  (hinstDLL)
        0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // offset 67
        // mov edx, 1  (DLL_PROCESS_ATTACH)
        0xBA, 0x01, 0x00, 0x00, 0x00,                                  // offset 75
        // xor r8, r8  (lpReserved = NULL)
        0x4D, 0x31, 0xC0,                                              // offset 80
        // mov rax, <entry point>
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // offset 85
        // call rax
        0xFF, 0xD0,                                                     // offset 93

        // xor eax, eax (return 0)
        0x31, 0xC0,                                                     // offset 95
        // add rsp, 0x38
        0x48, 0x83, 0xC4, 0x38,                                        // offset 97
        // ret
        0xC3                                                            // offset 101
    };

    // Patch addresses into shellcode
    memcpy(&sc[6],  &pTlsAlloc,    8);  // TlsAlloc function address
    memcpy(&sc[18], &tlsAddrOfIndex, 8);  // &_tls_index in remote image
    memcpy(&sc[30], &pdataAddr,    8);  // .pdata section in remote
    memcpy(&sc[39], &pdataEntries, 4);  // RUNTIME_FUNCTION count
    memcpy(&sc[45], &actualBase,   8);  // image base (for RtlAddFunctionTable)
    memcpy(&sc[55], &pRtlAddFT,   8);  // RtlAddFunctionTable address
    memcpy(&sc[67], &actualBase,   8);  // hinstDLL parameter
    memcpy(&sc[85], &entryPoint,   8);  // _DllMainCRTStartup address

    // Handle case where TLS doesn't exist — NOP out the TLS block
    if (!hasTLS)
    {
        // NOP bytes 4..27 (TlsAlloc call + index write)
        memset(&sc[4], 0x90, 24);
    }
    if (!hasExc)
    {
        // NOP bytes 28..64 (RtlAddFunctionTable call)
        memset(&sc[28], 0x90, 37);
    }

    SIZE_T scSize = sizeof(sc);
    PVOID scRemote = nullptr;
    // Allocate as RW first (HVCI blocks RWX), flip to RX after writing
    st = pAlloc(hProc, &scRemote, 0, &scSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (st != 0) {
        PrintFail("Failed to allocate shellcode: 0x%08lX", st);
        pFree(hProc, &remoteBase, &allocSz, MEM_RELEASE);
        pClose(hProc);
        return false;
    }

    st = pWrite(hProc, scRemote, sc, sizeof(sc), nullptr);
    if (st != 0) {
        PrintFail("Failed to write shellcode: 0x%08lX", st);
        pFree(hProc, &scRemote, &scSize, MEM_RELEASE);
        pFree(hProc, &remoteBase, &allocSz, MEM_RELEASE);
        pClose(hProc);
        return false;
    }

    // Flip shellcode page to executable
    PVOID scProtAddr = scRemote;
    SIZE_T scProtSize = scSize;
    ULONG scOldProt = 0;
    st = pProtect(hProc, &scProtAddr, &scProtSize, PAGE_EXECUTE_READ, &scOldProt);
    if (st != 0) {
        PrintWarn("Failed to set shellcode RX: 0x%08lX (trying RWX)", st);
        scProtAddr = scRemote; scProtSize = scSize;
        pProtect(hProc, &scProtAddr, &scProtSize, PAGE_EXECUTE_READWRITE, &scOldProt);
    }

    PrintOK("Shellcode written " DIM "(%zu bytes)  TLS:%s  SEH:%s",
           sizeof(sc), hasTLS ? "yes" : "no", hasExc ? "yes" : "no");

    // --- Execute shellcode ---
    HANDLE hThread = nullptr;
    st = pThread(&hThread, THREAD_ALL_ACCESS, nullptr, hProc,
                 scRemote, nullptr, 0, 0, 0, 0, nullptr);
    if (st != 0 || !hThread) {
        PrintFail("NtCreateThreadEx failed: 0x%08lX", st);
        pFree(hProc, &scRemote, &scSize, MEM_RELEASE);
        pFree(hProc, &remoteBase, &allocSz, MEM_RELEASE);
        pClose(hProc);
        return false;
    }

    PrintOK("Thread started — waiting for CRT init + DllMain...");
    DWORD wait = WaitForSingleObject(hThread, 30000);

    if (wait == WAIT_TIMEOUT)
        PrintOK("Thread still running " DIM "(cheat initializing — normal)");
    else {
        DWORD exitCode = 0;
        GetExitCodeThread(hThread, &exitCode);
        PrintOK("Thread finished " DIM "(exit code: %lu)", exitCode);
    }

    // Verify DLL init by checking log file
    pClose(hThread);
    Sleep(2000);  // Give EntryThread time to start logging

    char logPath[MAX_PATH];
    GetTempPathA(MAX_PATH, logPath);
    strcat_s(logPath, "cs2init.txt");
    bool verified = false;
    HANDLE hLog = CreateFileA(logPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr, OPEN_EXISTING, 0, nullptr);
    if (hLog != INVALID_HANDLE_VALUE) {
        DWORD sz = GetFileSize(hLog, nullptr);
        if (sz > 0 && sz < 0x100000) {
            DWORD startOff = (sz > 2048) ? sz - 2048 : 0;
            SetFilePointer(hLog, startOff, nullptr, FILE_BEGIN);
            char buf[2049] = {};
            DWORD rd = 0;
            ReadFile(hLog, buf, 2048, &rd, nullptr);
            if (strstr(buf, "[DllMain] Starting init") || strstr(buf, "[EntryThread] READY"))
                verified = true;
        }
        CloseHandle(hLog);
    }

    if (verified)
        PrintOK("DLL init " BOLD "VERIFIED" RST " via log file");
    else
        PrintWarn("Could not verify DLL init — check %TEMP%\\cs2init.txt");

    // Clean up shellcode (image stays — that's the cheat)
    pFree(hProc, &scRemote, &scSize, MEM_RELEASE);
    pClose(hProc);

    return true;
}

// =============================================================
// InjectLoadLibrary: Simple CreateRemoteThread + LoadLibraryA
// Most compatible fallback — works on all Windows versions.
// Requires DLL on disk (less stealthy but universal).
// =============================================================
static bool InjectLoadLibrary(DWORD pid, const char* dllFullPath)
{
    // Use standard Win32 API — avoids Nt* syscall hooks on Canary builds
    HANDLE hProc = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        FALSE, pid);
    if (!hProc) {
        PrintFail("OpenProcess failed: %lu", GetLastError());
        return false;
    }
    PrintOK("Process opened (Win32)");

    SIZE_T pathLen = strlen(dllFullPath) + 1;
    LPVOID remotePath = VirtualAllocEx(hProc, nullptr, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePath) {
        PrintFail("VirtualAllocEx failed: %lu", GetLastError());
        CloseHandle(hProc);
        return false;
    }

    if (!WriteProcessMemory(hProc, remotePath, dllFullPath, pathLen, nullptr)) {
        PrintFail("WriteProcessMemory failed: %lu", GetLastError());
        VirtualFreeEx(hProc, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    HMODULE hK32 = GetModuleHandleA("kernel32.dll");
    FARPROC pLoadLib = GetProcAddress(hK32, "LoadLibraryA");
    if (!pLoadLib) {
        PrintFail("Failed to resolve LoadLibraryA");
        VirtualFreeEx(hProc, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0,
        (LPTHREAD_START_ROUTINE)pLoadLib, remotePath, 0, nullptr);
    if (!hThread) {
        PrintFail("CreateRemoteThread failed: %lu", GetLastError());
        VirtualFreeEx(hProc, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    PrintOK("LoadLibrary thread created — waiting...");
    DWORD wait = WaitForSingleObject(hThread, 30000);

    if (wait == WAIT_TIMEOUT)
        PrintOK("Thread still running " DIM "(initializing — normal)");
    else {
        DWORD exitCode = 0;
        GetExitCodeThread(hThread, &exitCode);
        if (exitCode == 0) {
            PrintFail("LoadLibraryA returned NULL (DLL failed to load)");
            VirtualFreeEx(hProc, remotePath, 0, MEM_RELEASE);
            CloseHandle(hThread);
            CloseHandle(hProc);
            return false;
        }
        PrintOK("DLL loaded " DIM "(HMODULE: 0x%lX)", exitCode);
    }

    VirtualFreeEx(hProc, remotePath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProc);

    return true;
}
