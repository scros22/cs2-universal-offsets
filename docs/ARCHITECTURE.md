# CS2 Internal — Architecture Reference

## Module Map

```
dllmain.cpp               Entry point → EntryThread → init chain → main loop
├── core/
│   ├── memory.h           Process memory R/W primitives (ReadProcessMemory wrapper)
│   ├── math.h             Vec3, QAngle, ViewMatrix, CalcAngle, W2S, AngleFov
│   ├── game_state.h       Module bases, entity system, bone data, W2S
│   ├── sdk_offsets.h      Auto-offset layer (pulls from sdk/*.hpp dumper output)
│   ├── signatures.h       Byte-pattern scanner for function hooks
│   ├── stealth.h          PEB unlinking, header wipe, heartbeat monitor
│   ├── syscall.h          Direct NT syscall stubs (bypass ntdll hooks)
│   ├── spoof_call.h       Return-address spoofing (gadget trampoline in ntdll)
│   ├── vtable_swap.h      VMT shadow-copy swap (zero inline patches)
│   └── xorstr.h           Compile-time string encryption (XS/XW macros)
│
├── features/
│   ├── aimbot.h           Mouse-input aimbot (SendInput, neuromotor model, governor)
│   ├── bhop.h             Auto-hop (memory write — NEEDS MIGRATION to SendInput)
│   ├── triggerbot.h       Auto-fire (SHOULD BE REMOVED — was "deleted" in April)
│   ├── esp.h              2D overlay (boxes, skeleton, health, name, weapon, bomb)
│   ├── chams.h            D3D11 material shaders (wallhack + 9 material types)
│   ├── bullet_tracer.h    Shot trajectory visualization
│   ├── damage_indicator.h Floating damage numbers
│   └── world_effects.h    Sky color, no-flash, no-smoke, fire color, FOV, brightness
│
├── render/
│   ├── hooks.h            DX11 Present/DrawIndexed hooks, ImGui init, PresentCore
│   └── menu.h             ImGui menu (3 styles: Glass/Cyber/Minimal)
│
├── sdk/                   cs2-dumper output (auto-generated, don't edit)
├── vendor/                ImGui + MinHook
└── tools/                 Lucid.exe injector source
```

## Hook Architecture

1. **MinHook bootstrap** (1 frame) → detours DXGISwapChain::Present
2. **VMT shadow-copy swap** replaces MinHook → zero inline patches remain
3. **Function hooks** (persistent): GetWorldFov, DrawSkyboxArray via MinHook
4. **No CreateMove hook** — aimbot is pure read + SendInput mouse
5. **No game memory writes for aim** — only reads view angles, bones, flags

## Feature Call Chain (every frame)

```
PresentCore(sc)
  ├── Aimbot::Tick()           — read game state → compute angle → SendInput mouse
  ├── Triggerbot::Tick()       — [TO BE REMOVED]
  ├── Bhop::Tick()             — [TO BE REWRITTEN: SendInput instead of mem write]
  ├── WorldEffects::Tick()     — no-flash, no-smoke, fire color, brightness
  ├── DamageIndicator::Tick()  — update floating numbers
  ├── Chams::Tick(pDevice)     — compile shaders, etc.
  ├── ImGui::NewFrame()
  ├── Menu::Render()           — if showMenu
  ├── ESP::Render()            — boxes, skeleton, health bars
  ├── Aimbot::RenderFovCircle()
  ├── BulletTracer::Render()
  ├── DamageIndicator::Render()
  └── ImGui::Render() + draw
```

## Input Method

All aim output and automation output uses `SendInput(INPUT_MOUSE)` with:
- Sub-pixel accumulation (mouseAccumX/Y)
- Return-address spoofed via `SpoofCall::SpoofedSendInput()`
- Indistinguishable from hardware mouse at kernel level

## Anti-Detection Layers

| Layer | Method | Status |
|-------|--------|--------|
| Thread start | NtSetInformationThread (spoof to BaseThreadInitThunk) | ✅ |
| Thread hide | ThreadHideFromDebugger | ✅ |
| PEB | UnlinkFromPEB (all 3 lists + name scrub) | ✅ |
| Headers | Wiped (but NOT re-wipeable due to RtlAddFunctionTable) | ✅ |
| SEH | RtlAddFunctionTable registered | ✅ |
| Hooks | VMT swap (no .text patches after bootstrap) | ✅ |
| Syscalls | Direct NT syscall stubs | ✅ |
| Return addr | Gadget trampoline in ntdll | ✅ |
| Aim output | SendInput mouse (no game mem writes) | ✅ |
| Bhop output | Memory write 65537 to jump addr | ❌ DETECTED |
| Disk logging | Writes to %TEMP% in release builds | ❌ DETECTED |
| Heartbeat | Hollow — no real VAC module detection | ⚠️ WEAK |

## Build Commands

```powershell
# CS2.dll
& "C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe" `
  CS2.vcxproj /p:Configuration=Release /p:Platform=x64 /m /v:minimal

# Copy to injection directory
Copy-Item "x64\Release\CS2.dll" "x64\CS2.dll" -Force

# Lucid.exe
cmd /c "vcvarsall.bat x64 && cd /d tools && rc.exe app.rc && cl.exe /EHsc /O2 /std:c++17 /MT /Fe:..\x64\Lucid.exe injector.cpp app.res /link advapi32.lib shell32.lib ole32.lib Psapi.lib"
```
