use std::collections::{BTreeMap, BTreeSet};
use std::fs;
use std::path::Path;

use anyhow::Result;
use chrono::Utc;

use crate::analysis::*;

mod buttons;

pub mod amalgamation;
pub mod convars;
pub mod engine_structs;
pub mod entities;
pub mod entity_system;
pub mod gameevents;
pub mod ident;
pub mod interface_classes;
pub mod netvars;
pub mod protobufs;
pub mod sdk_classes;
pub mod verified;
pub mod vtables;
pub mod weapons;

/// Emit the C++ SDK outputs:
///   <out>/macros.hpp                  SCHEMA_FIELD macros
///   <out>/schemas/<module>_dll.hpp    typed schema classes per module
///   <out>/netvars.{json,hpp}          split networked fields
///   <out>/interfaces/                 (interfaces.hpp written by caller)
///   <out>/impl/entity_system.hpp      entity system helpers
///   <out>/cs2.hpp                     single-include amalgamation
///   <out>/verified_features.json      hand-curated catalogue
pub fn dump_sdk_extras(
    out_dir: &Path,
    result: &AnalysisResult,
    build_number: Option<u32>,
) -> Result<()> {
    let schemas_dir = out_dir.join("schemas");
    fs::create_dir_all(out_dir)?;
    fs::create_dir_all(&schemas_dir)?;

    let ts = Utc::now().to_rfc3339();

    // 1. shared SCHEMA_FIELD macros + per-module schema headers
    let module_data = sdk_classes::render_module_headers(&result.schemas, &result.buttons, build_number, &ts);

        // Render base macros (includes a rich set of global forward-decls / minimal types)
        let macros_path = out_dir.join("macros.hpp");
        let mut macros = sdk_classes::render_macros_header();

        // Scan all module bodies for cross-module ::sdk::NAMESPACE::Type references
        // and for enum definitions so we can emit safe forward-declarations in the
        // macros header. Forward-declarations avoid ordering problems between
        // per-module headers when included together.
        let mut namespace_blocks: BTreeMap<String, BTreeSet<String>> = BTreeMap::new();
        let mut enum_underlying: BTreeMap<(String, String), String> = BTreeMap::new();

        // The set of module namespaces, so we can recognise cross-module
        // qualified references of the form `::<module>::<Type>` (they no longer
        // carry the old `::sdk::` prefix).
        let parse_mod_ns = |body: &str| -> Option<String> {
            let p = body.find("\nnamespace ")?;
            let s = p + "\nnamespace ".len();
            let b = body.as_bytes();
            let mut e = s;
            while e < body.len() {
                let c = b[e] as char;
                if c.is_whitespace() || c == '{' { break; }
                e += 1;
            }
            Some(body[s..e].to_string())
        };
        let module_ns_set: BTreeSet<String> =
            module_data.iter().filter_map(|(_, body)| parse_mod_ns(body)).collect();

        for (_file_name, body) in &module_data {
            // Parse the current module namespace from the header preamble — the
            // first line-anchored `namespace <module> {` (no root prefix now).
            let mut current_ns = String::new();
            if let Some(ns_pos) = body.find("\nnamespace ") {
                let ns_start = ns_pos + "\nnamespace ".len();
                let mut ns_end = ns_start;
                while ns_end < body.len() {
                    let c = body.as_bytes()[ns_end] as char;
                    if c.is_whitespace() || c == '{' { break; }
                    ns_end += 1;
                }
                current_ns = body[ns_start..ns_end].to_string();
            }

            // Collect enum definitions and their underlying types scoped to this module
            let mut scan_idx = 0usize;
            let bytes = body.as_bytes();
            while let Some(found) = body[scan_idx..].find("enum class") {
                let pos = scan_idx + found;
                let mut name_start = pos + "enum class".len();
                // skip whitespace
                while name_start < bytes.len() && (bytes[name_start] as char).is_whitespace() {
                    name_start += 1;
                }
                let mut name_end = name_start;
                while name_end < bytes.len() {
                    let c = bytes[name_end] as char;
                    if c.is_ascii_alphanumeric() || c == '_' { name_end += 1; } else { break; }
                }
                if name_end > name_start {
                    let name = body[name_start..name_end].trim().to_string();
                    // Look for ':' and '{' to extract underlying type
                    let rest = &body[name_end..];
                    if let Some(colon_rel) = rest.find(':') {
                        if let Some(brace_rel) = rest.find('{') {
                            if brace_rel > colon_rel {
                                let underlying = rest[colon_rel + 1..brace_rel].trim().to_string();
                                enum_underlying.insert((current_ns.clone(), name), underlying);
                            }
                        }
                    }
                }
                scan_idx = pos + 1;
            }

            // Collect cross-module qualified uses like ::resourcesystem::Type
            // (root-less now) — anchor on `::<module>::<Type>` where <module> is
            // a known schema namespace.
            let b2 = body.as_bytes();
            let mut search_idx = 0usize;
            while let Some(found) = body[search_idx..].find("::") {
                let ns_start = search_idx + found + 2;
                let mut ns_end = ns_start;
                while ns_end < body.len() {
                    let c = b2[ns_end] as char;
                    if c.is_ascii_alphanumeric() || c == '_' { ns_end += 1; } else { break; }
                }
                let ns = &body[ns_start..ns_end];
                if ns_end > ns_start
                    && ns_end + 2 <= body.len()
                    && &body[ns_end..ns_end + 2] == "::"
                    && module_ns_set.contains(ns)
                {
                    let type_start = ns_end + 2;
                    let mut type_end = type_start;
                    while type_end < body.len() {
                        let c = b2[type_end] as char;
                        if c.is_ascii_alphanumeric() || c == '_' { type_end += 1; } else { break; }
                    }
                    if type_end > type_start {
                        let ty = &body[type_start..type_end];
                        namespace_blocks
                            .entry(ns.to_string())
                            .or_insert_with(BTreeSet::new)
                            .insert(ty.to_string());
                    }
                    search_idx = type_end;
                } else {
                    search_idx = ns_start;
                }
            }
        }

        macros.push_str("\n// ============================================================================\n");
        macros.push_str("// Cross-module forward declarations (auto-generated)\n");
        macros.push_str("// These provide declaration-only stubs for types referenced across\n");
        macros.push_str("// different sdk::<module> namespaces so headers can be included in any\n");
        macros.push_str("// order. They are intentionally declaration-only; the real definitions\n");
        macros.push_str("// live in the per-module headers.\n\n");

        for (ns, types_set) in &namespace_blocks {
            macros.push_str(&format!("namespace {} {{\n", ns));
            for ty in types_set {
                if let Some(under) = enum_underlying.get(&(ns.clone(), ty.clone())) {
                    macros.push_str(&format!("    enum class {} : {};\n", ty, under));
                } else {
                    macros.push_str(&format!("    class {};\n", ty));
                }
            }
            macros.push_str("}\n\n");
        }

        // Auto: forward-declare every type the runtime (non-editor) schemas
        // reference but that isn't defined in any included header. SCHEMA_FIELD
        // accessors only reinterpret_cast, so an incomplete type is enough —
        // this keeps the amalgamation self-compiling with zero hand maintenance.
        emit_auto_forward_decls(&mut macros, &module_data, &module_ns_set);

        fs::write(macros_path, macros)?;

        // 2. per-module SDK class headers → schemas/. Skip empty modules
        //    (namespace shell only).
        let mut module_stems = Vec::new();
        for (file_name, body) in module_data {
            let is_empty = !body.contains("class ") && !body.contains("enum class ");
            if is_empty { continue; }
            fs::write(schemas_dir.join(&file_name), body)?;
            if let Some(stem) = file_name.strip_suffix(".hpp") {
                module_stems.push(stem.to_string());
            }
        }

        // 2b. structured schema JSON (classes/enums with size/offsets/metadata)
        //     for the site's class browser and machine consumers.
        fs::write(
            schemas_dir.join("schemas.json"),
            sdk_classes::render_schemas_json(&result.schemas),
        )?;

        // 3. netvars (split from schema). Only emit if non-empty.
        let nvs = netvars::extract(&result.schemas);
        if !nvs.is_empty() {
            fs::write(out_dir.join("netvars.json"), netvars::render_json(&nvs))?;
            fs::write(out_dir.join("netvars.hpp"), netvars::render_hpp(&nvs, build_number))?;
        }

        // 4. interfaces directory is created here; the combined header is written
        //    by main.rs after the vtable analysis stage produces the class list.
        fs::create_dir_all(out_dir.join("interfaces"))?;

        // 4b. entity system helpers — an implementation header (real code that
        //     uses the SDK), kept under impl/ to separate it from pure SDK data.
        let impl_dir = out_dir.join("impl");
        fs::create_dir_all(&impl_dir)?;
        fs::write(
            impl_dir.join("entity_system.hpp"),
            entity_system::render_hpp(&result.offsets, build_number, &result.schemas),
        )?;

        // 5. amalgamation
        fs::write(
            out_dir.join("cs2.hpp"),
            amalgamation::render_hpp(&module_stems, build_number),
        )?;

        // 6. verified_features.json is written after the pattern pass (main.rs)
        //    so each recipe carries this build's function RVAs and patterns.

        // 7. engine (non-schema) struct layouts are written after the pattern
        //    pass (write_engine_structs) so their function/instance RVAs come
        //    from this run instead of being hardcoded.

    Ok(())
}

