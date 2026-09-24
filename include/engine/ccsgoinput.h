// ccsgoinput.h  -  CS2 build 14183  -  cs2-sdk.com
// Client input singleton: turns mouse/keyboard state into the per-tick user command. Not a schema class. View angles verified in a working internal on build 2000914.
// Module: client.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CCSGOInput {

// static object embedded in client.dll (no deref); pCSGOInput is a global POINTER to this same object
inline constexpr std::ptrdiff_t kInstance_rva = 0x2571A90; // pattern pCSGOInputInstance
inline constexpr std::ptrdiff_t kCreateMove_rva = 0xB64A10; // pattern CreateMove
inline constexpr std::ptrdiff_t kGetViewAngles_rva = 0xB6FC30; // pattern GetViewAngles
inline constexpr std::ptrdiff_t kSetViewAngles_rva = 0xB7FA90; // pattern SetViewAngles
inline constexpr std::ptrdiff_t kProcessInputEvent_rva = 0xB7B010; // pattern CCSGOInput_ProcessInputEvent
inline constexpr std::ptrdiff_t kReadFrameInput_rva = 0xD03D00; // pattern CCSGOInput_ReadFrameInput
inline constexpr std::ptrdiff_t kAddInputHistoryEntry_rva = 0xCDDA10; // pattern CCSGOInput_AddInputHistoryEntry

// --- fields ---
inline constexpr std::ptrdiff_t vtable          = 0x0   ; // void** - CCSGOInput vftable
inline constexpr std::ptrdiff_t m_FrameInput    = 0x228 ; // struct - per-frame input block (weapon select / frame data)
inline constexpr std::ptrdiff_t m_angViewAngles = 0x688 ; // QAngle - live view angles - pitch 0x688 / yaw 0x68C / roll 0x690; mouse delta is added into yaw each frame
} // namespace CCSGOInput
