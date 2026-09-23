//! Curated **engine struct** layouts: the non-schema client structs tool authors
//! keep re-reversing because they never appear in schema dumps. CCSGOInput, the
//! live CUserCmd and its ring, the user-command protobuf messages the game
//! serialises every tick (CCSGOUserCmdPB, CBaseUserCmdPB, CSubtickMoveStep,
//! CInButtonStatePB, CCSGOInputHistoryEntryPB, ...), and CViewSetup.
//!
//! Field offsets are hand-verified (IDA + a working internal on build 2000914).
//! Function and instance addresses are NOT hardcoded any more: each names a
//! pattern from the DB and is resolved from this run's pattern pass, so they
//! stay correct across updates as long as the pattern does.

use serde_json::json;

use crate::patterns::{PatternHit, display_name};

pub struct EField {
    pub name: &'static str,
    pub offset: u32,
    pub ty: &'static str,
    pub note: &'static str,
}

/// A function (or global) that belongs with the struct, resolved by DB name.
pub struct EFunc {
    pub name: &'static str,
    pub pattern: &'static str,
}

pub struct EStruct {
    pub name: &'static str,
    pub module: &'static str,
    pub desc: &'static str,
    /// Size in bytes when known (protobuf impl structs / ring entries).
    pub size: Option<u32>,
    /// DB pattern resolving to the instance (global/static object), if any.
    pub instance_pattern: Option<&'static str>,
    pub instance_note: &'static str,
    pub fields: &'static [EField],
    pub functions: &'static [EFunc],
}

const PB_NOTE: &str = "protobuf message: +0x00 vtable, +0x08 internal metadata/arena, fields from +0x10 (has_bits) - offsets below are from the message start";

