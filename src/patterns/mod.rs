//! PE/section-aware IDA-style Pattern scanner for CS2 modules.
//!
//! This module is the Rust port + evolution of the C++ `EnhancedScanner`
//! from the standalone Pattern-dumper.  It supports:
//!
//!   * IDA-style patterns (`"48 8B ? ? ? ? E8"`) scoped to a module's
//!     `.text` section (with fallback to `.rdata`/`.data` for globals).
//!   * Automatic relative address resolution:
//!       - `Rel32`     : follow E8/E9 disp32 to call/jump target
//!       - `RipRel`    : follow 48 8B/8D/89 05/0D disp32 to data/global
//!       - `StringRef` : locate a unique string in `.rdata`, find the
//!                       `.text` LEA that references it, walk back to the
//!                       function prologue — the Ghidra "find by string"
//!                       workflow, robust across CS2 patches.

use std::collections::BTreeMap;


use anyhow::{Context, Result, anyhow};
use memflow::prelude::v1::*;
use pelite::pe64::{Pe, PeView};

use crate::ui;

pub mod database;
pub mod writers;
pub mod offsets_writer;

// ---------------------------------------------------------------------------
// Public types
// ---------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ResolveKind {
    None,
    Rel32 { rel_off: usize },
    RipRel { rel_off: usize },
}

#[derive(Clone, Debug)]
pub struct Pattern {
    pub name: &'static str,
    pub module: &'static str,
    /// IDA-style bytes, or — for `StringRef` — the literal string to search.
    pub needle: &'static str,
    pub resolve: ResolveKind,
    pub extra_off: i64,
    /// IDA / Hex-Rays C-style function prototype, e.g.
    /// `__int64 __fastcall(__int64 a1, float *a2)`.  When present this is
    /// emitted into all generated artefacts (hpp typedef body, rs doc
    /// comment, md column) so consumers can hook with the real argument
    /// list instead of the generic `void __fastcall(void*, ...)` shape.
    /// Empty string means "not yet recovered".
    pub prototype: &'static str,
}

#[derive(Clone, Debug, serde::Serialize)]
pub struct PatternHit {
    pub name: String,
    pub module: String,
    pub resolve: &'static str,
    pub pattern: String,
    /// IDA / Hex-Rays C-style function prototype recovered for this
    /// Pattern, e.g. `__int64 __fastcall(__int64 a1, float *a2)`.
    /// Copied verbatim from `Pattern::prototype` so the JSON / hpp / rs
    /// / md emitters can render real argument lists for hookers.  `None`
    /// when no prototype has been recorded yet.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub prototype: Option<String>,
    /// 24 bytes of the resolved function's prologue, formatted as an
    /// IDA-style space-separated hex pattern (no wildcards).  Useful as a
    /// drop-in Pattern on builds where the database pattern is missing
    /// (e.g. `StringRef` entries) or has gone stale.  `None` for misses or
    /// when the resolved RVA falls outside the module's `.text`.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub bytes: Option<String>,
    /// Auto-synthesised IDA pattern at the resolved RVA, with `?`
    /// wildcards on relocatable bytes (CALL/JMP rel32 displacements,
    /// RIP-relative LEA/MOV displacements).  Designed to be the
    /// shortest unique-in-`.text` pattern for the resolved function;
    /// safe to paste straight into IDA / x64dbg / ReClass.NET.  `None`
    /// when the resolved RVA is outside `.text`.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub pattern_synth: Option<String>,
    /// Set when the database pattern stopped matching (or matched several
    /// places) and the function was re-found through the previous dump's
    /// prologue bytes: holds the old pattern; `pattern` is the fresh unique
    /// one generated at the new address. See [`heal`].
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub healed_from: Option<String>,
    pub found: bool,
    pub match_rva: Option<u64>,
    pub match_va: Option<u64>,
    pub rva: Option<u64>,
    pub va: Option<u64>,
    /// Number of distinct matches the pattern produced.
    /// `1` is ideal; `>1` means the pattern is ambiguous and should be tightened.
    pub matches: u32,
    pub error: Option<String>,
    /// Other database names that resolve to this same function. The entry is
    /// published once under `name`; these still identify it (site search/API).
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    pub aliases: Vec<String>,
}

