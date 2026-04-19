#include "Memory.h"
#include <iostream>
#include <algorithm>

Memory::Memory() : m_hProcess(nullptr), m_dwProcessId(0) {
}

Memory::~Memory() {
    Detach();
}

bool Memory::Attach(const std::wstring& processName) {
    Detach();
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return false;
    }

    do {
        if (processName == pe32.szExeFile) {
            m_dwProcessId = pe32.th32ProcessID;
            m_processName = processName;
            break;
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (m_dwProcessId == 0) {
        return false;
    }

    m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwProcessId);
    return m_hProcess != nullptr;
}

void Memory::Detach() {
    if (m_hProcess) {
        CloseHandle(m_hProcess);
        m_hProcess = nullptr;
    }
    m_dwProcessId = 0;
    m_processName.clear();
}

bool Memory::ReadMemory(uintptr_t address, void* buffer, size_t size) {
    if (!m_hProcess) {
        return false;
    }

    SIZE_T bytesRead = 0;
    return ReadProcessMemory(m_hProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytesRead) && bytesRead == size;
}

bool Memory::WriteMemory(uintptr_t address, const void* buffer, size_t size) {
    if (!m_hProcess) {
        return false;
    }

    SIZE_T bytesWritten = 0;
    return WriteProcessMemory(m_hProcess, reinterpret_cast<LPVOID>(address), buffer, size, &bytesWritten) && bytesWritten == size;
}

uintptr_t Memory::GetModuleBase(const std::wstring& moduleName) {
    if (!m_hProcess) {
        return 0;
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_dwProcessId);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32W me32;
    me32.dwSize = sizeof(MODULEENTRY32W);

    uintptr_t base = 0;
    if (Module32FirstW(hSnapshot, &me32)) {
        do {
            if (moduleName == me32.szModule) {
                base = reinterpret_cast<uintptr_t>(me32.modBaseAddr);
                break;
            }
        } while (Module32NextW(hSnapshot, &me32));
    }

    CloseHandle(hSnapshot);
    return base;
}

std::vector<MemoryRegion> Memory::GetMemoryRegions() {
    std::vector<MemoryRegion> regions;
    
    if (!m_hProcess) {
        return regions;
    }

    uintptr_t address = 0;
    MEMORY_BASIC_INFORMATION mbi;

    while (VirtualQueryEx(m_hProcess, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT && 
            (mbi.Protect == PAGE_EXECUTE_READ || 
             mbi.Protect == PAGE_EXECUTE_READWRITE ||
             mbi.Protect == PAGE_READONLY ||
             mbi.Protect == PAGE_READWRITE)) {
            
            MemoryRegion region;
            region.base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
            region.size = mbi.RegionSize;
            region.protect = mbi.Protect;
            
            // Try to get module name
            wchar_t moduleName[MAX_PATH] = { 0 };
            if (GetModuleFileNameExW(m_hProcess, reinterpret_cast<HMODULE>(mbi.AllocationBase), moduleName, MAX_PATH)) {
                std::wstring wstr(moduleName);
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
                region.module.resize(size_needed);
                WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &region.module[0], size_needed, NULL, NULL);
            }
            
            regions.push_back(region);
        }
        
        address = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
    }

    return regions;
}

