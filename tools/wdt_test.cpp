// wdt_test.cpp — Quick test to debug WDTKernel device creation
// Compile: cl /EHsc /O2 wdt_test.cpp advapi32.lib
#include <Windows.h>
#include <cstdio>

int main()
{
    // Redirect stdout to a file so we can read results
    freopen("C:\\Users\\Samuel\\wdt_result.txt", "w", stdout);
    printf("=== WDTKernel.sys Device Test ===\n\n");

    // Step 1: Clean up any existing service
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scm) { printf("OpenSCManager failed: 0x%lX (run as admin!)\n", GetLastError()); return 1; }

    // Try both service names
    const wchar_t* names[] = { L"WDTcs2", L"WDTKernel", L"WDT" };
    for (int n = 0; n < 3; n++)
    {
        auto* name = names[n];
        SC_HANDLE svc = OpenServiceW(scm, name, SERVICE_ALL_ACCESS);
        if (svc)
        {
            SERVICE_STATUS ss;
            ControlService(svc, SERVICE_CONTROL_STOP, &ss);
            DeleteService(svc);
            CloseServiceHandle(svc);
            printf("Cleaned up service: %ls\n", name);
            Sleep(1000);
        }
    }

    // Step 2: Copy driver to temp
    wchar_t drvPath[MAX_PATH];
    GetTempPathW(MAX_PATH, drvPath);
    wcscat_s(drvPath, L"WDTKernel.sys");

    // Check it exists
    if (GetFileAttributesW(drvPath) == INVALID_FILE_ATTRIBUTES)
    {
        printf("WDTKernel.sys not found at: %ls\n", drvPath);
        CloseServiceHandle(scm);
        return 1;
    }
    printf("Driver: %ls\n\n", drvPath);

    // Step 3: Create and start service with original name "WDTKernel"
    printf("--- Creating service 'WDTKernel' ---\n");
    SC_HANDLE svc = CreateServiceW(
        scm, L"WDTKernel", L"WDTKernel",
        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
        drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (!svc)
    {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS)
            svc = OpenServiceW(scm, L"WDTKernel", SERVICE_ALL_ACCESS);
        else
        {
            printf("CreateService failed: 0x%lX\n", err);
            CloseServiceHandle(scm);
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
            // Common errors:
            // 0x241 = ERROR_SERVICE_DISABLED (driver blocked)
            // 0x57A = ERROR_COULD_NOT_INTERPRET (driver init failed)
            // 0x5AD = ERROR_WORKING_SET_QUOTA
            // 0x643 = ERROR_INSTALL_FAILURE
            DeleteService(svc);
            CloseServiceHandle(svc);
            CloseServiceHandle(scm);
            return 1;
        }
        printf("Service already running\n");
    }
    else
    {
        printf("Service started OK\n");
    }

    // Step 4: Try all possible device paths
    printf("\n--- Testing device paths ---\n");
    const wchar_t* paths[] = {
        L"\\\\.\\__WDT__",
        L"\\\\.\\WDTKernel",
        L"\\\\.\\WDT",
        L"\\\\.\\GLOBALROOT\\Device\\__WDT__",
        L"\\\\.\\GLOBALROOT\\Device\\WDTKernel",
    };

    HANDLE hDev = INVALID_HANDLE_VALUE;
    for (auto* path : paths)
    {
        HANDLE h = CreateFileW(path, GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr);
        DWORD err = GetLastError();
        if (h != INVALID_HANDLE_VALUE)
        {
            printf("  %ls -> OPENED! (handle=%p)\n", path, h);
            hDev = h;
            break;
        }
        else
        {
            printf("  %ls -> err=0x%lX (%lu)\n", path, err, err);
        }
    }

    // Step 5: If opened, try a simple read IOCTL
    if (hDev != INVALID_HANDLE_VALUE)
    {
        printf("\n--- Testing IOCTL 0x9C412400 (Read DWORD at phys 0) ---\n");
        UINT64 physAddr = 0; // Read from physical address 0
        DWORD outVal = 0;
        DWORD bytesReturned = 0;
        BOOL ok = DeviceIoControl(hDev, 0x9C412400,
            &physAddr, sizeof(physAddr),
            &outVal, sizeof(outVal),
            &bytesReturned, nullptr);
        printf("  Result: ok=%d, value=0x%08X, bytes=%lu, err=0x%lX\n",
            ok, outVal, bytesReturned, GetLastError());
        CloseHandle(hDev);
    }
    else
    {
        printf("\n  No device path worked!\n");

        // Enumerate all device objects to find it
        printf("\n--- Checking if driver is truly loaded ---\n");
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        typedef NTSTATUS(NTAPI* NtQueryDirectoryObjectFn)(HANDLE, PVOID, ULONG, BOOLEAN, BOOLEAN, PULONG, PULONG);
        typedef NTSTATUS(NTAPI* NtOpenDirectoryObjectFn)(PHANDLE, ACCESS_MASK, PVOID);

        // Just check service state
        SERVICE_STATUS_PROCESS ssp;
        DWORD needed;
        if (QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &needed))
        {
            printf("  Service state: %lu (4=running), PID: %lu\n", ssp.dwCurrentState, ssp.dwProcessId);
        }
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
