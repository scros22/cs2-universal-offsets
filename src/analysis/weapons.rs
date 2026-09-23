//! Read-only walker that builds a per-weapon gameplay-values table by reading
//! each spawned weapon entity's `CCSWeaponBaseVData` from live memory.
//!
//! The schema dump already gives the `CCSWeaponBaseVData` field offsets; this
//! reads the actual VALUES. Path (build 14169):
//!   entity list  (via the `pEntitySystem` pattern global)
//!   -> chunked CGameEntitySystem: chunk[i>>9] @ (list + 0x10 + 8*(i>>9))
//!   -> CEntityIdentity inline @ chunk + 0x70*(i & 0x1FF)
//!        identity+0x00 = CEntityInstance*   identity+0x20 = designer-name char*
//!   -> weapon entities (designer name "weapon_*")
//!   -> CCSWeaponBaseVData* at entity + <vdata offset>  (offset validated, see below)
//!
//! Coverage note: only weapons that exist as ENTITIES in the dumped session are
//! captured (held/dropped weapons + view models). Running the dump inside a
//! match / deathmatch / practice-with-bots maximises coverage.

use std::collections::BTreeMap;

use anyhow::{Result, bail};
use memflow::prelude::v1::*;

use super::schema_lookup;
use super::schemas::SchemaMap;
use serde::Serialize;

// --- CGameEntitySystem chunk geometry (verified 14169) ---------------------
const CHUNK_ARRAY_BASE: u64 = 0x10;
const CHUNK_PTR_STRIDE: u64 = 0x8;
const CHUNK_ENTRY_STRIDE: u64 = 0x70;
const SLOT_INDEX_MASK: u32 = 0x1FF;
const HANDLE_INDEX_MASK: u32 = 0x7FFF;
const IDENTITY_DESIGNER_NAME: u64 = 0x20;
const MAX_ENTITY_INDEX: u32 = 8192;

/// Candidate offsets of the `CCSWeaponBaseVData*` within a weapon entity, tried
/// AFTER the schema-derived one (`C_BaseEntity::m_nSubclassID` + 8, where the
/// game keeps the resolved subclass vdata). Each is validated by dereferencing
/// to a vdata with sane values, so drift picks the right one, never garbage.
const VDATA_PTR_CANDIDATES: &[u64] = &[0x388, 0x340, 0x348, 0x338, 0x350, 0x358, 0x390];

// --- CCSWeaponBaseVData field offsets: LAST-KNOWN fallbacks (14169) ----------
// The walker resolves each from this run's schema first (see `Offsets`).
// CFiringModeFloat fields store the primary-fire value at the field offset.
const F_PRICE: u64 = 0x70C; // int32
const F_NUM_BULLETS: u64 = 0x738; // int32
const F_CYCLE_TIME: u64 = 0x740; // float (primary)
const F_MAX_SPEED: u64 = 0x750; // float (primary)
const F_SPREAD: u64 = 0x758; // float (primary)
const F_INACCURACY_STAND: u64 = 0x768; // float (primary)
const F_INACCURACY_MOVE: u64 = 0x790; // float (primary)
const F_RECOIL_MAGNITUDE: u64 = 0x7A8; // float (primary)
const F_DAMAGE: u64 = 0x828; // int32
const F_HEADSHOT_MULT: u64 = 0x82C; // float
const F_ARMOR_RATIO: u64 = 0x830; // float
const F_PENETRATION: u64 = 0x834; // float
const F_RANGE: u64 = 0x838; // float
const F_RANGE_MODIFIER: u64 = 0x83C; // float

#[derive(Debug, Serialize)]
pub struct Weapon {
    pub name: String,
    pub damage: i32,
    pub headshot_multiplier: f32,
    pub armor_ratio: f32,
    pub penetration: f32,
    pub range: f32,
    pub range_modifier: f32,
    pub cycle_time: f32,
    pub price: i32,
    pub num_bullets: i32,
    pub max_speed: f32,
    pub spread: f32,
    pub inaccuracy_stand: f32,
    pub inaccuracy_move: f32,
    pub recoil_magnitude: f32,
    /// VA of the CCSWeaponBaseVData object.
    pub address: u64,
    /// Offset the vdata pointer was found at (diagnostic).
    pub vdata_ptr_offset: u64,
}

/// Walk the entity list and collect one entry per distinct weapon.
/// `entity_system_global_va` is the resolved address of the `pEntitySystem`
/// global (a `CGameEntitySystem*`).
/// Weapon-vdata field offsets for this run: schema first, last-known fallback.
struct Offsets {
    vdata_ptr: Option<u64>,
    price: u64,
    num_bullets: u64,
    cycle_time: u64,
    max_speed: u64,
    spread: u64,
    inaccuracy_stand: u64,
    inaccuracy_move: u64,
    recoil_magnitude: u64,
    damage: u64,
    headshot_mult: u64,
    armor_ratio: u64,
    penetration: u64,
    range: u64,
    range_modifier: u64,
}

