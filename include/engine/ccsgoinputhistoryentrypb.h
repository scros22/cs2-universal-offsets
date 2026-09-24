// ccsgoinputhistoryentrypb.h  -  CS2 build 14183  -  cs2-sdk.com
// cs_usercmd.proto CSGOInputHistoryEntryPB - per-frame view/interp record the server uses for lag compensation and shot validation.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CCSGOInputHistoryEntryPB {

// protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start
inline constexpr std::size_t kSize = 0x78;
inline constexpr std::ptrdiff_t kNew_rva = 0x7B3310; // pattern CCSGOInputHistoryEntryPB_New

// --- fields ---
inline constexpr std::ptrdiff_t has_bits              = 0x10  ; // uint32
inline constexpr std::ptrdiff_t cached_size           = 0x14  ; // int32
inline constexpr std::ptrdiff_t view_angles           = 0x18  ; // CMsgQAngle* - has-bit 0x1
inline constexpr std::ptrdiff_t cl_interp             = 0x20  ; // CSGOInterpolationInfoPB_CL* - has-bit 0x2
inline constexpr std::ptrdiff_t sv_interp0            = 0x28  ; // CSGOInterpolationInfoPB* - has-bit 0x4
inline constexpr std::ptrdiff_t sv_interp1            = 0x30  ; // CSGOInterpolationInfoPB* - has-bit 0x8
inline constexpr std::ptrdiff_t player_interp         = 0x38  ; // CSGOInterpolationInfoPB* - has-bit 0x10
inline constexpr std::ptrdiff_t shoot_position        = 0x40  ; // CMsgVector* - has-bit 0x20
inline constexpr std::ptrdiff_t target_head_pos_check = 0x48  ; // CMsgVector* - has-bit 0x40
inline constexpr std::ptrdiff_t target_abs_pos_check  = 0x50  ; // CMsgVector* - has-bit 0x80
inline constexpr std::ptrdiff_t target_abs_ang_check  = 0x58  ; // CMsgQAngle* - has-bit 0x100
inline constexpr std::ptrdiff_t render_tick_count     = 0x60  ; // int32 - has-bit 0x200
inline constexpr std::ptrdiff_t render_tick_fraction  = 0x64  ; // float - has-bit 0x400
inline constexpr std::ptrdiff_t player_tick_count     = 0x68  ; // int32 - has-bit 0x800
inline constexpr std::ptrdiff_t player_tick_fraction  = 0x6C  ; // float - has-bit 0x1000
inline constexpr std::ptrdiff_t frame_number          = 0x70  ; // int32 - has-bit 0x2000
inline constexpr std::ptrdiff_t target_ent_index      = 0x74  ; // int32 - has-bit 0x4000
} // namespace CCSGOInputHistoryEntryPB
