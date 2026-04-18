#pragma once
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include "../sdk/game.h"
#include "../sdk/offsets.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace Chams
{
    enum Material : int
    {
        MAT_NONE        = 0,
        MAT_FLAT        = 1,
        MAT_CHROME      = 2,
        MAT_METALLIC    = 3,
        MAT_GLOW        = 4,
        MAT_HOLOGRAM    = 5,
        MAT_PEARLESCENT = 6,
        MAT_CRYSTAL     = 7,
        MAT_GLASS       = 8,
        MAT_COUNT
    };

    inline const char* MaterialNames[MAT_COUNT] = {
        "None (Original)", "Flat Color", "Chrome", "Metallic",
        "Glow", "Hologram", "Pearlescent", "Crystal", "Glass"
    };

    struct SlotStyle
    {
        int   material = MAT_FLAT;
        float color[4] = { 1.f, 1.f, 1.f, 1.f };
    };

    struct Config
    {
        bool  enabled   = false;
        bool  wallhack  = true;
        SlotStyle playerVis = { MAT_CHROME,  { 1.0f, 0.15f, 0.15f, 1.0f } };
        SlotStyle playerHid = { MAT_FLAT,    { 1.0f, 0.5f, 0.0f, 0.55f } };
        bool      handsEnabled = false;
        SlotStyle hands        = { MAT_CHROME, { 0.82f, 0.85f, 0.95f, 1.0f } };
        bool      weaponsEnabled = false;
        SlotStyle weapons        = { MAT_METALLIC, { 1.0f, 0.95f, 0.2f, 1.0f } };
    };

    inline Config cfg;

    struct alignas(16) ChamsCB
    {
        float tintR, tintG, tintB, tintA;
        float screenW, screenH;
        float time;
        float _pad;
    };

    inline ID3D11PixelShader*       psSlots[MAT_COUNT] = {};
    inline ID3D11DepthStencilState* dssOff   = nullptr;
    inline ID3D11Buffer*            cbuf     = nullptr;
    inline bool                     ready    = false;
    inline float                    gTime    = 0.f;

    // --- Shaders (User Provided) ---
    static const char* kHlslFlat = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{return tint;}";
    static const char* kHlslChrome = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{ float2 uv=p.xy/screen; float a=sin(uv.y*30.0+uv.x*10.0+time*2.0)*0.5+0.5; float b=cos(uv.x*20.0-uv.y*15.0+time*1.5)*0.5+0.5; float c=sin((uv.x+uv.y)*35.0-time*2.8)*0.5+0.5; float chrome=pow(a*0.4+b*0.35+c*0.25,1.3); float spec=pow(saturate(sin(uv.y*60.0+time*4.0)*0.5+0.5),12.0)*0.45; float3 base=lerp(float3(0.07,0.07,0.10),float3(1.05,1.05,1.1),chrome)+spec; return float4(base*tint.rgb,tint.a); }";
    static const char* kHlslMetallic = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{ float2 uv=p.xy/screen; float brush=sin(uv.y*140.0+uv.x*12.0)*0.06; float spec=pow(saturate(sin(uv.x*8.0+uv.y*3.0+time*0.7)*0.5+0.5),5.0)*0.35; float3 base=tint.rgb*(0.55+brush)+0.25+spec; return float4(base*tint.rgb,tint.a); }";
    static const char* kHlslGlow = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{ float pulse=sin(time*4.0)*0.2+0.8; float edge=sin(p.x*0.02+p.y*0.015+time*2.0)*0.12+0.88; float3 c=tint.rgb*pulse*edge*1.7; return float4(c,tint.a); }";
    static const char* kHlslHologram = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{ float scan=step(frac(p.y*0.15),0.5); float flick=sin(time*15.0)*0.08+0.92; float3 c; c.r=tint.r*(1.0+sin(p.y*0.3+time*3.0)*0.15); c.g=tint.g*(1.0+sin(p.y*0.3+time*3.0+2.094)*0.15); c.b=tint.b*(1.0+sin(p.y*0.3+time*3.0+4.189)*0.15); c*=flick*(0.5+scan*0.5); c+=float3(0.0,0.03,0.08)*scan; return float4(c,tint.a*(0.45+scan*0.4)); }";
    static const char* kHlslPearlescent = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{ float2 uv=p.xy/screen; float shift=sin(uv.x*5.0+uv.y*3.5+time*1.2)*0.5+0.5; float3 c1=tint.rgb; float3 c2=float3(tint.z,tint.x,tint.y); float3 c3=float3(tint.y,tint.z,tint.x); float3 col=lerp(lerp(c1,c2,shift),c3,sin(shift*3.14159)*0.45); float gloss=pow(saturate(sin(uv.y*30.0+time*2.5)*0.5+0.5),6.0)*0.2; return float4(col*1.2+gloss,tint.a); }";
    static const char* kHlslCrystal = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{ float2 uv=p.xy/screen; float facet=sin(uv.x*45.0)*sin(uv.y*45.0)*0.5+0.5; float rainbow=frac(uv.x*2.0+uv.y*1.5+time*0.3); float3 c; c.r=tint.r*(0.5+facet*0.5)+pow(saturate(1.0-abs(rainbow*3.0)),2.0)*0.25; c.g=tint.g*(0.5+facet*0.5)+pow(saturate(1.0-abs(rainbow*3.0-1.0)),2.0)*0.25; c.b=tint.b*(0.5+facet*0.5)+pow(saturate(1.0-abs(rainbow*3.0-2.0)),2.0)*0.25; float sparkle=pow(facet,8.0)*0.55; c+=sparkle; return float4(c,tint.a); }";
    static const char* kHlslGlass = "cbuffer CB:register(b12){float4 tint;float2 screen;float time;}; float4 main(float4 p:SV_Position):SV_Target{ float2 uv=p.xy/screen; float fresnel=pow(abs(sin(uv.x*6.28+uv.y*3.14)),0.4)*0.5+0.3; float3 c=tint.rgb*(1.0+fresnel*0.55); float shimmer=sin(uv.y*65.0+time*5.5)*0.025; c+=shimmer; float a=tint.a*(0.22+fresnel*0.48); return float4(c,a); }";

    // ---------------------------------------------------------------
    // Hook Logic & State
    // ---------------------------------------------------------------
    typedef void(__stdcall* DrawIndexedInstancedFn)(ID3D11DeviceContext*, UINT, UINT, UINT, INT, UINT);
    inline DrawIndexedInstancedFn oDrawIndexedInstanced = nullptr;

    inline HRESULT CompileMaterial(ID3D11Device* dev, const char* src, ID3D11PixelShader** ps)
    {
        *ps = nullptr;
        if (!src) return S_FALSE;
        ID3DBlob* blob = nullptr, *err = nullptr;
        HRESULT hr = D3DCompile(src, strlen(src), "chams", nullptr, nullptr, "main", "ps_4_0", 0, 0, &blob, &err);
        if (FAILED(hr)) return hr;
        hr = dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, ps);
        blob->Release();
        return hr;
    }

    inline void Init(ID3D11Device* dev)
    {
        if (ready || !dev) return;
        CompileMaterial(dev, kHlslFlat, &psSlots[MAT_FLAT]);
        CompileMaterial(dev, kHlslChrome, &psSlots[MAT_CHROME]);
        CompileMaterial(dev, kHlslMetallic, &psSlots[MAT_METALLIC]);
        CompileMaterial(dev, kHlslGlow, &psSlots[MAT_GLOW]);
        CompileMaterial(dev, kHlslHologram, &psSlots[MAT_HOLOGRAM]);
        CompileMaterial(dev, kHlslPearlescent, &psSlots[MAT_PEARLESCENT]);
        CompileMaterial(dev, kHlslCrystal, &psSlots[MAT_CRYSTAL]);
        CompileMaterial(dev, kHlslGlass, &psSlots[MAT_GLASS]);

        D3D11_BUFFER_DESC bd = { sizeof(ChamsCB), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };
        dev->CreateBuffer(&bd, nullptr, &cbuf);

        D3D11_DEPTH_STENCIL_DESC dd = { FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, 0, 0 };
        dev->CreateDepthStencilState(&dd, &dssOff);
        ready = true;
    }

    inline void Setup()
    {
        // Non-device initialization if needed (e.g. config defaults)
    }

    inline void Tick()
    {
        if (!ready) return;
        static LARGE_INTEGER freq = {}, start = {};
        if (!freq.QuadPart) { QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&start); }
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        gTime = static_cast<float>((double)(now.QuadPart - start.QuadPart) / (double)freq.QuadPart);

        memset(Game::GameState::originalDormant, 0, sizeof(Game::GameState::originalDormant));
        if (cfg.enabled && cfg.wallhack)
        {
            uintptr_t localPawn = Game::GetLocalPlayerPawn();
            if (!localPawn) return;
            int localTeam = Game::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);
            uintptr_t entList = Game::Read<uintptr_t>(Game::clientBase + Offsets::dwEntityList);
            if (!entList) return;

            for (int i = 1; i <= 64; ++i)
            {
                uintptr_t chunk = Game::Read<uintptr_t>(entList + 8 * (i >> 9) + 0x10);
                if (!chunk) continue;
                uintptr_t ctrl = Game::Read<uintptr_t>(chunk + 120 * (i & 0x1FF)); // 120 = stride
                if (!ctrl) continue;
                uint32_t ph = Game::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
                uintptr_t pawn = Game::GetEntityByHandle(ph);
                if (!pawn || pawn == localPawn) continue;
                if (Game::Read<uint8_t>(pawn + Offsets::m_iTeamNum) == localTeam) continue;

                uintptr_t sceneNode = Game::Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
                if (sceneNode)
                {
                    Game::GameState::originalDormant[i] = Game::Read<bool>(sceneNode + 0x10B);
                    Game::Write<bool>(sceneNode + 0x10B, false);
                }
            }
        }
    }

    inline void __stdcall hkDrawIndexedInstanced(ID3D11DeviceContext* ctx, UINT IndexCount, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
    {
        static bool firstCall = true;
        if (firstCall) { printf("[+] hkDrawIndexedInstanced HOOK HIT\n"); firstCall = false; }

        if (!cfg.enabled || !ready)
            return oDrawIndexedInstanced(ctx, IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

        ID3D11Buffer* vBuf = nullptr;
        UINT stride = 0, offset = 0;
        ctx->IAGetVertexBuffers(0, 1, &vBuf, &stride, &offset);
        if (vBuf) vBuf->Release();

        SlotStyle* active = nullptr;
        // The counts you logged: 612, 3312, 3648, 7218, 8160
        bool isPlayer = (stride == 32 && (IndexCount == 612 || IndexCount == 3312 || IndexCount == 3648 || IndexCount == 7218 || IndexCount == 8160));
        
        bool isHand   = (stride == 52 || stride == 48 || stride == 40 || stride == 44);
        bool isWeapon = (stride == 28 || stride == 24);

        if (isPlayer) active = &cfg.playerVis;
        else if (isHand && cfg.handsEnabled) active = &cfg.hands;
        else if (isWeapon && cfg.weaponsEnabled) active = &cfg.weapons;

        if (active)
        {
            // 1. Update Constant Buffer
            D3D11_MAPPED_SUBRESOURCE mapped;
            if (cbuf && SUCCEEDED(ctx->Map(cbuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                auto* cb = static_cast<ChamsCB*>(mapped.pData);
                cb->tintR = active->color[0]; cb->tintG = active->color[1];
                cb->tintB = active->color[2]; cb->tintA = active->color[3];
                cb->screenW = 1920.f; cb->screenH = 1080.f; cb->time = gTime;
                ctx->Unmap(cbuf, 0);
                ctx->PSSetConstantBuffers(12, 1, &cbuf);
            }

            // 2. Clear Pass (Hidden/XQZ)
            // CRITICAL: We only do this for enemies. 
            // Since we can't tell perfectly in D3D11, we exclude common "Viewmodel" counts
            // to prevent the black-screen arm glitch.
            bool isNearbyViewmodel = (IndexCount == 8160 || IndexCount == 7218); // Main arm segments
            if (isPlayer && cfg.wallhack && dssOff && !isNearbyViewmodel)
            {
                ctx->OMSetDepthStencilState(dssOff, 0);
                if (cfg.playerHid.material != MAT_NONE && psSlots[cfg.playerHid.material])
                    ctx->PSSetShader(psSlots[cfg.playerHid.material], nullptr, 0);

                oDrawIndexedInstanced(ctx, IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
                ctx->OMSetDepthStencilState(nullptr, 0); // Restore depth
            }

            if (active->material != MAT_NONE && psSlots[active->material])
                ctx->PSSetShader(psSlots[active->material], nullptr, 0);
        }

        oDrawIndexedInstanced(ctx, IndexCount, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        ctx->PSSetShader(nullptr, nullptr, 0);
    }

    inline void Shutdown()
    {
        for (auto& ps : psSlots)
        {
            if (ps) { ps->Release(); ps = nullptr; }
        }
        if (dssOff) { dssOff->Release(); dssOff = nullptr; }
        if (cbuf)   { cbuf->Release();   cbuf   = nullptr; }
        ready = false;
    }
}