pub const ENGINE_STRUCTS: &[EStruct] = &[
    EStruct {
        name: "CCSGOInput",
        module: "client.dll",
        desc: "Client input singleton: turns mouse/keyboard state into the per-tick user command. Not a schema class. View angles verified in a working internal on build 2000914.",
        size: None,
        instance_pattern: Some("pCSGOInputInstance"),
        instance_note: "static object embedded in client.dll (no deref); pCSGOInput is a global POINTER to this same object",
        fields: &[
            EField { name: "vtable",            offset: 0x000, ty: "void**", note: "CCSGOInput vftable" },
            EField { name: "m_FrameInput",      offset: 0x228, ty: "struct", note: "per-frame input block (weapon select / frame data)" },
            EField { name: "m_angViewAngles",   offset: 0x688, ty: "QAngle", note: "live view angles - pitch 0x688 / yaw 0x68C / roll 0x690; mouse delta is added into yaw each frame" },
        ],
        functions: &[
            EFunc { name: "CreateMove", pattern: "CreateMove" },
            EFunc { name: "GetViewAngles", pattern: "GetViewAngles" },
            EFunc { name: "SetViewAngles", pattern: "SetViewAngles" },
            EFunc { name: "ProcessInputEvent", pattern: "CCSGOInput_ProcessInputEvent" },
            EFunc { name: "ReadFrameInput", pattern: "CCSGOInput_ReadFrameInput" },
            EFunc { name: "AddInputHistoryEntry", pattern: "CCSGOInput_AddInputHistoryEntry" },
        ],
    },
    EStruct {
        name: "CUserCmd",
        module: "client.dll",
        desc: "The client's command object (one per tick). Embeds the CCSGOUserCmdPB that is serialised to the server and the live CInButtonState. Lives in a 150-entry ring per controller.",
        size: Some(0x98),
        instance_pattern: None,
        instance_note: "ring = GetUserCmdManager(controller); cmd = ring + 0x98 * (sequence % 150); current sequence = *(int*)(ring + 0x5910)  (150 * 0x98 = 0x5910)",
        fields: &[
            EField { name: "vtable",                 offset: 0x00, ty: "void**",          note: "CUserCmd vftable" },
            EField { name: "m_nCommandNumber",       offset: 0x08, ty: "int64",           note: "command / sequence number" },
            EField { name: "m_csgoUserCmd",          offset: 0x10, ty: "CCSGOUserCmdPB",  note: "embedded protobuf message (vtable at +0x10, fields from +0x20)" },
            EField { name: "m_csgoUserCmd.has_bits", offset: 0x20, ty: "uint32",          note: "CCSGOUserCmdPB has-bits (0x1 = base present)" },
            EField { name: "m_csgoUserCmd.input_history", offset: 0x28, ty: "RepeatedPtrField<CCSGOInputHistoryEntryPB>", note: "{arena* +0x28, int size +0x30, int capacity +0x34, rep* +0x38}" },
            EField { name: "m_csgoUserCmd.base",     offset: 0x40, ty: "CBaseUserCmdPB*", note: "the base command (movement, buttons, view angles, subtick steps)" },
            EField { name: "m_csgoUserCmd.attack1_start_history_index", offset: 0x4C, ty: "int32", note: "has-bit 0x20" },
            EField { name: "m_csgoUserCmd.attack2_start_history_index", offset: 0x50, ty: "int32", note: "has-bit 0x40" },
            EField { name: "m_ButtonState",          offset: 0x58, ty: "CInButtonState",  note: "live button state (vtable +0x58)" },
            EField { name: "m_ButtonState.m_nValue",        offset: 0x60, ty: "uint64", note: "buttons held (IN_* mask)" },
            EField { name: "m_ButtonState.m_nValueChanged", offset: 0x68, ty: "uint64", note: "buttons that changed this command" },
            EField { name: "m_ButtonState.m_nValueScroll",  offset: 0x70, ty: "uint64", note: "scroll-wheel buttons" },
            EField { name: "m_nSubtickState",        offset: 0x94, ty: "int32",           note: "== 2 while the engine re-runs this same command for a subtick (repeat pass)" },
        ],
        functions: &[
            EFunc { name: "GetUserCmdManager", pattern: "GetUserCmdManager" },
            EFunc { name: "GetCUserCmdBySequenceNumber", pattern: "GetCUserCmdBySequenceNumber" },
        ],
    },
    EStruct {
        name: "CCSGOUserCmdPB",
        module: "client.dll",
        desc: "cs_usercmd.proto CSGOUserCmdPB - the top-level command message the client sends. Layout from its parser/serialiser; verified in a working internal on build 2000914.",
        size: Some(0x48),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits",                     offset: 0x10, ty: "uint32", note: "0x1 base, 0x2 left_hand_desired, 0x4 body-shot fx, 0x8 head-shot fx, 0x10 kill ragdolls, 0x20 attack1 idx, 0x40 attack2 idx" },
            EField { name: "cached_size",                  offset: 0x14, ty: "int32",  note: "" },
            EField { name: "input_history",                offset: 0x18, ty: "RepeatedPtrField<CCSGOInputHistoryEntryPB>", note: "field 2; {arena* +0x18, size +0x20, capacity +0x24, rep* +0x28}" },
            EField { name: "base",                         offset: 0x30, ty: "CBaseUserCmdPB*", note: "field 1, has-bit 0x1" },
            EField { name: "left_hand_desired",            offset: 0x38, ty: "bool",   note: "has-bit 0x2" },
            EField { name: "is_predicting_body_shot_fx",   offset: 0x39, ty: "bool",   note: "has-bit 0x4" },
            EField { name: "is_predicting_head_shot_fx",   offset: 0x3A, ty: "bool",   note: "has-bit 0x8" },
            EField { name: "is_predicting_kill_ragdolls",  offset: 0x3B, ty: "bool",   note: "has-bit 0x10" },
            EField { name: "attack1_start_history_index",  offset: 0x3C, ty: "int32",  note: "has-bit 0x20" },
            EField { name: "attack2_start_history_index",  offset: 0x40, ty: "int32",  note: "has-bit 0x40" },
        ],
        functions: &[],
    },
    EStruct {
        name: "CBaseUserCmdPB",
        module: "client.dll",
        desc: "usercmd.proto CBaseUserCmdPB - movement, buttons, view angles and subtick steps. Ground truth is its _InternalParse (field number -> offset + has-bit); the dumper's protobuf walk has mislabelled the 4-byte fields on older builds, so prefer this table.",
        size: Some(0x88),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits",                      offset: 0x10, ty: "uint32", note: "see per-field bits" },
            EField { name: "cached_size",                   offset: 0x14, ty: "int32",  note: "" },
            EField { name: "subtick_moves",                 offset: 0x18, ty: "RepeatedPtrField<CSubtickMoveStep>", note: "field 18; {arena* +0x18, size +0x20, capacity +0x24, rep* +0x28}" },
            EField { name: "move_crc",                      offset: 0x30, ty: "std::string*", note: "field 19, has-bit 0x1 - server CRC32 over buttons + view angles" },
            EField { name: "buttons_pb",                    offset: 0x38, ty: "CInButtonStatePB*", note: "field 3, has-bit 0x2" },
            EField { name: "viewangles",                    offset: 0x40, ty: "CMsgQAngle*",  note: "field 4, has-bit 0x4" },
            EField { name: "execution_notes",               offset: 0x48, ty: "void*",        note: "internal" },
            EField { name: "legacy_command_number",         offset: 0x50, ty: "int32", note: "field 1, has-bit 0x10" },
            EField { name: "client_tick",                   offset: 0x54, ty: "int32", note: "field 2, has-bit 0x20" },
            EField { name: "forwardmove",                   offset: 0x58, ty: "float", note: "field 5, has-bit 0x40" },
            EField { name: "leftmove",                      offset: 0x5C, ty: "float", note: "field 6, has-bit 0x80 (sidemove - NOT +0x58)" },
            EField { name: "upmove",                        offset: 0x60, ty: "float", note: "field 7, has-bit 0x100" },
            EField { name: "impulse",                       offset: 0x64, ty: "int32", note: "field 8, has-bit 0x200" },
            EField { name: "weaponselect",                  offset: 0x68, ty: "int32", note: "has-bit 0x400" },
            EField { name: "random_seed",                   offset: 0x6C, ty: "int32", note: "has-bit 0x800" },
            EField { name: "mousedx",                       offset: 0x70, ty: "int32", note: "has-bit 0x1000" },
            EField { name: "mousedy",                       offset: 0x74, ty: "int32", note: "has-bit 0x2000" },
            EField { name: "prediction_offset_ticks_x256",  offset: 0x78, ty: "uint32", note: "has-bit 0x4000" },
            EField { name: "consumed_server_angle_changes", offset: 0x7C, ty: "uint32", note: "has-bit 0x8000" },
            EField { name: "cmd_flags",                     offset: 0x80, ty: "int32",  note: "has-bit 0x10000" },
            EField { name: "pawn_entity_handle",            offset: 0x84, ty: "uint32", note: "has-bit 0x20000" },
        ],
        functions: &[EFunc { name: "SerializeMoveCrc", pattern: "CBaseUserCmdPB_SerializeMoveCrc" }],
    },
    EStruct {
        name: "CSubtickMoveStep",
        module: "client.dll",
        desc: "usercmd.proto CSubtickMoveStep - one timed button edge / analog move / view-angle delta inside a tick. The list must ascend by `when`.",
        size: Some(0x38),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits",             offset: 0x10, ty: "uint32", note: "" },
            EField { name: "cached_size",          offset: 0x14, ty: "int32",  note: "" },
            EField { name: "button",               offset: 0x18, ty: "uint64", note: "IN_* bit, has-bit 0x1" },
            EField { name: "pressed",              offset: 0x20, ty: "bool",   note: "has-bit 0x2" },
            EField { name: "when",                 offset: 0x24, ty: "float",  note: "fraction of the tick [0,1), has-bit 0x4" },
            EField { name: "analog_forward_delta", offset: 0x28, ty: "float",  note: "has-bit 0x8" },
            EField { name: "analog_left_delta",    offset: 0x2C, ty: "float",  note: "has-bit 0x10" },
            EField { name: "pitch_delta",          offset: 0x30, ty: "float",  note: "delta vs the command's base view angle, has-bit 0x20" },
            EField { name: "yaw_delta",            offset: 0x34, ty: "float",  note: "delta vs the command's base view angle, has-bit 0x40" },
        ],
        functions: &[EFunc { name: "CreateSubtickMoveStep", pattern: "CreateSubtickMoveStep" }],
    },
    EStruct {
        name: "CInButtonStatePB",
        module: "client.dll",
        desc: "usercmd.proto CInButtonStatePB - the button masks as sent: buttonstate1 = held, buttonstate2 = changed, buttonstate3 = scroll.",
        size: Some(0x30),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits",     offset: 0x10, ty: "uint32", note: "" },
            EField { name: "cached_size",  offset: 0x14, ty: "int32",  note: "" },
            EField { name: "buttonstate1", offset: 0x18, ty: "uint64", note: "held (IN_* mask), has-bit 0x1" },
            EField { name: "buttonstate2", offset: 0x20, ty: "uint64", note: "changed vs the previous SENT command, has-bit 0x2" },
            EField { name: "buttonstate3", offset: 0x28, ty: "uint64", note: "scroll, has-bit 0x4" },
        ],
        functions: &[EFunc { name: "New", pattern: "CInButtonStatePB_New" }],
    },
    EStruct {
        name: "CCSGOInputHistoryEntryPB",
        module: "client.dll",
        desc: "cs_usercmd.proto CSGOInputHistoryEntryPB - per-frame view/interp record the server uses for lag compensation and shot validation.",
        size: Some(0x78),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits",               offset: 0x10, ty: "uint32", note: "" },
            EField { name: "cached_size",            offset: 0x14, ty: "int32",  note: "" },
            EField { name: "view_angles",            offset: 0x18, ty: "CMsgQAngle*",  note: "has-bit 0x1" },
            EField { name: "cl_interp",              offset: 0x20, ty: "CSGOInterpolationInfoPB_CL*", note: "has-bit 0x2" },
            EField { name: "sv_interp0",             offset: 0x28, ty: "CSGOInterpolationInfoPB*", note: "has-bit 0x4" },
            EField { name: "sv_interp1",             offset: 0x30, ty: "CSGOInterpolationInfoPB*", note: "has-bit 0x8" },
            EField { name: "player_interp",          offset: 0x38, ty: "CSGOInterpolationInfoPB*", note: "has-bit 0x10" },
            EField { name: "shoot_position",         offset: 0x40, ty: "CMsgVector*", note: "has-bit 0x20" },
            EField { name: "target_head_pos_check",  offset: 0x48, ty: "CMsgVector*", note: "has-bit 0x40" },
            EField { name: "target_abs_pos_check",   offset: 0x50, ty: "CMsgVector*", note: "has-bit 0x80" },
            EField { name: "target_abs_ang_check",   offset: 0x58, ty: "CMsgQAngle*", note: "has-bit 0x100" },
            EField { name: "render_tick_count",      offset: 0x60, ty: "int32", note: "has-bit 0x200" },
            EField { name: "render_tick_fraction",   offset: 0x64, ty: "float", note: "has-bit 0x400" },
            EField { name: "player_tick_count",      offset: 0x68, ty: "int32", note: "has-bit 0x800" },
            EField { name: "player_tick_fraction",   offset: 0x6C, ty: "float", note: "has-bit 0x1000" },
            EField { name: "frame_number",           offset: 0x70, ty: "int32", note: "has-bit 0x2000" },
            EField { name: "target_ent_index",       offset: 0x74, ty: "int32", note: "has-bit 0x4000" },
        ],
        functions: &[EFunc { name: "New", pattern: "CCSGOInputHistoryEntryPB_New" }],
    },
    EStruct {
        name: "CSGOInterpolationInfoPB",
        module: "client.dll",
        desc: "cs_usercmd.proto interpolation record (sv_interp0/1, player_interp). The _CL variant (cl_interp) only has frac at +0x18.",
        size: Some(0x28),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits",  offset: 0x10, ty: "uint32", note: "" },
            EField { name: "frac",      offset: 0x18, ty: "float",  note: "has-bit 0x1" },
            EField { name: "src_tick",  offset: 0x1C, ty: "int32",  note: "has-bit 0x2" },
            EField { name: "dst_tick",  offset: 0x20, ty: "int32",  note: "has-bit 0x4" },
        ],
        functions: &[],
    },
    EStruct {
        name: "CMsgQAngle",
        module: "client.dll",
        desc: "networkbasetypes.proto CMsgQAngle.",
        size: Some(0x28),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits", offset: 0x10, ty: "uint32", note: "x 0x1, y 0x2, z 0x4" },
            EField { name: "x",        offset: 0x18, ty: "float",  note: "pitch" },
            EField { name: "y",        offset: 0x1C, ty: "float",  note: "yaw" },
            EField { name: "z",        offset: 0x20, ty: "float",  note: "roll" },
        ],
        functions: &[],
    },
    EStruct {
        name: "CMsgVector",
        module: "client.dll",
        desc: "networkbasetypes.proto CMsgVector.",
        size: Some(0x28),
        instance_pattern: None,
        instance_note: PB_NOTE,
        fields: &[
            EField { name: "has_bits", offset: 0x10, ty: "uint32", note: "x 0x1, y 0x2, z 0x4" },
            EField { name: "x",        offset: 0x18, ty: "float",  note: "" },
            EField { name: "y",        offset: 0x1C, ty: "float",  note: "" },
            EField { name: "z",        offset: 0x20, ty: "float",  note: "" },
            EField { name: "w",        offset: 0x24, ty: "float",  note: "" },
        ],
        functions: &[],
    },
    EStruct {
        name: "CViewSetup",
        module: "client.dll",
        desc: "The camera/view description filled each frame (fov, origin, angles). Written by OverrideView; read by the renderer. Not a schema class. Verified on build 2000914.",
        size: None,
        instance_pattern: None,
        instance_note: "passed to OverrideView in rdx",
        fields: &[
            EField { name: "m_flFov",         offset: 0x498, ty: "float",  note: "field of view" },
            EField { name: "m_vecOrigin",     offset: 0x4A0, ty: "Vector", note: "world eye origin - x 0x4A0 / y 0x4A4 / z 0x4A8" },
            EField { name: "m_angViewAngles", offset: 0x4B8, ty: "QAngle", note: "view angles - pitch 0x4B8 / yaw 0x4BC / roll 0x4C0" },
        ],
        functions: &[EFunc { name: "OverrideView", pattern: "OverrideView" }],
    },
];

