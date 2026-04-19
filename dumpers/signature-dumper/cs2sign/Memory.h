#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>
#include <string>

struct MemoryRegion {
    uintptr_t base;
    size_t size;
    DWORD protect;
    std::string module;
};

class Memory {
public:
    Memory();
    ~Memory();

    bool Attach(const std::wstring& processName);
    void Detach();
    
    bool ReadMemory(uintptr_t address, void* buffer, size_t size);
    bool WriteMemory(uintptr_t address, const void* buffer, size_t size);
    
    uintptr_t GetModuleBase(const std::wstring& moduleName);
    std::vector<MemoryRegion> GetMemoryRegions();
    
    bool IsValid() const { return m_hProcess != nullptr; }
    DWORD GetProcessId() const { return m_dwProcessId; }
    HANDLE GetHandle() const { return m_hProcess; }

private:
    HANDLE m_hProcess;
    DWORD m_dwProcessId;
    std::wstring m_processName;
};

