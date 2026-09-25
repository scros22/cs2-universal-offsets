// cbaseusercmdpb.h  -  CS2 build 14184  -  cs2-sdk.com
// usercmd.proto CBaseUserCmdPB - movement, buttons, view angles and subtick steps. Ground truth is its _InternalParse (field number -> offset + has-bit); the dumper's protobuf walk has mislabelled the 4-byte fields on older builds, so prefer this table.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CBaseUserCmdPB {

// protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start
inline constexpr std::size_t kSize = 0x88;
inline constexpr std::ptrdiff_t kSerializeMoveCrc_rva = 0x122CFF0; // pattern CBaseUserCmdPB_SerializeMoveCrc

// --- fields ---
inline constexpr std::ptrdiff_t has_bits                      = 0x10  ; // uint32 - see per-field bits
inline constexpr std::ptrdiff_t cached_size                   = 0x14  ; // int32
inline constexpr std::ptrdiff_t subtick_moves                 = 0x18  ; // RepeatedPtrField<CSubtickMoveStep> - field 18; {arena* +0x18, size +0x20, capacity +0x24, rep* +0x28}
inline constexpr std::ptrdiff_t move_crc                      = 0x30  ; // std::string* - field 19, has-bit 0x1 - server CRC32 over buttons + view angles
inline constexpr std::ptrdiff_t buttons_pb                    = 0x38  ; // CInButtonStatePB* - field 3, has-bit 0x2
inline constexpr std::ptrdiff_t viewangles                    = 0x40  ; // CMsgQAngle* - field 4, has-bit 0x4
inline constexpr std::ptrdiff_t execution_notes               = 0x48  ; // void* - internal
inline constexpr std::ptrdiff_t legacy_command_number         = 0x50  ; // int32 - field 1, has-bit 0x10
inline constexpr std::ptrdiff_t client_tick                   = 0x54  ; // int32 - field 2, has-bit 0x20
inline constexpr std::ptrdiff_t forwardmove                   = 0x58  ; // float - field 5, has-bit 0x40
inline constexpr std::ptrdiff_t leftmove                      = 0x5C  ; // float - field 6, has-bit 0x80 (sidemove - NOT +0x58)
inline constexpr std::ptrdiff_t upmove                        = 0x60  ; // float - field 7, has-bit 0x100
inline constexpr std::ptrdiff_t impulse                       = 0x64  ; // int32 - field 8, has-bit 0x200
inline constexpr std::ptrdiff_t weaponselect                  = 0x68  ; // int32 - has-bit 0x400
inline constexpr std::ptrdiff_t random_seed                   = 0x6C  ; // int32 - has-bit 0x800
inline constexpr std::ptrdiff_t mousedx                       = 0x70  ; // int32 - has-bit 0x1000
inline constexpr std::ptrdiff_t mousedy                       = 0x74  ; // int32 - has-bit 0x2000
inline constexpr std::ptrdiff_t prediction_offset_ticks_x256  = 0x78  ; // uint32 - has-bit 0x4000
inline constexpr std::ptrdiff_t consumed_server_angle_changes = 0x7C  ; // uint32 - has-bit 0x8000
inline constexpr std::ptrdiff_t cmd_flags                     = 0x80  ; // int32 - has-bit 0x10000
inline constexpr std::ptrdiff_t pawn_entity_handle            = 0x84  ; // uint32 - has-bit 0x20000
} // namespace CBaseUserCmdPB
