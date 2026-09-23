//! Read-only "live entity snapshot" — walks the CGameEntitySystem entity list
//! and decodes each entity's index, classname, and a few universal
//! `C_BaseEntity` fields (health, team, world origin) at their schema offsets.
//!
//! Reuses the chunked entity-list geometry from the weapons walk. Coverage is
//! whatever exists in the world at dump time (a genuine live snapshot). All
//! reads are best-effort + sanity-gated so non-spatial/logic entities don't
//! emit pointer garbage.

use anyhow::{Result, bail};
use memflow::prelude::v1::*;

use super::schema_lookup;
use super::schemas::SchemaMap;
use serde::Serialize;

// --- entity-list geometry (verified 14169) ---------------------------------
const CHUNK_ARRAY_BASE: u64 = 0x10;
const CHUNK_PTR_STRIDE: u64 = 0x8;
const CHUNK_ENTRY_STRIDE: u64 = 0x70;
const SLOT_INDEX_MASK: u32 = 0x1FF;
const HANDLE_INDEX_MASK: u32 = 0x7FFF;
const IDENTITY_DESIGNER_NAME: u64 = 0x20;
const MAX_ENTITY_INDEX: u32 = 16384;

// --- C_BaseEntity field offsets: LAST-KNOWN fallbacks (14169) ----------------
// `walk` resolves each from this run's schema first.
const OFF_SCENE_NODE: u64 = 0x330; // CGameSceneNode*
const OFF_MAX_HEALTH: u64 = 0x348; // int32
const OFF_HEALTH: u64 = 0x34C; // int32
const OFF_TEAM: u64 = 0x3E7; // uint8
const SCENE_ABS_ORIGIN: u64 = 0xC8; // VectorWS (3 x f32)

#[derive(Debug, Serialize)]
pub struct EntitySnapshotEntry {
    pub index: u32,
    pub classname: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub health: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub max_health: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub team: Option<u8>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub origin: Option<[f32; 3]>,
}

pub fn walk<P: Process + MemoryView>(
    process: &mut P,
    entity_system_global_va: u64,
    schemas: Option<&SchemaMap>,
) -> Result<Vec<EntitySnapshotEntry>> {
    let off_scene_node = schema_lookup::field_or(schemas, "C_BaseEntity", "m_pGameSceneNode", OFF_SCENE_NODE);
    let off_max_health = schema_lookup::field_or(schemas, "C_BaseEntity", "m_iMaxHealth", OFF_MAX_HEALTH);
    let off_health = schema_lookup::field_or(schemas, "C_BaseEntity", "m_iHealth", OFF_HEALTH);
    let off_team = schema_lookup::field_or(schemas, "C_BaseEntity", "m_iTeamNum", OFF_TEAM);
    let scene_abs_origin = schema_lookup::field_or(schemas, "CGameSceneNode", "m_vecAbsOrigin", SCENE_ABS_ORIGIN);

    let list = rd_u64(process, entity_system_global_va);
    if list == 0 {
        bail!("entity system global is null");
    }

    let mut out = Vec::new();
    for idx in 0..MAX_ENTITY_INDEX {
        let i = idx & HANDLE_INDEX_MASK;
        let chunk = rd_u64(process, list + CHUNK_ARRAY_BASE + CHUNK_PTR_STRIDE * (i >> 9) as u64);
        if chunk == 0 {
            continue;
        }
        let ident = chunk + CHUNK_ENTRY_STRIDE * (i & SLOT_INDEX_MASK) as u64;
        let inst = rd_u64(process, ident);
        if inst == 0 {
            continue;
        }
        let name_ptr = rd_u64(process, ident + IDENTITY_DESIGNER_NAME);
        let classname = rd_cstr(process, name_ptr);
        if classname.is_empty() {
            continue;
        }

        // Universal C_BaseEntity fields — sanity-gated so logic/non-spatial
        // entities that don't actually have these don't emit garbage.
        let health_raw = rd_i32(process, inst + off_health);
        let health = (-16384..=1_000_000).contains(&health_raw).then_some(health_raw);
        let max_raw = rd_i32(process, inst + off_max_health);
        let max_health = (0..=1_000_000).contains(&max_raw).then_some(max_raw);
        let team_raw = rd_u8(process, inst + off_team);
        let team = (team_raw <= 4).then_some(team_raw);

        let scene = rd_u64(process, inst + off_scene_node);
        let origin = if scene > 0x10000 {
            let o = [
                rd_f32(process, scene + scene_abs_origin),
                rd_f32(process, scene + scene_abs_origin + 4),
                rd_f32(process, scene + scene_abs_origin + 8),
            ];
            let ok = o.iter().all(|c| c.is_finite() && c.abs() < 1_048_576.0);
            ok.then_some(o)
        } else {
            None
        };

        out.push(EntitySnapshotEntry {
            index: idx,
            classname,
            health,
            max_health,
            team,
            origin,
        });
    }

    Ok(out)
}

fn rd_u64<P: MemoryView>(process: &mut P, va: u64) -> u64 {
    process.read::<u64>(Address::from(va)).data_part().unwrap_or(0)
}
fn rd_i32<P: MemoryView>(process: &mut P, va: u64) -> i32 {
    process.read::<i32>(Address::from(va)).data_part().unwrap_or(i32::MIN)
}
fn rd_u8<P: MemoryView>(process: &mut P, va: u64) -> u8 {
    process.read::<u8>(Address::from(va)).data_part().unwrap_or(0xFF)
}
fn rd_f32<P: MemoryView>(process: &mut P, va: u64) -> f32 {
    process.read::<f32>(Address::from(va)).data_part().unwrap_or(f32::NAN)
}
fn rd_cstr<P: MemoryView>(process: &mut P, ptr: u64) -> String {
    if ptr == 0 {
        return String::new();
    }
    process.read_utf8_lossy(Address::from(ptr), 64).data_part().unwrap_or_default()
}
