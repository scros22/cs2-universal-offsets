#pragma once

// ---------------------------------------------------------------------------
// Chams — scenesystem-layer material override (kauht-style).
//
// Hooks CSceneAnimatableObject::GeneratePrimitives on scenesystem.dll and
// re-runs the renderer with custom KV3-built materials for the wallhack /
// visible passes. Materials are real Source 2 IMaterial2 instances created
// at init via materialsystem2 + tier0!LoadKV3, so visuals are pixel-identical
// to what shipped shaders produce — no custom HLSL.
//
// Replaces the previous D3D11 DrawIndexedInstanced hook entirely.
//
// Credit: design ported from the friend's reference implementation.
// ---------------------------------------------------------------------------

#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "../../core/memory.h"
#include "../../vendor/minhook/include/MinHook.h"

namespace Chams
{
    // -----------------------------------------------------------------------
    // Style presets (must match menu order). MaterialNames is exposed for
    // the menu's combo widget.
    // -----------------------------------------------------------------------
    enum Style : int
    {
        STYLE_GLASS = 0,
        STYLE_FLAT_WIRE,
        STYLE_PEARL,
        STYLE_GHOST,
        STYLE_OUTLINE,
        STYLE_FLAT,
        STYLE_GLOW,
        // ---- distinct second-tier styles ----
        STYLE_VELVET,        // inverted fresnel — bright body, dark rim (skin/fabric)
        STYLE_IRIDESCENT,    // tight high-frequency sheen, oil-slick rim
        STYLE_LIQUID_METAL,  // dark steel body + mirror rim, no wire
        STYLE_ACID,          // toxic neon-green additive + lime wire
        STYLE_BLUEPRINT,     // navy unlit body + cyan wire on both passes (CAD)
        STYLE_INK_SKETCH,    // white unlit body + thin black wire (paper drawing)
        STYLE_FROSTBITE,     // icy white-cyan translucent + soft white wire
        STYLE_GALAXY,        // animated cosmic nebula — purple/magenta/blue cycling rim + starlight wire
        STYLE_COUNT
    };

    inline const char* MaterialNames[STYLE_COUNT] = {
        "Glass", "Flat + Wire", "Pearlescent", "Ghost",
        "Outline", "Flat", "Glow",
        "Velvet", "Iridescent", "Liquid Metal", "Acid",
        "Blueprint", "Ink Sketch", "Frostbite", "Galaxy"
    };

    struct Config
    {
        bool enabled       = false;
        int  style         = STYLE_FLAT_WIRE;
        // Viewmodel overrides — disabled by default, when ON the chams
        // material override applies to your own viewmodel weapon / arms too.
        bool weaponChams   = false;
        bool handsChams    = false;
    };
    inline Config cfg;

    // -----------------------------------------------------------------------
    // Internal — engine types used by the hook
    // -----------------------------------------------------------------------
    struct CMaterial2 {};

    struct CMeshDrawPrimitive
    {
        char        pad_0000[32];
        CMaterial2* m_material;
        CMaterial2* m_material2;
        char        pad_0030[32];
        uint32_t    m_tint_color;
        float       m_alpha_scale;
        char        pad_0058[10];
        uint16_t    m_render_flags;
        uint16_t    m_render_flags2;
        char        pad_0066[2];
    };

    struct CMeshPrimitiveOutputBuffer
    {
        CMeshDrawPrimitive* m_out;
        int                 m_max_output_primitives;
        int                 m_start_primitive;
    };

    struct KV3_t { const char* name; uint64_t a; uint64_t b; };
    using fn_LoadKV3        = bool(__fastcall*)(void*, void*, const char*, const KV3_t*, const char*, uint32_t);
    using fn_CreateMaterial = void**(__fastcall*)(void*, void**, const char*, void*, void*, char);
    using fn_GeneratePrim   = void(__fastcall*)(void*, void*, void*, CMeshPrimitiveOutputBuffer*);

    // -----------------------------------------------------------------------
    // Style data
    // -----------------------------------------------------------------------
    struct MaterialSet
    {
        CMaterial2* occ;
        CMaterial2* vis;
        CMaterial2* wire;
        float       occ_color[4];
        float       vis_color[4];
        bool        use_wire;
    };

    inline MaterialSet g_materials[STYLE_COUNT] = {};
    inline fn_GeneratePrim o_GeneratePrim = nullptr;
    inline bool g_ready = false;