#[derive(Default, Debug, serde::Serialize)]
pub struct PatternReport {
    pub total: usize,
    pub found: usize,
    /// Distinct functions/globals among the found hits (aliases folded).
    pub unique_functions: usize,
    pub modules: Vec<String>,
    pub hits: Vec<PatternHit>,
}

// ---------------------------------------------------------------------------
/// Entry point
// ---------------------------------------------------------------------------

/// What the previous dump knew about a published entry: its 24 prologue
/// bytes and where it was. Keyed by published name and by every alias.
pub struct PrevHit {
    pub bytes: Vec<u8>,
    pub rva: u64,
}
pub type PrevMap = BTreeMap<String, PrevHit>;

/// Load `patterns/patterns.json` from a previous dump (strict JSON, dumper
/// >= 2.1.4). Missing or unreadable files just disable self-healing.
pub fn load_previous(path: &std::path::Path) -> Option<PrevMap> {
    let text = std::fs::read_to_string(path).ok()?;
    let text = text.trim_start_matches('\u{feff}');
    let v: serde_json::Value = serde_json::from_str(text).ok()?;
    let mut map = PrevMap::new();
    for p in v.get("patterns")?.as_array()? {
        let (Some(name), Some(bytes), Some(rva)) = (
            p.get("name").and_then(|x| x.as_str()),
            p.get("bytes").and_then(|x| x.as_str()),
            p.get("rva").and_then(|x| x.as_str()),
        ) else { continue };
        let Ok((b, _)) = parse_ida(bytes) else { continue };
        let Ok(rva) = u64::from_str_radix(rva.trim_start_matches("0x"), 16) else { continue };
        if b.len() < 16 {
            continue;
        }
        let mut names = vec![name.to_string()];
        if let Some(al) = p.get("aliases").and_then(|x| x.as_array()) {
            names.extend(al.iter().filter_map(|a| a.as_str().map(String::from)));
        }
        for n in names {
            map.entry(n).or_insert_with(|| PrevHit { bytes: b.clone(), rva });
        }
    }
    Some(map)
}

/// Re-anchor a function entry whose database pattern no longer works.
///
/// If the previous dump's prologue bytes for this entry occur exactly once in
/// the new module's `.text`, right after padding, the function is still there
/// and only moved. A fresh pattern that is unique in `.text` is generated at
/// that address and published in place of the stale one, flagged through
/// `healed_from` so the database can be updated (tools/verify/heal.py).
/// Globals (`RipRel`) cannot be healed this way and stay unresolved.
fn heal(mc: &ModuleCache, sig: &Pattern, prev: &PrevMap) -> Option<PatternHit> {
    if matches!(sig.resolve, ResolveKind::RipRel { .. }) {
        return None;
    }
    let ph = prev.get(&display_name(sig.name)).or_else(|| prev.get(sig.name))?;
    let mask = vec![true; ph.bytes.len()];
    let hits = find_all_pattern(mc.text(), &ph.bytes, &mask);
    if hits.len() != 1 {
        return None;
    }
    let rva = mc.text_rva as u64 + hits[0] as u64;
    let before = mc.image.get(rva as usize - 1).copied()?;
    if !matches!(before, 0xCC | 0xC3 | 0x00) {
        return None;
    }
    // Prefer the wildcarded synthesised pattern; fall back to the concrete
    // prologue when the synthesiser could not make a unique one.
    let unique = |p: &str| parse_ida(p).map(|(b, m)| count_matches_capped(mc.text(), &b, &m, 2) == 1).unwrap_or(false);
    let pattern = match synthesize_pattern(mc, rva) {
        Some(p) if unique(&p) => p,
        _ => {
            let concrete = format_ida(&ph.bytes, &mask);
            if !unique(&concrete) {
                return None;
            }
            concrete
        }
    };
    log::warn!(
        "{}: pattern re-anchored via previous prologue: {} -> 0x{:X} (was 0x{:X}); database needs the new pattern",
        sig.name, sig.module, rva, ph.rva
    );
    Some(PatternHit {
        name: display_name(sig.name),
        module: mc.name.clone(),
        resolve: "raw",
        pattern,
        prototype: opt_proto(sig.name, sig.prototype),
        bytes: capture_prologue(mc, rva),
        pattern_synth: synthesize_pattern(mc, rva),
        healed_from: Some(sig.needle.to_string()),
        found: true,
        aliases: Vec::new(),
        match_rva: Some(rva),
        match_va: Some(mc.base + rva),
        rva: Some(rva),
        va: Some(mc.base + rva),
        matches: 1,
        error: None,
    })
}