fn lookup<'a>(hits: &'a [PatternHit], raw: &str) -> Option<&'a PatternHit> {
    let want = display_name(raw);
    hits.iter().find(|h| h.found && (h.name == want || h.name == raw))
}

fn hex(v: Option<u64>) -> Option<String> {
    v.map(|r| format!("0x{:X}", r))
}

/// Structured JSON of every engine struct, with function/instance RVAs taken
/// from this run's pattern pass.
pub fn render_json(build: Option<u32>, hits: &[PatternHit]) -> String {
    let structs: Vec<_> = ENGINE_STRUCTS
        .iter()
        .map(|s| {
            let inst = s.instance_pattern.and_then(|p| lookup(hits, p)).and_then(|h| h.rva);
            json!({
                "name": s.name,
                "module": s.module,
                "desc": s.desc,
                "size": s.size.map(|v| format!("0x{:X}", v)),
                "instance_rva": hex(inst),
                "instance": s.instance_note,
                "fields": s.fields.iter().map(|f| json!({
                    "name": f.name,
                    "offset": format!("0x{:X}", f.offset),
                    "type": f.ty,
                    "note": f.note,
                })).collect::<Vec<_>>(),
                "functions": s.functions.iter().map(|fn_| json!({
                    "name": fn_.name,
                    "pattern": fn_.pattern,
                    "rva": hex(lookup(hits, fn_.pattern).and_then(|h| h.rva)),
                })).collect::<Vec<_>>(),
            })
        })
        .collect();

    serde_json::to_string_pretty(&json!({
        "build_number": build,
        "note": "Non-schema client struct layouts, hand-verified. Function/instance RVAs are resolved from this dump's patterns.",
        "struct_count": ENGINE_STRUCTS.len(),
        "structs": structs,
    }))
    .unwrap_or_else(|_| "{}".into())
}

