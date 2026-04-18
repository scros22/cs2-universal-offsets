#pragma once

// ---------------------------------------------------------------
// Skin / Knife / Glove Changer - STABLE VERSION
// Based on working implementation from friend
//
// Key improvements:
// - Only applies to active weapon (no mass iteration)
// - Uses proper attribute system (temporary attributes)
// - Has regeneration function with proper cleanup
// - Caching to prevent unnecessary applications
// - Exception handling for stability
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <map>
#include <mutex>
#include <atomic>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"

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

    // ---------------------------------------------------------------
    // Config and state
    // ---------------------------------------------------------------
    inline std::map<int, SkinConfig> weaponSkins;
    inline std::atomic<bool> forceUpdate = false;
    inline std::atomic<bool> running = false;
    inline std::mutex configMutex;
    
    // Caching to prevent unnecessary applications
    inline uintptr_t lastAppliedWeapon = 0;
    inline int lastAppliedKit = 0;
    inline int tickCounter = 0;
    
    // Regeneration function
    inline uintptr_t regenAddr = 0;
    inline bool regenPatched = false;
    inline CEconItemAttribute* g_attrBuffer = nullptr;

    // Debug info
    inline int dbgWeaponCount = 0;
    inline int dbgSkinsApplied = 0;
    inline int dbgTickCount = 0;

    // ---------------------------------------------------------------
    // Lookup tables
    // ---------------------------------------------------------------
    struct ItemDef {
        int defIndex;
        const char* name;
    };

    inline constexpr ItemDef kKnives[] = {
        { 0,   "Default" },
        { 500, "Bayonet" },
        { 503, "Classic Knife" },
        { 505, "Flip Knife" },
        { 506, "Gut Knife" },
        { 507, "Karambit" },
        { 508, "M9 Bayonet" },
        { 509, "Huntsman Knife" },
        { 512, "Falchion Knife" },
        { 514, "Bowie Knife" },
        { 515, "Butterfly Knife" },
        { 516, "Shadow Daggers" },
        { 517, "Paracord Knife" },
        { 518, "Survival Knife" },
        { 519, "Ursus Knife" },
        { 520, "Navaja Knife" },
        { 521, "Nomad Knife" },
        { 522, "Stiletto Knife" },
        { 523, "Talon Knife" },
        { 525, "Skeleton Knife" },
        { 526, "Kukri Knife" },
    };
    inline constexpr int kKnifeCount = sizeof(kKnives) / sizeof(kKnives[0]);

    struct WeaponDef {
        int defIndex;
        const char* name;
    };

    inline constexpr WeaponDef kWeapons[] = {
        { 1,  "Desert Eagle" },
        { 2,  "Dual Berettas" },
        { 3,  "Five-SeveN" },
        { 4,  "Glock-18" },
        { 7,  "AK-47" },
        { 8,  "AUG" },
        { 9,  "AWP" },
        { 10, "FAMAS" },
        { 13, "Galil AR" },
        { 14, "M249" },
        { 16, "M4A4" },
        { 17, "MAC-10" },
        { 19, "P90" },
        { 23, "MP5-SD" },
        { 24, "UMP-45" },
        { 25, "XM1014" },
        { 26, "PP-Bizon" },
        { 27, "MAG-7" },
        { 28, "Negev" },
        { 29, "Sawed-Off" },
        { 30, "Tec-9" },
        { 32, "P2000" },
        { 33, "MP7" },
        { 34, "MP9" },
        { 35, "Nova" },
        { 36, "P250" },
        { 39, "SG 553" },
        { 40, "SSG 08" },
        { 60, "M4A1-S" },
        { 61, "USP-S" },
        { 63, "CZ75-Auto" },
        { 64, "R8 Revolver" },
    };
    inline constexpr int kWeaponCount = sizeof(kWeapons) / sizeof(kWeapons[0]);

    // ---------------------------------------------------------------
    // UI Config (for menu integration)
    // ---------------------------------------------------------------
    struct Config {
        bool enabled = false;
        int activeWeaponSlot = 0;
        
        // Knife config
        bool knifeEnabled = false;
        int knifeModel = 0;
        int knifePaintKit = 0;
        int knifeSeed = 0;
        float knifeWear = 0.0001f;
        int knifeStatTrak = -1;
    };

    inline Config cfg;

    // ---------------------------------------------------------------
    // Helper functions
    // ---------------------------------------------------------------
    inline const char* GetWeaponName(int defIndex) {
        for (int i = 0; i < kWeaponCount; ++i) {
            if (kWeapons[i].defIndex == defIndex) {
                return kWeapons[i].name;
            }
        }
        switch (defIndex) {
            case 42: return "CT Knife";
            case 59: return "T Knife";
            default: return "Unknown";
        }
    }

    inline bool IsKnife(uint16_t defIndex) {
        return defIndex == 42 || defIndex == 59 || (defIndex >= 500 && defIndex <= 526);
    }

    inline CEconItemAttribute MakeAttribute(uint16_t defIndex, float value) {
        CEconItemAttribute attr{};
        attr.defIndex = defIndex;
        attr.value = value;
        attr.initValue = value;
        return attr;
    }

    // ---------------------------------------------------------------
    // Attribute management (temporary attributes for regeneration)
    // ---------------------------------------------------------------
    inline void CreateAttributes(uintptr_t item, int paintKit, int seed, float wear) {
        if (paintKit <= 0) return;

        uintptr_t attrListAddr = item + Offsets::m_AttributeList + Offsets::m_Attributes;
        CPtrGameVector existing = Mem::Read<CPtrGameVector>(attrListAddr);
        if (existing.size > 0 || existing.ptr != 0) return;

        if (!g_attrBuffer) {
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
        Mem::Write<CPtrGameVector>(attrListAddr, newList);
    }

    inline void RemoveAttributes(uintptr_t item) {
        uintptr_t attrListAddr = item + Offsets::m_AttributeList + Offsets::m_Attributes;
        CPtrGameVector existing = Mem::Read<CPtrGameVector>(attrListAddr);
        if (existing.size == 0) return;

        CPtrGameVector empty{};
        Mem::Write<CPtrGameVector>(attrListAddr, empty);
    }

    // ---------------------------------------------------------------
    // Regeneration function initialization and calling
    // ---------------------------------------------------------------
    inline void InitRegen() {
        if (regenAddr != 0) return;

        if (!GameState::clientBase) return;
        
        // Use our existing pattern finding
        const char* sig = "48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10";
        uintptr_t found = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (found) {
            regenAddr = found;
            
            // Patch the offset for attribute access
            uint16_t combinedOffset = static_cast<uint16_t>(
                Offsets::m_AttributeManager + Offsets::m_Item +
                Offsets::m_AttributeList + Offsets::m_Attributes
            );

            DWORD oldProtect;
            if (VirtualProtect(reinterpret_cast<void*>(regenAddr + 0x52), 2, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                *reinterpret_cast<uint16_t*>(regenAddr + 0x52) = combinedOffset;
                VirtualProtect(reinterpret_cast<void*>(regenAddr + 0x52), 2, oldProtect, &oldProtect);
                regenPatched = true;
            }
        }
    }

    inline void CallRegen() {
        if (!regenAddr || !regenPatched) return;

        __try {
            typedef void(__fastcall* RegenFn)();
            auto fn = reinterpret_cast<RegenFn>(regenAddr);
            fn();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Main application function (stable approach)
    // ---------------------------------------------------------------
    inline void ApplyAndRegen(uintptr_t weapon, const SkinConfig& skin, uint16_t defIndex) {
        uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;

        // Save original values
        uint32_t origItemIDHigh = Mem::Read<uint32_t>(item + Offsets::m_iItemIDHigh);

        // Apply temporary values for regeneration
        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, skin.paintKit);
        Mem::Write<float>(weapon + Offsets::m_flFallbackWear, skin.wear);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, skin.seed);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, skin.statTrak);

        // Create temporary attributes and regenerate
        CreateAttributes(item, skin.paintKit, skin.seed, skin.wear);
        CallRegen();

        // Clean up - remove attributes and restore original values
        RemoveAttributes(item);
        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, origItemIDHigh);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, 0);
        Mem::Write<float>(weapon + Offsets::m_flFallbackWear, 0.0f);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, 0);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, -1);
    }

    // ---------------------------------------------------------------
    // Initialization
    // ---------------------------------------------------------------
    inline void Init() {
        running = true;
        
        // Initialize with some default skins for testing
        weaponSkins[7] = {38, 0.001f, 0, -1, false};   // AK-47 Fade
        weaponSkins[4] = {38, 0.001f, 0, -1, false};   // Glock Fade
        weaponSkins[9] = {344, 0.001f, 0, -1, false};  // AWP Dragon Lore
        
        srand((unsigned)(__rdtsc() & 0xFFFFFFFF));
    }

    // ---------------------------------------------------------------
    // Main tick function (only processes active weapon)
    // ---------------------------------------------------------------
    inline void Tick() {
        if (!cfg.enabled || !running) return;
        if (!GameState::clientBase) return;

        // Sync old config format to new system
        SyncConfigs();

        // Remove SEH to fix C++ compilation
        tickCounter++;
        dbgTickCount = tickCounter;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) {
            lastAppliedWeapon = 0;
            lastAppliedKit = 0;
            return;
        }

        uint8_t lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
        int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);

        if (lifeState != 0 || health <= 0) {
            lastAppliedWeapon = 0;
            lastAppliedKit = 0;
            return;
        }

        InitRegen();

        bool force = forceUpdate.load();
        std::lock_guard<std::mutex> lock(configMutex);

        // Only process ACTIVE weapon (key difference from old approach)
        uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
        if (!activeWeapon) return;

        uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
        uint16_t defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);

        bool isWeapon = (defIndex > 0 && defIndex < 70) || (defIndex >= 500 && defIndex < 600);
        if (!isWeapon || defIndex == 31) return; // Skip grenades

        int lookupIndex = defIndex;
        
        // Handle knives
        if (IsKnife(defIndex) && cfg.knifeEnabled && cfg.knifeModel > 0) {
            lookupIndex = kKnives[cfg.knifeModel].defIndex;
            
            // Apply knife model change
            Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)lookupIndex);
            
            // Create knife skin config
            SkinConfig knifeSkin;
            knifeSkin.enabled = true;
            knifeSkin.paintKit = cfg.knifePaintKit;
            knifeSkin.wear = cfg.knifeWear;
            knifeSkin.seed = cfg.knifeSeed;
            knifeSkin.statTrak = cfg.knifeStatTrak;
            
            bool needsApply = force || (activeWeapon != lastAppliedWeapon) || (knifeSkin.paintKit != lastAppliedKit);
            
            if (needsApply && knifeSkin.paintKit > 0) {
                Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0);
                ApplyAndRegen(activeWeapon, knifeSkin, lookupIndex);
                lastAppliedWeapon = activeWeapon;
                lastAppliedKit = knifeSkin.paintKit;
                dbgSkinsApplied++;
            }
        }
        else {
            // Handle regular weapons
            auto it = weaponSkins.find(lookupIndex);
            if (it != weaponSkins.end() && it->second.enabled && it->second.paintKit > 0) {
                const SkinConfig& skin = it->second;
                bool needsApply = force || (activeWeapon != lastAppliedWeapon) || (skin.paintKit != lastAppliedKit);

                if (needsApply) {
                    // Double-check we're still alive
                    lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
                    health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
                    if (lifeState == 0 && health > 0) {
                        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0);
                        ApplyAndRegen(activeWeapon, skin, defIndex);
                        lastAppliedWeapon = activeWeapon;
                        lastAppliedKit = skin.paintKit;
                        dbgSkinsApplied++;
                    }
                }
            }
        }

        if (force) forceUpdate.store(false);
    }

    // ---------------------------------------------------------------
    // UI Helper functions
    // ---------------------------------------------------------------
    inline void RandomizeAll() {
        std::lock_guard<std::mutex> lock(configMutex);
        
        cfg.enabled = true;
        
        // Popular paint kits
        int kits[] = {12, 38, 44, 77, 135, 279, 309, 344, 409, 415, 417, 418, 433, 475, 524, 597, 637, 735, 811, 846};
        int kitCount = sizeof(kits) / sizeof(kits[0]);
        
        // Randomize weapons
        for (int i = 0; i < kWeaponCount; ++i) {
            int defIndex = kWeapons[i].defIndex;
            weaponSkins[defIndex].enabled = true;
            weaponSkins[defIndex].paintKit = kits[rand() % kitCount];
            weaponSkins[defIndex].seed = rand() % 1000;
            weaponSkins[defIndex].wear = 0.0001f;
            weaponSkins[defIndex].statTrak = (rand() % 4 == 0) ? (rand() % 500) : -1;
        }
        
        // Randomize knife
        cfg.knifeEnabled = true;
        cfg.knifeModel = 1 + (rand() % (kKnifeCount - 1));
        cfg.knifePaintKit = kits[rand() % kitCount];
        cfg.knifeSeed = rand() % 1000;
        cfg.knifeWear = 0.0001f;
        cfg.knifeStatTrak = (rand() % 3 == 0) ? (rand() % 500) : -1;
        
        forceUpdate.store(true);
    }
}
    // ---------------------------------------------------------------
    // Compatibility layer for old menu interface
    // ---------------------------------------------------------------
    
    // Old config structure for menu compatibility
    struct WeaponSkin {
        bool enabled = false;
        int paintKit = 0;
        int seed = 0;
        float wear = 0.0001f;
        int statTrak = -1;
    };
    
    // Add missing variables for menu compatibility
    inline int lastKnifeDefIdx = 0;
    inline float lastGloveSpawnTime = 0.f;
    inline int gloveRefreshFrames = 0;
    
    // Glove definitions for menu
    inline constexpr ItemDef kGloves[] = {
        { 0,    "Default" },
        { 5027, "Sport Gloves" },
        { 5028, "Driver Gloves" },
        { 5029, "Hand Wraps" },
        { 5030, "Moto Gloves" },
        { 5031, "Specialist Gloves" },
        { 5032, "Hydra Gloves" },
        { 5033, "Broken Fang Gloves" },
    };
    inline constexpr int kGloveCount = sizeof(kGloves) / sizeof(kGloves[0]);
    
    // Extend config for menu compatibility
    struct ExtendedConfig : Config {
        WeaponSkin weapons[32];
        bool gloveEnabled = false;
        int gloveModel = 0;
        int glovePaintKit = 10006;
        float gloveWear = 0.0001f;
    };
    
    // Replace cfg with extended version
    #undef cfg
    inline ExtendedConfig cfg;
    
    // Compatibility functions
    inline void ForceFullUpdate() {
        forceUpdate.store(true);
    }
    
    // Sync function to convert old config to new system
    inline void SyncConfigs() {
        std::lock_guard<std::mutex> lock(configMutex);
        
        // Sync weapon configs
        for (int i = 0; i < kWeaponCount && i < 32; ++i) {
            int defIndex = kWeapons[i].defIndex;
            const auto& oldSkin = cfg.weapons[i];
            
            if (oldSkin.enabled) {
                weaponSkins[defIndex] = {
                    oldSkin.paintKit,
                    oldSkin.wear,
                    oldSkin.seed,
                    oldSkin.statTrak,
                    true
                };
            } else {
                weaponSkins[defIndex].enabled = false;
            }
        }
    }