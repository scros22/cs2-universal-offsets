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

    inline bool Init()
    {
        clientBase  = Mem::GetModBase(L"client.dll");
        engine2Base = Mem::GetModBase(L"engine2.dll");
        return clientBase != 0;
    }

    // ---------------------------------------------------------------
    // Entity system primitives
    // ---------------------------------------------------------------
    constexpr uintptr_t kEntityStride = 0x70; // sizeof(CEntityIdentity)

    inline uintptr_t GetLocalPawn()
    {
        return Mem::Read<uintptr_t>(clientBase + Offsets::Global::dwLocalPlayerPawn);
    }

    inline uintptr_t GetLocalController()
    {
        return Mem::Read<uintptr_t>(clientBase + Offsets::Global::dwLocalPlayerController);
    }

    inline uintptr_t GetEntityList()
    {
        return Mem::Read<uintptr_t>(clientBase + Offsets::Global::dwEntityList);
    }

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
        uintptr_t gv = Mem::Read<uintptr_t>(clientBase + Offsets::Global::dwGlobalVars);
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
        uintptr_t arr = Mem::Read<uintptr_t>(
            node + Offsets::m_modelState + Offsets::m_BoneArray);
        if (!arr) return {};

        struct BoneData { float px, py, pz, pad, qx, qy, qz, qw; };
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
        return Mem::Read<Math::ViewMatrix>(
            clientBase + Offsets::Global::dwViewMatrix);
    }

    inline bool WorldToScreen(const float* world, float& sx, float& sy,
                              float scrW, float scrH)
    {
        return Math::WorldToScreen(GetViewMatrix(), world, sx, sy, scrW, scrH);
    }
}
