# Engine Structs

Some of the structures tool authors need most never appear in a schema dump: the input singleton, the per-tick command object and its ring, the user-command protobuf messages the client serialises every tick, the view setup, the swap chain. These are reverse-engineered by hand, verified in IDA and in a working internal, and shipped as **engine structs**.

Their field offsets are curated in [`src/output/engine_structs.rs`](https://github.com/scros22/cs2-universal-offsets/blob/main/src/output/engine_structs.rs). Their function and instance addresses are **not** typed in: each names a signature from the database and is resolved from the same run's pattern pass, so they stay correct across updates for as long as the pattern does.

## The structs on build 14184

| Struct | Module | Size | What it is | Functions resolved with it |
|---|---|---|---|---|
| `CCSGOInput` | client.dll | — | The client input singleton; turns mouse/keyboard state into the per-tick user command. `m_angViewAngles` at `+0x688` (pitch / yaw `+0x68C` / roll `+0x690`; read by `GetViewAngles`, written by `SetViewAngles`) | `CreateMove`, `GetViewAngles`, `SetViewAngles`, `ProcessInputEvent`, `ReadFrameInput`, `AddInputHistoryEntry` |
| `CUserCmd` | client.dll | 0x98 | The client's command object, one per tick, in a 150-entry ring per controller. Embeds the `CCSGOUserCmdPB` that is sent to the server and the live `CInButtonState` | `GetUserCmdManager`, `GetCUserCmdBySequenceNumber` |
| `CCSGOUserCmdPB` | client.dll | 0x48 | `cs_usercmd.proto` — the top-level command message | — |
| `CBaseUserCmdPB` | client.dll | 0x88 | `usercmd.proto` — movement, buttons, view angles, subtick steps | `SerializeMoveCrc` |
| `CSubtickMoveStep` | client.dll | 0x38 | One timed button edge / analog move / view-angle delta inside a tick; the list must ascend by `when` | `CreateSubtickMoveStep` |
| `CInButtonStatePB` | client.dll | 0x30 | The button masks as sent: `buttonstate1` held, `buttonstate2` changed, `buttonstate3` scroll | `New` |
| `CCSGOInputHistoryEntryPB` | client.dll | 0x78 | Per-frame view / interpolation record the server uses for lag compensation and shot validation | `New` |
| `CSGOInterpolationInfoPB` | client.dll | 0x28 | Interpolation record (`sv_interp0/1`, `player_interp`) | — |
| `CMsgQAngle` | client.dll | 0x28 | `networkbasetypes.proto` angle | — |
| `CMsgVector` | client.dll | 0x28 | `networkbasetypes.proto` vector | — |
| `CSwapChainDx11` | rendersystemdx11.dll | — | The engine's DX11 swap-chain wrapper; `m_pSwapChain` (`IDXGISwapChain*`) at `+0x170` | `CreateSwapChain` |
| `CViewSetup` | client.dll | — | The camera description filled each frame (fov, origin, angles); written by `OverrideView`, read by the renderer | `OverrideView` |

## `engine/engine_structs.json`

```json
{
  "build_number": 14184,
  "struct_count": 12,
  "structs": [
    {
      "name": "CCSGOInput",
      "module": "client.dll",
      "desc": "Client input singleton: turns mouse/keyboard state into the per-tick user command. …",
      "size": null,
      "instance": "static object embedded in client.dll (no deref); pCSGOInput is a global POINTER to this same object",
      "instance_rva": "0x2573B40",
      "fields": [
        { "name": "vtable",          "offset": "0x0",   "type": "void**", "note": "CCSGOInput vftable" },
        { "name": "m_FrameInput",    "offset": "0x228", "type": "struct", "note": "per-frame input block (weapon select / frame data)" },
        { "name": "m_angViewAngles", "offset": "0x688", "type": "QAngle", "note": "live view angles - pitch 0x688 / yaw 0x68C / roll 0x690; mouse delta is added into yaw each frame" }
      ],
      "functions": [
        { "name": "CreateMove",    "pattern": "CreateMove",    "rva": "0xB65A70" },
        { "name": "GetViewAngles", "pattern": "GetViewAngles", "rva": "0xB70C90" },
        …
      ]
    }
  ]
}
```

`instance` describes how to reach an object of this type; `instance_rva` is filled when a signature resolves to a static instance. Every field carries a `note` that says what the value is and, where it matters, how it was confirmed.

## `engine/<struct>.h` — drop-in headers

Each struct is also written as a header of constants, ready to include on its own:

```cpp
// cusercmd.h  -  CS2 build 14184  -  cs2-sdk.com
namespace CUserCmd {
// ring = GetUserCmdManager(controller); cmd = ring + 0x98 * (sequence % 150); current sequence = *(int*)(ring + 0x5910)
inline constexpr std::size_t    kSize = 0x98;
inline constexpr std::ptrdiff_t kGetUserCmdManager_rva          = 0x944590; // pattern GetUserCmdManager
inline constexpr std::ptrdiff_t kGetCUserCmdBySequenceNumber_rva = 0x944500; // pattern GetCUserCmdBySequenceNumber

inline constexpr std::ptrdiff_t m_nCommandNumber            = 0x8;  // int64 - command / sequence number
inline constexpr std::ptrdiff_t m_csgoUserCmd               = 0x10; // CCSGOUserCmdPB - embedded protobuf message
inline constexpr std::ptrdiff_t m_csgoUserCmd_base          = 0x40; // CBaseUserCmdPB* - movement, buttons, view angles, subtick steps
inline constexpr std::ptrdiff_t m_ButtonState_m_nValue      = 0x60; // uint64 - buttons held (IN_* mask)
inline constexpr std::ptrdiff_t m_nSubtickState             = 0x94; // int32 - == 2 while the engine re-runs this command for a subtick
…
}
```

```cpp
// cswapchaindx11.h
namespace CSwapChainDx11 {
inline constexpr std::ptrdiff_t kCreateSwapChain_rva = 0x3E7D0; // pattern CSwapChainDx11_CreateSwapChain
inline constexpr std::ptrdiff_t m_pSwapChain = 0x170; // IDXGISwapChain* - vtable: 8 Present, 9 GetBuffer, 10 SetFullscreenState, 12 GetDesc, 13 ResizeBuffers, 14 ResizeTarget
}
```

There is no single swap-chain global in this engine (swap chains are per-window objects on the render device), so the create function plus the member offset is the handle: catch the instance in `CreateSwapChain` (it arrives in `rcx`) and read `+0x170` after it returns.

## Why these are separate from the schema

Schema classes come with their layout from the game. These structs are C++ types without schema metadata (`CCSGOInput`, `CUserCmd`, `CViewSetup`, `CSwapChainDx11`) or protobuf implementation types whose layout only exists in their generated parser (`*PB`). Nothing in the process describes them, so each field has to be confirmed by reading the code that uses it. That is also why the list is short: it grows one verified struct at a time. Requests are welcome — the swap chain was added because someone asked on Discord.

## API

- `GET /api/engine` — every struct with fields, functions, instance pointer and the link to its header.
- The **Engine** tab on cs2-sdk.com, and `/struct <name>` in the Discord bot.
