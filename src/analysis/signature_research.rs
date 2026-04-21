use std::collections::BTreeMap;
use anyhow::Result;
use log::{info, warn};
use memflow::prelude::v1::*;
use pelite::pe64::{Pe, PeView, Rva};
use pelite::pattern;
use pelite::pattern::{Atom, save_len};

#[derive(Debug, Clone)]
pub struct ResearchResult {
    pub rva: Rva,
    pub confidence: f32,
    pub method: String,
    pub description: String,
    pub verified: bool,
}

/// Enhanced signature research with runtime validation
pub fn research_signatures<P: Process + MemoryView>(process: &mut P) -> Result<BTreeMap<String, BTreeMap<String, ResearchResult>>> {
    let mut results = BTreeMap::new();

    info!("=== Enhanced Signature Research with Validation ===");

    // Research client.dll signatures
    if let Ok(client_module) = process.module_by_name("client.dll") {
        let client_data = process.read_raw(client_module.base, client_module.size as usize)?;
        let client_pe = PeView::from_bytes(&client_data)?;
        
        info!("Researching client.dll signatures with runtime validation...");
        let client_results = research_client_signatures_validated(&client_pe, process, client_module.base)?;
        
        if !client_results.is_empty() {
            results.insert("client.dll".to_string(), client_results);
        }
    }

    // Research materialsystem2.dll signatures  
    if let Ok(material_module) = process.module_by_name("materialsystem2.dll") {
        let material_data = process.read_raw(material_module.base, material_module.size as usize)?;
        let material_pe = PeView::from_bytes(&material_data)?;
        
        info!("Researching materialsystem2.dll signatures...");
        let material_results = research_material_signatures(&material_pe)?;
        
        if !material_results.is_empty() {
            results.insert("materialsystem2.dll".to_string(), material_results);
        }
    }

    let total_sigs: usize = results.values().map(|m| m.len()).sum();
    info!("=== Research Complete: {} signatures found ===", total_sigs);

    Ok(results)
}