    // Per-frame deduplication so the same scene object isn't drawn twice
    inline void* g_seen[256] = {};
    inline int   g_seen_count = 0;
    inline void* g_last_buf  = nullptr;

    inline bool MarkSeen(void* obj, void* buf)
    {
        if (buf != g_last_buf) { g_last_buf = buf; g_seen_count = 0; }
        for (int i = 0; i < g_seen_count; ++i)
            if (g_seen[i] == obj) return false;
        if (g_seen_count < 256) g_seen[g_seen_count++] = obj;
        return true;
    }

    inline uint32_t PackColor(const float* c)
    {
        auto clamp = [](float v) -> uint32_t {
            if (v > 1.0f) v = 1.0f;
            if (v < 0.0f) v = 0.0f;
            return static_cast<uint32_t>(v * 255.0f);
        };
        return clamp(c[0]) | (clamp(c[1]) << 8) | (clamp(c[2]) << 16) | (clamp(c[3]) << 24);
    }

    inline void Apply(CMeshDrawPrimitive* p, CMaterial2* mat, const float* color)
    {
        if (!mat) return;
        p->m_material = p->m_material2 = mat;
        p->m_alpha_scale = color[3];
        p->m_tint_color  = PackColor(color);
    }

    inline bool IsPlayer(void* obj)
    {
        if (!obj) return false;
        // Renderable flags live at +0x80; bit 49 (0x2'0000'0000'0000) = player
        uint64_t flags = Mem::Read<uint64_t>(reinterpret_cast<uintptr_t>(obj) + 0x80);
        return (flags & 0x2000000000000ULL) != 0;
    }

    // -----------------------------------------------------------------------
    // Material creator — runs KV3 text through tier0!LoadKV3 and hands the
    // result to materialsystem2's CreateMaterial.
    // -----------------------------------------------------------------------
    inline CMaterial2* CreateMaterial(const char* name, const char* kv3_text)
    {
        HMODULE ms2 = GetModuleHandleA("materialsystem2.dll");
        HMODULE t0  = GetModuleHandleA("tier0.dll");
        if (!ms2 || !t0) return nullptr;

        auto ci = reinterpret_cast<void*(*)(const char*, int*)>(GetProcAddress(ms2, "CreateInterface"));
        if (!ci) return nullptr;
        void* ms = ci("VMaterialSystem2_001", nullptr);
        if (!ms) ms = ci("VMaterialSystem2_000", nullptr);
        if (!ms) return nullptr;

        auto load = reinterpret_cast<fn_LoadKV3>(GetProcAddress(t0,
            "?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEBDAEBUKV3ID_t@@2I@Z"));
        if (!load)
        {
            uintptr_t addr = Mem::FindPatternInModule(t0,
                "48 89 5C 24 08 57 48 83 EC 70 4C 8B D1 48 C7 C0 FF FF FF FF "
                "48 FF C0 41 80 3C 00 00 75 F6");
            if (addr) load = reinterpret_cast<fn_LoadKV3>(addr);
        }

        auto create = reinterpret_cast<fn_CreateMaterial>(Mem::FindPatternInModule(ms2,
            "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 "
            "48 81 EC 10 01 00 00 48 8B 05 ? ? ? ? 4C 8B F2 BA 90 06 00 00 "
            "49 8B D9 49 8B E8 48 8B 08 48 8B 01 FF 50 08 33 F6 48 8B F8"));

        if (!load || !create) return nullptr;

        void* buf = malloc(0x400);
        if (!buf) return nullptr;
        memset(buf, 0, 0x400);
        void* kv = reinterpret_cast<uint8_t*>(buf) + 0x100;
        KV3_t id{ "generic", 0x469806E97412167CULL, 0xE73790B53EE6F2AFULL };

        if (!load(kv, nullptr, kv3_text, &id, nullptr, 0)) { free(buf); return nullptr; }

        void* handle = nullptr;
        void* base   = nullptr;
        create(ms, &handle, name, kv, &base, 0);
        free(buf);
        if (!handle) return nullptr;
        return *reinterpret_cast<CMaterial2**>(handle);
    }

