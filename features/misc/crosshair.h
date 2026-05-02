// =====================================================================
// features/crosshair.h
// Static screen-center crosshair. Drawn each frame from the same place
// ESP renders. Useful when the in-game crosshair vanishes (no-scope
// AWP, etc.) so the user can confirm where the muzzle is actually
// pointed for triggerbot / no-scope work.
// =====================================================================
#pragma once
#include <imgui.h>
#include <cstdint>
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
#include "../../core/memory.h"

namespace Crosshair
{
    enum Style : int { Dot = 0, Cross = 1, TStyle = 2, Plus = 3 };

    struct Config
    {
        bool  enabled       = true;
        int   style         = Cross;
        float size          = 6.f;     // arm length in pixels
        float gap           = 3.f;     // pixels between center and arm
        float thickness     = 1.f;     // line thickness
        bool  outline       = true;    // 1px black outline for visibility
        float color[4]      = { 0.f, 1.f, 0.45f, 1.f };  // bright green
        bool  onlyWithWeapon = true;   // hide when not holding anything
        bool  hideWhenScoped = true;   // CS2 already draws its own scope reticle
        bool  dotCenter     = true;    // small filled dot in the middle
    };

    inline Config cfg;

    // Returns true if a primary/secondary firearm is currently equipped.
    // Defensive: any failure returns true so the crosshair shows by
    // default instead of mysteriously disappearing.
    inline bool HoldingGun()
    {
        std::uintptr_t pawn = GameState::GetLocalPawn();
        if (!pawn) return true;
        std::uintptr_t weapon = GameState::GetActiveWeapon(pawn);
        if (!weapon) return false;
        // We don't differentiate weapon class â€” knife/grenades/c4 still
        // show the crosshair, which matches what most cheats do.
        return true;
    }

    inline bool IsScoped()
    {
        std::uintptr_t pawn = GameState::GetLocalPawn();
        if (!pawn) return false;
        __try {
            return Mem::Read<bool>(pawn + Offsets::m_bIsScoped);
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    }

    inline void Render()
    {
        if (!cfg.enabled) return;
        if (cfg.onlyWithWeapon && !HoldingGun()) return;
        if (cfg.hideWhenScoped && IsScoped()) return;

        ImDrawList* dl = ImGui::GetForegroundDrawList();
        if (!dl) return;

        ImVec2 disp = ImGui::GetIO().DisplaySize;
        if (disp.x < 2.f || disp.y < 2.f) return;

        const float cx = floorf(disp.x * 0.5f) + 0.5f;
        const float cy = floorf(disp.y * 0.5f) + 0.5f;

        const ImU32 col = ImGui::ColorConvertFloat4ToU32(
            ImVec4(cfg.color[0], cfg.color[1], cfg.color[2], cfg.color[3]));
        const ImU32 outlineCol = IM_COL32(0, 0, 0, 200);

        const float gap   = cfg.gap;
        const float arm   = cfg.size;
        const float thick = cfg.thickness < 1.f ? 1.f : cfg.thickness;

        auto line = [&](float x1, float y1, float x2, float y2)
        {
            if (cfg.outline)
                dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), outlineCol, thick + 2.f);
            dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, thick);
        };

        switch (cfg.style)
        {
        case Dot:
            // just the center dot below
            break;
        case Cross:
            line(cx - gap - arm, cy, cx - gap, cy);   // left
            line(cx + gap,       cy, cx + gap + arm, cy); // right
            line(cx, cy - gap - arm, cx, cy - gap);   // top
            line(cx, cy + gap,       cx, cy + gap + arm); // bottom
            break;
        case TStyle:
            line(cx - gap - arm, cy, cx - gap, cy);
            line(cx + gap,       cy, cx + gap + arm, cy);
            line(cx, cy + gap,   cx, cy + gap + arm);
            break;
        case Plus:
            line(cx - arm, cy, cx + arm, cy);
            line(cx, cy - arm, cx, cy + arm);
            break;
        }

        if (cfg.dotCenter || cfg.style == Dot)
        {
            if (cfg.outline)
                dl->AddCircleFilled(ImVec2(cx, cy), 2.0f, outlineCol, 8);
            dl->AddCircleFilled(ImVec2(cx, cy), 1.2f, col, 8);
        }
    }
}
