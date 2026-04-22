#pragma once

// ---------------------------------------------------------------
// Grenade Prediction v2 — reads engine-computed trajectory data
// instead of simulating our own broken parabola. The engine has
// already done BSP tracing and collision detection, so the trail
// is pixel-accurate to where the nade actually goes.
//
// Features:
//   - Real trajectory from m_arrTrajectoryTrailPoints
//   - Fallback simple sim when trail data unavailable
//   - Type-specific coloring (Flash/Smoke/HE/Fire/Decoy)
//   - Detonation timer countdown
//   - Clean pill label at grenade position
//   - Gradient trail with thickness variation
//   - Landing/detonation marker with pulse ring
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "../core/memory.h"
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/math.h"
#include "../vendor/imgui/imgui.h"

namespace GrenadePrediction
{
    struct Config
    {
        bool  enabled     = false;
        bool  showTrail   = true;
        bool  showLanding = true;
        float trailColor[4] = { 1.f, 0.8f, 0.2f, 0.8f };
    };
    inline Config cfg;

    // Grenade physics constants (Source 2 — fallback sim only)
    constexpr float kGravity    = 400.f;
    constexpr float kDt         = 1.f / 64.f;
    constexpr float kFriction   = 0.45f;
    constexpr float kElasticity = 0.45f;

    // C_BaseCSGrenadeProjectile + C_SmokeGrenadeProjectile offsets (build 14152)
    namespace Off
    {
        constexpr std::ptrdiff_t m_vInitialPosition                    = 0x11A0;
        constexpr std::ptrdiff_t m_vInitialVelocity                    = 0x11AC;
        constexpr std::ptrdiff_t m_nBounces                            = 0x11B8;
        constexpr std::ptrdiff_t m_nExplodeEffectTickBegin             = 0x11C8;
        constexpr std::ptrdiff_t m_flSpawnTime                         = 0x11D8;
        constexpr std::ptrdiff_t m_bExplodeEffectBegan                 = 0x11EC;
        constexpr std::ptrdiff_t m_arrTrajectoryTrailPoints            = 0x1200; // CUtlVector<Vector>
        constexpr std::ptrdiff_t m_arrTrajectoryTrailPointCreationTimes = 0x1218; // CUtlVector<float>
        // Smoke-specific (C_SmokeGrenadeProjectile)
        constexpr std::ptrdiff_t m_nSmokeEffectTickBegin               = 0x1250;
        constexpr std::ptrdiff_t m_bDidSmokeEffect                     = 0x1254;
        constexpr std::ptrdiff_t m_vSmokeDetonationPos                 = 0x1268;
        // Molotov-specific (C_MolotovProjectile.m_bIsIncGrenade unconfirmed; tentative -0x200 shift)
        constexpr std::ptrdiff_t m_bIsIncGrenade                       = 0x1238;
    };

    // Grenade type enumeration
    enum GrenadeType { NADE_UNKNOWN, NADE_FLASH, NADE_SMOKE, NADE_HE, NADE_FIRE, NADE_DECOY };

    // Detonation timers (seconds after spawn)
    constexpr float kTimerFlash = 1.5f;
    constexpr float kTimerHE    = 1.5f;
    constexpr float kTimerSmoke = 3.0f;  // until puff
    constexpr float kTimerDecoy = 15.0f;
    constexpr float kTimerFire  = 2.0f;  // molotov/incendiary

    // Identify grenade type from designer name
    inline GrenadeType ClassifyGrenade(const char* dname)
    {
        if (strstr(dname, "flashbang"))    return NADE_FLASH;
        if (strstr(dname, "smokegrenade")) return NADE_SMOKE;
        if (strstr(dname, "hegrenade"))    return NADE_HE;
        if (strstr(dname, "molotov") || strstr(dname, "incendiary")) return NADE_FIRE;
        if (strstr(dname, "decoy"))        return NADE_DECOY;
        return NADE_UNKNOWN;
    }

    // Get display name and color for a grenade type
    inline const char* GetTypeName(GrenadeType t)
    {
        switch (t)
        {
        case NADE_FLASH: return "FLASH";
        case NADE_SMOKE: return "SMOKE";
        case NADE_HE:    return "HE";
        case NADE_FIRE:  return "FIRE";
        case NADE_DECOY: return "DECOY";
        default:         return "NADE";
        }
    }

