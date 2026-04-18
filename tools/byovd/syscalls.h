#pragma once
// ============================================================
// syscalls.h — Direct Syscall Infrastructure
// Reads syscall service numbers (SSNs) from ntdll.dll on disk,
// builds RX shellcode stubs that issue 'syscall' directly.
// Bypasses ALL ntdll.dll inline hooks (EDR, AC, etc.).
// ============================================================

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

#ifndef STATUS_NOT_FOUND
#define STATUS_NOT_FOUND ((NTSTATUS)0xC0000225L)
#endif

// NT types that winternl.h doesn't always fully define
#ifndef _NTDEF_
typedef struct _UNICODE_STRING_SC {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING_SC;

typedef struct _OBJECT_ATTRIBUTES_SC {
    ULONG           Length;
    HANDLE          RootDirectory;
    UNICODE_STRING_SC* ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES_SC;

typedef struct _CLIENT_ID_SC {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID_SC;
#endif

namespace DirectSyscall
{
    // Syscall stub: 12 bytes per function
    //   mov r10, rcx        ; 49 89 CA        (3 bytes)
    //   mov eax, <SSN>      ; B8 XX XX 00 00  (5 bytes)
    //   syscall              ; 0F 05           (2 bytes)
    //   ret                  ; C3              (1 byte)
    //   nop                  ; 90              (1 byte pad)
    static constexpr size_t STUB_SIZE = 12;
    static constexpr size_t MAX_STUBS = 32;

    struct SyscallTable
    {
        void*  stubPage    = nullptr;  // RX page with all stubs
        size_t stubCount   = 0;
        bool   initialized = false;

        struct Entry
        {
            const char* name;
            DWORD       ssn;
            void*       stub;
        };
        Entry entries[MAX_STUBS] = {};
    };

    inline SyscallTable g_table;

    // Read SSN from ntdll export by finding the mov eax, imm32 pattern
    inline DWORD GetSSN(HMODULE hNtdll, const char* funcName)
    {
        auto fn = reinterpret_cast<uint8_t*>(GetProcAddress(hNtdll, funcName));
        if (!fn) return MAXDWORD;

        // Standard pattern: mov r10, rcx; mov eax, SSN
        // 4C 8B D1 B8 XX XX XX XX
        for (int i = 0; i < 32; i++)
        {
            if (fn[i] == 0x4C && fn[i+1] == 0x8B && fn[i+2] == 0xD1 &&
                fn[i+3] == 0xB8)
            {
                return *reinterpret_cast<DWORD*>(&fn[i + 4]);
            }
        }

        // Hooked pattern: jmp <hook> — look for mov eax further down
        // or scan neighboring syscalls to interpolate
        // Try offset 8 (some hooks leave the mov eax intact after the jmp)
        for (int i = 0; i < 64; i++)
        {
            if (fn[i] == 0xB8 && fn[i+5] == 0x0F && fn[i+6] == 0x05)
                return *reinterpret_cast<DWORD*>(&fn[i + 1]);
        }

        return MAXDWORD;
    }

    // Alternative: read SSN from ntdll on disk (completely avoids in-memory hooks)
    inline DWORD GetSSNFromDisk(const char* funcName)
    {
        // Map ntdll from disk
        wchar_t path[MAX_PATH];
        GetSystemDirectoryW(path, MAX_PATH);
        wcscat_s(path, L"\\ntdll.dll");

        HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, 0, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return MAXDWORD;

        HANDLE hMap = CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        CloseHandle(hFile);
        if (!hMap) return MAXDWORD;

        auto base = static_cast<uint8_t*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
        CloseHandle(hMap);
        if (!base) return MAXDWORD;

        DWORD ssn = MAXDWORD;

        // Parse PE to find export
        auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto nt  = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
        auto& expDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        if (expDir.VirtualAddress)
        {
            // Convert RVA to file offset
            auto sections = IMAGE_FIRST_SECTION(nt);
            auto rvaToOffset = [&](DWORD rva) -> DWORD {
                for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++)
                {
                    if (rva >= sections[i].VirtualAddress &&
                        rva < sections[i].VirtualAddress + sections[i].SizeOfRawData)
                        return rva - sections[i].VirtualAddress + sections[i].PointerToRawData;
                }
                return 0;
            };

            DWORD expOffset = rvaToOffset(expDir.VirtualAddress);
            if (expOffset)
            {
                auto exp = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(base + expOffset);
                auto names = reinterpret_cast<DWORD*>(base + rvaToOffset(exp->AddressOfNames));
                auto ords  = reinterpret_cast<WORD*>(base + rvaToOffset(exp->AddressOfNameOrdinals));
                auto funcs = reinterpret_cast<DWORD*>(base + rvaToOffset(exp->AddressOfFunctions));

                for (DWORD i = 0; i < exp->NumberOfNames; i++)
                {
                    DWORD nameOff = rvaToOffset(names[i]);
                    if (!nameOff) continue;
                    const char* name = reinterpret_cast<const char*>(base + nameOff);
                    if (strcmp(name, funcName) != 0) continue;

                    DWORD funcRva = funcs[ords[i]];
                    DWORD funcOff = rvaToOffset(funcRva);
                    if (!funcOff) break;

                    auto fn = base + funcOff;
                    // Look for: 4C 8B D1 B8 XX XX XX XX
                    for (int j = 0; j < 32; j++)
                    {
                        if (fn[j] == 0x4C && fn[j+1] == 0x8B && fn[j+2] == 0xD1 &&
                            fn[j+3] == 0xB8)
                        {
                            ssn = *reinterpret_cast<DWORD*>(&fn[j + 4]);
                            break;
                        }
                    }
                    break;
                }
            }
        }

        UnmapViewOfFile(base);
        return ssn;
    }

    inline bool Init()
    {
        if (g_table.initialized) return true;

        // Syscalls we need
        static const char* needed[] = {
            "NtOpenProcess",
            "NtAllocateVirtualMemory",
            "NtProtectVirtualMemory",
            "NtWriteVirtualMemory",
            "NtOpenThread",
            "NtSuspendThread",
            "NtResumeThread",
            "NtGetContextThread",
            "NtSetContextThread",
            "NtCreateThreadEx",
            "NtQueueApcThread",
            "NtClose",
            "NtQuerySystemInformation",
        };
        constexpr size_t count = _countof(needed);
        static_assert(count <= MAX_STUBS);

        // Allocate RWX page for stubs (will flip to RX after writing)
        g_table.stubPage = VirtualAlloc(nullptr, STUB_SIZE * count,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!g_table.stubPage) return false;

        auto stubBytes = static_cast<uint8_t*>(g_table.stubPage);

        // Resolve SSNs from disk (immune to in-memory hooks)
        for (size_t i = 0; i < count; i++)
        {
            DWORD ssn = GetSSNFromDisk(needed[i]);
            if (ssn == MAXDWORD)
            {
                // Fallback: try from loaded ntdll
                HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
                if (hNtdll) ssn = GetSSN(hNtdll, needed[i]);
            }

            if (ssn == MAXDWORD)
            {
                printf("[Syscall] Failed to resolve SSN for %s\n", needed[i]);
                VirtualFree(g_table.stubPage, 0, MEM_RELEASE);
                g_table.stubPage = nullptr;
                return false;
            }

            // Build stub
            uint8_t* stub = stubBytes + i * STUB_SIZE;
            stub[0] = 0x49; stub[1] = 0x89; stub[2] = 0xCA;               // mov r10, rcx
            stub[3] = 0xB8;                                                 // mov eax, imm32
            memcpy(&stub[4], &ssn, 4);
            stub[8] = 0x0F; stub[9] = 0x05;                                // syscall
            stub[10] = 0xC3;                                                // ret
            stub[11] = 0x90;                                                // nop (align)

            g_table.entries[i] = { needed[i], ssn, stub };
            g_table.stubCount++;
        }

        // Flip to RX (no write after this)
        DWORD old;
        VirtualProtect(g_table.stubPage, STUB_SIZE * count, PAGE_EXECUTE_READ, &old);

        g_table.initialized = true;
        printf("[Syscall] %zu direct syscall stubs ready\n", count);
        return true;
    }

    inline void Cleanup()
    {
        if (g_table.stubPage)
        {
            VirtualFree(g_table.stubPage, 0, MEM_RELEASE);
            g_table.stubPage = nullptr;
        }
        g_table.initialized = false;
        g_table.stubCount = 0;
    }

    // Get a typed function pointer to a syscall stub
    template <typename Fn>
    Fn GetStub(const char* name)
    {
        for (size_t i = 0; i < g_table.stubCount; i++)
        {
            if (strcmp(g_table.entries[i].name, name) == 0)
                return reinterpret_cast<Fn>(g_table.entries[i].stub);
        }
        return nullptr;
    }

    // ============================================================
    // Typed syscall wrappers
    // ============================================================

    // NtOpenProcess
    using NtOpenProcess_t = NTSTATUS(NTAPI*)(
        PHANDLE ProcessHandle,
        ACCESS_MASK DesiredAccess,
        OBJECT_ATTRIBUTES_SC* ObjectAttributes,
        CLIENT_ID_SC* ClientId);

    inline NTSTATUS NtOpenProcess(PHANDLE handle, ACCESS_MASK access,
                                  OBJECT_ATTRIBUTES_SC* oa, CLIENT_ID_SC* cid)
    {
        auto fn = GetStub<NtOpenProcess_t>("NtOpenProcess");
        return fn ? fn(handle, access, oa, cid) : STATUS_NOT_FOUND;
    }

    // NtAllocateVirtualMemory
    using NtAllocateVirtualMemory_t = NTSTATUS(NTAPI*)(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
        ULONG AllocationType,
        ULONG Protect);

    inline NTSTATUS NtAllocateVirtualMemory(HANDLE proc, PVOID* base,
        ULONG_PTR zeroBits, PSIZE_T size, ULONG allocType, ULONG protect)
    {
        auto fn = GetStub<NtAllocateVirtualMemory_t>("NtAllocateVirtualMemory");
        return fn ? fn(proc, base, zeroBits, size, allocType, protect) : STATUS_NOT_FOUND;
    }

    // NtProtectVirtualMemory
    using NtProtectVirtualMemory_t = NTSTATUS(NTAPI*)(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        PSIZE_T RegionSize,
        ULONG NewProtect,
        PULONG OldProtect);

    inline NTSTATUS NtProtectVirtualMemory(HANDLE proc, PVOID* base,
        PSIZE_T size, ULONG newProt, PULONG oldProt)
    {
        auto fn = GetStub<NtProtectVirtualMemory_t>("NtProtectVirtualMemory");
        return fn ? fn(proc, base, size, newProt, oldProt) : STATUS_NOT_FOUND;
    }

    // NtWriteVirtualMemory (fallback if kernel R/W unavailable)
    using NtWriteVirtualMemory_t = NTSTATUS(NTAPI*)(
        HANDLE ProcessHandle,
        PVOID BaseAddress,
        PVOID Buffer,
        SIZE_T NumberOfBytesToWrite,
        PSIZE_T NumberOfBytesWritten);

    inline NTSTATUS NtWriteVirtualMemory(HANDLE proc, PVOID base,
        PVOID buf, SIZE_T size, PSIZE_T written)
    {
        auto fn = GetStub<NtWriteVirtualMemory_t>("NtWriteVirtualMemory");
        return fn ? fn(proc, base, buf, size, written) : STATUS_NOT_FOUND;
    }

    // NtOpenThread
    using NtOpenThread_t = NTSTATUS(NTAPI*)(
        PHANDLE ThreadHandle,
        ACCESS_MASK DesiredAccess,
        OBJECT_ATTRIBUTES_SC* ObjectAttributes,
        CLIENT_ID_SC* ClientId);

    inline NTSTATUS NtOpenThread(PHANDLE handle, ACCESS_MASK access,
                                 OBJECT_ATTRIBUTES_SC* oa, CLIENT_ID_SC* cid)
    {
        auto fn = GetStub<NtOpenThread_t>("NtOpenThread");
        return fn ? fn(handle, access, oa, cid) : STATUS_NOT_FOUND;
    }

    // NtSuspendThread
    using NtSuspendThread_t = NTSTATUS(NTAPI*)(
        HANDLE ThreadHandle,
        PULONG PreviousSuspendCount);

    inline NTSTATUS NtSuspendThread(HANDLE thread, PULONG prevCount)
    {
        auto fn = GetStub<NtSuspendThread_t>("NtSuspendThread");
        return fn ? fn(thread, prevCount) : STATUS_NOT_FOUND;
    }

    // NtResumeThread
    using NtResumeThread_t = NTSTATUS(NTAPI*)(
        HANDLE ThreadHandle,
        PULONG PreviousSuspendCount);

    inline NTSTATUS NtResumeThread(HANDLE thread, PULONG prevCount)
    {
        auto fn = GetStub<NtResumeThread_t>("NtResumeThread");
        return fn ? fn(thread, prevCount) : STATUS_NOT_FOUND;
    }

    // NtGetContextThread
    using NtGetContextThread_t = NTSTATUS(NTAPI*)(
        HANDLE ThreadHandle,
        PCONTEXT ThreadContext);

    inline NTSTATUS NtGetContextThread(HANDLE thread, PCONTEXT ctx)
    {
        auto fn = GetStub<NtGetContextThread_t>("NtGetContextThread");
        return fn ? fn(thread, ctx) : STATUS_NOT_FOUND;
    }

    // NtSetContextThread
    using NtSetContextThread_t = NTSTATUS(NTAPI*)(
        HANDLE ThreadHandle,
        PCONTEXT ThreadContext);

    inline NTSTATUS NtSetContextThread(HANDLE thread, PCONTEXT ctx)
    {
        auto fn = GetStub<NtSetContextThread_t>("NtSetContextThread");
        return fn ? fn(thread, ctx) : STATUS_NOT_FOUND;
    }

    // NtCreateThreadEx
    using NtCreateThreadEx_t = NTSTATUS(NTAPI*)(
        PHANDLE ThreadHandle,
        ACCESS_MASK DesiredAccess,
        PVOID ObjectAttributes,
        HANDLE ProcessHandle,
        PVOID StartRoutine,
        PVOID Argument,
        ULONG CreateFlags,
        SIZE_T ZeroBits,
        SIZE_T StackSize,
        SIZE_T MaximumStackSize,
        PVOID AttributeList);

    inline NTSTATUS NtCreateThreadEx(PHANDLE hThread, ACCESS_MASK access,
        PVOID oa, HANDLE proc, PVOID start, PVOID arg,
        ULONG flags, SIZE_T zeroBits, SIZE_T stackSz, SIZE_T maxStack, PVOID attrs)
    {
        auto fn = GetStub<NtCreateThreadEx_t>("NtCreateThreadEx");
        return fn ? fn(hThread, access, oa, proc, start, arg,
                       flags, zeroBits, stackSz, maxStack, attrs) : STATUS_NOT_FOUND;
    }

    // NtQueueApcThread
    using NtQueueApcThread_t = NTSTATUS(NTAPI*)(
        HANDLE ThreadHandle,
        PVOID ApcRoutine,
        PVOID ApcArgument1,
        PVOID ApcArgument2,
        PVOID ApcArgument3);

    inline NTSTATUS NtQueueApcThread(HANDLE thread, PVOID routine,
        PVOID arg1, PVOID arg2, PVOID arg3)
    {
        auto fn = GetStub<NtQueueApcThread_t>("NtQueueApcThread");
        return fn ? fn(thread, routine, arg1, arg2, arg3) : STATUS_NOT_FOUND;
    }

    // NtClose
    using NtClose_t = NTSTATUS(NTAPI*)(HANDLE Handle);

    inline NTSTATUS NtClose(HANDLE handle)
    {
        auto fn = GetStub<NtClose_t>("NtClose");
        return fn ? fn(handle) : STATUS_NOT_FOUND;
    }

    // Helper: open process handle via direct syscall
    inline HANDLE OpenProcessDirect(DWORD pid, ACCESS_MASK access)
    {
        HANDLE hProc = nullptr;
        OBJECT_ATTRIBUTES_SC oa{};
        oa.Length = sizeof(oa);
        CLIENT_ID_SC cid{};
        cid.UniqueProcess = ULongToHandle(pid);

        NTSTATUS st = NtOpenProcess(&hProc, access, &oa, &cid);
        if (st != 0)
        {
            printf("[Syscall] NtOpenProcess(PID=%lu) failed: 0x%08lX\n", pid, st);
            return nullptr;
        }
        return hProc;
    }

    // Helper: open thread handle via direct syscall
    inline HANDLE OpenThreadDirect(DWORD tid, ACCESS_MASK access)
    {
        HANDLE hThread = nullptr;
        OBJECT_ATTRIBUTES_SC oa{};
        oa.Length = sizeof(oa);
        CLIENT_ID_SC cid{};
        cid.UniqueThread = ULongToHandle(tid);

        NTSTATUS st = NtOpenThread(&hThread, access, &oa, &cid);
        if (st != 0) return nullptr;
        return hThread;
    }
}