/// Drop-in C++ header for a single struct (offset constants + notes).
pub fn render_header(s: &EStruct, build: Option<u32>, hits: &[PatternHit]) -> String {
    let mut out = String::new();
    out.push_str(&format!(
        "// {}.h  -  CS2{}  -  cs2-sdk.com\n",
        s.name.to_ascii_lowercase(),
        build.map(|b| format!(" build {b}")).unwrap_or_default()
    ));
    out.push_str(&format!("// {}\n", s.desc));
    out.push_str(&format!("// Module: {}. Offsets drift between builds - regenerate after a CS2 update.\n", s.module));
    out.push_str(&format!("#pragma once\n#include <cstddef>\n#include <cstdint>\n\nnamespace {} {{\n", s.name));

    out.push_str(&format!("\n// {}\n", s.instance_note));
    if let Some(p) = s.instance_pattern {
        match lookup(hits, p).and_then(|h| h.rva) {
            Some(rva) => out.push_str(&format!("inline constexpr std::ptrdiff_t kInstance_rva = 0x{:X}; // pattern {}\n", rva, p)),
            None => out.push_str(&format!("// kInstance_rva: pattern {} did not resolve in this dump\n", p)),
        }
    }
    if let Some(sz) = s.size {
        out.push_str(&format!("inline constexpr std::size_t kSize = 0x{:X};\n", sz));
    }
    for fn_ in s.functions {
        match lookup(hits, fn_.pattern).and_then(|h| h.rva) {
            Some(rva) => out.push_str(&format!("inline constexpr std::ptrdiff_t k{}_rva = 0x{:X}; // pattern {}\n", fn_.name, rva, fn_.pattern)),
            None => out.push_str(&format!("// k{}_rva: pattern {} did not resolve in this dump\n", fn_.name, fn_.pattern)),
        }
    }

    out.push_str("\n// --- fields ---\n");
    let ident = |n: &str| n.replace('.', "_");
    let w = s.fields.iter().map(|f| ident(f.name).len()).max().unwrap_or(0);
    for f in s.fields {
        let note = if f.note.is_empty() { String::new() } else { format!(" - {}", f.note) };
        out.push_str(&format!(
            "inline constexpr std::ptrdiff_t {:<w$} = 0x{:<4X}; // {}{}\n",
            ident(f.name), f.offset, f.ty, note,
            w = w
        ));
    }
    out.push_str(&format!("}} // namespace {}\n", s.name));
    out
}
