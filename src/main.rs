// cs2-universal-dumper
// --------------------
// One binary, two passes:
//   * offsets    (external offset/interface/schema/button dumper)
//   * signatures (PE/section-aware IDA-style scanner with
//                 rel32 / riprel / string-ref resolution)
//
// Output layout:
//
//     <OutputRoot>/<DD-MM-YY>-CS2-SDK/
//         manifest.json
//         logs/cs2-sdk.log
//         offsets/
//             buttons.(cs|hpp|json|rs|zig)
//             interfaces.(cs|hpp|json|rs|zig)
//             offsets.(cs|hpp|json|rs|zig)
//             <schema modules>.(cs|hpp|json|rs|zig)
//             info.json
//             # SDK extras (cheat-developer-friendly outputs):
//             cs2sdk.hpp                       single-include amalgamation
//             cs2sdk.rs                        Rust amalgamation module
//             netvars.(json|hpp|cs)            split networked-field offsets
//             interfaces_sdk.(hpp|cs)          typed accessor stubs
//             sdk/cs2sdk_macros.hpp            SCHEMA_FIELD macro family
//             sdk/<module>.hpp                 typed schema classes
//         signatures/
//             signatures.json   (hand-formatted, one entry per line)
//             signatures.cs     (C#  static class per module)
//             signatures.hpp    (C++ namespace per module)
//             signatures.rs     (Rust module per module)
//             SIGNATURES.md     (human-readable table)
//             diff.json         (delta vs. previous session, when found)
//
//     <OutputRoot>/latest/                     mirror of the most recent
//                                              successful session

use std::collections::BTreeMap;
use std::fs::{self, File};
use std::path::{Path, PathBuf};
use std::str::FromStr;
use std::time::{Duration, Instant};

use anyhow::{Context, Result};
use chrono::Local;
use clap::{ArgAction, Parser};
use log::LevelFilter;
use memflow::prelude::v1::*;
use serde_json::json;
use simplelog::*;

use output::Output;
use signatures::SignatureCache;

mod analysis;
mod memory;
mod output;
mod signatures;
mod source2;
mod ui;

#[derive(Debug, Parser)]
#[command(author, version, about = "CS2 Universal Dumper â€” offsets + signatures in one run")]
struct Args {
    #[arg(short, long)]
    connector: Option<String>,

    #[arg(short = 'a', long)]
    connector_args: Option<String>,

    #[arg(short, long, value_delimiter = ',', default_values = ["cs", "hpp", "json", "rs", "zig"])]
    file_types: Vec<String>,

    #[arg(short, long, default_value_t = 4)]
    indent_size: usize,

    /// Parent directory for the dated session folder.
    #[arg(short, long, default_value = "dumps")]
    output: PathBuf,

    #[arg(short, long, default_value = "cs2.exe")]
    process_name: String,

    #[arg(short, long, action = ArgAction::Count)]
    verbose: u8,

    #[arg(long)]
    skip_offsets: bool,

    #[arg(long)]
    skip_signatures: bool,

    #[arg(long)]
    no_sound: bool,

    /// Path to a previous `signatures.json` to use as a hot cache.  When
    /// the cached `match_rva` still matches the recorded pattern bytes,
    /// the entry is satisfied without a full module scan.  If omitted the
    /// most recent session under `--output` is used automatically.
    #[arg(long)]
    cache: Option<PathBuf>,

    /// Disable the automatic "use previous session as cache" behaviour.
    #[arg(long)]
    no_cache: bool,
}