pub fn scan_all<P>(process: &mut P, sigs: &[Pattern], prev: Option<&PrevMap>) -> Result<PatternReport>
where
    P: Process + MemoryView,
{
    let mut module_cache: BTreeMap<String, ModuleCache> = BTreeMap::new();
    for sig in sigs {
        let key = sig.module.to_ascii_lowercase();
        if !module_cache.contains_key(&key) {
            match ModuleCache::load(process, sig.module) {
                Ok(mc)  => { module_cache.insert(key, mc); }
                Err(e)  => { log::warn!("module load failed for {}: {}", sig.module, e); }
            }
        }
    }

    let mut report = PatternReport {
        total: sigs.len(),
        modules: module_cache.keys().cloned().collect(),
        ..Default::default()
    };

    let total = sigs.len();
    let mut ambiguous = 0u32;
    for (idx, sig) in sigs.iter().enumerate() {
        ui::progress(idx + 1, total, sig.name);

        let mc = module_cache.get(&sig.module.to_ascii_lowercase());
        let mut hit = match mc {
            Some(mc) => scan_one(mc, sig),
            None     => PatternHit::fail(sig, "module not loaded"),
        };
        // Stale or ambiguous pattern: try to re-find the function through
        // the previous dump's prologue bytes before giving up on it.
        if (!hit.found || hit.matches > 1)
            && let (Some(mc), Some(prev)) = (mc, prev)
            && let Some(h) = heal(mc, sig, prev)
        {
            hit = h;
        }

        if hit.found {
            if hit.matches > 1 { ambiguous += 1; }
            if hit.healed_from.is_some() {
                ui::warn(&format!("{} re-anchored via previous prologue -> 0x{:X} ({}); update the database", hit.name, hit.rva.unwrap_or(0), hit.module));
            } else {
                ui::found(&hit.name, hit.va.unwrap_or(0), &format!("[{}, {}]", hit.resolve, hit.module));
            }
            report.found += 1;
        } else {
            ui::not_found(&hit.name, hit.error.as_deref().unwrap_or("no hit"));
        }
        report.hits.push(hit);
    }
    ui::progress_clear();

    // Published names drop the class prefix (CCSGOInput_CreateMove -> CreateMove).
    // When two entries would publish under the same short name (both
    // CInButtonStatePB_New and CCSGOInputHistoryEntryPB_New -> "New"), keep the
    // full, class-qualified name for every one of them so consumers can tell
    // them apart.
    let mut counts: BTreeMap<String, u32> = BTreeMap::new();
    for h in &report.hits {
        *counts.entry(h.name.clone()).or_default() += 1;
    }
    for (hit, sig) in report.hits.iter_mut().zip(sigs.iter()) {
        if counts.get(&hit.name).copied().unwrap_or(0) > 1 && hit.name != sig.name {
            hit.name = sig.name.to_string();
        }
    }

    fold_aliases(&mut report);

    if ambiguous > 0 {
        log::warn!(
            "{} Pattern(s) matched more than once in their .text section — consider tightening",
            ambiguous
        );
    }

    Ok(report)
}

