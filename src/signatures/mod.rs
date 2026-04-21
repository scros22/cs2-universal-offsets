//! PE/section-aware IDA-style signature scanner for CS2 modules.
//!
//! This module is the Rust port + evolution of the C++ `EnhancedScanner`
//! from the standalone signature-dumper.  It supports:
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
use std::time::Instant;

use anyhow::{Context, Result, anyhow};
use memflow::prelude::v1::*;
use pelite::pe64::{Pe, PeView};

use crate::ui;

pub mod cache;
pub mod database;
pub mod diff;
pub mod writers;

pub use cache::SignatureCache;

// ---------------------------------------------------------------------------
// Public types
// ---------------------------------------------------------------------------

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ResolveKind {
    None,
    Rel32 { rel_off: usize },
    RipRel { rel_off: usize },
    StringRef,
}

#[derive(Clone, Debug)]
pub struct Signature {
    pub name: &'static str,
    pub module: &'static str,
    /// IDA-style bytes, or — for `StringRef` — the literal string to search.
    pub needle: &'static str,
    pub resolve: ResolveKind,
    pub extra_off: i64,
}

#[derive(Clone, Debug, serde::Serialize)]
pub struct SignatureHit {
    pub name: String,
    pub module: String,
    pub resolve: &'static str,
    pub pattern: String,
    /// 24 bytes of the resolved function's prologue, formatted as an
    /// IDA-style space-separated hex pattern (no wildcards).  Useful as a
    /// drop-in signature on builds where the database pattern is missing
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
    pub found: bool,
    pub match_rva: Option<u64>,
    pub match_va: Option<u64>,
    pub rva: Option<u64>,
    pub va: Option<u64>,
    /// Number of distinct matches the pattern produced.
    /// `1` is ideal; `>1` means the pattern is ambiguous and should be
    /// tightened.
    #[serde(default = "default_match_count")]
    pub matches: u32,
    /// `true` when the result was satisfied directly from a previously
    /// validated cache entry (no full module scan was performed).
    #[serde(default)]
    pub from_cache: bool,
    pub error: Option<String>,
}

fn default_match_count() -> u32 { 1 }

