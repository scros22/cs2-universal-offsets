#include "EnhancedScanner.h"
#include "PatternParser.h"
#include "Console.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>

EnhancedScanner::CachedModule& EnhancedScanner::Cache(const std::wstring& mod) {
    for (auto& c : m_cache) if (c.info.name == mod) return c;
    m_cache.push_back({});
    auto& c = m_cache.back();
    if (!m_ana.Load(mod, c.info)) return c;
    for (const auto& s : c.info.sections) {
        if (s.name == ".text") { c.text = &s; break; }
    }
    if (c.text) {
        c.textBytes.assign(c.text->size, 0);
        c.loaded = m_mem.ReadMemory(c.text->start, c.textBytes.data(), c.text->size);
    }
    return c;
}

uintptr_t EnhancedScanner::SearchBytes(const uint8_t* hay, size_t hayLen,
                                       const std::string& bytes,
                                       const std::string& mask) {
    const size_t need = bytes.size();
    if (!need || hayLen < need) return 0;
    const uint8_t firstByte = static_cast<uint8_t>(bytes[0]);
    const bool firstWild = mask[0] == '?';
    const size_t end = hayLen - need;
    for (size_t i = 0; i <= end; ++i) {
        if (!firstWild && hay[i] != firstByte) continue;
        bool ok = true;
        for (size_t j = 1; j < need; ++j) {
            if (mask[j] == '?') continue;
            if (hay[i + j] != static_cast<uint8_t>(bytes[j])) { ok = false; break; }
        }
        if (ok) return i;
    }
    return 0;
}

bool EnhancedScanner::ScanOne(EnhancedSig& s) {
    if (s.module.empty()) s.module = L"client.dll";
    auto& c = Cache(s.module);
    if (!c.loaded || !c.text) {
        s.error = "module/.text not loaded";
        return false;
    }

    // ---- StringRef path ---------------------------------------------------
    if (s.resolve == ResolveKind::StringRef) {
        auto strs = m_ana.FindStringRefs(c.info, s.ida);
        if (strs.empty()) { s.error = "string not found in .rdata"; return false; }
        for (uintptr_t strVA : strs) {
            auto refs = m_ana.FindRefsTo(c.info, strVA);
            for (uintptr_t r : refs) {
                uintptr_t fn = m_ana.FindFunctionStart(c.info, r);
                if (fn) {
                    s.matchVA    = r;
                    s.resolvedVA = fn + s.extraOff;
                    s.found      = true;
                    return true;
                }
            }
        }
        s.error = "string had no .text refs near a function start";
        return false;
    }

    // ---- Pattern path -----------------------------------------------------
    std::string bytes, mask;
    if (!PatternParser::Parse(s.ida.c_str(), bytes, mask)) {
        s.error = "bad IDA pattern";
        return false;
    }
    uintptr_t off = SearchBytes(c.textBytes.data(), c.textBytes.size(), bytes, mask);
    if (!off && bytes != "") {
        // Some sigs (e.g. globals) live in .rdata - try other readable secs.
        for (const auto& sec : c.info.sections) {
            if (sec.executable || !sec.readable || &sec == c.text) continue;
            std::vector<uint8_t> buf;
            if (!m_ana.ReadSection(sec, buf)) continue;
            uintptr_t lo = SearchBytes(buf.data(), buf.size(), bytes, mask);
            if (lo) { s.matchVA = sec.start + lo; break; }
        }
        if (!s.matchVA) { s.error = "pattern not found"; return false; }
    } else {
        s.matchVA = c.text->start + off;
    }
    s.found = true;
    return ResolveOne(s);
}

bool EnhancedScanner::ResolveOne(EnhancedSig& s) {
    if (s.resolve == ResolveKind::None) {
        s.resolvedVA = s.matchVA + s.extraOff;
        return true;
    }
    if (s.resolve == ResolveKind::Rel32) {
        int32_t d = 0;
        if (!m_mem.ReadMemory(s.matchVA + s.relOffset, &d, 4)) {
            s.error = "rel32 read failed"; return false;
        }
        s.resolvedVA = s.matchVA + s.relOffset + 4 + (intptr_t)d + s.extraOff;
        return true;
    }
    if (s.resolve == ResolveKind::RipRel) {
        int32_t d = 0;
        if (!m_mem.ReadMemory(s.matchVA + s.relOffset, &d, 4)) {
            s.error = "riprel read failed"; return false;
        }
        s.resolvedVA = s.matchVA + s.relOffset + 4 + (intptr_t)d + s.extraOff;
        return true;
    }
    return true;
}

size_t EnhancedScanner::FoundCount() const {
    size_t n = 0; for (const auto& s : m_sigs) if (s.found) ++n; return n;
}

