// cviewsetup.h  -  CS2 build 14183  -  cs2-sdk.com
// The camera/view description filled each frame (fov, origin, angles). Written by OverrideView; read by the renderer. Not a schema class. Verified on build 2000914.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CViewSetup {

// passed to OverrideView in rdx
inline constexpr std::ptrdiff_t kOverrideView_rva = 0xD015F0; // pattern OverrideView

// --- fields ---
inline constexpr std::ptrdiff_t m_flFov         = 0x498 ; // float - field of view
inline constexpr std::ptrdiff_t m_vecOrigin     = 0x4A0 ; // Vector - world eye origin - x 0x4A0 / y 0x4A4 / z 0x4A8
inline constexpr std::ptrdiff_t m_angViewAngles = 0x4B8 ; // QAngle - view angles - pitch 0x4B8 / yaw 0x4BC / roll 0x4C0
} // namespace CViewSetup
