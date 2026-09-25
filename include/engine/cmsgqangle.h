// cmsgqangle.h  -  CS2 build 14184  -  cs2-sdk.com
// networkbasetypes.proto CMsgQAngle.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CMsgQAngle {

// protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start
inline constexpr std::size_t kSize = 0x28;

// --- fields ---
inline constexpr std::ptrdiff_t has_bits = 0x10  ; // uint32 - x 0x1, y 0x2, z 0x4
inline constexpr std::ptrdiff_t x        = 0x18  ; // float - pitch
inline constexpr std::ptrdiff_t y        = 0x1C  ; // float - yaw
inline constexpr std::ptrdiff_t z        = 0x20  ; // float - roll
} // namespace CMsgQAngle
