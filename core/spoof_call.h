#pragma once

// ---------------------------------------------------------------
// spoof_call.h — Return address spoofing for Win32 API calls
//
// Prevents stack-walking from linking our code to API calls.
// Uses a "call rax; ret" gadget inside ntdll.dll as trampoline.
//
// Stub layout (24 bytes, generated at runtime on RX page):
//   mov rax, <target>     ; 10 bytes — load real API address
//   jmp [rip+0]           ; 6 bytes  — jump to gadget
//   <gadget address>      ; 8 bytes  — data for the jmp
//
// Call flow:
//   1. Caller → stub: mov rax, API; jmp gadget
//   2. Gadget (in ntdll): call rax → pushes ntdll ret addr
//   3. API executes, sees return address inside ntdll ✓
//   4. API ret → gadget+2 (ret) → original caller
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include "xorstr.h"

namespace SpoofCall
{
    inline uint8_t* gadget   = nullptr;   // "FF D0 C3" in ntdll
    inline uint8_t* stubPage = nullptr;
    inline int       stubOff  = 0;
    inline bool      ready    = false;

    // Find "call rax; ret" (FF D0 C3) in a module's executable sections
    inline uint8_t* FindCallRaxRet(HMODULE mod)
    {
        auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(mod);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
        auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
            reinterpret_cast<uint8_t*>(mod) + dos->e_lfanew);
        auto sec = IMAGE_FIRST_SECTION(nt);

        for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i)
        {
            if (!(sec[i].Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
            uint8_t* base = reinterpret_cast<uint8_t*>(mod) + sec[i].VirtualAddress;
            size_t size = sec[i].Misc.VirtualSize;
            for (size_t j = 0; j + 2 < size; ++j)
            {
                if (base[j] == 0xFF && base[j + 1] == 0xD0 && base[j + 2] == 0xC3)
                    return base + j;
            }
        }
        return nullptr;
    }

    inline bool Init()
    {
        // Find gadget in ntdll (most reliable — always loaded, large .text)
        HMODULE ntdll = GetModuleHandleW(XW(L"ntdll.dll"));
        if (ntdll) gadget = FindCallRaxRet(ntdll);

        // Fallback: try kernel32
        if (!gadget)
        {
            HMODULE k32 = GetModuleHandleW(XW(L"kernel32.dll"));
            if (k32) gadget = FindCallRaxRet(k32);
        }
        if (!gadget) return false;

        // Allocate stub page
        stubPage = reinterpret_cast<uint8_t*>(
            VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (!stubPage) return false;

        ready = true;
        return true;
    }

    // Seal the stub page (call after all MakeStub calls)
    inline void Seal()
    {
        if (stubPage)
        {
            DWORD old;
            VirtualProtect(stubPage, 4096, PAGE_EXECUTE_READ, &old);
        }
    }

    // Build a stub for a specific function. Returns a callable pointer
    // with the same signature as the original.
    template <typename T>
    T MakeStub(void* target)
    {
        if (!stubPage || !gadget || stubOff + 24 > 4096) return nullptr;
        uint8_t* p = stubPage + stubOff;

        // mov rax, <target>  (REX.W + B8)
        p[0] = 0x48; p[1] = 0xB8;
        *reinterpret_cast<void**>(p + 2) = target;

        // jmp [rip+0]
        p[10] = 0xFF; p[11] = 0x25;
        *reinterpret_cast<uint32_t*>(p + 12) = 0;

        // gadget address (data for the jmp)
        *reinterpret_cast<void**>(p + 16) = gadget;

        stubOff += 24;
        return reinterpret_cast<T>(p);
    }

    // ---- Pre-built spoofed API pointers ----

    // SendInput — most critical (aimbot mouse movement)
    using SendInput_t = UINT(WINAPI*)(UINT, LPINPUT, int);
    inline SendInput_t pSendInput = nullptr;

    // SleepEx — hide our thread's stack during sleep
    using SleepEx_t = DWORD(WINAPI*)(DWORD, BOOL);
    inline SleepEx_t pSleepEx = nullptr;

    // Build all spoofed APIs (call after Init, before Seal)
    inline void BuildAPIs()
    {
        HMODULE user32 = GetModuleHandleW(XW(L"user32.dll"));
        if (!user32) user32 = LoadLibraryW(XW(L"user32.dll"));
        if (user32)
        {
            auto real = reinterpret_cast<void*>(GetProcAddress(user32, XS("SendInput")));
            if (real) pSendInput = MakeStub<SendInput_t>(real);
        }

        // SleepEx lives in kernel32/KernelBase
        HMODULE k32 = GetModuleHandleW(XW(L"kernel32.dll"));
        if (k32)
        {
            auto real = reinterpret_cast<void*>(GetProcAddress(k32, XS("SleepEx")));
            if (real) pSleepEx = MakeStub<SleepEx_t>(real);
        }
    }

    // Drop-in SendInput replacement (falls back to dynamic resolve if spoof unavailable)
    inline UINT SpoofedSendInput(UINT nInputs, LPINPUT pInputs, int cbSize)
    {
        if (pSendInput)
            return pSendInput(nInputs, pInputs, cbSize);
        // Dynamic resolve — avoids IAT entry for SendInput
        HMODULE u32 = GetModuleHandleW(XW(L"user32.dll"));
        if (u32) {
            auto fn = reinterpret_cast<SendInput_t>(GetProcAddress(u32, XS("SendInput")));
            if (fn) return fn(nInputs, pInputs, cbSize);
        }
        return 0;
    }

    // Drop-in sleep replacement — return address shows ntdll gadget, not our DLL
    inline void SpoofedSleep(DWORD ms)
    {
        if (ready && pSleepEx)
        {
            pSleepEx(ms, FALSE);
        }
        else
        {
            // Fallback to regular Sleep when spoofing unavailable
            Sleep(ms);
        }
    }

    inline void Shutdown()
    {
        if (stubPage)
        {
            DWORD old;
            VirtualProtect(stubPage, 4096, PAGE_READWRITE, &old);
            SecureZeroMemory(stubPage, 4096);
            VirtualFree(stubPage, 0, MEM_RELEASE);
            stubPage = nullptr;
        }
        gadget    = nullptr;
        pSendInput = nullptr;
        pSleepEx   = nullptr;
        stubOff   = 0;
        ready     = false;
    }
}
