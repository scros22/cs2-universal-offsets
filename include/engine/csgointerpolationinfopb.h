// csgointerpolationinfopb.h  -  CS2 build 14184  -  cs2-sdk.com
// cs_usercmd.proto interpolation record (sv_interp0/1, player_interp). The _CL variant (cl_interp) only has frac at +0x18.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CSGOInterpolationInfoPB {

// protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start
inline constexpr std::size_t kSize = 0x28;

// --- fields ---
inline constexpr std::ptrdiff_t has_bits = 0x10  ; // uint32
inline constexpr std::ptrdiff_t frac     = 0x18  ; // float - has-bit 0x1
inline constexpr std::ptrdiff_t src_tick = 0x1C  ; // int32 - has-bit 0x2
inline constexpr std::ptrdiff_t dst_tick = 0x20  ; // int32 - has-bit 0x4
} // namespace CSGOInterpolationInfoPB
