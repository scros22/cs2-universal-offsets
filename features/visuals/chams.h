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
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
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
        STYLE_WATER,         // white electric/lightning arcs (friend's reference, baked color)
        // ---- tile-scroll family. Color comes from MaterialSet vis/occ
        // (routed to m_tint_color), KV3 g_vOverrideColor stays white so
        // the per-style color OR the user color picker can drive it.
        STYLE_INFERNO,       // electric arcs - default red/orange (hellfire)
        STYLE_TOXIC,         // electric arcs - default neon green (biohazard)
        STYLE_PHANTOM,       // electric arcs - default violet (ethereal)
        STYLE_GOLD,          // electric arcs - default warm gold (molten gilded)
        STYLE_STORM,         // electric arcs - giant slow single bolt (massive low-scale)
        STYLE_BUZZ,          // electric arcs - tiny fast static wash
        STYLE_MATRIX,        // digital noise - default neon green (vertical code rain)
        STYLE_CYBER,         // digital noise - default cyan (horizontal glitch sweep)
        STYLE_DATASTREAM,    // digital noise - default amber (REVERSE rising code)
        STYLE_HEX,           // digital noise - giant chunky pixels (low-scale slow)
        STYLE_STARLIGHT,     // starfield - default white-cyan (slow drift)
        STYLE_CRIMSON_STARS, // starfield - default red giant (denser, asymmetric drift)
        STYLE_COSMIC,        // starfield - default violet (sparse, near-still)
        STYLE_CONSTELLATION, // starfield - micro-stars at extreme tiling, fast diagonal
        // ---- gameplay-resident texture family (always loaded) ----
        STYLE_SPARKS,        // sparks/sparks - sharp star burst (muzzle flash texture)
        STYLE_BLEED,         // blood_spray - organic splatter
        STYLE_ARC,           // electrical_arc02 - thick lightning bolts (Zeus)
        STYLE_CRACKS,        // electrical_cracks - jagged voltage fractures
        STYLE_FLEKS,         // impact/fleks - bullet impact debris fleks (per-shot)
        STYLE_SONAR,         // particle_ring_wave - concentric radar rings
        STYLE_BURST,         // sparks_burst - radial muzzle starburst
        STYLE_PING,          // playerping_ring - target-reticle ring
        STYLE_HALO,          // particle_glow_01 - soft halo glow (per-shot)
        STYLE_BEAM,          // beam_hotwhite - hot solid energy beam
        STYLE_HEADSHOT,      // headshot/headshot - sharp kill-mark sprite
        STYLE_FLARE,         // particle_flares/aircraft_white - radiant flare
        STYLE_RAIN,          // warp_rain2 - vertical streak field
        STYLE_WORLEY,        // worley_noise_tiled - cellular voronoi noise
        STYLE_SIMPLEX,       // simplex_noise_tiled - smooth perlin-ish noise
        STYLE_SPARSE,        // sparse_noise_tiled - sparse dot pattern
        STYLE_DROPLET,       // water_drop - raindrop sprite tile
        STYLE_LEAF,          // leaf/leafdead - organic leaf-vein pattern
        STYLE_PAPER,         // paper - crumpled paper grain
        STYLE_FLEKS_GLOW,    // impact/fleks_glow - glowing debris fleks
        STYLE_TRACER,        // bullet_tracer_seq - hot streak
        // ---- SOLID material family (opaque, no additive blend) ----
        // Body is fully opaque. Differentiation is shader fresnel curve
        // + body color, not texture. Reads as a sculpted figurine made
        // of the named substance.
        STYLE_CLAY,          // matte terra cotta - no fresnel, pure body color
        STYLE_PLASTIC,       // solid color w/ subtle sheen rim (toy plastic)
        STYLE_GOLD_SOLID,    // lustrous gold body w/ hot golden fresnel rim
        STYLE_DIAMOND,       // bright crystal w/ intense rainbow rim spike
        STYLE_CHROME,        // dark mirror body w/ blue-white reflection rim
        STYLE_OBSIDIAN,      // black volcanic glass - dark body, sharp rim
        STYLE_PORCELAIN,     // off-white matte w/ soft satin rim sheen
        STYLE_RUBBER,        // deep matte black, zero fresnel (tire rubber)
        STYLE_NEON,          // pure unlit hot color - flat saturated body
        // ---- User-requested workshop / paintkit textures ----
        STYLE_ANTIQUED,      // workshop antiqued 01-tex2b paintkit
        STYLE_AVATAR,        // tournament panorama avatar texture
        STYLE_CASEHARDENED,  // AK Case Hardened albedo (set_realism_camo)
        STYLE_TITANIUM,      // Titanium Rainbow albedo (set_realism_camo)
        STYLE_COUNT
    };

    inline const char* MaterialNames[STYLE_COUNT] = {
        "Glass", "Flat + Wire", "Pearlescent", "Ghost",
        "Outline", "Flat", "Glow",
        "Velvet", "Iridescent", "Liquid Metal", "Acid",
        "Blueprint", "Ink Sketch", "Frostbite", "Galaxy",
        "Water", "Inferno", "Toxic", "Phantom", "Gold", "Storm", "Buzz",
        "Matrix", "Cyber", "Datastream", "Hex",
        "Starlight", "Crimson Stars", "Cosmic", "Constellation",
        "Sparks", "Bleed", "Arc", "Cracks",
        "Fleks", "Sonar", "Burst", "Ping", "Halo",
        "Beam", "Headshot", "Flare", "Rain", "Worley",
        "Simplex", "Sparse", "Droplet", "Leaf", "Paper",
        "Fleks Glow", "Tracer",
        "Clay", "Plastic", "Gold (Solid)", "Diamond", "Chrome",
        "Obsidian", "Porcelain", "Rubber", "Neon",
        "Antiqued", "Avatar", "Case Hardened", "Titanium Rainbow"
    };

    // First style index that belongs to the tile-scroll family. Styles
    // at or above this index honor the user color picker (cfg.useCustomColor).
    // STYLE_WATER is intentionally EXCLUDED from the picker -- it ships
    // the friend's reference snippet verbatim with baked white arcs.
    static constexpr int kFirstCustomColorStyle = STYLE_INFERNO;

    struct Config
    {
        bool enabled       = false;
        int  style         = STYLE_FLAT_WIRE;
        // Viewmodel overrides — disabled by default, when ON the chams
        // material override applies to your own viewmodel weapon / arms too.
        bool weaponChams   = false;
        bool handsChams    = false;
        // -------------------------------------------------------------------
        // User color picker -- applies only to the tile-scroll family
        // (STYLE_INFERNO and above, excluding STYLE_WATER which is the
        // verbatim reference snippet). When useCustomColor is on, the
        // per-style baked color is replaced with customColorVis (in-LOS)
        // and customColorOcc (occluded/wallhack). Alpha drives opacity.
        // -------------------------------------------------------------------
        bool  useCustomColor       = false;
        float customColorVis[4]    = { 0.10f, 0.85f, 1.00f, 1.0f }; // cyan default
        float customColorOcc[4]    = { 1.00f, 0.30f, 0.10f, 1.0f }; // hot orange default
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

        // SEH wrap the entire body. This hook runs on the scene-system
        // thread and is fired by the engine for every renderable in the
        // PVS. During map teardown / round-end / disconnect the engine
        // briefly walks objects whose backing entity has already been
        // freed -- buf->m_out, obj+0xB0, IsPlayer's chained reads can all
        // touch released memory. A single AV here brings down cs2.exe
        // because the scene-system thread is non-suspendable. Without this
        // guard end-of-game crashes were a recurring user report.
        __try {

        // Suppress shadow pass (avoid double-drawing wallhack shadows)
        if (ctx)
        {
            uint8_t* vd = Mem::Read<uint8_t*>(reinterpret_cast<uintptr_t>(ctx) + 16);
            if (vd && ((Mem::Read<uint64_t>(reinterpret_cast<uintptr_t>(vd) + 0x48) >> 24) & 1))
                return;
        }

        // Per-frame dedup
        if (!MarkSeen(obj, buf->m_out)) return;

        // NOTE: previously we tried to filter out the local-player's own
        // renderables (viewmodel/weapon) so the player could see the
        // unmodified weapon skin while teammates/enemies got chams.
        // The runtime back-ptr discovery walked too aggressively and
        // caused per-frame flicker (false positives on legit enemies
        // when the local pawn's scene node was momentarily null between
        // round transitions). Reverted to the simple pass-through path
        // until we have a more reliable owner check.

        // Pass 1: normal (visible)
        int start = buf->m_start_primitive;
        o_GeneratePrim(scene, obj, ctx, buf);
        int end = buf->m_start_primitive;

        // Players only.
        if (!IsPlayer(obj)) return;

        int idx = cfg.style;
        if (idx < 0 || idx >= STYLE_COUNT) idx = 0;
        MaterialSet& mat = g_materials[idx];

        // -------------------------------------------------------------------
        // User color override -- applies only to the tile-scroll family
        // (STYLE_INFERNO and above). Water (the verbatim friend snippet)
        // is intentionally excluded so it always reads as the reference.
        // We mutate the SHARED MaterialSet in-place; safe because each
        // frame only the active style is read, and writes are tiny float
        // copies. Apply() routes these into m_tint_color which multiplies
        // the texture sample, so any RGB picks up live.
        // -------------------------------------------------------------------
        if (cfg.useCustomColor && idx >= kFirstCustomColorStyle && idx != STYLE_WATER)
        {
            for (int c = 0; c < 4; ++c)
            {
                mat.vis_color[c] = cfg.customColorVis[c];
                mat.occ_color[c] = cfg.customColorOcc[c];
            }
        }

        // -------------------------------------------------------------------
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
            // Fast cosmic flow cycle (~8s) - the visible "movement" of the
            // galaxy comes from the tint hue-shifting across the colorful
            // acrylic-flow texture. Three-phase RGB rotation drives the
            // marbled pattern through the full cosmic spectrum:
            // violet -> magenta -> rose -> cobalt -> cyan -> back to violet.
            const float p = t * (TAU / 8.0f) + ph * 0.3f;
            // sin pair offset by 120 degrees per channel for a smooth hue
            // rotation. Bias toward deep cosmic tones (more blue/violet,
            // less red lift) so the body reads as nebula rather than a
            // pastel pink. The fresnel rim brightens regardless via
            // g_flColorBoost so the silhouette still pops.
            // Deep cosmic palette: low base + low amplitude so colors
            // stay rich and saturated rather than washing to pastel.
            // Heavy bias to violet/blue with a small magenta lift so the
            // body reads as the dim guts of a nebula, not a glowing blob.
            float vr = 0.08f + 0.22f * (0.5f + 0.5f * sinf(p));
            float vg = 0.02f + 0.10f * (0.5f + 0.5f * sinf(p + TAU * (1.0f/3.0f)));
            float vb = 0.25f + 0.30f * (0.5f + 0.5f * sinf(p + TAU * (2.0f/3.0f)));
            // Occluded: same rotation, larger phase offset so the through-
            // wall view sits ahead in the spectrum (visibly distinct).
            const float pq = p + TAU * 0.45f;
            float orC = 0.18f + 0.25f * (0.5f + 0.5f * sinf(pq));
            float ogC = 0.02f + 0.10f * (0.5f + 0.5f * sinf(pq + TAU * (1.0f/3.0f)));
            float obC = 0.30f + 0.30f * (0.5f + 0.5f * sinf(pq + TAU * (2.0f/3.0f)));
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
            tw *= 1.50f; // sparkle intensity - punches the stars on the body
            // Sparkle hue cycles at a DIFFERENT rate than the body
            // (slower, shifted) so the two layers visibly drift relative to
            // each other - that's what reads as cosmic flow / motion.
            const float sp = t * (TAU / 11.0f) + ph * 0.7f;
            float sparkle[4] = {
                0.65f + 0.35f * (0.5f + 0.5f * sinf(sp + TAU * 0.10f)),
                0.55f + 0.40f * (0.5f + 0.5f * sinf(sp + TAU * 0.43f)),
                0.85f + 0.15f * (0.5f + 0.5f * sinf(sp + TAU * 0.76f)),
                tw
            };            // Copy visible primitives and apply sparkle on top.
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
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            // Freed entity / racing teardown. Skip this primitive batch.
            // The trampoline call already happened at the top so the
            // engine still gets its normal output for the visible pass.
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
// Cosmic textures used by the Galaxy style.
//
// IMPORTANT: paintkit textures (items/assets/paintkits/...) are workshop
// assets that ONLY load when an inventory image / preview pane needs them.
// In regular gameplay they're not bound, so binding one to a player-model
// shader produces the Source 2 magenta/black "missing material" checker
// (the bug we're fixing here). The particle/tile/* textures, by contrast,
// are part of the particle system's resident set and are guaranteed to be
// resident on every map -- they're what the engine uses to draw nebula
// clouds and starfields in the actual particle effects, so we can rely on
// them being a valid bind for our chams material.
//
//   NEBULA  : tile_clouds_02 -- volumetric purple/blue nebula cloud sheet,
//             reads as a real galactic camo when tinted with our cycling
//             cosmic palette.
//   STARS   : tile_starfield -- the literal CS2 starfield tile used by the
//             skybox / particle starfield, perfect twinkling stars overlay.
#define NEBULA  "materials/particle/tile/tile_clouds_02.vtex"
#define STARS   "materials/particle/tile/tile_starfield.vtex"
#define PLASMA  "materials/particle/tile/tile_noise_plasma.vtex"
#define ELECTRIC "materials/particle/tile/tile_electric_01.vtex"
#define DIGITAL  "materials/particle/tile/tile_digital.vtex"
#define DISTORT  "materials/particle/tile/tile_distortion/tile_distortion_01.vtex"
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

        // Galaxy — high-fidelity moving cosmic camo.
        //
        // Body uses csgo_effects.vfx (same shader the other rim-lit styles
        // use, known good on skinned player models) sampling the engine's
        // resident "tile_clouds_02" nebula tile texture. The cycling
        // per-frame tint hue-shifts that nebula across the cosmic palette
        // so the body visibly flows. A soft fresnel rim adds violet edge
        // glow so the silhouette pops without going full glow-blob.
        //
        // The third slot (k14_star) is an additive csgo_effects pass with
        // the literal CS2 starfield tile, sampled at higher tiling so it
        // reads as discrete star pinpricks. Its tint cycles at a different
        // rate than the body so the two layers visibly drift relative to
        // each other -- that's the "movement" your eye reads as cosmic
        // flow.
        // Galaxy — high-fidelity moving cosmic camo.
        //
        // Body: csgo_effects.vfx on the dev white texture (W) — the SAME
        // texture every other working cham style binds, so it is always
        // resident and can never fall back to the magenta/black "missing
        // material" checker. The cosmic look comes from three layered
        // signals on top of that white base:
        //
        //   1. The hk_GeneratePrim per-frame palette cycle hue-shifts
        //      vis_color / occ_color through the deep-violet → magenta →
        //      cobalt → cyan-white spectrum, giving a continuous flow.
        //   2. A fairly tight fresnel rim (exp 3.0) so the silhouette
        //      glows brighter than the body, reading as the bright halo
        //      of a galaxy core viewed edge-on.
        //   3. The k14_star pass (csgo_effects.vfx + tile_starfield.vtex)
        //      adds the actual twinkling stars on top, additive blend, at
        //      a different hue cycle rate than the body so the two layers
        //      visibly drift relative to each other -- that drift is what
        //      reads as cosmic motion.
        //
        // Earlier revisions tried to use particle/tile/tile_clouds_02 or
        // workshop paintkit textures for the body, but those assets are
        // not always resident on regular gameplay maps and the bind would
        // silently fail, producing the magenta checker on the player body.
        // The starfield tile is reliably resident (skybox/particle uses
        // it), so the overlay pass is safe to keep.
        // Galaxy — high-fidelity moving cosmic camo.
        //
        // Texture binds use ONLY W (materials/dev/primary_white...) and MK
        // (materials/default/default_mask...) — the same two textures every
        // working cham style above uses. These are guaranteed resident in
        // every gameplay scenario. Earlier revisions tried particle/tile/*
        // assets (tile_clouds_02, tile_noise_plasma, tile_digital,
        // tile_starfield) hoping the particle system kept them resident,
        // but on regular gameplay maps those binds silently fail and the
        // engine falls back to the magenta/black "missing material"
        // checker. Using only W + MK eliminates that failure path
        // permanently.
        //
        // The cosmic look is entirely procedural and comes from three
        // signals layered on the white base:
        //   1. Per-frame palette cycle in hk_GeneratePrim hue-shifts
        //      vis_color / occ_color through the deep violet → magenta →
        //      cobalt → cyan-white spectrum on every frame, so the body
        //      visibly flows.
        //   2. A tight, hot fresnel rim (exp 4.0, max 2.4, falloff 2.5)
        //      plus colorBoost — the silhouette glows much brighter than
        //      the body, reading as the bright halo of a galaxy core
        //      viewed edge-on.
        //   3. The k14_star overlay pass uses a VERY tight high-frequency
        //      fresnel (exp 28, falloff 0.6) with high colorBoost on the
        //      same dev white texture — this turns into a sparse rim of
        //      hot pinprick highlights along the silhouette that twinkle
        //      in/out as the per-frame sparkle tint pulses, faking the
        //      starfield without needing a starfield texture.
        const char k14_vis[]  = H R"({shader="csgo_effects.vfx" g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" g_flFresnelExponent=4.0 g_flFresnelFalloff=2.5 g_flFresnelMax=2.4 g_flFresnelMin=0.35 g_flColorBoost=2.0 g_flOpacityScale=1.0 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k14_occ[]  = H R"({shader="csgo_effects.vfx" )" ZD R"(g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" g_flFresnelExponent=4.0 g_flFresnelFalloff=2.5 g_flFresnelMax=2.4 g_flFresnelMin=0.35 g_flColorBoost=1.8 g_flOpacityScale=0.95 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k14_star[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" g_flFresnelExponent=28.0 g_flFresnelFalloff=0.6 g_flFresnelMax=6.0 g_flFresnelMin=0.0 g_flColorBoost=8.0 g_flOpacityScale=1.0 F_ADDITIVE_BLEND=1 F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Water — flowing aqua/cyan camo. EXACT material from the friend's
        // reference snippet: csgo_unlitgeneric.vfx + tile_electric_01.vtex
        // with g_vTexCoordScrollSpeed driving the visible flow. The two
        // passes differ only in the Z-buffer flags (occluded disables Z so
        // it draws through walls; visible respects Z so the body shades
        // correctly in-LOS). The per-frame palette cycle in
        // hk_GeneratePrim still hue-shifts the tint across the cyan/aqua
        // spectrum, layered on top of the texture's own scrolling motion.
        const char k15_occ[]    =
            H R"({
    shader = "csgo_unlitgeneric.vfx"
    F_TRANSLUCENT = 1
    F_ADDITIVE_BLEND = 1
    F_NO_CULLING = 1
    F_UNLIT = 1
    F_DISABLE_Z_PREPASS = 1
    F_DISABLE_Z_WRITE = 1
    F_DISABLE_Z_BUFFERING = 1
    g_tColor = resource:"materials/particle/tile/tile_electric_01.vtex"
    g_vTexCoordScale = [2.0, 2.0]
    g_vTexCoordOffset = [0.0, 0.0]
    g_vTexCoordScrollSpeed = [0.45, 0.25]
    g_flFresnelExponent = 1.0
    g_flFresnelFalloff = 1.0
    g_flFresnelMax = 1.0
    g_vOverrideColor = [1.0, 1.0, 1.0, 1.0]
})";
        const char k15_vis[]    =
            H R"({
    shader = "csgo_unlitgeneric.vfx"
    F_TRANSLUCENT = 1
    F_ADDITIVE_BLEND = 1
    F_NO_CULLING = 1
    F_UNLIT = 1
    F_DISABLE_Z_PREPASS = 0
    F_DISABLE_Z_WRITE = 0
    F_DISABLE_Z_BUFFERING = 0
    g_tColor = resource:"materials/particle/tile/tile_electric_01.vtex"
    g_vTexCoordScale = [2.0, 2.0]
    g_vTexCoordOffset = [0.0, 0.0]
    g_vTexCoordScrollSpeed = [0.45, 0.25]
    g_flFresnelExponent = 1.0
    g_flFresnelFalloff = 1.0
    g_flFresnelMax = 1.0
    g_vOverrideColor = [1.0, 1.0, 1.0, 1.0]
})";
        // No third (ripple) pass -- friend's snippet is two passes only.

        // -------------------------------------------------------------------
        // Tier 3 -- "tile-scroll" family inspired by the Water/Electric
        // reference. Same shader skeleton as Water (csgo_unlitgeneric.vfx,
        // F_TRANSLUCENT + F_ADDITIVE_BLEND + F_NO_CULLING + F_UNLIT, scrolling
        // UVs, fresnel) -- only the bound texture, scroll speed, scale, and
        // override color change. Each preset gets two passes that differ
        // only in the Z-buffer flags (occluded disables Z so it draws
        // through walls; visible respects Z so the body shades in-LOS).
        //
        // All five new textures are particle/tile/* assets, the same
        // resident set as tile_electric_01 -- confirmed loadable since
        // Water binds reliably.
        // -------------------------------------------------------------------
