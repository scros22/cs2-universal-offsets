#pragma once

// ---------------------------------------------------------------
// Signature Scanner & Verification System
// Validates patterns and finds correct offsets automatically
// ---------------------------------------------------------------

#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
#include "../core/memory.h"

namespace SignatureScanner
{
    struct PatternInfo {
        const char* name;
        const char* module;
        const char* pattern;
        uintptr_t   result;
        bool        found;
        bool        verified;
    };

    // Enhanced patterns with verification
    inline std::vector<PatternInfo> g_patterns = {
        // Skinchanger patterns
        {"EquipItemInLoadout", "client.dll", 
         "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA", 0, false, false},
        
        {"GetItemInLoadout", "client.dll", 
         "40 55 48 83 EC ? 49 63 E8", 0, false, false},
        
        {"SetBodyGroup", "client.dll", 
         "85 D2 0F 88 5C", 0, false, false},
        
        {"SetModel", "client.dll", 
         "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40", 0, false, false},
        
        {"GetPaintKitIndex", "client.dll", 
         "48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25", 0, false, false},
        
        // World effects patterns
        {"TonemapController", "client.dll", 
         "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?", 0, false, false},
        
        {"FogController", "client.dll", 
         "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 48 8B EA", 0, false, false},
        
        // Material system
        {"CreateMaterial", "materialsystem2.dll", 
         "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05", 0, false, false},
    };

    // ---------------------------------------------------------------
    // Pattern scanning with enhanced validation
    // ---------------------------------------------------------------
    inline uintptr_t ScanPattern(const wchar_t* moduleName, const char* pattern)
    {
        HMODULE hModule = GetModuleHandleW(moduleName);
        if (!hModule) return 0;

        MODULEINFO modInfo;
        if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
            return 0;

        uintptr_t base = reinterpret_cast<uintptr_t>(hModule);
        size_t size = modInfo.SizeOfImage;

        // Convert pattern string to bytes
        std::vector<int> patternBytes;
        std::vector<bool> mask;
        
        const char* p = pattern;
        while (*p) {
            if (*p == ' ') {
                p++;
                continue;
            }
            if (*p == '?') {
                patternBytes.push_back(0);
                mask.push_back(false);
                p++;
                if (*p == '?') p++; // Skip second ?
            } else {
                char hex[3] = {p[0], p[1], 0};
                patternBytes.push_back(strtol(hex, nullptr, 16));
                mask.push_back(true);
                p += 2;
            }
        }

        // Scan memory
        for (size_t i = 0; i <= size - patternBytes.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < patternBytes.size(); j++) {
                if (mask[j] && *(uint8_t*)(base + i + j) != patternBytes[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return base + i;
            }
        }
        return 0;
    }

    // ---------------------------------------------------------------
    // Verify function signatures by checking nearby code
    // ---------------------------------------------------------------
    inline bool VerifySignature(const char* name, uintptr_t address)
    {
        if (!address) return false;

        __try {
            // Basic validation - check if it looks like a function start
            uint8_t* bytes = reinterpret_cast<uint8_t*>(address);
            
            // Common function prologues
            if (bytes[0] == 0x48 && bytes[1] == 0x89) return true; // mov [rsp+?], reg
            if (bytes[0] == 0x40 && bytes[1] == 0x53) return true; // push rbx (with REX)
            if (bytes[0] == 0x48 && bytes[1] == 0x83) return true; // sub rsp, ?
            if (bytes[0] == 0x55) return true; // push rbp
            if (bytes[0] == 0x48 && bytes[1] == 0x8B) return true; // mov reg, reg
            if (bytes[0] == 0x85 && bytes[1] == 0xD2) return true; // test edx, edx (SetBodyGroup)
            
            return false;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    // ---------------------------------------------------------------
    // Scan all patterns and verify them
    // ---------------------------------------------------------------
    inline void ScanAllPatterns()
    {
        std::cout << "=== Signature Scanner Results ===" << std::endl;
        
        for (auto& pattern : g_patterns) {
            std::wstring moduleW(pattern.module, pattern.module + strlen(pattern.module));
            pattern.result = ScanPattern(moduleW.c_str(), pattern.pattern);
            pattern.found = (pattern.result != 0);
            
            if (pattern.found) {
                pattern.verified = VerifySignature(pattern.name, pattern.result);
                std::cout << "[+] " << pattern.name << ": 0x" << std::hex << pattern.result 
                         << (pattern.verified ? " (VERIFIED)" : " (UNVERIFIED)") << std::endl;
            } else {
                std::cout << "[-] " << pattern.name << ": NOT FOUND" << std::endl;
            }
        }
        
        // Summary
        int found = 0, verified = 0;
        for (const auto& p : g_patterns) {
            if (p.found) found++;
            if (p.verified) verified++;
        }
        
        std::cout << "\nSummary: " << verified << "/" << found << "/" << g_patterns.size() 
                  << " (verified/found/total)" << std::endl;
    }

    // ---------------------------------------------------------------
    // Get pattern result by name
    // ---------------------------------------------------------------
    inline uintptr_t GetPattern(const char* name)
    {
        for (const auto& p : g_patterns) {
            if (strcmp(p.name, name) == 0 && p.verified) {
                return p.result;
            }
        }
        return 0;
    }

    // ---------------------------------------------------------------
    // Alternative pattern search - tries multiple variations
    // ---------------------------------------------------------------
    inline uintptr_t FindAlternativePattern(const char* name, const wchar_t* module)
    {
        // Alternative patterns for common functions
        if (strcmp(name, "SetBodyGroup") == 0) {
            // Try different SetBodyGroup patterns
            const char* alternatives[] = {
                "85 D2 0F 88 5C",
                "85 D2 78 ?",
                "85 D2 0F 88",
                "40 53 48 83 EC ? 8B DA",
            };
            
            for (const char* alt : alternatives) {
                uintptr_t result = ScanPattern(module, alt);
                if (result && VerifySignature(name, result)) {
                    std::cout << "[ALT] Found " << name << " with alternative pattern: " << alt << std::endl;
                    return result;
                }
            }
        }
        
        if (strcmp(name, "EquipItemInLoadout") == 0) {
            const char* alternatives[] = {
                "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57",
                "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC",
                "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24",
            };
            
            for (const char* alt : alternatives) {
                uintptr_t result = ScanPattern(module, alt);
                if (result && VerifySignature(name, result)) {
                    std::cout << "[ALT] Found " << name << " with alternative pattern: " << alt << std::endl;
                    return result;
                }
            }
        }
        
        return 0;
    }

    // ---------------------------------------------------------------
    // Enhanced scan with fallbacks
    // ---------------------------------------------------------------
    inline void EnhancedScan()
    {
        std::cout << "=== Enhanced Signature Scanner ===" << std::endl;
        
        // First pass - try original patterns
        ScanAllPatterns();
        
        // Second pass - try alternatives for failed patterns
        std::cout << "\n=== Trying Alternative Patterns ===" << std::endl;
        
        for (auto& pattern : g_patterns) {
            if (!pattern.verified) {
                std::wstring moduleW(pattern.module, pattern.module + strlen(pattern.module));
                uintptr_t alt = FindAlternativePattern(pattern.name, moduleW.c_str());
                if (alt) {
                    pattern.result = alt;
                    pattern.found = true;
                    pattern.verified = true;
                }
            }
        }
        
        // Final summary
        int verified = 0;
        for (const auto& p : g_patterns) {
            if (p.verified) verified++;
        }
        
        std::cout << "\nFinal Result: " << verified << "/" << g_patterns.size() 
                  << " signatures verified and ready to use!" << std::endl;
    }
}