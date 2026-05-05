use std::fmt::{self, Write};
use std::fs;
use std::path::Path;

use anyhow::Result;

use chrono::{DateTime, Utc};

use memflow::prelude::v1::*;

use formatter::Formatter;

use crate::analysis::*;

mod buttons;
mod formatter;

// SDK-style emitters — every disk output the dumper produces is now
// expressed in typed, cheat-developer-friendly form.
pub mod amalgamation;
pub mod ident;
pub mod interfaces_sdk;
pub mod netvars;
pub mod sdk_classes;
pub mod verified;
pub mod vtables;

enum Item<'a> {
    Buttons(&'a ButtonMap),
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
        }
    }

    fn write_hpp(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_hpp(fmt),
        }
    }

    fn write_json(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_json(fmt),
        }
    }

    fn write_rs(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_rs(fmt),
        }
    }

    fn write_zig(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Item::Buttons(buttons) => buttons.write_zig(fmt),
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

    /// Emit the cheat-developer-friendly SDK files into `self.out_dir`
    /// (which is the session's `sdk/` directory):
    ///
    /// * `cs2sdk_macros.hpp`         — SCHEMA_FIELD macros
    /// * `<module>.hpp`              — typed schema classes
    /// * `netvars.{json,hpp,cs}`     — split networked fields
    /// * `interfaces_sdk.{hpp,cs}`   — typed accessor stubs
    /// * `cs2sdk.hpp`                — single-include amalgamation
    /// * `cs2sdk.rs`                 — Rust amalgamation module
    /// * `verified_features.{json,md,hpp}` — verified-working catalogue
    ///
    /// `build_number` is pinned into every emitted file as `CS2_BUILD`
    /// so internal cheats can `static_assert` against the running game.
    pub fn dump_sdk_extras(&self, build_number: Option<u32>) -> Result<()> {
        let ts = self.timestamp.to_rfc3339();

        // 1. shared SCHEMA_FIELD macros
        fs::write(self.out_dir.join("cs2sdk_macros.hpp"), sdk_classes::render_macros_header())?;

        // 2. per-module SDK class headers
        let mut module_stems = Vec::new();
        for (file_name, body) in sdk_classes::render_module_headers(&self.result.schemas, build_number, &ts) {
            fs::write(self.out_dir.join(&file_name), body)?;
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

    pub fn dump_all<P: MemoryView + Process>(&self, _process: &mut P) -> Result<()> {
        // The legacy `a2x/cs2-dumper`-style raw flat-offset emitters
        // (`offsets.{hpp,cs,...}`, `interfaces.{hpp,cs,...}`,
        // `<schema>.{hpp,cs,...}`, the skinchanger pattern dump) have
        // been retired — every value they exposed is now reachable in a
        // typed form through the SDK headers (`sdk_classes`, netvars
        // split-out, interface accessor stubs, `cs2sdk.hpp`
        // amalgamation), and the per-file pattern dump is superseded by
        // the dedicated `signatures/` directory.
        //
        // We still emit the symbolic button table since there's no
        // equivalent in the typed SDK yet.
        self.dump_item("buttons", &Item::Buttons(&self.result.buttons))?;

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

    fn write_banner(&self, fmt: &mut Formatter<'_>) -> Result<()> {
        writeln!(fmt, "// CS2 SDK — generated by cs2-universal-dumper v{}", env!("CARGO_PKG_VERSION"))?;
        writeln!(fmt, "// https://github.com/Lucid-cs2/universal-dumper")?;
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
