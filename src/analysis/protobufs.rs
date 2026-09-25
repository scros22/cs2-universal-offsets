//! Protobuf message-layout dumper.
//!
//! CS2's usercmd / netmessage types are **Google protobuf**, not Source2
//! schema, so the schema pass never sees them. But libprotobuf stores the
//! exact in-memory field layout in the per-file reflection tables, so we can
//! recover it precisely (offsets + has-bits) instead of hand-RE'ing it.
//!
//! For each generated `.pb.cc` the binary holds a `DescriptorTable`:
//! ```text
//!   +0x04 i32   descriptor size
//!   +0x08 ptr   serialized FileDescriptorProto  (starts `0A <len> "<name>.proto"`)
//!   +0x10 ptr   filename "<name>.proto"
//!   +0x2C i32   num_messages
//!   +0x30 ptr   schemas[]   (MigrationSchema, 16 bytes each)
//!   +0x40 ptr   offsets[]   (u32 array)
//! ```
//! `MigrationSchema = { i32 offsets_index, i32 has_bit_indices_index, i32 _, u32 object_size }`.
//! `offsets[]` per message = `[6-entry header][field offsets...][has-bit indices...]`,
//! header[0] = `_has_bits_` byte offset. Field offsets/has-bit indices are in
//! the descriptor's DECLARATION order (not field-number order). Verified against CS2's
//! `CSubtickMoveStep` (fields @ 0x18/0x20/0x24, has-bits 0/1/2).

use std::collections::BTreeMap;

use anyhow::Result;
use log::debug;
use memflow::prelude::v1::*;

/// module name -> messages in that module.
pub type ProtobufMap = BTreeMap<String, Vec<ProtoMessage>>;

#[derive(Debug, Default, Clone)]
pub struct ProtoMessage {
    pub name: String,
    pub size: u32,
    pub has_bits_offset: Option<u32>,
    pub fields: Vec<ProtoField>,
}

#[derive(Debug, Default, Clone)]
pub struct ProtoField {
    pub name: String,
    pub number: i64,
    pub offset: u32,
    pub has_bit: Option<u32>,
    pub label: i64, // 1=optional 2=required 3=repeated
    pub ty: i64,    // FieldDescriptorProto.Type
    pub type_name: String,
}

/// Modules known to carry generated protobuf message tables.
const MODULES: &[&str] = &["client.dll", "engine2.dll", "networksystem.dll", "server.dll"];

const HEADER_ENTRIES: i64 = 6; // has_bits, metadata, ext, oneof, weak, inlined

pub fn protobufs<P: Process + MemoryView>(process: &mut P) -> Result<ProtobufMap> {
    let mut out = ProtobufMap::new();
    for &module_name in MODULES {
        let module = match process.module_by_name(module_name) {
            Ok(m) => m,
            Err(_) => continue,
        };
        let base: u64 = module.base.to_umem() as u64;
        let buf = match process.read_raw(module.base, module.size as _).data_part() {
            Ok(b) => b,
            Err(_) => continue,
        };

        let msgs = scan_module(&buf, base);
        if !msgs.is_empty() {
            debug!("[protobuf] {}: {} messages", module_name, msgs.len());
            out.insert(module_name.to_string(), msgs);
        }
    }
    Ok(out)
}

fn rd_u32(buf: &[u8], rva: usize) -> Option<u32> {
    buf.get(rva..rva + 4).map(|b| u32::from_le_bytes(b.try_into().unwrap()))
}
fn rd_i32(buf: &[u8], rva: usize) -> Option<i32> {
    rd_u32(buf, rva).map(|v| v as i32)
}
fn rd_u64(buf: &[u8], rva: usize) -> Option<u64> {
    buf.get(rva..rva + 8).map(|b| u64::from_le_bytes(b.try_into().unwrap()))
}
/// Convert an in-image VA pointer to a buffer RVA, if it lands inside the module.
fn va_to_rva(va: u64, base: u64, len: usize) -> Option<usize> {
    let rva = va.checked_sub(base)? as usize;
    if rva < len { Some(rva) } else { None }
}