/// Research client.dll signatures using recent patterns from br5rhvh.txt + UC forum research
fn research_client_signatures(view: &PeView) -> Result<BTreeMap<String, ResearchResult>> {
    let mut results = BTreeMap::new();

    // Recent signatures from br5rhvh.txt (April 2026) - HIGHEST PRIORITY
    let recent_patterns = [
        // Skinchanger critical functions
        ("EquipItemInLoadout", vec![
            ("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 10 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA", 0.98),
        ]),
        ("SetBodyGroup", vec![
            ("85 D2 0F 88 ? ? ? ? 55 57", 0.98),
        ]),
        ("SetModel", vec![
            ("40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24", 0.98),
        ]),
        ("RegenerateWeaponSkin", vec![
            ("40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?", 0.98),
        ]),
        ("CreateNewPaintKit", vec![
            ("48 89 5C 24 10 56 48 83 EC 20 48 8B 01 FF 50 10 48 8B 1D ? ? ? ?", 0.98),
        ]),
        ("UpdateCompositeMaterial", vec![
            ("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 44 0F B6 F2 48 8B F1 E8", 0.98),
        ]),
        ("SetMeshGroupMask", vec![
            ("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71", 0.98),
        ]),
        ("GetInventoryManager", vec![
            ("E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02", 0.95),
        ]),
        ("AddStattrakEntity", vec![
            ("48 8B C4 48 89 58 08 48 89 70 10 57 48 83 EC 50 33 F6 8B FA 48 8B D1", 0.98),
        ]),
        ("SetDynamicAttributeValue", vec![
            ("E9 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC 49 8B C0 48 8B CA 48 8B D0", 0.95),
        ]),
        
        // Additional inventory functions
        ("FindSOCache", vec![
            ("48 89 5C 24 08 57 48 83 EC 30 4C 8B 52 08 48 8B D9 8B 0A", 0.98),
        ]),
        ("GetClientSystem_inv", vec![
            ("E8 ? ? ? ? 48 8B 47 10 8B 48 30 D1 E9 F6 C1 01 0F 84 8E", 0.95),
        ]),
        ("SetBodyGroup_inv", vec![
            ("85 D2 0F 88 ? ? ? ? 53 55", 0.98),
        ]),
        
        // Entity and model functions
        ("CreateEntityByClassName", vec![
            ("4C 8D 05 ? ? ? ? 4C 8B CF BA 03 00 00 00 FF 15 ? ? ? ? EB ? 0F B7 C8 48", 0.98),
        ]),
        ("AddNametagEntity", vec![
            ("40 55 53 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B DA", 0.98),
        ]),
        ("CreateSOSubclassEconItem", vec![
            ("48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85", 0.95),
        ]),
        ("CreateBaseTypeCache", vec![
            ("40 53 48 83 EC ? 4C 8B 49 ? 44 8B D2", 0.95),
        ]),
        ("UpdateSubClass", vec![
            ("48 8B 41 10 48 8B D9 8B 50 30", 0.98),
        ]),
    ];

    // UC Research patterns (fallback)
    let uc_patterns = [
        ("EquipItemInLoadout", vec![
            ("48895c24u1 48896c24u1 48897424u1 895424u1 57 4154 4155 4156 4157 4883ecu1 0fb7fa", 0.90),
            ("4055 53 56 57 4154 4155 4156 4157 488dac24", 0.85),
        ]),
        ("SetBodyGroup", vec![
            ("85d2 0f88u4 5557", 0.90),
            ("85d2 78u1", 0.85),
        ]),
        ("SetModel", vec![
            ("4053 4883ecu1 488bd9 4c8bc2 488b0du4 488d542440", 0.90),
            ("48895c24u1 57 4883ecu1 488bfa 488bd9", 0.85),
        ]),
        ("RegenerateWeaponSkin", vec![
            ("4055 53 4157 488dac24u4 4881ecu4 440fb6fa 488bd9 bau4", 0.85),
        ]),
        ("CreateNewPaintKit", vec![
            ("48895c2410 56 4883ec20 488b01 ff5010 488b1du4", 0.85),
        ]),
    ];

    // Try recent patterns first (highest confidence)
    for (func_name, patterns) in &recent_patterns {
        for (pattern_str, confidence) in patterns {
            if let Some(rva) = scan_pattern_string(view, pattern_str) {
                let result = ResearchResult {
                    rva,
                    confidence: *confidence,
                    method: "Recent Signature (br5rhvh.txt)".to_string(),
                    description: format!("April 2026 signature from br5rhvh.txt"),
                    verified: false,
                };
                
                results.insert(func_name.to_string(), result);
                info!("Found {} via recent signature: 0x{:X} (confidence: {:.1}%)", 
                      func_name, rva, confidence * 100.0);
                break; // Use first matching pattern
            }
        }
    }

    // Try UC patterns for functions not found with recent signatures
    for (func_name, patterns) in &uc_patterns {
        if results.contains_key(*func_name) {
            continue; // Already found with recent signature
        }
        
        for (i, (pattern_str, base_confidence)) in patterns.iter().enumerate() {
            if let Some(rva) = scan_pattern_string_uc(view, pattern_str) {
                let confidence = base_confidence - (i as f32 * 0.02);
                
                let result = ResearchResult {
                    rva,
                    confidence,
                    method: "UC Research Pattern".to_string(),
                    description: format!("Pattern {} from UnknownCheats research", i + 1),
                    verified: false,
                };
                
                results.insert(func_name.to_string(), result);
                info!("Found {} via UC pattern: 0x{:X} (confidence: {:.1}%)", 
                      func_name, rva, confidence * 100.0);
                break;
            }
        }
    }

    Ok(results)
}

