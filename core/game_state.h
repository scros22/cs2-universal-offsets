#pragma once

// ---------------------------------------------------------------
// Game state management — module bases, entity helpers, bone data.
// Central point for all game-memory reads that features depend on.
// ---------------------------------------------------------------

#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <vector>
#include <string>
#include "memory.h"
#include "math.h"
#include "sdk_offsets.h"

namespace GameState
{
    inline uintptr_t clientBase  = 0;
    inline uintptr_t engine2Base = 0;

    // --- Original dormancy state (before chams anti-dormancy overwrite) ---
    // Chams stores the REAL server dormancy state here each frame before forcing
    // m_bDormant=false for rendering. Aimbot reads this to avoid targeting
    // entities the server isn't actually transmitting data for.
    // Index = entity list slot (1-64). true = server considers dormant (stale data).
    inline bool originalDormant[65] = {};

    // Local player controller entity index (1-64), cached per frame.
    // Used by aimbot to check m_bSpottedByMask for OUR specific bit.
    inline int localPlayerIndex = -1;

    // ---------------------------------------------------------------
    // Live-resolved global RVAs.
    // The dumper occasionally produces stale RVAs after game updates
    // because its anchor patterns drift. To keep the internal working
    // across mid-build patches without re-dumping, we pattern-scan the
    // canonical write/read sites in client.dll at Init() and prefer
    // those values over the constants in sdk/offsets.hpp.
    // ---------------------------------------------------------------
    inline std::ptrdiff_t resolved_dwEntityList            = 0;
    inline std::ptrdiff_t resolved_dwLocalPlayerController = 0;
    inline std::ptrdiff_t resolved_dwLocalPlayerPawn       = 0;
    inline std::ptrdiff_t resolved_dwViewMatrix            = 0;
    inline std::ptrdiff_t resolved_dwGlobalVars            = 0;
    inline std::ptrdiff_t resolved_dwGameRules             = 0;
    inline std::ptrdiff_t resolved_dwPlantedC4             = 0;
    inline std::ptrdiff_t resolved_dwViewAngles            = 0;
    inline std::ptrdiff_t resolved_dwCSGOInput             = 0;

    namespace detail
    {
        // Resolve a 7-byte RIP-relative instruction (e.g. 48 89 0D xx xx xx xx)
        //  -> returns the RVA of the global it references in client.dll.
        inline std::ptrdiff_t RipRel32(uintptr_t insAddr, int dispOff, int insLen)
        {
            if (!insAddr || !clientBase) return 0;
            int32_t rel = *(int32_t*)(insAddr + dispOff);
            uintptr_t target = insAddr + insLen + rel;
            return (std::ptrdiff_t)(target - clientBase);
        }
    }

