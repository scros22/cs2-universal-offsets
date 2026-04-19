#pragma once

#include "Memory.h"
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

struct Signature {
    std::string name;
    std::string pattern;
    std::string mask;
    uintptr_t offset;
    bool found;
    std::string error;
    size_t regionsScanned;
    size_t bytesScanned;
};

class SignatureScanner {
public:
    SignatureScanner(Memory& memory);
    ~SignatureScanner();
    
    void AddSignature(const std::string& name, const std::string& pattern, const std::string& mask, uintptr_t offset = 0);
    void AddSignature(const std::string& name, const char* pattern, size_t patternLen, const std::string& mask, uintptr_t offset = 0);
    void ScanAll();
    void DumpResultsJSON(const std::string& filename = "cs2_signatures.json");
    void UpdateJSONFile();
    
    std::vector<Signature>& GetSignatures() { return m_signatures; }
    const std::vector<Signature>& GetSignatures() const { return m_signatures; }

private:
    uintptr_t ScanPattern(uintptr_t start, size_t size, const std::string& pattern, const std::string& mask, std::string& error);
    uintptr_t ScanPatternOptimized(uintptr_t start, size_t size, const std::string& pattern, const std::string& mask, std::string& error);
    bool ComparePattern(const uint8_t* data, const std::string& pattern, const std::string& mask, size_t length);
    std::string EscapeJSON(const std::string& str);
    
    Memory& m_memory;
    std::vector<Signature> m_signatures;
    std::string m_jsonFilename;
    bool m_jsonInitialized;
    std::mutex m_mutex;
};

