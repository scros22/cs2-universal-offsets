#include "SignatureScanner.h"
#include "JSONWriter.h"
#include "Console.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <chrono>

SignatureScanner::SignatureScanner(Memory& memory) 
    : m_memory(memory), m_jsonFilename("cs2_signatures.json"), m_jsonInitialized(false) {
}

SignatureScanner::~SignatureScanner() {
}

void SignatureScanner::AddSignature(const std::string& name, const std::string& pattern, const std::string& mask, uintptr_t offset) {
    // For patterns without null bytes, use the string directly
    // For patterns with null bytes, we need to use the length-based version
    // Check if pattern contains null bytes
    bool hasNull = (pattern.find('\0') != std::string::npos);
    if (hasNull) {
        // Pattern has null bytes - need explicit length
        // This shouldn't happen with the old API, but handle it
        AddSignature(name, pattern.c_str(), pattern.size(), mask, offset);
    } else {
        // No null bytes, safe to use normally
        Signature sig;
        sig.name = name;
        sig.pattern = pattern;
        sig.mask = mask;
        sig.offset = offset;
        sig.found = false;
        sig.error = "";
        sig.regionsScanned = 0;
        sig.bytesScanned = 0;
        m_signatures.push_back(sig);
    }
}

void SignatureScanner::AddSignature(const std::string& name, const char* pattern, size_t patternLen, const std::string& mask, uintptr_t offset) {
    Signature sig;
    sig.name = name;
    sig.pattern = std::string(pattern, patternLen); // Use size constructor to preserve null bytes
    sig.mask = mask;
    sig.offset = offset;
    sig.found = false;
    sig.error = "";
    sig.regionsScanned = 0;
    sig.bytesScanned = 0;
    m_signatures.push_back(sig);
}

