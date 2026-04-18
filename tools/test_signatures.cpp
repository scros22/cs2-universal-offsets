#include <iostream>
#include <Windows.h>
#include "signature_scanner.h"

// ---------------------------------------------------------------
// Signature Testing Program
// Run this to verify all patterns work before using them
// ---------------------------------------------------------------

int main()
{
    std::cout << "CS2 Signature Scanner & Verification Tool" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // Wait for CS2 to be running
    while (!GetModuleHandleW(L"client.dll")) {
        std::cout << "Waiting for CS2 (client.dll) to be loaded..." << std::endl;
        Sleep(1000);
    }
    
    std::cout << "CS2 detected! Starting signature scan..." << std::endl;
    
    // Run enhanced scanning
    SignatureScanner::EnhancedScan();
    
    // Test specific patterns
    std::cout << "\n=== Testing Critical Patterns ===" << std::endl;
    
    uintptr_t equipItem = SignatureScanner::GetPattern("EquipItemInLoadout");
    if (equipItem) {
        std::cout << "[TEST] EquipItemInLoadout: 0x" << std::hex << equipItem << " - READY" << std::endl;
    } else {
        std::cout << "[TEST] EquipItemInLoadout: FAILED - Skinchanger may not work properly" << std::endl;
    }
    
    uintptr_t setBodyGroup = SignatureScanner::GetPattern("SetBodyGroup");
    if (setBodyGroup) {
        std::cout << "[TEST] SetBodyGroup: 0x" << std::hex << setBodyGroup << " - READY" << std::endl;
    } else {
        std::cout << "[TEST] SetBodyGroup: FAILED - Glove changer may not work" << std::endl;
    }
    
    uintptr_t setModel = SignatureScanner::GetPattern("SetModel");
    if (setModel) {
        std::cout << "[TEST] SetModel: 0x" << std::hex << setModel << " - READY" << std::endl;
    } else {
        std::cout << "[TEST] SetModel: FAILED - Knife model changes may not work" << std::endl;
    }
    
    // Generate updated signatures.h file
    std::cout << "\n=== Generating Updated Signatures ===" << std::endl;
    
    FILE* f = fopen("verified_signatures.h", "w");
    if (f) {
        fprintf(f, "#pragma once\n\n");
        fprintf(f, "// Auto-generated verified signatures\n");
        fprintf(f, "// Generated at runtime from working CS2 build\n\n");
        fprintf(f, "namespace VerifiedSignatures {\n");
        
        for (const auto& pattern : SignatureScanner::g_patterns) {
            if (pattern.verified) {
                fprintf(f, "    // %s - VERIFIED\n", pattern.name);
                fprintf(f, "    constexpr const char* %s = \"%s\";\n", pattern.name, pattern.pattern);
                fprintf(f, "    // Found at: 0x%llX in %s\n\n", pattern.result, pattern.module);
            } else {
                fprintf(f, "    // %s - FAILED TO VERIFY\n", pattern.name);
                fprintf(f, "    // constexpr const char* %s = \"%s\";\n\n", pattern.name, pattern.pattern);
            }
        }
        
        fprintf(f, "}\n");
        fclose(f);
        
        std::cout << "Generated verified_signatures.h with working patterns!" << std::endl;
    }
    
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}