    // -----------------------------------------------------------------------
    // The hook — runs once per scene-animatable object per frame.
    // First trampoline call = visible pass; second = occluded pass.
    // -----------------------------------------------------------------------
    inline void __fastcall hk_GeneratePrim(void* scene, void* obj, void* ctx,
                                           CMeshPrimitiveOutputBuffer* buf)
    {
        if (!cfg.enabled || !g_ready || !obj || !buf) {
            o_GeneratePrim(scene, obj, ctx, buf);
            return;
        }

        // Suppress shadow pass (avoid double-drawing wallhack shadows)
        if (ctx)
        {
            uint8_t* vd = Mem::Read<uint8_t*>(reinterpret_cast<uintptr_t>(ctx) + 16);
            if (vd && ((Mem::Read<uint64_t>(reinterpret_cast<uintptr_t>(vd) + 0x48) >> 24) & 1))
                return;
        }

        // Per-frame dedup
        if (!MarkSeen(obj, buf->m_out)) return;

        // Viewmodel (your own arms + weapon) — child renderables have a non-null
        // parent at +0xB0. We can't cheaply distinguish hands from weapon at
        // the primitive level, so the two checkboxes share one effective gate:
        // either being on lets the viewmodel through.
        const bool is_viewmodel = (Mem::Read<void*>(reinterpret_cast<uintptr_t>(obj) + 0xB0) != nullptr);
        if (is_viewmodel && !cfg.weaponChams && !cfg.handsChams) {
            o_GeneratePrim(scene, obj, ctx, buf);
            return;
        }

        // Pass 1: normal (visible)
        int start = buf->m_start_primitive;
        o_GeneratePrim(scene, obj, ctx, buf);
        int end = buf->m_start_primitive;

        // Players always allowed; viewmodel allowed via the checkbox above.
        if (!is_viewmodel && !IsPlayer(obj)) return;

        int idx = cfg.style;
        if (idx < 0 || idx >= STYLE_COUNT) idx = 0;
        MaterialSet& mat = g_materials[idx];

        // -------------------------------------------------------------------
        // Galaxy live color animation. Runs only when this style is active.
        // We mutate the SHARED color arrays in g_materials[GALAXY] in-place;
        // safe because no other style ever reads them (each frame we only
        // pull the active style's set), and writes are simple float arrays.
        //
        // Three layered sine waves give a rich nebula cycle through the
        // signature Galaxy palette: deep violet -> magenta -> electric
        // blue -> cyan-white core flashes. Visible / occluded passes use
        // slightly out-of-phase palettes so the wallhack reads independently
        // of the in-LOS body -- you can see at a glance which is which.
        // -------------------------------------------------------------------
        if (idx == STYLE_GALAXY)
        {
            const float t   = (float)GetTickCount() * 0.001f;
            const float TAU = 6.2831853f;
            const uint32_t h = (uint32_t)((uintptr_t)obj * 2654435761u);
            const float    ph = (float)(h & 0xFFFF) * (TAU / 65536.0f);
            // Slow nebula cycle (~16s) - keeps the cosmic palette in deep
            // ranges (dark violet <-> indigo <-> nebula magenta accent),
            // never washes out to flat pink.
            const float p   = t * (TAU / 16.0f) + ph * 0.25f;
            float s0 = sinf(p);
            float s1 = sinf(p * 1.20f + 1.4f);
            float s2 = sinf(p * 0.85f + 2.7f);
            // Visible body: tints the cosmic texture. Floors lifted high
            // so even the dark regions of the cloud texture render as
            // colored cosmos rather than going black on multiply.
            float vr = 0.65f + 0.30f * s0 * s0;             // 0.65..0.95
            float vg = 0.30f + 0.30f * s1 * s1;             // 0.30..0.60  enough to keep texture detail
            float vb = 0.85f + 0.15f * s2 * s2;             // 0.85..1.00  blue-dominant
            // Occluded: brighter / phase-shifted so wallhack reads.
            float o0 = sinf(p + 1.7f);
            float o1 = sinf(p * 1.10f + 3.4f);
            float o2 = sinf(p * 0.90f + 0.6f);
            float orC = 0.75f + 0.25f * o0 * o0;            // 0.75..1.00
            float ogC = 0.30f + 0.30f * o1 * o1;            // 0.30..0.60
            float obC = 0.90f + 0.10f * o2 * o2;            // 0.90..1.00
            // No supernova flash - it just washed the body to white. The
            // sparkle pass below is what provides the star highlights.
            mat.vis_color[0] = vr;  mat.vis_color[1] = vg;  mat.vis_color[2] = vb;  mat.vis_color[3] = 1.0f;
            mat.occ_color[0] = orC; mat.occ_color[1] = ogC; mat.occ_color[2] = obC; mat.occ_color[3] = 1.0f;
        }

        for (int i = start; i < end; ++i)
            Apply(&buf->m_out[i], mat.vis, mat.vis_color);

        // Pass 2: occluded
        int occ_start = buf->m_start_primitive;
        o_GeneratePrim(scene, obj, ctx, buf);
        int occ_end = buf->m_start_primitive;

        for (int i = occ_start; i < occ_end; ++i)
            Apply(&buf->m_out[i], mat.occ, mat.occ_color);

        // Optional wireframe overlay (occluded only)
        if (mat.use_wire && mat.wire)
        {
            int occ_count  = occ_end - occ_start;
            int space_left = buf->m_max_output_primitives - buf->m_start_primitive;
            int to_copy    = (occ_count < space_left) ? occ_count : space_left;
            if (to_copy > 0)
            {
                int wire_start = buf->m_start_primitive;
                float wire_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                for (int i = 0; i < to_copy; ++i)
                {
                    buf->m_out[wire_start + i] = buf->m_out[occ_start + i];
                    Apply(&buf->m_out[wire_start + i], mat.wire, wire_color);
                }
                buf->m_start_primitive += to_copy;
            }
        }

        // -------------------------------------------------------------------
        // Galaxy sparkle / "stars" overlay. We re-use the wire slot to hold
        // a high-frequency additive fresnel material (k14_star). It's
        // copied on TOP of the visible primitives so the star pinpricks
        // shimmer along the body silhouette \u2014 not a wireframe pattern.
        //
        // Color is animated very fast (multi-Hz twinkle) and offset by the
        // per-object hash so different bodies twinkle independently. The
        // tint cycles white \u2192 cyan \u2192 light-violet to feel like real
        // stars rather than a single solid glow.
        // -------------------------------------------------------------------
        if (idx == STYLE_GALAXY && mat.wire)
        {
            const float t   = (float)GetTickCount() * 0.001f;
            const float TAU = 6.2831853f;
            const uint32_t h = (uint32_t)((uintptr_t)obj * 2654435761u);
            const float ph   = (float)(h & 0xFFFF) * (TAU / 65536.0f);
            // Fast twinkle (~2.5 Hz) with a second harmonic for a
            // shimmering, non-uniform sparkle intensity.
            float tw = 0.55f + 0.45f * sinf(t * (TAU * 2.5f) + ph)
                             * sinf(t * (TAU * 1.3f) + ph * 1.7f);
            if (tw < 0.0f) tw = 0.0f;
            tw *= 1.20f; // sparkle intensity - texture-based star pinpricks need extra punch
            // Slight tint shift so sparkles aren't just flat white.
            float ti = sinf(t * (TAU / 3.0f) + ph);
            float sparkle[4] = {
                0.85f + 0.15f * ti,            // R: white\u2194pink
                0.95f,                          // G: high
                1.00f,                          // B: full blue
                tw                              // alpha = twinkle intensity
            };

            // Copy visible primitives and apply sparkle on top.
            int vis_count   = end - start;
            int space_left  = buf->m_max_output_primitives - buf->m_start_primitive;
            int to_copy_v   = (vis_count < space_left) ? vis_count : space_left;
            if (to_copy_v > 0)
            {
                int sp_start = buf->m_start_primitive;
                for (int i = 0; i < to_copy_v; ++i)
                {
                    buf->m_out[sp_start + i] = buf->m_out[start + i];
                    Apply(&buf->m_out[sp_start + i], mat.wire, sparkle);
                }
                buf->m_start_primitive += to_copy_v;
            }
            // Copy occluded primitives too, dimmer, so sparkle reads through walls.
            int occ_count2 = occ_end - occ_start;
            space_left     = buf->m_max_output_primitives - buf->m_start_primitive;
            int to_copy_o  = (occ_count2 < space_left) ? occ_count2 : space_left;
            if (to_copy_o > 0)
            {
                int sp_start = buf->m_start_primitive;
                float sparkle_occ[4] = { sparkle[0], sparkle[1], sparkle[2], sparkle[3] * 0.6f };
                for (int i = 0; i < to_copy_o; ++i)
                {
                    buf->m_out[sp_start + i] = buf->m_out[occ_start + i];
                    Apply(&buf->m_out[sp_start + i], mat.wire, sparkle_occ);
                }
                buf->m_start_primitive += to_copy_o;
            }
        }
    }