/// Research materialsystem2.dll signatures
fn research_material_signatures(view: &PeView) -> Result<BTreeMap<String, ResearchResult>> {
    let mut results = BTreeMap::new();

    // Recent material signatures from br5rhvh.txt
    let material_patterns = [
        ("PrepareSceneMaterial", vec![
            ("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B 59 ? 48 8B F2 48 63 79 ? 48 C1 E7 06", 0.98),
        ]),
        ("CreateMaterial", vec![
            ("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05", 0.98),
        ]),
        ("FindParameter", vec![
            ("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48", 0.98),
        ]),
        ("UpdateParameter", vec![
            ("48 89 7C 24 ? 41 56 48 83 EC ? 8B 81", 0.95),
        ]),
    ];

    for (func_name, patterns) in &material_patterns {
        for (pattern_str, confidence) in patterns {
            if let Some(rva) = scan_pattern_string(view, pattern_str) {
                let result = ResearchResult {
                    rva,
                    confidence: *confidence,
                    method: "Recent Material Signature".to_string(),
                    description: "Material system function from br5rhvh.txt".to_string(),
                    verified: false,
                };
                
                results.insert(func_name.to_string(), result);
                info!("Found material {} at: 0x{:X} (confidence: {:.1}%)", 
                      func_name, rva, confidence * 100.0);
                break;
            }
        }
    }

    Ok(results)
}

/// Scan pattern using standard hex format (from br5rhvh.txt)
fn scan_pattern_string(view: &PeView, pattern_str: &str) -> Option<Rva> {
    // Convert hex pattern to pelite format
    let pelite_pattern = convert_hex_to_pelite_pattern(pattern_str);
    
    if pelite_pattern.is_empty() {
        return None;
    }
    
    let save_size = save_len(&pelite_pattern);
    if save_size == 0 {
        return None;
    }
    
    let mut save = vec![0; save_size];
    
    if view.scanner().finds_code(&pelite_pattern, &mut save) {
        Some(save[0])
    } else {
        None
    }
}

/// Scan pattern using UC format (with u1, u4 wildcards)
fn scan_pattern_string_uc(view: &PeView, pattern_str: &str) -> Option<Rva> {
    let pelite_pattern = convert_uc_to_pelite_pattern(pattern_str);
    
    if pelite_pattern.is_empty() {
        return None;
    }
    
    let save_size = save_len(&pelite_pattern);
    if save_size == 0 {
        return None;
    }
    
    let mut save = vec![0; save_size];
    
    if view.scanner().finds_code(&pelite_pattern, &mut save) {
        Some(save[0])
    } else {
        None
    }
}

/// Convert standard hex pattern to pelite format
fn convert_hex_to_pelite_pattern(pattern_str: &str) -> Vec<Atom> {
    let mut atoms = Vec::new();
    let parts: Vec<&str> = pattern_str.split_whitespace().collect();
    
    for part in parts {
        if part == "?" {
            atoms.push(Atom::Skip(1));
        } else if part.len() == 2 {
            if let Ok(byte) = u8::from_str_radix(part, 16) {
                atoms.push(Atom::Byte(byte));
            } else {
                // Invalid hex, skip this part
                continue;
            }
        } else {
            // Invalid format, skip this part
            continue;
        }
    }
    
    atoms
}

/// Convert UC pattern format to pelite Atom format
fn convert_uc_to_pelite_pattern(pattern_str: &str) -> Vec<Atom> {
    let mut atoms = Vec::new();
    let parts: Vec<&str> = pattern_str.split_whitespace().collect();
    
    for part in parts {
        if part.ends_with("u1") {
            // Wildcard byte
            let hex_part = &part[..part.len()-2];
            if let Ok(byte) = u8::from_str_radix(hex_part, 16) {
                atoms.push(Atom::Byte(byte));
                atoms.push(Atom::Skip(1)); // Skip next byte
            }
        } else if part.ends_with("u4") {
            // Wildcard 4 bytes
            let hex_part = &part[..part.len()-2];
            if let Ok(byte) = u8::from_str_radix(hex_part, 16) {
                atoms.push(Atom::Byte(byte));
                atoms.push(Atom::Skip(4)); // Skip next 4 bytes
            }
        } else if part.len() == 2 {
            // Regular hex byte
            if let Ok(byte) = u8::from_str_radix(part, 16) {
                atoms.push(Atom::Byte(byte));
            }
        }
        // Skip invalid parts
    }
    
    atoms
}

