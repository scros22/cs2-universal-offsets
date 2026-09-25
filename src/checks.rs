//! Self-checks that run after every dump, and the `status.json` they produce.
//!
//! The 2026-09-25 audit found four kinds of silent breakage that a plain
//! "did the pattern match" pass cannot see: an a2x-style global that lands on
//! a vtable pointer in `.rdata`, one that resolves a different ConVar than its
//! name says, a member offset read from a stack slot, and a protobuf layout
//! that disagrees with the hand-verified engine struct. Every one of those is
//! catchable from the process alone, so the dumper now checks for them and
//! refuses to report success when they happen. The results are written to
//! `status.json` so the site, the Discord bot and the auto-dump pipeline can
//! show exactly what is and is not right about the current dump.
use std::fs;
use std::path::Path;

use anyhow::{Context, Result};
use memflow::prelude::v1::*;
use serde_json::{json, Value};

use crate::analysis::OffsetMap;
use crate::analysis::ProtobufMap;
use crate::output::engine_structs::ENGINE_STRUCTS;
use crate::patterns::{display_name, PatternHit, PatternReport};

pub struct Check {
    pub name: &'static str,
    pub status: &'static str, // "pass" | "warn" | "fail"
    pub detail: String,
    pub items: Vec<Value>,
}

impl Check {
    fn new(name: &'static str, status: &'static str, detail: impl Into<String>, items: Vec<Value>) -> Self {
        Self { name, status, detail: detail.into(), items }
    }
}

/// a2x-style global and the signature global that must resolve to the same
/// address. A mismatch means one of the two anchors drifted onto the wrong
/// instruction — exactly what happened to `dwGameRules` and `dwSensitivity`.
const TWINS: &[(&str, &str, &str)] = &[
    ("client.dll", "dwGameRules", "pGameRules"),
    ("client.dll", "dwSensitivity", "pSensitivity"),
    ("client.dll", "dwPrediction", "pPrediction"),
    ("client.dll", "dwGlowManager", "pGlowManager"),
    ("client.dll", "dwLocalPlayerController", "pLocalPlayerController"),
    ("client.dll", "dwCSGOInput", "pCSGOInputInstance"),
    ("engine2.dll", "dwBuildNumber", "pBuildNumber"),
    ("engine2.dll", "dwNetworkGameClient", "pNetworkGameClient"),
    ("engine2.dll", "dwWindowWidth", "pWindowWidth"),
    ("engine2.dll", "dwWindowHeight", "pWindowHeight"),
    ("soundsystem.dll", "dwSoundSystem", "pSoundSystem"),
];

/// Database entries that deliberately resolve to an instruction inside a
/// function (patch / read sites). They are exempt from the prologue check.
const SITES: &[&str] = &[
    "UntrustedFlagSetter",
    "CAM_ThinkReturn",
    "CCSPlayer_ThirdPersonReset",
    "DisablePvsAccessor",
    "IGameSystem_InitAllSystems_pFirst",
    "IGameSystem_LoopDestroyAllSystems_s_GameSystems",
    "IGameSystem_LoopPostInitAllSystems_pEventDispatcher",
];

/// Engine structs whose protobuf twin carries a different name.
fn proto_name_of(engine_struct: &str) -> &str {
    match engine_struct {
        "CCSGOUserCmdPB" => "CSGOUserCmdPB",
        "CCSGOInputHistoryEntryPB" => "CSGOInputHistoryEntryPB",
        other => other,
    }
}

struct Sections {
    base: u64,
    secs: Vec<(String, u32, u32)>, // name, va, size
}

impl Sections {
    fn load<P: Process + MemoryView>(process: &mut P, module: &str) -> Result<Self> {
        let info = process.module_by_name(module).with_context(|| format!("{} not loaded", module))?;
        let base = info.base.to_umem() as u64;
        let hdr = process.read_raw(info.base, 0x1000).context("PE header read")?;
        let rd32 = |o: usize| u32::from_le_bytes([hdr[o], hdr[o + 1], hdr[o + 2], hdr[o + 3]]);
        let rd16 = |o: usize| u16::from_le_bytes([hdr[o], hdr[o + 1]]);
        let pe = rd32(0x3C) as usize;
        let nsec = rd16(pe + 6) as usize;
        let opt = rd16(pe + 20) as usize;
        let mut secs = Vec::new();
        let mut so = pe + 24 + opt;
        for _ in 0..nsec {
            if so + 40 > hdr.len() {
                break;
            }
            let name = String::from_utf8_lossy(&hdr[so..so + 8]).trim_end_matches('\0').to_string();
            secs.push((name, rd32(so + 12), rd32(so + 8)));
            so += 40;
        }
        Ok(Self { base, secs })
    }