    // -----------------------------------------------------------------------
    // Best-effort: flip the engine PVS manager off so chams render at any
    // distance. Failure is non-fatal — visuals still work for visible PVS.
    // -----------------------------------------------------------------------
    inline void DisablePvs()
    {
        HMODULE engine2 = GetModuleHandleA("engine2.dll");
        if (!engine2) return;
        uintptr_t pvs = Mem::FindPatternInModule(engine2, "48 8D 0D ? ? ? ? 33 D2 FF 50");
        if (!pvs) return;
        void* mgr = reinterpret_cast<void*>(pvs + 7 + Mem::Read<int32_t>(pvs + 3));
        void** vt = Mem::Read<void**>(reinterpret_cast<uintptr_t>(mgr));
        if (!vt) return;
        __try {
            reinterpret_cast<void(__fastcall*)(void*, bool)>(vt[6])(mgr, false);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // -----------------------------------------------------------------------
    // Build the 7 KV3 material presets.
    // -----------------------------------------------------------------------
    inline void BuildMaterials()
    {
#define H  "<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->\n"
#define W  "materials/dev/primary_white_color_tga_21186c76.vtex"
#define MK "materials/default/default_mask_tga_fde710a5.vtex"
// Cosmic textures used by the Galaxy style. Effect-domain textures are
// guaranteed runtime-loadable on player models (particle/tile textures are
// only loaded into the particle atlas).
#define NEBULA  "materials/effects/smoke/cloud_003_mask_psd_3fa6a080.vtex"
#define GLITTER "materials/effects/squares_glitter_color_psd_8d090324.vtex"
#define GLITNRM "materials/effects/squares_glitter_normal_tga_d78920fb.vtex"
#define ZD "    F_DISABLE_Z_BUFFERING = 1\n    F_DISABLE_Z_PREPASS = 1\n    F_DISABLE_Z_WRITE = 1\n"

        // Glass
        const char k0_vis[] = H R"({shader="csgo_effects.vfx" g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" g_flFresnelExponent=0.75 g_flFresnelFalloff=1.0 g_flFresnelMax=0.0 g_flFresnelMin=1.0 F_ADDITIVE_BLEND=1 F_ALPHA_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k0_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" g_flFresnelExponent=0.75 g_flFresnelFalloff=1.0 g_flFresnelMax=0.0 g_flFresnelMin=1.0 F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Flat + Wire
        const char k1_vis[]  = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_vColorTint=[1.0,1.0,1.0,1.0] g_tColor=resource:")" W R"("})";
        const char k1_occ[]  = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_vColorTint=[1.0,1.0,1.0,1.0] g_tColor=resource:")" W R"("})";
        const char k1_wire[] = H R"({shader="tools_wireframe.vfx" )" ZD R"(F_UNLIT=1 F_WIREFRAME=1 g_DepthBiasAmount=0.0 g_LineThickness=0.25 g_OverrideColorFactor=1.0 g_vOverrideColor=[1.0,1.0,1.0,1.0]})";

        // Pearl
        const char k2_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=2.0 g_flFresnelFalloff=3.0 g_flFresnelMax=1.0 g_flFresnelMin=0.2 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k2_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=2.0 g_flFresnelFalloff=3.0 g_flFresnelMax=1.0 g_flFresnelMin=0.2 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Ghost
        const char k3_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=3.0 g_flFresnelFalloff=5.0 g_flFresnelMax=1.0 g_flFresnelMin=0.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k3_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=3.0 g_flFresnelFalloff=5.0 g_flFresnelMax=1.0 g_flFresnelMin=0.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Outline
        const char k4_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=15.0 g_flFresnelFalloff=1.0 g_flFresnelMax=2.0 g_flFresnelMin=0.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k4_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=15.0 g_flFresnelFalloff=1.0 g_flFresnelMax=2.0 g_flFresnelMin=0.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Flat
        const char k5[] = H R"({shader="csgo_unlitgeneric.vfx" g_tColor=resource:")" W R"(" F_IGNOREZ=1 F_DISABLE_Z_WRITE=1 F_DISABLE_Z_BUFFERING=1 F_RENDER_BACKFACES=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Glow
        const char k6[] = H R"({shader="csgo_effects.vfx" g_tColor=resource:")" W R"(" g_flColorBoost=20.0 g_flOpacityScale=0.7 g_flFresnelExponent=10.0 g_flFresnelFalloff=10.0 g_flFresnelMax=0.0 g_flFresnelMin=1.0 F_ADDITIVE_BLEND=1 F_BLEND_MODE=1 F_TRANSLUCENT=1 F_IGNOREZ=1 F_DISABLE_Z_BUFFERING=1 F_RENDER_BACKFACES=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // -------------------------------------------------------------------
        // Tier 2 — distinct styles, each visually unmistakable.
        // -------------------------------------------------------------------

        // Velvet — INVERTED fresnel (min > max): body lit, rim darkens.
        // Reads like soft fabric / skin SSS. No wire.
        const char k7_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=2.0 g_flFresnelFalloff=2.0 g_flFresnelMax=0.0 g_flFresnelMin=1.6 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k7_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=2.0 g_flFresnelFalloff=2.0 g_flFresnelMax=0.0 g_flFresnelMin=1.6 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Iridescent — very tight high-frequency rim sheen (oil-slick).
        // Hot multi-color spike right at the silhouette. No wire.
        const char k8_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=18.0 g_flFresnelFalloff=1.5 g_flFresnelMax=4.5 g_flFresnelMin=0.05 g_flColorBoost=3.5 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k8_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=14.0 g_flFresnelFalloff=2.0 g_flFresnelMax=3.5 g_flFresnelMin=0.0 g_flColorBoost=2.5 g_flOpacityScale=0.9 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Liquid Metal — dark satin body (vis_color near-black) with a
        // bright fresnel rim acting as the spec highlight. No wire.
        const char k9_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=4.0 g_flFresnelFalloff=2.5 g_flFresnelMax=2.5 g_flFresnelMin=0.05 g_flColorBoost=1.4 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k9_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=4.0 g_flFresnelFalloff=2.5 g_flFresnelMax=2.5 g_flFresnelMin=0.05 g_flColorBoost=1.4 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Acid — toxic neon-green additive with mid bloom + lime wire on occluded.
        const char k10_vis[]  = H R"({shader="csgo_effects.vfx" g_flColorBoost=6.0 g_flOpacityScale=0.85 g_flFresnelExponent=2.0 g_flFresnelFalloff=2.0 g_flFresnelMax=2.0 g_flFresnelMin=0.6 g_tColor=resource:")" W R"(" F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k10_occ[]  = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flColorBoost=9.0 g_flOpacityScale=0.7 g_flFresnelExponent=3.0 g_flFresnelFalloff=3.0 g_flFresnelMax=2.5 g_flFresnelMin=0.4 g_tColor=resource:")" W R"(" F_ADDITIVE_BLEND=1 F_BLEND_MODE=1 F_TRANSLUCENT=1 F_IGNOREZ=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k10_wire[] = H R"({shader="tools_wireframe.vfx" )" ZD R"(F_UNLIT=1 F_WIREFRAME=1 g_LineThickness=0.22 g_OverrideColorFactor=1.0 g_vOverrideColor=[0.7,1.0,0.0,1.0]})";

        // Blueprint — dark-navy unlit body + bright cyan wire on BOTH passes.
        // Reads like a CAD/blueprint diagram.
        const char k11_vis[]  = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:")" W R"(" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k11_occ[]  = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:")" W R"(" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k11_wire[] = H R"({shader="tools_wireframe.vfx" F_UNLIT=1 F_WIREFRAME=1 g_LineThickness=0.28 g_OverrideColorFactor=1.0 g_vOverrideColor=[0.35,0.85,1.0,1.0]})";

        // Ink Sketch — pure white unlit body + thin black wire (negative of Toon).
        // Crisp paper-drawing look.
        const char k12_vis[]  = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:")" W R"(" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k12_occ[]  = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:")" W R"(" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k12_wire[] = H R"({shader="tools_wireframe.vfx" F_UNLIT=1 F_WIREFRAME=1 g_LineThickness=0.18 g_OverrideColorFactor=1.0 g_vOverrideColor=[0.0,0.0,0.0,1.0]})";

        // Frostbite — pale cyan-white translucent body w/ soft fresnel + thin white wire.
        // Genuinely icy, not glowy.
        const char k13_vis[]  = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=3.5 g_flFresnelFalloff=2.0 g_flFresnelMax=1.6 g_flFresnelMin=0.5 g_flColorBoost=1.6 g_flOpacityScale=0.9 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k13_occ[]  = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=3.0 g_flFresnelFalloff=2.0 g_flFresnelMax=1.4 g_flFresnelMin=0.3 g_flColorBoost=1.4 g_flOpacityScale=0.7 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k13_wire[] = H R"({shader="tools_wireframe.vfx" )" ZD R"(F_UNLIT=1 F_WIREFRAME=1 g_LineThickness=0.16 g_OverrideColorFactor=1.0 g_vOverrideColor=[0.95,1.0,1.0,1.0]})";

        // Galaxy — the marquee animated style. Uses CS2's shipped
        // tile_clouds_02 (nebula) particle texture as the body so the
        // silhouette reads as a real cosmic camo pattern instead of a
        // flat tinted blob. csgo_unlitgeneric.vfx is used for the body
        // (it actually samples g_tColor — csgo_effects largely ignores
        // it). The cycling per-frame tint then hue-shifts the nebula
        // pattern, giving the moving "galaxy camo" look. The third slot
        // is an additive csgo_effects pass with the literal starfield
        // texture for twinkling stars on top.
        const char k14_vis[]  = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:")" NEBULA R"(" g_vColorTint=[1.0,1.0,1.0,1.0] g_vTexCoordScale=[4.0,4.0] g_vTexCoordScrollSpeed=[0.08,0.13]})";
        const char k14_occ[]  = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:")" NEBULA R"(" g_vColorTint=[1.0,1.0,1.0,1.0] g_vTexCoordScale=[4.0,4.0] g_vTexCoordScrollSpeed=[0.08,0.13]})";
        // Sparkle/star pass: glitter texture additively layered on top.
        // Z-disabled so it shows through walls along with the wallhack body.
        // Fast scroll in opposite direction to the body for parallax — the
        // star field clearly drifts across the player as they move.
        const char k14_star[] = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_tColor=resource:")" GLITTER R"(" g_vColorTint=[1.0,1.0,1.0,1.0] g_vTexCoordScale=[6.0,6.0] g_vTexCoordScrollSpeed=[-0.18,0.22]})";

