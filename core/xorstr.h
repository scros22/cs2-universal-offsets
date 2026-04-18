#pragma once

// ---------------------------------------------------------------
// xorstr.h — Compile-time string encryption
//
// XOR-encrypts string literals at compile time so plaintext never
// appears in .rdata. Decryption happens at runtime on the stack.
//
// Usage:
//   printf(XS("secret string"));     // narrow
//   GetModuleHandleW(XW(L"ntdll.dll")); // wide
//
// The decrypted string lives as a stack temporary — valid until
// the end of the full expression (the semicolon). Do NOT store
// the pointer across statements.
// ---------------------------------------------------------------

#include <cstdint>
#include <cstddef>

namespace xor_detail
{
    // --- Narrow (char) ---

    template <size_t N>
    struct EncA
    {
        char data[N]{};
        uint8_t key;
        static constexpr size_t len = N;
    };

    template <size_t N>
    consteval EncA<N> enc_a(const char (&s)[N], uint8_t key)
    {
        EncA<N> e{};
        e.key = key;
        for (size_t i = 0; i < N; ++i)
            e.data[i] = static_cast<char>(s[i] ^ static_cast<char>((key + i * 37u) & 0xFF));
        return e;
    }

    template <size_t N>
    struct DecA
    {
        char buf[N];

        __forceinline DecA(const EncA<N>& e)
        {
            volatile uint8_t k = e.key;
            for (size_t i = 0; i < N; ++i)
                buf[i] = static_cast<char>(e.data[i] ^ static_cast<char>((k + i * 37u) & 0xFF));
        }

        __forceinline operator const char* () const { return buf; }
        __forceinline operator char* ()             { return buf; }
    };

    // --- Wide (wchar_t) ---

    template <size_t N>
    struct EncW
    {
        wchar_t data[N]{};
        uint8_t key;
        static constexpr size_t len = N;
    };

    template <size_t N>
    consteval EncW<N> enc_w(const wchar_t (&s)[N], uint8_t key)
    {
        EncW<N> e{};
        e.key = key;
        for (size_t i = 0; i < N; ++i)
            e.data[i] = static_cast<wchar_t>(s[i] ^ static_cast<wchar_t>((key + i * 37u) & 0xFF));
        return e;
    }

    template <size_t N>
    struct DecW
    {
        wchar_t buf[N];

        __forceinline DecW(const EncW<N>& e)
        {
            volatile uint8_t k = e.key;
            for (size_t i = 0; i < N; ++i)
                buf[i] = static_cast<wchar_t>(e.data[i] ^ static_cast<wchar_t>((k + i * 37u) & 0xFF));
        }

        __forceinline operator const wchar_t* () const { return buf; }
        __forceinline operator wchar_t* ()             { return buf; }
    };
}

// XS("text") — encrypted narrow string (const char*)
// Decrypts on the stack; valid until end of full expression.
#define XS(str) ([&]() -> ::xor_detail::DecA<sizeof(str)> { \
    constexpr auto _e = ::xor_detail::enc_a(str, \
        static_cast<uint8_t>((__LINE__ * 7u + __COUNTER__ * 13u) & 0xFFu)); \
    return ::xor_detail::DecA<sizeof(str)>(_e); \
}())

// XW(L"text") — encrypted wide string (const wchar_t*)
// Decrypts on the stack; valid until end of full expression.
#define XW(str) ([&]() -> ::xor_detail::DecW<sizeof(str) / sizeof(wchar_t)> { \
    constexpr auto _e = ::xor_detail::enc_w(str, \
        static_cast<uint8_t>((__LINE__ * 7u + __COUNTER__ * 13u) & 0xFFu)); \
    return ::xor_detail::DecW<sizeof(str) / sizeof(wchar_t)>(_e); \
}())
