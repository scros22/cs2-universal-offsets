#pragma once

// ---------------------------------------------------------------
// syscall.h — Direct NT syscall stubs (bypass ntdll hooks)
//
// Resolves System Service Numbers (SSN) at runtime from ntdll
// and builds tiny RX shellcode stubs:
//     mov r10, rcx
//     mov eax, <SSN>
//     syscall
//     ret
//
// Falls back to Halo's Gate (neighbor scan) if the target stub
// is inline-hooked (first bytes != 4C 8B D1 B8).
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include "xorstr.h"

namespace DirectSyscall
{
    // Stub layout: 11 bytes per syscall
    // 49 89 CA        mov r10, rcx
    // B8 XX XX XX XX  mov eax, SSN
    // 0F 05           syscall
    // C3              ret
    struct SyscallStub
    {
        uint8_t code[16]; // 11 used + padding for alignment
    };

    inline uint8_t*  stubPage  = nullptr;
    inline int        stubCount = 0;
    inline bool       ready     = false;

    // ---- SSN resolution ----

    // Try reading SSN directly from function bytes (unhook case)
    inline DWORD ReadSSN(uint8_t* fn)
    {
        // Standard ntdll Nt stub: 4C 8B D1 B8 XX XX XX XX ...
        if (fn[0] == 0x4C && fn[1] == 0x8B && fn[2] == 0xD1 && fn[3] == 0xB8)
            return *reinterpret_cast<DWORD*>(fn + 4);
        return 0xFFFFFFFF;
    }

    // Halo's Gate: if the target is hooked, scan neighbors (±1,2,3...)
    // Neighboring Nt stubs have SSN ±N from the target.
    inline DWORD ResolveSSN(uint8_t* fn)
    {
        DWORD ssn = ReadSSN(fn);
        if (ssn != 0xFFFFFFFF) return ssn;

        // Scan up to 32 neighbors in both directions
        for (int dist = 1; dist <= 32; ++dist)
        {
            // Down (higher SSN)
            uint8_t* down = fn + dist * 0x20; // ntdll stubs are ~32 bytes apart
            DWORD s = ReadSSN(down);
            if (s != 0xFFFFFFFF) return s - dist;

            // Up (lower SSN)
            uint8_t* up = fn - dist * 0x20;
            s = ReadSSN(up);
            if (s != 0xFFFFFFFF) return s + dist;
        }
        return 0xFFFFFFFF; // truly unresolvable
    }

    // Build a callable stub for a given SSN
    inline void* BuildStub(DWORD ssn)
    {
        if (!stubPage || ssn == 0xFFFFFFFF) return nullptr;
        int offset = stubCount * 16;
        if (offset + 16 > 4096) return nullptr; // page full

        uint8_t* p = stubPage + offset;
        p[0] = 0x49; p[1] = 0x89; p[2] = 0xCA;         // mov r10, rcx
        p[3] = 0xB8;                                      // mov eax, ...
        *reinterpret_cast<DWORD*>(p + 4) = ssn;           // ... SSN
        p[8] = 0x0F; p[9] = 0x05;                         // syscall
        p[10] = 0xC3;                                      // ret
        stubCount++;
        return p;
    }

    // ---- Typed syscall function pointers ----

    // NtProtectVirtualMemory(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG)
    using NtProtectVirtualMemory_t = NTSTATUS(NTAPI*)(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        PSIZE_T RegionSize,
        ULONG NewProtect,
        PULONG OldProtect);

    // NtSetInformationThread(HANDLE, ULONG, PVOID, ULONG)
    using NtSetInformationThread_t = NTSTATUS(NTAPI*)(
        HANDLE ThreadHandle,
        ULONG ThreadInformationClass,
        PVOID ThreadInformation,
        ULONG ThreadInformationLength);

