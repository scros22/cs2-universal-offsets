#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <cstdint>
#include <vector>
#include <string>
#include <cstdio>
#include "offsets.h"
#include "structs.h"

namespace Game
{
    inline uintptr_t clientBase = 0;
    inline uintptr_t engine2Base = 0;

    namespace GameState
    {
        inline bool originalDormant[65] = { false };
    }

    inline uintptr_t GetModuleBase(const wchar_t* moduleName)
    {
        return reinterpret_cast<uintptr_t>(GetModuleHandleW(moduleName));
    }

    inline bool Init()
    {
        clientBase = GetModuleBase(L"client.dll");
        engine2Base = GetModuleBase(L"engine2.dll");
        return clientBase != 0;
    }

    template<typename T>
    inline T Read(uintptr_t address)
    {
        if (!address) return T{};
        __try {
            return *reinterpret_cast<T*>(address);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return T{};
        }
    }

    template<typename T>
    inline void Write(uintptr_t address, const T& value)
    {
        if (!address) return;
        __try {
            *reinterpret_cast<T*>(address) = value;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    template<typename T>
    inline T SafeRead(uintptr_t address)
    {
        if (!address) return T{};
        __try {
            T result;
            volatile uint8_t* src = reinterpret_cast<volatile uint8_t*>(address);
            uint8_t buf[sizeof(T)];
            for (size_t i = 0; i < sizeof(T); i++)
                buf[i] = src[i];
            memcpy(&result, buf, sizeof(T));
            return result;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return T{};
        }
    }

    template<typename T>
    inline void SafeWrite(uintptr_t address, const T& value)
    {
        if (!address) return;
        __try {
            uint8_t buf[sizeof(T)];
            memcpy(buf, &value, sizeof(T));
            volatile uint8_t* dst = reinterpret_cast<volatile uint8_t*>(address);
            for (size_t i = 0; i < sizeof(T); i++)
                dst[i] = buf[i];
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void SafeWriteFloat(float* dst, float value)
    {
        if (!dst) return;
        __try {
            uint8_t buf[sizeof(float)];
            memcpy(buf, &value, sizeof(float));
            volatile uint8_t* d = reinterpret_cast<volatile uint8_t*>(dst);
            for (size_t i = 0; i < sizeof(float); i++)
                d[i] = buf[i];
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline float SafeReadFloat(float* src)
    {
        if (!src) return 0.f;
        __try {
            volatile uint8_t* s = reinterpret_cast<volatile uint8_t*>(src);
            uint8_t buf[sizeof(float)];
            for (size_t i = 0; i < sizeof(float); i++)
                buf[i] = s[i];
            float result;
            memcpy(&result, buf, sizeof(float));
            return result;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return 0.f;
        }
    }

    inline uintptr_t GetLocalPlayerPawn()
    {
        return Read<uintptr_t>(clientBase + Offsets::dwLocalPlayerPawn);
    }

    inline uintptr_t GetLocalController()
    {
        return Read<uintptr_t>(clientBase + Offsets::dwLocalPlayerController);
    }

    inline uintptr_t GetEntityList()
    {
        return Read<uintptr_t>(clientBase + Offsets::dwEntityList);
    }

    constexpr uintptr_t ENTITY_IDENTITY_SIZE = 0x70;

    inline uintptr_t GetEntityByHandle(uint32_t handle, bool debug = false)
    {
        if (!handle || handle == 0xFFFFFFFF) return 0;

        uintptr_t entityList = GetEntityList();
        if (!entityList) return 0;

        uint32_t entityIndex = handle & 0x7FFF;
        uint32_t chunkIndex = entityIndex >> 9;
        uint32_t entryIndex = entityIndex & 0x1FF;

        uintptr_t chunkAddr = entityList + 0x8 * chunkIndex + 0x10;
        uintptr_t listEntry = Read<uintptr_t>(chunkAddr);

        if (!listEntry) return 0;

        uintptr_t entityAddr = listEntry + ENTITY_IDENTITY_SIZE * entryIndex;
        uintptr_t entity = Read<uintptr_t>(entityAddr);

        return entity;
    }

    // Alias for compatibility
    inline uintptr_t GetEntityFromHandle(uint32_t handle) { return GetEntityByHandle(handle); }

    inline Vector3 GetEntityOrigin(uintptr_t pawn)
    {
        uintptr_t node = Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
        if (!node) return {};
        return Read<Vector3>(node + Offsets::m_vecAbsOrigin);
    }

    inline Vector3 GetBonePosition(uintptr_t pawn, int boneId)
    {
        uintptr_t node = Read<uintptr_t>(pawn + Offsets::m_pGameSceneNode);
        if (!node) return {};

        uintptr_t boneArr = Read<uintptr_t>(node + Offsets::m_modelState + Offsets::m_BoneArray);
        if (!boneArr) return {};

        struct BoneEntry { float px, py, pz, pad, qx, qy, qz, qw; };
        BoneEntry b = Read<BoneEntry>(boneArr + boneId * sizeof(BoneEntry));
        return { b.px, b.py, b.pz };
    }

    inline bool WorldToScreen(const float* worldPos, float& outX, float& outY, float screenW, float screenH)
    {
        if (!clientBase) return false;
        ViewMatrix vm = Read<ViewMatrix>(clientBase + Offsets::dwViewMatrix);

        float w = vm.m[3][0] * worldPos[0] + vm.m[3][1] * worldPos[1] + vm.m[3][2] * worldPos[2] + vm.m[3][3];
        if (w < 0.001f) return false;

        float invW = 1.0f / w;
        float x = vm.m[0][0] * worldPos[0] + vm.m[0][1] * worldPos[1] + vm.m[0][2] * worldPos[2] + vm.m[0][3];
        float y = vm.m[1][0] * worldPos[0] + vm.m[1][1] * worldPos[1] + vm.m[1][2] * worldPos[2] + vm.m[1][3];

        outX = (screenW * 0.5f) + (x * invW * screenW * 0.5f);
        outY = (screenH * 0.5f) - (y * invW * screenH * 0.5f);
        return true;
    }

    // --- Signature Scanner (internal) ---
    inline uintptr_t SigScan(uintptr_t moduleBase, size_t moduleSize, const char* pattern)
    {
        auto patternToBytes = [](const char* pattern) -> std::vector<std::pair<uint8_t, bool>>
        {
            std::vector<std::pair<uint8_t, bool>> bytes;
            const char* start = pattern;
            const char* end = start + strlen(pattern);

            while (start < end)
            {
                if (*start == '?')
                {
                    start++;
                    if (*start == '?') start++;
                    bytes.emplace_back(0, true);
                }
                else
                {
                    bytes.emplace_back(static_cast<uint8_t>(strtoul(start, const_cast<char**>(&start), 16)), false);
                }
                while (*start == ' ') start++;
            }
            return bytes;
        };

        auto sig = patternToBytes(pattern);
        size_t sigSize = sig.size();
        uint8_t* scanBytes = reinterpret_cast<uint8_t*>(moduleBase);

        for (size_t i = 0; i < moduleSize - sigSize; i++)
        {
            bool found = true;
            for (size_t j = 0; j < sigSize; j++)
            {
                if (!sig[j].second && scanBytes[i + j] != sig[j].first)
                {
                    found = false;
                    break;
                }
            }
            if (found)
                return moduleBase + i;
        }
        return 0;
    }

    inline uintptr_t SigScanModule(const wchar_t* moduleName, const char* pattern)
    {
        HMODULE hModule = GetModuleHandleW(moduleName);
        if (!hModule) return 0;

        MODULEINFO modInfo{};
        if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
            return 0;

        return SigScan(reinterpret_cast<uintptr_t>(hModule), modInfo.SizeOfImage, pattern);
    }

    // Helper to change memory protection and write bytes
    inline void PatchBytes(uintptr_t address, const uint8_t* code, size_t size)
    {
        DWORD oldProtect;
        VirtualProtect((void*)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)address, code, size);
        VirtualProtect((void*)address, size, oldProtect, &oldProtect);
    }

    inline uintptr_t FindPattern(const wchar_t* moduleName, const char* pattern)
    {
        uintptr_t moduleBase = GetModuleBase(moduleName);
        if (!moduleBase) return 0;

        IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)moduleBase;
        IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(moduleBase + dosHeader->e_lfanew);
        uintptr_t sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;

        auto patternToByte = [](const char* pattern) {
            std::vector<int> bytes;
            char* start = const_cast<char*>(pattern);
            char* end = const_cast<char*>(pattern) + strlen(pattern);
            for (char* current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?') ++current;
                    bytes.push_back(-1);
                } else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        std::vector<int> patternBytes = patternToByte(pattern);
        uint8_t* scanBytes = reinterpret_cast<uint8_t*>(moduleBase);
        size_t s = patternBytes.size();
        int* d = patternBytes.data();

        for (size_t i = 0; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (size_t j = 0; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return reinterpret_cast<uintptr_t>(&scanBytes[i]);
            }
        }
        return 0;
    }

    // FindPattern variant that takes HMODULE directly
    inline uintptr_t FindPatternInModule(HMODULE hModule, const char* pattern)
    {
        if (!hModule) return 0;

        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(hModule);
        IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)moduleBase;
        IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(moduleBase + dosHeader->e_lfanew);
        uintptr_t sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;

        auto patternToByte = [](const char* pat) {
            std::vector<int> bytes;
            char* start = const_cast<char*>(pat);
            char* end = const_cast<char*>(pat) + strlen(pat);
            for (char* current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?') ++current;
                    bytes.push_back(-1);
                } else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        std::vector<int> patBytes = patternToByte(pattern);
        uint8_t* scanBytes = reinterpret_cast<uint8_t*>(moduleBase);
        size_t s = patBytes.size();
        int* d = patBytes.data();

        for (size_t i = 0; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (size_t j = 0; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) return moduleBase + i;
        }
        return 0;
    }

    inline std::string GetEntityDesignerName(uintptr_t entity)
    {
        if (!entity) return "";
        uintptr_t pEntityIdentity = Read<uintptr_t>(entity + 0x10);
        if (!pEntityIdentity) return "";
        uintptr_t pDesignerName = Read<uintptr_t>(pEntityIdentity + 0x20);
        if (!pDesignerName) return "";
        
        struct StringBuf { char data[64]; };
        StringBuf buf = Read<StringBuf>(pDesignerName);
        buf.data[63] = '\0';
        return std::string(buf.data);
    }

    // --- Weapon Helpers ---
    inline void GetWeaponsDebug(uintptr_t pawn, std::vector<uintptr_t>& weapons, std::vector<uint32_t>& handles)
    {
        weapons.clear();
        handles.clear();
        if (!pawn) return;

        uintptr_t weaponServices = Read<uintptr_t>(pawn + Offsets::m_pWeaponServices);
        if (!weaponServices) return;

        uint64_t weaponCount = Read<uint64_t>(weaponServices + Offsets::m_hMyWeapons);
        uintptr_t weaponData = Read<uintptr_t>(weaponServices + Offsets::m_hMyWeapons + 8);

        if (weaponCount == 0 || weaponCount > 64 || !weaponData) return;

        for (uint64_t i = 0; i < weaponCount; i++)
        {
            uint32_t handle = Read<uint32_t>(weaponData + (i * sizeof(uint32_t)));
            handles.push_back(handle);
            uintptr_t weapon = GetEntityByHandle(handle);
            weapons.push_back(weapon);
        }
    }

    inline std::vector<uintptr_t> GetWeapons(uintptr_t pawn)
    {
        std::vector<uintptr_t> weapons;
        if (!pawn) return weapons;

        uintptr_t weaponServices = Read<uintptr_t>(pawn + Offsets::m_pWeaponServices);
        if (!weaponServices) return weapons;

        uint64_t weaponCount = Read<uint64_t>(weaponServices + Offsets::m_hMyWeapons);
        uintptr_t weaponData = Read<uintptr_t>(weaponServices + Offsets::m_hMyWeapons + 8);

        if (weaponCount == 0 || weaponCount > 64 || !weaponData) return weapons;

        for (uint64_t i = 0; i < weaponCount; i++)
        {
            uint32_t handle = Read<uint32_t>(weaponData + (i * sizeof(uint32_t)));
            uintptr_t weapon = GetEntityByHandle(handle);
            if (weapon)
                weapons.push_back(weapon);
        }

        return weapons;
    }
}
