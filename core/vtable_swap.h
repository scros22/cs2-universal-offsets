#pragma once

// ---------------------------------------------------------------
// VMT hook — copies a COM/C++ vtable and swaps the object's vptr.
// Safer than detour/inline patching: never touches .text section,
// operates entirely on heap memory which VAC doesn't checksum.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cstring>

class VTableSwap
{
public:
    VTableSwap() = default;
    ~VTableSwap() { Restore(); }

    bool Init(void* object, size_t tableLen = 64)
    {
        if (!object) return false;
        m_obj      = reinterpret_cast<uintptr_t**>(object);
        m_original = *m_obj;
        if (!m_original) return false;

        m_len = tableLen + 16;
        m_copy = new uintptr_t[m_len + 1];

        // slot 0 = RTTI (index -1 of original)
        m_copy[0] = m_original[-1];

        uintptr_t* usable = &m_copy[1];
        memcpy(usable, m_original, m_len * sizeof(uintptr_t));

        // Use NtProtectVirtualMemory (via syscall if possible) for maximum stealth
        // For now, we use a custom VirtualProtect wrapper to swap the VPtr
        DWORD old;
        if (VirtualProtect(m_obj, sizeof(void*), PAGE_READWRITE, &old))
        {
            *m_obj    = usable;
            VirtualProtect(m_obj, sizeof(void*), old, &old);
        }

        m_exposed = usable;
        m_active  = true;
        return true;
    }

    void Replace(size_t idx, void* fn)
    {
        if (!m_active || idx >= m_len || !m_exposed) return;
        m_exposed[idx] = reinterpret_cast<uintptr_t>(fn);
    }

    template <typename T>
    T Original(size_t idx) const
    {
        if (!m_original || idx >= m_len) return nullptr;
        return reinterpret_cast<T>(m_original[idx]);
    }

    void Restore()
    {
        if (m_active && m_obj && m_original)
            *m_obj = m_original;
        delete[] m_copy;
        m_copy = nullptr;
        m_exposed = nullptr;
        m_active = false;
    }

private:
    uintptr_t** m_obj      = nullptr;
    uintptr_t*  m_original = nullptr;
    uintptr_t*  m_copy     = nullptr;
    uintptr_t*  m_exposed  = nullptr;
    size_t      m_len      = 0;
    bool        m_active   = false;
};
