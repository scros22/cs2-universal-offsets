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

        // dwCSGOInput / dwViewAngles:
        // The historical pattern `48 89 05 ? ? ? ? 0F 57 C0 0F 11 05 …`
        // matched the CCSGOInput constructor, but in 14153+ that constructor
        // zero-initializes an internal field that is NOT the live view-angles
        // global. Using its address gave a write target the renderer never
        // reads, breaking aim. Until a reliable pattern is verified for this
        // build, prefer the dumper constants from sdk/offsets.hpp.
        // (Leaving resolved_dwCSGOInput / resolved_dwViewAngles at 0 means
        // RVA_dwViewAngles()/RVA_dwCSGOInput() fall back to the SDK values.)
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
    // Re-export the EntitySys stride so callers using the old name keep
    // compiling. Prefer Offsets::EntitySys::kChunkEntryStride in new code.
    constexpr uintptr_t kEntityStride = Offsets::EntitySys::kChunkEntryStride;

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

    // Look up an entity slot by integer index (1..N). Returns 0 if the
    // slot is empty or the entity list isn't reachable. Centralizes the
    // chunked entity-list lookup so feature code never re-derives it.
    inline uintptr_t GetEntityByIndex(int idx)
    {
        if (idx < 0) return 0;
        uintptr_t list = GetEntityList();
        if (!list) return 0;
        uint32_t  i = (uint32_t)idx & Offsets::EntitySys::kHandleIndexMask;
        uintptr_t chunk = Mem::Read<uintptr_t>(
            list + Offsets::EntitySys::kChunkArrayBase
                 + Offsets::EntitySys::kChunkPtrStride * (i >> 9));
        if (!chunk) return 0;
        return Mem::Read<uintptr_t>(
            chunk + Offsets::EntitySys::kChunkEntryStride * (i & Offsets::EntitySys::kSlotIndexMask));
    }

    // Cache local player's controller entity index (1-64).
    // Call once per frame. Scans entity list for matching controller pointer.
    inline void UpdateLocalPlayerIndex()
    {
        uintptr_t localCtrl = GetLocalController();
        if (!localCtrl) { localPlayerIndex = -1; return; }

        for (int i = 1; i <= 64; ++i)
        {
            __try {
                if (GetEntityByIndex(i) == localCtrl) { localPlayerIndex = i; return; }
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
    // Entity list is a chunked array with 512 entries per chunk; each
    // entry is kChunkEntryStride bytes, entity pointer at offset 0.
    inline uintptr_t ResolveHandle(uint32_t handle)
    {
        if (!handle || handle == 0xFFFFFFFF) return 0;
        return GetEntityByIndex((int)(handle & Offsets::EntitySys::kHandleIndexMask));
    }

    // Gets the designer name (e.g. "cs_player_controller") directly from
    // the CEntityIdentity's m_designerName field.
    inline std::string GetDesignerName(uintptr_t ent)
    {
        if (!ent) return "";
        uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
        if (!identity) return "";
        uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
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
        uintptr_t namePtr = Mem::Read<uintptr_t>(identAddr + Offsets::EntitySys::kIdentityDesignerName);
        if (!namePtr) return "";
        struct Buf { char d[64]; };
        Buf b = Mem::Read<Buf>(namePtr);
        b.d[63] = '\0';
        return std::string(b.d);
    }

    // ---------------------------------------------------------------
    // Resolve the local player's currently-equipped weapon entity.
    // Build 14152+ removed the schema field m_pClippingWeapon, so the
    // skinchanger / knife code must derive the active weapon through the
    // weapon-services chain instead:
    //   pawn -> m_pWeaponServices -> m_hActiveWeapon -> EntityList
    // Returns 0 if anything along the chain is null/invalid.
    // ---------------------------------------------------------------
    inline uintptr_t GetActiveWeapon(uintptr_t pawn)
    {
        if (!pawn) return 0;
        __try {
            uintptr_t svc = Mem::Read<uintptr_t>(pawn + Offsets::m_pWeaponServices);
            if (!svc) return 0;
            uint32_t h = Mem::Read<uint32_t>(svc + Offsets::m_hActiveWeapon);
            return ResolveHandle(h);
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
    }

    // ---------------------------------------------------------------
    // Anti-detection: observer awareness
    //
    // Reverse-engineered from client.dll @ build 14152 — silent aim and
    // angle-rewriting features are overwhelmingly reported by SPECTATORS,
    // not the target being shot. (Spectators see the player's *real*
    // view angle stream and the *server's* shoot-angle stream, so any
    // disagreement is glaring on their POV.) Counting the number of
    // alive players currently observing our local controller lets the
    // aimbot / anti-aim soften behavior when anyone is watching.
    //
    // Also reads the EntitySpottedState_t::m_bSpottedByMask bitmask on
    // the local pawn — non-zero means at least one enemy currently has
    // us in their PVS (we're "spotted"), so flicks are visible to them.
    // ---------------------------------------------------------------
    inline int CountObserversWatchingLocal()
    {
        uintptr_t localCtrl = GetLocalController();
        if (!localCtrl) return 0;
        int count = 0;
        for (int i = 1; i <= 64; ++i) {
            __try {
                uintptr_t ctrl = GetEntityByIndex(i);
                if (!ctrl || ctrl == localCtrl) continue;
                uint32_t pawnH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                uintptr_t pawn = ResolveHandle(pawnH);
                if (!pawn) continue;
                int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                if (hp > 0) continue; // alive players aren't spectating
                uintptr_t obs = Mem::Read<uintptr_t>(pawn + Offsets::m_pObserverServices);
                if (!obs) continue;
                uint8_t mode = Mem::Read<uint8_t>(obs + Offsets::m_iObserverMode);
                if (mode == 0) continue; // OBS_MODE_NONE
                uint32_t targetH = Mem::Read<uint32_t>(obs + Offsets::m_hObserverTarget);
                uintptr_t target = ResolveHandle(targetH);
                // ObserverTarget can be either the controller or the pawn
                // depending on observer mode — accept either.
                if (target == localCtrl || target == GetLocalPawn())
                    ++count;
            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }
        return count;
    }

    inline bool IsLocalPawnSpotted()
    {
        uintptr_t pawn = GetLocalPawn();
        if (!pawn) return false;
        __try {
            uintptr_t state = pawn + Offsets::m_entitySpottedState;
            uint32_t lo = Mem::Read<uint32_t>(state + Offsets::m_bSpottedByMask_inSpottedState);
            uint32_t hi = Mem::Read<uint32_t>(state + Offsets::m_bSpottedByMask_inSpottedState + 4);
            return (lo | hi) != 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    }

    // ---------------------------------------------------------------
    // IsValveOfficialServer — reads C_CSGameRules::m_bIsValveDS.
    //
    // VAC Live and the Overwatch reviewer system only ingest demos from
    // Valve's official matchmaking dedicated servers (Premier, Competitive,
    // Wingman, Casual, DM on Valve infra). Community servers, FACEIT,
    // ESEA, local listen servers, and most workshop maps return false.
    //
    // Used as the master throttle gate for silent aim — on Valve servers
    // we cap flicks much harder and skip more often, on community servers
    // we let the full configured strength through.
    // ---------------------------------------------------------------
    inline bool IsValveOfficialServer()
    {
        if (!clientBase) return true;  // assume worst-case until resolved
        uintptr_t rules = Mem::Read<uintptr_t>(clientBase + RVA_dwGameRules());
        if (!rules) return false;       // no rules = main menu / loading
        __try {
            return Mem::Read<bool>(rules + Offsets::m_bIsValveDS);
        } __except (EXCEPTION_EXECUTE_HANDLER) { return true; }
    }

    inline bool HasMatchStarted()
    {
        if (!clientBase) return false;
        uintptr_t rules = Mem::Read<uintptr_t>(clientBase + RVA_dwGameRules());
        if (!rules) return false;
        __try {
            return Mem::Read<bool>(rules + Offsets::m_bHasMatchStarted);
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    }

    // ---------------------------------------------------------------
    // IsFreezeOrWarmup — returns true when the server is in any state
    // where attack inputs are dropped before they reach FireBullet:
    //   * m_bFreezePeriod  — round-start armory window
    //   * m_bWarmupPeriod  — pre-match warmup
    // Silent angle rewrites during these states cannot fire a bullet
    // but the angle desync is still serialised to the demo, so we
    // suppress them entirely. Free win, zero downside.
    // ---------------------------------------------------------------
    inline bool IsFreezeOrWarmup()
    {
        if (!clientBase) return false;
        uintptr_t rules = Mem::Read<uintptr_t>(clientBase + RVA_dwGameRules());
        if (!rules) return false;
        __try {
            bool fz = Mem::Read<bool>(rules + Offsets::m_bFreezePeriod);
            bool wu = Mem::Read<bool>(rules + Offsets::m_bWarmupPeriod);
            return fz || wu;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    }

    // ---------------------------------------------------------------
    // IsSniperItemDef — true for AWP / SSG-08 / G3SG1 / SCAR-20.
    // Used by anti-detection to refuse silent firing while no-scoped.
    // ---------------------------------------------------------------
    inline bool IsSniperItemDef(uint16_t def)
    {
        return def == Offsets::kItemDefAWP    ||
               def == Offsets::kItemDefSSG08  ||
               def == Offsets::kItemDefG3SG1  ||
               def == Offsets::kItemDefSCAR20;
    }

    // ---------------------------------------------------------------
    // ReadWeaponItemDef — reads m_AttributeManager.m_Item.m_iItemDef
    // from a weapon entity. Returns 0 on failure.
    // ---------------------------------------------------------------
    inline uint16_t ReadWeaponItemDef(uintptr_t weapon)
    {
        if (!weapon) return 0;
        __try {
            return Mem::Read<uint16_t>(weapon + Offsets::kWeaponItemDefIndexOffset);
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
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

    // Detected bone-array offset inside CModelState (set lazily by GetBonePos).
    // Exposed for diagnostic overlays so we can verify auto-detect succeeded.
    inline int detectedBoneArrayOffset = 0;

    inline Math::Vec3 GetBonePos(uintptr_t pawn, int bone)
    {
        if (bone < 0 || bone > 128) return {}; // sanity clamp
        uintptr_t node = Mem::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
        if (!node) return {};

        // Bone-array offset inside CModelState shifts between CS2 builds.
        // Auto-detect once by probing a short list of candidates and picking
        // the one whose bone[0] (root) sits within ~80 units of the entity
        // origin. Sticky after first hit.
        static int s_boneArrayOffset = 0;
        struct BoneData { float px, py, pz, pad, qx, qy, qz, qw; };

        if (!s_boneArrayOffset)
        {
            Math::Vec3 origin = Mem::Read<Math::Vec3>(node + Offsets::m_vecAbsOrigin);
            if (origin.IsZero()) return {};
            // Build 14152+ uses 0x1D0 (verified UC intel Apr 2026).
            // Probe in order of likelihood — newest first — so we settle on
            // the live offset on the first pass instead of a coincidental match.
            static const int kCandidates[] = {
                0x1D0, 0x80, 0x28, 0x40, 0x60, 0xA0, 0xC0, 0xE0,
                0x100, 0x140, 0x180, 0x1C0, 0x1E0, 0x200, 0x220
            };
            // Pick the BEST candidate (smallest distance to origin) instead
            // of the first that passes a loose 80u test. Old logic locked
            // onto 0x80 in build 14152+ because some unrelated float trio
            // there happened to land within 80u of origin, even though the
            // actual bone array lives at 0x1D0. Demand <= 30u and pick the
            // tightest match across the whole candidate list.
            //
            // ADDITIONAL VALIDATOR: bone data also contains a quaternion at
            // +0x10 (qx,qy,qz,qw). Random memory won't have unit-length
            // quaternions; real bones do. Probe bones[0] AND bones[1] AND
            // require both pass position + quaternion sanity.
            auto probeOk = [](const BoneData& b, const Math::Vec3& org, float maxDistSq) -> bool {
                if (isnan(b.px) || isinf(b.px)) return false;
                if (isnan(b.py) || isinf(b.py)) return false;
                if (isnan(b.pz) || isinf(b.pz)) return false;
                if (isnan(b.qx) || isinf(b.qx)) return false;
                if (isnan(b.qw) || isinf(b.qw)) return false;
                float dx = b.px - org.x, dy = b.py - org.y, dz = b.pz - org.z;
                if (dx*dx + dy*dy + dz*dz > maxDistSq) return false;
                // Unit quaternion check: |q|² should be ~1.0
                float qlen = b.qx*b.qx + b.qy*b.qy + b.qz*b.qz + b.qw*b.qw;
                if (qlen < 0.85f || qlen > 1.15f) return false;
                return true;
            };
            int   bestCand   = 0;
            float bestDistSq = 900.f; // 30u squared
            for (int cand : kCandidates)
            {
                uintptr_t arrTry = Mem::Read<uintptr_t>(
                    node + Offsets::m_modelState + cand);
                if (!arrTry || arrTry < 0x10000) continue;
                BoneData b0 = Mem::Read<BoneData>(arrTry);
                BoneData b1 = Mem::Read<BoneData>(arrTry + sizeof(BoneData));
                // bone[0] within 30u of origin, bone[1] within 80u (pelvis ~ origin, next bone close)
                if (!probeOk(b0, origin, 900.f))   continue;
                if (!probeOk(b1, origin, 6400.f)) continue;
                float dx = b0.px - origin.x;
                float dy = b0.py - origin.y;
                float dz = b0.pz - origin.z;
                float d2 = dx*dx + dy*dy + dz*dz;
                if (d2 < bestDistSq)
                {
                    bestDistSq = d2;
                    bestCand   = cand;
                }
            }
            if (bestCand)
            {
                s_boneArrayOffset = bestCand;
                detectedBoneArrayOffset = bestCand;
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
