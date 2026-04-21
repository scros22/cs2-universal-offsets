use std::collections::BTreeMap;

use anyhow::Result;
use log::{error, info, warn};
use memflow::prelude::v1::*;

mod buttons;
mod interfaces;
mod offsets;
mod schemas;
mod skinchanger;
mod signature_research;
mod signature_verification;

pub use buttons::*;
pub use interfaces::*;
pub use offsets::*;
pub use schemas::*;
pub use skinchanger::*;
pub use signature_research::*;
pub use signature_verification::*;

use std::any::type_name;

#[derive(Debug)]
pub struct AnalysisResult {
    pub buttons: ButtonMap,
    pub interfaces: InterfaceMap,
    pub offsets: OffsetMap,
    pub schemas: SchemaMap,
    pub skinchanger: SkinchangerMap,
}

pub fn analyze_all<P: Process + MemoryView>(process: &mut P) -> Result<AnalysisResult> {
    let buttons = analyze(process, buttons);

    info!("found {} buttons", buttons.len());

    let interfaces = analyze(process, interfaces);

    info!(
        "found {} interfaces across {} modules",
        interfaces
            .iter()
            .map(|(_, ifaces)| ifaces.len())
            .sum::<usize>(),
        interfaces.len()
    );

    let offsets = analyze(process, offsets);

    info!(
        "found {} offsets across {} modules",
        offsets
            .iter()
            .map(|(_, offsets)| offsets.len())
            .sum::<usize>(),
        offsets.len()
    );

    let schemas = analyze(process, schemas);

    let (class_count, enum_count) =
        schemas
            .values()
            .fold((0, 0), |(classes, enums), (class_vec, enum_vec)| {
                (classes + class_vec.len(), enums + enum_vec.len())
            });

    info!(
        "found {} classes and {} enums across {} modules",
        class_count,
        enum_count,
        schemas.len()
    );

    // Enhanced signature analysis
    info!("=== Starting Advanced Signature Research ===");
    
    let skinchanger = analyze(process, skinchanger);
    info!(
        "found {} skinchanger patterns across {} modules",
        skinchanger
            .iter()
            .map(|(_, patterns)| patterns.len())
            .sum::<usize>(),
        skinchanger.len()
    );

    // Run advanced signature research
    match signature_research::research_signatures(process) {
        Ok(research_results) => {
            let research_count: usize = research_results
                .iter()
                .map(|(_, sigs)| sigs.len())
                .sum();
            
            info!(
                "found {} research-based signatures across {} modules",
                research_count,
                research_results.len()
            );
            
            // Log detailed results
            for (module, signatures) in &research_results {
                info!("=== {} Research Results ===", module);
                for (name, result) in signatures {
                    info!("  {} -> 0x{:X} (confidence: {:.1}%, method: {})", 
                          name, result.rva, result.confidence * 100.0, result.method);
                }
            }

            // Run verification on research results
            info!("=== Starting Signature Verification ===");
            match signature_verification::verify_signatures(process, &research_results) {
                Ok(verification_results) => {
                    let verified_count: usize = verification_results
                        .values()
                        .map(|module| module.values().filter(|v| v.is_valid).count())
                        .sum();
                    let total_count: usize = verification_results
                        .values()
                        .map(|module| module.len())
                        .sum();
                    
                    info!("Verification complete: {}/{} signatures verified ({:.1}% success rate)",
                          verified_count, total_count, 
                          if total_count > 0 { verified_count as f32 / total_count as f32 * 100.0 } else { 0.0 });
                    
                    // Log verification details
                    for (module, verifications) in &verification_results {
                        info!("=== {} Verification Results ===", module);
                        for (name, verification) in verifications {
                            let status = if verification.is_valid { "✓ VERIFIED" } else { "✗ FAILED" };
                            info!("  {} {} (confidence: {:.1}%)", 
                                  name, status, verification.confidence * 100.0);
                        }
                    }
                }
                Err(e) => {
                    warn!("Signature verification failed: {}", e);
                }
            }
        }
        Err(e) => {
            error!("Advanced signature research failed: {}", e);
        }
    }

    Ok(AnalysisResult {
        buttons,
        interfaces,
        offsets,
        schemas,
        skinchanger,
    })
}

fn analyze<P, F, T>(process: &mut P, f: F) -> T
where
    P: Process + MemoryView,
    F: FnOnce(&mut P) -> Result<T>,
    T: Default,
{
    let name = type_name::<F>();

    match f(process) {
        Ok(result) => result,
        Err(err) => {
            error!("failed to read {}: {}", name, err);

            T::default()
        }
    }
}
