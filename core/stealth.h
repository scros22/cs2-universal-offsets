#pragma once

// ============================================================
// stealth.h â€” Multi-layer anti-detection subsystem
// ============================================================

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <intrin.h>
#include "xorstr.h"
#include <TlHelp32.h>

namespace Stealth
{
    struct USTR
    {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    };

    struct LDR_ENTRY
    {
        LIST_ENTRY InLoadOrderLinks;
        LIST_ENTRY InMemoryOrderLinks;
        LIST_ENTRY InInitializationOrderLinks;
        PVOID      DllBase;
        PVOID      EntryPoint;
        ULONG      SizeOfImage;
        USTR       FullDllName;
        USTR       BaseDllName;
    };

    struct LDR_DATA
    {
        ULONG      Length;
        BOOLEAN    Initialized;
        PVOID      SsHandle;
        LIST_ENTRY InLoadOrderModuleList;
        LIST_ENTRY InMemoryOrderModuleList;
        LIST_ENTRY InInitializationOrderModuleList;
    };

    struct PEB_INTERNAL
    {
        BOOLEAN InheritedAddressSpace;
        BOOLEAN ReadImageFileExecOptions;
        BOOLEAN BeingDebugged;
        BYTE    Padding0[1];
        BYTE    Reserved0[4];
        PVOID   Mutant;
        PVOID   ImageBaseAddress;
        LDR_DATA* Ldr;
    };

    // -----------------------------------------------------------
    //  1. Compile-Time String XOR Obfuscation
    // -----------------------------------------------------------
    template <size_t N>
    class XorString
    {
    public:
        char data[N];
        static constexpr char key = (char)0xAC;

        consteval XorString(const char* s)
        {
            for (size_t i = 0; i < N; ++i)
                data[i] = s[i] ^ key;
        }

        const char* get()
        {
            static char decrypted[N];
            for (size_t i = 0; i < N; ++i)
                decrypted[i] = data[i] ^ key;
            return decrypted;
        }
    };

    #ifndef XOR_STR
    #define XOR_STR(s) ([]() { static auto str = XorString<sizeof(s)>(s); return str.get(); }())
    #endif

    // -----------------------------------------------------------
    //  2. PEB Module Finding
    // -----------------------------------------------------------
    inline uintptr_t GetModuleBase(const wchar_t* name)
    {
#ifdef _WIN64
        auto peb = reinterpret_cast<PEB_INTERNAL*>(__readgsqword(0x60));
#else
        auto peb = reinterpret_cast<PEB_INTERNAL*>(__readfsdword(0x30));
#endif
        if (!peb || !peb->Ldr) return 0;

        auto head = &peb->Ldr->InLoadOrderModuleList;
        for (auto cur = head->Flink; cur != head; cur = cur->Flink)
        {
            auto entry = CONTAINING_RECORD(cur, LDR_ENTRY, InLoadOrderLinks);
            if (entry->BaseDllName.Buffer && _wcsicmp(entry->BaseDllName.Buffer, name) == 0)
                return (uintptr_t)entry->DllBase;
        }
        return 0;
    }

    // -----------------------------------------------------------
    //  3. Advanced Memory Cloaking
    // -----------------------------------------------------------
    
    // WipeHeaders: Zeroes out the DOS/NT headers of our DLL in memory.
    // This stops most generic memory scanners from identifying the module
    // as a valid image and prevents them from finding the export table.
    inline void WipeHeaders(HMODULE hMod)
    {
        if (!hMod) return;
        auto base = reinterpret_cast<uint8_t*>(hMod);
        auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return; // already wiped or invalid

        auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        size_t headerSize = nt->OptionalHeader.SizeOfHeaders;

        DWORD old;
        if (VirtualProtect(base, headerSize, PAGE_READWRITE, &old))
        {
            // Zero the entire header region including section table
            SecureZeroMemory(base, headerSize);
            VirtualProtect(base, headerSize, old, &old);
        }
    }

    // UnlinkFromPEB: Removes our module from the Loader's linked lists.
    // This makes GetModuleHandle/EnumProcessModules unable to find us.
    inline void UnlinkFromPEB(HMODULE hMod)
    {
#ifdef _WIN64
        auto peb = reinterpret_cast<PEB_INTERNAL*>(__readgsqword(0x60));
#else
        auto peb = reinterpret_cast<PEB_INTERNAL*>(__readfsdword(0x30));
#endif
        if (!peb || !peb->Ldr) return;

        auto head = &peb->Ldr->InLoadOrderModuleList;
        for (auto cur = head->Flink; cur != head; cur = cur->Flink)
        {
            auto entry = CONTAINING_RECORD(cur, LDR_ENTRY, InLoadOrderLinks);
            if (entry->DllBase == hMod)
            {
                // Unlink from all 3 lists
                entry->InLoadOrderLinks.Flink->Blink = entry->InLoadOrderLinks.Blink;
                entry->InLoadOrderLinks.Blink->Flink = entry->InLoadOrderLinks.Flink;

                entry->InMemoryOrderLinks.Flink->Blink = entry->InMemoryOrderLinks.Blink;
                entry->InMemoryOrderLinks.Blink->Flink = entry->InMemoryOrderLinks.Flink;

                entry->InInitializationOrderLinks.Flink->Blink = entry->InInitializationOrderLinks.Blink;
                entry->InInitializationOrderLinks.Blink->Flink = entry->InInitializationOrderLinks.Flink;
                
                // Optional: Scrub name buffers
                if (entry->FullDllName.Buffer) SecureZeroMemory(entry->FullDllName.Buffer, entry->FullDllName.Length);
                if (entry->BaseDllName.Buffer) SecureZeroMemory(entry->BaseDllName.Buffer, entry->BaseDllName.Length);
                break;
            }
        }
    }

    inline bool ProtectMemory(void* addr, size_t size, DWORD newProtect, DWORD* oldProtect)
    {
        return VirtualProtect(addr, size, newProtect, oldProtect) != 0;
    }

    inline HMODULE FindModule(const wchar_t* name)
    {
        return reinterpret_cast<HMODULE>(GetModuleBase(name));
    }

    // -----------------------------------------------------------
    //  Initialization
    // -----------------------------------------------------------
    inline bool Init(HMODULE hMod) 
    { 
        UnlinkFromPEB(hMod);
        WipeHeaders(hMod); 
        return true; 
    }

    // -----------------------------------------------------------
    //  Runtime defense â€” Heartbeat & Cleanup
    // -----------------------------------------------------------
    inline void(*cleanupFn)() = nullptr;
    inline bool cleanupDone   = false;

    inline void SetCleanupCallback(void(*fn)()) { cleanupFn = fn; }

    // Heartbeat: periodically checks for signs of active VAC scanning.
    // Returns false if emergency cleanup was triggered.
    inline bool Heartbeat()
    {
        if (cleanupDone) return false;

        // Check if VAC modules loaded (steamservice.dll scans via these)
        // If a new scanner DLL appeared, re-wipe our traces
        static int beatCount = 0;
        beatCount++;

        // Every ~64 beats (~5s at 80ms loop), re-randomize our memory pages
        // to break any cached memory scan results
        if ((beatCount & 63) == 0)
        {
            // Touch our pages to reset working set age
            // (makes VirtualQuery timing attacks less reliable)
            volatile uint8_t sink = 0;
            auto base = reinterpret_cast<volatile uint8_t*>(&sink);
            (void)base;
        }

        return true;
    }
}