void SignatureScanner::ScanAll() {
    if (!m_memory.IsValid()) {
        std::wcout << L"Memory not attached!" << std::endl;
        return;
    }

    // Initialize JSON file
    {
        std::ofstream file(m_jsonFilename);
        if (file.is_open()) {
            file << "{\n";
            file << "  \"metadata\": {\n";
            file << "    \"game\": \"Counter-Strike 2\",\n";
            file << "    \"process_id\": " << m_memory.GetProcessId() << ",\n";
            file << "    \"total_signatures\": " << m_signatures.size() << ",\n";
            file << "    \"scan_time\": \"" << __DATE__ << " " << __TIME__ << "\"\n";
            file << "  },\n";
            file << "  \"signatures\": [\n";
            file.close();
        }
        m_jsonInitialized = true;
    }

    auto allRegions = m_memory.GetMemoryRegions();
    
    // Filter and prioritize regions - executable first, then by size (smaller first for faster scanning)
    std::vector<MemoryRegion> validRegions;
    for (const auto& region : allRegions) {
        if (region.size >= 16 && // Skip tiny regions
            (region.protect == PAGE_EXECUTE_READ || 
             region.protect == PAGE_EXECUTE_READWRITE ||
             region.protect == PAGE_READONLY ||
             region.protect == PAGE_READWRITE)) {
            validRegions.push_back(region);
        }
    }
    
    // Sort regions: executable first, then by size (smaller = faster to scan)
    std::sort(validRegions.begin(), validRegions.end(), [](const MemoryRegion& a, const MemoryRegion& b) {
        bool aExec = (a.protect == PAGE_EXECUTE_READ || a.protect == PAGE_EXECUTE_READWRITE);
        bool bExec = (b.protect == PAGE_EXECUTE_READ || b.protect == PAGE_EXECUTE_READWRITE);
        if (aExec != bExec) return aExec; // Executable first
        return a.size < b.size; // Smaller first
    });

    // Determine optimal thread count (use all CPU cores)
    size_t numThreads = (std::max)(1u, std::thread::hardware_concurrency());
    if (numThreads > 8) numThreads = 8; // Cap at 8 for stability

    for (size_t sigIndex = 0; sigIndex < m_signatures.size(); ++sigIndex) {
        auto& sig = m_signatures[sigIndex];
        std::wstring sigName(sig.name.begin(), sig.name.end());
        
        sig.regionsScanned = 0;
        sig.bytesScanned = 0;
        
        // Validate pattern
        size_t patternBytes = sig.pattern.size();
        
        if (patternBytes != sig.mask.length()) {
            std::ostringstream oss;
            oss << "Pattern (" << patternBytes << " bytes) != mask (" << sig.mask.length() << " chars)";
            sig.error = oss.str();
            Console::ClearLine();
            Console::PrintErrorMsg(sigName, sig.error);
            UpdateJSONFile();
            continue;
        }
        
        if (sig.pattern.empty()) {
            sig.error = "Empty pattern";
            Console::ClearLine();
            Console::PrintErrorMsg(sigName, sig.error);
            UpdateJSONFile();
            continue;
        }
        
        // Print progress
        Console::PrintProgress(sigIndex + 1, m_signatures.size(), sigName);
        
        // Filter regions for this signature (must be large enough)
        std::vector<MemoryRegion> scanRegions;
        for (const auto& region : validRegions) {
            if (region.size >= patternBytes) {
                scanRegions.push_back(region);
            }
        }
        
        if (scanRegions.empty()) {
            sig.error = "No suitable memory regions";
            Console::ClearLine();
            Console::PrintNotFound(sigName, sig.error);
            UpdateJSONFile();
            continue;
        }
        
        // Parallel scanning with early termination
        std::atomic<bool> found(false);
        std::atomic<uintptr_t> foundAddress(0);
        std::atomic<size_t> regionsScanned(0);
        std::atomic<size_t> bytesScanned(0);
        
        std::vector<std::thread> threads;
        
        // Launch worker threads
        for (size_t t = 0; t < numThreads; ++t) {
            threads.emplace_back([&, t]() {
                for (size_t i = t; i < scanRegions.size() && !found; i += numThreads) {
                    if (found) break;
                    
                    const auto& region = scanRegions[i];
                    regionsScanned++;
                    bytesScanned += region.size;
                    
                    std::string scanError;
                    uintptr_t result = ScanPatternOptimized(region.base, region.size, sig.pattern, sig.mask, scanError);
                    
                    if (result != 0) {
                        foundAddress = result;
                        found = true;
                        break;
                    }
                }
            });
        }
        
        // Wait for all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        // Update signature with results
        sig.regionsScanned = regionsScanned;
        sig.bytesScanned = bytesScanned;
        
        if (found && foundAddress != 0) {
            sig.found = true;
            sig.offset = foundAddress + sig.offset;
            sig.error = "";
            Console::ClearLine();
            Console::PrintFound(sigName, sig.offset, sig.regionsScanned, sig.bytesScanned);
        } else {
            std::ostringstream oss;
            oss << "Not found after " << sig.regionsScanned << " regions";
            sig.error = oss.str();
            Console::ClearLine();
            Console::PrintNotFound(sigName, sig.error);
        }
        
        UpdateJSONFile();
    }
    
    Console::ClearLine();
    
    // Close JSON array and object
    {
        std::ofstream file(m_jsonFilename, std::ios::app);
        if (file.is_open()) {
            file << "\n  ],\n";
            file << "  \"summary\": {\n";
            size_t foundCount = 0;
            for (const auto& s : m_signatures) {
                if (s.found) foundCount++;
            }
            file << "    \"found\": " << foundCount << ",\n";
            file << "    \"missing\": " << (m_signatures.size() - foundCount) << ",\n";
            file << "    \"total\": " << m_signatures.size() << "\n";
            file << "  }\n";
            file << "}\n";
            file.close();
        }
    }
    
    Console::PrintSuccess(L"Scan complete! Results saved to: " + std::wstring(m_jsonFilename.begin(), m_jsonFilename.end()));
}

uintptr_t SignatureScanner::ScanPattern(uintptr_t start, size_t size, const std::string& pattern, const std::string& mask, std::string& error) {
    return ScanPatternOptimized(start, size, pattern, mask, error);
}

