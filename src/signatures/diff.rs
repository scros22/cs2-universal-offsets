//! Diff a current [`SignatureReport`] against a previously-emitted
//! `signatures.json` so consumers can see at a glance what moved between
//! two CS2 patches.
//!
//! The diff is intentionally lightweight: added / removed entries plus
//! every signature whose resolved `va` changed.  Pattern changes are
//! logged separately because they indicate the *signature itself* was
//! tightened, not necessarily a code shift.

use std::collections::BTreeMap;
use std::fs;
use std::path::Path;

use anyhow::Result;
use serde::{Deserialize, Serialize};

use super::SignatureReport;

#[derive(Debug, Default, Deserialize)]
struct PreviousSignatures {
    #[serde(default)]
    signatures: Vec<PreviousEntry>,
}

#[derive(Debug, Clone, Deserialize)]
struct PreviousEntry {
    name: String,
    module: String,
    #[serde(default)]
    pattern: Option<String>,
    #[serde(default)]
    va: Option<u64>,
    #[serde(default)]
    rva: Option<u64>,
}

#[derive(Debug, Serialize)]
pub struct DiffReport {
    pub previous: String,
    pub added: Vec<DiffEntry>,
    pub removed: Vec<DiffEntry>,
    pub shifted: Vec<DiffShift>,
    pub pattern_changed: Vec<DiffPatternChange>,
}

#[derive(Debug, Serialize)]
pub struct DiffEntry {
    pub name: String,
    pub module: String,
    pub va: Option<u64>,
}

#[derive(Debug, Serialize)]
pub struct DiffShift {
    pub name: String,
    pub module: String,
    pub previous_va: u64,
    pub current_va: u64,
    pub delta: i64,
}

#[derive(Debug, Serialize)]
pub struct DiffPatternChange {
    pub name: String,
    pub module: String,
    pub previous_pattern: String,
    pub current_pattern: String,
}

/// Compute a diff against the JSON file at `previous_path`.  A missing
/// file is *not* an error — it just yields an empty diff with the
/// `previous` field tagged appropriately.
pub fn diff_against(previous_path: &Path, current: &SignatureReport) -> Result<DiffReport> {
    if !previous_path.exists() {
        return Ok(DiffReport {
            previous: format!("{} (not found)", previous_path.display()),
            added: Vec::new(),
            removed: Vec::new(),
            shifted: Vec::new(),
            pattern_changed: Vec::new(),
        });
    }

    let raw = fs::read_to_string(previous_path)?;
    let parsed: PreviousSignatures = serde_json::from_str(&raw).unwrap_or_default();

    let mut prev_map: BTreeMap<(String, String), PreviousEntry> = BTreeMap::new();
    for e in parsed.signatures {
        prev_map.insert((e.module.to_ascii_lowercase(), e.name.clone()), e);
    }

    let mut current_map: BTreeMap<(String, String), &super::SignatureHit> = BTreeMap::new();
    for h in &current.hits {
        if h.found {
            current_map.insert((h.module.to_ascii_lowercase(), h.name.clone()), h);
        }
    }

    let mut added = Vec::new();
    let mut shifted = Vec::new();
    let mut pattern_changed = Vec::new();

    for ((module, name), hit) in &current_map {
        match prev_map.get(&(module.clone(), name.clone())) {
            None => added.push(DiffEntry {
                name: name.clone(),
                module: hit.module.clone(),
                va: hit.va,
            }),
            Some(prev) => {
                if let (Some(p_va), Some(c_va)) = (prev.va, hit.va)
                    && p_va != c_va
                {
                    shifted.push(DiffShift {
                        name: name.clone(),
                        module: hit.module.clone(),
                        previous_va: p_va,
                        current_va: c_va,
                        delta: c_va as i64 - p_va as i64,
                    });
                }
                if let Some(p_pat) = &prev.pattern
                    && p_pat != &hit.pattern
                {
                    pattern_changed.push(DiffPatternChange {
                        name: name.clone(),
                        module: hit.module.clone(),
                        previous_pattern: p_pat.clone(),
                        current_pattern: hit.pattern.clone(),
                    });
                }
            }
        }
    }

    let mut removed = Vec::new();
    for ((module, name), prev) in &prev_map {
        if !current_map.contains_key(&(module.clone(), name.clone())) {
            removed.push(DiffEntry {
                name: name.clone(),
                module: prev.module.clone(),
                va: prev.va,
            });
        }
    }

    added.sort_by(|a, b| (a.module.as_str(), a.name.as_str()).cmp(&(b.module.as_str(), b.name.as_str())));
    removed.sort_by(|a, b| (a.module.as_str(), a.name.as_str()).cmp(&(b.module.as_str(), b.name.as_str())));
    shifted.sort_by(|a, b| (a.module.as_str(), a.name.as_str()).cmp(&(b.module.as_str(), b.name.as_str())));
    pattern_changed.sort_by(|a, b| (a.module.as_str(), a.name.as_str()).cmp(&(b.module.as_str(), b.name.as_str())));

    Ok(DiffReport {
        previous: previous_path.display().to_string(),
        added,
        removed,
        shifted,
        pattern_changed,
    })
}
