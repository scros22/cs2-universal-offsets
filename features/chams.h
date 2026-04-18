#pragma once

// ---------------------------------------------------------------
// Chams — D3D11 DrawIndexedInstanced hook with MATERIAL shaders.
// Detection: debug-name primary + vertex-stride fallback.
// Per-frame constant buffer drives animated GPU materials.
// ---------------------------------------------------------------

#include <Windows.h>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"

namespace Chams
{
    // ---------------------------------------------------------------
    // Material types — MAT_NONE means "keep original model look"
    // ---------------------------------------------------------------
    enum Material : int
    {
        MAT_NONE        = 0,   // wallhack only — original model preserved
        MAT_FLAT        = 1,   // solid tint
        MAT_CHROME      = 2,   // GTA chrome reflections
        MAT_METALLIC    = 3,   // brushed metal
        MAT_GLOW        = 4,   // pulsing emissive
        MAT_HOLOGRAM    = 5,   // scanlines + RGB fringe
        MAT_PEARLESCENT = 6,   // colour-shifting iridescence
        MAT_CRYSTAL     = 7,   // prismatic facets + sparkle
        MAT_GLASS       = 8,   // transparent fresnel
        MAT_COUNT
    };

    inline const char* MaterialNames[MAT_COUNT] = {
        "None (Original)", "Flat Color", "Chrome", "Metallic",
        "Glow", "Hologram", "Pearlescent", "Crystal", "Glass"
    };

    // Per-slot style: material + tint colour
    struct SlotStyle
    {
        int   material = MAT_FLAT;
        float color[4] = { 1.f, 1.f, 1.f, 1.f };
    };

    struct Config
    {
        bool  enabled   = false;
        bool  wallhack  = true;

        // Players (all characters — D3D11 hook can't differentiate teams)
        SlotStyle playerVis = { MAT_CHROME,  { 1.0f, 0.15f, 0.15f, 1.0f } };
        SlotStyle playerHid = { MAT_NONE,    { 1.0f, 0.5f, 0.0f, 0.55f } };

        // Viewmodel (own hands + held weapon)
        bool      handsEnabled = false;
        SlotStyle hands        = { MAT_CHROME, { 0.82f, 0.85f, 0.95f, 1.0f } };

        // World weapon drops
        bool      weaponsEnabled = false;
        SlotStyle weapons        = { MAT_METALLIC, { 1.0f, 0.95f, 0.2f, 1.0f } };
    };

    inline Config cfg;

    enum class HitType { None, Character, Hand, Weapon };

    // ---------------------------------------------------------------
    // GPU constant buffer layout  (register b12 to avoid engine clashes)
    // ---------------------------------------------------------------
    struct alignas(16) ChamsCB
    {
        float tintR, tintG, tintB, tintA;   // 16 bytes
        float screenW, screenH;             //  8
        float time;                         //  4
        float _pad;                         //  4  → 32 total
    };

    // D3D11 resources — one compiled shader per material type
    inline ID3D11PixelShader*       psSlots[MAT_COUNT] = {};
    inline ID3D11DepthStencilState* dssOff   = nullptr;
    inline ID3D11Buffer*            cbuf     = nullptr;
    inline bool                     ready    = false;
    inline float                    gTime    = 0.f;

    // WKPDID_D3DDebugObjectName
    static const GUID kDbgName =
        { 0x429b8c22, 0x9188, 0x4b0c, { 0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00 } };

    inline bool GetDebugName(ID3D11DeviceChild* obj, char* out, int maxLen)
    {
        if (!obj) return false;
        UINT sz = 0;
        if (FAILED(obj->GetPrivateData(kDbgName, &sz, nullptr)) || sz == 0 || (int)sz >= maxLen)
            return false;
        UINT sz2 = sz;
        if (FAILED(obj->GetPrivateData(kDbgName, &sz2, out)))
            return false;
        out[sz] = '\0';
        return true;
    }

    // ---------------------------------------------------------------
    // HLSL sources  (compiled as ps_4_0 for max compat)
    // MAT_NONE has no shader — hook preserves original PS
    // ---------------------------------------------------------------

