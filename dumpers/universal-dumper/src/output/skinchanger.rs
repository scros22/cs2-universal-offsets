use std::fmt::{self, Write};

use heck::{AsPascalCase, AsSnakeCase};

use crate::analysis::SkinchangerMap;
use crate::output::{Formatter, slugify, zig_ident};

// Create a newtype to avoid conflicting with OffsetMap
pub struct SkinchangerOutput<'a>(pub &'a SkinchangerMap);

impl<'a> SkinchangerOutput<'a> {
    pub fn write_cs(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        fmt.block("namespace CS2Dumper.Signatures", false, |fmt| {
            for (module_name, patterns) in self.0 {
                writeln!(fmt, "// Module: {}", module_name)?;
                
                fmt.block(
                    &format!("public static class {}", AsPascalCase(slugify(module_name))),
                    false,
                    |fmt| {
                        for (name, address) in patterns {
                            writeln!(fmt, "public const nint {} = 0x{:X};", name, address)?;
                        }
                        Ok(())
                    },
                )?;
            }
            Ok(())
        })
    }

    pub fn write_hpp(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        writeln!(fmt, "#pragma once\n")?;
        writeln!(fmt, "#include <cstddef>")?;
        writeln!(fmt, "#include <cstdint>\n")?;

        fmt.block("namespace cs2_dumper", false, |fmt| {
            fmt.block("namespace signatures", false, |fmt| {
                for (module_name, patterns) in self.0 {
                    writeln!(fmt, "// Module: {}", module_name)?;

                    fmt.block(
                        &format!("namespace {}", AsSnakeCase(slugify(module_name))),
                        false,
                        |fmt| {
                            for (name, address) in patterns {
                                writeln!(fmt, "constexpr std::ptrdiff_t {} = 0x{:X};", name, address)?;
                            }
                            Ok(())
                        },
                    )?;
                }
                Ok(())
            })
        })
    }

    pub fn write_json(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        writeln!(fmt, "{{")?;
        
        fmt.indent(|fmt| {
            let mut module_iter = self.0.iter().peekable();
            while let Some((module_name, patterns)) = module_iter.next() {
                writeln!(fmt, "\"{}\": {{", module_name)?;
                
                fmt.indent(|fmt| {
                    let mut pattern_iter = patterns.iter().peekable();
                    while let Some((name, address)) = pattern_iter.next() {
                        let comma = if pattern_iter.peek().is_some() { "," } else { "" };
                        writeln!(fmt, "\"{}\": \"0x{:X}\"{}", name, address, comma)?;
                    }
                    Ok(())
                })?;

                let comma = if module_iter.peek().is_some() { "," } else { "" };
                writeln!(fmt, "}}{}", comma)?;
            }
            Ok(())
        })?;

        writeln!(fmt, "}}")?;
        Ok(())
    }

    pub fn write_rs(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        fmt.block("pub mod signatures", false, |fmt| {
            for (module_name, patterns) in self.0 {
                writeln!(fmt, "// Module: {}", module_name)?;
                
                fmt.block(
                    &format!("pub mod {}", AsSnakeCase(slugify(module_name))),
                    false,
                    |fmt| {
                        for (name, address) in patterns {
                            writeln!(fmt, "pub const {}: usize = 0x{:X};", name.to_uppercase(), address)?;
                        }
                        Ok(())
                    },
                )?;
            }
            Ok(())
        })
    }

    pub fn write_zig(&self, fmt: &mut Formatter<'_>) -> fmt::Result {
        fmt.block("pub const signatures = struct", true, |fmt| {
            for (module_name, patterns) in self.0 {
                writeln!(fmt, "// Module: {}", module_name)?;
                
                fmt.block(
                    &format!("pub const {} = struct", zig_ident(&AsSnakeCase(slugify(module_name)).to_string())),
                    true,
                    |fmt| {
                        for (name, address) in patterns {
                            writeln!(fmt, "pub const {}: usize = 0x{:X};", zig_ident(name), address)?;
                        }
                        Ok(())
                    },
                )?;
            }
            Ok(())
        })
    }
}