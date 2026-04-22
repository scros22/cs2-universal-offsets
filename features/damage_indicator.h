#pragma once

// ---------------------------------------------------------------
// Damage Indicator — on-screen feed showing damage dealt/received.
// Tracks per-player health deltas correlated with local player shots.
// Format: [License] +24 Damage to PlayerName
// ---------------------------------------------------------------

#include <Windows.h>
#include <vector>
#include <mutex>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../vendor/imgui/imgui.h"

namespace DamageIndicator
{
    struct Config
    {
        bool  enabled     = true;    // on by default
        float displayTime = 5.0f;    // seconds to show each entry
        int   position    = 0;       // 0 = left, 1 = right
        int   style       = 0;       // 0 = Classic, 1 = Minimal, 2 = Bold
    };

    inline Config cfg;

    struct Entry
    {
        char  text[128];
        float spawnTime;
        bool  outgoing;   // true = we dealt, false = we received
    };

    inline std::vector<Entry> entries;
    inline std::mutex mtx;

    // Per-player health tracking (indexed by entity slot 1-64)
    struct PlayerTrack
    {
        int       health = 0;
        int       shots  = 0;
        uintptr_t pawn   = 0;
        uintptr_t ctrl   = 0;
    };

    inline PlayerTrack tracked[65] = {};
    inline bool trackInit  = false;
    inline int  prevShots  = 0;
    inline float lastShotT = 0.f;
    inline int  localPrevHp = 0;

    inline float Now() { return (float)GetTickCount64() / 1000.f; }

    // ---------------------------------------------------------------
    // Internal: push an entry (no SEH objects, safe for mutex)
    // ---------------------------------------------------------------
    inline void PushEntry(const char* text, bool outgoing)
    {
        Entry e{};
        strncpy_s(e.text, text, _TRUNCATE);
        e.spawnTime = Now();
        e.outgoing  = outgoing;
        std::lock_guard<std::mutex> lk(mtx);
        entries.push_back(e);
        if (entries.size() > 15)
            entries.erase(entries.begin());
    }

    // ---------------------------------------------------------------
    // Floating damage numbers — appear on-screen at the player hit
    // ---------------------------------------------------------------
    struct FloatingDmg
    {
        uintptr_t pawn;
        int       damage;
        float     spawnTime;
    };

    inline std::vector<FloatingDmg> floatingDmgs;

    inline void PushFloating(uintptr_t pawn, int damage)
    {
        FloatingDmg f;
        f.pawn      = pawn;
        f.damage    = damage;
        f.spawnTime = Now();
        std::lock_guard<std::mutex> lk(mtx);
        floatingDmgs.push_back(f);
        if (floatingDmgs.size() > 20)
            floatingDmgs.erase(floatingDmgs.begin());
    }

    // ---------------------------------------------------------------
    // Tick — run every frame, detect health deltas
    // ---------------------------------------------------------------
    inline void Tick()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        __try {
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn) { trackInit = false; return; }

            uint8_t life = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
            if (life != 0) { trackInit = false; return; }

