#pragma once

// =====================================================================
//  EngineTrace — direct wrappers over Valve's bullet-trace pipeline.
//
//  Six exported helpers from client.dll let us reproduce a full
//  weapon-fire ray cast (init data → init filter → create trace →
//  iterate surface hits → optionally handle penetration), giving us
//  bit-exact ground-truth for vischeck, autowall, edge-jump, and
//  seeded-triggerbot validation — no need to roll our own BSP code.
//
//  Sigs verified for CS2 build 14158 (IDA, 2026-05-03). One sig
//  (HandleBulletPenetration) needed updating from the original UC
//  post — see core/signatures.h for the full story.
//
//  Layout caveats (struct sizes mirror UC source):
//   * `CGameTrace` ~0xE0 bytes, 16-aligned. We only read fraction,
//     hit-entity ptr, end-pos and start-pos — the rest is opaque pad.
//   * `TraceData_t` carries the surface array + per-segment modulate
//     entries the engine fills as the ray scans through volumes.
//
//  Thread-safety: all engine fns are reentrant-safe at frame scope.
//  We still wrap calls in SEH so a stale/null skip-pawn during map
//  load can't take the cheat down.
// =====================================================================

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "math.h"
#include "memory.h"
#include "signatures.h"
#include "sdk_offsets.h"
#include "game_state.h"

namespace EngineTrace
{
    // ----- TraceMask presets (subset; standard Source convention) -----
    namespace Mask
    {
        constexpr std::uint64_t Solid       = 0x4003B;   // PLAYERSOLID|MOVEABLE|WINDOW|MONSTER|GRATE
        constexpr std::uint64_t Visible     = 0x46004B;  // VISIBLE|OPAQUE
        constexpr std::uint64_t ShotHull    = 0x60003B;  // CONTENTS_HITBOX|SOLID
        constexpr std::uint64_t Autowall    = 0x1C200B;  // SOLID|MONSTER|WINDOW|HITBOX
    }

    // ----- Mirror of UC structs, padded for build-14158 layout -----
    struct alignas(16) CGameTrace
    {
        void*      m_pSurfaceProps;     // +0
        void*      m_pHitEntity;        // +8   (CEntityInstance*)
        void*      m_pHitboxData;       // +16
        char       _pad1[0x10];         // +24
        std::uint32_t m_Contents;       // +40
        char       _pad2[0x4A];         // +44
        Math::Vec3 m_StartPos;          // +118
        Math::Vec3 m_EndPos;            // +124  (impact world pos)
        Math::Vec3 m_Normal;            // +130
        Math::Vec3 m_Pos;               // +136
        char       _pad3[0x4];
        float      fraction;            // 1.0 == clear path
        char       _padA[0xC];
        char       _padB[0x6];
        bool       m_AllSolid;
        char       _padC[0x4D];
    };

    struct trace_array_element_t { char data[0x30]; };

    struct bullet_modulate_entry_t
    {
        float    startFrac;
        float    endFrac;
        float    damage;
        int      maxSecondaryTraces;
        std::uint16_t surfStart;
        std::uint16_t surfEnd;
        std::uint8_t  flags;
        std::uint8_t  pad[3];
    };

    struct bullet_mod_array_t
    {
        int                       size;
        char                      pad4[4];
        bullet_modulate_entry_t*  data;
        char                      pad16[8];
    };

    struct alignas(16) TraceData_t
    {
        char                     pad0[24];
        trace_array_element_t    m_Arr[0x80];
        char                     pad6168[8];
        bullet_mod_array_t       mod_array;
        bullet_modulate_entry_t  mod_inline[8];
        Math::Vec3               tail_start;
        Math::Vec3               tail_end;
        char                     _pad6200[12];
    };

    struct alignas(16) CTraceFilter { char pad[164]; };

    struct handle_bullet_data_t
    {
        float damage;
        float penetration;
        float rangeModifier;
        float tailLength;
        int   penetrationCount;
        bool  penetrationStopped;
        char  pad[3];
    };