uintptr_t SignatureScanner::ScanPatternOptimized(uintptr_t start, size_t size, const std::string& pattern, const std::string& mask, std::string& error) {
    size_t patternLen = pattern.size();
    if (patternLen != mask.length() || patternLen == 0) {
        return 0;
    }

    if (size < patternLen) {
        return 0;
    }

    // Ultra-fast scanning with optimized chunking
    const size_t MAX_CHUNK = 4 * 1024 * 1024; // 4MB chunks for maximum throughput
    size_t chunkSize = (size > MAX_CHUNK) ? MAX_CHUNK : size;
    
    // Pre-extract first byte for fast filtering
    uint8_t firstByte = static_cast<uint8_t>(pattern[0]);
    bool firstByteWild = (mask[0] == '?');
    
    // Pre-extract second byte if available for even faster filtering
    uint8_t secondByte = (patternLen > 1) ? static_cast<uint8_t>(pattern[1]) : 0;
    bool secondByteWild = (patternLen > 1 && mask[1] == '?');
    
    std::vector<uint8_t> buffer(chunkSize);
    
    for (size_t offset = 0; offset <= size - patternLen; offset += chunkSize - patternLen + 1) {
        size_t readSize = (offset + chunkSize > size) ? (size - offset) : chunkSize;
        if (readSize < patternLen) break;
        
        if (!m_memory.ReadMemory(start + offset, buffer.data(), readSize)) {
            continue;
        }

        size_t searchLen = readSize - patternLen + 1;
        
        // Ultra-optimized scanning with multi-byte filtering
        for (size_t i = 0; i < searchLen; ++i) {
            // Fast path: check first byte
            if (!firstByteWild && buffer[i] != firstByte) {
                continue;
            }
            
            // Fast path: check second byte if available
            if (patternLen > 1 && !secondByteWild && buffer[i + 1] != secondByte) {
                continue;
            }
            
            // Full pattern match only if first bytes match
            if (ComparePattern(buffer.data() + i, pattern, mask, patternLen)) {
                return start + offset + i;
            }
        }
    }

    return 0;
}

bool SignatureScanner::ComparePattern(const uint8_t* data, const std::string& pattern, const std::string& mask, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (mask[i] == 'x' && data[i] != static_cast<uint8_t>(pattern[i])) {
            return false;
        }
    }
    return true;
}

void SignatureScanner::UpdateJSONFile() {
    // Rewrite entire JSON file with current state
    std::ofstream file(m_jsonFilename);
    if (!file.is_open()) {
        return;
    }

    file << "{\n";
    file << "  \"metadata\": {\n";
    file << "    \"game\": \"Counter-Strike 2\",\n";
    file << "    \"process_id\": " << m_memory.GetProcessId() << ",\n";
    file << "    \"total_signatures\": " << m_signatures.size() << ",\n";
    file << "    \"scan_time\": \"" << __DATE__ << " " << __TIME__ << "\"\n";
    file << "  },\n";
    file << "  \"signatures\": [\n";

    for (size_t i = 0; i < m_signatures.size(); ++i) {
        const auto& sig = m_signatures[i];
        file << "    {\n";
        file << "      \"name\": \"" << EscapeJSON(sig.name) << "\",\n";
        
        // Convert pattern to hex string (handle null bytes correctly)
        std::ostringstream patternHex;
        size_t patternLen = sig.pattern.size();
        for (size_t j = 0; j < patternLen; ++j) {
            patternHex << std::hex << std::setw(2) << std::setfill('0') 
                      << static_cast<int>(static_cast<unsigned char>(sig.pattern[j]));
            if (j < patternLen - 1) patternHex << " ";
        }
        file << "      \"pattern\": \"" << patternHex.str() << "\",\n";
        file << "      \"mask\": \"" << EscapeJSON(sig.mask) << "\",\n";
        file << "      \"found\": " << (sig.found ? "true" : "false") << ",\n";
        
        if (sig.found) {
            file << "      \"address\": \"0x" << std::hex << std::uppercase << sig.offset << "\",\n";
            file << "      \"error\": null,\n";
        } else {
            file << "      \"address\": null,\n";
            file << "      \"error\": \"" << EscapeJSON(sig.error) << "\",\n";
        }
        
        file << "      \"regions_scanned\": " << sig.regionsScanned << ",\n";
        file << "      \"bytes_scanned\": " << sig.bytesScanned << "\n";
        file << "    }";
        
        if (i < m_signatures.size() - 1) {
            file << ",";
        }
        file << "\n";
    }

    file << "  ],\n";
    file << "  \"summary\": {\n";
    size_t foundCount = 0;
    for (const auto& s : m_signatures) {
        if (s.found) foundCount++;
    }
    file << "    \"found\": " << foundCount << ",\n";
    file << "    \"missing\": " << (m_signatures.size() - foundCount) << ",\n";
    file << "    \"total\": " << m_signatures.size() << "\n";
    file << "  }\n";
    file << "}\n";
    
    file.flush();
    file.close();
}

void SignatureScanner::DumpResultsJSON(const std::string& filename) {
    m_jsonFilename = filename;
    UpdateJSONFile();
}

std::string SignatureScanner::EscapeJSON(const std::string& str) {
    std::ostringstream o;
    for (size_t i = 0; i < str.length(); ++i) {
        switch (str[i]) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= str[i] && str[i] <= '\x1f') {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)str[i];
                } else {
                    o << str[i];
                }
        }
    }
    return o.str();
}


