#pragma once

// ---------------------------------------------------------------
// CParticleSystemMgr accessor (particles.dll)
//
// Resolves the global g_pParticleMgr pointer via the
// Signatures::GetParticleManager 1-line accessor:
//
//     mov rax, [rip+rel32_to_g_pParticleMgr]   ; 7 bytes
//     ret                                       ; 1 byte
//
// We don't call the function — we read the rel32 it dereferences,
// add it to RIP, then dereference once to get the live manager
// pointer. Lazy + cached per process lifetime.
//
// Wiring CreateParticleEffect itself requires the manager's vtable
// slot for CreateNewParticleSystem — that's a separate verification
// step per game build (the slot index has shifted historically).
// This header gives downstream features the resolved pointer; they
// can then call vtable[N] once the slot is confirmed.
// ---------------------------------------------------------------

#include <cstdint>
#include <Windows.h>
#include "memory.h"
#include "signatures.h"

namespace ParticleManager
{
    inline void*  g_mgr      = nullptr;
    inline bool   g_resolved = false;
    inline bool   g_failed   = false;

    // Lazy resolve. Safe to call from any thread; the worst case on
    // race is two parallel resolves writing the same pointer.
    inline void* Get()
    {
        if (g_resolved) return g_mgr;
        if (g_failed)   return nullptr;

        __try {
            uintptr_t fn = Mem::FindPattern(L"particles.dll",
                                            Signatures::GetParticleManager);
            if (!fn) { g_failed = true; return nullptr; }

            // mov rax, [rip+rel32]  →  bytes: 48 8B 05 [rel32]
            int32_t rel  = *reinterpret_cast<int32_t*>(fn + 3);
            uintptr_t slot = (fn + 7) + static_cast<intptr_t>(rel);

            // Sanity-bound: slot must lie inside particles.dll image.
            HMODULE hParticles = GetModuleHandleW(L"particles.dll");
            if (!hParticles) { g_failed = true; return nullptr; }
            MODULEINFO mi{};
            if (!GetModuleInformation(GetCurrentProcess(), hParticles,
                                      &mi, sizeof(mi)))
            {
                g_failed = true;
                return nullptr;
            }
            uintptr_t modBase = reinterpret_cast<uintptr_t>(hParticles);
            uintptr_t modEnd  = modBase + mi.SizeOfImage;
            if (slot < modBase || slot >= modEnd) { g_failed = true; return nullptr; }

            void* mgr = *reinterpret_cast<void**>(slot);
            // The manager is constructed during particles.dll init.
            // If we resolve before that completes, retry next call
            // rather than caching a null.
            if (!mgr) return nullptr;

            g_mgr = mgr;
            g_resolved = true;
            return g_mgr;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            g_failed = true;
            return nullptr;
        }
    }
}
