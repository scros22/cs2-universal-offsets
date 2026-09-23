//! Field offsets resolved from THIS run's live schema dump.
//!
//! The runtime walkers (entities, weapons) used to carry offsets reversed on
//! one build, which silently went stale on the next CS2 update. They now ask
//! the schema they were dumped alongside, climbing parent classes the way the
//! game's own lookup does not, and only fall back to their last-known value
//! when the schema pass was skipped or failed.

use log::warn;

use super::schemas::SchemaMap;

/// Offset of `class::field`, searching `class` then each parent in turn.
pub fn field(map: Option<&SchemaMap>, class: &str, field: &str) -> Option<u64> {
    let map = map?;
    let mut current = class.to_string();
    for _ in 0..24 {
        let cls = map
            .values()
            .flat_map(|(classes, _)| classes.iter())
            .find(|c| c.name == current)?;
        if let Some(f) = cls.fields.iter().find(|f| f.name == field) {
            return u64::try_from(f.offset).ok();
        }
        current = cls.parent_name.clone()?;
    }
    None
}

/// `field()` with a last-known fallback; logs when the fallback is used so a
/// stale constant shows up in cs2-sdk.log instead of as silent garbage.
pub fn field_or(map: Option<&SchemaMap>, class: &str, name: &str, fallback: u64) -> u64 {
    match field(map, class, name) {
        Some(off) => off,
        None => {
            if map.is_some() {
                warn!("schema lookup missed {}::{} - using last-known {:#X}", class, name, fallback);
            }
            fallback
        }
    }
}