fn main() -> Result<()> {
    let args = Args::parse();

    ui::init(args.no_sound);
    ui::banner();
    ui::sound(ui::Cue::Start);

    let now = Local::now();
    let session_name = format!("{}-CS2-SDK", now.format("%d-%m-%y"));
    let session_dir = args.output.join(&session_name);
    let offsets_dir = session_dir.join("offsets");
    let sigs_dir = session_dir.join("signatures");
    let logs_dir = session_dir.join("logs");
    for d in [&session_dir, &offsets_dir, &sigs_dir, &logs_dir] {
        fs::create_dir_all(d)
            .with_context(|| format!("failed to create {}", d.display()))?;
    }

    init_logging(&logs_dir, args.verbose)?;

    ui::section("Session");
    ui::kv("Timestamp", &now.format("%Y-%m-%d %H:%M:%S").to_string());
    ui::kv("Session", &session_name);
    ui::kv("Root", &session_dir.display().to_string());
    ui::kv("Process", &args.process_name);
    ui::kv("File types", &args.file_types.join(","));
    ui::kv("Offsets", if args.skip_offsets { "skipped" } else { "enabled" });
    ui::kv("Signatures", if args.skip_signatures { "skipped" } else { "enabled" });

    ui::section("Attach");
    let mut os = build_os(&args)?;
    let mut process = os
        .process_by_name(&args.process_name)
        .with_context(|| format!("process '{}' not found", args.process_name))?;
    let pid = process.info().pid;
    ui::ok(&format!("attached to {} (pid {})", args.process_name, pid));

    // --- stage 1: offsets --------------------------------------------------
    let mut offsets_ok = true;
    let mut offsets_elapsed = Duration::ZERO;
    let mut build_number: Option<u32> = None;
    let mut analysis_result: Option<analysis::AnalysisResult> = None;

    if !args.skip_offsets {
        ui::section("Offsets, interfaces, buttons, schemas");
        ui::sound(ui::Cue::Step);
        let t0 = Instant::now();

        match analysis::analyze_all(&mut process) {
            Ok(result) => {
                ui::ok(&format!(
                    "interfaces: {} across {} modules",
                    result.interfaces.iter().map(|(_, v)| v.len()).sum::<usize>(),
                    result.interfaces.len()
                ));
                ui::ok(&format!(
                    "offsets: {} across {} modules",
                    result.offsets.iter().map(|(_, v)| v.len()).sum::<usize>(),
                    result.offsets.len()
                ));
                let (cc, ec) = result.schemas.values().fold((0, 0), |(c, e), (cv, ev)| {
                    (c + cv.len(), e + ev.len())
                });
                ui::ok(&format!(
                    "schemas: {} classes, {} enums across {} modules",
                    cc, ec, result.schemas.len()
                ));

                let out = Output::new(
                    &args.file_types,
                    args.indent_size,
                    &offsets_dir,
                    &result,
                )?;
                out.dump_all(&mut process)?;

                build_number = result
                    .offsets
                    .iter()
                    .find_map(|(mname, offs)| {
                        let m = process.module_by_name(mname).ok()?;
                        let o = offs.iter().find(|(n, _)| *n == "dwBuildNumber")?.1;
                        process.read::<u32>(m.base + o).data_part().ok()
                    });

                // Emit the cheat-developer-friendly SDK extras (typed schema
                // classes, netvars split-out, interface accessor stubs,
                // single-include amalgamation). Pure-additive — the original
                // outputs above are untouched.
                if let Err(e) = out.dump_sdk_extras(build_number) {
                    ui::warn(&format!("sdk extras emitter failed: {}", e));
                } else {
                    ui::ok("sdk extras (classes, netvars, accessors, amalgamation) emitted");
                }

                let total_vts: usize = result.vtables.values().map(|m| m.len()).sum();
                let total_methods: usize = result
                    .vtables
                    .values()
                    .flat_map(|m| m.values())
                    .map(|i| i.methods.len())
                    .sum();
                let total_rtti: usize = result
                    .vtables
                    .values()
                    .flat_map(|m| m.values())
                    .filter(|i| i.rtti_class.is_some())
                    .count();
                if total_vts > 0 {
                    ui::ok(&format!(
                        "vtables: {} interfaces, {} method slots, {} RTTI class names",
                        total_vts, total_methods, total_rtti
                    ));
                }

                offsets_elapsed = t0.elapsed();
                ui::ok(&format!("offsets pass completed in {}", ui::fmt_duration(offsets_elapsed)));
                drop(out);
                analysis_result = Some(result);
            }
            Err(e) => {
                offsets_ok = false;
                ui::err(&format!("offsets pass failed: {}", e));
            }
        }
    }

    // --- stage 2: signatures ----------------------------------------------
    let mut sigs_ok = true;
    let mut sig_report: Option<signatures::SignatureReport> = None;

    if !args.skip_signatures {
        ui::section("Signatures (PE/section aware)");
        ui::sound(ui::Cue::Step);

        // Build the warm-start cache.  Explicit `--cache` wins; otherwise
        // we look for the newest sibling session folder.
        let cache_path = args.cache.clone().or_else(|| {
            if args.no_cache {
                None
            } else {
                find_previous_signatures_json(&args.output, &session_name)
            }
        });
        let cache = match &cache_path {
            Some(p) => match SignatureCache::load(p) {
                Ok(c) => {
                    if !c.is_empty() {
                        ui::info(&format!("warm cache from {} ({} entries)", p.display(), c.len()));
                    }
                    c
                }
                Err(e) => {
                    ui::warn(&format!("cache load failed ({}): {}", p.display(), e));
                    SignatureCache::default()
                }
            },
            None => SignatureCache::default(),
        };

        match signatures::scan_all_with_cache(
            &mut process,
            signatures::database::CS2_SIGNATURES,
            &cache,
        ) {
            Ok(report) => {
                ui::ok(&format!(
                    "{}/{} signatures resolved across {} module(s) in {}",
                    report.found,
                    report.total,
                    report.modules.len(),
                    ui::fmt_duration(Duration::from_millis(report.elapsed_ms as u64))
                ));

                let json_path = sigs_dir.join("signatures.json");
                fs::write(&json_path, format_found_signatures(&report))?;
                ui::ok(&format!("wrote {}", json_path.display()));

                // Multi-language fan-out.
                fs::write(sigs_dir.join("signatures.cs"), signatures::writers::render_cs(&report.hits))?;
                fs::write(sigs_dir.join("signatures.hpp"), signatures::writers::render_hpp(&report.hits))?;
                fs::write(sigs_dir.join("signatures.rs"), signatures::writers::render_rs(&report.hits))?;
                fs::write(sigs_dir.join("SIGNATURES.md"), signatures::writers::render_markdown(&report.hits))?;

                // Diff vs. previous session, if any.
                if let Some(prev) = &cache_path
                    && let Ok(diff) = signatures::diff::diff_against(prev, &report)
                {
                    let path = sigs_dir.join("diff.json");
                    fs::write(&path, serde_json::to_string_pretty(&diff)?)?;
                    let n = diff.added.len() + diff.removed.len() + diff.shifted.len();
                    if n > 0 {
                        ui::info(&format!(
                            "diff vs previous: +{} -{} ~{} (pattern changes: {})",
                            diff.added.len(),
                            diff.removed.len(),
                            diff.shifted.len(),
                            diff.pattern_changed.len(),
                        ));
                    }
                }

                sig_report = Some(report);
            }
            Err(e) => {
                sigs_ok = false;
                ui::err(&format!("signatures pass failed: {}", e));
            }
        }
    }

    // --- stage 3: vtables emit (post-sigs so we can use sig hits as a
    // method-name oracle: a vtable slot whose RVA matches a known
    // signature is labelled with that signature's name).
    if let Some(result) = analysis_result.as_ref() {
        if !result.vtables.is_empty() {
            let oracle = sig_report
                .as_ref()
                .map(|r| output::vtables::name_oracle_from_signatures(&r.hits))
                .unwrap_or_default();
            let json = output::vtables::render_json(&result.vtables, &oracle);
            let hpp  = output::vtables::render_hpp(&result.vtables, &oracle, build_number);
            let cs   = output::vtables::render_cs(&result.vtables, &oracle, build_number);
            let _ = fs::write(offsets_dir.join("vtables.json"), json);
            let _ = fs::write(offsets_dir.join("vtables.hpp"),  hpp);
            let _ = fs::write(offsets_dir.join("vtables.cs"),   cs);
            let labelled = result
                .vtables
                .values()
                .flat_map(|m| m.values())
                .flat_map(|i| i.methods.iter())
                .filter(|m| oracle.contains_key(&(m.module.clone(), m.rva)))
                .count();
            ui::ok(&format!(
                "vtables emitted (vtables.{{json,hpp,cs}}) — {} slots labelled from signatures",
                labelled
            ));
        }
    }

    // --- manifest ----------------------------------------------------------
    let sig_counts = sig_report
        .as_ref()
        .map(|r| json!({ "found": r.found, "total": r.total }))
        .unwrap_or(json!(null));

    // Module fingerprints (PE timestamp + image size) for the major
    // CS2 modules — lets consumers detect a build mismatch instantly.
    let module_fingerprints = collect_module_fingerprints(&mut process);

    let manifest = json!({
        "session": session_name,
        "generated_at": now.to_rfc3339(),
        "process": args.process_name,
        "build_number": build_number,
        "modules": module_fingerprints,
        "stages": {
            "offsets": {
                "enabled": !args.skip_offsets,
                "success": offsets_ok,
                "elapsed_ms": offsets_elapsed.as_millis(),
                "output_dir": offsets_dir,
            },
            "signatures": {
                "enabled": !args.skip_signatures,
                "success": sigs_ok,
                "counts": sig_counts,
                "elapsed_ms": sig_report.as_ref().map(|r| r.elapsed_ms),
                "output_dir": sigs_dir,
            },
        },
    });
    fs::write(
        session_dir.join("manifest.json"),
        serde_json::to_string_pretty(&manifest)?,
    )?;

    // --- summary -----------------------------------------------------------
    ui::section("Summary");
    ui::kv("Session dir", &session_dir.display().to_string());
    if !args.skip_offsets {
        ui::kv("Offsets", if offsets_ok { "ok" } else { "FAIL" });
    }
    if !args.skip_signatures {
        match &sig_report {
            Some(r) => ui::kv("Signatures", &format!("{} / {}", r.found, r.total)),
            None => ui::kv("Signatures", "FAIL"),
        }
    }
    if let Some(bn) = build_number {
        ui::kv("Build number", &bn.to_string());
    }

    ui::divider();
    let all_ok = offsets_ok && sigs_ok;
    if all_ok {
        // Mirror this session into <output>/latest/ so URLs of the form
        // raw.githubusercontent.com/.../dumps/latest/signatures/signatures.hpp
        // always resolve to the most recent successful dump.
        if let Err(e) = mirror_to_latest(&args.output, &session_dir) {
            ui::warn(&format!("latest/ mirror failed: {}", e));
        } else {
            ui::ok(&format!(
                "latest/ mirror updated -> {}",
                args.output.join("latest").display()
            ));
        }

        ui::sound(ui::Cue::Success);
        ui::step("All stages completed successfully.");
        Ok(())
    } else {
        ui::sound(ui::Cue::Failure);
        ui::err("One or more stages failed â€” see logs/cs2-sdk.log.");
        std::process::exit(1);
    }
}