    // Flat — solid tint
    static const char* kHlslFlat = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{return tint;})";

    // Chrome — multi-band reflections with bright specular streaks
    static const char* kHlslChrome = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{
  float2 uv=p.xy/screen;
  float a=sin(uv.y*30.0+uv.x*10.0+time*2.0)*0.5+0.5;
  float b=cos(uv.x*20.0-uv.y*15.0+time*1.5)*0.5+0.5;
  float c=sin((uv.x+uv.y)*35.0-time*2.8)*0.5+0.5;
  float chrome=pow(a*0.4+b*0.35+c*0.25,1.3);
  float spec=pow(saturate(sin(uv.y*60.0+time*4.0)*0.5+0.5),12.0)*0.45;
  float3 base=lerp(float3(0.07,0.07,0.10),float3(1.05,1.05,1.1),chrome)+spec;
  return float4(base*tint.rgb,tint.a);
})";

    // Metallic — brushed texture with directional highlights
    static const char* kHlslMetallic = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{
  float2 uv=p.xy/screen;
  float brush=sin(uv.y*140.0+uv.x*12.0)*0.06;
  float spec=pow(saturate(sin(uv.x*8.0+uv.y*3.0+time*0.7)*0.5+0.5),5.0)*0.35;
  float3 base=tint.rgb*(0.55+brush)+0.25+spec;
  return float4(base*tint.rgb,tint.a);
})";

    // Glow — pulsing emissive aura
    static const char* kHlslGlow = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{
  float pulse=sin(time*4.0)*0.2+0.8;
  float edge=sin(p.x*0.02+p.y*0.015+time*2.0)*0.12+0.88;
  float3 c=tint.rgb*pulse*edge*1.7;
  return float4(c,tint.a);
})";

    // Hologram — sci-fi scanlines with RGB colour fringing
    static const char* kHlslHologram = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{
  float scan=step(frac(p.y*0.15),0.5);
  float flick=sin(time*15.0)*0.08+0.92;
  float3 c;
  c.r=tint.r*(1.0+sin(p.y*0.3+time*3.0)*0.15);
  c.g=tint.g*(1.0+sin(p.y*0.3+time*3.0+2.094)*0.15);
  c.b=tint.b*(1.0+sin(p.y*0.3+time*3.0+4.189)*0.15);
  c*=flick*(0.5+scan*0.5);
  c+=float3(0.0,0.03,0.08)*scan;
  return float4(c,tint.a*(0.45+scan*0.4));
})";

    // Pearlescent — oil-slick colour shifting
    static const char* kHlslPearlescent = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{
  float2 uv=p.xy/screen;
  float shift=sin(uv.x*5.0+uv.y*3.5+time*1.2)*0.5+0.5;
  float3 c1=tint.rgb;
  float3 c2=float3(tint.z,tint.x,tint.y);
  float3 c3=float3(tint.y,tint.z,tint.x);
  float3 col=lerp(lerp(c1,c2,shift),c3,sin(shift*3.14159)*0.45);
  float gloss=pow(saturate(sin(uv.y*30.0+time*2.5)*0.5+0.5),6.0)*0.2;
  return float4(col*1.2+gloss,tint.a);
})";

    // Crystal — prismatic facets with rainbow sparkle
    static const char* kHlslCrystal = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{
  float2 uv=p.xy/screen;
  float facet=sin(uv.x*45.0)*sin(uv.y*45.0)*0.5+0.5;
  float rainbow=frac(uv.x*2.0+uv.y*1.5+time*0.3);
  float3 c;
  c.r=tint.r*(0.5+facet*0.5)+pow(saturate(1.0-abs(rainbow*3.0)),2.0)*0.25;
  c.g=tint.g*(0.5+facet*0.5)+pow(saturate(1.0-abs(rainbow*3.0-1.0)),2.0)*0.25;
  c.b=tint.b*(0.5+facet*0.5)+pow(saturate(1.0-abs(rainbow*3.0-2.0)),2.0)*0.25;
  float sparkle=pow(facet,8.0)*0.55;
  c+=sparkle;
  return float4(c,tint.a);
})";

    // Glass — frosted transparent with fresnel shimmer
    static const char* kHlslGlass = R"(
