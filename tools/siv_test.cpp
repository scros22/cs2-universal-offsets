// siv_test.cpp — Quick test for SIVX64.sys: load, open device, read phys 0
// Compile: cl /EHsc /O2 /std:c++20 siv_test.cpp advapi32.lib
// Run: elevated (admin)
// Output goes to C:\Users\Samuel\siv_result.txt
#include <Windows.h>
#include <cstdio>
#include <cstdint>

#pragma pack(push, 1)
struct SIV_MAP_RW_INPUT
{
    UINT64 PhysicalAddress; // +0x00
    UINT32 Size;            // +0x08
    UINT16 Padding;         // +0x0C
    UINT16 Flags;           // +0x0E
};
#pragma pack(pop)

int main()
{
    freopen("C:\\Users\\Samuel\\siv_result.txt", "w", stdout);
    printf("=== SIVX64.sys Device Test ===\n\n");

    // Step 1: Copy driver to temp
    wchar_t drvPath[MAX_PATH];
    GetTempPathW(MAX_PATH, drvPath);
    wcscat_s(drvPath, L"SIVX64.sys");

    // Copy from project dir
    CopyFileW(L"c:\\Users\\Samuel\\License-Loader\\Loader\\Products\\CS2\\SIVX64.sys",
              drvPath, FALSE);

    if (GetFileAttributesW(drvPath) == INVALID_FILE_ATTRIBUTES)
    {
        printf("SIVX64.sys not found at: %ls\n", drvPath);
        fflush(stdout);
        return 1;
    }
    printf("Driver: %ls\n\n", drvPath);

    // Step 2: Clean up existing service
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scm) { printf("OpenSCManager failed: 0x%lX (run as admin!)\n", GetLastError()); fflush(stdout); return 1; }

    SC_HANDLE existing = OpenServiceW(scm, L"SIVDRVcs2", SERVICE_ALL_ACCESS);
    if (existing)
    {
        SERVICE_STATUS ss;
        ControlService(existing, SERVICE_CONTROL_STOP, &ss);
        DeleteService(existing);
        CloseServiceHandle(existing);
        printf("Cleaned up existing service\n");
        Sleep(1000);
    }

    // Step 3: Create and start service
    printf("--- Creating service 'SIVDRVcs2' ---\n");
    SC_HANDLE svc = CreateServiceW(
        scm, L"SIVDRVcs2", L"SIVDRVcs2",
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
        drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (!svc)
    {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS)
            svc = OpenServiceW(scm, L"SIVDRVcs2", SERVICE_ALL_ACCESS);
        else
        {
            printf("CreateService failed: 0x%lX\n", err);
            CloseServiceHandle(scm);
            fflush(stdout);
            return 1;
        }
    }
    printf("Service created OK\n");

    if (!StartServiceW(svc, 0, nullptr))
    {
        DWORD err = GetLastError();
        if (err != ERROR_SERVICE_ALREADY_RUNNING)
        {
            printf("StartService failed: 0x%lX\n", err);
            if (err == 0x5AD) printf("  (ERROR_WORKING_SET_QUOTA)\n");
            if (err == 0x241) printf("  (ERROR_SERVICE_DISABLED - driver blocked?)\n");
            if (err == 0x57A) printf("  (ERROR_COULD_NOT_INTERPRET - driver init failed)\n");
            if (err == 0x422) printf("  (ERROR_SERVICE_DISABLED)\n");
            if (err == 0x7E)  printf("  (ERROR_MOD_NOT_FOUND)\n");
            if (err == 0x3E6) printf("  (ERROR_NOACCESS - HVCI blocked?)\n");
            if (err == 577)   printf("  (ERROR_INVALID_IMAGE_HASH - signature rejected)\n");
            DeleteService(svc);
            CloseServiceHandle(svc);
            CloseServiceHandle(scm);
            fflush(stdout);
            return 1;
        }
        printf("Service already running\n");
    }
    else
    {
        printf("Service STARTED OK!\n");
    }

    // Step 4: Open device
    printf("\n--- Opening device ---\n");
    HANDLE hDev = CreateFileW(L"\\\\.\\SIVDRIVER",
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (hDev == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile(\\\\.\\SIVDRIVER) failed: 0x%lX\n", GetLastError());

        // Try alternate paths
        const wchar_t* alts[] = {
            L"\\\\.\\GLOBALROOT\\Device\\SIVDRIVER",
            L"\\\\.\\Global\\SIVDRIVER",
        };
        for (auto* alt : alts)
        {
            HANDLE h2 = CreateFileW(alt, GENERIC_READ | GENERIC_WRITE,
                0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (h2 != INVALID_HANDLE_VALUE)
            {
                printf("  Alternate path worked: %ls\n", alt);
                hDev = h2;
                break;
            }
            printf("  %ls -> err=0x%lX\n", alt, GetLastError());
        }
    }
    else
    {
        printf("Device OPENED at \\\\.\\SIVDRIVER !\n");
    }

    // Step 5: Test IOCTL
    if (hDev != INVALID_HANDLE_VALUE)
    {
        printf("\n--- Testing Cmd 0x14 (Read 8 bytes at phys 0) ---\n");
        SIV_MAP_RW_INPUT hdr{};
        hdr.PhysicalAddress = 0;
        hdr.Size = 8;
        hdr.Padding = 0;
        hdr.Flags = 0x0004; // readback

        uint8_t outBuf[8] = {};
        DWORD bytesReturned = 0;
        BOOL ok = DeviceIoControl(hDev, 0x14,
            &hdr, sizeof(hdr),
            outBuf, 8,
            &bytesReturned, nullptr);

        printf("  ok=%d, bytesReturned=%lu, err=0x%lX\n", ok, bytesReturned, GetLastError());
        if (ok)
        {
            printf("  Data at phys 0: ");
            for (int i = 0; i < 8; i++) printf("%02X ", outBuf[i]);
            printf("\n");
        }

        // Test read at physical 0x1000 (should be the IVT area)
        printf("\n--- Testing Cmd 0x14 (Read 16 bytes at phys 0x1000) ---\n");
        hdr.PhysicalAddress = 0x1000;
        hdr.Size = 16;
        memset(outBuf, 0, sizeof(outBuf));
        uint8_t outBuf2[16] = {};
        bytesReturned = 0;
        ok = DeviceIoControl(hDev, 0x14,
            &hdr, sizeof(hdr),
            outBuf2, 16,
            &bytesReturned, nullptr);

        printf("  ok=%d, bytesReturned=%lu, err=0x%lX\n", ok, bytesReturned, GetLastError());
        if (ok)
        {
            printf("  Data at phys 0x1000: ");
            for (int i = 0; i < 16; i++) printf("%02X ", outBuf2[i]);
            printf("\n");
        }

        // If read failed, try Cmd 0x10 (scatter-gather read) as fallback
        if (!ok)
        {
            printf("\n--- Fallback: Testing Cmd 0x10 (Scatter-Gather Read) ---\n");
            uint8_t scatterBuf[32] = {};
            // Cmd 0x10 structure might be: PhysAddr(8) + Size(4) + UserBufPtr(8)?
            // Or it might be different. Let me just try with similar header.
            SIV_MAP_RW_INPUT hdr2{};
            hdr2.PhysicalAddress = 0x1000;
            hdr2.Size = 16;
            hdr2.Padding = 0;
            hdr2.Flags = 0;
            bytesReturned = 0;
            ok = DeviceIoControl(hDev, 0x10,
                &hdr2, sizeof(hdr2),
                scatterBuf, 16,
                &bytesReturned, nullptr);
            printf("  Cmd 0x10: ok=%d, bytes=%lu, err=0x%lX\n", ok, bytesReturned, GetLastError());
            if (ok)
            {
                printf("  Data: ");
                for (DWORD i = 0; i < bytesReturned && i < 16; i++) printf("%02X ", scatterBuf[i]);
                printf("\n");
            }
        }

        CloseHandle(hDev);
    }
    else
    {
        printf("\n  No device path worked!\n");
    }

    // Cleanup
    printf("\n--- Cleanup ---\n");
    SERVICE_STATUS ss;
    ControlService(svc, SERVICE_CONTROL_STOP, &ss);
    DeleteService(svc);
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    printf("Done\n");
    fflush(stdout);
    fclose(stdout);
    return 0;
}