/// engine/engine_structs.json + a drop-in .h per struct. Called after the
/// pattern pass: function and instance RVAs are looked up in `hits` by name.
pub fn write_engine_structs(
    out_dir: &Path,
    build_number: Option<u32>,
    hits: &[crate::patterns::PatternHit],
) -> Result<()> {
    let engine_dir = out_dir.join("engine");
    fs::create_dir_all(&engine_dir)?;
    fs::write(
        engine_dir.join("engine_structs.json"),
        engine_structs::render_json(build_number, hits),
    )?;
    for s in engine_structs::ENGINE_STRUCTS {
        fs::write(
            engine_dir.join(format!("{}.h", s.name.to_ascii_lowercase())),
            engine_structs::render_header(s, build_number, hits),
        )?;
    }
    Ok(())
}

/// Modules excluded from the runtime amalgamation (editor/tool only). Kept in
/// sync with `amalgamation::EDITOR_MODULES` — we don't scan them for the
/// forward-decl pass since their schemas aren't compiled by runtime consumers.
const EDITOR_MODULE_STEMS: &[&str] = &[
    "assetbrowser_dll",
    "assetpreview_dll",
    "assetrename_dll",
    "met_dll",
    "modtools_dll",
    "resourcecompiler_dll",
];

fn is_builtin_type(t: &str) -> bool {
    matches!(
        t,
        "bool" | "char" | "double" | "float" | "void" | "int" | "short" | "long"
            | "unsigned" | "signed" | "wchar_t" | "float32" | "fltx4" | "char8_t"
            | "char16_t" | "char32_t" | "size_t" | "std" | "const" | "volatile"
    )
}

