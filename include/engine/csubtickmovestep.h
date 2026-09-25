// csubtickmovestep.h  -  CS2 build 14184  -  cs2-sdk.com
// usercmd.proto CSubtickMoveStep - one timed button edge / analog move / view-angle delta inside a tick. The list must ascend by `when`.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CSubtickMoveStep {

// protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start
inline constexpr std::size_t kSize = 0x38;
inline constexpr std::ptrdiff_t kCreateSubtickMoveStep_rva = 0x4F6C40; // pattern CreateSubtickMoveStep

// --- fields ---
inline constexpr std::ptrdiff_t has_bits             = 0x10  ; // uint32
inline constexpr std::ptrdiff_t cached_size          = 0x14  ; // int32
inline constexpr std::ptrdiff_t button               = 0x18  ; // uint64 - IN_* bit, has-bit 0x1
inline constexpr std::ptrdiff_t pressed              = 0x20  ; // bool - has-bit 0x2
inline constexpr std::ptrdiff_t when                 = 0x24  ; // float - fraction of the tick [0,1), has-bit 0x4
inline constexpr std::ptrdiff_t analog_forward_delta = 0x28  ; // float - has-bit 0x8
inline constexpr std::ptrdiff_t analog_left_delta    = 0x2C  ; // float - has-bit 0x10
inline constexpr std::ptrdiff_t pitch_delta          = 0x30  ; // float - delta vs the command's base view angle, has-bit 0x20
inline constexpr std::ptrdiff_t yaw_delta            = 0x34  ; // float - delta vs the command's base view angle, has-bit 0x40
} // namespace CSubtickMoveStep