    fn section_of(&self, rva: u64) -> Option<&str> {
        self.secs
            .iter()
            .find(|(_, va, size)| rva >= *va as u64 && rva < (*va as u64 + *size as u64))
            .map(|(n, _, _)| n.as_str())
    }
}

fn hex(v: u64) -> String {
    format!("0x{:X}", v)
}

/// Run every check, print them, write `status.json`. Returns `true` when no
/// check failed (warnings do not fail the run).
pub fn run<P: Process + MemoryView>(
    process: &mut P,
    out_dir: &Path,
    generated_at: &str,
    build_number: Option<u32>,
    report: Option<&PatternReport>,
    offsets: Option<&OffsetMap>,
    protobufs: &ProtobufMap,
) -> Result<bool> {
    let mut checks: Vec<Check> = Vec::new();
    let mut sections: std::collections::BTreeMap<String, Sections> = Default::default();
    fn secs_for<'a, P: Process + MemoryView>(
        process: &mut P,
        sections: &'a mut std::collections::BTreeMap<String, Sections>,
        module: &str,
    ) -> Option<&'a Sections> {
        if !sections.contains_key(module) {
            let s = Sections::load(process, module).ok()?;
            sections.insert(module.to_string(), s);
        }
        sections.get(module)
    }

    // --- signatures: coverage + ambiguity --------------------------------
    if let Some(r) = report {
        let missing: Vec<Value> = r
            .hits
            .iter()
            .filter(|h| !h.found)
            .map(|h| json!({ "name": h.name, "module": h.module }))
            .collect();
        checks.push(if missing.is_empty() {
            Check::new("signatures_resolve", "pass", format!("{} of {} entries resolve", r.found, r.total), vec![])
        } else {
            Check::new(
                "signatures_resolve",
                "warn",
                format!("{} of {} entries resolve; {} need re-anchoring", r.found, r.total, missing.len()),
                missing,
            )
        });
        let ambiguous: Vec<Value> = r
            .hits
            .iter()
            .filter(|h| h.found && h.matches > 1)
            .map(|h| json!({ "name": h.name, "module": h.module, "matches": h.matches }))
            .collect();
        checks.push(if ambiguous.is_empty() {
            Check::new("patterns_unique", "pass", "every pattern matches exactly once in its module", vec![])
        } else {
            Check::new("patterns_unique", "fail", format!("{} pattern(s) match more than once", ambiguous.len()), ambiguous)
        });
    }

    // --- globals: a2x-style vs signature twins ----------------------------
    if let (Some(r), Some(off)) = (report, offsets) {
        let sig_rva = |module: &str, name: &str| -> Option<u64> {
            let dn = display_name(name);
            r.hits
                .iter()
                .find(|h| h.found && h.module.eq_ignore_ascii_case(module) && (h.name == name || h.name == dn || h.aliases.iter().any(|a| a == name)))
                .and_then(|h| h.rva)
        };
        let mut items = Vec::new();
        let mut bad = 0usize;
        let mut half = 0usize;
        for (module, dw, p) in TWINS {
            let g = off.get(*module).and_then(|m| m.get(*dw)).map(|v| *v as u64);
            let s = sig_rva(module, p);
            let ok = match (g, s) {
                (Some(a), Some(b)) => a == b,
                _ => false,
            };
            if g.is_some() && s.is_some() && !ok {
                bad += 1;
            }
            if g.is_none() != s.is_none() {
                half += 1;
            }
            items.push(json!({
                "module": module, "global": dw, "signature": p,
                "global_rva": g.map(hex), "signature_rva": s.map(hex),
                "ok": ok,
            }));
        }
        let status = if bad > 0 { "fail" } else if half > 0 { "warn" } else { "pass" };
        checks.push(Check::new(
            "global_twins",
            status,
            match status {
                "pass" => format!("{} global pairs agree", TWINS.len()),
                "warn" => format!("{} pair(s) have only one side resolved", half),
                _ => format!("{} pair(s) resolve to different addresses", bad),
            },
            items,
        ));
    }

    // --- globals: must live in a data section -----------------------------
    if let Some(off) = offsets {
        let mut items = Vec::new();
        let mut bad = 0usize;
        for (module, map) in off {
            let Some(s) = secs_for(process, &mut sections, module) else { continue };
            for (name, rva) in map {
                // dwOwner_member entries are struct member offsets, not RVAs.
                if name.len() > 2 && name[2..].contains('_') {
                    continue;
                }
                let sec = s.section_of(*rva as u64).unwrap_or("?").to_string();
                let ok = matches!(sec.as_str(), ".data" | ".bss" | ".didat");
                if !ok {
                    bad += 1;
                    items.push(json!({ "module": module, "name": name, "rva": hex(*rva as u64), "section": sec }));
                }
            }
        }
        checks.push(if bad == 0 {
            Check::new("globals_in_data", "pass", "every module-relative global lands in a data section", vec![])
        } else {
            Check::new("globals_in_data", "fail", format!("{} global(s) resolve outside .data", bad), items)
        });
    }

    // --- signatures: functions start on a prologue, globals sit in data ---
    if let Some(r) = report {
        let site_names: Vec<String> = SITES.iter().map(|s| display_name(s)).collect();
        let mut odd = Vec::new();
        let mut data_in_text = Vec::new();
        let mut checked = 0usize;
        for h in r.hits.iter().filter(|h| h.found) {
            let Some(rva) = h.rva else { continue };
            let Some(s) = secs_for(process, &mut sections, &h.module) else { continue };
            let base = s.base;
            let sec = s.section_of(rva).unwrap_or("?").to_string();
            // `riprel` entries named pXxx are globals (`mov rax,[rip+g]`); the
            // others are functions reached through a `lea rcx,[rip+fn]` and are
            // checked like any function below.
            let is_global = h.resolve == "riprel"
                && h.name.starts_with('p')
                && h.name.chars().nth(1).map(|c| c.is_ascii_uppercase()).unwrap_or(false);
            if is_global {
                if sec == ".text" {
                    data_in_text.push(json!({ "name": h.name, "module": h.module, "rva": hex(rva) }));
                }
                continue;
            }
            if site_names.iter().any(|n| *n == h.name) || SITES.iter().any(|n| *n == h.name) {
                continue;
            }
            checked += 1;
            if sec != ".text" {
                odd.push(json!({ "name": h.name, "module": h.module, "rva": hex(rva), "reason": format!("in {}", sec) }));
                continue;
            }
            let prev = process
                .read_raw(Address::from(base + rva - 1), 1)
                .ok()
                .and_then(|b| b.first().copied());
            match prev {
                Some(0xCC) | Some(0xC3) | Some(0x00) => {}
                Some(b) => odd.push(json!({ "name": h.name, "module": h.module, "rva": hex(rva), "reason": format!("byte before is 0x{:02X}, not padding", b) })),
                None => {}
            }
        }
        checks.push(if odd.is_empty() {
            Check::new("function_prologues", "pass", format!("{} function targets start after padding", checked), vec![])
        } else {
            Check::new("function_prologues", "warn", format!("{} of {} function targets do not follow padding (review)", odd.len(), checked), odd)
        });
        checks.push(if data_in_text.is_empty() {
            Check::new("globals_not_code", "pass", "no signature global resolves into .text", vec![])
        } else {
            Check::new("globals_not_code", "fail", format!("{} signature global(s) resolve into .text", data_in_text.len()), data_in_text)
        });
    }

    // --- protobuf layouts vs hand-verified engine structs -----------------
    {
        let client = protobufs.get("client.dll");
        let mut items = Vec::new();
        let mut compared = 0usize;
        let mut bad = 0usize;
        if let Some(msgs) = client {
            for es in ENGINE_STRUCTS {
                let Some(pm) = msgs.iter().find(|m| m.name == proto_name_of(es.name)) else { continue };
                if let Some(size) = es.size {
                    if size != pm.size {
                        bad += 1;
                        items.push(json!({ "struct": es.name, "field": "(size)", "engine": hex(size as u64), "protobuf": hex(pm.size as u64) }));
                    }
                }
                for f in es.fields {
                    let pf = pm
                        .fields
                        .iter()
                        .find(|p| p.name == f.name || Some(p.name.as_str()) == f.name.strip_prefix("m_"));
                    let Some(pf) = pf else { continue };
                    compared += 1;
                    if pf.offset != f.offset {
                        bad += 1;
                        items.push(json!({ "struct": es.name, "field": f.name, "engine": hex(f.offset as u64), "protobuf": hex(pf.offset as u64) }));
                    }
                }
            }
        }
        checks.push(if client.is_none() {
            Check::new("protobufs_vs_engine", "warn", "no client.dll protobuf tables read", vec![])
        } else if bad == 0 {
            Check::new("protobufs_vs_engine", "pass", format!("{} fields agree with the hand-verified engine structs", compared), vec![])
        } else {
            Check::new("protobufs_vs_engine", "fail", format!("{} field(s) disagree with the hand-verified engine structs", bad), items)
        });
    }

    // --- weapons snapshot (informational) ---------------------------------
    let weapons = fs::read_to_string(out_dir.join("weapons").join("weapons.json"))
        .ok()
        .and_then(|s| serde_json::from_str::<Value>(&s).ok())
        .and_then(|v| v.get("weapon_count").and_then(|c| c.as_u64()));
    checks.push(match weapons {
        Some(n) if n >= 40 => Check::new("weapons_snapshot", "pass", format!("{} weapons read from the session", n), vec![]),
        Some(n) => Check::new("weapons_snapshot", "warn", format!("only {} weapons present in the session; dump in a match with every weapon spawned for full coverage", n), vec![]),
        None => Check::new("weapons_snapshot", "warn", "no weapon entities in the session", vec![]),
    });

    // --- game version from steam.inf --------------------------------------
    let mut client_version: Option<String> = None;
    let mut patch_version: Option<String> = None;
    if let Ok(info) = process.module_by_name("client.dll") {
        let p: &str = info.path.as_ref();
        // .../game/csgo/bin/win64/client.dll -> .../game/csgo/steam.inf
        if let Some(csgo) = Path::new(p).ancestors().nth(3) {
            if let Ok(inf) = fs::read_to_string(csgo.join("steam.inf")) {
                for line in inf.lines() {
                    if let Some(v) = line.strip_prefix("ClientVersion=") {
                        client_version = Some(v.trim().to_string());
                    }
                    if let Some(v) = line.strip_prefix("PatchVersion=") {
                        patch_version = Some(v.trim().to_string());
                    }
                }
            }
        }
    }

    // --- print + write -----------------------------------------------------
    let mut ok = true;
    for c in &checks {
        let line = format!("{}: {}", c.name, c.detail);
        match c.status {
            "pass" => crate::ui::ok(&line),
            "warn" => crate::ui::warn(&line),
            _ => {
                ok = false;
                crate::ui::err(&line);
            }
        }
    }
    let status = json!({
        "build_number": build_number,
        "client_version": client_version,
        "patch_version": patch_version,
        "generated_at": generated_at,
        "dumper_version": env!("CARGO_PKG_VERSION"),
        "ok": ok,
        "summary": {
            "pass": checks.iter().filter(|c| c.status == "pass").count(),
            "warn": checks.iter().filter(|c| c.status == "warn").count(),
            "fail": checks.iter().filter(|c| c.status == "fail").count(),
        },
        "checks": checks.iter().map(|c| json!({ "name": c.name, "status": c.status, "detail": c.detail, "items": c.items })).collect::<Vec<_>>(),
        "signatures": report.map(|r| json!({
            "total": r.total, "found": r.found, "unique_functions": r.unique_functions,
            "missing": r.hits.iter().filter(|h| !h.found).map(|h| json!({ "name": h.name, "module": h.module })).collect::<Vec<_>>(),
        })),
        "note": "Written by cs2-sdk after every dump. `ok` is false when a check failed; warnings list what to look at. The auto-dump pipeline adds `update`, `auto_healed` and `known_issues`.",
    });
    fs::write(out_dir.join("status.json"), serde_json::to_string_pretty(&status)?)?;
    Ok(ok)
}

#[allow(dead_code)]
fn _hit_name(h: &PatternHit) -> &str {
    &h.name
}
