#pragma once
//
// EnhancedScanner  -  Modern, IDA + Ghidra-style signature dumper for CS2.
//
// Capabilities (per signature):
//   * IDA-style pattern strings ("48 8B ? ? ? ? E8")
//   * Per-module scoping (only .text of `client.dll`, etc.) -> 100x faster +
//     correct (no false hits in unrelated DLLs)
//   * Optional automatic relative-address resolution after a hit:
//       - Rel32     (E8/E9 + 4 bytes)         => target call/jump
//       - RipRel    (48 8B/8D/89 05/0D + 4)   => target global / function
//       - StringRef (locate a unique string in .rdata, then find the .text
//                    instruction that LEAs it, then walk back to the function
//                    prologue) - this is the Ghidra "find by string" workflow
//
// Outputs a JSON document containing every requested signature plus, as a
// bonus pass:
//   * every PE export of every scanned module
//   * every Source 2 interface (walked from the module's CreateInterface
//     export through the s_pInterfaceRegs linked list)
//
#include "Memory.h"
#include "ModuleAnalyzer.h"
#include <string>
#include <vector>
#include <cstdint>

enum class ResolveKind {
    None,        // raw match address
    Rel32,       // E8/E9 relative call/jump - byte at `relOffset` is the
                 // first byte of the disp32; instruction size = relOffset + 4
    RipRel,      // 48 8B/8D/89 05/0D - same offset semantics, but target
                 // is data, not code
    StringRef,   // pattern field is interpreted as the literal string;
                 // we search .rdata, find an .text reference, then walk
                 // backwards to the function start.
};

struct EnhancedSig {
    std::string  name;          // human label
    std::wstring module;        // e.g. L"client.dll"; empty = any module
    std::string  ida;           // IDA-style pattern OR string literal
    ResolveKind  resolve  = ResolveKind::None;
    int          relOffset = 0; // for Rel32/RipRel
    int          extraOff = 0;  // additional offset added to the resolved VA

    // Filled in by Scan():
    bool         found = false;
    uintptr_t    matchVA = 0;     // raw match
    uintptr_t    resolvedVA = 0;  // after resolution
    std::string  error;
};

class EnhancedScanner {
public:
    explicit EnhancedScanner(Memory& mem) : m_mem(mem), m_ana(mem) {}

    void Add(EnhancedSig sig) { m_sigs.push_back(std::move(sig)); }

    // Convenience helpers
    void AddRaw(const char* name, const wchar_t* mod, const char* ida) {
        Add({ name, mod ? mod : L"", ida, ResolveKind::None, 0, 0 });
    }
    void AddRel32(const char* name, const wchar_t* mod, const char* ida, int relOff) {
        Add({ name, mod ? mod : L"", ida, ResolveKind::Rel32, relOff, 0 });
    }
    void AddRipRel(const char* name, const wchar_t* mod, const char* ida, int relOff) {
        Add({ name, mod ? mod : L"", ida, ResolveKind::RipRel, relOff, 0 });
    }
    void AddStringRef(const char* name, const wchar_t* mod, const char* str) {
        Add({ name, mod ? mod : L"client.dll", str, ResolveKind::StringRef, 0, 0 });
    }

    // Run the scan; writes JSON, prints progress.
    bool Run(const std::string& jsonOut);

    size_t Count() const { return m_sigs.size(); }
    size_t FoundCount() const;

private:
    // Per-module pre-loaded section data so each sig only reads the .text once.
    struct CachedModule {
        ModuleInfo                       info;
        std::vector<uint8_t>             textBytes;
        const ModuleSection*             text = nullptr;
        bool                             loaded = false;
    };
    CachedModule& Cache(const std::wstring& mod);

    bool ScanOne(EnhancedSig& s);
    bool ResolveOne(EnhancedSig& s);

    static uintptr_t SearchBytes(const uint8_t* hay, size_t hayLen,
                                 const std::string& bytes,
                                 const std::string& mask);

    void DumpJson(const std::string& path);

    Memory& m_mem;
    ModuleAnalyzer m_ana;
    std::vector<EnhancedSig> m_sigs;
    std::vector<CachedModule> m_cache;

    // Per-module bonus dumps
    struct ModuleDump {
        std::wstring                module;
        std::vector<ExportEntry>    exports;
        std::vector<InterfaceEntry> interfaces;
    };
    std::vector<ModuleDump> m_moduleDumps;
};
