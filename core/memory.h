#pragma once

// ---------------------------------------------------------------
// Memory access layer — internal DLL, direct pointer dereference.
// Wrapped with SEH for safety, plus indirect R/W variants that
// break byte-level write patterns commonly fingerprinted by VAC.
// ---------------------------------------------------------------

#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include "stealth.h"

namespace Mem
{
    // Direct read — plain dereference behind SEH fence
    template <typename T>
    inline T Read(uintptr_t addr)
    {
        if (!addr) return T{};
        __try { return *reinterpret_cast<T*>(addr); }
        __except (EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    }

    // Direct write
    template <typename T>
    inline void Write(uintptr_t addr, const T& val)
    {
        if (!addr) return;
        __try { *reinterpret_cast<T*>(addr) = val; }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Indirect (stealth) R/W — byte-by-byte through volatile pointer.
    // This breaks the direct MOV pattern that signature scanners
    // use for detecting memory writes to game structs such as
    // dwViewAngles and frame-history buffers.
    // ---------------------------------------------------------------
    template <typename T>
    inline T IndirectRead(uintptr_t addr)
    {
        if (!addr) return T{};
        __try
        {
            T out;
            volatile uint8_t* src = reinterpret_cast<volatile uint8_t*>(addr);
            uint8_t tmp[sizeof(T)];
            for (size_t i = 0; i < sizeof(T); ++i) tmp[i] = src[i];
            memcpy(&out, tmp, sizeof(T));
            return out;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    }

    template <typename T>
    inline void IndirectWrite(uintptr_t addr, const T& val)
    {
        if (!addr) return;
        __try
        {
            uint8_t tmp[sizeof(T)];
            memcpy(tmp, &val, sizeof(T));
            volatile uint8_t* dst = reinterpret_cast<volatile uint8_t*>(addr);
            for (size_t i = 0; i < sizeof(T); ++i) dst[i] = tmp[i];
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Float-specific indirect helpers (used by aim subsystem)
    inline float IndirectReadFloat(float* ptr)
    {
        if (!ptr) return 0.f;
        __try
        {
            volatile uint8_t* s = reinterpret_cast<volatile uint8_t*>(ptr);
            uint8_t buf[sizeof(float)];
            for (size_t i = 0; i < sizeof(float); ++i) buf[i] = s[i];
            float r;
            memcpy(&r, buf, sizeof(float));
            return r;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { return 0.f; }
    }

    inline void IndirectWriteFloat(float* ptr, float val)
    {
        if (!ptr) return;
        __try
        {
            uint8_t buf[sizeof(float)];
            memcpy(buf, &val, sizeof(float));
            volatile uint8_t* d = reinterpret_cast<volatile uint8_t*>(ptr);
            for (size_t i = 0; i < sizeof(float); ++i) d[i] = buf[i];
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Smart write — only writes if the value actually changed.
    // Reduces memory write frequency for anti-detection.
    // ---------------------------------------------------------------
    template <typename T>
    inline void SmartWrite(uintptr_t addr, const T& val)
    {
        if (!addr) return;
        __try {
            T current = *reinterpret_cast<T*>(addr);
            if (memcmp(&current, &val, sizeof(T)) != 0)
                *reinterpret_cast<T*>(addr) = val;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Module helpers
    // ---------------------------------------------------------------
    inline uintptr_t GetModBase(const wchar_t* name)
    {
        return reinterpret_cast<uintptr_t>(Stealth::FindModule(name));
    }

    // ---------------------------------------------------------------
    // Pattern scanner — IDA-style "48 8B C4 ? ?" with ? wildcards.
    // Two implementations: one takes module name, one takes base+size.
    // ---------------------------------------------------------------
    inline uintptr_t PatternScan(uintptr_t base, size_t size, const char* sig)
    {
        auto parse = [](const char* pat) -> std::vector<std::pair<uint8_t, bool>>
        {
            std::vector<std::pair<uint8_t, bool>> bytes;
            const char* cur = pat;
            const char* end = pat + strlen(pat);
            while (cur < end)
            {
                if (*cur == '?')
                {
                    ++cur;
                    if (*cur == '?') ++cur;
                    bytes.emplace_back(0, true);
                }
                else
                {
                    bytes.emplace_back(
                        static_cast<uint8_t>(strtoul(cur, const_cast<char**>(&cur), 16)), false);
                }
                while (*cur == ' ') ++cur;
            }
            return bytes;
        };

        auto pat = parse(sig);
        size_t patLen = pat.size();
        uint8_t* mem = reinterpret_cast<uint8_t*>(base);

        for (size_t i = 0; i + patLen <= size; ++i)
        {
            bool match = true;
            for (size_t j = 0; j < patLen; ++j)
            {
                if (!pat[j].second && mem[i + j] != pat[j].first)
                {
                    match = false;
                    break;
                }
            }
            if (match) return base + i;
        }
        return 0;
    }

    inline uintptr_t FindPattern(const wchar_t* modName, const char* sig)
    {
        uintptr_t base = GetModBase(modName);
        if (!base) return 0;

        auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto* nt  = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
        size_t imgSize = nt->OptionalHeader.SizeOfImage;

        return PatternScan(base, imgSize, sig);
    }

    inline uintptr_t FindPatternInModule(HMODULE hMod, const char* sig)
    {
        if (!hMod) return 0;
        uintptr_t base = reinterpret_cast<uintptr_t>(hMod);
        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
        return PatternScan(base, mi.SizeOfImage, sig);
    }

    // Overload for uintptr_t base
    inline uintptr_t FindPatternInModule(uintptr_t base, const char* sig)
    {
        if (!base) return 0;
        HMODULE hMod = reinterpret_cast<HMODULE>(base);
        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
        return PatternScan(base, mi.SizeOfImage, sig);
    }

    // Mask-based pattern scan (for patterns with wildcards)
    inline uintptr_t FindPatternInModule(HMODULE hMod, const char* pattern, const char* mask)
    {
        if (!hMod || !pattern || !mask) return 0;
        
        uintptr_t base = reinterpret_cast<uintptr_t>(hMod);
        MODULEINFO mi{};
        GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
        
        size_t patternLen = strlen(mask);
        uint8_t* scanStart = reinterpret_cast<uint8_t*>(base);
        uint8_t* scanEnd = scanStart + mi.SizeOfImage - patternLen;
        
        for (uint8_t* current = scanStart; current < scanEnd; ++current)
        {
            bool found = true;
            for (size_t i = 0; i < patternLen; ++i)
            {
                if (mask[i] != '?' && current[i] != static_cast<uint8_t>(pattern[i]))
                {
                    found = false;
                    break;
                }
            }
            if (found) return reinterpret_cast<uintptr_t>(current);
        }
        return 0;
    }

    // Small helper to patch bytes with page protection toggle
    inline void PatchBytes(uintptr_t addr, const uint8_t* code, size_t len)
    {
        DWORD old;
        Stealth::ProtectMemory(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &old);
        memcpy(reinterpret_cast<void*>(addr), code, len);
        Stealth::ProtectMemory(reinterpret_cast<void*>(addr), len, old, nullptr);
    }
}
