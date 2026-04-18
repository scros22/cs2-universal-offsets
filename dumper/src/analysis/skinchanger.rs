use std::collections::BTreeMap;

use anyhow::Result;
use log::{error, info};
use memflow::prelude::v1::*;
use pelite::pattern;
use pelite::pattern::{Atom, save_len};
use pelite::pe64::{Pe, PeView, Rva};
use phf::{Map, phf_map};

pub type SkinchangerMap = BTreeMap<String, BTreeMap<String, Rva>>;

macro_rules! skinchanger_patterns {
    ($($module:ident => {
        $($name:expr => $pattern:expr),+ $(,)?
    }),+ $(,)?) => {
        $(
            mod $module {
                use super::*;

                pub(super) const PATTERNS: Map<&'static str, &'static [Atom]> = phf_map! {
                    $($name => $pattern),+
                };

                pub fn scan_patterns(view: PeView<'_>) -> BTreeMap<String, Rva> {
                    let mut map = BTreeMap::new();

                    for (&name, &pattern) in &PATTERNS {
                        let mut save = vec![0; save_len(pattern)];

                        if view.scanner().finds_code(pattern, &mut save) {
                            let rva = save[0]; // Use first match address
                            map.insert(name.to_string(), rva);
                            info!("found {}: 0x{:X}", name, rva);
                        } else {
                            error!("pattern not found: {}", name);
                        }
                    }

                    map
                }
            }
        )+
    };
}

skinchanger_patterns! {
    client => {
        // Core inventory functions for proper skin/knife/glove changing
        "EquipItemInLoadout" => pattern!("48895c24u1 48896c24u1 48897424u1 895424u1 57 4154 4155 4156 4157 4883ecu1 0fb7fa"),
        "GetItemInLoadout" => pattern!("4055 4883ecu1 4963e8"),
        "SetBodyGroup" => pattern!("85d2 0f88u4 5557"),
        "SetModel" => pattern!("4053 4883ecu1 488bd9 4c8bc2 488b0du4 488d542440"),
        "SetMeshGroupMask" => pattern!("48895c24u1 48897424u1 57 4883ecu1 488d99u4 488b71"),
        
        // Network update functions
        "ForceFullUpdate" => pattern!("488b05u4 834800ff"),
        
        // World effects for night vision / dark mode
        "TonemapController" => pattern!("488bc4 48895800 48896800 48897000 57 4154 4155 4156 4157 4881ecu4"),
        "FogController" => pattern!("48895c24u1 48896c24u1 48897424u1 57 4883ecu1 488bf9 488bea"),
        "PostProcessController" => pattern!("4055 53 56 57 4154 4155 4156 4157 488dac24"),
        
        // Additional skinchanger functions from recent signatures
        "CreateNewPaintKit" => pattern!("48895c2410 56 4883ec20 488b01 ff5010 488b1du4"),
        "RegenerateWeaponSkin" => pattern!("4055 53 4157 488dac24u4 4881ecu4 440fb6fa 488bd9 bau4 488d0du4 e8u4"),
        "UpdateCompositeMaterial" => pattern!("48895c2410 48896c2418 48897424u1 57 4156 4157 4883ec20 440fb6f2 488bf1 e8"),
        "GetInventoryManager" => pattern!("e8u4 488bd3 488bc8 4c8b00 41ff900002"),
        "CreateSOSubclassEconItem" => pattern!("4883ec28 b948000000 e8u4 4885"),
        "SetDynamicAttributeValue" => pattern!("e9u4 cccccccccccccccccccccccccccccccc 498bc0 488bca 488bd0"),
    },
    
    materialsystem2 => {
        // Material system for advanced chams
        "CreateMaterial" => pattern!("48895c24u1 48896c24u1 56 57 4156 4881ecu4 488b05"),
        "PrepareSceneMaterial" => pattern!("48895c2408 48897424u1 57 4883ec30 488b59u1 488bf2 486379u1 48c1e706"),
        "FindParameter" => pattern!("48895c24u1 48897424u1 57 4883ec20 488b5920 48"),
        "UpdateParameter" => pattern!("48897c24u1 4156 4883ecu1 8b81"),
    },
}

/// Enhanced skinchanger pattern analysis
/// Automatically finds function signatures needed for proper skin/knife/glove changing
/// and advanced world effects like night vision mode
pub fn skinchanger<P: Process + MemoryView>(process: &mut P) -> Result<SkinchangerMap> {
    let mut map = BTreeMap::new();

    // Scan client.dll patterns
    if let Ok(client_module) = process.module_by_name("client.dll") {
        let client_data = process.read_raw(client_module.base, client_module.size as usize)?;
        let client_pe = PeView::from_bytes(&client_data)?;
        
        info!("scanning client.dll for skinchanger patterns");
        let client_results = client::scan_patterns(client_pe);
        
        if !client_results.is_empty() {
            map.insert("client.dll".to_string(), client_results);
        }
    }

    // Scan materialsystem2.dll patterns
    if let Ok(material_module) = process.module_by_name("materialsystem2.dll") {
        let material_data = process.read_raw(material_module.base, material_module.size as usize)?;
        let material_pe = PeView::from_bytes(&material_data)?;
        
        info!("scanning materialsystem2.dll for material patterns");
        let material_results = materialsystem2::scan_patterns(material_pe);
        
        if !material_results.is_empty() {
            map.insert("materialsystem2.dll".to_string(), material_results);
        }
    }

    // Summary
    let total_patterns: usize = map.values().map(|m| m.len()).sum();
    info!("skinchanger analysis complete: found {} patterns across {} modules", 
          total_patterns, map.len());

    Ok(map)
}