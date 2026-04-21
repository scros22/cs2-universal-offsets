//! Sidecar cache for previously-resolved signature RVAs.
//!
//! Pattern scanning is fast (a full pass is ~1s on a stock CS2), but
//! re-running the dumper on a *known-good* image where nothing has moved
//! is even faster if we can simply re-verify the bytes at the last known
//! offset.  This module loads the previous run's `signatures.json` (or any
//! file with the same shape) and lets the scanner check it before doing a
//! full module sweep.
//!
//! The cache hit is **only** trusted when the original IDA pattern still
//! matches at the cached `match_rva`.  If a single byte has moved we fall
//! back to a full scan automatically.
//!
//! No cache is created if `--cache <path>` isn't supplied; this keeps
//! reproducible "from scratch" runs trivial.
//!
//! File format is the public `signatures.json` we already emit, so the
//! cache is just the previous session's output — no extra format to learn.

use std::collections::HashMap;
use std::fs;
use std::path::Path;

use anyhow::Result;
use serde::Deserialize;

#[derive(Debug, Clone, Deserialize)]
struct CachedSignature {
    name: String,
    module: String,
    pattern: String,
    #[serde(default)]
    match_rva: Option<u64>,
    #[serde(default)]
    rva: Option<u64>,
}

#[derive(Debug, Default, Deserialize)]
struct CacheFile {
    #[serde(default)]
    signatures: Vec<CachedSignature>,
}

#[derive(Debug, Default)]
pub struct SignatureCache {
    /// keyed by `"<module>|<name>|<pattern>"`
    by_key: HashMap<String, CacheEntry>,
}

#[derive(Debug, Clone, Copy)]
pub struct CacheEntry {
    pub match_rva: u32,
    pub rva: u64,
}

impl SignatureCache {
    /// Load a `signatures.json` produced by a previous run.  Missing or
    /// unreadable files yield an empty cache; the caller can treat this as
    /// a no-op.
    pub fn load(path: &Path) -> Result<Self> {
        if !path.exists() {
            return Ok(Self::default());
        }
        let raw = fs::read_to_string(path)?;
        let parsed: CacheFile = serde_json::from_str(&raw).unwrap_or_default();

        let mut by_key = HashMap::with_capacity(parsed.signatures.len());
        for s in parsed.signatures {
            let (Some(match_rva), Some(rva)) = (s.match_rva, s.rva) else {
                continue;
            };
            let key = key_for(&s.module, &s.name, &s.pattern);
            by_key.insert(
                key,
                CacheEntry {
                    match_rva: match_rva as u32,
                    rva,
                },
            );
        }
        Ok(Self { by_key })
    }

    /// Look up a previously-resolved entry for the given signature.
    pub fn get(&self, module: &str, name: &str, pattern: &str) -> Option<CacheEntry> {
        let key = key_for(module, name, pattern);
        self.by_key.get(&key).copied()
    }

    pub fn len(&self) -> usize {
        self.by_key.len()
    }

    pub fn is_empty(&self) -> bool {
        self.by_key.is_empty()
    }
}

fn key_for(module: &str, name: &str, pattern: &str) -> String {
    let mut s = String::with_capacity(module.len() + name.len() + pattern.len() + 2);
    s.push_str(&module.to_ascii_lowercase());
    s.push('|');
    s.push_str(name);
    s.push('|');
    s.push_str(pattern);
    s
}
