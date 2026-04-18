#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <map>
#include <mutex>
#include <atomic>
#include "../sdk/game.h"
#include "../sdk/offsets.h"

namespace SkinChanger
{
    struct SkinConfig
    {
        int paintKit = 0;
        float wear = 0.001f;
        int seed = 0;
        int statTrak = -1;
        bool enabled = false;
    };

    #pragma pack(push, 1)
    struct CEconItemAttribute
    {
        uintptr_t vtable;
        uintptr_t owner;
        char pad_0010[32];
        uint16_t defIndex;
        char pad_0032[2];
        float value;
        float initValue;
        int32_t refundableCurrency;
        bool setBonus;
        char pad_0041[7];
    };

    struct CPtrGameVector
    {
        uint64_t size;
        uintptr_t ptr;
    };
    #pragma pack(pop)

    inline std::map<int, SkinConfig> weaponSkins;
    inline std::atomic<bool> forceUpdate = false;
    inline std::atomic<bool> running = false;
    inline bool thirdPerson = false;
    inline uintptr_t regenAddr = 0;
    inline bool regenPatched = false;
    inline std::mutex configMutex;
    inline int tickCounter = 0;
    inline uintptr_t lastAppliedWeapon = 0;
    inline int lastAppliedKit = 0;

    enum WeaponDefIndex : uint16_t
    {
        WEAPON_KNIFE_CT = 42, WEAPON_KNIFE_T = 59,
    };

    inline const char* GetWeaponName(int d) {
        switch (d) {
        case 1: return "Deagle"; case 4: return "Glock"; case 7: return "AK-47";
        case 8: return "AUG"; case 9: return "AWP"; case 10: return "FAMAS";
        case 16: return "M4A4"; case 19: return "P90"; case 32: return "P2000";
        case 42: return "CT Knife"; case 59: return "T Knife";
        case 60: return "M4A1-S"; case 61: return "USP-S";
        default: return "Weapon";
        }
    }

    inline CEconItemAttribute MakeAttribute(uint16_t def, float value)
    {
        CEconItemAttribute attr{};
        attr.defIndex = def;
        attr.value = value;
        attr.initValue = value;
        return attr;
    }

    inline CEconItemAttribute* g_attrBuffer = nullptr;

