// cviewsetup.h  ·  CS2 build 14172  ·  cs2-sdk.com
// The camera/view description filled each frame (fov, world origin, view angles). Written by OverrideView; read by the renderer. Not a schema class.
// Reverse-engineered from client.dll; offsets drift between builds — re-verify after a CS2 update.
#pragma once
#include <cstdint>

namespace CViewSetup {

// instance:  received by OverrideView (client.dll + 0xC987F0) in rdx
inline constexpr std::ptrdiff_t kOverrideView_rva = 0xC987F0;

// --- fields ---
inline constexpr std::ptrdiff_t m_flFov         = 0x498 ; // float — field of view
inline constexpr std::ptrdiff_t m_vecOrigin     = 0x4A0 ; // Vector — world eye origin — x 0x4A0 / y 0x4A4 / z 0x4A8
inline constexpr std::ptrdiff_t m_angViewAngles = 0x4B8 ; // QAngle — view angles — pitch 0x4B8 / yaw 0x4BC / roll 0x4C0
} // namespace CViewSetup
