#pragma once

// ---------------------------------------------------------------
// Model Changer â€” client-side player model swap.
//
// Approach: pure memory write, no hooking required.
//
//   pawn -> m_pGameSceneNode (0x330)
//        -> m_modelState     (0x150)
//        -> m_hModel         (0xA0)   <-- CStrongHandle<CModel>
//
// Each tick we walk the entity list, find a "donor" pawn already
// in the world that uses our chosen agent model, and copy that
// pawn's m_hModel handle onto the local pawn. The handle is just
// an 8-byte pointer to a precached CModel resource â€” same format
// across all entities, so the renderer happily uses it.
//
// Why this works:
//   * The agent's vmdl_c is already loaded (the donor is rendering
//     it), so no resource I/O is triggered by our write.
//   * Skeleton + bone count + hitboxes are server-authoritative;
//     a Phoenix-shaped Phoenix and a Phoenix-shaped Cmdr.Frank
//     have effectively identical rigs, so animations don't break.
//   * Other players never see our swap â€” the model handle lives
//     only on the local copy of our pawn entity.
//
// Limits:
//   * Need a donor pawn alive in the current match using the
//     desired agent. If the only Cmdr.Frank dies and respawns
//     as something else, our model snaps back. We cache the last
//     known good handle to ride out short gaps.
//   * Server hitboxes don't change. Headshots register on the
//     ORIGINAL model's head position. Stick to similar-height
//     agents to avoid surprises (all default agents are within
//     a few units â€” fine in practice).
//   * We restore the original handle on disable so toggling off
//     is clean (no permanent corruption).
//
// Anti-cheat posture: this is a pure data write to client-only
// memory (the entity copy in client.dll's entity list, which is
// reconstructed each tick from network deltas anyway). VAC has
// no signature for this â€” it's how scoreboard/agent-preview menus
// internally swap models too.
// ---------------------------------------------------------------

#include <cstdint>
#include <cstring>
#include "../../core/memory.h"
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"

namespace ModelChanger
{
    // Known agent models present on most competitive maps.
    // The "name match" string is what we expect to find in
    // CModelState::m_ModelName (CUtlSymbolLarge â€” points to an
    // interned char* the engine never moves once loaded).
    struct AgentModel
    {
        const char* displayName;
        const char* matchSubstr;  // any substring is fine; case-insensitive contains
        int         team;         // 2 = T, 3 = CT, 0 = either
    };

    inline const AgentModel kAgents[] = {
        // Terrorists
        { "Phoenix",            "tm_phoenix",         2 },
        { "Balkan",             "tm_balkan",          2 },
        { "Professional",       "tm_professional",    2 },
        { "Leet",               "tm_leet",            2 },
        { "Anarchist",          "tm_anarchist",       2 },
        { "Pirate",             "tm_pirate",          2 },
        { "Separatist",         "tm_separatist",      2 },
        // Counter-Terrorists
        { "SAS",                "ctm_sas",            3 },
        { "GIGN",               "ctm_gign",           3 },
        { "FBI",                "ctm_fbi",            3 },
        { "IDF",                "ctm_idf",            3 },
        { "ST6",                "ctm_st6",            3 },
        { "SWAT",               "ctm_swat",           3 },
        { "Heavy",              "ctm_heavy",          3 },
    };
    inline constexpr int kAgentCount = (int)(sizeof(kAgents) / sizeof(kAgents[0]));

    struct Config
    {
        bool enabled         = false;
        int  selectedAgent   = 0;   // index into kAgents
    };
    inline Config cfg;

    // Cached state.
    inline uintptr_t s_savedHandle      = 0;   // local pawn's original m_hModel (for restore on disable)
    inline uintptr_t s_savedFromPawn    = 0;   // which pawn the cached handle came from (for invalidation)
    inline uintptr_t s_lastDonorHandle  = 0;   // last good donor handle (re-applied if donor dies temporarily)
    inline int       s_lastAppliedIdx   = -1;  // which agent index we last copied
    inline int       s_tickThrottle     = 0;   // re-scan donor every N frames (cheap, but not every frame)

    // Case-insensitive substring search on a NUL-terminated string at `cstr`.
    // Reads up to maxLen bytes. Returns true on match. SEH-safe wrapper used
    // by callers; this helper assumes the read already succeeded.
    inline bool ContainsI(const char* hay, int hayLen, const char* needle)
    {
        if (!hay || !needle) return false;
        int nlen = 0;
        while (needle[nlen]) ++nlen;
        if (nlen == 0 || nlen > hayLen) return false;
        for (int i = 0; i <= hayLen - nlen; ++i) {
            int j = 0;
            for (; j < nlen; ++j) {
                char a = hay[i + j];
                char b = needle[j];
                if (a >= 'A' && a <= 'Z') a += 32;
                if (b >= 'A' && b <= 'Z') b += 32;
                if (a != b) break;
            }
            if (j == nlen) return true;
        }
        return false;
    }

