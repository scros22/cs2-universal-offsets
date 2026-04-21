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
mod interfaces;
mod offsets;
mod schemas;
mod skinchanger;

pub use buttons::*;
pub use interfaces::*;
pub use offsets::*;
pub use schemas::*;
pub use skinchanger::*;

/// Aggregated output of every analysis stage.
#[derive(Debug)]
pub struct AnalysisResult {
    pub buttons: ButtonMap,
    pub interfaces: InterfaceMap,
    pub offsets: OffsetMap,
    pub schemas: SchemaMap,
    pub skinchanger: SkinchangerMap,
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

    let skinchanger = analyze(process, skinchanger);
    info!(
        "found {} skinchanger patterns across {} modules",
        skinchanger.iter().map(|(_, p)| p.len()).sum::<usize>(),
        skinchanger.len(),
    );

    Ok(AnalysisResult {
        buttons,
        interfaces,
        offsets,
        schemas,
        skinchanger,
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
