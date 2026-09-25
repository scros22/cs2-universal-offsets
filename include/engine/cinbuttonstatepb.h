// cinbuttonstatepb.h  -  CS2 build 14184  -  cs2-sdk.com
// usercmd.proto CInButtonStatePB - the button masks as sent: buttonstate1 = held, buttonstate2 = changed, buttonstate3 = scroll.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CInButtonStatePB {

// protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start
inline constexpr std::size_t kSize = 0x30;
inline constexpr std::ptrdiff_t kNew_rva = 0x4F6BC0; // pattern CInButtonStatePB_New

// --- fields ---
inline constexpr std::ptrdiff_t has_bits     = 0x10  ; // uint32
inline constexpr std::ptrdiff_t cached_size  = 0x14  ; // int32
inline constexpr std::ptrdiff_t buttonstate1 = 0x18  ; // uint64 - held (IN_* mask), has-bit 0x1
inline constexpr std::ptrdiff_t buttonstate2 = 0x20  ; // uint64 - changed vs the previous SENT command, has-bit 0x2
inline constexpr std::ptrdiff_t buttonstate3 = 0x28  ; // uint64 - scroll, has-bit 0x4
} // namespace CInButtonStatePB
