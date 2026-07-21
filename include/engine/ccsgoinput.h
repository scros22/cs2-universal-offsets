// ccsgoinput.h  ·  CS2 build 14172  ·  cs2-sdk.com
// Client input singleton — turns raw mouse/keyboard state into the per-tick CUserCmd sent to the server. Not a schema (networked) class.
// Reverse-engineered from client.dll; offsets drift between builds — re-verify after a CS2 update.
#pragma once
#include <cstdint>

namespace CCSGOInput {

// instance:  (CCSGOInput*)(client.dll + 0x23B95F0) — static object embedded at this RVA (no deref; the GetInput getter returns its address)
inline constexpr std::ptrdiff_t kInstance_rva = 0x23B95F0;
inline constexpr std::ptrdiff_t kCreateMove_rva = 0xB1FB80;

// --- fields ---
inline constexpr std::ptrdiff_t vtable                = 0x0   ; // void** — CCSGOInput vftable
inline constexpr std::ptrdiff_t m_pButtons            = 0x20  ; // kbutton*[] — registered in_* commands (in_forward, in_jump, ...)
inline constexpr std::ptrdiff_t m_angSubtickMoveAngle = 0x270 ; // QAngle — per-subtick move-direction source
inline constexpr std::ptrdiff_t m_angViewHistory      = 0x288 ; // QAngle — previous / interpolated view-angle history
inline constexpr std::ptrdiff_t m_angViewAngles       = 0x688 ; // QAngle — current command view angles — pitch 0x688 / yaw 0x68C / roll 0x690; mouse delta is added into yaw here each frame
inline constexpr std::ptrdiff_t m_mouseAccumHistory   = 0x6F8 ; // struct — input-delta smoothing sub-object {int count; void* samples; ...}
inline constexpr std::ptrdiff_t m_nCommandNumber      = 0xB50 ; // int — command / sequence number (add 0x18*slot for splitscreen)
inline constexpr std::ptrdiff_t m_pUserCmds           = 0xB58 ; // CUserCmd* — ring buffer base, 1088-byte entries (add 0x18*slot for splitscreen)
} // namespace CCSGOInput
