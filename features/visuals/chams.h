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
        STYLE_COUNT
    };

    inline const char* MaterialNames[STYLE_COUNT] = {
        "Glass", "Flat + Wire", "Pearlescent", "Ghost",
        "Outline", "Flat", "Glow"
    };

    struct Config
    {
        bool enabled = false;
        int  style   = STYLE_FLAT_WIRE;
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

        // Skip viewmodel (anything with +0xB0 set is a child renderable like
        // first-person hands)
        if (Mem::Read<void*>(reinterpret_cast<uintptr_t>(obj) + 0xB0) != nullptr) {
            o_GeneratePrim(scene, obj, ctx, buf);
            return;
        }

        // Pass 1: normal (visible)
        int start = buf->m_start_primitive;
        o_GeneratePrim(scene, obj, ctx, buf);
        int end = buf->m_start_primitive;

        if (!IsPlayer(obj)) return;

        int idx = cfg.style;
        if (idx < 0 || idx >= STYLE_COUNT) idx = 0;
        MaterialSet& mat = g_materials[idx];

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

#undef H
#undef W
#undef MK
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
