#pragma once
// ============================================================
// driver_provider.h — Abstract BYOVD Driver Interface
// All vulnerable drivers implement this interface so the
// kernel context and mapper work identically regardless of
// which driver is loaded underneath.
// ============================================================

#include <Windows.h>
#include <cstdint>

namespace BYOVD
{
    // Supported driver backends
    enum class DriverType
    {
        RTCore64,   // MSI Afterburner — CVE-2019-16098
        CorMem,     // CorMem.sys — physical memory map/unmap
        WDTKernel,  // Dell Watchdog Timer — WHQL signed, HVCI compatible
        SIVDriver,  // SIVX64.sys — WHQL signed, HVCI compatible, 0/73 VT
        EchoDriver  // echo_driver.sys — HVCI/MmCopyMemory, WHQL signed
    };

    // Abstract interface for kernel memory R/W providers
    class IDriverProvider
    {
    public:
        virtual ~IDriverProvider() = default;

        virtual bool Open() = 0;
        virtual void Close() = 0;
        virtual bool IsOpen() const = 0;

        // Read a value at a kernel virtual address
        template <typename T>
        T ReadMemory(uintptr_t address)
        {
            T result{};
            ReadBuffer(address, &result, sizeof(T));
            return result;
        }

        // Write a value at a kernel virtual address
        template <typename T>
        bool WriteMemory(uintptr_t address, const T& value)
        {
            return WriteBuffer(address, &value, sizeof(T));
        }

        // Bulk read/write (kernel virtual addresses, uses system CR3)
        virtual bool ReadBuffer(uintptr_t address, void* buffer, size_t size) = 0;
        virtual bool WriteBuffer(uintptr_t address, const void* buffer, size_t size) = 0;

        // Cross-process R/W (user virtual addresses, uses process CR3)
        // Override in drivers that support physical memory + page table walking
        virtual bool ReadBufferProcess(uint64_t processCR3, uintptr_t va, void* buffer, size_t size)
        { (void)processCR3; (void)va; (void)buffer; (void)size; return false; }
        virtual bool WriteBufferProcess(uint64_t processCR3, uintptr_t va, const void* buffer, size_t size)
        { (void)processCR3; (void)va; (void)buffer; (void)size; return false; }

        // Driver lifecycle
        virtual const wchar_t* ServiceName() const = 0;
        virtual const char* DisplayName() const = 0;
    };

    // Cleanup any loaded driver via its service name
    inline void UnloadDriverService(const wchar_t* svcName, const wchar_t* drvFileName)
    {
        SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (!scm) return;

        SC_HANDLE svc = OpenServiceW(scm, svcName, SERVICE_ALL_ACCESS);
        if (svc)
        {
            SERVICE_STATUS ss;
            ControlService(svc, SERVICE_CONTROL_STOP, &ss);
            DeleteService(svc);
            CloseServiceHandle(svc);
        }
        CloseServiceHandle(scm);

        if (drvFileName)
        {
            wchar_t drvPath[MAX_PATH];
            GetTempPathW(MAX_PATH, drvPath);
            wcscat_s(drvPath, drvFileName);
            DeleteFileW(drvPath);
        }

        printf("[BYOVD] Driver service '%ls' cleaned up\n", svcName);
    }
}
