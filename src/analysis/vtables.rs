//! Per-interface vtable dumper.
//!
//! For every entry in [`InterfaceMap`] we follow the instance pointer
//! to the C++ vftable (the first qword of any polymorphic object) and
//! walk it as a contiguous array of function pointers, stopping at the
//! first entry that doesn't look like a code address.
//!
//! # What this lets internals do
//!
//! With `inline constexpr std::ptrdiff_t Connect = 0;` etc. emitted per
//! interface, hooking by index becomes one line:
//!
//! ```cpp
//! using Connect_t = bool(__thiscall*)(void* self, ...);
//! auto* iface = cs2::ifaces::client_dll::Source2Client002(client_base);
//! auto** vt = *reinterpret_cast<void***>(iface);
//! auto fn  = reinterpret_cast<Connect_t>(vt[cs2::vtables::client_dll::Source2Client002::Connect]);
//! ```
//!
//! # Method-name recovery
//!
//! We don't have PDBs, so most slots are emitted as `method_<N>`. As a
//! bonus pass, callers can cross-reference each method RVA against the
//! signature database — if a hit's resolved RVA matches a method RVA,
//! the signature name is used in place of `method_<N>` (handled in the
//! writer, not here, so this analyzer stays pure).

use std::collections::BTreeMap;

use anyhow::Result;
use log::debug;
use memflow::prelude::v1::*;

use super::InterfaceMap;

/// Dump of one interface's virtual function table.
#[derive(Debug, Clone, Default)]
pub struct VTableInfo {
    /// RVA of the vftable itself within the *owning* module (may differ
    /// from the interface instance's module if the vftable was emitted
    /// into a different translation unit).
    pub vtable_rva: u64,
    /// Module name that hosts the vftable bytes.
    pub vtable_module: String,
    /// One entry per virtual method, in slot order. `module` is the DLL
    /// that hosts the method body; `rva` is its offset within that DLL.
    pub methods: Vec<VTableMethod>,
}

#[derive(Debug, Clone)]
pub struct VTableMethod {
    pub module: String,
    pub rva: u64,
}

/// `module → interface_name → vtable_info`
pub type VTableMap = BTreeMap<String, BTreeMap<String, VTableInfo>>;

/// Maximum vtable slots to read per interface. CS2's biggest interface
/// vtables sit comfortably under 200; 256 gives plenty of headroom and
/// caps memory in the worst case.
const MAX_METHODS: usize = 256;

/// Walk every interface's vtable. Failures on individual interfaces are
/// logged and skipped — one bad pointer doesn't take the whole pass
/// down.
pub fn vtables<P: Process + MemoryView>(
    process: &mut P,
    interfaces: &InterfaceMap,
) -> Result<VTableMap> {
    // Build a `[module_base, module_size, name]` table once so we can
    // classify any address into "(module, rva)" without re-walking the
    // module list per method.
    let modules: Vec<(String, u64, u64)> = process
        .module_list()?
        .into_iter()
        .map(|m| (m.name.to_string(), m.base.to_umem() as u64, m.size as u64))
        .collect();

    let mut out: VTableMap = BTreeMap::new();

    for (module_name, ifaces) in interfaces {
        let Some(host) = modules.iter().find(|(n, _, _)| n == module_name) else {
            continue;
        };

        let mut by_iface: BTreeMap<String, VTableInfo> = BTreeMap::new();

        for (iface_name, iface_rva) in ifaces {
            let inst_va = host.1 + *iface_rva as u64;
            match dump_one(process, inst_va, &modules) {
                Ok(Some(info)) => {
                    debug!(
                        "{}::{} vtable @ {}+{:#X} ({} methods)",
                        module_name,
                        iface_name,
                        info.vtable_module,
                        info.vtable_rva,
                        info.methods.len()
                    );
                    by_iface.insert(iface_name.clone(), info);
                }
                Ok(None) => {} // not a polymorphic instance — skip silently
                Err(e) => {
                    debug!("{}::{} vtable walk failed: {}", module_name, iface_name, e);
                }
            }
        }

        if !by_iface.is_empty() {
            out.insert(module_name.clone(), by_iface);
        }
    }

    Ok(out)
}

fn dump_one<P: MemoryView>(
    process: &mut P,
    instance_va: u64,
    modules: &[(String, u64, u64)],
) -> Result<Option<VTableInfo>> {
    // [instance][0] = vftable VA
    let vt_va = process
        .read::<u64>(Address::from(instance_va))
        .data_part()?;
    let Some((vt_mod, vt_rva)) = classify(vt_va, modules) else {
        return Ok(None);
    };

    // Slurp up to MAX_METHODS slots in one shot for speed; truncate at
    // the first non-code pointer.
    let raw = process
        .read_raw(Address::from(vt_va), MAX_METHODS * 8)
        .data_part()?;

    let mut methods = Vec::with_capacity(MAX_METHODS / 4);
    for chunk in raw.chunks_exact(8) {
        let p = u64::from_le_bytes(chunk.try_into().unwrap());
        match classify(p, modules) {
            Some((m, rva)) => methods.push(VTableMethod { module: m, rva }),
            None => break,
        }
    }

    if methods.is_empty() {
        return Ok(None);
    }

    Ok(Some(VTableInfo {
        vtable_rva: vt_rva,
        vtable_module: vt_mod,
        methods,
    }))
}

/// Map a VA back to `(module_name, rva)` if it falls inside any loaded
/// module's image range. We don't bother gating on per-section bounds —
/// a vftable entry could legally point at a thunk in `.text`,
/// `.text$mn`, or `__icall_thunks`, and we'd rather over-accept than
/// truncate a real vtable on a stylistic edge case.
fn classify(va: u64, modules: &[(String, u64, u64)]) -> Option<(String, u64)> {
    if va < 0x10000 {
        return None; // null + low-canonical garbage
    }
    for (name, base, size) in modules {
        if va >= *base && va < base.wrapping_add(*size) {
            return Some((name.clone(), va - *base));
        }
    }
    None
}