cbuffer CB:register(b12){float4 tint;float2 screen;float time;};
float4 main(float4 p:SV_Position):SV_Target{
  float2 uv=p.xy/screen;
  float fresnel=pow(abs(sin(uv.x*6.28+uv.y*3.14)),0.4)*0.5+0.3;
  float3 c=tint.rgb*(1.0+fresnel*0.55);
  float shimmer=sin(uv.y*65.0+time*5.5)*0.025;
  c+=shimmer;
  float a=tint.a*(0.22+fresnel*0.48);
  return float4(c,a);
})";

    // Indexed by Material enum  (MAT_NONE = index 0 → nullptr)
    inline const char* kShaderSources[MAT_COUNT] = {
        nullptr,             // MAT_NONE
        kHlslFlat,           // MAT_FLAT
        kHlslChrome,         // MAT_CHROME
        kHlslMetallic,       // MAT_METALLIC
        kHlslGlow,           // MAT_GLOW
        kHlslHologram,       // MAT_HOLOGRAM
        kHlslPearlescent,    // MAT_PEARLESCENT
        kHlslCrystal,        // MAT_CRYSTAL
        kHlslGlass           // MAT_GLASS
    };

    // ---------------------------------------------------------------
    // Compile a material shader (ps_4_0 for widest GPU compat)
    // ---------------------------------------------------------------
    inline HRESULT CompileMaterial(ID3D11Device* dev, int mat, ID3D11PixelShader** ps)
    {
        *ps = nullptr;
        if (mat < 0 || mat >= MAT_COUNT) return E_INVALIDARG;
        const char* src = kShaderSources[mat];
        if (!src) return S_FALSE; // MAT_NONE has no shader
        ID3DBlob* blob = nullptr;
        ID3DBlob* err  = nullptr;
        HRESULT hr = D3DCompile(src, strlen(src), "chams", nullptr, nullptr,
                                "main", "ps_4_0", 0, 0, &blob, &err);
        if (err) err->Release();
        if (FAILED(hr)) return hr;
        hr = dev->CreatePixelShader(blob->GetBufferPointer(),
                                    blob->GetBufferSize(), nullptr, ps);
        blob->Release();
        return hr;
    }

    inline void ReleaseShader(ID3D11PixelShader*& ps)
    { if (ps) { ps->Release(); ps = nullptr; } }

    inline void BuildAllShaders(ID3D11Device* dev)
    {
        for (int i = 0; i < MAT_COUNT; ++i)
        {
            ReleaseShader(psSlots[i]);
            CompileMaterial(dev, i, &psSlots[i]);
        }
    }

    inline ID3D11PixelShader* GetShader(const SlotStyle& s)
    {
        int m = s.material;
        if (m <= 0 || m >= MAT_COUNT) return nullptr; // MAT_NONE → null
        return psSlots[m];
    }

    // ---------------------------------------------------------------
    // Update constant buffer with tint + screen + time
    // ---------------------------------------------------------------
    inline void UpdateCB(ID3D11DeviceContext* ctx, const SlotStyle& s,
                         float screenW, float screenH, float t)
    {
        if (!cbuf) return;
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(ctx->Map(cbuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            auto* cb = static_cast<ChamsCB*>(mapped.pData);
            cb->tintR   = s.color[0];
            cb->tintG   = s.color[1];
            cb->tintB   = s.color[2];
            cb->tintA   = s.color[3];
            cb->screenW = screenW > 0 ? screenW : 1920.f;
            cb->screenH = screenH > 0 ? screenH : 1080.f;
            cb->time    = t;
            cb->_pad    = 0.f;
            ctx->Unmap(cbuf, 0);
        }
        ctx->PSSetConstantBuffers(12, 1, &cbuf);
    }

    // ---------------------------------------------------------------
    // Init
    // ---------------------------------------------------------------
    inline bool Init(ID3D11Device* dev)
    {
        if (ready || !dev) return ready;

        BuildAllShaders(dev);

        // Constant buffer
        D3D11_BUFFER_DESC bd = {};
        bd.ByteWidth      = sizeof(ChamsCB);
        bd.Usage           = D3D11_USAGE_DYNAMIC;
        bd.BindFlags       = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(dev->CreateBuffer(&bd, nullptr, &cbuf)))
            return false;

        // Depth-stencil disabed (wallhack pass)
        D3D11_DEPTH_STENCIL_DESC dd = {};
        dd.DepthEnable    = FALSE;
        dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dd.DepthFunc      = D3D11_COMPARISON_LESS;
        dd.StencilEnable  = FALSE;
        dd.StencilReadMask  = 0xFF;
        dd.StencilWriteMask = 0xFF;
        if (FAILED(dev->CreateDepthStencilState(&dd, &dssOff)))
            return false;

        ready = true;
        return true;
    }

    // ---------------------------------------------------------------
    // Tick — update animation timer (called from Present)
    // ---------------------------------------------------------------
    inline void Tick(ID3D11Device* dev)
    {
        if (!ready) return;
        static LARGE_INTEGER freq = {}, start = {};
        if (!freq.QuadPart)
        {
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&start);
        }
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        gTime = static_cast<float>(
            (double)(now.QuadPart - start.QuadPart) / (double)freq.QuadPart);

        // --- Force-render all enemy pawns (anti-dormancy) ---
        // The game marks far/occluded entities as "dormant" and skips
        // rendering them. By forcing m_bDormant = false every frame,
        // the client always issues draw calls for all enemy players,
        // Clear dormancy cache EVERY frame regardless of chams state.
        // When chams WH is on, we repopulate it below.
        // When chams WH is off, all zeros = aimbot trusts live scene node directly.
        memset(GameState::originalDormant, 0, sizeof(GameState::originalDormant));

        // letting our wallhack chams work at any distance on the map.
        if (cfg.enabled && cfg.wallhack && GameState::clientBase)
        {
            __try {
                uintptr_t entList = GameState::GetEntityList();
                uintptr_t localPawn = GameState::GetLocalPawn();
                if (entList && localPawn)
                {
                    int localTeam = Mem::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);

                    for (int i = 1; i <= 64; ++i)
                    {
                        uintptr_t chunk = Mem::Read<uintptr_t>(
                            entList + 8 * ((i & 0x7FFF) >> 9) + 0x10);
                        if (!chunk) continue;
                        uintptr_t ctrl = Mem::Read<uintptr_t>(
                            chunk + GameState::kEntityStride * (i & 0x1FF));
                        if (!ctrl) continue;
                        if (!Mem::Read<bool>(ctrl + Offsets::m_bPawnIsAlive)) continue;
                        uint32_t ph = Mem::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                        if (!ph) continue;
                        uintptr_t pawn = GameState::ResolveHandle(ph);
                        if (!pawn || pawn == localPawn) continue;
                        int team = Mem::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
                        if (team == localTeam) continue;

                        // Save original dormancy state for aimbot,
                        // then force scene node non-dormant so the renderer
                        // draws this entity regardless of occlusion/distance
                        uintptr_t sceneNode = Mem::Read<uintptr_t>(
                            pawn + Offsets::m_pGameSceneNode);
                        if (sceneNode)
                        {
                            GameState::originalDormant[i] = Mem::Read<bool>(sceneNode + 0x10B);
                            Mem::SmartWrite<bool>(sceneNode + 0x10B, false);
                        }
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
        }
    }

    // ---------------------------------------------------------------
    // Shutdown
    // ---------------------------------------------------------------
    inline void Shutdown()
    {
        for (auto& ps : psSlots) ReleaseShader(ps);
        if (dssOff) { dssOff->Release(); dssOff = nullptr; }
        if (cbuf)   { cbuf->Release();   cbuf   = nullptr; }
        ready = false;
    }
}