/// The database carries several community names for some functions
/// (TraceShape / CGameTrace_TraceShape_Client). Two entries that resolve to the
/// same address ARE the same function, so publish it once: the most descriptive
/// name becomes `name`, the rest go under `aliases`. Preference: not a
/// `_v2` / `_raw` / `_E8` / `_caller` / `_Client` variant, then class-qualified
/// (`CClass_Method`), then the longest name, then alphabetical.
fn fold_aliases(report: &mut PatternReport) {
    fn is_variant(n: &str) -> bool {
        ["_v2", "_v3", "_raw", "_E8", "_caller", "_legacy", "_Client", "_inv"]
            .iter()
            .any(|s| n.ends_with(s))
    }
    fn class_qualified(n: &str) -> bool {
        let b = n.as_bytes();
        b.len() > 2
            && (b[0] == b'C' || b[0] == b'I')
            && (b[1].is_ascii_uppercase() || b[1] == b'_')
            && n[1..].find('_').map(|i| n[i + 2..].chars().next().map(|c| c.is_ascii_uppercase()).unwrap_or(false)).unwrap_or(false)
    }
    fn rank(n: &str) -> (u8, u8, isize, String) {
        (is_variant(n) as u8, (!class_qualified(n)) as u8, -(n.len() as isize), n.to_string())
    }
    let mut groups: BTreeMap<(String, u64), Vec<usize>> = BTreeMap::new();
    for (i, h) in report.hits.iter().enumerate() {
        if let (true, Some(rva)) = (h.found, h.rva) {
            groups.entry((h.module.to_ascii_lowercase(), rva)).or_default().push(i);
        }
    }
    let mut drop = std::collections::BTreeSet::new();
    for idxs in groups.values() {
        if idxs.len() < 2 {
            continue;
        }
        let mut order = idxs.clone();
        order.sort_by_key(|&i| rank(&report.hits[i].name));
        let primary = order[0];
        let mut aliases: Vec<String> = order[1..].iter().map(|&i| report.hits[i].name.clone()).collect();
        aliases.sort();
        aliases.dedup();
        // keep a prototype if only an alias entry carried one
        if report.hits[primary].prototype.is_none() {
            if let Some(p) = order[1..].iter().find_map(|&i| report.hits[i].prototype.clone()) {
                report.hits[primary].prototype = Some(p);
            }
        }
        report.hits[primary].aliases = aliases;
        drop.extend(order[1..].iter().copied());
    }
    if !drop.is_empty() {
        let mut i = 0usize;
        report.hits.retain(|_| { let keep = !drop.contains(&i); i += 1; keep });
    }
    report.unique_functions = report.hits.iter().filter(|h| h.found).count();
}

// ---------------------------------------------------------------------------
// Module cache — full image + PeView
// ---------------------------------------------------------------------------

struct ModuleCache {
    name: String,
    base: u64,
    image: Vec<u8>,
    text_rva: u32,
    text_size: u32,
    rdata_rva: u32,
    rdata_size: u32,
}

impl ModuleCache {
    fn load<P: Process + MemoryView>(process: &mut P, module: &str) -> Result<Self> {
        let info = process
            .module_by_name(module)
            .with_context(|| format!("module {} not present in process", module))?;

        let image = process
            .read_raw(info.base, info.size as usize)
            .with_context(|| format!("failed to read image of {}", module))?;

        let view = PeView::from_bytes(&image).context("invalid PE image")?;

        let mut text_rva = 0u32;
        let mut text_size = 0u32;
        let mut rdata_rva = 0u32;
        let mut rdata_size = 0u32;

        for section in view.section_headers() {
            let name = section.name().unwrap_or("");
            match name {
                ".text" => {
                    text_rva = section.VirtualAddress;
                    text_size = section.VirtualSize;
                }
                ".rdata" => {
                    rdata_rva = section.VirtualAddress;
                    rdata_size = section.VirtualSize;
                }
                _ => {}
            }
        }

        if text_size == 0 {
            return Err(anyhow!(".text section missing in {}", module));
        }

        Ok(Self {
            name: module.to_string(),
            base: info.base.to_umem() as u64,
            image,
            text_rva,
            text_size,
            rdata_rva,
            rdata_size,
        })
    }

