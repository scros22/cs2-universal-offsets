#include <iostream>
#include <windows.h>
#include "Memory.h"
#include "SignatureScanner.h"
#include "CS2Signatures.h"
#include "EnhancedScanner.h"
#include "CS2EnhancedSignatures.h"
#include "Console.h"
#include <vector>
#include <string>
#include <iomanip>

int main() {
    // Initialize console
    Console::Init();
    system("title CS2 Signature Dumper - bcwtf");
    
    // Print banner
    Console::PrintBanner();
    
    Console::PrintHeader(L"Initialization");
    Memory memory;
    
    // Try to attach to CS2
    std::wstring processName = L"cs2.exe";
    Console::PrintInfo(L"Searching for process: " + processName);
    
    if (!memory.Attach(processName)) {
        Console::PrintError(L"Failed to attach to " + processName + L"!");
        Console::PrintWarning(L"Make sure CS2 is running and try again.");
        Console::PrintFooter();
        system("pause");
        return 1;
    }

    Console::PrintSuccess(L"Successfully attached to CS2");
    Console::SetColor(Console::CYAN);
    std::cout << "  Process ID: ";
    Console::SetColor(Console::YELLOW);
    std::cout << memory.GetProcessId() << std::endl;
    Console::ResetColor();
    Console::PrintFooter();

    // Create scanner and add all signatures
    Console::PrintHeader(L"Signature Database");
    SignatureScanner scanner(memory);
    AddCS2Signatures(scanner);
    
    Console::PrintSuccess(L"Loaded " + std::to_wstring(scanner.GetSignatures().size()) + L" signatures");
    Console::PrintFooter();

    // Scan all signatures
    Console::PrintHeader(L"Memory Scanning");
    Console::PrintInfo(L"Scanning memory regions...");
    
    auto regions = memory.GetMemoryRegions();
    Console::SetColor(Console::CYAN);
    std::cout << "  Memory Regions: ";
    Console::SetColor(Console::YELLOW);
    std::cout << regions.size() << std::endl;
    Console::ResetColor();
    
    std::cout << "\n";
    scanner.ScanAll();
    
    Console::PrintFooter();
    
    // Print summary
    Console::PrintHeader(L"Scan Results");
    size_t found = 0;
    size_t errors = 0;
    for (const auto& sig : scanner.GetSignatures()) {
        if (sig.found) {
            found++;
        } else if (!sig.error.empty() && sig.error.find("Pattern and mask length mismatch") != std::string::npos) {
            errors++;
        }
    }
    
    Console::SetColor(Console::GREEN);
    std::cout << "  [+] Found: ";
    Console::SetColor(Console::YELLOW);
    std::cout << found;
    Console::SetColor(Console::WHITE);
    std::cout << " / " << scanner.GetSignatures().size() << std::endl;
    Console::ResetColor();
    
    if (errors > 0) {
        Console::SetColor(Console::RED);
        std::cout << "  [-] Errors: ";
        Console::SetColor(Console::YELLOW);
        std::cout << errors << std::endl;
        Console::ResetColor();
    }
    
    Console::SetColor(Console::CYAN);
    std::cout << "  [*] Results saved to: ";
    Console::SetColor(Console::YELLOW);
    std::cout << "cs2_signatures.json" << std::endl;
    Console::ResetColor();
    
    Console::PrintFooter();

    // ============================================================
    // ENHANCED PASS - PE/section-aware, IDA-style, with Ghidra-style
    // string-xref resolution + interface registry walking
    // ============================================================
    {
        EnhancedScanner enh(memory);
        RegisterCS2Signatures(enh);
        Console::PrintHeader(L"Enhanced Signature DB");
        Console::PrintSuccess(L"Loaded " + std::to_wstring(enh.Count()) + L" enhanced signatures");
        Console::PrintFooter();
        enh.Run("cs2_signatures_enhanced.json");
        Console::PrintSuccess(L"Enhanced: " + std::to_wstring(enh.FoundCount())
                              + L" / " + std::to_wstring(enh.Count()));
    }

    memory.Detach();
    
    Console::SetColor(Console::DARK_GRAY);
    std::cout << "\n  Press any key to exit...";
    Console::ResetColor();
    system("pause >nul");
    
    return 0;
}