    inline ImU32 GetTypeColor(GrenadeType t)
    {
        switch (t)
        {
        case NADE_FLASH: return IM_COL32(255, 255, 100, 220);
        case NADE_SMOKE: return IM_COL32(160, 200, 255, 220);
        case NADE_HE:    return IM_COL32(255, 80, 40, 220);
        case NADE_FIRE:  return IM_COL32(255, 130, 30, 220);
        case NADE_DECOY: return IM_COL32(120, 255, 120, 220);
        default:         return IM_COL32(255, 200, 60, 220);
        }
    }

    inline float GetDetonationTime(GrenadeType t)
    {
        switch (t)
        {
        case NADE_FLASH: return kTimerFlash;
        case NADE_SMOKE: return kTimerSmoke;
        case NADE_HE:    return kTimerHE;
        case NADE_FIRE:  return kTimerFire;
        case NADE_DECOY: return kTimerDecoy;
        default:         return 2.0f;
        }
    }

    // Read engine trajectory trail points (CUtlVector<Vector>)
    // Returns number of points read (0 if unavailable)
    inline int ReadTrailPoints(uintptr_t ent, Math::Vec3* outPts, int maxPts)
    {
        // CUtlVector layout: int size at +0x0, T* data at +0x8
        int count = Mem::Read<int>(ent + Off::m_arrTrajectoryTrailPoints);
        if (count <= 0 || count > 512) return 0;
        if (count > maxPts) count = maxPts;

        uintptr_t dataPtr = Mem::Read<uintptr_t>(ent + Off::m_arrTrajectoryTrailPoints + 0x8);
        if (!dataPtr) return 0;

        // Read all points in one batch for efficiency (Vec3 = 12 bytes each)
        // We read in chunks to avoid huge single reads
        for (int i = 0; i < count; ++i)
        {
            outPts[i] = Mem::Read<Math::Vec3>(dataPtr + i * 12);
            // Sanity: reject garbage data
            if (outPts[i].x == 0.f && outPts[i].y == 0.f && outPts[i].z == 0.f)
                return i; // Truncate at first zero point
        }
        return count;
    }