    inline void CreateAttributes(uintptr_t item, int paintKit, int seed, float wear)
    {
        if (paintKit <= 0) return;

        uintptr_t attrListAddr = item + Offsets::m_AttributeList + Offsets::m_Attributes;
        CPtrGameVector existing = Game::Read<CPtrGameVector>(attrListAddr);
        if (existing.size > 0 || existing.ptr != 0) return;

        if (!g_attrBuffer)
        {
            g_attrBuffer = reinterpret_cast<CEconItemAttribute*>(
                VirtualAlloc(nullptr, sizeof(CEconItemAttribute) * 3,
                    MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            if (!g_attrBuffer) return;
        }

        g_attrBuffer[0] = MakeAttribute(6, static_cast<float>(paintKit));
        g_attrBuffer[1] = MakeAttribute(7, static_cast<float>(seed));
        g_attrBuffer[2] = MakeAttribute(8, wear);

        CPtrGameVector newList;
        newList.size = 3;
        newList.ptr = reinterpret_cast<uintptr_t>(g_attrBuffer);
        Game::Write<CPtrGameVector>(attrListAddr, newList);
    }

    inline void RemoveAttributes(uintptr_t item)
    {
        uintptr_t attrListAddr = item + Offsets::m_AttributeList + Offsets::m_Attributes;
        CPtrGameVector existing = Game::Read<CPtrGameVector>(attrListAddr);
        if (existing.size == 0) return;

        CPtrGameVector empty{};
        Game::Write<CPtrGameVector>(attrListAddr, empty);
    }

    inline void InitRegen()
    {
        if (regenAddr != 0) return;

        HMODULE clientModule = GetModuleHandleW(L"client.dll");
        if (!clientModule) return;
        MODULEINFO modInfo{};
        if (!GetModuleInformation(GetCurrentProcess(), clientModule, &modInfo, sizeof(modInfo))) return;

        const char* sig = "48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10";
        
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

        auto sigBytes = patternToBytes(sig);
        size_t sigSize = sigBytes.size();
        uint8_t* scanBytes = reinterpret_cast<uint8_t*>(clientModule);

        for (size_t i = 0; i < modInfo.SizeOfImage - sigSize; i++)
        {
            bool found = true;
            for (size_t j = 0; j < sigSize; j++)
            {
                if (!sigBytes[j].second && scanBytes[i + j] != sigBytes[j].first)
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                regenAddr = reinterpret_cast<uintptr_t>(&scanBytes[i]);
                break;
            }
        }

        if (regenAddr)
        {
            uint16_t combinedOffset = static_cast<uint16_t>(
                Offsets::m_AttributeManager + Offsets::m_Item +
                Offsets::m_AttributeList + Offsets::m_Attributes
            );

            DWORD oldProtect;
            if (VirtualProtect(reinterpret_cast<void*>(regenAddr + 0x52), 2, PAGE_EXECUTE_READWRITE, &oldProtect))
            {
                *reinterpret_cast<uint16_t*>(regenAddr + 0x52) = combinedOffset;
                VirtualProtect(reinterpret_cast<void*>(regenAddr + 0x52), 2, oldProtect, &oldProtect);
                regenPatched = true;
            }
        }
    }

    inline void CallRegen()
    {
        if (!regenAddr || !regenPatched) return;

        __try {
            typedef void(__fastcall* RegenFn)();
            auto fn = reinterpret_cast<RegenFn>(regenAddr);
            fn();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    inline void ApplyAndRegen(uintptr_t weapon, const SkinConfig& skin, uint16_t defIndex)
    {
        uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;

        uint32_t origItemIDHigh = Game::Read<uint32_t>(item + Offsets::m_iItemIDHigh);

        Game::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
        Game::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, skin.paintKit);
        Game::Write<float>(weapon + Offsets::m_flFallbackWear, skin.wear);
        Game::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, skin.seed);
        Game::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, skin.statTrak);

        CreateAttributes(item, skin.paintKit, skin.seed, skin.wear);
        CallRegen();

        RemoveAttributes(item);
        Game::Write<uint32_t>(item + Offsets::m_iItemIDHigh, origItemIDHigh);
        Game::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, 0);
        Game::Write<float>(weapon + Offsets::m_flFallbackWear, 0.0f);
        Game::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, 0);
        Game::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, -1);
    }

    inline void TickInner()
    {
        tickCounter++;

        uintptr_t localPawn = Game::GetLocalPlayerPawn();
        if (!localPawn)
        {
            lastAppliedWeapon = 0;
            lastAppliedKit = 0;
            return;
        }

        uint8_t lifeState = Game::Read<uint8_t>(localPawn + Offsets::m_lifeState);
        int32_t health = Game::Read<int32_t>(localPawn + Offsets::m_iHealth);

        if (lifeState != 0 || health <= 0)
        {
            lastAppliedWeapon = 0;
            lastAppliedKit = 0;
            return;
        }

        InitRegen();

        bool force = forceUpdate.load();

        std::lock_guard<std::mutex> lock(configMutex);

        uintptr_t activeWeapon = Game::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
        if (activeWeapon)
        {
            uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
            uint16_t defIndex = Game::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);

            bool isWeapon = (defIndex > 0 && defIndex < 70) || (defIndex >= 500 && defIndex < 600);
            if (isWeapon && defIndex != 31)
            {
                int lookupIndex = defIndex;
                if (defIndex == WEAPON_KNIFE_CT || defIndex == WEAPON_KNIFE_T)
                {
                    for (auto& [key, cfg] : weaponSkins)
                        if (key >= 500 && key < 600 && cfg.enabled) { lookupIndex = key; break; }
                }

                auto it = weaponSkins.find(lookupIndex);
                if (it != weaponSkins.end() && it->second.enabled && it->second.paintKit > 0)
                {
                    const SkinConfig& skin = it->second;
                    bool needsApply = force
                        || (activeWeapon != lastAppliedWeapon)
                        || (skin.paintKit != lastAppliedKit);

                    if (needsApply)
                    {
                        lifeState = Game::Read<uint8_t>(localPawn + Offsets::m_lifeState);
                        health = Game::Read<int32_t>(localPawn + Offsets::m_iHealth);
                        if (lifeState == 0 && health > 0)
                        {
                            Game::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0);
                            ApplyAndRegen(activeWeapon, skin, defIndex);
                            lastAppliedWeapon = activeWeapon;
                            lastAppliedKit = skin.paintKit;
                        }
                    }
                }
            }
        }

        if (force) forceUpdate.store(false);
    }

    inline void Tick()
    {
        __try {
            TickInner();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            lastAppliedWeapon = 0;
            lastAppliedKit = 0;
        }
    }
}