fn scan_module(buf: &[u8], base: u64) -> Vec<ProtoMessage> {
    let mut messages = Vec::new();

    // 1) Locate every serialized FileDescriptorProto: `0A <len:varint> "<name>.proto"`.
    //    Record the blob's VA so we can find the DescriptorTable that points at it.
    let mut blob_vas: BTreeMap<u64, ()> = BTreeMap::new();
    let mut i = 0usize;
    while i + 2 < buf.len() {
        if buf[i] == 0x0A {
            if let Some((name_len, hdr)) = read_varint(buf, i + 1) {
                let name_start = i + 1 + hdr;
                let name_end = name_start + name_len as usize;
                if name_len > 3
                    && name_len < 128
                    && name_end <= buf.len()
                    && buf[name_start..name_end].ends_with(b".proto")
                    && buf[name_start..name_end].iter().all(|&c| c.is_ascii_graphic())
                {
                    blob_vas.insert(base + i as u64, ());
                }
            }
        }
        i += 1;
    }
    if blob_vas.is_empty() {
        return messages;
    }

    // 2) One aligned pass: any 8-byte slot holding a blob VA is a DescriptorTable's
    //    `descriptor` field (+0x08), so the table starts 8 bytes earlier. A blob can
    //    be referenced by several pointers — only parse each file once (first table
    //    that validates wins).
    let mut done: std::collections::BTreeSet<u64> = std::collections::BTreeSet::new();
    let mut p = 0usize;
    while p + 8 <= buf.len() {
        let val = u64::from_le_bytes(buf[p..p + 8].try_into().unwrap());
        if p >= 8 && blob_vas.contains_key(&val) && !done.contains(&val) {
            if let Some(file) = parse_descriptor_table(buf, base, p - 8) {
                if !file.is_empty() {
                    done.insert(val);
                    messages.extend(file);
                }
            }
        }
        p += 8;
    }
    messages
}

fn parse_descriptor_table(buf: &[u8], base: u64, t: usize) -> Option<Vec<ProtoMessage>> {
    let desc_size = rd_i32(buf, t + 0x04)? as usize;
    let desc_va = rd_u64(buf, t + 0x08)?;
    let filename_va = rd_u64(buf, t + 0x10)?;
    let num_messages = rd_i32(buf, t + 0x2C)?;
    let schemas_va = rd_u64(buf, t + 0x30)?;
    let offsets_va = rd_u64(buf, t + 0x40)?;

    if !(1..=4096).contains(&num_messages) {
        return None;
    }
    let desc_rva = va_to_rva(desc_va, base, buf.len())?;
    let _ = va_to_rva(filename_va, base, buf.len())?;
    let schemas_rva = va_to_rva(schemas_va, base, buf.len())?;
    let offsets_rva = va_to_rva(offsets_va, base, buf.len())?;
    if desc_size < 4 || desc_rva + desc_size > buf.len() {
        return None;
    }

    // Parse the FileDescriptorProto for message names + fields (number order).
    let descriptor = &buf[desc_rva..desc_rva + desc_size];
    let mut proto_messages = Vec::new();
    parse_file_descriptor(descriptor, &mut proto_messages);
    if proto_messages.is_empty() {
        return None;
    }

    let mut out = Vec::new();
    for (idx, pm) in proto_messages.iter().enumerate() {
        if idx as i32 >= num_messages {
            break;
        }
        let s = schemas_rva + idx * 16;
        let offsets_index = rd_i32(buf, s)? as i64;
        let has_bit_index = rd_i32(buf, s + 4)? as i64;
        let object_size = rd_u32(buf, s + 12)?;
        if offsets_index < 0 {
            continue;
        }

        let oi = offsets_index as usize;
        let has_bits_offset = rd_u32(buf, offsets_rva + oi * 4).filter(|&v| v != u32::MAX);

        let mut fields = Vec::new();
        for (j, f) in pm.fields.iter().enumerate() {
            let off = rd_u32(buf, offsets_rva + (oi + HEADER_ENTRIES as usize + j) * 4)?;
            let has_bit = if has_bit_index >= 0 {
                rd_u32(buf, offsets_rva + (has_bit_index as usize + j) * 4)
                    .filter(|&v| v != u32::MAX)
            } else {
                None
            };
            fields.push(ProtoField {
                name: f.name.clone(),
                number: f.number,
                offset: off,
                has_bit,
                label: f.label,
                ty: f.ty,
                type_name: f.type_name.clone(),
            });
        }

        out.push(ProtoMessage {
            name: pm.name.clone(),
            size: object_size,
            has_bits_offset,
            fields,
        });
    }
    Some(out)
}

// ---- minimal protobuf wire reader for descriptor.proto ---------------------

#[derive(Default)]
struct PField {
    name: String,
    number: i64,
    label: i64,
    ty: i64,
    type_name: String,
}
#[derive(Default)]
struct PMessage {
    name: String,
    fields: Vec<PField>,
}