fn build_os(args: &Args) -> Result<OsInstanceArcBox<'static>> {
    let conn_args = args
        .connector_args
        .as_deref()
        .map(ConnectorArgs::from_str)
        .transpose()
        .map_err(|e| anyhow::anyhow!("invalid connector args: {}", e))?
        .unwrap_or_default();

    match &args.connector {
        Some(conn) => {
            let mut inventory = Inventory::scan();
            Ok(inventory
                .builder()
                .connector(conn)
                .args(conn_args)
                .os("win32")
                .build()?)
        }
        None => {
            #[cfg(windows)]
            {
                Ok(memflow_native::create_os(&OsArgs::default(), LibArc::default())?)
            }
            #[cfg(not(windows))]
            {
                anyhow::bail!("no connector specified and no native backend on this platform")
            }
        }
    }
}

/// Locate the newest sibling session folder under `output` (skipping the
/// one we're currently writing) and return the path to its
/// `signatures/signatures.json` if it exists.
/// Used to enable warm-cache + diff behaviour without any explicit flag.
fn find_previous_signatures_json(output: &Path, current_session: &str) -> Option<PathBuf> {
    let mut candidates: Vec<(std::time::SystemTime, PathBuf)> = Vec::new();
    let entries = fs::read_dir(output).ok()?;
    for entry in entries.flatten() {
        let name = entry.file_name();
        let Some(name) = name.to_str() else { continue };
        if name == current_session || !name.ends_with("-CS2-SDK") {
            continue;
        }
        let sigs = entry.path().join("signatures").join("signatures.json");
        if !sigs.exists() {
            continue;
        }
        let mtime = entry
            .metadata()
            .and_then(|m| m.modified())
            .unwrap_or(std::time::UNIX_EPOCH);
        candidates.push((mtime, sigs));
    }
    candidates.sort_by(|a, b| b.0.cmp(&a.0));
    candidates.into_iter().next().map(|(_, p)| p)
}