    // ----- Engine fn types -----
    using InitTraceData_t   = void(__fastcall*)(TraceData_t*);
    using InitTraceInfo_t   = void(__fastcall*)(CGameTrace*);
    using InitTraceFilter_t = void*(__fastcall*)(CTraceFilter*, std::uintptr_t pawn,
                                                 std::uint64_t mask, int traceType, int filterType);
    using CreateTrace_t     = bool(__fastcall*)(TraceData_t*, Math::Vec3 start, Math::Vec3 delta,
                                                CTraceFilter*, int penCount, bool unk);
    using GetTraceInfo_t    = void(__fastcall*)(TraceData_t*, CGameTrace*, float frac, void* surf);
    using HandleBulletPen_t = bool(__fastcall*)(TraceData_t*, handle_bullet_data_t*,
                                                bullet_modulate_entry_t*, int teamNum, void* impactDbg);

    inline InitTraceData_t   pInitTraceData   = nullptr;
    inline InitTraceInfo_t   pInitTraceInfo   = nullptr;
    inline InitTraceFilter_t pInitFilter      = nullptr;
    inline CreateTrace_t     pCreateTrace     = nullptr;
    inline GetTraceInfo_t    pGetTraceInfo    = nullptr;
    inline HandleBulletPen_t pHandleBulletPen = nullptr;

    // Returns count of resolved fns. 6 == fully armed.
    inline int Init()
    {
        HMODULE hClient = GetModuleHandleW(L"client.dll");
        if (!hClient) return 0;
        int n = 0;
        if (auto a = Mem::FindPatternInModule(hClient, Signatures::TraceInitData))
            { pInitTraceData = (InitTraceData_t)a; ++n; }
        if (auto a = Mem::FindPatternInModule(hClient, Signatures::TraceInitInfo))
            { pInitTraceInfo = (InitTraceInfo_t)a; ++n; }
        if (auto a = Mem::FindPatternInModule(hClient, Signatures::TraceInitFilter))
            { pInitFilter = (InitTraceFilter_t)a; ++n; }
        if (auto a = Mem::FindPatternInModule(hClient, Signatures::TraceCreate))
            { pCreateTrace = (CreateTrace_t)a; ++n; }
        if (auto a = Mem::FindPatternInModule(hClient, Signatures::TraceGetInfo))
            { pGetTraceInfo = (GetTraceInfo_t)a; ++n; }
        if (auto a = Mem::FindPatternInModule(hClient, Signatures::TraceHandleBulletPen))
            { pHandleBulletPen = (HandleBulletPen_t)a; ++n; }
        return n;
    }

    inline bool Ready()
    {
        return pInitTraceData && pInitTraceInfo && pInitFilter
            && pCreateTrace   && pGetTraceInfo;
    }

    // -------------------------------------------------------------
    // TraceLine — fire a single ray start→end, skipping `skipPawn`.
    // Fills out CGameTrace (fraction, hit ent, end pos). Returns true
    // if the engine produced a real surface hit; false on clear path
    // or pipeline error (in which case fraction is forced to 1.0).
    // -------------------------------------------------------------
    inline bool TraceLine(const Math::Vec3& start, const Math::Vec3& end,
                          CGameTrace* outTrace,
                          std::uintptr_t skipPawn = 0,
                          std::uint64_t mask = Mask::Autowall)
    {
        if (!outTrace) return false;
        outTrace->fraction = 1.0f;
        if (!Ready()) return false;

        __try {
            TraceData_t td{};
            pInitTraceData(&td);

            CTraceFilter filter{};
            pInitFilter(&filter, skipPawn, mask, /*traceType*/4, /*filterType*/7);

            Math::Vec3 delta = end - start;
            pCreateTrace(&td, start, delta, &filter, /*penCount*/4, /*unk*/true);

            pInitTraceInfo(outTrace);

            auto& arr = td.mod_array;
            if (arr.size > 0 && arr.data)
            {
                bullet_modulate_entry_t* entry = &arr.data[0];
                std::uint16_t surfIdx = entry->surfEnd & 0x7FFF;
                if (surfIdx < 0x80)
                {
                    pGetTraceInfo(&td, outTrace, entry->startFrac, &td.m_Arr[surfIdx]);
                    return true;
                }
            }
            return false;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            outTrace->fraction = 1.0f;
            return false;
        }
    }

