//! MSVC RTTI class-name recovery for C++ vftables.
//!
//! Every polymorphic class compiled by MSVC carries a back-pointer to a
//! `RTTICompleteObjectLocator` (COL) at slot `-1` of its virtual
//! function table.  From the COL we can reach a `TypeDescriptor` that
//! holds the mangled class name (e.g. `.?AVCCSPlayerPawn@@`), which we
//! demangle to `CCSPlayerPawn`.
//!
//! ## Layouts (x64, MSVC)
//!
//! ```text
//! qword @ vftable - 8                    -> RTTICompleteObjectLocator*
//!
//! struct RTTICompleteObjectLocator {     // 24 bytes total, x64
//!     uint32_t signature;                // 1 on x64
//!     uint32_t offset;                   // offset of vftable in the complete object
//!     uint32_t cd_offset;                // ctor displacement offset
//!     uint32_t type_descriptor_rva;      // -> TypeDescriptor (module-relative)
//!     uint32_t class_descriptor_rva;     // -> ClassHierarchyDescriptor (unused here)
//!     uint32_t self_rva;                 // -> this COL itself (image-base recovery)
//! };
//!
//! struct TypeDescriptor {                // variable size (name is C-string)
//!     void*    type_info_vftable;        // +0x00
//!     void*    spare;                    // +0x08
//!     char     mangled_name[];           // +0x10  e.g. ".?AVCCSPlayerPawn@@"
//! };
//! ```
//!
//! ## Demangling
//!
//! MSVC mangled type names are ASCII strings of the form:
//!
//! ```text
//! .?A<kind><scope0>@<scope1>@...@@
//!     ^ kind: 'V' = class, 'U' = struct, 'W' = enum
//! ```
//!
//! Scopes are reversed (innermost first), so `.?AVfoo@bar@@` is
//! `bar::foo`.  Templates (`?$Name@<args>@`) are not unmangled — we
//! return the raw stripped form, which is still useful as a stable
//! identifier across patches.

use anyhow::{Result, anyhow};
use memflow::prelude::v1::*;

/// Maximum bytes to read for a single mangled name C-string.  The
/// longest names in CS2 sit comfortably under 256.
const MAX_NAME_BYTES: usize = 512;

/// Resolve the C++ class name for a vftable located at `vftable_va`,
/// hosted by the module that starts at `module_base` and is
/// `module_size` bytes long.
///
/// Returns `None` (rather than `Err`) for vtables that don't carry RTTI
/// — this is normal for compiler-generated thunks and for builds
/// produced with `/GR-`.
pub fn resolve_class_name<P: MemoryView>(
    process: &mut P,
    vftable_va: u64,
    module_base: u64,
    module_size: u64,
) -> Option<String> {
    try_resolve(process, vftable_va, module_base, module_size).ok()
}

fn try_resolve<P: MemoryView>(
    process: &mut P,
    vftable_va: u64,
    module_base: u64,
    module_size: u64,
) -> Result<String> {
    if vftable_va < 8 + module_base {
        return Err(anyhow!("vftable too low for COL slot"));
    }
    // [vftable - 8] = COL pointer
    let col_va = process
        .read::<u64>(Address::from(vftable_va - 8))
        .data_part()?;

    // COL must live inside the same module image.
    if col_va < module_base || col_va >= module_base + module_size {
        return Err(anyhow!("COL pointer outside module"));
    }

    // Read the 24-byte COL struct in one shot.
    let col_bytes = process
        .read_raw(Address::from(col_va), 24)
        .data_part()?;
    if col_bytes.len() < 24 {
        return Err(anyhow!("short COL read"));
    }

    let signature = u32::from_le_bytes(col_bytes[0..4].try_into().unwrap());
    let type_desc_rva = u32::from_le_bytes(col_bytes[12..16].try_into().unwrap());
    let self_rva = u32::from_le_bytes(col_bytes[20..24].try_into().unwrap());

    // x64 COL signature is always 1.
    if signature != 1 {
        return Err(anyhow!("bad COL signature {:#x}", signature));
    }
    // Self-RVA must point back to the COL we just read — this is what
    // lets MSVC reconstruct the image base at runtime, and is a cheap
    // sanity check that we're looking at a real COL.
    if (col_va - module_base) as u32 != self_rva {
        return Err(anyhow!("COL self-RVA mismatch"));
    }
    if (type_desc_rva as u64) >= module_size {
        return Err(anyhow!("TypeDescriptor RVA out of range"));
    }

    let td_va = module_base + type_desc_rva as u64;
    // The mangled name starts at TypeDescriptor + 0x10.
    let name_va = td_va + 0x10;

    let raw = process
        .read_raw(Address::from(name_va), MAX_NAME_BYTES)
        .data_part()?;
    let nul = raw.iter().position(|&b| b == 0).unwrap_or(raw.len());
    let mangled = std::str::from_utf8(&raw[..nul])
        .map_err(|_| anyhow!("non-utf8 mangled name"))?;

    Ok(demangle(mangled))
}

/// Best-effort MSVC RTTI demangler.  Handles the common `.?AV…@@` /
/// `.?AU…@@` / `.?AW…@@` shapes; returns the raw input for anything
/// exotic (templates, function pointers in nested scopes, etc.).
fn demangle(mangled: &str) -> String {
    let Some(rest) = mangled.strip_prefix(".?A") else {
        return mangled.to_string();
    };
    if rest.len() < 2 {
        return mangled.to_string();
    }

    let kind = rest.as_bytes()[0];
    if !matches!(kind, b'V' | b'U' | b'W') {
        return mangled.to_string();
    }
    let body = &rest[1..];

    // Body is `scope0@scope1@…@@`.  Trailing `@@` separates the scope
    // list from CV-qualifiers (which we don't care about).
    let scope_part = body
        .strip_suffix("@@")
        .or_else(|| body.split_once("@@").map(|(s, _)| s))
        .unwrap_or(body);

    let segments: Vec<&str> = scope_part
        .split('@')
        .filter(|s| !s.is_empty())
        .collect();

    if segments.is_empty() {
        return mangled.to_string();
    }

    // If any segment is a template invocation (`?$Foo@…`), bail out and
    // return the raw form rather than producing something misleading.
    if segments.iter().any(|s| s.starts_with("?$")) {
        return mangled.to_string();
    }

    // Mangled scopes are stored innermost-first; C++ source order is
    // outermost-first.
    let mut joined = String::with_capacity(scope_part.len() + segments.len() * 2);
    for (i, seg) in segments.iter().rev().enumerate() {
        if i > 0 {
            joined.push_str("::");
        }
        joined.push_str(seg);
    }
    joined
}

#[cfg(test)]
mod tests {
    use super::demangle;

    #[test]
    fn class() {
        assert_eq!(demangle(".?AVCCSPlayerPawn@@"), "CCSPlayerPawn");
    }

    #[test]
    fn struct_kind() {
        assert_eq!(demangle(".?AUFooBar@@"), "FooBar");
    }

    #[test]
    fn nested() {
        assert_eq!(demangle(".?AVfoo@bar@baz@@"), "baz::bar::foo");
    }

    #[test]
    fn template_left_raw() {
        let m = ".?AV?$Foo@H@@";
        assert_eq!(demangle(m), m);
    }

    #[test]
    fn unknown_left_raw() {
        let m = "totally not RTTI";
        assert_eq!(demangle(m), m);
    }
}