    // NtQueryVirtualMemory(HANDLE, PVOID, ULONG, PVOID, SIZE_T, PSIZE_T)
    using NtQueryVirtualMemory_t = NTSTATUS(NTAPI*)(
        HANDLE ProcessHandle,
        PVOID BaseAddress,
        ULONG MemoryInformationClass,
        PVOID MemoryInformation,
        SIZE_T MemoryInformationLength,
        PSIZE_T ReturnLength);

    inline NtProtectVirtualMemory_t pNtProtectVirtualMemory = nullptr;
    inline NtSetInformationThread_t pNtSetInformationThread = nullptr;
    inline NtQueryVirtualMemory_t   pNtQueryVirtualMemory   = nullptr;

    // ---- Init: resolve SSNs and build stubs ----
    inline bool Init()
    {
        HMODULE ntdll = GetModuleHandleW(XW(L"ntdll.dll"));
        if (!ntdll) return false;

        // Allocate one RWX page for stubs (we'll make it RX after building)
        stubPage = reinterpret_cast<uint8_t*>(
            VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (!stubPage) return false;

        // Resolve each syscall
        auto resolve = [&](const char* name) -> void* {
            auto fn = reinterpret_cast<uint8_t*>(GetProcAddress(ntdll, name));
            if (!fn) return nullptr;
            DWORD ssn = ResolveSSN(fn);
            return BuildStub(ssn);
        };

        pNtProtectVirtualMemory = reinterpret_cast<NtProtectVirtualMemory_t>(
            resolve(XS("NtProtectVirtualMemory")));
        pNtSetInformationThread = reinterpret_cast<NtSetInformationThread_t>(
            resolve(XS("NtSetInformationThread")));
        pNtQueryVirtualMemory = reinterpret_cast<NtQueryVirtualMemory_t>(
            resolve(XS("NtQueryVirtualMemory")));

        // Seal stubs: RW → RX
        DWORD old;
        VirtualProtect(stubPage, 4096, PAGE_EXECUTE_READ, &old);

        ready = (pNtProtectVirtualMemory && pNtSetInformationThread);
        return ready;
    }

    // ---- Convenience wrappers ----

    // Drop-in VirtualProtect replacement using direct syscall
    inline bool ProtectMemory(void* addr, size_t size, DWORD newProt, DWORD* oldProt)
    {
        if (pNtProtectVirtualMemory)
        {
            PVOID base = addr;
            SIZE_T sz = size;
            NTSTATUS st = pNtProtectVirtualMemory(
                GetCurrentProcess(), &base, &sz, newProt, reinterpret_cast<PULONG>(oldProt));
            return st >= 0; // NT_SUCCESS
        }
        // Fallback
        return VirtualProtect(addr, size, newProt, oldProt) != 0;
    }

    // ThreadHideFromDebugger via direct syscall
    inline bool HideThread(HANDLE hThread)
    {
        if (pNtSetInformationThread)
        {
            NTSTATUS st = pNtSetInformationThread(hThread, 0x11, nullptr, 0);
            return st >= 0;
        }
        return false;
    }

    // Thread start address spoof via direct syscall
    inline bool SpoofThreadStartAddr(HANDLE hThread, PVOID fakeAddr)
    {
        if (pNtSetInformationThread)
        {
            NTSTATUS st = pNtSetInformationThread(
                hThread, 9 /*ThreadQuerySetWin32StartAddress*/, &fakeAddr, sizeof(fakeAddr));
            return st >= 0;
        }
        return false;
    }

    inline void Shutdown()
    {
        if (stubPage)
        {
            // Zero stubs before freeing
            DWORD old;
            VirtualProtect(stubPage, 4096, PAGE_READWRITE, &old);
            SecureZeroMemory(stubPage, 4096);
            VirtualFree(stubPage, 0, MEM_RELEASE);
            stubPage = nullptr;
        }
        pNtProtectVirtualMemory = nullptr;
        pNtSetInformationThread = nullptr;
        pNtQueryVirtualMemory   = nullptr;
        ready = false;
    }
}