    // -------------------------------------------------------------
    // IsVisible — eye-of-localPawn → targetWorld vischeck. Returns
    // true if the ray either reaches the target (fraction > 0.97)
    // or terminates on the target pawn itself.
    // -------------------------------------------------------------
    inline bool IsVisible(std::uintptr_t targetPawn, const Math::Vec3& targetPos)
    {
        if (!targetPawn || targetPos.IsZero()) return false;

        std::uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return false;

        Math::Vec3 eye = GameState::GetEntityOrigin(localPawn);
        Math::Vec3 vo  = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecViewOffset);
        eye.x += vo.x; eye.y += vo.y; eye.z += vo.z;
        if (eye.IsZero()) return false;

        alignas(16) CGameTrace tr{};
        TraceLine(eye, targetPos, &tr, localPawn, Mask::Visible);

        if (tr.fraction > 0.97f) return true;
        if (tr.m_pHitEntity && reinterpret_cast<std::uintptr_t>(tr.m_pHitEntity) == targetPawn)
            return true;
        return false;
    }

    // -------------------------------------------------------------
    // GetDamage — autowall scan from start→targetPos. Walks each
    // surface segment via HandleBulletPenetration; returns predicted
    // raw damage if a segment terminates on `targetPawn`. Hardcoded
    // weapon stats (SSG default per UC reference) — callers should
    // override by reading active-weapon stats off-frame and passing
    // a customised wpn block in a future refactor.
    // -------------------------------------------------------------
    inline int GetDamage(const Math::Vec3& start, const Math::Vec3& targetPos,
                         std::uintptr_t skipPawn, std::uintptr_t targetPawn,
                         float wpnDamage    = 88.0f,
                         float wpnPenetrate = 2.5f,
                         float wpnRangeMod  = 0.85f,
                         float wpnRange     = 8192.0f,
                         int   maxPenCount  = 4,
                         int   teamNum      = 3)
    {
        if (!pCreateTrace || !pInitTraceData || !pHandleBulletPen || !pInitFilter || !pGetTraceInfo || !pInitTraceInfo)
            return 0;

        Math::Vec3 dir = targetPos - start;
        float distToTgt = dir.Length();
        if (distToTgt < 1.0f) return 0;
        Math::Vec3 dirN = { dir.x / distToTgt, dir.y / distToTgt, dir.z / distToTgt };
        Math::Vec3 delta = { dirN.x * wpnRange, dirN.y * wpnRange, dirN.z * wpnRange };

        __try {
            TraceData_t td{};
            pInitTraceData(&td);

            alignas(16) CTraceFilter filter{};
            pInitFilter(&filter, skipPawn, Mask::Autowall, 4, 7);
            pCreateTrace(&td, start, delta, &filter, maxPenCount, true);

            handle_bullet_data_t bullet{};
            bullet.damage             = wpnDamage;
            bullet.penetration        = wpnPenetrate;
            bullet.rangeModifier      = wpnRangeMod;
            bullet.penetrationCount   = maxPenCount;
            bullet.penetrationStopped = false;

            auto& arr = td.mod_array;
            for (int i = 0; i < arr.size; ++i)
            {
                bullet_modulate_entry_t* entry = &arr.data[i];
                std::uint16_t surfIdx = entry->surfEnd & 0x7FFF;
                if (surfIdx >= 0x80) break;

                alignas(16) CGameTrace tr{};
                pInitTraceInfo(&tr);
                pGetTraceInfo(&td, &tr, entry->startFrac, &td.m_Arr[surfIdx]);
                bullet.tailLength = td.tail_end.Length();

                if (tr.m_pHitEntity &&
                    reinterpret_cast<std::uintptr_t>(tr.m_pHitEntity) == targetPawn)
                {
                    Math::Vec3 d = tr.m_EndPos - start;
                    float travel = d.Length();
                    float distMod = std::pow(bullet.rangeModifier, travel / 500.0f);
                    return static_cast<int>(bullet.damage * distMod);
                }

                if (pHandleBulletPen(&td, &bullet, entry, teamNum, nullptr)) break;
                if (bullet.penetrationCount <= 0 || bullet.penetrationStopped) break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }

        return 0;
    }
}