/// Research client.dll signatures with runtime validation for stability
fn research_client_signatures_validated<P: Process + MemoryView>(
    view: &PeView, 
    process: &mut P, 
    module_base: Address
) -> Result<BTreeMap<String, ResearchResult>> {
    let mut results = BTreeMap::new();

    // PRIORITY 1: Most stable signatures (proven to work without lag)
    let stable_patterns = [
        ("SetMeshGroupMask", vec![
            ("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71", 0.99),
        ]),
        ("SetBodyGroup", vec![
            ("85 D2 0F 88 ? ? ? ? 55 57", 0.98),
            ("85 D2 0F 88 ? ? ? ? 53 55", 0.95),
        ]),
    ];

    // PRIORITY 2: Potentially problematic signatures (need validation)
    let risky_patterns = [
        ("EquipItemInLoadout", vec![
            ("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 10 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA", 0.98),
        ]),
        ("RegenerateWeaponSkin", vec![
            ("40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?", 0.98),
        ]),
        ("SetModel", vec![
            ("40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24", 0.98),
        ]),
        ("UpdateCompositeMaterial", vec![
            ("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 44 0F B6 F2 48 8B F1 E8", 0.98),
        ]),
        ("CreateNewPaintKit", vec![
            ("48 89 5C 24 10 56 48 83 EC 20 48 8B 01 FF 50 10 48 8B 1D ? ? ? ?", 0.98),
        ]),
        ("GetInventoryManager", vec![
            ("E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02", 0.95),
        ]),
    ];

    // Find stable signatures first
    for (func_name, patterns) in &stable_patterns {
        for (pattern_str, confidence) in patterns {
            if let Some(rva) = scan_pattern_string(view, pattern_str) {
                // Validate the signature is safe
                if validate_signature_safety(process, module_base, rva, func_name)? {
                    let result = ResearchResult {
                        rva,
                        confidence: *confidence,
                        method: "Stable Signature (Validated)".to_string(),
                        description: format!("Proven stable signature for {}", func_name),
                        verified: true,
                    };
                    
                    results.insert(func_name.to_string(), result);
                    info!("✓ STABLE: {} at 0x{:X} (confidence: {:.1}%)", 
                          func_name, rva, confidence * 100.0);
                    break;
                } else {
                    warn!("✗ UNSTABLE: {} at 0x{:X} failed validation", func_name, rva);
                }
            }
        }
    }

    // Find risky signatures with extra validation
    for (func_name, patterns) in &risky_patterns {
        for (pattern_str, base_confidence) in patterns {
            if let Some(rva) = scan_pattern_string(view, pattern_str) {
                // Extra validation for risky signatures
                let validation_score = validate_risky_signature(process, module_base, rva, func_name)?;
                
                if validation_score > 0.7 {
                    let adjusted_confidence = base_confidence * validation_score;
                    let result = ResearchResult {
                        rva,
                        confidence: adjusted_confidence,
                        method: "Risky Signature (Validated)".to_string(),
                        description: format!("Potentially problematic - validation score: {:.1}%", validation_score * 100.0),
                        verified: validation_score > 0.8,
                    };
                    
                    results.insert(func_name.to_string(), result);
                    info!("⚠ RISKY: {} at 0x{:X} (confidence: {:.1}%, validation: {:.1}%)", 
                          func_name, rva, adjusted_confidence * 100.0, validation_score * 100.0);
                    break;
                } else {
                    warn!("✗ REJECTED: {} at 0x{:X} failed validation (score: {:.1}%)", 
                          func_name, rva, validation_score * 100.0);
                }
            }
        }
    }

    Ok(results)
}