    inline void ResolveGlobals()
    {
        if (!clientBase) return;

        // dwEntityList:  48 89 0D ?? ?? ?? ?? E9 ?? ?? ?? ?? CC
        if (auto p = Mem::FindPattern(L"client.dll", "48 89 0D ? ? ? ? E9 ? ? ? ? CC"))
            resolved_dwEntityList = detail::RipRel32(p, 3, 7);

        // dwLocalPlayerController:  48 8B 05 ?? ?? ?? ?? 41 89 BE
        if (auto p = Mem::FindPattern(L"client.dll", "48 8B 05 ? ? ? ? 41 89 BE"))
            resolved_dwLocalPlayerController = detail::RipRel32(p, 3, 7);

        // dwPrediction:  48 8D 05 ?? ?? ?? ?? C3 (8x CC) 40 53 56 41 54
        // Then dwLocalPlayerPawn = dwPrediction + disp32 from `4C 39 B6 ?? ?? ?? ?? 74 ?? 44 88 BE`
        std::ptrdiff_t predRva = 0;
        if (auto p = Mem::FindPattern(L"client.dll",
            "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 40 53 56 41 54"))
            predRva = detail::RipRel32(p, 3, 7);
        // Fallback: dumper's dwPrediction RVA (may also be stale, but worth trying).
        if (!predRva) predRva = (std::ptrdiff_t)Offsets::Global::dwPrediction;
        if (auto p = Mem::FindPattern(L"client.dll", "4C 39 B6 ? ? ? ? 74 ? 44 88 BE"))
        {
            uint32_t disp = *(uint32_t*)(p + 3);
            if (predRva) resolved_dwLocalPlayerPawn = predRva + (std::ptrdiff_t)disp;
        }
        // Last-ditch fallback: if both scans miss, dwLocalPlayerPawn ≈ dwPrediction + 0xF0
        // (relationship has been stable across 2024-2026 builds).
        if (!resolved_dwLocalPlayerPawn && predRva)
            resolved_dwLocalPlayerPawn = predRva + 0xF0;

        // dwViewMatrix:  48 8D 0D ?? ?? ?? ?? 48 C1 E0 06
        if (auto p = Mem::FindPattern(L"client.dll", "48 8D 0D ? ? ? ? 48 C1 E0 06"))
            resolved_dwViewMatrix = detail::RipRel32(p, 3, 7);

        // dwGlobalVars:  48 89 15 ?? ?? ?? ?? 48 89 42
        if (auto p = Mem::FindPattern(L"client.dll", "48 89 15 ? ? ? ? 48 89 42"))
            resolved_dwGlobalVars = detail::RipRel32(p, 3, 7);

        // dwGameRules:  48 8D 05 ?? ?? ?? ?? 48 89 06 48 8D 4E 44
        if (auto p = Mem::FindPattern(L"client.dll", "48 8D 05 ? ? ? ? 48 89 06 48 8D 4E 44"))
            resolved_dwGameRules = detail::RipRel32(p, 3, 7);

        // dwPlantedC4:  48 8B 15 ?? ?? ?? ?? 41 FF C0 48 8D 4C 24
        if (auto p = Mem::FindPattern(L"client.dll", "48 8B 15 ? ? ? ? 41 FF C0 48 8D 4C 24"))
            resolved_dwPlantedC4 = detail::RipRel32(p, 3, 7);

        // dwCSGOInput:  48 89 05 ?? ?? ?? ?? 0F 57 C0 0F 11 05 ?? ?? ?? ??
        // The second 7-byte RIP-rel (0F 11 05 …) points to dwViewAngles.
        if (auto p = Mem::FindPattern(L"client.dll",
            "48 89 05 ? ? ? ? 0F 57 C0 0F 11 05 ? ? ? ?"))
        {
            resolved_dwCSGOInput  = detail::RipRel32(p,      3, 7);
            // The 0F 11 05 lives at +10; its disp32 starts at +13 and the
            // instruction is 7 bytes long (so insLen=7 from the +10 anchor).
            resolved_dwViewAngles = detail::RipRel32(p + 10, 3, 7);
        }
    }

    inline bool Init()
    {
        clientBase  = Mem::GetModBase(L"client.dll");
        engine2Base = Mem::GetModBase(L"engine2.dll");
        ResolveGlobals();
        return clientBase != 0;
    }

    // ---------------------------------------------------------------
    // Entity system primitives
    // ---------------------------------------------------------------
    constexpr uintptr_t kEntityStride = 0x70; // sizeof(CEntityIdentity)

    inline uintptr_t GetLocalPawn()
    {
        std::ptrdiff_t off = resolved_dwLocalPlayerPawn ? resolved_dwLocalPlayerPawn
                                                        : Offsets::Global::dwLocalPlayerPawn;
        return Mem::Read<uintptr_t>(clientBase + off);
    }

    inline uintptr_t GetLocalController()
    {
        std::ptrdiff_t off = resolved_dwLocalPlayerController ? resolved_dwLocalPlayerController
                                                              : Offsets::Global::dwLocalPlayerController;
        return Mem::Read<uintptr_t>(clientBase + off);
    }

    inline uintptr_t GetEntityList()
    {
        std::ptrdiff_t off = resolved_dwEntityList ? resolved_dwEntityList
                                                   : Offsets::Global::dwEntityList;
        return Mem::Read<uintptr_t>(clientBase + off);
    }

