// cusercmd.h  -  CS2 build 14183  -  cs2-sdk.com
// The client's command object (one per tick). Embeds the CCSGOUserCmdPB that is serialised to the server and the live CInButtonState. Lives in a 150-entry ring per controller.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CUserCmd {

// ring = GetUserCmdManager(controller); cmd = ring + 0x98 * (sequence % 150); current sequence = *(int*)(ring + 0x5910)  (150 * 0x98 = 0x5910)
inline constexpr std::size_t kSize = 0x98;
inline constexpr std::ptrdiff_t kGetUserCmdManager_rva = 0x943540; // pattern GetUserCmdManager
inline constexpr std::ptrdiff_t kGetCUserCmdBySequenceNumber_rva = 0x9434B0; // pattern GetCUserCmdBySequenceNumber

// --- fields ---
inline constexpr std::ptrdiff_t vtable                                    = 0x0   ; // void** - CUserCmd vftable
inline constexpr std::ptrdiff_t m_nCommandNumber                          = 0x8   ; // int64 - command / sequence number
inline constexpr std::ptrdiff_t m_csgoUserCmd                             = 0x10  ; // CCSGOUserCmdPB - embedded protobuf message (vtable at +0x10, fields from +0x20)
inline constexpr std::ptrdiff_t m_csgoUserCmd_has_bits                    = 0x20  ; // uint32 - CCSGOUserCmdPB has-bits (0x1 = base present)
inline constexpr std::ptrdiff_t m_csgoUserCmd_input_history               = 0x28  ; // RepeatedPtrField<CCSGOInputHistoryEntryPB> - {arena* +0x28, int size +0x30, int capacity +0x34, rep* +0x38}
inline constexpr std::ptrdiff_t m_csgoUserCmd_base                        = 0x40  ; // CBaseUserCmdPB* - the base command (movement, buttons, view angles, subtick steps)
inline constexpr std::ptrdiff_t m_csgoUserCmd_attack1_start_history_index = 0x4C  ; // int32 - has-bit 0x20
inline constexpr std::ptrdiff_t m_csgoUserCmd_attack2_start_history_index = 0x50  ; // int32 - has-bit 0x40
inline constexpr std::ptrdiff_t m_ButtonState                             = 0x58  ; // CInButtonState - live button state (vtable +0x58)
inline constexpr std::ptrdiff_t m_ButtonState_m_nValue                    = 0x60  ; // uint64 - buttons held (IN_* mask)
inline constexpr std::ptrdiff_t m_ButtonState_m_nValueChanged             = 0x68  ; // uint64 - buttons that changed this command
inline constexpr std::ptrdiff_t m_ButtonState_m_nValueScroll              = 0x70  ; // uint64 - scroll-wheel buttons
inline constexpr std::ptrdiff_t m_nSubtickState                           = 0x94  ; // int32 - == 2 while the engine re-runs this same command for a subtick (repeat pass)
} // namespace CUserCmd