#[derive(Default, Debug, serde::Serialize)]
pub struct SignatureReport {
    pub total: usize,
    pub found: usize,
    pub modules: Vec<String>,
    pub hits: Vec<SignatureHit>,
    pub elapsed_ms: u128,
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

pub fn scan_all<P>(process: &mut P, sigs: &[Signature]) -> Result<SignatureReport>
where
    P: Process + MemoryView,
{
    scan_all_with_cache(process, sigs, &SignatureCache::default())
}

/// Like [`scan_all`] but consults `cache` first.  When a cached
/// `match_rva` still has the pattern bytes the scanner expects, the entry
/// is satisfied without performing a full module sweep — useful when
/// re-running on the same CS2 build.
pub fn scan_all_with_cache<P>(
    process: &mut P,
    sigs: &[Signature],
    cache: &SignatureCache,
) -> Result<SignatureReport>
where
    P: Process + MemoryView,
{
    let t0 = Instant::now();

    // Pre-load each unique module image once so every signature reuses it.
    let mut module_cache: BTreeMap<String, ModuleCache> = BTreeMap::new();
    for sig in sigs {
        let key = sig.module.to_ascii_lowercase();
        if !module_cache.contains_key(&key) {
            match ModuleCache::load(process, sig.module) {
                Ok(mc) => {
                    module_cache.insert(key, mc);
                }
                Err(e) => {
                    log::warn!("module load failed for {}: {}", sig.module, e);
                }
            }
        }
    }

    let mut report = SignatureReport {
        total: sigs.len(),
        modules: module_cache.keys().cloned().collect(),
        ..Default::default()
    };

    let total = sigs.len();
    let mut cache_hits = 0u32;
    let mut ambiguous = 0u32;
    for (idx, sig) in sigs.iter().enumerate() {
        ui::progress(idx + 1, total, sig.name);

        let hit = match module_cache.get(&sig.module.to_ascii_lowercase()) {
            Some(mc) => {
                // Try the cache first.  Only accept the cache entry if the
                // pattern still matches at the recorded RVA — otherwise the
                // module has shifted and we must do a full scan.
                if let Some(entry) = cache.get(&mc.name, sig.name, sig.needle)
                    && let Some(mut hit) = try_satisfy_from_cache(mc, sig, entry)
                {
                    cache_hits += 1;
                    hit.from_cache = true;
                    hit
                } else {
                    scan_one(mc, sig)
                }
            }
            None => SignatureHit::fail(sig, "module not loaded"),
        };

        if hit.found {
            if hit.matches > 1 {
                ambiguous += 1;
            }
            let detail = format!("[{}, {}]", hit.resolve, hit.module);
            ui::found(&hit.name, hit.va.unwrap_or(0), &detail);
            report.found += 1;
        } else {
            ui::not_found(&hit.name, hit.error.as_deref().unwrap_or("no hit"));
        }
        report.hits.push(hit);
    }
    ui::progress_clear();

    if cache_hits > 0 {
        log::info!("signature cache satisfied {}/{} entries", cache_hits, total);
    }
    if ambiguous > 0 {
        log::warn!(
            "{} signature(s) matched more than once in their .text section — consider tightening",
            ambiguous
        );
    }

    report.elapsed_ms = t0.elapsed().as_millis();
    Ok(report)
}

/// Verify a cached `match_rva` is still valid by re-checking the pattern
/// bytes at that exact location.  On success we replay the same `resolve`
/// step the scanner would have performed, so the returned `va`/`rva` are
/// fully up to date with the current module base.
fn try_satisfy_from_cache(
    mc: &ModuleCache,
    sig: &Signature,
    entry: cache::CacheEntry,
) -> Option<SignatureHit> {
    if matches!(sig.resolve, ResolveKind::StringRef) {
        // String-ref entries don't have a stable byte-pattern at
        // `match_rva`, so they always re-scan.
        return None;
    }
    let (bytes, mask) = parse_ida(sig.needle).ok()?;
    let lo = entry.match_rva as usize;
    let hi = lo.checked_add(bytes.len())?;
    let img = mc.image.get(lo..hi)?;
    for (i, &b) in img.iter().enumerate() {
        if mask[i] && b != bytes[i] {
            return None;
        }
    }

    let match_va = mc.base + entry.match_rva as u64;
    let (res_rva, res_va, err) = resolve(mc, sig, entry.match_rva, match_va);
    if err.is_some() {
        return None;
    }

    Some(SignatureHit {
        name: sig.name.to_string(),
        module: mc.name.clone(),
        resolve: kind_name(sig.resolve),
        pattern: sig.needle.to_string(),
        bytes: capture_prologue(mc, res_rva),
        pattern_synth: synthesize_pattern(mc, res_rva),
        found: true,
        match_rva: Some(entry.match_rva as u64),
        match_va: Some(match_va),
        rva: Some(res_rva),
        va: Some(res_va),
        matches: 1,
        from_cache: true,
        error: None,
    })
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

fn scan_one(mc: &ModuleCache, sig: &Signature) -> SignatureHit {
    match sig.resolve {
        ResolveKind::StringRef => scan_string_ref(mc, sig),
        _ => scan_pattern(mc, sig),
    }
}

fn scan_pattern(mc: &ModuleCache, sig: &Signature) -> SignatureHit {
    let (bytes, mask) = match parse_ida(sig.needle) {
        Ok(v) => v,
        Err(e) => return SignatureHit::fail(sig, &format!("bad pattern: {}", e)),
    };

    // .text first; .rdata fallback for globals.
    // We always count *all* matches in the primary section so callers can
    // detect ambiguous patterns and tighten them.
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

    let Some(match_rva) = off else {
        return SignatureHit::fail(sig, "pattern not found");
    };

    let match_va = mc.base + match_rva as u64;
    let (res_rva, res_va, err) = resolve(mc, sig, match_rva, match_va);

    if let Some(e) = err {
        return SignatureHit::fail(sig, &e);
    }

    SignatureHit {
        name: sig.name.to_string(),
        module: mc.name.clone(),
        resolve: kind_name(sig.resolve),
        pattern: sig.needle.to_string(),
        bytes: capture_prologue(mc, res_rva),
        pattern_synth: synthesize_pattern(mc, res_rva),
        found: true,
        match_rva: Some(match_rva as u64),
        match_va: Some(match_va),
        rva: Some(res_rva),
        va: Some(res_va),
        matches,
        from_cache: false,
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
    sig: &Signature,
    match_rva: u32,
    match_va: u64,
) -> (u64, u64, Option<String>) {
    match sig.resolve {
        ResolveKind::None => {
            let va = (match_va as i64 + sig.extra_off) as u64;
            (match_rva as u64, va, None)
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
        ResolveKind::StringRef => unreachable!(),
    }
}

// ---------------------------------------------------------------------------
// String-ref resolution
// ---------------------------------------------------------------------------

fn scan_string_ref(mc: &ModuleCache, sig: &Signature) -> SignatureHit {
    let Some(rdata) = mc.rdata() else {
        return SignatureHit::fail(sig, ".rdata not present");
    };

    // C string with trailing NUL, lone occurrences only
    let needle_bytes: Vec<u8> = {
        let mut v = sig.needle.as_bytes().to_vec();
        v.push(0);
        v
    };

    let mut str_rvas: Vec<u32> = Vec::new();
    let mask: Vec<bool> = vec![true; needle_bytes.len()];
    for off in find_all_pattern(rdata, &needle_bytes, &mask) {
        // ensure it's a real string start (preceded by NUL or at section start)
        let preceded_ok = off == 0 || rdata[off - 1] == 0;
        if preceded_ok {
            str_rvas.push(mc.rdata_rva + off as u32);
        }
    }
    if str_rvas.is_empty() {
        return SignatureHit::fail(sig, "string not in .rdata");
    }

    let text = mc.text();
    for str_rva in &str_rvas {
        let str_rva = *str_rva;

        // Search for a `48 8D ?5 disp32` (LEA rcx/rdx/r8/r9/...) that
        // points to this string, then walk back to function start.
        if let Some(hit) = find_lea_ref(text, mc.text_rva, str_rva)
            && let Some(fn_rva) = find_function_start(text, mc.text_rva, hit)
        {
            let match_va = mc.base + hit as u64;
            let fn_va = mc.base + fn_rva as u64 + sig.extra_off as u64;
            return SignatureHit {
                name: sig.name.to_string(),
                module: mc.name.clone(),
                resolve: kind_name(sig.resolve),
                pattern: format!("\"{}\"", sig.needle),
                bytes: capture_prologue(mc, fn_rva as u64),
                pattern_synth: synthesize_pattern(mc, fn_rva as u64),
                found: true,
                match_rva: Some(hit as u64),
                match_va: Some(match_va),
                rva: Some(fn_rva as u64),
                va: Some(fn_va),
                matches: 1,
                from_cache: false,
                error: None,
            };
        }
    }

    SignatureHit::fail(sig, "no .text ref near a function start")
}

/// Scan `.text` for `48 8D _5 disp32` (LEA rax/rcx/rdx/rbx/rsp/rbp/rsi/rdi
/// RIP-rel) or `4C 8D _5 disp32` (LEA r8..r15) whose disp32 targets `str_rva`.
/// Returns the RVA of the instruction start on success.
fn find_lea_ref(text: &[u8], text_rva: u32, str_rva: u32) -> Option<u32> {
    if text.len() < 7 {
        return None;
    }
    let mut i = 0usize;
    let end = text.len() - 7;
    while i <= end {
        let rex = text[i];
        // 0x48..0x4F with bit 3 (W) set = REX.W prefix used by LEA r64
        if (rex & 0xF0) == 0x40 && (rex & 0x08) != 0 && text[i + 1] == 0x8D {
            let modrm = text[i + 2];
            // mod=00, rm=101 => RIP-relative
            if (modrm & 0xC7) == 0x05 {
                let disp = i32::from_le_bytes(text[i + 3..i + 7].try_into().unwrap()) as i64;
                let instr_rva = text_rva as i64 + i as i64;
                let target = instr_rva + 7 + disp;
                if target as u32 == str_rva {
                    return Some((text_rva as u64 + i as u64) as u32);
                }
            }
        }
        i += 1;
    }
    None
}

/// Walk backwards from `instr_rva` (RVA inside `.text`) looking for the end
/// of the previous function — a run of `0xCC` (int3) padding.  The byte
/// immediately after is the prologue of the enclosing function.
fn find_function_start(text: &[u8], text_rva: u32, instr_rva: u32) -> Option<u32> {
    let mut i = (instr_rva - text_rva) as isize;
    let min = i.saturating_sub(0x4000);
    let mut run = 0usize;
    while i >= min && i > 0 {
        let b = text[i as usize - 1];
        if b == 0xCC {
            run += 1;
            if run >= 1 {
                // Peek forward past padding for a recognisable prologue start.
                let start = i as usize;
                if start < text.len() && is_prologue(&text[start..]) {
                    return Some(text_rva + start as u32);
                }
            }
        } else {
            run = 0;
        }
        i -= 1;
    }
    None
}

fn is_prologue(b: &[u8]) -> bool {
    if b.is_empty() {
        return false;
    }
    // Common x64 CS2 prologues
    match b[0] {
        0x40 | 0x41 | 0x48 | 0x4C | 0x55 | 0x53 | 0x56 | 0x57 => true,
        _ => false,
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
        ResolveKind::StringRef => "stringref",
    }
}

impl SignatureHit {
    fn fail(sig: &Signature, err: &str) -> Self {
        Self {
            name: sig.name.to_string(),
            module: sig.module.to_string(),
            resolve: kind_name(sig.resolve),
            pattern: sig.needle.to_string(),
            bytes: None,
            pattern_synth: None,
            found: false,
            match_rva: None,
            match_va: None,
            rva: None,
            va: None,
            matches: 0,
            from_cache: false,
            error: Some(err.to_string()),
        }
    }
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
