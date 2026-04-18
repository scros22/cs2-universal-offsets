#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstring>

// ============================================
// VMT (Virtual Method Table) Hook
//
// Safer than inline/trampoline hooks (MinHook):
// - Never modifies original function bytes
// - Only swaps a vtable pointer in heap memory
// - VAC does not checksum heap allocations
//
// Usage:
//   VMTHook hook;
//   hook.Setup(pObject, maxIndex);              // copies vtable
//   hook.Hook(index, &MyHookFn);                // overwrites copied entry
//   auto original = hook.GetOriginal<Fn>(index);// get original fn
//   hook.Unhook();                              // restore original vptr
// ============================================

class VMTHook
{
public:
    VMTHook() = default;
    ~VMTHook() { Unhook(); }

    // Setup: copy the vtable from the target object
    // maxIndex: the highest vtable index we plan to hook (we copy maxIndex+1 entries)
    // We add padding beyond that for safety
    bool Setup(void* pObject, size_t maxIndex = 64)
    {
        if (!pObject) return false;

        m_ppObject = reinterpret_cast<uintptr_t**>(pObject);
        m_pOriginalVTable = *m_ppObject;

        if (!m_pOriginalVTable) return false;

        // Use a fixed size: maxIndex + generous padding
        // Don't try to "count" vtable entries — vtables aren't null-terminated
        m_vtableSize = maxIndex + 16;

        // Allocate new vtable with space for RTTI pointer at index [-1]
        // MSVC stores RTTI Complete Object Locator pointer at vtable[-1]
        m_pNewVTable = new uintptr_t[m_vtableSize + 1]; // +1 for RTTI slot
        
        // Copy RTTI entry (vtable[-1])
        m_pNewVTable[0] = m_pOriginalVTable[-1]; // RTTI
        
        // Our usable vtable starts at index 1 in our allocation
        // but we expose it as starting at the right offset
        uintptr_t* newVTableStart = &m_pNewVTable[1];
        
        // Copy all vtable entries
        memcpy(newVTableStart, m_pOriginalVTable, m_vtableSize * sizeof(uintptr_t));

        // Swap the object's vptr to point to our copy (offset by 1 for RTTI)
        *m_ppObject = newVTableStart;

        m_pExposedVTable = newVTableStart;
        m_bHooked = true;
        return true;
    }

    // Hook a specific vtable index
    void Hook(size_t index, void* pHookFn)
    {
        if (!m_bHooked || index >= m_vtableSize || !m_pExposedVTable) return;
        m_pExposedVTable[index] = reinterpret_cast<uintptr_t>(pHookFn);
    }

    // Get original function at index
    template<typename T>
    T GetOriginal(size_t index)
    {
        if (!m_pOriginalVTable || index >= m_vtableSize)
            return nullptr;
        return reinterpret_cast<T>(m_pOriginalVTable[index]);
    }

    // Restore original vtable pointer
    void Unhook()
    {
        if (m_bHooked && m_ppObject && m_pOriginalVTable)
        {
            *m_ppObject = m_pOriginalVTable;
        }

        if (m_pNewVTable)
        {
            delete[] m_pNewVTable;
            m_pNewVTable = nullptr;
        }

        m_pExposedVTable = nullptr;
        m_bHooked = false;
        m_ppObject = nullptr;
        m_pOriginalVTable = nullptr;
        m_vtableSize = 0;
    }

    bool IsHooked() const { return m_bHooked; }

private:
    uintptr_t** m_ppObject = nullptr;        // pointer to the object's vptr
    uintptr_t*  m_pOriginalVTable = nullptr;  // original vtable (never modified)
    uintptr_t*  m_pNewVTable = nullptr;       // raw allocation (includes RTTI at [0])
    uintptr_t*  m_pExposedVTable = nullptr;   // usable vtable start (m_pNewVTable + 1)
    size_t      m_vtableSize = 0;
    bool        m_bHooked = false;
};