    // Fallback: simple parabolic simulation (no BSP tracing)
    inline int SimulateTrajectory(Math::Vec3 startPos, Math::Vec3 velocity,
                                  Math::Vec3* outPts, int maxPts)
    {
        int count = 0;
        Math::Vec3 pos = startPos;
        Math::Vec3 vel = velocity;

        for (int i = 0; i < maxPts; ++i)
        {
            outPts[count++] = pos;
            vel.z -= kGravity * kDt;
            pos.x += vel.x * kDt;
            pos.y += vel.y * kDt;
            pos.z += vel.z * kDt;
            if (pos.z < startPos.z - 200.f && vel.z < 0.f)
            {
                vel.z = -vel.z * kElasticity;
                vel.x *= kFriction;
                vel.y *= kFriction;
                pos.z = startPos.z - 200.f;
                float speed = sqrtf(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
                if (speed < 10.f) break;
            }
        }
        return count;
    }

    // Draw a gradient trail line with thickness variation
    inline void DrawTrail(ImDrawList* dl, float scrW, float scrH,
                          Math::Vec3* pts, int count, ImU32 col)
    {
        if (count < 2) return;

        float prevSx = 0.f, prevSy = 0.f;
        bool prevValid = false;

        for (int i = 0; i < count; ++i)
        {
            float w[3] = { pts[i].x, pts[i].y, pts[i].z };
            float sx, sy;
            bool valid = GameState::WorldToScreen(w, sx, sy, scrW, scrH);

            if (valid && prevValid)
            {
                // Alpha fades toward the end of the trail
                float t = (float)i / (float)(count - 1);
                float alpha = 1.0f - t * 0.5f;
                // Thickness tapers from 2.0 to 1.0
                float thick = 2.0f - t * 1.0f;

                uint8_t baseA = (col >> 24) & 0xFF;
                uint8_t a = (uint8_t)(baseA * alpha);
                ImU32 c = (col & 0x00FFFFFF) | ((uint32_t)a << 24);

                dl->AddLine(ImVec2(prevSx, prevSy), ImVec2(sx, sy), c, thick);
            }

            prevSx = sx;
            prevSy = sy;
            prevValid = valid;
        }
    }

    // Draw landing/detonation marker with animated pulse
    inline void DrawLandingMarker(ImDrawList* dl, float scrW, float scrH,
                                  Math::Vec3 pos, ImU32 col, GrenadeType type)
    {
        float w[3] = { pos.x, pos.y, pos.z };
        float sx, sy;
        if (!GameState::WorldToScreen(w, sx, sy, scrW, scrH)) return;

        // Pulsing outer ring (2 Hz)
        float t = (float)(GetTickCount() % 500) / 500.f;
        float pulse = 1.0f + sinf(t * 6.283f) * 0.3f;

        float baseR = 8.f;
        float outerR = baseR * pulse;

        // Death zone indicator for HE/Fire (damage radius hint)
        if (type == NADE_HE || type == NADE_FIRE)
        {
            uint8_t ringA = (uint8_t)(40 + 20 * pulse);
            dl->AddCircleFilled(ImVec2(sx, sy), outerR + 6.f,
                                (col & 0x00FFFFFF) | ((uint32_t)ringA << 24), 24);
        }

        // X marker
        float sz = 6.f;
        dl->AddLine(ImVec2(sx - sz, sy - sz), ImVec2(sx + sz, sy + sz), col, 2.f);
        dl->AddLine(ImVec2(sx + sz, sy - sz), ImVec2(sx - sz, sy + sz), col, 2.f);

        // Inner circle
        dl->AddCircle(ImVec2(sx, sy), baseR, col, 16, 1.5f);
        // Outer pulse ring
        uint8_t pulseA = (uint8_t)((col >> 24) * 0.4f);
        dl->AddCircle(ImVec2(sx, sy), outerR + 3.f,
                      (col & 0x00FFFFFF) | ((uint32_t)pulseA << 24), 20, 1.f);
    }

    // Draw styled pill label at a screen position
    inline void DrawPillLabel(ImDrawList* dl, float sx, float sy,
                              const char* text, ImU32 col, const char* subText = nullptr)
    {
        ImVec2 tsz = ImGui::CalcTextSize(text);
        float pw = tsz.x + 12.f;
        float ph = tsz.y + 6.f;

        if (subText)
        {
            ImVec2 ssz = ImGui::CalcTextSize(subText);
            if (ssz.x + 12.f > pw) pw = ssz.x + 12.f;
            ph += ssz.y + 2.f;
        }

        float px = sx - pw * 0.5f;
        float py = sy - ph - 6.f;

        // Background pill
        dl->AddRectFilled(ImVec2(px, py), ImVec2(px + pw, py + ph),
                          IM_COL32(10, 10, 15, 200), 5.f);
        // Colored top accent line
        dl->AddLine(ImVec2(px + 4.f, py), ImVec2(px + pw - 4.f, py), col, 2.f);
        // Border
        dl->AddRect(ImVec2(px, py), ImVec2(px + pw, py + ph),
                    (col & 0x00FFFFFF) | 0x50000000, 5.f, 0, 1.f);

        // Main text (centered)
        dl->AddText(ImVec2(sx - tsz.x * 0.5f, py + 3.f), col, text);

        // Sub-text (timer, etc.)
        if (subText)
        {
            ImVec2 ssz = ImGui::CalcTextSize(subText);
            dl->AddText(ImVec2(sx - ssz.x * 0.5f, py + 3.f + tsz.y + 2.f),
                        IM_COL32(200, 200, 200, 180), subText);
        }
    }

    // Process a single entity — called from __try block
    inline bool ProcessEntity(ImDrawList* dl, float scrW, float scrH,
                              uintptr_t ent, float gameTime)
    {
        // Read designer name to identify grenades
        char dname[64] = {};
        {
            uintptr_t identity = Mem::Read<uintptr_t>(ent + Offsets::EntitySys::kInstanceToIdentity);
            if (!identity) return false;
            uintptr_t namePtr = Mem::Read<uintptr_t>(identity + Offsets::EntitySys::kIdentityDesignerName);
            if (!namePtr) return false;
            struct Buf { char d[64]; };
            Buf b = Mem::Read<Buf>(namePtr);
            b.d[63] = '\0';
            memcpy(dname, b.d, 64);
        }

        if (!strstr(dname, "projectile")) return false;

        GrenadeType type = ClassifyGrenade(dname);
        ImU32 col = GetTypeColor(type);
        const char* name = GetTypeName(type);

        // Skip if explode effect already started (grenade already detonated)
        bool exploded = Mem::Read<bool>(ent + Off::m_bExplodeEffectBegan);
        // For smoke, also check if smoke effect already started
        if (type == NADE_SMOKE)
        {
            bool didSmoke = Mem::Read<bool>(ent + Off::m_bDidSmokeEffect);
            if (didSmoke) return false; // Smoke already popped
        }
        else if (exploded)
            return false;

        // Get current position
        uintptr_t node = Mem::Read<uintptr_t>(ent + Offsets::m_pGameSceneNode);
        if (!node) return false;
        Math::Vec3 curPos = Mem::Read<Math::Vec3>(node + Offsets::m_vecAbsOrigin);
        if (curPos.IsZero()) return false;

        // Calculate time remaining
        float spawnTime = Mem::Read<float>(ent + Off::m_flSpawnTime);
        float detonationTotal = GetDetonationTime(type);
        float elapsed = (gameTime > spawnTime && spawnTime > 0.f) ? (gameTime - spawnTime) : 0.f;
        float remaining = detonationTotal - elapsed;
        if (remaining < 0.f) remaining = 0.f;

        // Format timer text
        char timerBuf[16] = {};
        if (spawnTime > 0.f && gameTime > 0.f)
            snprintf(timerBuf, sizeof(timerBuf), "%.1fs", remaining);

        // Screen position for label
        float sx, sy;
        float cp[3] = { curPos.x, curPos.y, curPos.z + 8.f }; // Slightly above
        if (GameState::WorldToScreen(cp, sx, sy, scrW, scrH))
        {
            DrawPillLabel(dl, sx, sy, name, col,
                          timerBuf[0] ? timerBuf : nullptr);
        }

        // Draw trajectory trail
        if (cfg.showTrail)
        {
            Math::Vec3 trailPts[256];
            int trailCount = 0;

            // Try reading engine's real trajectory first
            trailCount = ReadTrailPoints(ent, trailPts, 256);

            // Fallback: simulate from current pos/vel
            if (trailCount < 2)
            {
                Math::Vec3 curVel = Mem::Read<Math::Vec3>(ent + Offsets::m_vecVelocity);
                if (!curVel.IsZero())
                    trailCount = SimulateTrajectory(curPos, curVel, trailPts, 128);
            }

            if (trailCount >= 2)
            {
                DrawTrail(dl, scrW, scrH, trailPts, trailCount, col);

                // Draw landing/detonation marker at trail end
                if (cfg.showLanding && trailCount > 0)
                {
                    Math::Vec3 landPos = trailPts[trailCount - 1];

                    // For smoke, prefer actual detonation position if available
                    if (type == NADE_SMOKE)
                    {
                        Math::Vec3 detPos = Mem::Read<Math::Vec3>(ent + Off::m_vSmokeDetonationPos);
                        if (!detPos.IsZero())
                            landPos = detPos;
                    }

                    DrawLandingMarker(dl, scrW, scrH, landPos, col, type);
                }
            }
        }

        return true;
    }

    // Main render entry point
    inline void Render()
    {
        if (!cfg.enabled) return;
        if (!GameState::clientBase) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 disp = ImGui::GetIO().DisplaySize;
        float scrW = disp.x, scrH = disp.y;

        uintptr_t entList = GameState::GetEntityList();
        if (!entList) return;

        float gameTime = GameState::GetGameTime();

        // Get max entity index
        uintptr_t entitySystem = Mem::Read<uintptr_t>(
            GameState::clientBase + Offsets::Global::dwGameEntitySystem);
        int maxIdx = 512;
        if (entitySystem)
        {
            int hi = Mem::Read<int>(entitySystem + 0x20A0);
            if (hi > 0 && hi < 4096) maxIdx = hi;
        }

        for (int i = 64; i <= maxIdx; ++i)
        {
            __try {
                uintptr_t ent = GameState::GetEntityByIndex(i);
                if (!ent) continue;

                ProcessEntity(dl, scrW, scrH, ent, gameTime);

            } __except (EXCEPTION_EXECUTE_HANDLER) { continue; }
        }
    }
}