impl Offsets {
    fn resolve(schemas: Option<&SchemaMap>) -> Self {
        const VD: &str = "CCSWeaponBaseVData";
        let f = |name: &str, fb: u64| schema_lookup::field_or(schemas, VD, name, fb);
        Self {
            vdata_ptr: schema_lookup::field(schemas, "C_BaseEntity", "m_nSubclassID").map(|o| o + 8),
            price: f("m_nPrice", F_PRICE),
            num_bullets: f("m_nNumBullets", F_NUM_BULLETS),
            cycle_time: f("m_flCycleTime", F_CYCLE_TIME),
            max_speed: f("m_flMaxSpeed", F_MAX_SPEED),
            spread: f("m_flSpread", F_SPREAD),
            inaccuracy_stand: f("m_flInaccuracyStand", F_INACCURACY_STAND),
            inaccuracy_move: f("m_flInaccuracyMove", F_INACCURACY_MOVE),
            recoil_magnitude: f("m_flRecoilMagnitude", F_RECOIL_MAGNITUDE),
            damage: f("m_nDamage", F_DAMAGE),
            headshot_mult: f("m_flHeadshotMultiplier", F_HEADSHOT_MULT),
            armor_ratio: f("m_flArmorRatio", F_ARMOR_RATIO),
            penetration: f("m_flPenetration", F_PENETRATION),
            range: f("m_flRange", F_RANGE),
            range_modifier: f("m_flRangeModifier", F_RANGE_MODIFIER),
        }
    }
}

pub fn walk<P: Process + MemoryView>(
    process: &mut P,
    entity_system_global_va: u64,
    schemas: Option<&SchemaMap>,
) -> Result<Vec<Weapon>> {
    let offs = Offsets::resolve(schemas);
    let list = rd_u64(process, entity_system_global_va);
    if list == 0 {
        bail!("entity system global is null");
    }

    let mut by_name: BTreeMap<String, Weapon> = BTreeMap::new();
    for idx in 0..MAX_ENTITY_INDEX {
        let i = idx & HANDLE_INDEX_MASK;
        let chunk = rd_u64(process, list + CHUNK_ARRAY_BASE + CHUNK_PTR_STRIDE * (i >> 9) as u64);
        if chunk == 0 {
            continue;
        }
        let ident = chunk + CHUNK_ENTRY_STRIDE * (i & SLOT_INDEX_MASK) as u64;
        let inst = rd_u64(process, ident); // entity ptr @ identity+0
        if inst == 0 {
            continue;
        }
        let name_ptr = rd_u64(process, ident + IDENTITY_DESIGNER_NAME);
        let name = rd_cstr(process, name_ptr);
        if !name.starts_with("weapon_") || by_name.contains_key(&name) {
            continue;
        }
        if let Some(w) = read_weapon(process, &offs, &name, inst) {
            by_name.insert(name, w);
        }
    }

    Ok(by_name.into_values().collect())
}

fn read_weapon<P: MemoryView>(process: &mut P, o: &Offsets, name: &str, entity: u64) -> Option<Weapon> {
    // Find the vdata pointer offset by validating the dereferenced object:
    // the schema-derived slot first, then the last-known candidates.
    let candidates = o.vdata_ptr.into_iter().chain(VDATA_PTR_CANDIDATES.iter().copied());
    for off in candidates {
        let vd = rd_u64(process, entity + off);
        if vd < 0x10000 {
            continue;
        }
        let damage = rd_i32(process, vd + o.damage);
        let penetration = rd_f32(process, vd + o.penetration);
        let price = rd_i32(process, vd + o.price);
        // Sanity gate: real weapon vdata has damage in [1,1000], penetration in
        // [0,10], price in [0,20000]. Rejects wrong offsets / uninitialised reads.
        if (1..=1000).contains(&damage)
            && (0.0..=10.0).contains(&penetration)
            && (0..=20000).contains(&price)
        {
            return Some(Weapon {
                name: name.to_string(),
                damage,
                headshot_multiplier: rd_f32(process, vd + o.headshot_mult),
                armor_ratio: rd_f32(process, vd + o.armor_ratio),
                penetration,
                range: rd_f32(process, vd + o.range),
                range_modifier: rd_f32(process, vd + o.range_modifier),
                cycle_time: rd_f32(process, vd + o.cycle_time),
                price,
                num_bullets: rd_i32(process, vd + o.num_bullets),
                max_speed: rd_f32(process, vd + o.max_speed),
                spread: rd_f32(process, vd + o.spread),
                inaccuracy_stand: rd_f32(process, vd + o.inaccuracy_stand),
                inaccuracy_move: rd_f32(process, vd + o.inaccuracy_move),
                recoil_magnitude: rd_f32(process, vd + o.recoil_magnitude),
                address: vd,
                vdata_ptr_offset: off,
            });
        }
    }
    None
}

// --- read helpers (best-effort) --------------------------------------------

fn rd_u64<P: MemoryView>(process: &mut P, va: u64) -> u64 {
    process.read::<u64>(Address::from(va)).data_part().unwrap_or(0)
}
fn rd_i32<P: MemoryView>(process: &mut P, va: u64) -> i32 {
    process.read::<i32>(Address::from(va)).data_part().unwrap_or(0)
}
fn rd_f32<P: MemoryView>(process: &mut P, va: u64) -> f32 {
    process.read::<f32>(Address::from(va)).data_part().unwrap_or(0.0)
}
fn rd_cstr<P: MemoryView>(process: &mut P, ptr: u64) -> String {
    if ptr == 0 {
        return String::new();
    }
    process.read_utf8_lossy(Address::from(ptr), 64).data_part().unwrap_or_default()
}