    #[inline]
    fn text(&self) -> &[u8] {
        let lo = self.text_rva as usize;
        let hi = lo + self.text_size as usize;
        &self.image[lo..hi.min(self.image.len())]
    }

    #[inline]
    fn rdata(&self) -> Option<&[u8]> {
        if self.rdata_size == 0 {
            return None;
        }
        let lo = self.rdata_rva as usize;
        let hi = lo + self.rdata_size as usize;
        self.image.get(lo..hi.min(self.image.len()))
    }
}

// ---------------------------------------------------------------------------
// IDA pattern parser
// ---------------------------------------------------------------------------

fn parse_ida(pattern: &str) -> Result<(Vec<u8>, Vec<bool>)> {
    let mut bytes = Vec::with_capacity(pattern.len() / 3);
    let mut mask = Vec::with_capacity(pattern.len() / 3);

    for tok in pattern.split_ascii_whitespace() {
        if tok == "?" || tok == "??" {
            bytes.push(0);
            mask.push(false);
        } else if tok.len() == 2 {
            let b = u8::from_str_radix(tok, 16)
                .with_context(|| format!("invalid hex byte '{}'", tok))?;
            bytes.push(b);
            mask.push(true);
        } else {
            return Err(anyhow!("invalid pattern token '{}'", tok));
        }
    }

    if bytes.is_empty() {
        return Err(anyhow!("empty pattern"));
    }
    Ok((bytes, mask))
}

fn find_pattern(hay: &[u8], bytes: &[u8], mask: &[bool]) -> Option<usize> {
    let need = bytes.len();
    if hay.len() < need {
        return None;
    }
    let first = bytes[0];
    let first_wild = !mask[0];
    let end = hay.len() - need;
    'outer: for i in 0..=end {
        if !first_wild && hay[i] != first {
            continue;
        }
        for j in 1..need {
            if mask[j] && hay[i + j] != bytes[j] {
                continue 'outer;
            }
        }
        return Some(i);
    }
    None
}

fn find_all_pattern(hay: &[u8], bytes: &[u8], mask: &[bool]) -> Vec<usize> {
    let mut out = Vec::new();
    let need = bytes.len();
    if hay.len() < need {
        return out;
    }
    let first = bytes[0];
    let first_wild = !mask[0];
    let end = hay.len() - need;
    let mut i = 0usize;
    while i <= end {
        if first_wild || hay[i] == first {
            let mut ok = true;
            for j in 1..need {
                if mask[j] && hay[i + j] != bytes[j] {
                    ok = false;
                    break;
                }
            }
            if ok {
                out.push(i);
            }
        }
        i += 1;
    }
    out
}

// ---------------------------------------------------------------------------
// Core scan + resolve
// ---------------------------------------------------------------------------

fn scan_one(mc: &ModuleCache, sig: &Pattern) -> PatternHit {
    scan_pattern(mc, sig)
}

fn scan_pattern(mc: &ModuleCache, sig: &Pattern) -> PatternHit {
    let (bytes, mask) = match parse_ida(sig.needle) {
        Ok(v) => v,
        Err(e) => return PatternHit::fail(sig, &format!("bad pattern: {}", e)),
    };

    // .text first; .rdata fallback for globals; full-image last resort so
    // patterns in non-standard sections (e.g. client.dll's .PAGE) are found
    // the same way the cheat's own memory::FindPattern (SizeOfImage scan) does.
    let text_hits = find_all_pattern(mc.text(), &bytes, &mask);
    let mut matches: u32 = text_hits.len() as u32;
    let mut off: Option<u32> = text_hits.first().map(|o| mc.text_rva + *o as u32);

    if off.is_none()
        && let Some(rd) = mc.rdata()
        && let Some(o) = find_pattern(rd, &bytes, &mask)
    {
        off = Some(mc.rdata_rva + o as u32);
        matches = 1;
    }

    if off.is_none() {
        if let Some(o) = find_pattern(&mc.image, &bytes, &mask) {
            off = Some(o as u32);
            matches = 1;
        }
    }

    let Some(match_rva) = off else {
        return PatternHit::fail(sig, "pattern not found");
    };

    let match_va = mc.base + match_rva as u64;
    let (res_rva, res_va, err) = resolve(mc, sig, match_rva, match_va);

    if let Some(e) = err {
        return PatternHit::fail(sig, &e);
    }

    PatternHit {
        name: display_name(sig.name),
        module: mc.name.clone(),
        resolve: kind_name(sig.resolve),
        pattern: sig.needle.to_string(),
        prototype: opt_proto(sig.name, sig.prototype),
        bytes: capture_prologue(mc, res_rva),
        pattern_synth: synthesize_pattern(mc, res_rva),
        healed_from: None,
        found: true,
        aliases: Vec::new(),
        match_rva: Some(match_rva as u64),
        match_va: Some(match_va),
        rva: Some(res_rva),
        va: Some(res_va),
        matches,
        error: None,
    }
}