#define WATER_LIKE(TEX, SX, SY, OFFX, OFFY, SCRX, SCRY, FE, FF, FM) \
    H "{\n" \
    "    shader = \"csgo_unlitgeneric.vfx\"\n" \
    "    F_TRANSLUCENT = 1\n" \
    "    F_ADDITIVE_BLEND = 1\n" \
    "    F_NO_CULLING = 1\n" \
    "    F_UNLIT = 1\n" \
    "    F_DISABLE_Z_PREPASS = 1\n" \
    "    F_DISABLE_Z_WRITE = 1\n" \
    "    F_DISABLE_Z_BUFFERING = 1\n" \
    "    g_tColor = resource:\"" TEX "\"\n" \
    "    g_vTexCoordScale = [" #SX ", " #SY "]\n" \
    "    g_vTexCoordOffset = [" #OFFX ", " #OFFY "]\n" \
    "    g_vTexCoordScrollSpeed = [" #SCRX ", " #SCRY "]\n" \
    "    g_flFresnelExponent = " #FE "\n" \
    "    g_flFresnelFalloff = " #FF "\n" \
    "    g_flFresnelMax = " #FM "\n" \
    "    g_vOverrideColor = [1.0, 1.0, 1.0, 1.0]\n" \
    "}"

#define WATER_LIKE_VIS(TEX, SX, SY, OFFX, OFFY, SCRX, SCRY, FE, FF, FM) \
    H "{\n" \
    "    shader = \"csgo_unlitgeneric.vfx\"\n" \
    "    F_TRANSLUCENT = 1\n" \
    "    F_ADDITIVE_BLEND = 1\n" \
    "    F_NO_CULLING = 1\n" \
    "    F_UNLIT = 1\n" \
    "    F_DISABLE_Z_PREPASS = 0\n" \
    "    F_DISABLE_Z_WRITE = 0\n" \
    "    F_DISABLE_Z_BUFFERING = 0\n" \
    "    g_tColor = resource:\"" TEX "\"\n" \
    "    g_vTexCoordScale = [" #SX ", " #SY "]\n" \
    "    g_vTexCoordOffset = [" #OFFX ", " #OFFY "]\n" \
    "    g_vTexCoordScrollSpeed = [" #SCRX ", " #SCRY "]\n" \
    "    g_flFresnelExponent = " #FE "\n" \
    "    g_flFresnelFalloff = " #FF "\n" \
    "    g_flFresnelMax = " #FM "\n" \
    "    g_vOverrideColor = [1.0, 1.0, 1.0, 1.0]\n" \
    "}"

        // -------------------------------------------------------------------
        // ELECTRIC family (tile_electric_01) -- four pattern variants.
        // Color is white in the KV3; per-style color comes from the
        // MaterialSet vis_color/occ_color (m_tint_color). The picker
        // (cfg.useCustomColor) overrides those per-frame.
        //
        // Inferno -- standard arc tiling, Water-like scroll. Default red.
        const char k16_occ[] = WATER_LIKE    ("materials/particle/tile/tile_electric_01.vtex", 2.0, 2.0, 0.0, 0.0, 0.45, 0.25, 1.0, 1.0, 1.2);
        const char k16_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_electric_01.vtex", 2.0, 2.0, 0.0, 0.0, 0.45, 0.25, 1.0, 1.0, 1.2);

        // Toxic -- thicker arcs (larger scale). Default green.
        const char k17_occ[] = WATER_LIKE    ("materials/particle/tile/tile_electric_01.vtex", 2.5, 2.5, 0.0, 0.0, 0.30, 0.18, 1.0, 1.0, 1.2);
        const char k17_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_electric_01.vtex", 2.5, 2.5, 0.0, 0.0, 0.30, 0.18, 1.0, 1.0, 1.2);

        // Phantom -- horizontal pull, slow vertical. Higher fresnel for rim halo.
        const char k18_occ[] = WATER_LIKE    ("materials/particle/tile/tile_electric_01.vtex", 1.8, 1.8, 0.0, 0.0, 0.65, 0.10, 1.0, 1.0, 1.4);
        const char k18_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_electric_01.vtex", 1.8, 1.8, 0.0, 0.0, 0.65, 0.10, 1.0, 1.0, 1.4);

        // Gold -- viscous slow scroll, hot rim. Sharper fresnel curve.
        const char k19_occ[] = WATER_LIKE    ("materials/particle/tile/tile_electric_01.vtex", 1.5, 1.5, 0.0, 0.0, 0.12, 0.08, 1.5, 1.5, 1.8);
        const char k19_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_electric_01.vtex", 1.5, 1.5, 0.0, 0.0, 0.12, 0.08, 1.5, 1.5, 1.8);

        // Storm -- giant slow single bolt. Tiny scale (0.5) = one HUGE
        // arc per body, very slow scroll so the bolt crawls dramatically.
        const char k20_occ[] = WATER_LIKE    ("materials/particle/tile/tile_electric_01.vtex", 0.5, 0.5, 0.0, 0.0, 0.05, 0.03, 1.5, 1.5, 1.6);
        const char k20_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_electric_01.vtex", 0.5, 0.5, 0.0, 0.0, 0.05, 0.03, 1.5, 1.5, 1.6);

        // Buzz -- tiny fast static. High tiling (5.0) + fast multi-axis
        // scroll = body-wide buzzing static interference field.
        const char k21_occ[] = WATER_LIKE    ("materials/particle/tile/tile_electric_01.vtex", 5.0, 5.0, 0.0, 0.0, 1.20, 0.85, 1.0, 1.0, 1.2);
        const char k21_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_electric_01.vtex", 5.0, 5.0, 0.0, 0.0, 1.20, 0.85, 1.0, 1.0, 1.2);

        // -------------------------------------------------------------------
        // DIGITAL family (tile_digital) -- four pattern variants.
        // -------------------------------------------------------------------

        // Matrix -- vertical code rain. Fast downward scroll only.
        const char k22_occ[] = WATER_LIKE    ("materials/particle/tile/tile_digital.vtex", 2.5, 2.5, 0.0, 0.0, 0.00, 0.85, 1.5, 1.5, 1.4);
        const char k22_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_digital.vtex", 2.5, 2.5, 0.0, 0.0, 0.00, 0.85, 1.5, 1.5, 1.4);

        // Cyber -- horizontal glitch sweep. Chunky, fast horizontal only.
        const char k23_occ[] = WATER_LIKE    ("materials/particle/tile/tile_digital.vtex", 1.8, 1.8, 0.0, 0.0, 0.95, 0.05, 1.0, 1.0, 1.3);
        const char k23_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_digital.vtex", 1.8, 1.8, 0.0, 0.0, 0.95, 0.05, 1.0, 1.0, 1.3);

        // Datastream -- REVERSE rising code (negative vertical scroll).
        // Reads as data uploading rather than falling. Diagonal for variety.
        const char k24_occ[] = WATER_LIKE    ("materials/particle/tile/tile_digital.vtex", 3.0, 3.0, 0.0, 0.0, -0.20, -0.65, 1.2, 1.2, 1.3);
        const char k24_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_digital.vtex", 3.0, 3.0, 0.0, 0.0, -0.20, -0.65, 1.2, 1.2, 1.3);

        // Hex -- giant chunky pixels. Tiny scale (0.6) so each "pixel"
        // covers a body region. Very slow drift = blocky data mosaic.
        const char k25_occ[] = WATER_LIKE    ("materials/particle/tile/tile_digital.vtex", 0.6, 0.6, 0.0, 0.0, 0.04, 0.04, 1.5, 1.5, 1.5);
        const char k25_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_digital.vtex", 0.6, 0.6, 0.0, 0.0, 0.04, 0.04, 1.5, 1.5, 1.5);

        // -------------------------------------------------------------------
        // STARFIELD family (tile_starfield) -- four pattern variants.
        // -------------------------------------------------------------------

        // Starlight -- standard cool starfield, slow drift.
        const char k26_occ[] = WATER_LIKE    ("materials/particle/tile/tile_starfield.vtex", 3.0, 3.0, 0.0, 0.0, 0.05, 0.05, 1.0, 1.0, 1.0);
        const char k26_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_starfield.vtex", 3.0, 3.0, 0.0, 0.0, 0.05, 0.05, 1.0, 1.0, 1.0);

        // Crimson Stars -- denser stars (high tiling), asymmetric churn.
        const char k27_occ[] = WATER_LIKE    ("materials/particle/tile/tile_starfield.vtex", 5.0, 5.0, 0.0, 0.0, 0.08, 0.03, 1.5, 1.5, 1.4);
        const char k27_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_starfield.vtex", 5.0, 5.0, 0.0, 0.0, 0.08, 0.03, 1.5, 1.5, 1.4);

        // Cosmic -- sparse stars, near-still, hot rim halo.
        const char k28_occ[] = WATER_LIKE    ("materials/particle/tile/tile_starfield.vtex", 2.0, 2.0, 0.0, 0.0, 0.02, 0.02, 2.0, 2.0, 1.8);
        const char k28_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_starfield.vtex", 2.0, 2.0, 0.0, 0.0, 0.02, 0.02, 2.0, 2.0, 1.8);

        // Constellation -- micro-stars at extreme tiling (10), fast diagonal
        // drift -- reads as a body-shaped warp tunnel of streaking stars.
        const char k29_occ[] = WATER_LIKE    ("materials/particle/tile/tile_starfield.vtex", 10.0, 10.0, 0.0, 0.0, 0.55, 0.40, 1.0, 1.0, 1.2);
        const char k29_vis[] = WATER_LIKE_VIS("materials/particle/tile/tile_starfield.vtex", 10.0, 10.0, 0.0, 0.0, 0.55, 0.40, 1.0, 1.0, 1.2);

        // -------------------------------------------------------------------
        // GAMEPLAY-RESIDENT family. These textures are referenced by
        // particle systems that fire on every shot/hit/round, so they're
        // guaranteed loaded on every gameplay map (no magenta checker).
        // -------------------------------------------------------------------

        // Sparks -- muzzle flash texture. Sharp star-burst pattern, fast
        // multi-axis scroll = body covered in shimmering muzzle flashes.
        const char k30_occ[] = WATER_LIKE    ("materials/particle/sparks/sparks.vtex", 4.0, 4.0, 0.0, 0.0, 0.85, 0.70, 1.5, 1.5, 1.6);
        const char k30_vis[] = WATER_LIKE_VIS("materials/particle/sparks/sparks.vtex", 4.0, 4.0, 0.0, 0.0, 0.85, 0.70, 1.5, 1.5, 1.6);

        // Bleed -- blood spray texture. Organic splatter, very slow drift
        // so the splatters feel painted on. Default deep red.
        const char k31_occ[] = WATER_LIKE    ("materials/particle/fluids/blood/blood_spray_low.vtex", 1.5, 1.5, 0.0, 0.0, 0.05, 0.03, 1.0, 1.0, 1.2);
        const char k31_vis[] = WATER_LIKE_VIS("materials/particle/fluids/blood/blood_spray_low.vtex", 1.5, 1.5, 0.0, 0.0, 0.05, 0.03, 1.0, 1.0, 1.2);

        // Arc -- Zeus electrical arc. Thick, dramatic lightning bolt.
        // Slow horizontal scroll = bolt slithers across the body.
        const char k32_occ[] = WATER_LIKE    ("materials/particle/electrical/electrical_arc02.vtex", 1.2, 1.2, 0.0, 0.0, 0.35, 0.05, 1.5, 1.5, 1.6);
        const char k32_vis[] = WATER_LIKE_VIS("materials/particle/electrical/electrical_arc02.vtex", 1.2, 1.2, 0.0, 0.0, 0.35, 0.05, 1.5, 1.5, 1.6);

        // Lens REMOVED -> Fleks. impact/fleks fires every bullet impact
        // so it's guaranteed resident. Sharp angular debris pattern.
        const char k33_occ[] = WATER_LIKE    ("materials/particle/electrical/electrical_cracks.vtex", 1.5, 1.5, 0.0, 0.0, 0.10, 0.05, 1.5, 1.5, 1.6);
        const char k33_vis[] = WATER_LIKE_VIS("materials/particle/electrical/electrical_cracks.vtex", 1.5, 1.5, 0.0, 0.0, 0.10, 0.05, 1.5, 1.5, 1.6);

        // Fleks -- bullet impact debris (impact_fx, per-shot resident).
        const char k34_occ[] = WATER_LIKE    ("materials/particle/impact/fleks.vtex", 3.0, 3.0, 0.0, 0.0, 0.30, 0.20, 1.5, 1.5, 1.6);
        const char k34_vis[] = WATER_LIKE_VIS("materials/particle/impact/fleks.vtex", 3.0, 3.0, 0.0, 0.0, 0.30, 0.20, 1.5, 1.5, 1.6);

        // Sonar -- concentric radar rings. Slow scale, near-still = body
        // becomes pulsing radar sweep.
        const char k35_occ[] = WATER_LIKE    ("materials/particle/particle_ring_wave.vtex", 1.5, 1.5, 0.0, 0.0, 0.04, 0.04, 1.5, 1.5, 1.5);
        const char k35_vis[] = WATER_LIKE_VIS("materials/particle/particle_ring_wave.vtex", 1.5, 1.5, 0.0, 0.0, 0.04, 0.04, 1.5, 1.5, 1.5);

        // Burst -- radial spark starburst. Sister of sparks/sparks (muzzle
        // flash). High tiling = body covered in star explosions.
        const char k36_occ[] = WATER_LIKE    ("materials/particle/sparks/sparks_burst.vtex", 3.5, 3.5, 0.0, 0.0, 0.50, 0.50, 1.5, 1.5, 1.6);
        const char k36_vis[] = WATER_LIKE_VIS("materials/particle/sparks/sparks_burst.vtex", 3.5, 3.5, 0.0, 0.0, 0.50, 0.50, 1.5, 1.5, 1.6);

        // Ping -- player-ping target reticle ring. Crisp circle pattern,
        // slow zoom-feel via low scale and minimal scroll.
        const char k37_occ[] = WATER_LIKE    ("materials/particle/playerping/playerping_ring.vtex", 1.0, 1.0, 0.0, 0.0, 0.03, 0.03, 1.5, 1.5, 1.5);
        const char k37_vis[] = WATER_LIKE_VIS("materials/particle/playerping/playerping_ring.vtex", 1.0, 1.0, 0.0, 0.0, 0.03, 0.03, 1.5, 1.5, 1.5);

        // Aura REMOVED -> Halo. particle_glow_01 is referenced by impact_fx
        // particles (per-shot). Soft body-wide halo glow.
        const char k38_occ[] = WATER_LIKE    ("materials/particle/particle_glow_01.vtex", 2.0, 2.0, 0.0, 0.0, 0.05, 0.05, 1.5, 1.5, 1.6);
        const char k38_vis[] = WATER_LIKE_VIS("materials/particle/particle_glow_01.vtex", 2.0, 2.0, 0.0, 0.0, 0.05, 0.05, 1.5, 1.5, 1.6);

        // Beam -- hot solid white energy beam. Plain bright tile, fast
        // scroll = body looks like an energy reactor.
        const char k39_occ[] = WATER_LIKE    ("materials/particle/beam_hotwhite.vtex", 2.5, 2.5, 0.0, 0.0, 0.40, 0.30, 1.5, 1.5, 1.7);
        const char k39_vis[] = WATER_LIKE_VIS("materials/particle/beam_hotwhite.vtex", 2.5, 2.5, 0.0, 0.0, 0.40, 0.30, 1.5, 1.5, 1.7);

        // Fireball REMOVED -> Headshot. headshot.vtex fires on every kill
        // (per-shot resident). Sharp kill-mark sprite tiled across body.
        const char k40_occ[] = WATER_LIKE    ("materials/particle/headshot/headshot.vtex", 2.5, 2.5, 0.0, 0.0, 0.20, 0.20, 1.5, 1.5, 1.6);
        const char k40_vis[] = WATER_LIKE_VIS("materials/particle/headshot/headshot.vtex", 2.5, 2.5, 0.0, 0.0, 0.20, 0.20, 1.5, 1.5, 1.6);

        // Flame REMOVED -> Flare. aircraft_white flare from sparks family
        // (always resident). Radiant glowing flare stamps.
        const char k41_occ[] = WATER_LIKE    ("materials/particle/particle_flares/aircraft_white.vtex", 2.0, 2.0, 0.0, 0.0, 0.25, 0.20, 1.5, 1.5, 1.7);
        const char k41_vis[] = WATER_LIKE_VIS("materials/particle/particle_flares/aircraft_white.vtex", 2.0, 2.0, 0.0, 0.0, 0.25, 0.20, 1.5, 1.5, 1.7);

        // Rain -- warp rain normal. Vertical streak field, fast downward
        // = matrix-style but on a totally different texture.
        const char k42_occ[] = WATER_LIKE    ("materials/particle/warp_rain2_normal.vtex", 2.0, 2.0, 0.0, 0.0, 0.00, 1.20, 1.0, 1.0, 1.3);
        const char k42_vis[] = WATER_LIKE_VIS("materials/particle/warp_rain2_normal.vtex", 2.0, 2.0, 0.0, 0.0, 0.00, 1.20, 1.0, 1.0, 1.3);

        // Wisp REMOVED -> Worley. worley_noise_tiled_normal is referenced
        // by muzzleflash distortion (per-shot resident). Cellular pattern.
        const char k43_occ[] = WATER_LIKE    ("materials/particle/effects/worley_noise_tiled_normal.vtex", 3.0, 3.0, 0.0, 0.0, 0.10, 0.08, 1.5, 1.5, 1.6);
        const char k43_vis[] = WATER_LIKE_VIS("materials/particle/effects/worley_noise_tiled_normal.vtex", 3.0, 3.0, 0.0, 0.0, 0.10, 0.08, 1.5, 1.5, 1.6);

        // ---- Additional per-shot guaranteed-resident styles (k44+) ----

        // Simplex -- smooth perlin-ish noise (muzzleflash distortion).
        const char k44_occ[] = WATER_LIKE    ("materials/particle/effects/simplex_noise_tiled_normal.vtex", 1.5, 1.5, 0.0, 0.0, 0.08, 0.06, 1.0, 1.0, 1.4);
        const char k44_vis[] = WATER_LIKE_VIS("materials/particle/effects/simplex_noise_tiled_normal.vtex", 1.5, 1.5, 0.0, 0.0, 0.08, 0.06, 1.0, 1.0, 1.4);

        // Sparse -- sparse-dot noise (muzzleflash distortion). Body covered
        // in scattered specks like fireflies.
        const char k45_occ[] = WATER_LIKE    ("materials/particle/effects/sparse_noise_tiled_normal.vtex", 4.0, 4.0, 0.0, 0.0, 0.12, 0.10, 1.5, 1.5, 1.6);
        const char k45_vis[] = WATER_LIKE_VIS("materials/particle/effects/sparse_noise_tiled_normal.vtex", 4.0, 4.0, 0.0, 0.0, 0.12, 0.10, 1.5, 1.5, 1.6);

        // Droplet -- raindrop sprite (water_drop, fires on water surfaces
        // and is referenced by impact splash). Drippy texture.
        const char k46_occ[] = WATER_LIKE    ("materials/particle/water_drop.vtex", 2.5, 2.5, 0.0, 0.0, 0.08, 0.30, 1.0, 1.0, 1.3);
        const char k46_vis[] = WATER_LIKE_VIS("materials/particle/water_drop.vtex", 2.5, 2.5, 0.0, 0.0, 0.08, 0.30, 1.0, 1.0, 1.3);

        // Leaf -- organic leaf-vein pattern (impact_fx for foliage hits).
        const char k47_occ[] = WATER_LIKE    ("materials/particle/leaf/leafdead.vtex", 2.0, 2.0, 0.0, 0.0, 0.05, 0.10, 1.0, 1.0, 1.3);
        const char k47_vis[] = WATER_LIKE_VIS("materials/particle/leaf/leafdead.vtex", 2.0, 2.0, 0.0, 0.0, 0.05, 0.10, 1.0, 1.0, 1.3);

        // Paper -- crumpled paper grain (impact_fx for paper surfaces).
        const char k48_occ[] = WATER_LIKE    ("materials/particle/paper/paper.vtex", 2.0, 2.0, 0.0, 0.0, 0.04, 0.04, 1.5, 1.5, 1.5);
        const char k48_vis[] = WATER_LIKE_VIS("materials/particle/paper/paper.vtex", 2.0, 2.0, 0.0, 0.0, 0.04, 0.04, 1.5, 1.5, 1.5);

        // Fleks Glow -- glowing variant of impact fleks (per-shot).
        const char k49_occ[] = WATER_LIKE    ("materials/particle/impact/fleks_glow.vtex", 3.0, 3.0, 0.0, 0.0, 0.30, 0.20, 1.5, 1.5, 1.7);
        const char k49_vis[] = WATER_LIKE_VIS("materials/particle/impact/fleks_glow.vtex", 3.0, 3.0, 0.0, 0.0, 0.30, 0.20, 1.5, 1.5, 1.7);

        // Tracer -- bullet tracer streak (every shot fires a tracer).
        // Strong horizontal bias = body looks streaked with hot lines.
        const char k50_occ[] = WATER_LIKE    ("materials/particle/effects/bullet_tracer_seq.vtex", 2.0, 2.0, 0.0, 0.0, 0.95, 0.05, 1.5, 1.5, 1.7);
        const char k50_vis[] = WATER_LIKE_VIS("materials/particle/effects/bullet_tracer_seq.vtex", 2.0, 2.0, 0.0, 0.0, 0.95, 0.05, 1.5, 1.5, 1.7);