/// Mirror `session_dir` into `<output>/latest/`, replacing any existing
/// content. The `latest/` folder is what consumers point their
/// `raw.githubusercontent.com/.../dumps/latest/...` URLs at, so it
/// always reflects the most recent successful dump.
///
/// We avoid an external `robocopy` dependency by walking the tree
/// ourselves; this also keeps behaviour identical on dev machines that
/// don't have robocopy on PATH.
fn mirror_to_latest(output_root: &Path, session_dir: &Path) -> Result<()> {
    let dest = output_root.join("latest");
    if dest.exists() {
        fs::remove_dir_all(&dest)
            .with_context(|| format!("failed to clear {}", dest.display()))?;
    }
    copy_tree(session_dir, &dest)?;
    Ok(())
}

fn copy_tree(src: &Path, dst: &Path) -> Result<()> {
    fs::create_dir_all(dst)?;
    for entry in fs::read_dir(src)? {
        let entry = entry?;
        let from = entry.path();
        let to = dst.join(entry.file_name());
        let ft = entry.file_type()?;
        if ft.is_dir() {
            copy_tree(&from, &to)?;
        } else if ft.is_file() {
            fs::copy(&from, &to)?;
        }
    }
    Ok(())
}

fn init_logging(logs_dir: &Path, verbose: u8) -> Result<()> {
    let level = match verbose {
        0 => LevelFilter::Warn,
        1 => LevelFilter::Info,
        2 => LevelFilter::Debug,
        _ => LevelFilter::Trace,
    };
    let mut loggers: Vec<Box<dyn SharedLogger>> = Vec::new();
    // Terminal: warnings+ only (UI is our main output).
    loggers.push(TermLogger::new(
        LevelFilter::Warn,
        Config::default(),
        TerminalMode::Mixed,
        ColorChoice::Auto,
    ));
    loggers.push(WriteLogger::new(
        level,
        Config::default(),
        File::create(logs_dir.join("cs2-sdk.log"))?,
    ));
    CombinedLogger::init(loggers).ok();
    Ok(())
}

