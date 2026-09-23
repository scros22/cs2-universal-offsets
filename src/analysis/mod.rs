//! Offset / interface / schema / button extraction pipeline.
//!
//! Ports the pelite-based scanners from [a2x's `cs2-dumper`] and exposes a
//! single [`analyze_all`] entry point that the binary's main loop calls.
//! Errors in any individual scanner are downgraded to a warning so a
//! partial run still produces a useful set of files.
//!
//! [a2x's `cs2-dumper`]: https://github.com/a2x/cs2-dumper

use std::any::type_name;

use anyhow::Result;
use log::{error, info};
use memflow::prelude::v1::*;

mod buttons;
pub mod convars;
pub mod entities;
pub mod gameevents;
mod interfaces;
pub mod manual_iface;
mod offsets;
mod protobufs;
pub mod rtti;
pub mod schema_lookup;
mod schemas;
mod vtables;
pub mod weapons;

pub use buttons::*;
pub use interfaces::*;
pub use offsets::*;
pub use protobufs::*;
pub use schemas::*;
pub use vtables::*;

/// Aggregated output of every analysis stage.
#[derive(Debug)]
pub struct AnalysisResult {
    pub buttons: ButtonMap,
    pub interfaces: InterfaceMap,
    pub offsets: OffsetMap,
    pub schemas: SchemaMap,
    pub vtables: VTableMap,
}

/// Run every static analyser against the live process.
///
/// Each stage is independent — a failure in one (e.g. a missing module) is
/// logged and replaced with that stage's [`Default`] value so subsequent
/// stages can still complete.
pub fn analyze_all<P: Process + MemoryView>(process: &mut P) -> Result<AnalysisResult> {
    let buttons = analyze(process, buttons);
    info!("found {} buttons", buttons.len());

    let interfaces = analyze(process, interfaces);
    info!(
        "found {} interfaces across {} modules",
        interfaces.iter().map(|(_, ifaces)| ifaces.len()).sum::<usize>(),
        interfaces.len(),
    );

    let offsets = analyze(process, offsets);
    info!(
        "found {} offsets across {} modules",
        offsets.iter().map(|(_, offsets)| offsets.len()).sum::<usize>(),
        offsets.len(),
    );

    let schemas = analyze(process, schemas);
    let (class_count, enum_count) = schemas
        .values()
        .fold((0, 0), |(c, e), (cv, ev)| (c + cv.len(), e + ev.len()));
    info!(
        "found {} classes and {} enums across {} modules",
        class_count,
        enum_count,
        schemas.len(),
    );

    // NOTE: the protobuf pass (`analysis::protobufs`) is intentionally NOT run
    // here. It `read_raw`s several large modules in full, which degrades memflow's
    // subsequent reads and makes the later Pattern pass miss tail patterns.
    // main.rs runs it as its own stage AFTER patterns so sigs read clean.

    // VTable walk depends on the resolved interface table; run it
    // inline rather than through `analyze` so we can pass `&interfaces`.
    let vtables = match vtables::vtables(process, &interfaces) {
        Ok(v) => {
            let total: usize = v.values().map(|m| m.len()).sum();
            let methods: usize = v.values().flat_map(|m| m.values()).map(|i| i.methods.len()).sum();
            let rtti: usize = v
                .values()
                .flat_map(|m| m.values())
                .filter(|i| i.rtti_class.is_some())
                .count();
            info!(
                "dumped {} interface vtables ({} method slots, {} class names recovered via RTTI) across {} modules",
                total, methods, rtti, v.len()
            );
            v
        }
        Err(err) => {
            error!("vtable walk failed: {}", err);
            Default::default()
        }
    };

    Ok(AnalysisResult {
        buttons,
        interfaces,
        offsets,
        schemas,
        vtables,
    })
}

/// Run a single analyser and convert any failure into the type's default.
fn analyze<P, F, T>(process: &mut P, f: F) -> T
where
    P: Process + MemoryView,
    F: FnOnce(&mut P) -> Result<T>,
    T: Default,
{
    let name = type_name::<F>();
    match f(process) {
        Ok(v) => v,
        Err(err) => {
            error!("failed to read {name}: {err}");
            T::default()
        }
    }
}
