#pragma once

// ---------------------------------------------------------------------------
// chams_scene.h — Phase 1 of GeneratePrimitives chams migration.
//
// GOAL (multi-phase):
//   Replace the existing D3D11 DrawIndexedInstanced-based chams in
//   features/chams.h with a scenesystem-layer hook on
//   `CSceneAnimatableObject::GeneratePrimitives`. The scene-layer hook
//   is what professional internals (kauht et al.) use because:
//     - We hold a real scene object, so we can resolve owning entity
//       and pick PER-ENTITY (friend/enemy/local) material — the D3D11
//       hook can't differentiate teams since the GPU only sees raw
//       vertex/index buffers and a debug name.
//     - We're upstream of culling/LOD decisions in the renderer, so
//       paired with the PVS disable in WorldEffects::TryDisablePvs we
//       get unlimited chams render distance for free.
//     - Material override happens via materialsystem2 IMaterial2 (the
//       same path the engine uses), so visuals are pixel-identical to
//       a real model — no ad-hoc pixel shader.
//
// PHASE 1 (this file): land the hook infrastructure.
//   - Sigscan + MinHook on CSceneAnimatableObject::GeneratePrimitives
//     (verified pattern in core/signatures.h, RVA 0x73520 build 14155).
//   - Detour body is a PURE PASSTHROUGH — calls original, no override,
//     no behavior change. We count calls per second so the watermark /
//     log can confirm the hook is alive.
//   - Existing features/chams.h D3D11 path is UNCHANGED. This module
//     runs in parallel as a no-op until phase 2 wires the override.
//
// PHASE 2 (next session, requires scenesystem.dll loaded into IDA):
//   - Map exact arg layout of GeneratePrimitives (we know the prologue
//     but not the parameter semantics — CSceneAnimatableObject* this is
//     RCX, but RDX/R8/R9 need IDA verification).
//   - Resolve scene object -> owning CBaseEntity (likely via a back-ptr
//     stored at a known field offset on CSceneObject).
//   - Use materialsystem2 FindParameter / UpdateParameter (already in
//     verified_features.md as the canonical chams param-swap path) to
//     temporarily push g_flMetalness/g_flRoughness/g_vTintColor on the
//     bound IMaterial2 around the trampoline call, then restore.
//   - Add wallhack pass by recalling the trampoline with depth-test off
//     (mirrors the v1.11.0 SceneSystem_Thread_RenderSceneDrawList
//     comment in repo memory).
//
// PHASE 3: rip out the D3D11 hook in features/chams.h once Phase 2 is
//   pixel-verified equivalent. Keep menu/Config layout identical so the
//   UI doesn't shift.
//
// SAFETY:
//   - All work is __try-guarded. A misfiring sig must NEVER splatter
//     scene memory.
//   - The hook is OPT-IN at runtime via cfg.enabled (default: false).
//     With cfg.enabled = false, the detour does literally nothing
//     except inc the call counter — a regression here is impossible.
// ---------------------------------------------------------------------------

#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <atomic>
#include "../core/memory.h"
#include "../core/signatures.h"
#include "../vendor/minhook/include/MinHook.h"

namespace ChamsScene
{
    // ---- Config ---------------------------------------------------------
    // Default OFF — Phase 1 is observation-only. Phase 2 will enable
    // material override behind this flag.
    struct Config { bool enabled = false; };
    inline Config cfg;

    // ---- State ----------------------------------------------------------
    inline bool          g_setupTried   = false;
    inline bool          g_hookOk       = false;
    inline void*         g_targetFn     = nullptr;
    inline std::atomic<uint64_t> g_callCount{0};   // total trampoline hits
    inline std::atomic<uint32_t> g_callsThisSec{0}; // resets per second

    // ---- Original trampoline -------------------------------------------
    // Signature is conservative — RCX = scene object, the rest are
    // unknown until Phase 2 IDA pass. We pass them through register-
    // perfectly using a 4-arg __fastcall prototype which matches the
    // Win64 ABI for the first 4 integer args. Variadic-ish args beyond
    // 4 land on the stack and the trampoline preserves them verbatim
    // (we never read them).
    using GeneratePrimitivesFn =
        void(__fastcall*)(void* sceneObj, void* a2, void* a3, void* a4);
    inline GeneratePrimitivesFn g_orig = nullptr;

    // ---- Detour ---------------------------------------------------------
    // Phase 1: pure passthrough. We do NOT touch the scene object, do
    // NOT swap materials, do NOT add a second draw pass. Just count
    // and forward. cfg.enabled is reserved for Phase 2.
    inline void __fastcall hkGeneratePrimitives(
        void* sceneObj, void* a2, void* a3, void* a4)
    {
        g_callCount.fetch_add(1, std::memory_order_relaxed);
        g_callsThisSec.fetch_add(1, std::memory_order_relaxed);

        // Phase 2 will branch here on cfg.enabled to push override
        // material vars before the trampoline and pop them after.
        if (g_orig)
            g_orig(sceneObj, a2, a3, a4);
    }

    // ---- Setup / Shutdown ----------------------------------------------
    // Idempotent. Call once after MH_Initialize() and after
    // scenesystem.dll has been loaded by the engine.
    inline bool Setup()
    {
        if (g_setupTried) return g_hookOk;
        g_setupTried = true;

        __try {
            uintptr_t addr = Mem::FindPattern(
                L"scenesystem.dll",
                Signatures::CSceneAnimatableObject_GeneratePrimitives);
            if (!addr) return false;

            // Sanity-bound to scenesystem.dll image so a wild sig hit
            // can never be hooked.
            HMODULE hScene = GetModuleHandleW(L"scenesystem.dll");
            if (!hScene) return false;
            MODULEINFO mi{};
            if (!GetModuleInformation(GetCurrentProcess(), hScene, &mi, sizeof(mi)))
                return false;
            uintptr_t modBase = reinterpret_cast<uintptr_t>(hScene);
            uintptr_t modEnd  = modBase + mi.SizeOfImage;
            if (addr < modBase || addr >= modEnd) return false;

            g_targetFn = reinterpret_cast<void*>(addr);
            MH_STATUS st = MH_CreateHook(
                g_targetFn,
                reinterpret_cast<void*>(&hkGeneratePrimitives),
                reinterpret_cast<void**>(&g_orig));
            if (st != MH_OK && st != MH_ERROR_ALREADY_CREATED) return false;

            st = MH_EnableHook(g_targetFn);
            g_hookOk = (st == MH_OK || st == MH_ERROR_ENABLED);
            return g_hookOk;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline void Shutdown()
    {
        if (g_hookOk && g_targetFn)
        {
            MH_DisableHook(g_targetFn);
            MH_RemoveHook(g_targetFn);
        }
        g_hookOk   = false;
        g_targetFn = nullptr;
        g_orig     = nullptr;
    }

    // ---- Diagnostics ---------------------------------------------------
    // Reset the per-second counter — call from the watchdog/heartbeat
    // tick. Returns the count observed in the previous interval.
    inline uint32_t SampleAndResetCps()
    {
        return g_callsThisSec.exchange(0, std::memory_order_relaxed);
    }
}
