use std::fmt::{self, Write};
use std::fs;
use std::path::Path;

use anyhow::{Result, anyhow};

use chrono::{DateTime, Utc};

use memflow::prelude::v1::*;

use serde_json::json;

use formatter::Formatter;
use skinchanger::SkinchangerOutput;

use crate::analysis::*;

mod buttons;
mod formatter;
mod interfaces;
mod offsets;
mod schemas;
mod skinchanger;

// New SDK-style emitters (additive — leave the originals untouched).
pub mod amalgamation;
pub mod ident;
pub mod interfaces_sdk;
pub mod netvars;
pub mod sdk_classes;
pub mod verified;
pub mod vtables;

enum Item<'a> {
    Buttons(&'a ButtonMap),
    Interfaces(&'a InterfaceMap),
    Offsets(&'a OffsetMap),
    Schemas(&'a SchemaMap),
    Skinchanger(SkinchangerOutput<'a>),
}

impl<'a> Item<'a> {
    fn write(&self, fmt: &mut Formatter<'a>, file_type: &str) -> fmt::Result {
        match file_type {
            "cs" => self.write_cs(fmt),
            "hpp" => self.write_hpp(fmt),
            "json" => self.write_json(fmt),
            "rs" => self.write_rs(fmt),
            "zig" => self.write_zig(fmt),
            _ => unimplemented!(),
        }
    }
}

trait CodeWriter {
    fn write_cs(&self, fmt: &mut Formatter<'_>) -> fmt::Result;
    fn write_hpp(&self, fmt: &mut Formatter<'_>) -> fmt::Result;
    fn write_json(&self, fmt: &mut Formatter<'_>) -> fmt::Result;
    fn write_rs(&self, fmt: &mut Formatter<'_>) -> fmt::Result;
    fn write_zig(&self, fmt: &mut Formatter<'_>) -> fmt::Result;
}

impl<'a> CodeWriter for Item<'a> {
    fn write_cs(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_cs(fmt),
            Item::Interfaces(ifaces) => ifaces.write_cs(fmt),
            Item::Offsets(offsets) => offsets.write_cs(fmt),
            Item::Schemas(schemas) => schemas.write_cs(fmt),
            Item::Skinchanger(skinchanger) => skinchanger.write_cs(fmt),
        }
    }

    fn write_hpp(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_hpp(fmt),
            Item::Interfaces(ifaces) => ifaces.write_hpp(fmt),
            Item::Offsets(offsets) => offsets.write_hpp(fmt),
            Item::Schemas(schemas) => schemas.write_hpp(fmt),
            Item::Skinchanger(skinchanger) => skinchanger.write_hpp(fmt),
        }
    }

    fn write_json(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_json(fmt),
            Item::Interfaces(ifaces) => ifaces.write_json(fmt),
            Item::Offsets(offsets) => offsets.write_json(fmt),
            Item::Schemas(schemas) => schemas.write_json(fmt),
            Item::Skinchanger(skinchanger) => skinchanger.write_json(fmt),
        }
    }

    fn write_rs(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_rs(fmt),
            Item::Interfaces(ifaces) => ifaces.write_rs(fmt),
            Item::Offsets(offsets) => offsets.write_rs(fmt),
            Item::Schemas(schemas) => schemas.write_rs(fmt),
            Item::Skinchanger(skinchanger) => skinchanger.write_rs(fmt),
        }
    }

    fn write_zig(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_zig(fmt),
            Item::Interfaces(ifaces) => ifaces.write_zig(fmt),
            Item::Offsets(offsets) => offsets.write_zig(fmt),
            Item::Schemas(schemas) => schemas.write_zig(fmt),
            Item::Skinchanger(skinchanger) => skinchanger.write_zig(fmt),
        }
    }
}

pub struct Output<'a> {
    file_types: &'a [String],
    indent_size: usize,
    out_dir: &'a Path,
    result: &'a AnalysisResult,
    timestamp: DateTime<Utc>,
}

impl<'a> Output<'a> {
    pub fn new(
        file_types: &'a [String],
        indent_size: usize,
        out_dir: &'a Path,
        result: &'a AnalysisResult,
    ) -> Result<Self> {
        fs::create_dir_all(&out_dir)?;

        Ok(Self {
            file_types,
            indent_size,
            out_dir,
            result,
            timestamp: Utc::now(),
        })
    }

    /// Emit the additional cheat-developer-friendly SDK files alongside
    /// the original outputs. This is `dump_all`'s younger sibling — it
    /// reads the *same* `AnalysisResult` and writes:
    ///
    /// * `<offsets>/sdk/cs2sdk_macros.hpp`         — SCHEMA_FIELD macros
    /// * `<offsets>/sdk/<module>.hpp`              — typed schema classes
    /// * `<offsets>/netvars.{json,hpp,cs}`         — split networked fields
    /// * `<offsets>/interfaces_sdk.{hpp,cs}`       — typed accessor stubs
    /// * `<offsets>/cs2sdk.hpp`                    — single-include amalgamation
    /// * `<offsets>/cs2sdk.rs`                     — Rust amalgamation module
    ///
    /// `build_number` is pinned into every emitted file as `CS2_BUILD`
    /// so internal cheats can `static_assert` against the running game.
    pub fn dump_sdk_extras(&self, build_number: Option<u32>) -> Result<()> {
        let ts = self.timestamp.to_rfc3339();

        // 1. shared SCHEMA_FIELD macros
        let sdk_dir = self.out_dir.join("sdk");
        fs::create_dir_all(&sdk_dir)?;
        fs::write(sdk_dir.join("cs2sdk_macros.hpp"), sdk_classes::render_macros_header())?;

        // 2. per-module SDK class headers
        let mut module_stems = Vec::new();
        for (file_name, body) in sdk_classes::render_module_headers(&self.result.schemas, build_number, &ts) {
            fs::write(sdk_dir.join(&file_name), body)?;
            if let Some(stem) = file_name.strip_suffix(".hpp") {
                module_stems.push(stem.to_string());
            }
        }

        // 3. netvars (split from schema)
        let nvs = netvars::extract(&self.result.schemas);
        fs::write(self.out_dir.join("netvars.json"), netvars::render_json(&nvs))?;
        fs::write(self.out_dir.join("netvars.hpp"), netvars::render_hpp(&nvs, build_number))?;
        fs::write(self.out_dir.join("netvars.cs"), netvars::render_cs(&nvs, build_number))?;

        // 4. interface accessor stubs
        fs::write(
            self.out_dir.join("interfaces_sdk.hpp"),
            interfaces_sdk::render_hpp(&self.result.interfaces, build_number),
        )?;
        fs::write(
            self.out_dir.join("interfaces_sdk.cs"),
            interfaces_sdk::render_cs(&self.result.interfaces, build_number),
        )?;

        // 5. amalgamations
        fs::write(
            self.out_dir.join("cs2sdk.hpp"),
            amalgamation::render_hpp(&module_stems, build_number),
        )?;
        fs::write(
            self.out_dir.join("cs2sdk.rs"),
            amalgamation::render_rs(true, build_number),
        )?;

        // 6. hand-curated "verified working features" catalogue.
        // Lives next to the auto-generated outputs so cheat developers
        // can copy a single file (.md / .hpp / .json) and know which
        // offsets are confirmed working in a live internal cheat plus
        // the gotchas (skybox tint moved to +0xE8, mat_fullbright needs
        // both ConVar slots written, etc).
        fs::write(
            self.out_dir.join("verified_features.json"),
            verified::render_json(build_number),
        )?;
        fs::write(
            self.out_dir.join("verified_features.md"),
            verified::render_md(build_number),
        )?;
        fs::write(
            self.out_dir.join("verified_features.hpp"),
            verified::render_hpp(build_number),
        )?;

        Ok(())
    }

    pub fn dump_all<P: MemoryView + Process>(&self, process: &mut P) -> Result<()> {
        let items = [
            ("buttons", Item::Buttons(&self.result.buttons)),
            ("interfaces", Item::Interfaces(&self.result.interfaces)),
            ("offsets", Item::Offsets(&self.result.offsets)),
            ("signatures", Item::Skinchanger(SkinchangerOutput(&self.result.skinchanger))),
        ];

        for (file_name, item) in &items {
            self.dump_item(file_name, item)?;
        }

        self.dump_schemas()?;
        self.dump_info(process)?;

        Ok(())
    }

    fn dump_info<P: MemoryView + Process>(&self, process: &mut P) -> Result<()> {
        let file_path = self.out_dir.join("info.json");

        let build_number = self
            .result
            .offsets
            .iter()
            .find_map(|(module_name, offsets)| {
                let module = process.module_by_name(module_name).ok()?;
                let offset = offsets.iter().find(|(name, _)| *name == "dwBuildNumber")?.1;

                process.read::<u32>(module.base + offset).data_part().ok()
            })
            .ok_or(anyhow!("failed to read build number"))?;

        let content = serde_json::to_string_pretty(&json!({
            "timestamp": self.timestamp.to_rfc3339(),
            "build_number": build_number,
        }))?;

        fs::write(&file_path, &content)?;

        Ok(())
    }

    fn dump_item(&self, file_name: &str, item: &Item) -> Result<()> {
        for file_type in self.file_types {
            let mut out = String::new();
            let mut fmt = Formatter::new(&mut out, self.indent_size);

            if file_type != "json" {
                self.write_banner(&mut fmt)?;
            }

            item.write(&mut fmt, file_type)?;

            let file_path = self.out_dir.join(format!("{}.{}", file_name, file_type));

            fs::write(&file_path, out)?;
        }

        Ok(())
    }

    fn dump_schemas(&self) -> Result<()> {
        for (module_name, (classes, enums)) in &self.result.schemas {
            let map = SchemaMap::from([(module_name.clone(), (classes.clone(), enums.clone()))]);

            self.dump_item(&slugify(&module_name), &Item::Schemas(&map))?;
        }

        Ok(())
    }

    fn write_banner(&self, fmt: &mut Formatter<'_>) -> Result<()> {
        writeln!(fmt, "// Generated using https://github.com/a2x/cs2-dumper")?;
        writeln!(fmt, "// {}\n", self.timestamp)?;

        Ok(())
    }
}

#[inline]
fn slugify(input: &str) -> String {
    input.replace(|c: char| !c.is_alphanumeric(), "_")
}

#[inline]
fn zig_ident(input: &str) -> String {
    if is_zig_identifier(input) && !is_zig_keyword(input) {
        input.to_string()
    } else {
        let escaped = input.replace('\\', "\\\\").replace('"', "\\\"");

        format!("@\"{}\"", escaped)
    }
}

#[inline]
fn is_zig_identifier(input: &str) -> bool {
    let mut chars = input.chars();

    match chars.next() {
        Some(c) if c == '_' || c.is_ascii_alphabetic() => {}
        _ => return false,
    }

    chars.all(|c| c == '_' || c.is_ascii_alphanumeric())
}

#[inline]
fn is_zig_keyword(input: &str) -> bool {
    matches!(
        input,
        "addrspace"
            | "align"
            | "allowzero"
            | "and"
            | "anyframe"
            | "anytype"
            | "asm"
            | "async"
            | "await"
            | "break"
            | "callconv"
            | "catch"
            | "comptime"
            | "const"
            | "continue"
            | "defer"
            | "else"
            | "enum"
            | "errdefer"
            | "error"
            | "export"
            | "extern"
            | "false"
            | "fn"
            | "for"
            | "if"
            | "inline"
            | "linksection"
            | "noalias"
            | "noinline"
            | "nosuspend"
            | "null"
            | "opaque"
            | "or"
            | "orelse"
            | "packed"
            | "pub"
            | "resume"
            | "return"
            | "struct"
            | "suspend"
            | "switch"
            | "test"
            | "threadlocal"
            | "true"
            | "try"
            | "union"
            | "unreachable"
            | "usingnamespace"
            | "var"
            | "volatile"
            | "while"
    )
}
