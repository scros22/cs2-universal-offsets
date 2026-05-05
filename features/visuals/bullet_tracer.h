#pragma once

// ---------------------------------------------------------------
// Bullet tracer â€” renders animated trace lines along shot paths.
// ---------------------------------------------------------------

#include <Windows.h>
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
#include "../../core/math.h"
#include "../../core/memory.h"
#include "../../vendor/imgui/imgui.h"

namespace BulletTracer
{
    enum Style : int {
        STYLE_BEAM = 0,    // classic white-blue laser w/ glow
        STYLE_PLASMA,      // thick saturated colored beam + bright core
        STYLE_NEON,        // hard-edge cyber line, sharp head, no glow
        STYLE_FIRE,        // red->orange->yellow gradient, flickering width
        STYLE_RAINBOW,     // hue-cycling along the beam
        STYLE_TRACER_MIL,  // green/red mil-tracer w/ short bright head
        STYLE_GHOST,       // soft white particle dots fading along path
        STYLE_LIGHTNING,   // jagged offset polyline, electric blue
        STYLE_COUNT
    };

    inline const char* kStyleNames[] = {
        "Beam", "Plasma", "Neon", "Fire", "Rainbow",
        "Mil-Tracer", "Ghost", "Lightning"
    };

    struct Config
    {
        bool  enabled     = true;
        int   style       = STYLE_BEAM;
        float trailLife   = 2.5f;
        float bulletSpeed = 8000.f;
        float thickness   = 2.f;
        float rayLength   = 8000.f;

        // Per-style customisation. Color is the dominant tint; styles
        // that animate (Rainbow, Fire) ignore it.
        float color[4]    = { 0.78f, 0.86f, 1.0f, 1.0f };
        float glowColor[4]= { 0.71f, 0.78f, 1.0f, 0.16f };
        bool  glow        = true;
        float glowMult    = 3.5f;       // glow line width = thickness * glowMult
        bool  showHead    = true;       // bright dot at the bullet leading edge
        float headRadius  = 3.f;
        bool  fadeFromMuzzle = true;    // brightness ramps from muzzle->tip
    };


    inline Config cfg;

    struct Trace
    {
        float start[3], end[3];
        float spawnTime, totalDist;
    };

    inline std::vector<Trace> traces;
    inline std::mutex traceMtx;
    inline int prevShotsFired = 0;

    inline float Now() { return static_cast<float>(GetTickCount64()) / 1000.f; }

    inline void AddTraceFromAngles(float ex, float ey, float ez, float pitch, float yaw)
    {
        if (!cfg.enabled) return;
        float dir[3];
        Math::AngleToDirection(pitch, yaw, dir);

        Trace t;
        t.start[0] = ex; t.start[1] = ey; t.start[2] = ez;
        t.end[0] = ex + dir[0] * cfg.rayLength;
        t.end[1] = ey + dir[1] * cfg.rayLength;
        t.end[2] = ez + dir[2] * cfg.rayLength;
        t.spawnTime = Now();

        float dx = t.end[0] - ex, dy = t.end[1] - ey, dz = t.end[2] - ez;
        t.totalDist = sqrtf(dx * dx + dy * dy + dz * dz);

        std::lock_guard<std::mutex> lk(traceMtx);
        traces.push_back(t);
    }

    inline bool DetectShot(uintptr_t pawn)
    {
        if (!pawn) return false;
        int cur = Mem::Read<int>(pawn + Offsets::m_iShotsFired);
        if (cur > prevShotsFired && prevShotsFired >= 0)
        {
            prevShotsFired = cur;
            return true;
        }
        prevShotsFired = cur;
        return false;
    }

    inline ImU32 PackColor(const float c[4], float alpha)
    {
        auto cl = [](float v){ int i=(int)(v*255.f); if(i<0)i=0; if(i>255)i=255; return i; };
        int a = (int)(alpha * 255.f); if (a < 0) a = 0; if (a > 255) a = 255;
        return IM_COL32(cl(c[0]), cl(c[1]), cl(c[2]), a);
    }

