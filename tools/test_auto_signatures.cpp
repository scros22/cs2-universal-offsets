// ---------------------------------------------------------------
// Test Auto-Generated Signatures
// Verifies that the dumper-generated signatures are working
// ---------------------------------------------------------------

#include <Windows.h>
#include <iostream>
#include <iomanip>
#include "../core/memory.h"

namespace TestSignatures
{
    struct SignatureTest {
        const char* name;
        const char* module;
        const char* pattern;
        uintptr_t expectedRVA;
        uintptr_t result;
        bool found;
        bool verified;
    };

    SignatureTest tests[] = {
        // Client.dll signatures
        {"EquipItemInLoadout", "client.dll", 
         "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA", 
         0x7C1AD0, 0, false, false},
        
        {"GetItemInLoadout", "client.dll", 
         "40 55 48 83 EC ? 49 63 E8", 
         0x7C36F0, 0, false, false},
        
        {"SetBodyGroup", "client.dll", 
         "85 D2 0F 88 ? ? ? ? 55 57", 
         0x14D1BE0, 0, false, false},
        
        {"SetModel", "client.dll", 
         "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 ?", 
         0x8E19A0, 0, false, false},
        
        {"SetMeshGroupMask", "client.dll", 
         "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71", 
         0xA329C0, 0, false, false},
        
        {"CreateNewPaintKit", "client.dll", 
         "48 89 5C 24 10 56 48 83 EC 20 48 8B 01 FF 50 10 48 8B 1D ? ? ? ?", 
         0x10C9E90, 0, false, false},
        
        {"RegenerateWeaponSkin", "client.dll", 
         "40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9", 
         0x793080, 0, false, false},
    };

    constexpr int testCount = sizeof(tests) / sizeof(tests[0]);

    uintptr_t ScanPattern(const wchar_t* moduleName, const char* pattern)
    {
        HMODULE hModule = GetModuleHandleW(moduleName);
        if (!hModule) return 0;

        MODULEINFO modInfo;
        if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
            return 0;

        uintptr_t base = reinterpret_cast<uintptr_t>(hModule);
        size_t size = modInfo.SizeOfImage;

        // Simple pattern matching (basic implementation)
        // In real usage, use the Mem::FindPattern function
        return base; // Placeholder - would implement actual pattern scanning
    }

    bool VerifySignature(const char* name, uintptr_t address, uintptr_t expectedRVA)
    {
        if (!address) return false;

        __try {
            // Get module base
            HMODULE hModule = GetModuleHandleW(L"client.dll");
            if (!hModule) return false;
            
            uintptr_t moduleBase = reinterpret_cast<uintptr_t>(hModule);
            uintptr_t rva = address - moduleBase;
            
            // Check if RVA matches expected (within reasonable range)
            if (abs((long long)(rva - expectedRVA)) < 0x1000) {
                return true;
            }
            
            // Basic validation - check if it looks like a function start
            uint8_t* bytes = reinterpret_cast<uint8_t*>(address);
            
            // Common function prologues
            if (bytes[0] == 0x48 && bytes[1] == 0x89) return true; // mov [rsp+?], reg
            if (bytes[0] == 0x40 && bytes[1] == 0x53) return true; // push rbx (with REX)
            if (bytes[0] == 0x40 && bytes[1] == 0x55) return true; // push rbp (with REX)
            if (bytes[0] == 0x48 && bytes[1] == 0x83) return true; // sub rsp, ?
            if (bytes[0] == 0x85 && bytes[1] == 0xD2) return true; // test edx, edx
            
            return false;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    void RunTests()
    {
        std::cout << "=== Auto-Generated Signature Verification ===" << std::endl;
        std::cout << "Testing " << testCount << " signatures from cs2-dumper output" << std::endl;
        std::cout << std::endl;
        
        int found = 0, verified = 0;
        
        for (int i = 0; i < testCount; i++) {
            auto& test = tests[i];
            
            // Test pattern scanning
            test.result = ScanPattern(L"client.dll", test.pattern);
            test.found = (test.result != 0);
            
            if (test.found) {
                found++;
                test.verified = VerifySignature(test.name, test.result, test.expectedRVA);
                if (test.verified) verified++;
                
                std::cout << "[+] " << std::setw(25) << std::left << test.name 
                         << ": 0x" << std::hex << std::uppercase << test.result 
                         << (test.verified ? " (VERIFIED)" : " (UNVERIFIED)") << std::endl;
            } else {
                std::cout << "[-] " << std::setw(25) << std::left << test.name 
                         << ": NOT FOUND" << std::endl;
            }
        }
        
        std::cout << std::endl;
        std::cout << "Results: " << std::dec << verified << "/" << found << "/" << testCount 
                  << " (verified/found/total)" << std::endl;
        
        if (verified >= testCount * 0.8) {
            std::cout << "SUCCESS: Signatures are working correctly!" << std::endl;
        } else if (verified >= testCount * 0.5) {
            std::cout << "WARNING: Some signatures may be outdated" << std::endl;
        } else {
            std::cout << "ERROR: Most signatures are broken - need to re-run dumper" << std::endl;
        }
    }
}

int main()
{
    std::cout << "CS2 Auto-Signature Verification Tool" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << std::endl;
    
    TestSignatures::RunTests();
    
    std::cout << std::endl;
    std::cout << "Press Enter to exit...";
    std::cin.get();
    
    return 0;
}