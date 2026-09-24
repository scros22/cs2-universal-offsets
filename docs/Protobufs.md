# Protobufs

CS2 talks to the server, the Game Coordinator and itself in protobuf messages, and the client keeps live copies of many of them (`CBaseUserCmdPB`, `CCSGOInputHistoryEntryPB`, …). The dumper reads libprotobuf's reflection tables from the running modules and emits the exact in-memory layout of every message — field offsets, field numbers, has-bits — so you can cast a live message pointer and read it directly. Build 14183: 2,003 messages across 4 modules.

## `protobufs/protobufs.hpp`

```cpp
namespace pb::client {
    #pragma pack(push, 1)
    struct CBaseUserCmdPB { // sizeof 0x88, _has_bits_ @ 0x10
        uint8_t _pad_0[0x18];
        pb::RepeatedPtrField<pb::client::CSubtickMoveStep> subtick_moves; // #18 repeated message CSubtickMoveStep, no has-bit
        pb::string_t* move_crc;              // #19 bytes, has-bit 0
        pb::client::CMsgQAngle* viewangles;  // #4 message CMsgQAngle, has-bit 1
        float forwardmove;                   // #5 float, has-bit 2
        uint8_t _pad_44[0x4];
        pb::client::CBaseUserCmdExecutionNotes* execution_notes; // #22 message, has-bit 3
        int32_t legacy_command_number;       // #1 int32, has-bit 4
        int32_t client_tick;                 // #2 int32, has-bit 5
        float leftmove;                      // #6 float, has-bit 6
        …
        static constexpr std::ptrdiff_t kSizeOf  = 0x88;
        static constexpr std::ptrdiff_t kHasBits = 0x10;
    };
    #pragma pack(pop)
    static_assert(sizeof(CBaseUserCmdPB) == 0x88);
    static_assert(offsetof(CBaseUserCmdPB, client_tick) == 0x54);
}
```

- Structs are `#pragma pack(1)` with explicit padding, and every field offset and the total size are `static_assert`ed, so the header cannot silently drift from the JSON.
- Sub-messages are pointers (`CMsgQAngle*`), which is how libprotobuf stores optional message fields. Strings and bytes are `pb::string_t*` (an opaque `ArenaStringPtr` target).
- Repeated fields are `pb::RepeatedField<T>` / `pb::RepeatedPtrField<T>` — `{ arena, current_size, total_size, elements }`, 0x18 bytes on x64.
- A field is present when its has-bit is set:

```cpp
auto* cmd = reinterpret_cast<pb::client::CBaseUserCmdPB*>(msg);
auto has_bits = *reinterpret_cast<std::uint32_t*>(reinterpret_cast<char*>(cmd) + pb::client::CBaseUserCmdPB::kHasBits);
bool has_viewangles = (has_bits >> 1) & 1;   // viewangles: has-bit 1
```

## `protobufs/protobufs.json`

```json
{
  "client.dll": {
    "CSubtickMoveStep": {
      "size": 56,
      "has_bits_offset": 16,
      "fields": {
        "button":  { "offset": 24, "number": 1, "has_bit": 0, "type": 4, "label": 1 },
        "pressed": { "offset": 32, "number": 2, "has_bit": 1, "type": 8, "label": 1 },
        "when":    { "offset": 36, "number": 3, "has_bit": 2, "type": 2, "label": 1 },
        …
      }
    }
  }
}
```

`type` and `label` are libprotobuf's `FieldDescriptor` codes:

| `type` | | `type` | | `label` | |
|---|---|---|---|---|---|
| 1 | double | 10 | group | 1 | optional |
| 2 | float | 11 | message | 2 | required |
| 3 | int64 | 12 | bytes | 3 | repeated |
| 4 | uint64 | 13 | uint32 | | |
| 5 | int32 | 14 | enum | | |
| 6 | fixed64 | 15 | sfixed32 | | |
| 7 | fixed32 | 16 | sfixed64 | | |
| 8 | bool | 17 | sint32 | | |
| 9 | string | 18 | sint64 | | |

Offsets and sizes are decimal.

## User commands

The user-command messages are also documented as [engine structs](Engine-Structs.md) (`CBaseUserCmdPB`, `CCSGOUserCmdPB`, `CSubtickMoveStep`, `CInButtonStatePB`, `CCSGOInputHistoryEntryPB`, `CSGOInterpolationInfoPB`, `CMsgQAngle`, `CMsgVector`), with a note per field on what the game does with the value. The two views agree; the engine-struct one is the annotated version.

## Net messages

`GET /api/netmessages` joins the message ids the engine routes (`NET_Messages`, `SVC_Messages`, `CLC_Messages`, the user-message and entity-message enums — 196 ids on build 14183, read from the schema enums) to their names and, where the client holds one, to the protobuf layout above. `?group=SVC` and `?q=usercmd` filter. The **Net Messages** tab on the site shows the same table.

## API

- `GET /api/protobufs` — everything; `?module=client.dll` and `?message=CBaseUserCmdPB` filter.