/// Collect identifiers introduced by a definition keyword (`class Foo`,
/// `using Foo =`, `typedef ... Foo;`, …) into `set`.
fn collect_defined(text: &str, set: &mut BTreeSet<String>) {
    for kw in ["class ", "struct ", "enum class ", "enum ", "using ", "typedef "] {
        let mut from = 0;
        while let Some(p) = text[from..].find(kw) {
            let mut i = from + p + kw.len();
            let b = text.as_bytes();
            while i < text.len() && (b[i] as char).is_ascii_whitespace() {
                i += 1;
            }
            let start = i;
            while i < text.len() {
                let c = b[i] as char;
                if c.is_ascii_alphanumeric() || c == '_' { i += 1; } else { break; }
            }
            if i > start {
                set.insert(text[start..i].to_string());
            }
            from = from + p + kw.len();
        }
    }
}

/// Auto-detect and forward-declare every type referenced by the runtime
/// schemas' `SCHEMA_FIELD(...)` type arguments that isn't defined anywhere
/// they include. Handles plain, templated (`Foo<...>`) and cross-module
/// qualified (`ns::Foo`) names. Qualified names whose namespace is a real
/// emitted schema module are left to the existing cross-module pass.
fn emit_auto_forward_decls(
    macros: &mut String,
    module_data: &[(String, String)],
    module_ns_set: &BTreeSet<String>,
) {
    let is_editor = |fname: &str| EDITOR_MODULE_STEMS.iter().any(|e| fname.starts_with(e));

    // 1. Everything defined in the compiled set (runtime bodies + macros header).
    let mut defined = BTreeSet::<String>::new();
    collect_defined(macros, &mut defined);
    for (fname, body) in module_data {
        if is_editor(fname) { continue; }
        collect_defined(body, &mut defined);
    }

    // 2. Walk every SCHEMA_FIELD type argument and bucket undefined references.
    let mut plain = BTreeSet::<String>::new();
    let mut templated = BTreeSet::<String>::new();
    let mut qual: BTreeMap<String, BTreeMap<String, bool>> = BTreeMap::new();

    for (fname, body) in module_data {
        if is_editor(fname) { continue; }
        let bytes = body.as_bytes();
        let mut from = 0;
        const TAG: &str = "SCHEMA_FIELD(";
        while let Some(p) = body[from..].find(TAG) {
            let mut i = from + p + TAG.len();
            // Capture the type argument up to the top-level comma.
            let tstart = i;
            let mut depth = 0i32;
            while i < body.len() {
                match bytes[i] as char {
                    '<' => depth += 1,
                    '>' => depth -= 1,
                    ',' if depth <= 0 => break,
                    _ => {}
                }
                i += 1;
            }
            let targ = &body[tstart..i];
            extract_type_idents(targ, &defined, module_ns_set, &mut plain, &mut templated, &mut qual);
            from = from + p + TAG.len();
        }
        // Types that only appear behind a `using _TypeN = ...;` alias (template
        // arguments with commas are hoisted into one) would otherwise be missed.
        let mut from = 0;
        const ALIAS: &str = "using _Type";
        while let Some(p) = body[from..].find(ALIAS) {
            let start = from + p;
            let end = body[start..].find(';').map(|e| start + e).unwrap_or(body.len());
            if let Some(eq) = body[start..end].find('=') {
                extract_type_idents(&body[start + eq + 1..end], &defined, module_ns_set, &mut plain, &mut templated, &mut qual);
            }
            from = end;
        }
    }
    for t in templated.iter() {
        plain.remove(t);
    }

    if plain.is_empty() && templated.is_empty() && qual.is_empty() {
        return;
    }

    macros.push_str("// Auto-generated: forward declarations for types the runtime schemas\n");
    macros.push_str("// reference but that aren't defined in any included header.\n");
    for t in &templated {
        macros.push_str(&format!("template <class...> class {};\n", t));
    }
    for t in &plain {
        macros.push_str(&format!("class {};\n", t));
    }
    for (ns, names) in &qual {
        macros.push_str(&format!("namespace {} {{ ", ns));
        for (name, is_t) in names {
            if *is_t {
                macros.push_str(&format!("template <class...> class {}; ", name));
            } else {
                macros.push_str(&format!("class {}; ", name));
            }
        }
        macros.push_str("}\n");
    }
    macros.push('\n');
}

