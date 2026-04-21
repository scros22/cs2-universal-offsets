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

pub mod database;

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
    pub found: bool,
    pub match_rva: Option<u64>,
    pub match_va: Option<u64>,
    pub rva: Option<u64>,
    pub va: Option<u64>,
    pub error: Option<String>,
}

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
    let t0 = Instant::now();

    // Pre-load each unique module image once so every signature reuses it.
    let mut cache: BTreeMap<String, ModuleCache> = BTreeMap::new();
    for sig in sigs {
        let key = sig.module.to_ascii_lowercase();
        if !cache.contains_key(&key) {
            match ModuleCache::load(process, sig.module) {
                Ok(mc) => {
                    cache.insert(key, mc);
                }
                Err(e) => {
                    // A single failure shouldn't kill the whole pass — we just
                    // mark any sigs for this module as failed below.
                    log::warn!("module load failed for {}: {}", sig.module, e);
                }
            }
        }
    }

    let mut report = SignatureReport {
        total: sigs.len(),
        modules: cache.keys().cloned().collect(),
        ..Default::default()
    };

    let total = sigs.len();
    for (idx, sig) in sigs.iter().enumerate() {
        ui::progress(idx + 1, total, sig.name);

        let hit = match cache.get(&sig.module.to_ascii_lowercase()) {
            Some(mc) => scan_one(mc, sig),
            None => SignatureHit::fail(sig, "module not loaded"),
        };

        if hit.found {
            let detail = format!(
                "[{}, {}]",
                hit.resolve,
                hit.module
            );
            ui::found(&hit.name, hit.va.unwrap_or(0), &detail);
            report.found += 1;
        } else {
            ui::not_found(&hit.name, hit.error.as_deref().unwrap_or("no hit"));
        }
        report.hits.push(hit);
    }
    ui::progress_clear();

    report.elapsed_ms = t0.elapsed().as_millis();
    Ok(report)
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

    // .text first
    let mut off: Option<(u32, &'static str)> = None;
    if let Some(o) = find_pattern(mc.text(), &bytes, &mask) {
        off = Some((mc.text_rva + o as u32, ".text"));
    } else if let Some(rd) = mc.rdata() {
        // Fallback for globals that live in .rdata
        if let Some(o) = find_pattern(rd, &bytes, &mask) {
            off = Some((mc.rdata_rva + o as u32, ".rdata"));
        }
    }

    let (match_rva, _section) = match off {
        Some(p) => p,
        None => return SignatureHit::fail(sig, "pattern not found"),
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
        found: true,
        match_rva: Some(match_rva as u64),
        match_va: Some(match_va),
        rva: Some(res_rva),
        va: Some(res_va),
        error: None,
    }
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
                found: true,
                match_rva: Some(hit as u64),
                match_va: Some(match_va),
                rva: Some(fn_rva as u64),
                va: Some(fn_va),
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
            found: false,
            match_rva: None,
            match_va: None,
            rva: None,
            va: None,
            error: Some(err.to_string()),
        }
    }
}