/// Collect a per-module fingerprint (PE TimeDateStamp + image size +
/// base) for every loaded CS2 module of interest.  Lets consumers
/// confirm at a glance whether their cached SDK still matches the live
/// build, without having to re-run the full dumper.
///
/// We don't use `pelite::PeView::from_bytes` here because we only read
/// the first 4 KiB of headers from the live process — `PeView` requires
/// a complete image and would reject the truncated buffer.  The PE
/// header layout is fixed, so a hand-rolled parse is safe and avoids
/// the round-trip cost of dragging in a whole image just for one u32.
fn collect_module_fingerprints<P: Process + MemoryView>(
    process: &mut P,
) -> BTreeMap<String, serde_json::Value> {
    const MODULES: &[&str] = &[
        "client.dll",
        "engine2.dll",
        "server.dll",
        "schemasystem.dll",
        "animationsystem.dll",
        "materialsystem2.dll",
        "particles.dll",
        "scenesystem.dll",
        "soundsystem.dll",
        "tier0.dll",
        "vphysics2.dll",
        "networksystem.dll",
        "host.dll",
        "panorama.dll",
        "rendersystemdx11.dll",
        "resourcesystem.dll",
        "vstdlib.dll",
        "pulse_system.dll",
        "inputsystem.dll",
        "filesystem_stdio.dll",
    ];

    let mut out = BTreeMap::new();
    for name in MODULES {
        let Ok(m) = process.module_by_name(name) else {
            continue;
        };
        let hdr_size = (m.size as usize).min(0x1000);
        let Ok(hdr) = process.read_raw(m.base, hdr_size).data_part() else {
            continue;
        };
        if hdr.len() < 0x40 {
            continue;
        }
        let e_lfanew =
            u32::from_le_bytes(hdr[0x3C..0x40].try_into().unwrap()) as usize;
        // PE\0\0 at e_lfanew, then COFF FileHeader (20 bytes):
        //   Machine(2) NumberOfSections(2) TimeDateStamp(4) ...
        if e_lfanew + 24 > hdr.len() {
            continue;
        }
        if &hdr[e_lfanew..e_lfanew + 4] != b"PE\0\0" {
            continue;
        }
        let coff = e_lfanew + 4;
        let timestamp =
            u32::from_le_bytes(hdr[coff + 4..coff + 8].try_into().unwrap());

        // OptionalHeader.SizeOfImage lives at COFF + 20 + 56 (PE32+).
        let opt = coff + 20;
        let size_of_image = if opt + 60 <= hdr.len() {
            u32::from_le_bytes(hdr[opt + 56..opt + 60].try_into().unwrap())
        } else {
            m.size as u32
        };

        out.insert(
            (*name).to_string(),
            json!({
                "base":      format!("{:#X}", m.base.to_umem() as u64),
                "size":      m.size,
                "image":     size_of_image,
                "timestamp": timestamp,
            }),
        );
    }
    out
}

