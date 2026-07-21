// cusercmd.h  ·  CS2 build 14172  ·  cs2-sdk.com
// One entry in CCSGOInput's per-command ring buffer (1088 bytes / 0x440 each) — the resolved view angle + button masks CreateMove writes each tick. This is the client working buffer, not the networked protobuf.
// Reverse-engineered from client.dll; offsets drift between builds — re-verify after a CS2 update.
#pragma once
#include <cstdint>

namespace CUserCmd {

// instance:  CCSGOInput.m_pUserCmds[i]  (base at CCSGOInput+0xB58, stride 1088)

// --- fields ---
inline constexpr std::ptrdiff_t m_nButtons             = 0x0   ; // uint64 — pressed (down) button mask
inline constexpr std::ptrdiff_t m_nButtonsQueuedDown   = 0x8   ; // uint64 — bits forced DOWN this cmd (subtick)
inline constexpr std::ptrdiff_t m_nButtonsQueuedChange = 0x10  ; // uint64 — bits that CHANGED this cmd (subtick gate)
inline constexpr std::ptrdiff_t m_angViewAngles        = 0x18  ; // QAngle — clamped view angle for this cmd — pitch 0x18 / yaw 0x1C / roll 0x20
inline constexpr std::ptrdiff_t m_nSubtickCount        = 0x2C  ; // int — subtick move-step count
inline constexpr std::ptrdiff_t m_pSubtickData         = 0x30  ; // void* — subtick move-step data
inline constexpr std::ptrdiff_t m_flMouseDX            = 0x430 ; // float — raw mouse-delta X (accumulated)
inline constexpr std::ptrdiff_t m_flMouseDY            = 0x434 ; // float — raw mouse-delta Y
} // namespace CUserCmd