    inline ImU32 HsvToCol(float h, float s, float v, float a)
    {
        float r=0,g=0,b=0;
        ImGui::ColorConvertHSVtoRGB(h - floorf(h), s, v, r, g, b);
        int A = (int)(a * 255.f); if (A < 0) A = 0; if (A > 255) A = 255;
        return IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), A);
    }

    inline void Render()
    {
        if (!cfg.enabled || !GameState::clientBase) return;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        float now = Now();
        ImVec2 disp = ImGui::GetIO().DisplaySize;
        std::lock_guard<std::mutex> lk(traceMtx);

        float maxAge = cfg.trailLife + 2.f;
        traces.erase(std::remove_if(traces.begin(), traces.end(),
            [now, maxAge](const Trace& t) { return now - t.spawnTime > maxAge; }), traces.end());

        for (const auto& t : traces)
        {
            float age = now - t.spawnTime;
            float travelT = t.totalDist / cfg.bulletSpeed;
            if (travelT < 0.01f) travelT = 0.01f;
            float bulletFrac = (age / travelT < 1.f) ? age / travelT : 1.f;

            float trailAge = age - travelT;
            float trailAlpha = 1.f;
            if (trailAge > 0.f)
            {
                trailAlpha = 1.f - trailAge / cfg.trailLife;
                if (trailAlpha <= 0.f) continue;
                trailAlpha *= trailAlpha;
            }

            // Tessellate the visible portion of the trace.
            constexpr int SEG = 24;
            ImVec2 pts[SEG + 1];
            bool   ok[SEG + 1] = {};

            for (int s = 0; s <= SEG; ++s)
            {
                float f = (float)s / SEG * bulletFrac;
                float p[3] = {
                    t.start[0] + (t.end[0] - t.start[0]) * f,
                    t.start[1] + (t.end[1] - t.start[1]) * f,
                    t.start[2] + (t.end[2] - t.start[2]) * f
                };
                ok[s] = GameState::WorldToScreen(p, pts[s].x, pts[s].y, disp.x, disp.y);
            }

            const float thick = cfg.thickness;
            const int   style = (cfg.style < 0 || cfg.style >= STYLE_COUNT) ? 0 : cfg.style;

            // Per-style segment renderer.
            auto SegBright = [&](int s) -> float {
                if (!cfg.fadeFromMuzzle) return 1.0f;
                return 0.20f + 0.80f * ((float)s / SEG);
            };

            for (int s = 0; s < SEG; ++s)
            {
                if (!ok[s] || !ok[s + 1]) continue;
                float bright = SegBright(s);

                switch (style)
                {
                case STYLE_BEAM:
                {
                    float a = trailAlpha * bright * 0.86f;
                    dl->AddLine(pts[s], pts[s + 1], PackColor(cfg.color, a), thick);
                    if (cfg.glow) {
                        float ga = trailAlpha * bright * cfg.glowColor[3];
                        dl->AddLine(pts[s], pts[s + 1], PackColor(cfg.glowColor, ga),
                                    thick * cfg.glowMult);
                    }
                    break;
                }
                case STYLE_PLASMA:
                {
                    // Outer wide soft halo + saturated core + white-hot center
                    float ha = trailAlpha * bright * 0.30f;
                    dl->AddLine(pts[s], pts[s + 1], PackColor(cfg.color, ha),
                                thick * 5.5f);
                    float ca = trailAlpha * bright * 0.85f;
                    dl->AddLine(pts[s], pts[s + 1], PackColor(cfg.color, ca),
                                thick * 2.2f);
                    float cw[4] = { 1.f, 1.f, 1.f, 1.f };
                    float wa = trailAlpha * bright * 1.0f;
                    dl->AddLine(pts[s], pts[s + 1], PackColor(cw, wa), thick * 0.8f);
                    break;
                }
                case STYLE_NEON:
                {
                    // Hard line, no glow, full alpha along whole length.
                    float a = trailAlpha * (cfg.fadeFromMuzzle ? bright : 1.0f) * 0.95f;
                    dl->AddLine(pts[s], pts[s + 1], PackColor(cfg.color, a), thick);
                    float cw[4] = { 1.f, 1.f, 1.f, 1.f };
                    float wa = trailAlpha * bright * 0.55f;
                    dl->AddLine(pts[s], pts[s + 1], PackColor(cw, wa), thick * 0.4f);
                    break;
                }
                case STYLE_FIRE:
                {
                    // Hue red->orange->yellow along beam, width flickers
                    // by spawn-time hash so each shot looks different.
                    float f = (float)s / SEG;
                    float hue = 0.02f + 0.10f * f;        // 0.02=red, 0.12=yellow
                    float sat = 1.0f - 0.4f * f;
                    float val = 1.0f;
                    uint32_t h = (uint32_t)((uintptr_t)&t * 2654435761u);
                    float jitter = 0.6f + 0.8f * (float)((h >> (s & 7)) & 0xFF) / 255.f;
                    float a = trailAlpha * bright * 0.92f;
                    dl->AddLine(pts[s], pts[s + 1],
                                HsvToCol(hue, sat, val, a),
                                thick * jitter);
                    if (cfg.glow) {
                        float ga = trailAlpha * bright * 0.20f;
                        dl->AddLine(pts[s], pts[s + 1],
                                    HsvToCol(hue, sat * 0.7f, val, ga),
                                    thick * cfg.glowMult * jitter);
                    }
                    break;
                }
                case STYLE_RAINBOW:
                {
                    // Hue rotates along the beam AND with time.
                    float f = (float)s / SEG;
                    float hue = f + now * 0.35f;
                    float a = trailAlpha * bright * 0.95f;
                    dl->AddLine(pts[s], pts[s + 1],
                                HsvToCol(hue, 0.95f, 1.0f, a), thick);
                    if (cfg.glow) {
                        float ga = trailAlpha * bright * 0.25f;
                        dl->AddLine(pts[s], pts[s + 1],
                                    HsvToCol(hue, 0.7f, 1.0f, ga),
                                    thick * cfg.glowMult);
                    }
                    break;
                }
                case STYLE_TRACER_MIL:
                {
                    // Faint dim trail along most of the beam, with a
                    // bright concentrated head near the leading edge.
                    float headBoost = 1.0f;
                    float head_t = (float)(s + 1) / SEG;          // 0..1
                    float distFromHead = bulletFrac - head_t;     // >=0
                    if (distFromHead < 0.f) distFromHead = 0.f;
                    if (distFromHead < 0.18f)
                        headBoost = 1.0f + 4.5f * (1.f - distFromHead / 0.18f);
                    float a = trailAlpha * 0.55f * headBoost;
                    dl->AddLine(pts[s], pts[s + 1], PackColor(cfg.color, a),
                                thick * (0.7f + 0.5f * (headBoost > 1.f ? 1.f : 0.f)));
                    break;
                }
                case STYLE_GHOST:
                {
                    // Soft particle dots, no continuous line. Skip even
                    // segments for spacing; size scales with brightness.
                    if (s & 1) break;
                    float a = trailAlpha * bright * 0.85f;
                    dl->AddCircleFilled(pts[s + 1],
                                        thick * 1.4f * (0.6f + 0.6f * bright),
                                        PackColor(cfg.color, a));
                    if (cfg.glow) {
                        float ga = trailAlpha * bright * 0.25f;
                        dl->AddCircleFilled(pts[s + 1],
                                            thick * 3.2f, PackColor(cfg.glowColor, ga));
                    }
                    break;
                }
                case STYLE_LIGHTNING:
                {
                    // Jaggy offset polyline, perturbing endpoints by a
                    // hash-derived perpendicular offset. Per-frame
                    // re-jitter (using `now`) keeps it lively.
                    uint32_t h = (uint32_t)((uintptr_t)&t * 2654435761u
                                + (uint32_t)(now * 60.f)) ^ (uint32_t)(s * 0x9E3779B1u);
                    float jx = (((h >> 8) & 0xFF) / 255.f - 0.5f) * thick * 8.f;
                    float jy = (((h >> 16) & 0xFF) / 255.f - 0.5f) * thick * 8.f;
                    ImVec2 a0 = pts[s];
                    ImVec2 b0 = { pts[s + 1].x + jx, pts[s + 1].y + jy };
                    float al = trailAlpha * bright * 0.9f;
                    dl->AddLine(a0, b0, PackColor(cfg.color, al), thick);
                    if (cfg.glow) {
                        float ga = trailAlpha * bright * 0.30f;
                        dl->AddLine(a0, b0, PackColor(cfg.glowColor, ga),
                                    thick * cfg.glowMult);
                    }
                    break;
                }
                }
            }

            // Bullet head dot (skip for Ghost — head is already implicit).
            if (cfg.showHead && bulletFrac < 1.f && style != STYLE_GHOST)
            {
                float hp[3] = {
                    t.start[0] + (t.end[0] - t.start[0]) * bulletFrac,
                    t.start[1] + (t.end[1] - t.start[1]) * bulletFrac,
                    t.start[2] + (t.end[2] - t.start[2]) * bulletFrac
                };
                float hx, hy;
                if (GameState::WorldToScreen(hp, hx, hy, disp.x, disp.y))
                {
                    float w[4] = { 1.f, 1.f, 1.f, 1.f };
                    dl->AddCircleFilled({hx, hy}, cfg.headRadius,
                                        PackColor(w, trailAlpha));
                    if (cfg.glow) {
                        float ga = trailAlpha * 0.45f;
                        dl->AddCircleFilled({hx, hy}, cfg.headRadius * 2.5f,
                                            PackColor(cfg.glowColor, ga));
                    }
                }
            }
        }
    }
}