/// Pretty-print only successfully-resolved signatures, one hit per line.
/// Unfound entries are dropped entirely — they have no usable address.
fn format_found_signatures(report: &signatures::SignatureReport) -> String {
    let found: Vec<&signatures::SignatureHit> =
        report.hits.iter().filter(|h| h.found).collect();

    let name_w = found.iter().map(|h| h.name.len()).max().unwrap_or(0);
    let mod_w = found.iter().map(|h| h.module.len()).max().unwrap_or(0);
    let res_w = found.iter().map(|h| h.resolve.len()).max().unwrap_or(0);
    let pat_w = found.iter().map(|h| h.pattern.len()).max().unwrap_or(0);
    let byt_w = found
        .iter()
        .map(|h| h.bytes.as_deref().map(|b| b.len() + 2).unwrap_or(6))
        .max()
        .unwrap_or(6);
    let synth_w = found
        .iter()
        .map(|h| h.pattern_synth.as_deref().map(|b| b.len() + 2).unwrap_or(6))
        .max()
        .unwrap_or(6);

    let mut s = String::new();
    s.push_str("{\n");
    s.push_str(&format!("  \"total_scanned\":  {},\n", report.total));
    s.push_str(&format!("  \"found\":          {},\n", report.found));
    s.push_str(&format!("  \"missing\":        {},\n", report.total - report.found));
    s.push_str(&format!("  \"elapsed_ms\":     {},\n", report.elapsed_ms));
    s.push_str(&format!(
        "  \"modules\":        [{}],\n",
        report
            .modules
            .iter()
            .map(|m| format!("\"{}\"", m))
            .collect::<Vec<_>>()
            .join(", ")
    ));
    s.push_str("  \"signatures\": [\n");
    for (i, h) in found.iter().enumerate() {
        let comma = if i + 1 == found.len() { "" } else { "," };
        let va = h.va.map(|v| format!("0x{:X}", v)).unwrap_or_else(|| "null".into());
        let rva = h.rva.map(|v| format!("0x{:X}", v)).unwrap_or_else(|| "null".into());
        let bytes_field = h
            .bytes
            .as_deref()
            .map(|b| format!("\"{}\"", b))
            .unwrap_or_else(|| "null".into());
        let synth_field = h
            .pattern_synth
            .as_deref()
            .map(|b| format!("\"{}\"", b))
            .unwrap_or_else(|| "null".into());
        s.push_str(&format!(
            "    {{ \"name\": {:<nw$}, \"module\": {:<mw$}, \"resolve\": {:<rw$}, \"va\": {:>12}, \"rva\": {:>10}, \"pattern\": {:<pw$}, \"bytes\": {:<bw$}, \"pattern_synth\": {:<sw$} }}{}\n",
            format!("\"{}\"", h.name),
            format!("\"{}\"", h.module),
            format!("\"{}\"", h.resolve),
            va,
            rva,
            format!("\"{}\"", h.pattern),
            bytes_field,
            synth_field,
            comma,
            nw = name_w + 2,
            mw = mod_w + 2,
            rw = res_w + 2,
            pw = pat_w + 2,
            bw = byt_w,
            sw = synth_w,
        ));
    }
    s.push_str("  ]\n");
    s.push_str("}\n");
    s
}
