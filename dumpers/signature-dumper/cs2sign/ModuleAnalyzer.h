#pragma once
//
// ModuleAnalyzer  -  PE-aware helpers that read the target process's
//                    module headers so we can:
//      * scan only the .text section (faster + correct)
//      * walk the export directory
//      * walk the s_pInterfaceRegs linked list created by every Source 2
//        DLL when CreateInterface is exported (Ghidra-style enumeration)
//      * resolve RIP-relative operands captured inside a signature
//
#include "Memory.h"
#include <string>
#include <vector>
#include <cstdint>
#include <windows.h>

struct ModuleSection {
    std::string name;        // ".text", ".rdata", etc.
    uintptr_t   start;       // VA in target process
    size_t      size;
    DWORD       characteristics;
    bool        executable;
    bool        readable;
};

struct ModuleInfo {
    std::wstring name;       // "client.dll"
    uintptr_t    base;
    size_t       size;
    std::vector<ModuleSection> sections;
};

struct ExportEntry {
    std::string name;
    uintptr_t   address;     // absolute VA in target
    uint32_t    rva;
    uint16_t    ordinal;
};

struct InterfaceEntry {
    std::string name;        // e.g. "Source2Client002"
    uintptr_t   factory;     // factory function VA
    uintptr_t   regNode;     // address of the InterfaceReg node
};

class ModuleAnalyzer {
public:
    explicit ModuleAnalyzer(Memory& mem) : m_mem(mem) {}

    bool Load(const std::wstring& moduleName, ModuleInfo& out);

    // Snapshot the .text section into a local buffer for fast scanning.
    bool ReadSection(const ModuleSection& sec, std::vector<uint8_t>& out);

    // Find every occurrence of a NUL-terminated ASCII string inside .rdata.
    // Returns absolute VAs.
    std::vector<uintptr_t> FindStringRefs(const ModuleInfo& mod, const std::string& s);

    // Find every instruction in .text whose RIP-relative operand resolves to
    // `target`.  Recognises:
    //   E8 disp32         (call rel32)
    //   E9 disp32         (jmp  rel32)
    //   48 8B 05/0D/15/1D disp32
    //   48 8D 05/0D/15/1D disp32
    //   48 89 05/0D ...   (mov [rip+disp32], reg)
    // Returns the address OF the instruction.
    std::vector<uintptr_t> FindRefsTo(const ModuleInfo& mod, uintptr_t target);

    // Walk back from an arbitrary address inside a function to the nearest
    // recognisable prologue.  Useful for converting a string-xref into a
    // function-address signature.  Returns 0 on failure.
    uintptr_t FindFunctionStart(const ModuleInfo& mod, uintptr_t insideFn,
                                size_t maxLookback = 0x800);

    // Dump every PE export.
    bool DumpExports(const ModuleInfo& mod, std::vector<ExportEntry>& out);

    // Walk the Source 2 InterfaceReg linked list rooted at `s_pInterfaceRegs`
    // (the global written by every Source 2 DLL inside CreateInterface()).
    bool DumpInterfaces(const ModuleInfo& mod, std::vector<InterfaceEntry>& out);

    // Resolve a captured RIP-relative operand at `instrAddr` into an absolute
    // address.  `dispOffset` is the offset within the instruction where the
    // 4-byte little-endian disp32 sits.  `instrSize` is the instruction's
    // total length (so RIP = instrAddr + instrSize).
    bool ResolveRipRel(uintptr_t instrAddr, size_t dispOffset, size_t instrSize,
                       uintptr_t& outAbs);

private:
    Memory& m_mem;
};
