// ccsgousercmdpb.h  -  CS2 build 14183  -  cs2-sdk.com
// cs_usercmd.proto CSGOUserCmdPB - the top-level command message the client sends. Layout from its parser/serialiser; verified in a working internal on build 2000914.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CCSGOUserCmdPB {

// protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start
inline constexpr std::size_t kSize = 0x48;

// --- fields ---
inline constexpr std::ptrdiff_t has_bits                    = 0x10  ; // uint32 - 0x1 base, 0x2 left_hand_desired, 0x4 body-shot fx, 0x8 head-shot fx, 0x10 kill ragdolls, 0x20 attack1 idx, 0x40 attack2 idx
inline constexpr std::ptrdiff_t cached_size                 = 0x14  ; // int32
inline constexpr std::ptrdiff_t input_history               = 0x18  ; // RepeatedPtrField<CCSGOInputHistoryEntryPB> - field 2; {arena* +0x18, size +0x20, capacity +0x24, rep* +0x28}
inline constexpr std::ptrdiff_t base                        = 0x30  ; // CBaseUserCmdPB* - field 1, has-bit 0x1
inline constexpr std::ptrdiff_t left_hand_desired           = 0x38  ; // bool - has-bit 0x2
inline constexpr std::ptrdiff_t is_predicting_body_shot_fx  = 0x39  ; // bool - has-bit 0x4
inline constexpr std::ptrdiff_t is_predicting_head_shot_fx  = 0x3A  ; // bool - has-bit 0x8
inline constexpr std::ptrdiff_t is_predicting_kill_ragdolls = 0x3B  ; // bool - has-bit 0x10
inline constexpr std::ptrdiff_t attack1_start_history_index = 0x3C  ; // int32 - has-bit 0x20
inline constexpr std::ptrdiff_t attack2_start_history_index = 0x40  ; // int32 - has-bit 0x40
} // namespace CCSGOUserCmdPB
