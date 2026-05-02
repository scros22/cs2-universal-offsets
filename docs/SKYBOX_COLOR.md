# CS2 Skybox Color — Snippet Pack

Simple recipe for tinting the CS2 skybox in real time. Internal cheat / DLL.

- **Game:** CS2, build **14152+** (Source 2)
- **Module:** `scenesystem.dll`
- **Function:** `DrawSkyboxArray` (IDA: `sub_18014FB90`)
- **Hook lib:** MinHook

---

## Why a hook (and not entity writes)

Writing `m_vTintColor` on the `env_sky` entity does **nothing** — the renderer
caches sky material params at setup. You have to scribble the color into the
draw-primitive buffer right before `DrawSkyboxArray` consumes it.

(Brightness is the exception: `m_flBrightnessScale` on `env_sky` does update
live. But color = hook only.)

---

## Signature

```cpp
// scenesystem.dll  —  unique single match
constexpr const char* DrawSkyboxArray =
    "45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55";
```

Resolve with whatever pattern scanner you use:

```cpp
uintptr_t addr = FindPattern(L"scenesystem.dll", DrawSkyboxArray_sig);
```

---

## Layout (the important offsets)

The 4th argument (`drawPrimitive`) is the base of an array of `0x68`-byte
primitive descriptors. `count` tells you how many. The skybox object pointer
lives at the tail:

```
skyboxObjPtr = drawPrimitive + (count * 0x68) - 0x50    // dereference once
```

Then on the skybox object itself:

```
+0xE8 .. +0xF0   vec3 tint  (RGB, 3× float, range 0..1)   <-- write here
+0xF4            int  mode  (1 or 2)
+0xF8 .. +0x104  four sun-angle floats  (V_sinf inputs — DO NOT TOUCH)
```

> **Trap:** in older builds the tint was at `+0x100`. That slot is now a
> sun-angle float. Writing RGB there NaN-poisons `V_sinf` and the renderer
> crashes ~60s later. Use `+0xE8`.

---

## Hook prototype

```cpp
using DrawSkyboxArrayFn =
    void(__fastcall*)(void*, void*, void*, int, void*, void*, void*);

inline DrawSkyboxArrayFn oDrawSkyboxArray = nullptr;

// User config
struct { bool enabled = true; float color[3] = {0.4f, 0.0f, 0.6f}; } sky;
```

---

## The hook

```cpp
inline void __fastcall hkDrawSkyboxArray(
    void* a1, void* a2, void* drawPrimitive,
    int count, void* a5, void* a6, void* a7)
{
    // Real skybox draws have count <= 8. Cap at 16 to skip cubemap-rebuild
    // calls that have a different layout.
    if (sky.enabled && drawPrimitive && count > 0 && count <= 16)
    {
        __try {
            size_t   off          = (size_t)(count * 0x68) - 0x50;
            void**   skyboxObjPtr = (void**)((char*)drawPrimitive + off);
            if (skyboxObjPtr && *skyboxObjPtr)
            {
                float* rgb = (float*)((char*)(*skyboxObjPtr) + 0xE8);
                rgb[0] = sky.color[0];
                rgb[1] = sky.color[1];
                rgb[2] = sky.color[2];
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (oDrawSkyboxArray)
        oDrawSkyboxArray(a1, a2, drawPrimitive, count, a5, a6, a7);
}
```

---

## Install (MinHook)

```cpp
uintptr_t addr = FindPattern(L"scenesystem.dll", DrawSkyboxArray_sig);
if (addr) {
    MH_CreateHook((void*)addr, &hkDrawSkyboxArray, (void**)&oDrawSkyboxArray);
    MH_EnableHook((void*)addr);
}
```

That's it — change `sky.color[]` from your menu and the sky updates every frame.

---

## Optional: rainbow

```cpp
inline void HsvToRgb(float h, float s, float v, float& r, float& g, float& b) {
    float c = v * s;
    float x = c * (1.f - fabsf(fmodf(h / 60.f, 2.f) - 1.f));
    float m = v - c;
    if      (h <  60) { r=c; g=x; b=0; }
    else if (h < 120) { r=x; g=c; b=0; }
    else if (h < 180) { r=0; g=c; b=x; }
    else if (h < 240) { r=0; g=x; b=c; }
    else if (h < 300) { r=x; g=0; b=c; }
    else              { r=c; g=0; b=x; }
    r += m; g += m; b += m;
}

// inside the hook, instead of the 3 rgb[] writes:
float t   = (float)GetTickCount64() / 1000.f;
float hue = fmodf(t * 0.3f * 360.f, 360.f);   // 0.3 cycles/sec
HsvToRgb(hue, 1.f, 1.f, rgb[0], rgb[1], rgb[2]);
```

---

## Gotcha: FOV hooks

`DrawSkyboxArray` internally queries `GetWorldFov` to build the sky projection.
If you also hook `GetWorldFov` to override player FOV, you **must** pass through
the original value while the skybox draw is on the stack — otherwise the
sun-angle floats compute out-of-range and NaN-poison the skybox object.

```cpp
inline bool g_inSkyboxDraw = false;

// In hkDrawSkyboxArray, wrap the original call:
g_inSkyboxDraw = true;
__try { oDrawSkyboxArray(a1, a2, drawPrimitive, count, a5, a6, a7); }
__except (EXCEPTION_EXECUTE_HANDLER) {}
g_inSkyboxDraw = false;

// In your FOV hook:
float __fastcall hkGetWorldFov(void* rcx) {
    float orig = oGetWorldFov(rcx);
    if (g_inSkyboxDraw) return orig;   // <-- the line that matters
    return userFov;
}
```

Both run on the render thread, so a plain `bool` is fine — no `thread_local`.

---

## TL;DR

| Piece | Value |
|---|---|
| Module | `scenesystem.dll` |
| Sig | `45 85 C9 0F 8E ? ? ? ? 4C 8B DC 55` |
| Skybox obj ptr | `drawPrimitive + count*0x68 - 0x50`, deref |
| Tint RGB (vec3 float) | object **+0xE8** |
| Old (broken) offset | ~~+0x100~~ — sun-angle float now, will crash |