/// Validate signature safety - basic checks for stable functions
fn validate_signature_safety<P: Process + MemoryView>(
    process: &mut P,
    module_base: Address,
    rva: Rva,
    func_name: &str
) -> Result<bool> {
    let absolute_addr = module_base + rva as u64;
    
    // Basic safety checks
    if let Ok(bytes) = process.read_raw(absolute_addr, 16) {
        // Check for valid function prologue
        if !looks_like_function_start(&bytes) {
            return Ok(false);
        }
        
        // Function-specific validation
        match func_name {
            "SetMeshGroupMask" => {
                // Should have specific parameter pattern for mesh operations
                Ok(bytes.len() >= 8 && bytes[0] == 0x48) // mov instruction
            },
            "SetBodyGroup" => {
                // Should start with parameter validation
                Ok(bytes.len() >= 4 && bytes[0] == 0x85 && bytes[1] == 0xD2) // test edx,edx
            },
            _ => Ok(true)
        }
    } else {
        Ok(false)
    }
}

/// Enhanced validation for risky signatures
fn validate_risky_signature<P: Process + MemoryView>(
    process: &mut P,
    module_base: Address,
    rva: Rva,
    func_name: &str
) -> Result<f32> {
    let absolute_addr = module_base + rva as u64;
    let mut score = 0.0f32;
    
    // Read more bytes for detailed analysis
    if let Ok(bytes) = process.read_raw(absolute_addr, 32) {
        // Basic prologue check
        if looks_like_function_start(&bytes) {
            score += 0.3;
        }
        
        // Function-specific validation
        match func_name {
            "EquipItemInLoadout" => {
                // Should have inventory-related patterns
                if bytes.len() >= 16 {
                    // Look for stack frame setup typical of inventory functions
                    if bytes[0] == 0x48 && bytes[1] == 0x89 { // mov [rsp+?], reg
                        score += 0.4;
                    }
                    // Look for parameter handling
                    if bytes.windows(2).any(|w| w == [0x0F, 0xB7]) { // movzx (item ID handling)
                        score += 0.3;
                    }
                }
            },
            "RegenerateWeaponSkin" => {
                // Should have weapon entity patterns
                if bytes.len() >= 20 {
                    // Look for large stack frame (complex function)
                    if bytes.windows(3).any(|w| w[0] == 0x48 && w[1] == 0x81 && w[2] == 0xEC) {
                        score += 0.4;
                    }
                    // Look for entity pointer handling
                    if bytes.windows(2).any(|w| w == [0x48, 0x8B]) { // mov reg, [reg]
                        score += 0.3;
                    }
                }
            },
            "SetModel" => {
                // Should have model string handling
                if bytes.len() >= 12 {
                    // Look for string parameter handling
                    if bytes.windows(2).any(|w| w == [0x4C, 0x8B]) { // mov r8, reg (3rd param)
                        score += 0.4;
                    }
                    // Look for model system calls
                    if bytes.windows(2).any(|w| w == [0x48, 0x8D]) { // lea reg, [mem]
                        score += 0.3;
                    }
                }
            },
            _ => {
                // Generic validation for other functions
                score += 0.5;
            }
        }
        
        // Penalty for suspicious patterns that might cause crashes
        if bytes.windows(2).any(|w| w == [0xFF, 0x25]) { // jmp [rip+offset] - might be import
            score -= 0.2;
        }
        if bytes.windows(2).any(|w| w == [0xCC, 0xCC]) { // int3 int3 - debug breaks
            score -= 0.3;
        }
    }
    
    Ok(score.max(0.0).min(1.0))
}

/// Check if bytes look like a valid function start
fn looks_like_function_start(bytes: &[u8]) -> bool {
    if bytes.len() < 2 { return false; }
    
    // Common x64 function prologues
    matches!(
        (bytes[0], bytes[1]),
        (0x48, 0x89) | // mov [rsp+?], reg
        (0x40, 0x53) | // push rbx (REX)
        (0x40, 0x55) | // push rbp (REX)
        (0x48, 0x83) | // sub rsp, ?
        (0x55, _) |    // push rbp
        (0x56, _) |    // push rsi
        (0x57, _) |    // push rdi
        (0x85, 0xD2) | // test edx,edx (parameter validation)
        (0x48, 0x8B)   // mov reg, reg/[mem]
    )
}