    // ---------------------------------------------------------------
    // RVA accessors that prefer the live-resolved value, falling back
    // to the dumper constant from sdk/offsets.hpp.
    // Use these instead of `Offsets::Global::dwXxx` for the runtime
    // self-healing offset to take effect.
    // ---------------------------------------------------------------
    inline std::ptrdiff_t RVA_dwViewAngles()
    { return resolved_dwViewAngles ? resolved_dwViewAngles : Offsets::Global::dwViewAngles; }
    inline std::ptrdiff_t RVA_dwGlobalVars()
    { return resolved_dwGlobalVars ? resolved_dwGlobalVars : Offsets::Global::dwGlobalVars; }
    inline std::ptrdiff_t RVA_dwGameRules()
    { return resolved_dwGameRules ? resolved_dwGameRules : Offsets::Global::dwGameRules; }
    inline std::ptrdiff_t RVA_dwPlantedC4()
    { return resolved_dwPlantedC4 ? resolved_dwPlantedC4 : Offsets::Global::dwPlantedC4; }
    inline std::ptrdiff_t RVA_dwCSGOInput()
    { return resolved_dwCSGOInput ? resolved_dwCSGOInput : Offsets::Global::dwCSGOInput; }

    // Cache local player's controller entity index (1-64).
    // Call once per frame. Scans entity list for matching controller pointer.
    inline void UpdateLocalPlayerIndex()
    {
        uintptr_t localCtrl = GetLocalController();
        if (!localCtrl) { localPlayerIndex = -1; return; }
        uintptr_t entList = GetEntityList();
        if (!entList) { localPlayerIndex = -1; return; }

        for (int i = 1; i <= 64; ++i)
        {
            __try {
                uintptr_t chunk = Mem::Read<uintptr_t>(entList + 0x8 * (i >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ctrl = Mem::Read<uintptr_t>(chunk + kEntityStride * (i & 0x1FF));
                if (ctrl == localCtrl) { localPlayerIndex = i; return; }
            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }
        localPlayerIndex = -1;
    }

    // Current game time from CGlobalVarsBase
    inline float GetGameTime()
    {
        uintptr_t gv = Mem::Read<uintptr_t>(clientBase + GameState::RVA_dwGlobalVars());
        if (!gv) return 0.f;
        // Try 0x2C first (m_flCurTime in most CS2 builds)
        float t = Mem::Read<float>(gv + 0x2C);
        if (t > 0.1f && t < 100000.f) return t;
        // Fallback to 0x34 if 0x2C doesn't look valid
        t = Mem::Read<float>(gv + 0x34);
        if (t > 0.1f && t < 100000.f) return t;
        return 0.f;
    }

    // Resolves a CHandle to an entity pointer via the global entity list.
    // Entity list is a chunked array with 512 entries per chunk.
    // Each entry is kEntityStride (0x70) bytes, entity pointer at offset 0x00.
    inline uintptr_t ResolveHandle(uint32_t handle)
    {
        if (!handle || handle == 0xFFFFFFFF) return 0;
        uintptr_t list = GetEntityList();
        if (!list) return 0;

        uint32_t idx = handle & 0x7FFF;

        uintptr_t chunkBase = Mem::Read<uintptr_t>(list + 8 * (idx >> 9) + 0x10);
        if (!chunkBase) return 0;

        return Mem::Read<uintptr_t>(chunkBase + kEntityStride * (idx & 0x1FF));
    }

    // Gets the designer name (e.g. "cs_player_controller") directly from
    // the CEntityIdentity's m_designerName field (CUtlSymbolLarge at +0x20).
    inline std::string GetDesignerName(uintptr_t ent)
    {
        if (!ent) return "";
        // CEntityInstance.m_pEntity = 0x10 → CEntityIdentity*
        uintptr_t identity = Mem::Read<uintptr_t>(ent + 0x10);
        if (!identity) return "";
        // CEntityIdentity.m_designerName = 0x20 → CUtlSymbolLarge (pointer to string)
        uintptr_t namePtr = Mem::Read<uintptr_t>(identity + 0x20);
        if (!namePtr) return "";
        struct Buf { char d[64]; };
        Buf b = Mem::Read<Buf>(namePtr);
        b.d[63] = '\0';
        return std::string(b.d);
    }

    // Alternative: read designer name directly from an identity address (no entity needed)
    inline std::string GetDesignerNameFromIdentity(uintptr_t identAddr)
    {
        if (!identAddr) return "";
        uintptr_t namePtr = Mem::Read<uintptr_t>(identAddr + 0x20);
        if (!namePtr) return "";
        struct Buf { char d[64]; };
        Buf b = Mem::Read<Buf>(namePtr);
        b.d[63] = '\0';
        return std::string(b.d);
    }

    // ---------------------------------------------------------------
    // Weapon enumeration
    // ---------------------------------------------------------------
    inline std::vector<uintptr_t> GetWeapons(uintptr_t pawn)
    {
        std::vector<uintptr_t> out;
        if (!pawn) return out;

        uintptr_t svc = Mem::Read<uintptr_t>(pawn + Offsets::m_pWeaponServices);
        if (!svc) return out;

        // C_NetworkUtlVectorBase layout: count (uint32) at +0x0, data ptr at +0x8
        uint32_t  count = Mem::Read<uint32_t>(svc + Offsets::m_hMyWeapons);
        uintptr_t data  = Mem::Read<uintptr_t>(svc + Offsets::m_hMyWeapons + 0x8);
        if (!count || count > 64 || !data) return out;

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t h = Mem::Read<uint32_t>(data + i * sizeof(uint32_t));
            uintptr_t w = ResolveHandle(h);
            if (w) out.push_back(w);
        }
        return out;
    }

    // ---------------------------------------------------------------
    // Spatial helpers
    // ---------------------------------------------------------------
    inline Math::Vec3 GetEntityOrigin(uintptr_t pawn)
    {
        uintptr_t node = Mem::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
        if (!node) return {};
        return Mem::Read<Math::Vec3>(node + Offsets::m_vecAbsOrigin);
    }

    inline Math::Vec3 GetBonePos(uintptr_t pawn, int bone)
    {
        if (bone < 0 || bone > 128) return {}; // sanity clamp
        uintptr_t node = Mem::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
        if (!node) return {};

        // Bone-array offset inside CModelState shifts between CS2 builds.
        // Auto-detect once by probing a short list of candidates and picking
        // the one whose bone[6] (head) sits within ~120 units of the entity
        // origin. Sticky after first hit.
        static int s_boneArrayOffset = 0;
        struct BoneData { float px, py, pz, pad, qx, qy, qz, qw; };

        if (!s_boneArrayOffset)
        {
            Math::Vec3 origin = Mem::Read<Math::Vec3>(node + Offsets::m_vecAbsOrigin);
            if (origin.IsZero()) return {};
            static const int kCandidates[] = {
                0x80, 0x28, 0x40, 0x60, 0xA0, 0xC0, 0xE0, 0x100, 0x140, 0x180, 0x1C0
            };
            for (int cand : kCandidates)
            {
                uintptr_t arrTry = Mem::Read<uintptr_t>(
                    node + Offsets::m_modelState + cand);
                if (!arrTry || arrTry < 0x10000) continue;
                BoneData probe = Mem::Read<BoneData>(arrTry + 6 * sizeof(BoneData));
                if (isnan(probe.px) || isinf(probe.px)) continue;
                float dx = probe.px - origin.x;
                float dy = probe.py - origin.y;
                float dz = probe.pz - origin.z;
                float d2 = dx*dx + dy*dy + dz*dz;
                // head should be within ~120 units of entity origin
                if (d2 > 1.f && d2 < 14400.f)
                {
                    s_boneArrayOffset = cand;
                    break;
                }
            }
            if (!s_boneArrayOffset) return {};
        }

        uintptr_t arr = Mem::Read<uintptr_t>(
            node + Offsets::m_modelState + s_boneArrayOffset);
        if (!arr) return {};

        BoneData b = Mem::Read<BoneData>(arr + bone * sizeof(BoneData));
        // Reject NaN/Inf results (corrupted/uninitialized bone data)
        if (isnan(b.px) || isnan(b.py) || isnan(b.pz)) return {};
        if (isinf(b.px) || isinf(b.py) || isinf(b.pz)) return {};
        return { b.px, b.py, b.pz };
    }

    // ---------------------------------------------------------------
    // View matrix -> world-to-screen
    // ---------------------------------------------------------------
    inline Math::ViewMatrix GetViewMatrix()
    {
        std::ptrdiff_t off = resolved_dwViewMatrix ? resolved_dwViewMatrix
                                                   : Offsets::Global::dwViewMatrix;
        return Mem::Read<Math::ViewMatrix>(clientBase + off);
    }

    inline bool WorldToScreen(const float* world, float& sx, float& sy,
                              float scrW, float scrH)
    {
        return Math::WorldToScreen(GetViewMatrix(), world, sx, sy, scrW, scrH);
    }
}