/// Read up to 24 bytes from the resolved RVA and format them as a
/// space-separated, fully-concrete IDA pattern.  Returns `None` if the
/// RVA is not inside the module's `.text` window.
fn capture_prologue(mc: &ModuleCache, rva: u64) -> Option<String> {
    let lo = rva as usize;
    let text_lo = mc.text_rva as usize;
    let text_hi = text_lo + mc.text_size as usize;
    if lo < text_lo || lo >= text_hi {
        return None;
    }
    let hi = (lo + 24).min(text_hi).min(mc.image.len());
    let slice = mc.image.get(lo..hi)?;
    if slice.is_empty() {
        return None;
    }
    let mut s = String::with_capacity(slice.len() * 3);
    for (i, b) in slice.iter().enumerate() {
        if i > 0 {
            s.push(' ');
        }
        s.push_str(&format!("{:02X}", b));
    }
    Some(s)
}

fn resolve(
    mc: &ModuleCache,
    sig: &Pattern,
    match_rva: u32,
    match_va: u64,
) -> (u64, u64, Option<String>) {
    match sig.resolve {
        ResolveKind::None => {
            // extra_off moves BOTH: the published rva used to stay on the match
            // start while va moved, so entries like ConvarGet (needle begins in
            // the previous function's tail, extra_off 4) reported the wrong rva.
            let va = (match_va as i64 + sig.extra_off) as u64;
            let rva = (match_rva as i64 + sig.extra_off) as u64;
            (rva, va, None)
        }
        ResolveKind::Rel32 { rel_off } | ResolveKind::RipRel { rel_off } => {
            let idx = match_rva as usize + rel_off;
            if idx + 4 > mc.image.len() {
                return (0, 0, Some("disp32 out of image".into()));
            }
            let disp = i32::from_le_bytes(mc.image[idx..idx + 4].try_into().unwrap()) as i64;
            let target_va = match_va as i64 + rel_off as i64 + 4 + disp + sig.extra_off;
            let target_rva = (target_va - mc.base as i64) as u64;
            (target_rva, target_va as u64, None)
        }
    }
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------

fn kind_name(k: ResolveKind) -> &'static str {
    match k {
        ResolveKind::None => "raw",
        ResolveKind::Rel32 { .. } => "rel32",
        ResolveKind::RipRel { .. } => "riprel",
    }
}

impl PatternHit {
    fn fail(sig: &Pattern, err: &str) -> Self {
        Self {
            name: display_name(sig.name),
            module: sig.module.to_string(),
            resolve: kind_name(sig.resolve),
            pattern: sig.needle.to_string(),
            prototype: opt_proto(sig.name, sig.prototype),
                bytes: None,
            pattern_synth: None,
            healed_from: None,
            found: false,
            aliases: Vec::new(),
            match_rva: None,
            match_va: None,
            rva: None,
            va: None,
            matches: 0,
                error: Some(err.to_string()),
        }
    }
}