bool EnhancedScanner::Run(const std::string& jsonOut) {
    Console::PrintHeader(L"Enhanced Scan");
    for (size_t i = 0; i < m_sigs.size(); ++i) {
        auto& s = m_sigs[i];
        std::wstring wn(s.name.begin(), s.name.end());
        Console::PrintProgress(i + 1, m_sigs.size(), wn);
        ScanOne(s);
        Console::ClearLine();
        if (s.found) {
            Console::PrintFound(wn, s.resolvedVA ? s.resolvedVA : s.matchVA, 1, 0);
        } else {
            Console::PrintNotFound(wn, s.error);
        }
    }
    Console::ClearLine();

    // Bonus passes - for every module we touched, dump exports + interfaces
    Console::PrintHeader(L"Module Enumeration");
    std::set<std::wstring> mods;
    for (const auto& c : m_cache) if (c.loaded) mods.insert(c.info.name);

    for (const auto& m : mods) {
        ModuleInfo mi;
        if (!m_ana.Load(m, mi)) continue;
        ModuleDump md; md.module = m;
        m_ana.DumpExports(mi, md.exports);
        m_ana.DumpInterfaces(mi, md.interfaces);
        std::wstring line = m + L": " +
            std::to_wstring(md.exports.size()) + L" exports, " +
            std::to_wstring(md.interfaces.size()) + L" interfaces";
        Console::PrintInfo(line);
        m_moduleDumps.push_back(std::move(md));
    }

    DumpJson(jsonOut);
    Console::PrintSuccess(L"Wrote " + std::wstring(jsonOut.begin(), jsonOut.end()));
    return true;
}

static std::string EscapeJ(const std::string& in) {
    std::string out; out.reserve(in.size());
    for (char c : in) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8]; sprintf_s(buf, "\\u%04x", c); out += buf;
                } else out += c;
        }
    }
    return out;
}
static std::string ToHex(uintptr_t v) {
    std::ostringstream o; o << "0x" << std::hex << std::uppercase << v; return o.str();
}
static std::string ResolveName(ResolveKind k) {
    switch (k) {
        case ResolveKind::None:      return "raw";
        case ResolveKind::Rel32:     return "rel32";
        case ResolveKind::RipRel:    return "riprel";
        case ResolveKind::StringRef: return "stringref";
    }
    return "?";
}

void EnhancedScanner::DumpJson(const std::string& path) {
    std::ofstream f(path);
    if (!f) return;
    f << "{\n  \"game\": \"Counter-Strike 2\",\n";
    f << "  \"signatures\": [\n";
    for (size_t i = 0; i < m_sigs.size(); ++i) {
        const auto& s = m_sigs[i];
        std::string mod(s.module.begin(), s.module.end());
        f << "    { \"name\": \"" << EscapeJ(s.name) << "\""
          << ", \"module\": \"" << EscapeJ(mod) << "\""
          << ", \"resolve\": \"" << ResolveName(s.resolve) << "\""
          << ", \"pattern\": \"" << EscapeJ(s.ida) << "\""
          << ", \"found\": " << (s.found ? "true" : "false");
        if (s.found) {
            f << ", \"match\": \"" << ToHex(s.matchVA) << "\""
              << ", \"address\": \"" << ToHex(s.resolvedVA ? s.resolvedVA : s.matchVA) << "\"";
        } else {
            f << ", \"error\": \"" << EscapeJ(s.error) << "\"";
        }
        f << " }" << (i + 1 < m_sigs.size() ? "," : "") << "\n";
    }
    f << "  ],\n  \"modules\": [\n";
    for (size_t i = 0; i < m_moduleDumps.size(); ++i) {
        const auto& md = m_moduleDumps[i];
        std::string mod(md.module.begin(), md.module.end());
        f << "    { \"name\": \"" << EscapeJ(mod) << "\",\n";
        f << "      \"interfaces\": [\n";
        for (size_t j = 0; j < md.interfaces.size(); ++j) {
            const auto& it = md.interfaces[j];
            f << "        { \"name\": \"" << EscapeJ(it.name) << "\""
              << ", \"factory\": \"" << ToHex(it.factory) << "\" }"
              << (j + 1 < md.interfaces.size() ? "," : "") << "\n";
        }
        f << "      ],\n      \"exports\": [\n";
        for (size_t j = 0; j < md.exports.size(); ++j) {
            const auto& e = md.exports[j];
            f << "        { \"name\": \"" << EscapeJ(e.name) << "\""
              << ", \"address\": \"" << ToHex(e.address) << "\""
              << ", \"rva\": \"" << ToHex(e.rva) << "\""
              << ", \"ordinal\": " << e.ordinal << " }"
              << (j + 1 < md.exports.size() ? "," : "") << "\n";
        }
        f << "      ]\n    }" << (i + 1 < m_moduleDumps.size() ? "," : "") << "\n";
    }
    f << "  ],\n  \"summary\": { \"total\": " << m_sigs.size()
      << ", \"found\": " << FoundCount() << " }\n}\n";
}
