//! Emitter for the weapon gameplay-values table (weapons.json).

use serde_json::json;

use crate::analysis::weapons::Weapon;

/// f32 as its shortest decimal form (0.15, not 0.15000000596046448): widening to
/// f64 before serialising exposes the float's binary tail.
fn f(v: f32) -> serde_json::Value {
    format!("{}", v).parse::<f64>().map(|d| json!(d)).unwrap_or_else(|_| json!(v))
}

pub fn render_json(weapons: &[Weapon], build: Option<u32>) -> String {
    let rows: Vec<_> = weapons
        .iter()
        .map(|w| {
            json!({
                "name": w.name,
                "damage": w.damage,
                "headshot_multiplier": f(w.headshot_multiplier),
                "armor_ratio": f(w.armor_ratio),
                "penetration": f(w.penetration),
                "range": f(w.range),
                "range_modifier": f(w.range_modifier),
                "cycle_time": f(w.cycle_time),
                "price": w.price,
                "num_bullets": w.num_bullets,
                "max_speed": f(w.max_speed),
                "spread": f(w.spread),
                "inaccuracy_stand": f(w.inaccuracy_stand),
                "inaccuracy_move": f(w.inaccuracy_move),
                "recoil_magnitude": f(w.recoil_magnitude),
                "address": format!("0x{:X}", w.address),
            })
        })
        .collect();

    serde_json::to_string_pretty(&json!({
        "build_number": build,
        "weapon_count": weapons.len(),
        "note": "Values read live from CCSWeaponBaseVData of weapons present in the dumped session.",
        "weapons": rows,
    }))
    .unwrap_or_else(|_| "{}".into())
}