pub(crate) fn display_name(raw: &str) -> String {
    if raw.is_empty() {
        return String::new();
    }
    if let Some(idx) = raw.rfind("::") {
        return raw[idx + 2..].to_string();
    }
    if raw.starts_with("m_") || raw.starts_with("dw") || raw.starts_with("g_") || raw.starts_with("C_") || raw.ends_with("_t") {
        return raw.to_string();
    }

    let parts: Vec<&str> = raw.split('_').filter(|p| !p.is_empty()).collect();
    if parts.len() > 1 {
        let head = parts[0];
        // Only strip the leading token when it is genuinely a C++ class prefix
        // (`CCSPlayer`, `CBaseEntity`, `IGameSystem`) — i.e. `C`/`I` followed by
        // an uppercase letter. An ordinary verb head like `SetAbsOrigin` or
        // `PhysicsRunThink` must NOT be stripped, otherwise a descriptive name
        // collapses to a meaningless fragment (`SetAbsOrigin_Pawn` -> `Pawn`).
        let mut hc = head.chars();
        let looks_like_class = matches!(hc.next(), Some('C') | Some('I'))
            && hc.next().map(|c| c.is_ascii_uppercase()).unwrap_or(false);
        if looks_like_class {
            // Keep every segment after the class prefix so multi-word method
            // names survive (`CCSPlayer_RunCommand_Context` -> `RunCommand_Context`).
            let rest = parts[1..].join("_");
            // Only strip when the remainder reads as a real method name (starts
            // uppercase). Otherwise keep the full name so we don't reduce it to a
            // meaningless fragment (`CSGOInput_ptr` -> `ptr`).
            // ... and reads as a real method name: a single bare word
            // (`Get`, `New`, `Init`, `Think`) says nothing without its class,
            // so those keep the prefix (`CCSInventoryManager_Get`).
            let multi_word = rest.chars().skip(1).any(|c| c.is_ascii_uppercase() || c == '_');
            if rest.chars().next().map(|c| c.is_ascii_uppercase()).unwrap_or(false) && multi_word {
                return rest;
            }
        }
    }
    raw.to_string()
}

fn opt_proto(sig_name: &str, p: &'static str) -> Option<String> {
    if p.is_empty() {
        return None;
    }

    let display = display_name(sig_name);
    let mut out = p.to_string();
    if let Some(start) = out.find("sub_") {
        let mut end = start + 4;
        while end < out.len() && out.as_bytes()[end].is_ascii_hexdigit() {
            end += 1;
        }
        out.replace_range(start..end, &display);
    }
    Some(out)
}

// ---------------------------------------------------------------------------
// Auto-tightened pattern synthesiser
// ---------------------------------------------------------------------------

/// Build the shortest unique-in-`.text` IDA pattern at `rva`, with `?`
/// wildcards on bytes that look like rel32 displacements (CALL/JMP near,
/// jcc near, RIP-relative LEA/MOV).  The result is deterministic and
/// safe to paste into IDA, x64dbg, ReClass.NET, etc. without any
/// post-processing.
///
/// Strategy:
///   1. read N bytes from `rva` (try 16, 24, 32, 40, 48 in order)
///   2. mark suspected rel32 displacements as wildcards
///   3. count matches of the masked pattern within the module's `.text`
///   4. accept the first length whose match count is exactly 1
///   5. if none unique, return the longest-attempted pattern as a
///      best-effort fallback (consumers can still tighten by hand)
fn synthesize_pattern(mc: &ModuleCache, rva: u64) -> Option<String> {
    let lo = rva as usize;
    let text_lo = mc.text_rva as usize;
    let text_hi = text_lo + mc.text_size as usize;
    if lo < text_lo || lo >= text_hi {
        return None;
    }
    let cap = text_hi.min(mc.image.len());

    let try_lengths = [16usize, 20, 24, 28, 32, 40, 48];
    let mut best: Option<(Vec<u8>, Vec<bool>)> = None;
    for &len in &try_lengths {
        let hi = (lo + len).min(cap);
        if hi <= lo {
            break;
        }
        let bytes = mc.image[lo..hi].to_vec();
        if bytes.is_empty() {
            break;
        }
        let mask = relocatable_mask(&bytes);
        let count = count_matches_capped(mc.text(), &bytes, &mask, 2);
        // We always match ourselves once; require uniqueness.
        if count == 1 {
            return Some(format_ida(&bytes, &mask));
        }
        best = Some((bytes, mask));
    }
    // Couldn't disambiguate within 48 bytes — return the longest attempt
    // anyway; it's still useful in IDA.
    best.map(|(b, m)| format_ida(&b, &m))
}