    // Read CModelState* off a pawn safely. Returns 0 on any failure.
    inline uintptr_t GetModelState(uintptr_t pawn)
    {
        if (!pawn) return 0;
        __try {
            uintptr_t sceneNode = Mem::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
            if (!sceneNode) return 0;
            return sceneNode + Offsets::m_modelState;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
    }

    // Read the model-name string for diagnostics / donor matching.
    // CUtlSymbolLarge is just a pointer to an interned char*; most
    // entities have a real string here, but during loading we may see
    // null/garbage â€” caller must SEH-wrap.
    inline bool ReadModelName(uintptr_t modelState, char* out, int outCap)
    {
        if (!modelState || !out || outCap <= 1) return false;
        __try {
            uintptr_t namePtr = Mem::Read<uintptr_t>(modelState + Offsets::m_ModelName_state);
            if (!namePtr) return false;
            // Bounded copy â€” engine strings are usually < 128 bytes.
            int i = 0;
            for (; i < outCap - 1; ++i) {
                char c = Mem::Read<char>(namePtr + i);
                out[i] = c;
                if (c == 0) break;
            }
            out[i] = 0;
            return i > 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    }

    // Find a donor pawn whose model name contains the desired agent
    // substring. Skips the local pawn (can't donate from self).
    // Returns the donor's m_hModel value (8-byte handle), or 0 if not found.
    inline uintptr_t FindDonorHandle(const char* matchSubstr, int wantTeam, uintptr_t localPawn)
    {
        char nameBuf[160];
        for (int i = 1; i <= 64; ++i) {
            __try {
                uintptr_t ctrl = GameState::GetEntityByIndex(i);
                if (!ctrl || ctrl == GameState::GetLocalController()) continue;
                uint32_t pawnH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                uintptr_t pawn = GameState::ResolveHandle(pawnH);
                if (!pawn || pawn == localPawn) continue;
                int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                if (hp <= 0) continue;  // dead pawns may have stale model state
                if (wantTeam != 0) {
                    uint8_t team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                    if (team != (uint8_t)wantTeam) continue;
                }
                uintptr_t ms = GetModelState(pawn);
                if (!ms) continue;
                if (!ReadModelName(ms, nameBuf, sizeof(nameBuf))) continue;
                int nameLen = 0; while (nameBuf[nameLen]) ++nameLen;
                if (!ContainsI(nameBuf, nameLen, matchSubstr)) continue;
                uintptr_t donorHandle = Mem::Read<uintptr_t>(ms + Offsets::m_hModel);
                if (donorHandle) return donorHandle;
            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }
        return 0;
    }

    // Restore the local pawn's original handle (called on disable
    // and on agent-selection change so we cleanly switch).
    inline void Restore()
    {
        if (!s_savedHandle) return;
        __try {
            uintptr_t pawn = GameState::GetLocalPawn();
            if (!pawn || pawn != s_savedFromPawn) {
                // Pawn changed (round restart, team switch, etc.) â€” drop
                // the stale cache rather than write a handle from a now-
                // unrelated pawn into a different one.
                s_savedHandle = 0;
                s_savedFromPawn = 0;
                s_lastAppliedIdx = -1;
                return;
            }
            uintptr_t ms = GetModelState(pawn);
            if (!ms) return;
            Mem::SmartWrite<uintptr_t>(ms + Offsets::m_hModel, s_savedHandle);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        s_savedHandle = 0;
        s_savedFromPawn = 0;
        s_lastAppliedIdx = -1;
        s_lastDonorHandle = 0;
    }

    inline void Tick()
    {
        if (!cfg.enabled) {
            if (s_savedHandle) Restore();
            return;
        }
        if (cfg.selectedAgent < 0 || cfg.selectedAgent >= kAgentCount) return;

        uintptr_t pawn = GameState::GetLocalPawn();
        if (!pawn) return;

        // Pawn pointer changed (respawn / team switch) â€” drop cache so
        // we re-snapshot the new pawn's original handle below.
        if (s_savedFromPawn && s_savedFromPawn != pawn) {
            s_savedHandle = 0;
            s_savedFromPawn = 0;
            s_lastAppliedIdx = -1;
        }

        uintptr_t ms = GetModelState(pawn);
        if (!ms) return;

        // Snapshot original handle once per pawn so Restore() can undo.
        if (!s_savedHandle) {
            __try {
                s_savedHandle   = Mem::Read<uintptr_t>(ms + Offsets::m_hModel);
                s_savedFromPawn = pawn;
            } __except (EXCEPTION_EXECUTE_HANDLER) { return; }
            if (!s_savedHandle) return;  // model not loaded yet â€” try next tick
        }

        // If user changed agent, invalidate last-good cache.
        if (s_lastAppliedIdx != cfg.selectedAgent) {
            s_lastDonorHandle = 0;
            s_lastAppliedIdx = cfg.selectedAgent;
        }

        // Re-scan for donor every ~30 frames (~0.5s @ 60fps). In between
        // we re-apply s_lastDonorHandle, which is dirt cheap. Walking the
        // 64-slot entity list every frame is fine too, but this keeps the
        // per-frame cost essentially zero once we have a donor.
        if (++s_tickThrottle >= 30 || s_lastDonorHandle == 0) {
            s_tickThrottle = 0;
            const AgentModel& a = kAgents[cfg.selectedAgent];
            uintptr_t donor = FindDonorHandle(a.matchSubstr, a.team, pawn);
            if (donor) s_lastDonorHandle = donor;
        }

        if (!s_lastDonorHandle) return;  // no donor in match â€” leave model alone

        // Per-tick rewrite. The engine refreshes m_hModel from the
        // network state during entity updates, so we have to keep
        // writing it. Cost: one 8-byte write per frame.
        __try {
            Mem::SmartWrite<uintptr_t>(ms + Offsets::m_hModel, s_lastDonorHandle);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Called on shutdown / unload. Best-effort; if the game is already
    // tearing down the pawn, the SmartWrite no-ops harmlessly.
    inline void Shutdown()
    {
        Restore();
    }
}