/// LEB128 varint. Returns (value, bytes_consumed).
fn read_varint(buf: &[u8], mut pos: usize) -> Option<(u64, usize)> {
    let start = pos;
    let mut val: u64 = 0;
    let mut shift = 0;
    loop {
        let b = *buf.get(pos)?;
        val |= ((b & 0x7F) as u64) << shift;
        pos += 1;
        if b & 0x80 == 0 {
            break;
        }
        shift += 7;
        if shift >= 64 {
            return None;
        }
    }
    Some((val, pos - start))
}

/// Iterate top-level fields of a protobuf message, calling `f(field_number, wire, payload)`.
/// `payload` is the raw slice for length-delimited fields, or the varint bytes otherwise.
/// `payload` borrows `buf` (`'a`) so length-delimited slices may be collected.
fn for_each_field<'a, F: FnMut(u64, u64, &'a [u8])>(buf: &'a [u8], mut f: F) {
    let mut pos = 0usize;
    while pos < buf.len() {
        let (tag, n) = match read_varint(buf, pos) {
            Some(v) => v,
            None => break,
        };
        pos += n;
        let field = tag >> 3;
        let wire = tag & 7;
        match wire {
            0 => {
                let (_, n) = match read_varint(buf, pos) {
                    Some(v) => v,
                    None => break,
                };
                f(field, wire, &buf[pos..pos + n]);
                pos += n;
            }
            2 => {
                let (len, n) = match read_varint(buf, pos) {
                    Some(v) => v,
                    None => break,
                };
                pos += n;
                let end = (pos + len as usize).min(buf.len());
                f(field, wire, &buf[pos..end]);
                pos = end;
            }
            1 => {
                f(field, wire, &buf[pos..(pos + 8).min(buf.len())]);
                pos += 8;
            }
            5 => {
                f(field, wire, &buf[pos..(pos + 4).min(buf.len())]);
                pos += 4;
            }
            _ => break,
        }
    }
}

fn varint_of(payload: &[u8]) -> i64 {
    read_varint(payload, 0).map(|(v, _)| v as i64).unwrap_or(0)
}

/// FileDescriptorProto: field 4 = message_type (DescriptorProto, repeated).
fn parse_file_descriptor(buf: &[u8], out: &mut Vec<PMessage>) {
    for_each_field(buf, |field, wire, payload| {
        if field == 4 && wire == 2 {
            parse_descriptor_proto(payload, "", out);
        }
    });
}

/// DescriptorProto: 1=name, 2=field (FieldDescriptorProto), 3=nested_type.
/// Flatten pre-order (message then nested) to match protoc's `FlattenMessagesInFile`
/// indexing. Nested messages are qualified `Parent_Nested` (protoc's C++ naming)
/// so distinct nested types with the same short name don't collide.
fn parse_descriptor_proto(buf: &[u8], prefix: &str, out: &mut Vec<PMessage>) {
    let mut name = String::new();
    let mut fields = Vec::new();
    let mut nested: Vec<&[u8]> = Vec::new();
    for_each_field(buf, |field, wire, payload| match (field, wire) {
        (1, 2) => name = String::from_utf8_lossy(payload).into_owned(),
        (2, 2) => fields.push(parse_field_descriptor(payload)),
        (3, 2) => nested.push(payload),
        _ => {}
    });
    // Keep DECLARATION order: `offsets[]` and the has-bit indices are indexed by
    // the descriptor's field index, i.e. the order the fields appear in the
    // .proto, which is not always ascending field number. CBaseUserCmdPB declares
    // prediction_offset_ticks_x256 (= 17) third and pawn_entity_handle (= 14)
    // after mousedy; sorting by number shifted every field between them by one
    // slot (verified against CBaseUserCmdPB::_InternalParse, client 0x4F8400 on
    // build 14184). The emitter orders the struct by offset anyway.
    let full = if prefix.is_empty() {
        name
    } else {
        format!("{prefix}_{name}")
    };
    out.push(PMessage { name: full.clone(), fields });
    for n in nested {
        parse_descriptor_proto(n, &full, out);
    }
}

/// FieldDescriptorProto: 1=name, 3=number, 4=label, 5=type, 6=type_name.
fn parse_field_descriptor(buf: &[u8]) -> PField {
    let mut f = PField::default();
    for_each_field(buf, |field, wire, payload| match (field, wire) {
        (1, 2) => f.name = String::from_utf8_lossy(payload).into_owned(),
        (3, 0) => f.number = varint_of(payload),
        (4, 0) => f.label = varint_of(payload),
        (5, 0) => f.ty = varint_of(payload),
        (6, 2) => f.type_name = String::from_utf8_lossy(payload).into_owned(),
        _ => {}
    });
    f
}