/// Mark bytes that are part of a rel32 displacement as wildcards.
/// Conservative: only handles instructions whose layout is well known,
/// leaves everything else as-is.  Over-matching is fine — a wildcard
/// just means "don't care", which only loosens the pattern.
fn relocatable_mask(bytes: &[u8]) -> Vec<bool> {
    let mut mask = vec![true; bytes.len()];
    let mut i = 0;
    while i < bytes.len() {
        let b = bytes[i];

        // E8 cd / E9 cd  — call / jmp near, rel32
        if (b == 0xE8 || b == 0xE9) && i + 5 <= bytes.len() {
            for j in 0..4 {
                mask[i + 1 + j] = false;
            }
            i += 5;
            continue;
        }

        // 0F 8x cd  — jcc near, rel32
        if b == 0x0F && i + 6 <= bytes.len() && (bytes[i + 1] & 0xF0) == 0x80 {
            for j in 0..4 {
                mask[i + 2 + j] = false;
            }
            i += 6;
            continue;
        }

        // REX.W (48..4F) + 8B/89/8D/03/0B/13/1B/23/2B/33/3B/85 + ModR/M
        // with mod=00 rm=101  (RIP-relative addressing)
        if (b & 0xF8) == 0x48 && i + 7 <= bytes.len() {
            let op2 = bytes[i + 1];
            let modrm = bytes[i + 2];
            let rip_rel_op = matches!(
                op2,
                0x03 | 0x0B | 0x13 | 0x1B | 0x23 | 0x2B | 0x33 | 0x3B | 0x85 | 0x89 | 0x8B | 0x8D
            );
            if rip_rel_op && (modrm & 0xC7) == 0x05 {
                for j in 0..4 {
                    mask[i + 3 + j] = false;
                }
                i += 7;
                continue;
            }
        }

        // Plain MOV/LEA RIP-rel without REX prefix (32-bit dest).
        if (b == 0x8B || b == 0x89 || b == 0x8D) && i + 6 <= bytes.len() {
            let modrm = bytes[i + 1];
            if (modrm & 0xC7) == 0x05 {
                for j in 0..4 {
                    mask[i + 2 + j] = false;
                }
                i += 6;
                continue;
            }
        }

        i += 1;
    }
    mask
}

/// Count matches of `(bytes, mask)` in `hay`, but stop early after
/// `cap` matches — we only need to distinguish "1" from ">=2".
fn count_matches_capped(hay: &[u8], bytes: &[u8], mask: &[bool], cap: usize) -> usize {
    let need = bytes.len();
    if hay.len() < need || need == 0 {
        return 0;
    }
    let first = bytes[0];
    let first_wild = !mask[0];
    let end = hay.len() - need;
    let mut count = 0usize;
    let mut i = 0usize;
    while i <= end {
        if first_wild || hay[i] == first {
            let mut ok = true;
            for j in 1..need {
                if mask[j] && hay[i + j] != bytes[j] {
                    ok = false;
                    break;
                }
            }
            if ok {
                count += 1;
                if count >= cap {
                    return count;
                }
            }
        }
        i += 1;
    }
    count
}

fn format_ida(bytes: &[u8], mask: &[bool]) -> String {
    let mut s = String::with_capacity(bytes.len() * 3);
    for (i, b) in bytes.iter().enumerate() {
        if i > 0 {
            s.push(' ');
        }
        if mask[i] {
            s.push_str(&format!("{:02X}", b));
        } else {
            s.push('?');
        }
    }
    s
}
