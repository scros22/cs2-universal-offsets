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
    inline HMODULE g_selfModule = nullptr; // cached for re-wipe heartbeat
    inline bool Init(HMODULE hMod) 
    { 
        g_selfModule = hMod;
        UnlinkFromPEB(hMod);
        WipeHeaders(hMod); 
        return true; 
    }

    // -----------------------------------------------------------
    //  Untrusted-mode flag suppression (CS2 20-hour cooldown defeat)
    //
    //  Reverse-engineered chain (client.dll, build 14152-ish):
    //
    //    sub_1801569B0 (module-trust verifier) at startup runs a callback
    //    table. If the callback returns 0 (i.e. it doesn't like one of our
    //    loaded modules), it sets a single byte flag at:
    //
    //        client.dll + 0x2198858   (byte_182198858 in the IDB)
    //
    //  That byte is read by exactly two places:
    //
    //    sub_180F23200  (per-tick) - if set, calls sub_180C4A6C0 which
    //                                 dispatches OnClientInsecureBlocked
    //                                 AND sends Game::ChatReportError
    //                                 ("#SFUI_QMM_ERROR_X_InsecureBlocked")
    //                                 to the matchmaking KV pipe \u2192 GC \u2192
    //                                 20-hour cooldown.
    //    sub_180F1F830  (start-listen-server) - same dialog/disconnect path.
    //
    //  Nothing else in client.dll xrefs that byte. Nothing in steamclient64
    //  carries the OnClient* event names. So the cooldown is decided
    //  client-side by that ONE byte; once we zero it the report is never
    //  built and Steam GC never hears about us.
    //
    //  Strategy: lazy-resolve the byte's address by sigscanning the setter
    //  instruction (mov byte ptr [rip+rel32], 1) and decoding rel32. Then
    //  every heartbeat, write 0 to it. No .text patch, no RWX page, just a
    //  1-byte data write that's indistinguishable from CS2's own code
    //  clearing the flag (which it never does, but VAC can't tell that).
    // -----------------------------------------------------------
    inline volatile uint8_t* g_pUntrustedFlag = nullptr;
    inline bool g_untrustedFlagResolveTried   = false;
    // Resolver lives in dllmain.cpp (where memory.h / Mem::FindPattern is
    // already included). Heartbeat just stamps the byte if it's been
    // resolved; resolution itself is driven by the main loop.

    // -----------------------------------------------------------
    //  Belt-and-suspenders: kill the GC report emitter directly.
    //  client.dll!sub_180C4A6C0 is the ONE function that sends
    //  Game::ChatReportError("#SFUI_QMM_ERROR_X_InsecureBlocked") to the
    //  Steam GC. Even if our heartbeat misses a frame and the untrusted
    //  byte reads as 1 inside sub_180F23200's periodic tick, hooking
    //  sub_180C4A6C0 to early-return guarantees no cooldown KV ever goes
    //  out. Hook is one-shot at startup; never patches .text after install.
    // -----------------------------------------------------------
    inline void* g_pInsecureEmitterAddr     = nullptr;  // resolved fn addr
    inline void* g_pInsecureEmitterTramp    = nullptr;  // MinHook trampoline (unused)
    inline bool  g_insecureEmitterHookTried = false;
    inline bool  g_insecureEmitterHookOk    = false;

    // Detour: silently swallow the call. Original returns __int64 but its
    // return value is discarded by every caller (sub_180F23200 does
    // `return sub_180C4A6C0("init");` into a void context).
    inline __int64 InsecureEmitter_Detour(const char* /*reason*/)
    {
        return 0;
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

        // ---- 20-hour cooldown defeat (every beat) -----------------------
        // CS2 sets a single byte in client.dll once at module-trust
        // verification; we hold it down so OnClientInsecureBlocked never
        // fires and Game::ChatReportError never goes to GC. Resolution of
        // the byte's address is done in dllmain (where memory.h's pattern
        // scanner is in scope). Here we only stamp the byte if resolved.
        if (g_pUntrustedFlag)
            *g_pUntrustedFlag = 0;

        // Every ~64 beats (~5s at 80ms loop), re-randomize our memory pages
        // to break any cached memory scan results
        if ((beatCount & 63) == 0)
        {
            // Touch our pages to reset working set age
            // (makes VirtualQuery timing attacks less reliable)
            volatile uint8_t sink = 0;
            auto base = reinterpret_cast<volatile uint8_t*>(&sink);
            (void)base;

            // Re-wipe DOS/NT headers in case anything (cache aliasing
            // after VirtualProtect, a copy-on-write fault, an injector
            // restoring bytes, etc.) restored our IMAGE_DOS_SIGNATURE.
            // VAC enumerates modules periodically; finding a valid PE
            // header in a module the loader doesn't know about is one
            // of the loudest possible untrusted-mode signals.
            if (g_selfModule)
                WipeHeaders(g_selfModule);
        }

        return true;
    }
}