#undef WATER_LIKE
#undef WATER_LIKE_VIS

        // -------------------------------------------------------------------
        // SOLID material family. Opaque body (no F_ADDITIVE_BLEND), driven
        // entirely by m_tint_color. Each style differs ONLY in the fresnel
        // exponent / falloff / max -- that controls how shiny vs matte the
        // body reads. csgo_effects.vfx with F_TRANSLUCENT=1 + alpha=1.0
        // renders fully opaque (no see-through) but still respects fresnel.
        // -------------------------------------------------------------------

        // Clay -- matte. Effectively zero fresnel, body is pure tint color.
        const char k51_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=0.0 g_flFresnelFalloff=1.0 g_flFresnelMax=0.0 g_flFresnelMin=0.0 g_flColorBoost=0.9 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k51_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=0.0 g_flFresnelFalloff=1.0 g_flFresnelMax=0.0 g_flFresnelMin=0.0 g_flColorBoost=0.9 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Plastic -- subtle sheen, soft rim highlight (toy plastic figurine).
        const char k52_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=4.0 g_flFresnelFalloff=2.0 g_flFresnelMax=0.5 g_flFresnelMin=0.6 g_flColorBoost=1.0 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k52_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=4.0 g_flFresnelFalloff=2.0 g_flFresnelMax=0.5 g_flFresnelMin=0.6 g_flColorBoost=1.0 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Gold Solid -- lustrous gold body, hot wide gold rim. Mid-strong fresnel.
        const char k53_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=3.0 g_flFresnelFalloff=2.0 g_flFresnelMax=2.0 g_flFresnelMin=0.7 g_flColorBoost=1.6 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k53_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=3.0 g_flFresnelFalloff=2.0 g_flFresnelMax=2.0 g_flFresnelMin=0.7 g_flColorBoost=1.6 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Diamond -- bright crystal. Very high colorBoost + intense narrow
        // rim spike = body sparkles, edges glow brilliantly.
        const char k54_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=8.0 g_flFresnelFalloff=1.2 g_flFresnelMax=4.0 g_flFresnelMin=0.9 g_flColorBoost=3.0 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k54_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=8.0 g_flFresnelFalloff=1.2 g_flFresnelMax=4.0 g_flFresnelMin=0.9 g_flColorBoost=3.0 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Chrome -- mirror metal. Dark body color (set in g_materials)
        // with a strong wide fresnel rim faking specular reflection.
        const char k55_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=2.0 g_flFresnelFalloff=2.0 g_flFresnelMax=2.5 g_flFresnelMin=0.3 g_flColorBoost=1.4 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k55_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=2.0 g_flFresnelFalloff=2.0 g_flFresnelMax=2.5 g_flFresnelMin=0.3 g_flColorBoost=1.4 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Obsidian -- volcanic glass. Near-black body, sharp tight rim.
        const char k56_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=6.0 g_flFresnelFalloff=1.5 g_flFresnelMax=1.5 g_flFresnelMin=0.25 g_flColorBoost=1.2 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k56_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=6.0 g_flFresnelFalloff=1.5 g_flFresnelMax=1.5 g_flFresnelMin=0.25 g_flColorBoost=1.2 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Porcelain -- off-white matte with very soft satin sheen.
        const char k57_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=2.5 g_flFresnelFalloff=3.0 g_flFresnelMax=0.6 g_flFresnelMin=0.7 g_flColorBoost=1.0 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k57_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=2.5 g_flFresnelFalloff=3.0 g_flFresnelMax=0.6 g_flFresnelMin=0.7 g_flColorBoost=1.0 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Rubber -- deep matte. Zero fresnel, low boost, body fully flat.
        const char k58_vis[] = H R"({shader="csgo_effects.vfx" g_flFresnelExponent=0.0 g_flFresnelFalloff=1.0 g_flFresnelMax=0.0 g_flFresnelMin=0.0 g_flColorBoost=0.7 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k58_occ[] = H R"({shader="csgo_effects.vfx" )" ZD R"(g_flFresnelExponent=0.0 g_flFresnelFalloff=1.0 g_flFresnelMax=0.0 g_flFresnelMin=0.0 g_flColorBoost=0.7 g_flOpacityScale=1.0 g_tColor=resource:")" W R"(" g_tMask1=resource:")" MK R"(" g_tMask2=resource:")" MK R"(" g_tMask3=resource:")" MK R"(" F_TRANSLUCENT=1 g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Neon -- pure unlit hot color. csgo_unlitgeneric so the body is
        // perfectly flat & saturated with no shading falloff. Reads as
        // a glowing solid (e.g. tron-suit / neon sign).
        const char k59_vis[] = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:")" W R"(" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k59_occ[] = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:")" W R"(" g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // -------------------------------------------------------------------
        // User-requested texture binds. csgo_unlitgeneric so the texture's
        // own pattern shows directly without lighting interfering. Tint is
        // white so the texture's native colors come through.
        //
        // NOTE: paths use .vtex (the engine resolves to the compiled .vtex_c
        // automatically). If a particular asset is not resident in the
        // current map's resource set the bind silently falls back to the
        // magenta/black checker -- these textures are workshop / paintkit
        // assets so residency depends on whether something currently in PVS
        // references them.
        // -------------------------------------------------------------------

        // Antiqued -- workshop antiqued paintkit (antique brass / bronze).
        const char k60_vis[] = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:"materials/models/weapons/customization/paints/antiqued/workshop/01-tex2b_tga_b4afab2a.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k60_occ[] = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:"materials/models/weapons/customization/paints/antiqued/workshop/01-tex2b_tga_b4afab2a.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Avatar -- panorama tournament avatar (UI texture). May not be
        // resident at gameplay time; will checker if not loaded.
        const char k61_vis[] = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:"panorama/images/tournaments/avatars/22/76561198246607476_png.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k61_occ[] = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:"panorama/images/tournaments/avatars/22/76561198246607476_png.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Case Hardened -- AK-47 Case Hardened albedo (mottled blue/purple/yellow steel).
        const char k62_vis[] = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:"items/assets/paintkits/set_realism_camo/aq_case_hardened_albedo_texture_psd_594c9a73.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k62_occ[] = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:"items/assets/paintkits/set_realism_camo/aq_case_hardened_albedo_texture_psd_594c9a73.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";

        // Titanium Rainbow -- shimmery titanium oil-slick rainbow albedo.
        const char k63_vis[] = H R"({shader="csgo_unlitgeneric.vfx" F_UNLIT=1 g_tColor=resource:"items/assets/paintkits/set_realism_camo/aq_titanium_rainbow_albedo_texture_psd_e57f4f8e.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";
        const char k63_occ[] = H R"({shader="csgo_unlitgeneric.vfx" )" ZD R"(F_UNLIT=1 g_tColor=resource:"items/assets/paintkits/set_realism_camo/aq_titanium_rainbow_albedo_texture_psd_e57f4f8e.vtex" g_vColorTint=[1.0,1.0,1.0,1.0]})";

#undef H
#undef W
#undef MK
#undef NEBULA
#undef STARS
#undef PLASMA
#undef ELECTRIC
#undef DIGITAL
#undef DISTORT
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
        // Water -- friend's reference snippet, two passes only (visible +
        // occluded). g_vOverrideColor inside the KV3 is [1,1,1,1], so we
        // pass white tints here too -- the texture's own scroll/scale UVs
        // produce the entire visual; no per-frame palette cycle, no ripple
        // overlay (use_wire=false, no third pass).
        g_materials[STYLE_WATER]        = { CreateMaterial("cham15_occ", k15_occ), CreateMaterial("cham15_vis", k15_vis), nullptr,
                                            {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, false };

        // Tier 3 tile-scroll family. Only confirmed-resident textures
        // (tile_electric_01, tile_digital, tile_starfield). Each preset
        // gets a unique pattern (scale/scroll/fresnel) so styles read as
        // distinct shapes, not recolors. The KV3's g_vOverrideColor is
        // white, so per-style color is driven entirely by vis_color /
        // occ_color (m_tint_color via Apply()). The user color picker
        // (cfg.useCustomColor) overrides these per-frame in hk_GeneratePrim.
        //
        // ---- ELECTRIC family (tile_electric_01) ------------------------
        g_materials[STYLE_INFERNO]       = { CreateMaterial("cham16_occ", k16_occ), CreateMaterial("cham16_vis", k16_vis), nullptr,
                                             {1.50f, 0.12f, 0.04f, 1.0f}, {1.70f, 0.45f, 0.08f, 1.0f}, false };
        g_materials[STYLE_TOXIC]         = { CreateMaterial("cham17_occ", k17_occ), CreateMaterial("cham17_vis", k17_vis), nullptr,
                                             {0.10f, 1.20f, 0.10f, 1.0f}, {0.30f, 1.80f, 0.20f, 1.0f}, false };
        g_materials[STYLE_PHANTOM]       = { CreateMaterial("cham18_occ", k18_occ), CreateMaterial("cham18_vis", k18_vis), nullptr,
                                             {1.50f, 0.10f, 0.80f, 1.0f}, {1.20f, 0.20f, 1.60f, 1.0f}, false };
        g_materials[STYLE_GOLD]          = { CreateMaterial("cham19_occ", k19_occ), CreateMaterial("cham19_vis", k19_vis), nullptr,
                                             {1.60f, 0.70f, 0.05f, 1.0f}, {1.80f, 1.20f, 0.10f, 1.0f}, false };
        g_materials[STYLE_STORM]         = { CreateMaterial("cham20_occ", k20_occ), CreateMaterial("cham20_vis", k20_vis), nullptr,
                                             {0.20f, 0.40f, 1.50f, 1.0f}, {0.40f, 0.90f, 1.80f, 1.0f}, false };
        g_materials[STYLE_BUZZ]          = { CreateMaterial("cham21_occ", k21_occ), CreateMaterial("cham21_vis", k21_vis), nullptr,
                                             {1.50f, 0.10f, 0.90f, 1.0f}, {1.80f, 0.30f, 1.20f, 1.0f}, false };

        // ---- DIGITAL family (tile_digital) -----------------------------
        g_materials[STYLE_MATRIX]        = { CreateMaterial("cham22_occ", k22_occ), CreateMaterial("cham22_vis", k22_vis), nullptr,
                                             {0.30f, 1.20f, 0.40f, 1.0f}, {0.20f, 1.80f, 0.30f, 1.0f}, false };
        g_materials[STYLE_CYBER]         = { CreateMaterial("cham23_occ", k23_occ), CreateMaterial("cham23_vis", k23_vis), nullptr,
                                             {0.05f, 0.90f, 1.40f, 1.0f}, {0.10f, 1.40f, 1.80f, 1.0f}, false };
        g_materials[STYLE_DATASTREAM]    = { CreateMaterial("cham24_occ", k24_occ), CreateMaterial("cham24_vis", k24_vis), nullptr,
                                             {1.50f, 0.55f, 0.10f, 1.0f}, {1.80f, 1.00f, 0.20f, 1.0f}, false };
        g_materials[STYLE_HEX]           = { CreateMaterial("cham25_occ", k25_occ), CreateMaterial("cham25_vis", k25_vis), nullptr,
                                             {0.40f, 1.20f, 0.10f, 1.0f}, {0.60f, 1.80f, 0.20f, 1.0f}, false };

        // ---- STARFIELD family (tile_starfield) -------------------------
        g_materials[STYLE_STARLIGHT]     = { CreateMaterial("cham26_occ", k26_occ), CreateMaterial("cham26_vis", k26_vis), nullptr,
                                             {0.80f, 1.00f, 1.50f, 1.0f}, {1.20f, 1.40f, 1.80f, 1.0f}, false };
        g_materials[STYLE_CRIMSON_STARS] = { CreateMaterial("cham27_occ", k27_occ), CreateMaterial("cham27_vis", k27_vis), nullptr,
                                             {1.50f, 0.10f, 0.10f, 1.0f}, {1.80f, 0.30f, 0.20f, 1.0f}, false };
        g_materials[STYLE_COSMIC]        = { CreateMaterial("cham28_occ", k28_occ), CreateMaterial("cham28_vis", k28_vis), nullptr,
                                             {0.60f, 0.10f, 1.20f, 1.0f}, {0.85f, 0.20f, 1.50f, 1.0f}, false };
        g_materials[STYLE_CONSTELLATION] = { CreateMaterial("cham29_occ", k29_occ), CreateMaterial("cham29_vis", k29_vis), nullptr,
                                             {0.80f, 1.00f, 1.40f, 1.0f}, {1.40f, 1.40f, 1.80f, 1.0f}, false };

        // ---- GAMEPLAY-RESIDENT family ----------------------------------
        g_materials[STYLE_SPARKS]        = { CreateMaterial("cham30_occ", k30_occ), CreateMaterial("cham30_vis", k30_vis), nullptr,
                                             {1.50f, 0.90f, 0.20f, 1.0f}, {1.80f, 1.30f, 0.40f, 1.0f}, false };
        g_materials[STYLE_BLEED]         = { CreateMaterial("cham31_occ", k31_occ), CreateMaterial("cham31_vis", k31_vis), nullptr,
                                             {1.40f, 0.05f, 0.05f, 1.0f}, {1.80f, 0.15f, 0.10f, 1.0f}, false };
        g_materials[STYLE_ARC]           = { CreateMaterial("cham32_occ", k32_occ), CreateMaterial("cham32_vis", k32_vis), nullptr,
                                             {0.30f, 0.80f, 1.50f, 1.0f}, {0.50f, 1.20f, 1.80f, 1.0f}, false };
        g_materials[STYLE_CRACKS]        = { CreateMaterial("cham33_occ", k33_occ), CreateMaterial("cham33_vis", k33_vis), nullptr,
                                             {1.20f, 0.40f, 1.50f, 1.0f}, {1.50f, 0.70f, 1.80f, 1.0f}, false };
        g_materials[STYLE_FLEKS]         = { CreateMaterial("cham34_occ", k34_occ), CreateMaterial("cham34_vis", k34_vis), nullptr,
                                             {1.30f, 0.50f, 0.10f, 1.0f}, {1.70f, 0.90f, 0.20f, 1.0f}, false };
        g_materials[STYLE_SONAR]         = { CreateMaterial("cham35_occ", k35_occ), CreateMaterial("cham35_vis", k35_vis), nullptr,
                                             {0.10f, 1.20f, 0.80f, 1.0f}, {0.20f, 1.80f, 1.20f, 1.0f}, false };
        g_materials[STYLE_BURST]         = { CreateMaterial("cham36_occ", k36_occ), CreateMaterial("cham36_vis", k36_vis), nullptr,
                                             {1.50f, 0.60f, 0.10f, 1.0f}, {1.80f, 1.00f, 0.20f, 1.0f}, false };
        g_materials[STYLE_PING]          = { CreateMaterial("cham37_occ", k37_occ), CreateMaterial("cham37_vis", k37_vis), nullptr,
                                             {0.10f, 1.40f, 0.30f, 1.0f}, {0.20f, 1.80f, 0.40f, 1.0f}, false };
        g_materials[STYLE_HALO]          = { CreateMaterial("cham38_occ", k38_occ), CreateMaterial("cham38_vis", k38_vis), nullptr,
                                             {1.20f, 0.40f, 1.50f, 1.0f}, {1.60f, 0.80f, 1.80f, 1.0f}, false };
        g_materials[STYLE_BEAM]          = { CreateMaterial("cham39_occ", k39_occ), CreateMaterial("cham39_vis", k39_vis), nullptr,
                                             {0.30f, 1.20f, 1.50f, 1.0f}, {0.60f, 1.50f, 1.80f, 1.0f}, false };
        g_materials[STYLE_HEADSHOT]      = { CreateMaterial("cham40_occ", k40_occ), CreateMaterial("cham40_vis", k40_vis), nullptr,
                                             {1.60f, 0.20f, 0.10f, 1.0f}, {1.80f, 0.40f, 0.20f, 1.0f}, false };
        g_materials[STYLE_FLARE]         = { CreateMaterial("cham41_occ", k41_occ), CreateMaterial("cham41_vis", k41_vis), nullptr,
                                             {1.40f, 1.20f, 0.30f, 1.0f}, {1.80f, 1.50f, 0.50f, 1.0f}, false };
        g_materials[STYLE_RAIN]          = { CreateMaterial("cham42_occ", k42_occ), CreateMaterial("cham42_vis", k42_vis), nullptr,
                                             {0.20f, 1.20f, 1.50f, 1.0f}, {0.40f, 1.50f, 1.80f, 1.0f}, false };
        g_materials[STYLE_WORLEY]        = { CreateMaterial("cham43_occ", k43_occ), CreateMaterial("cham43_vis", k43_vis), nullptr,
                                             {0.80f, 1.40f, 0.30f, 1.0f}, {1.20f, 1.80f, 0.50f, 1.0f}, false };
        g_materials[STYLE_SIMPLEX]       = { CreateMaterial("cham44_occ", k44_occ), CreateMaterial("cham44_vis", k44_vis), nullptr,
                                             {0.40f, 0.80f, 1.30f, 1.0f}, {0.70f, 1.20f, 1.60f, 1.0f}, false };
        g_materials[STYLE_SPARSE]        = { CreateMaterial("cham45_occ", k45_occ), CreateMaterial("cham45_vis", k45_vis), nullptr,
                                             {1.50f, 1.20f, 0.30f, 1.0f}, {1.80f, 1.60f, 0.60f, 1.0f}, false };
        g_materials[STYLE_DROPLET]       = { CreateMaterial("cham46_occ", k46_occ), CreateMaterial("cham46_vis", k46_vis), nullptr,
                                             {0.30f, 0.90f, 1.50f, 1.0f}, {0.60f, 1.30f, 1.80f, 1.0f}, false };
        g_materials[STYLE_LEAF]          = { CreateMaterial("cham47_occ", k47_occ), CreateMaterial("cham47_vis", k47_vis), nullptr,
                                             {0.20f, 1.20f, 0.30f, 1.0f}, {0.40f, 1.60f, 0.50f, 1.0f}, false };
        g_materials[STYLE_PAPER]         = { CreateMaterial("cham48_occ", k48_occ), CreateMaterial("cham48_vis", k48_vis), nullptr,
                                             {1.30f, 1.20f, 1.00f, 1.0f}, {1.60f, 1.50f, 1.30f, 1.0f}, false };
        g_materials[STYLE_FLEKS_GLOW]    = { CreateMaterial("cham49_occ", k49_occ), CreateMaterial("cham49_vis", k49_vis), nullptr,
                                             {1.50f, 0.80f, 0.20f, 1.0f}, {1.80f, 1.20f, 0.40f, 1.0f}, false };
        g_materials[STYLE_TRACER]        = { CreateMaterial("cham50_occ", k50_occ), CreateMaterial("cham50_vis", k50_vis), nullptr,
                                             {1.50f, 0.40f, 0.10f, 1.0f}, {1.80f, 0.70f, 0.20f, 1.0f}, false };

        // ---- SOLID material family --------------------------------------
        // Body color comes from vis_color/occ_color (tint multiplies the
        // dev white texture sample). Visible = in-LOS palette, Occluded =
        // wallhack palette. Most use a slightly brighter / hotter tint on
        // the occluded pass so the wallhack reads distinctly.

        // Clay -- terra cotta orange (vis), darker brick (occ).
        g_materials[STYLE_CLAY]         = { CreateMaterial("cham51_occ", k51_occ), CreateMaterial("cham51_vis", k51_vis), nullptr,
                                            {0.85f, 0.35f, 0.18f, 1.0f}, {0.95f, 0.55f, 0.30f, 1.0f}, false };
        // Plastic -- saturated toy red (vis), bright cyan (occ).
        g_materials[STYLE_PLASTIC]      = { CreateMaterial("cham52_occ", k52_occ), CreateMaterial("cham52_vis", k52_vis), nullptr,
                                            {1.00f, 0.20f, 0.20f, 1.0f}, {0.20f, 0.95f, 1.00f, 1.0f}, false };
        // Gold (Solid) -- warm rich gold both passes, occ slightly hotter.
        g_materials[STYLE_GOLD_SOLID]   = { CreateMaterial("cham53_occ", k53_occ), CreateMaterial("cham53_vis", k53_vis), nullptr,
                                            {1.40f, 0.95f, 0.20f, 1.0f}, {1.00f, 0.75f, 0.15f, 1.0f}, false };
        // Diamond -- icy white-cyan (vis), pure white sparkle (occ).
        g_materials[STYLE_DIAMOND]      = { CreateMaterial("cham54_occ", k54_occ), CreateMaterial("cham54_vis", k54_vis), nullptr,
                                            {0.85f, 0.95f, 1.00f, 1.0f}, {0.70f, 0.90f, 1.00f, 1.0f}, false };
        // Chrome -- gunmetal steel (vis), cool blue-steel (occ).
        g_materials[STYLE_CHROME]       = { CreateMaterial("cham55_occ", k55_occ), CreateMaterial("cham55_vis", k55_vis), nullptr,
                                            {0.40f, 0.50f, 0.65f, 1.0f}, {0.25f, 0.30f, 0.40f, 1.0f}, false };
        // Obsidian -- near-black with cool blue undertone (vis), violet (occ).
        g_materials[STYLE_OBSIDIAN]     = { CreateMaterial("cham56_occ", k56_occ), CreateMaterial("cham56_vis", k56_vis), nullptr,
                                            {0.10f, 0.10f, 0.18f, 1.0f}, {0.20f, 0.05f, 0.25f, 1.0f}, false };
        // Porcelain -- ivory off-white (vis), cool ceramic blue-white (occ).
        g_materials[STYLE_PORCELAIN]    = { CreateMaterial("cham57_occ", k57_occ), CreateMaterial("cham57_vis", k57_vis), nullptr,
                                            {0.95f, 0.92f, 0.85f, 1.0f}, {0.85f, 0.92f, 1.00f, 1.0f}, false };
        // Rubber -- pure matte black (vis), very dark gray (occ).
        g_materials[STYLE_RUBBER]       = { CreateMaterial("cham58_occ", k58_occ), CreateMaterial("cham58_vis", k58_vis), nullptr,
                                            {0.05f, 0.05f, 0.05f, 1.0f}, {0.10f, 0.10f, 0.10f, 1.0f}, false };
        // Neon -- electric pink (vis), neon green (occ). Pushed past 1.0
        // for HDR bloom contribution.
        g_materials[STYLE_NEON]         = { CreateMaterial("cham59_occ", k59_occ), CreateMaterial("cham59_vis", k59_vis), nullptr,
                                            {1.50f, 0.10f, 0.90f, 1.0f}, {0.20f, 1.50f, 0.30f, 1.0f}, false };

        // ---- User-requested texture binds. White tint so the texture's
        // native colors come through unmodified. The color picker still
        // works on these (multiplies the texture sample) if user enables it.
        g_materials[STYLE_ANTIQUED]     = { CreateMaterial("cham60_occ", k60_occ), CreateMaterial("cham60_vis", k60_vis), nullptr,
                                            {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, false };
        g_materials[STYLE_AVATAR]       = { CreateMaterial("cham61_occ", k61_occ), CreateMaterial("cham61_vis", k61_vis), nullptr,
                                            {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, false };
        g_materials[STYLE_CASEHARDENED] = { CreateMaterial("cham62_occ", k62_occ), CreateMaterial("cham62_vis", k62_vis), nullptr,
                                            {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, false };
        g_materials[STYLE_TITANIUM]     = { CreateMaterial("cham63_occ", k63_occ), CreateMaterial("cham63_vis", k63_vis), nullptr,
                                            {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, false };
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
