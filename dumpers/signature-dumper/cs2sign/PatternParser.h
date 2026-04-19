#pragma once
//
// PatternParser - converts IDA-style strings ("48 8B ? E8 ? ? ? ?") into the
// (pattern + mask) form the existing SignatureScanner consumes.
//
// Recognised tokens:
//   - "AB"  (two hex chars)         => exact byte
//   - "?"   or "??"                 => wildcard
//
// Whitespace is ignored.  Returns false on malformed input.
//
#include <string>
#include <cctype>

namespace PatternParser {

inline int HexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

inline bool Parse(const char* ida, std::string& outBytes, std::string& outMask) {
    outBytes.clear();
    outMask.clear();
    if (!ida) return false;

    while (*ida) {
        while (*ida == ' ' || *ida == '\t') ++ida;
        if (!*ida) break;

        if (*ida == '?') {
            outBytes.push_back('\x00');
            outMask.push_back('?');
            ++ida;
            if (*ida == '?') ++ida;       // consume '??' as a single wildcard
            continue;
        }

        int hi = HexNibble(ida[0]);
        int lo = HexNibble(ida[1]);
        if (hi < 0 || lo < 0) return false;
        outBytes.push_back(static_cast<char>((hi << 4) | lo));
        outMask.push_back('x');
        ida += 2;
    }
    return !outBytes.empty();
}

} // namespace PatternParser