/// Pull every type identifier out of a `SCHEMA_FIELD` type argument (including
/// nested template arguments) and bucket the undefined ones.
fn extract_type_idents(
    arg: &str,
    defined: &BTreeSet<String>,
    module_ns_set: &BTreeSet<String>,
    plain: &mut BTreeSet<String>,
    templated: &mut BTreeSet<String>,
    qual: &mut BTreeMap<String, BTreeMap<String, bool>>,
) {
    let bytes = arg.as_bytes();
    let mut i = 0;
    while i < arg.len() {
        let c = bytes[i] as char;
        if c.is_ascii_alphabetic() || c == '_' || c == ':' {
            let start = i;
            while i < arg.len() {
                let d = bytes[i] as char;
                if d.is_ascii_alphanumeric() || d == '_' || d == ':' { i += 1; } else { break; }
            }
            let tok = arg[start..i].trim_matches(':');
            if tok.is_empty() {
                continue;
            }
            // Template usage = next non-space char is '<'.
            let mut j = i;
            while j < arg.len() && (bytes[j] as char).is_ascii_whitespace() {
                j += 1;
            }
            let is_t = j < arg.len() && bytes[j] as char == '<';

            if let Some((ns, name)) = tok.rsplit_once("::") {
                let ns = ns.split("::").next().unwrap_or(ns);
                // Qualified into a real schema module → handled elsewhere; into
                // an unknown namespace (e.g. rendersystemdx11) → declare it here.
                if !is_builtin_type(ns) && !module_ns_set.contains(ns) && !defined.contains(name) {
                    let e = qual.entry(ns.to_string()).or_default();
                    let v = e.entry(name.to_string()).or_insert(false);
                    *v = *v || is_t;
                }
            } else if !is_builtin_type(tok) && !defined.contains(tok) {
                if is_t { templated.insert(tok.to_string()); } else { plain.insert(tok.to_string()); }
            }
        } else {
            i += 1;
        }
    }
}

pub fn write_buttons(out_dir: &Path, buttons: &ButtonMap) -> Result<()> {
    fs::create_dir_all(out_dir)?;
    fs::write(out_dir.join("buttons.hpp"),  buttons::render_hpp(buttons))?;
    fs::write(out_dir.join("buttons.json"), buttons::render_json(buttons))?;
    Ok(())
}
