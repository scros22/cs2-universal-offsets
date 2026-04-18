# UNIX CS2 Internal Cheat

A feature-rich internal cheat for Counter-Strike 2 built with DirectX 11 and ImGui.

## Features

### Rage
- **Aimbot**
  - FOV-based targeting (Angle or Screen distance)
  - Silent aim (no visual camera movement)
  - Bone selection (Head, Neck, Chest, Pelvis)
  - Team check
  - Customizable aim key
  - Auto shoot

- **Triggerbot**
  - Automatic firing when crosshair is on enemy
  - Configurable delay
  - Team check
  - Key binding support

### Visuals
- **ESP**
  - 2D Boxes (Normal or Corner style)
  - Skeleton
  - Health bars with color gradient
  - Player names
  - Distance indicator
  - Team check
  - Customizable colors

- **Other**
  - Glow ESP with color customization
  - Bomb timer
  - Spectator list
  - Third person camera

### Skins
- **Skin Changer**
  - Apply skins to any weapon
  - Custom paint kit ID support
  - Wear/float customization
  - Seed customization
  - StatTrak support
  - Knife skin support
  - Force update & clear all options

## Project Structure

```
unix cs2/
├── dllmain.cpp           # DLL entry point
├── sdk/
│   ├── game.h            # Game memory helpers
│   ├── offsets.h         # CS2 offsets (Feb 5, 2026)
│   ├── structs.h         # Math structures
│   └── font.h            # Embedded font
├── features/
│   ├── aimbot.h          # Aimbot implementation
│   ├── triggerbot.h      # Triggerbot implementation
│   ├── esp.h             # ESP/Visuals
│   └── skin_changer.h    # Skin changer
├── hooks/
│   ├── present.h         # DirectX 11 Present hook
│   └── menu.h            # ImGui menu
├── imgui/                # ImGui library (ADD THIS)
└── minhook/              # MinHook library (ADD THIS)
```

## Setup Instructions

### 1. Add Required Libraries

You need to manually add these folders to the project:

#### ImGui (DirectX 11 backend)
Copy the following files to `imgui/`:
- `imgui.h`
- `imgui.cpp`
- `imgui_draw.cpp`
- `imgui_widgets.cpp`
- `imgui_tables.cpp`
- `imgui_impl_dx11.cpp`
- `imgui_impl_dx11.h`
- `imgui_impl_win32.cpp`
- `imgui_impl_win32.h`
- `imgui_internal.h`
- `imconfig.h`
- `imstb_rectpack.h`
- `imstb_textedit.h`
- `imstb_truetype.h`

#### MinHook
Copy the following files to `minhook/`:
- `MinHook.h`
- `MinHook.lib` (x64 version)

### 2. Configure Visual Studio Project

1. **Project Type**: Dynamic Library (DLL)
2. **Platform**: x64
3. **C++ Standard**: C++17 or later

#### Additional Include Directories
Add to project properties:
- `$(ProjectDir)`
- `$(ProjectDir)imgui`
- `$(ProjectDir)minhook`

#### Additional Library Directories
- `$(ProjectDir)minhook`

#### Linker → Input → Additional Dependencies
```
d3d11.lib
dxgi.lib
Psapi.lib
minhook.lib
```

#### C/C++ → Precompiled Headers
- Set to "Not Using Precompiled Headers"

### 3. Build

1. Set configuration to **Release x64**
2. Build the project
3. Output DLL will be in `x64/Release/unix cs2.dll`

## Usage

### Injection
1. Launch Counter-Strike 2
2. Inject `unix cs2.dll` using your preferred injector
3. Console window will appear showing initialization status

### Controls
- **INSERT** - Toggle menu
- **END** - Unload cheat

### Menu Navigation
- **Rage Tab** - Aimbot and Triggerbot settings
- **Visuals Tab** - ESP and visual features
- **Skins Tab** - Weapon skin customization

## Important Notes

### Memory Safety
- Uses **0x70 entity stride** (correct for Feb 2026 build)
- Entity handles masked with `& 0x7FFF`
- Player index starts at 1 (index 0 is invalid)
- All memory operations wrapped in SEH (`__try/__except`)

### Anti-Detection
- Silent aim using frame history manipulation
- Indirect memory R/W for sensitive addresses
- VTable hooking instead of inline hooks for Present
- MinHook for CreateMove (signature-based)

### Offsets
Offsets are from **cs2-dumper** dated **February 5, 2026**. If the game updates, you'll need to:
1. Get new dumps from `C:\Users\Alex\Desktop\output\`
2. Update `sdk/offsets.h` with new values

## Credits

- **ImGui** - Dear ImGui by Omar Cornut
- **MinHook** - Minimalistic x86/x64 API Hooking Library
- **cs2-dumper** - Offset dumping tool
- **Epstein Rage** - Reference implementation for skin changer
- **Smallest Pixel 7** - Embedded font

## Disclaimer

This software is for educational purposes only. Use at your own risk.