#undef H
#undef W
#undef MK
#undef NEBULA
#undef GLITTER
#undef GLITNRM
#undef ZD

        g_materials[STYLE_GLASS]     = { CreateMaterial("cham0_occ", k0_occ), CreateMaterial("cham0_vis", k0_vis), nullptr,
                                         {1.0f,0.3f,0.3f,0.5f}, {0.3f,0.7f,1.0f,0.5f}, false };
        g_materials[STYLE_FLAT_WIRE] = { CreateMaterial("cham1_occ", k1_occ), CreateMaterial("cham1_vis", k1_vis), CreateMaterial("cham1_wire", k1_wire),
                                         {0.0f,0.0f,0.0f,1.0f}, {0.0f,0.0f,0.0f,0.0f}, true };
        g_materials[STYLE_PEARL]     = { CreateMaterial("cham2_occ", k2_occ), CreateMaterial("cham2_vis", k2_vis), nullptr,
                                         {1.0f,0.6f,0.0f,1.0f}, {0.5f,1.0f,1.0f,1.0f}, false };
        g_materials[STYLE_GHOST]     = { CreateMaterial("cham3_occ", k3_occ), CreateMaterial("cham3_vis", k3_vis), nullptr,
                                         {0.0f,1.0f,0.5f,1.0f}, {0.8f,0.0f,1.0f,1.0f}, false };
        g_materials[STYLE_OUTLINE]   = { CreateMaterial("cham4_occ", k4_occ), CreateMaterial("cham4_vis", k4_vis), nullptr,
                                         {1.0f,1.0f,0.0f,1.0f}, {0.0f,1.0f,1.0f,1.0f}, false };
        g_materials[STYLE_FLAT]      = { CreateMaterial("cham5_occ", k5),     CreateMaterial("cham5_vis", k5),     nullptr,
                                         {1.0f,0.0f,0.0f,1.0f}, {0.0f,1.0f,0.0f,1.0f}, false };
        g_materials[STYLE_GLOW]      = { CreateMaterial("cham6_occ", k6),     CreateMaterial("cham6_vis", k6),     nullptr,
                                         {0.0f,1.0f,1.0f,1.0f}, {1.0f,1.0f,0.0f,1.0f}, false };

        // ---- distinct second-tier styles ----
        // Velvet — warm raspberry body fading to dark rim (vis), violet (occ).
        g_materials[STYLE_VELVET]       = { CreateMaterial("cham7_occ",  k7_occ),  CreateMaterial("cham7_vis",  k7_vis),  nullptr,
                                            {0.55f, 0.10f, 0.45f, 1.0f}, {1.00f, 0.35f, 0.55f, 1.0f}, false };
        // Iridescent — emerald shimmer (vis), magenta sheen (occ).
        g_materials[STYLE_IRIDESCENT]   = { CreateMaterial("cham8_occ",  k8_occ),  CreateMaterial("cham8_vis",  k8_vis),  nullptr,
                                            {0.75f, 0.20f, 1.00f, 1.0f}, {0.20f, 1.00f, 0.55f, 1.0f}, false };
        // Liquid Metal — cool charcoal (vis), warm gunmetal (occ). Body dark,
        // fresnel rim does the lighting.
        g_materials[STYLE_LIQUID_METAL] = { CreateMaterial("cham9_occ",  k9_occ),  CreateMaterial("cham9_vis",  k9_vis),  nullptr,
                                            {0.18f, 0.16f, 0.22f, 1.0f}, {0.10f, 0.10f, 0.12f, 1.0f}, false };
        // Acid — toxic green throughout, lime wire on occluded only.
        g_materials[STYLE_ACID]         = { CreateMaterial("cham10_occ", k10_occ), CreateMaterial("cham10_vis", k10_vis), CreateMaterial("cham10_wire", k10_wire),
                                            {0.30f, 1.00f, 0.05f, 1.0f}, {0.55f, 1.00f, 0.10f, 1.0f}, true };
        // Blueprint — navy body + cyan wire on both passes.
        g_materials[STYLE_BLUEPRINT]    = { CreateMaterial("cham11_occ", k11_occ), CreateMaterial("cham11_vis", k11_vis), CreateMaterial("cham11_wire", k11_wire),
                                            {0.04f, 0.08f, 0.22f, 1.0f}, {0.08f, 0.14f, 0.32f, 1.0f}, true };
        // Ink Sketch — pure white body + thin black wire.
        g_materials[STYLE_INK_SKETCH]   = { CreateMaterial("cham12_occ", k12_occ), CreateMaterial("cham12_vis", k12_vis), CreateMaterial("cham12_wire", k12_wire),
                                            {1.00f, 1.00f, 1.00f, 1.0f}, {0.85f, 0.88f, 0.92f, 1.0f}, true };
        // Frostbite — pale ice (vis), deeper ice-blue (occ), soft white wire.
        g_materials[STYLE_FROSTBITE]    = { CreateMaterial("cham13_occ", k13_occ), CreateMaterial("cham13_vis", k13_vis), CreateMaterial("cham13_wire", k13_wire),
                                            {0.55f, 0.85f, 1.00f, 1.0f}, {0.85f, 0.95f, 1.00f, 0.95f}, true };
        // Galaxy — deep cosmic violet/blue base; vis_color / occ_color get
        // overwritten per-frame in hk_GeneratePrim with the cycling nebula
        // palette. The third slot holds the sparkle ("stars") material that
        // is rendered as an extra additive pass; use_wire stays FALSE so the
        // generic occluded-wire overlay code path is skipped.
        g_materials[STYLE_GALAXY]       = { CreateMaterial("cham14_occ", k14_occ), CreateMaterial("cham14_vis", k14_vis), CreateMaterial("cham14_star", k14_star),
                                            {0.30f, 0.05f, 0.55f, 1.0f}, {0.45f, 0.15f, 0.95f, 1.0f}, false };
    }

    // -----------------------------------------------------------------------
    // Public API
    //   Setup()    — one-shot installer (call after scenesystem/material/tier0
    //                are loaded; safe to retry)
    //   Shutdown() — best-effort unhook
    // -----------------------------------------------------------------------
    inline bool Setup()
    {
        if (g_ready) return true;

        HMODULE scenesystem = GetModuleHandleA("scenesystem.dll");
        if (!scenesystem) return false;
        if (!GetModuleHandleA("materialsystem2.dll")) return false;
        if (!GetModuleHandleA("tier0.dll"))           return false;

        DisablePvs(); // best-effort

        BuildMaterials();
        if (!g_materials[0].vis || !g_materials[0].occ) return false;

        uintptr_t target = Mem::FindPatternInModule(scenesystem,
            "48 8B C4 48 89 58 08 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 "
            "48 81 EC ? ? ? ?");
        if (!target) return false;

        if (MH_CreateHook(reinterpret_cast<void*>(target),
                          reinterpret_cast<void*>(&hk_GeneratePrim),
                          reinterpret_cast<void**>(&o_GeneratePrim)) != MH_OK)
            return false;
        if (MH_EnableHook(reinterpret_cast<void*>(target)) != MH_OK)
            return false;

        g_ready = true;
        return true;
    }

    inline void Shutdown()
    {
        if (!g_ready) return;
        // o_GeneratePrim points at the trampoline; safest disable-all on exit
        MH_DisableHook(MH_ALL_HOOKS);
        g_ready = false;
    }
}