            int localHp   = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
            int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);

            // Shot correlation — track when we fire
            int curShots = Mem::Read<int32_t>(localPawn + Offsets::m_iShotsFired);
            if (curShots > prevShots)
                lastShotT = Now();
            prevShots = curShots;

            uintptr_t entList = GameState::GetEntityList();
            if (!entList) return;

            // First tick after spawn/respawn — snapshot all health, skip detection
            if (!trackInit)
            {
                trackInit = true;
                localPrevHp = localHp;
                for (int i = 0; i < 65; ++i) tracked[i] = {};

                for (int i = 1; i <= 64; ++i)
                {
                    uintptr_t ctrl = GameState::GetEntityByIndex(i);
                    if (!ctrl) continue;
                    uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                    if (!pH || pH == 0xFFFFFFFF) continue;
                    uintptr_t pawn = GameState::ResolveHandle(pH);
                    if (!pawn) continue;
                    tracked[i].pawn   = pawn;
                    tracked[i].ctrl   = ctrl;
                    tracked[i].health = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                    tracked[i].shots  = Mem::Read<int32_t>(pawn + Offsets::m_iShotsFired);
                }
                return;
            }

            // --- Incoming damage (to us) ---
            if (localHp < localPrevHp && localHp > 0)
            {
                int dmg = localPrevHp - localHp;
                if (dmg > 0 && dmg <= 200)
                {
                    // Find attacker: enemy whose shots increased this frame
                    char attacker[32] = "Unknown";
                    for (int i = 1; i <= 64; ++i)
                    {
                        if (!tracked[i].pawn || tracked[i].pawn == localPawn) continue;
                        int team = Mem::Read<uint8_t>(tracked[i].pawn + Offsets::m_iTeamNum);
                        if (team == localTeam) continue;
                        int curS = Mem::Read<int32_t>(tracked[i].pawn + Offsets::m_iShotsFired);
                        if (curS > tracked[i].shots && tracked[i].ctrl)
                        {
                            uintptr_t np = Mem::Read<uintptr_t>(tracked[i].ctrl + Offsets::m_sSanitizedPlayerName);
                            if (np)
                            {
                                struct NB { char d[32]; };
                                NB nb = Mem::Read<NB>(np);
                                nb.d[31] = '\0';
                                if (nb.d[0]) memcpy(attacker, nb.d, 32);
                            }
                            break;
                        }
                    }

                    char buf[128];
                    snprintf(buf, sizeof(buf), "-%d Damage from %s", dmg, attacker);
                    PushEntry(buf, false);
                }
            }
            localPrevHp = localHp;

            // --- Outgoing damage (from us to enemies) ---
            float now = Now();
            bool recentShot = (now - lastShotT) < 0.25f;

            for (int i = 1; i <= 64; ++i)
            {
                uintptr_t ctrl = GameState::GetEntityByIndex(i);
                if (!ctrl) { tracked[i] = {}; continue; }

                uint32_t pH = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                if (!pH || pH == 0xFFFFFFFF) { tracked[i] = {}; continue; }
                uintptr_t pawn = GameState::ResolveHandle(pH);
                if (!pawn || pawn == localPawn) continue;

                int hp = Mem::Read<int32_t>(pawn + Offsets::m_iHealth);
                int shots = Mem::Read<int32_t>(pawn + Offsets::m_iShotsFired);

                // Player changed (disconnect/reconnect) — reset tracking
                if (pawn != tracked[i].pawn)
                {
                    tracked[i].pawn   = pawn;
                    tracked[i].ctrl   = ctrl;
                    tracked[i].health = hp;
                    tracked[i].shots  = shots;
                    continue;
                }

                int prevHp = tracked[i].health;
                tracked[i].health = hp;
                tracked[i].shots  = shots;
                tracked[i].ctrl   = ctrl;

                int team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);

                // Enemy health dropped while we recently fired
                if (team != localTeam && prevHp > 0 && hp < prevHp && recentShot)
                {
                    int dmg = prevHp - hp;
                    if (dmg > 0 && dmg <= 200)
                    {
                        // Read victim name
                        char name[32] = "Enemy";
                        uintptr_t np = Mem::Read<uintptr_t>(ctrl + Offsets::m_sSanitizedPlayerName);
                        if (np)
                        {
                            struct NB { char d[32]; };
                            NB nb = Mem::Read<NB>(np);
                            nb.d[31] = '\0';
                            if (nb.d[0]) memcpy(name, nb.d, 32);
                        }

                        char buf[128];
                        snprintf(buf, sizeof(buf), "+%d Damage to %s", dmg, name);
                        PushEntry(buf, true);
                        PushFloating(pawn, dmg);
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Outlined text helper (4-dir outline for readability)
    // ---------------------------------------------------------------
    inline void OutlinedText(ImDrawList* dl, ImFont* font, float fs,
                             ImVec2 pos, ImU32 col, const char* txt)
    {
        ImU32 outl = IM_COL32(0, 0, 0, 180);
        dl->AddText(font, fs, ImVec2(pos.x - 1, pos.y), outl, txt);
        dl->AddText(font, fs, ImVec2(pos.x + 1, pos.y), outl, txt);
        dl->AddText(font, fs, ImVec2(pos.x, pos.y - 1), outl, txt);
        dl->AddText(font, fs, ImVec2(pos.x, pos.y + 1), outl, txt);
        dl->AddText(font, fs, pos, col, txt);
    }

    // ---------------------------------------------------------------
    // Render — draw the damage feed on screen
    // ---------------------------------------------------------------
    inline void Render()
    {
        if (!cfg.enabled) return;

        std::lock_guard<std::mutex> lk(mtx);
        float now = Now();

        // Remove expired entries
        entries.erase(
            std::remove_if(entries.begin(), entries.end(),
                [&](const Entry& e) { return (now - e.spawnTime) > cfg.displayTime; }),
            entries.end());

        // Remove expired floating damage
        floatingDmgs.erase(
            std::remove_if(floatingDmgs.begin(), floatingDmgs.end(),
                [&](const FloatingDmg& f) { return (now - f.spawnTime) > 1.5f; }),
            floatingDmgs.end());

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        ImFont* font = ImGui::GetFont();
        ImVec2  disp = ImGui::GetIO().DisplaySize;

        // --- Style sizing ---
        float fs, lineH;
        switch (cfg.style)
        {
        case 1:  // Minimal — smaller, no prefix
            fs = ImGui::GetFontSize();
            lineH = fs + 4.f;
            break;
        case 2:  // Bold — larger
            fs = ImGui::GetFontSize() + 5.f;
            lineH = fs + 8.f;
            break;
        default: // Classic
            fs = ImGui::GetFontSize() + 3.f;
            lineH = fs + 6.f;
            break;
        }

        // --- Position ---
        float baseY = disp.y * 0.65f;
        bool rightSide = (cfg.position == 1);

        // --- Draw feed entries ---
        if (!entries.empty())
        {
            // Build prefix at runtime to avoid static string in binary
            char prefix[12];
            prefix[0]='['; prefix[1]='L'; prefix[2]='i'; prefix[3]='c';
            prefix[4]='e'; prefix[5]='n'; prefix[6]='s'; prefix[7]='e';
            prefix[8]=']'; prefix[9]=' '; prefix[10]='\0';
            ImVec2 prefixSz = font->CalcTextSizeA(fs, FLT_MAX, 0.f, prefix);

            int count = (int)entries.size();
            for (int i = 0; i < count; ++i)
            {
                const Entry& e = entries[i];
                float age   = now - e.spawnTime;
                float alpha = 1.0f;
                float fadeStart = cfg.displayTime - 1.0f;
                if (age > fadeStart)
                    alpha = cfg.displayTime - age;
                if (alpha <= 0.01f) continue;
                if (alpha > 1.f) alpha = 1.f;

                float y = baseY - (count - 1 - i) * lineH;

                // Compute x position
                float x;
                if (rightSide)
                {
                    // Right-align: measure total text width
                    float totalW = (cfg.style == 1) ? 0.f : prefixSz.x;
                    totalW += font->CalcTextSizeA(fs, FLT_MAX, 0.f, e.text).x;
                    x = disp.x - totalW - 14.f;
                }
                else
                {
                    x = 14.f;
                }

                ImU32 textCol = e.outgoing
                    ? IM_COL32(255, 255, 255, (int)(255 * alpha))
                    : IM_COL32(255, 110, 110, (int)(255 * alpha));

                if (cfg.style == 1)
                {
                    // Minimal style — no prefix, just the message
                    OutlinedText(dl, font, fs, ImVec2(x, y), textCol, e.text);
                }
                else
                {
                    // Classic / Bold — "[License] " in orange, then message
                    ImU32 orange = IM_COL32(255, 165, 0, (int)(255 * alpha));
                    OutlinedText(dl, font, fs, ImVec2(x, y), orange, prefix);
                    float tx = x + prefixSz.x;
                    OutlinedText(dl, font, fs, ImVec2(tx, y), textCol, e.text);
                }
            }
        }

        // --- Floating damage numbers (world-space, above enemy head) ---
        float scrW = disp.x, scrH = disp.y;
        for (const auto& f : floatingDmgs)
        {
            float age = now - f.spawnTime;
            if (age > 1.5f) continue;

            Math::Vec3 head = GameState::GetBonePos(f.pawn, 7);
            if (head.IsZero()) continue;

            // Float upward over time
            head.z += 20.f + age * 40.f;

            float sx, sy;
            float wp[3] = {head.x, head.y, head.z};
            if (!GameState::WorldToScreen(wp, sx, sy, scrW, scrH)) continue;

            // Fade out
            float alpha = 1.0f;
            if (age > 0.8f) alpha = (1.5f - age) / 0.7f;
            if (alpha <= 0.01f) continue;

            char buf[16];
            snprintf(buf, sizeof(buf), "-%d", f.damage);

            float dmgFs = ImGui::GetFontSize() + 4.f;
            ImVec2 tSz = font->CalcTextSizeA(dmgFs, FLT_MAX, 0.f, buf);
            ImVec2 pos(sx - tSz.x * 0.5f, sy);

            ImU32 col = IM_COL32(255, 70, 70, (int)(255 * alpha));
            OutlinedText(dl, font, dmgFs, pos, col, buf);
        }
    }
}
