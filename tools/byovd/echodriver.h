#pragma once
// ============================================================
// echodriver.h — echo_driver.sys BYOVD Provider
// Modern HVCI-compatible physical memory access via MmCopyMemory
// Signed by: Microsoft Windows Hardware Compatibility Publisher
// SHA256: ada2b855757c9062231f5ed4e80365b8d8094e9adbce8f26d1ff5ea0b7a70c77
// ============================================================

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "driver_provider.h"

namespace BYOVD
{
    // Echo Driver IOCTL codes
    // 0x9e6a0594: Physical memory read (MmCopyMemory)
    // 0x9e6a0598: Physical memory write (MmCopyMemory)
    constexpr DWORD ECHO_READ_PHYS  = 0x9e6a0594;
    constexpr DWORD ECHO_WRITE_PHYS = 0x9e6a0598;

    #pragma pack(push, 1)
    struct ECHO_COPY_MEMORY
    {
        uint64_t PhysicalAddress;
        void*    Buffer;
        uint32_t Size;
    };
    #pragma pack(pop)

    class EchoDriverProvider : public IDriverProvider
    {
        HANDLE m_hDevice = INVALID_HANDLE_VALUE;

        static constexpr const wchar_t* SVC_NAME = L"EchoDriverCS2";
        static constexpr const wchar_t* DEV_PATH = L"\\\\.\\echo_driver";
        static constexpr const char*    DISP_NAME = "EchoDriver (HVCI-Safe)";

    public:
        const wchar_t* ServiceName() const override { return SVC_NAME; }
        const char*    DisplayName() const override  { return DISP_NAME; }

        bool Open() override
        {
            m_hDevice = CreateFileW(
                DEV_PATH,
                GENERIC_READ | GENERIC_WRITE,
                0, nullptr, OPEN_EXISTING, 0, nullptr);

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] EchoDriver device not found (cert revoked - unusable)\n");
                return false;

                m_hDevice = CreateFileW(
                    DEV_PATH,
                    GENERIC_READ | GENERIC_WRITE,
                    0, nullptr, OPEN_EXISTING, 0, nullptr);
            }

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] Failed to open EchoDriver device (0x%lX)\n", GetLastError());
                return false;
            }

            printf("[BYOVD] EchoDriver device opened successfully (HVCI mode active)\n");
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

        bool IsOpen() const override { return m_hDevice != INVALID_HANDLE_VALUE; }

        // Implementation of IDriverProvider interface
        bool ReadBuffer(uintptr_t physicalAddress, void* buffer, size_t size) override
        {
            if (m_hDevice == INVALID_HANDLE_VALUE) return false;

            ECHO_COPY_MEMORY cmd{};
            cmd.PhysicalAddress = physicalAddress;
            cmd.Buffer = buffer;
            cmd.Size = static_cast<uint32_t>(size);

            DWORD bytesReturned = 0;
            return DeviceIoControl(m_hDevice, ECHO_READ_PHYS,
                &cmd, sizeof(cmd),
                &cmd, sizeof(cmd),
                &bytesReturned, nullptr);
        }

        bool WriteBuffer(uintptr_t physicalAddress, const void* buffer, size_t size) override
        {
            if (m_hDevice == INVALID_HANDLE_VALUE) return false;

            ECHO_COPY_MEMORY cmd{};
            cmd.PhysicalAddress = physicalAddress;
            cmd.Buffer = const_cast<void*>(buffer);
            cmd.Size = static_cast<uint32_t>(size);

            DWORD bytesReturned = 0;
            return DeviceIoControl(m_hDevice, ECHO_WRITE_PHYS,
                &cmd, sizeof(cmd),
                &cmd, sizeof(cmd),
                &bytesReturned, nullptr);
        }

        // Helper for single values
        template <typename T>
        T ReadPhysicalT(uintptr_t physicalAddress)
        {
            T val{};
            ReadBuffer(physicalAddress, &val, sizeof(T));
            return val;
        }
    };
}
