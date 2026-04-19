#include "ModuleAnalyzer.h"
#include <psapi.h>
#include <tlhelp32.h>
#include <cstring>
#include <algorithm>

bool ModuleAnalyzer::Load(const std::wstring& moduleName, ModuleInfo& out) {
    out = {};
    out.name = moduleName;

    out.base = m_mem.GetModuleBase(moduleName);
    if (!out.base) return false;

    // Read DOS + NT headers
    IMAGE_DOS_HEADER dos{};
    if (!m_mem.ReadMemory(out.base, &dos, sizeof(dos))) return false;
    if (dos.e_magic != IMAGE_DOS_SIGNATURE) return false;

    IMAGE_NT_HEADERS64 nt{};
    if (!m_mem.ReadMemory(out.base + dos.e_lfanew, &nt, sizeof(nt))) return false;
    if (nt.Signature != IMAGE_NT_SIGNATURE) return false;

    out.size = nt.OptionalHeader.SizeOfImage;

    const uintptr_t secHdrBase = out.base + dos.e_lfanew
        + offsetof(IMAGE_NT_HEADERS64, OptionalHeader)
        + nt.FileHeader.SizeOfOptionalHeader;

    for (WORD i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
        IMAGE_SECTION_HEADER sh{};
        if (!m_mem.ReadMemory(secHdrBase + i * sizeof(sh), &sh, sizeof(sh))) continue;

        ModuleSection sec;
        sec.name.assign(reinterpret_cast<const char*>(sh.Name),
                        strnlen(reinterpret_cast<const char*>(sh.Name), 8));
        sec.start           = out.base + sh.VirtualAddress;
        sec.size            = sh.Misc.VirtualSize ? sh.Misc.VirtualSize : sh.SizeOfRawData;
        sec.characteristics = sh.Characteristics;
        sec.executable      = (sh.Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
        sec.readable        = (sh.Characteristics & IMAGE_SCN_MEM_READ) != 0;
        out.sections.push_back(sec);
    }
    return !out.sections.empty();
}

bool ModuleAnalyzer::ReadSection(const ModuleSection& sec, std::vector<uint8_t>& out) {
    out.assign(sec.size, 0);
    return m_mem.ReadMemory(sec.start, out.data(), sec.size);
}

std::vector<uintptr_t> ModuleAnalyzer::FindStringRefs(const ModuleInfo& mod,
                                                     const std::string& s) {
    std::vector<uintptr_t> hits;
    if (s.empty()) return hits;

    for (const auto& sec : mod.sections) {
        // Strings live in .rdata for the most part
        if (sec.name != ".rdata" && sec.name != ".data") continue;

        std::vector<uint8_t> buf;
        if (!ReadSection(sec, buf)) continue;

        const size_t need = s.size();
        if (buf.size() < need + 1) continue;

        // Boyer-Moore-Horspool-ish (simple linear good enough for ~10 MB .rdata)
        for (size_t i = 0; i + need <= buf.size(); ++i) {
            if (buf[i] != static_cast<uint8_t>(s[0])) continue;
            if (memcmp(buf.data() + i, s.data(), need) != 0) continue;
            // Need NUL terminator (so we don't match substrings of larger strings)
            if (i + need < buf.size() && buf[i + need] != 0) continue;
            hits.push_back(sec.start + i);
        }
    }
    return hits;
}

bool ModuleAnalyzer::ResolveRipRel(uintptr_t instrAddr, size_t dispOffset,
                                   size_t instrSize, uintptr_t& outAbs) {
    int32_t disp = 0;
    if (!m_mem.ReadMemory(instrAddr + dispOffset, &disp, sizeof(disp))) return false;
    outAbs = instrAddr + instrSize + static_cast<intptr_t>(disp);
    return true;
}

std::vector<uintptr_t> ModuleAnalyzer::FindRefsTo(const ModuleInfo& mod,
                                                  uintptr_t target) {
    std::vector<uintptr_t> hits;
    for (const auto& sec : mod.sections) {
        if (!sec.executable) continue;

        std::vector<uint8_t> buf;
        if (!ReadSection(sec, buf)) continue;

        const size_t n = buf.size();
        for (size_t i = 0; i + 7 < n; ++i) {
            const uint8_t b0 = buf[i];

            // E8/E9 disp32
            if (b0 == 0xE8 || b0 == 0xE9) {
                int32_t d = *reinterpret_cast<const int32_t*>(&buf[i + 1]);
                uintptr_t va = sec.start + i;
                if (va + 5 + static_cast<intptr_t>(d) == target) hits.push_back(va);
                continue;
            }

            // 48 8B 05/0D/15/1D disp32   (mov reg, [rip+disp32])
            // 48 8D 05/0D/15/1D disp32   (lea reg, [rip+disp32])
            if (b0 == 0x48 && (buf[i + 1] == 0x8B || buf[i + 1] == 0x8D)) {
                const uint8_t modrm = buf[i + 2];
                if ((modrm & 0xC7) == 0x05) {
                    int32_t d = *reinterpret_cast<const int32_t*>(&buf[i + 3]);
                    uintptr_t va = sec.start + i;
                    if (va + 7 + static_cast<intptr_t>(d) == target) hits.push_back(va);
                }
                continue;
            }

            // 48 89 05/0D disp32 (mov [rip+disp32], reg)
            if (b0 == 0x48 && buf[i + 1] == 0x89) {
                const uint8_t modrm = buf[i + 2];
                if ((modrm & 0xC7) == 0x05) {
                    int32_t d = *reinterpret_cast<const int32_t*>(&buf[i + 3]);
                    uintptr_t va = sec.start + i;
                    if (va + 7 + static_cast<intptr_t>(d) == target) hits.push_back(va);
                }
            }
        }
    }
    return hits;
}

uintptr_t ModuleAnalyzer::FindFunctionStart(const ModuleInfo& mod,
                                            uintptr_t insideFn,
                                            size_t maxLookback) {
    // Heuristic: scan backwards for INT3 / RET padding followed by a typical
    // x64 prologue (`48 89 5C 24` / `48 83 EC` / `40 5? ...` / `48 8B C4`).
    // Read a window ending at insideFn.
    if (insideFn == 0) return 0;
    size_t window = (insideFn > maxLookback) ? maxLookback : (size_t)insideFn;
    std::vector<uint8_t> buf(window);
    if (!m_mem.ReadMemory(insideFn - window, buf.data(), window)) return 0;

    for (ptrdiff_t i = static_cast<ptrdiff_t>(window) - 1; i >= 4; --i) {
        // Look for 0xCC / 0xC3 separator
        const uint8_t b = buf[i];
        if (b != 0xCC && b != 0xC3) continue;
        // Skip CC padding
        ptrdiff_t j = i + 1;
        while (j < (ptrdiff_t)window && buf[j] == 0xCC) ++j;
        if (j + 4 >= (ptrdiff_t)window) continue;

        // Recognise common prologues
        const uint8_t* p = &buf[j];
        const bool prologue =
            (p[0] == 0x48 && p[1] == 0x89 && p[2] == 0x5C && p[3] == 0x24) || // mov [rsp+x],rbx
            (p[0] == 0x48 && p[1] == 0x83 && p[2] == 0xEC)                 || // sub rsp,imm
            (p[0] == 0x48 && p[1] == 0x8B && p[2] == 0xC4)                 || // mov rax,rsp
            (p[0] == 0x40 && (p[1] >= 0x50 && p[1] <= 0x57))               || // push rXX (REX)
            (p[0] >= 0x50 && p[0] <= 0x57)                                 || // push rXX
            (p[0] == 0x55)                                                 || // push rbp
            (p[0] == 0x4C && p[1] == 0x89);                                   // mov [rsp+x],rXX
        if (prologue) {
            return insideFn - window + j;
        }
    }
    return 0;
}

bool ModuleAnalyzer::DumpExports(const ModuleInfo& mod,
                                 std::vector<ExportEntry>& out) {
    out.clear();
    if (!mod.base) return false;

    IMAGE_DOS_HEADER dos{};
    if (!m_mem.ReadMemory(mod.base, &dos, sizeof(dos))) return false;
    IMAGE_NT_HEADERS64 nt{};
    if (!m_mem.ReadMemory(mod.base + dos.e_lfanew, &nt, sizeof(nt))) return false;

    const auto& dir = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (!dir.VirtualAddress || !dir.Size) return false;

    IMAGE_EXPORT_DIRECTORY exp{};
    if (!m_mem.ReadMemory(mod.base + dir.VirtualAddress, &exp, sizeof(exp))) return false;

    std::vector<uint32_t> funcs(exp.NumberOfFunctions);
    std::vector<uint32_t> names(exp.NumberOfNames);
    std::vector<uint16_t> ords(exp.NumberOfNames);

    if (!funcs.empty())
        m_mem.ReadMemory(mod.base + exp.AddressOfFunctions, funcs.data(),
                         funcs.size() * sizeof(uint32_t));
    if (!names.empty())
        m_mem.ReadMemory(mod.base + exp.AddressOfNames, names.data(),
                         names.size() * sizeof(uint32_t));
    if (!ords.empty())
        m_mem.ReadMemory(mod.base + exp.AddressOfNameOrdinals, ords.data(),
                         ords.size() * sizeof(uint16_t));

    for (DWORD i = 0; i < exp.NumberOfNames; ++i) {
        char name[256] = {};
        if (!m_mem.ReadMemory(mod.base + names[i], name, sizeof(name) - 1)) continue;
        ExportEntry e;
        e.name    = name;
        e.ordinal = static_cast<uint16_t>(ords[i] + exp.Base);
        e.rva     = funcs[ords[i]];
        e.address = mod.base + e.rva;
        out.push_back(std::move(e));
    }
    return true;
}

bool ModuleAnalyzer::DumpInterfaces(const ModuleInfo& mod,
                                    std::vector<InterfaceEntry>& out) {
    out.clear();

    // Find CreateInterface export
    std::vector<ExportEntry> exports;
    if (!DumpExports(mod, exports)) return false;

    uintptr_t pCreateInterface = 0;
    for (const auto& e : exports) {
        if (e.name == "CreateInterface") { pCreateInterface = e.address; break; }
    }
    if (!pCreateInterface) return false;

    // Read first ~64 bytes of CreateInterface and locate
    //   48 8B 05 disp32      ; mov rax, [s_pInterfaceRegs]
    // or
    //   48 8B 0D disp32 / 48 8D 0D disp32
    // The first such RIP-relative load points to the head pointer.
    uint8_t code[0x60] = {};
    if (!m_mem.ReadMemory(pCreateInterface, code, sizeof(code))) return false;

    uintptr_t pHeadVar = 0;
    for (size_t i = 0; i + 7 < sizeof(code); ++i) {
        if (code[i] == 0x48 && (code[i+1] == 0x8B || code[i+1] == 0x8D)) {
            uint8_t modrm = code[i+2];
            if ((modrm & 0xC7) == 0x05) {
                int32_t d = *reinterpret_cast<int32_t*>(&code[i + 3]);
                uintptr_t instr = pCreateInterface + i;
                pHeadVar = instr + 7 + d;
                break;
            }
        }
    }
    if (!pHeadVar) return false;

    // *pHeadVar = first InterfaceReg*
    // Source 2 InterfaceReg layout (verified for CS2):
    //   +0x00  void* (*m_CreateFn)();
    //   +0x08  const char* m_pName;
    //   +0x10  InterfaceReg* m_pNext;
    uintptr_t node = 0;
    if (!m_mem.ReadMemory(pHeadVar, &node, sizeof(node))) return false;

    int safety = 0;
    while (node && safety++ < 4096) {
        struct Reg { uintptr_t fn; uintptr_t name; uintptr_t next; } r{};
        if (!m_mem.ReadMemory(node, &r, sizeof(r))) break;

        char nameBuf[256] = {};
        if (r.name) m_mem.ReadMemory(r.name, nameBuf, sizeof(nameBuf) - 1);

        InterfaceEntry e;
        e.name    = nameBuf;
        e.factory = r.fn;
        e.regNode = node;
        out.push_back(std::move(e));

        node = r.next;
    }
    return !out.empty();
}
