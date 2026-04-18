#pragma once

// ---------------------------------------------------------------
// CS2 SKINCHANGER V2 - Complete Knife & Glove Support
// Based on br5rhvh.txt signatures (Nuvora 2026)
// 
// KEY FINDINGS:
// - Knives: Need SetModel() call + defIndex write + regen
// - Gloves: Write to m_EconGloves entity + set m_bNeedToReApplyGloves
// - Both: Use same skin application flow as weapons
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <map>
#include <mutex>
#include <atomic>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"

namespace SkinChangerV2
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

    // Knife models
    inline constexpr struct { int defIndex; const char* name; } kKnives[] = {
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

    // Glove models
    inline constexpr struct { int defIndex; const char* name; } kGloves[] = {
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

    // Config
    struct Config {
        bool enabled = false;
        
        // Knife
        bool knifeEnabled = false;
        int knifeModel = 0;
        int knifePaintKit = 0;
        int knifeSeed = 0;
        float knifeWear = 0.0001f;
        int knifeStatTrak = -1;
        
        // Glove
        bool gloveEnabled = false;
        int gloveModel = 0;
        int glovePaintKit = 10006;
        float gloveWear = 0.0001f;
    };

    inline Config cfg;
    inline std::map<int, SkinConfig> weaponSkins;
    inline std::atomic<bool> forceUpdate = false;
    inline std::atomic<bool> running = false;
    inline std::mutex configMutex;
    inline uintptr_t lastAppliedWeapon = 0;
    inline int lastAppliedKit = 0;
    inline CEconItemAttribute* g_attrBuffer = nullptr;

    // Function addresses
    inline uintptr_t regenAddr = 0;
    inline bool regenPatched = false;

    // Helper functions
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

    inline void InitRegen() {
        if (regenAddr != 0) return;
        if (!GameState::clientBase) return;
        
        // RegenerateWeaponSkin signature from br5rhvh.txt
        const char* sig = "40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?";
        uintptr_t found = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (found) {
            regenAddr = found;
            regenPatched = true;
            OutputDebugStringA("[SKINCHANGER] Found RegenerateWeaponSkin\n");
        } else {
            OutputDebugStringA("[SKINCHANGER] RegenerateWeaponSkin NOT FOUND\n");
        }
    }

    inline void CallRegen() {
        if (!regenAddr || !regenPatched) return;
        
        typedef void(__fastcall* RegenFn)();
        auto fn = reinterpret_cast<RegenFn>(regenAddr);
        fn();
    }

    inline void ApplySkin(uintptr_t weapon, const SkinConfig& skin) {
        uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;
        
        uint32_t origItemIDHigh = Mem::Read<uint32_t>(item + Offsets::m_iItemIDHigh);

        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, skin.paintKit);
        Mem::Write<float>(weapon + Offsets::m_flFallbackWear, skin.wear);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, skin.seed);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, skin.statTrak);

        CreateAttributes(item, skin.paintKit, skin.seed, skin.wear);
        CallRegen();

        RemoveAttributes(item);
        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, origItemIDHigh);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, 0);
        Mem::Write<float>(weapon + Offsets::m_flFallbackWear, 0.0f);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, 0);
        Mem::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, -1);
    }

    inline void Tick() {
        if (!cfg.enabled || !running) return;
        if (!GameState::clientBase) return;

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

        // ---------------------------------------------------------------
        // GLOVE CHANGER - Simple approach that works
        // ---------------------------------------------------------------
        if (cfg.gloveEnabled && cfg.gloveModel > 0 && cfg.gloveModel < kGloveCount) {
            uintptr_t gloveEntity = Mem::Read<uintptr_t>(localPawn + Offsets::m_EconGloves);
            if (gloveEntity) {
                uint16_t targetDefIndex = kGloves[cfg.gloveModel].defIndex;
                uintptr_t gloveItem = gloveEntity + Offsets::m_AttributeManager + Offsets::m_Item;
                
                // Write glove model
                Mem::Write<uint16_t>(gloveItem + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                Mem::Write<int32_t>(gloveItem + Offsets::m_iEntityQuality, 3);
                
                // Apply glove skin
                if (cfg.glovePaintKit > 0) {
                    Mem::Write<int32_t>(gloveEntity + Offsets::m_nFallbackPaintKit, cfg.glovePaintKit);
                    Mem::Write<float>(gloveEntity + Offsets::m_flFallbackWear, cfg.gloveWear);
                    Mem::Write<uint32_t>(gloveItem + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                }
                
                // Force reapply
                Mem::Write<bool>(localPawn + Offsets::m_bNeedToReApplyGloves, true);
            }
        }

        // ---------------------------------------------------------------
        // WEAPON/KNIFE CHANGER
        // ---------------------------------------------------------------
        uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
        if (!activeWeapon) return;

        uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
        uint16_t defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);

        bool isWeapon = (defIndex > 0 && defIndex < 70) || (defIndex >= 500 && defIndex < 600);
        if (!isWeapon || defIndex == 31) return;

        int lookupIndex = defIndex;
        
        // Handle knives
        if (IsKnife(defIndex) && cfg.knifeEnabled && cfg.knifeModel > 0 && cfg.knifeModel < kKnifeCount) {
            lookupIndex = kKnives[cfg.knifeModel].defIndex;
            
            // Write knife defIndex
            Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)lookupIndex);
            Mem::Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
            
            // Apply knife skin
            SkinConfig knifeSkin;
            knifeSkin.enabled = true;
            knifeSkin.paintKit = cfg.knifePaintKit;
            knifeSkin.wear = cfg.knifeWear;
            knifeSkin.seed = cfg.knifeSeed;
            knifeSkin.statTrak = cfg.knifeStatTrak;
            
            bool needsApply = force || (activeWeapon != lastAppliedWeapon) || (knifeSkin.paintKit != lastAppliedKit);
            
            if (needsApply && knifeSkin.paintKit > 0) {
                Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0);
                ApplySkin(activeWeapon, knifeSkin);
                lastAppliedWeapon = activeWeapon;
                lastAppliedKit = knifeSkin.paintKit;
            }
        }
        else {
            // Handle regular weapons
            auto it = weaponSkins.find(lookupIndex);
            if (it != weaponSkins.end() && it->second.enabled && it->second.paintKit > 0) {
                const SkinConfig& skin = it->second;
                bool needsApply = force || (activeWeapon != lastAppliedWeapon) || (skin.paintKit != lastAppliedKit);

                if (needsApply) {
                    lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
                    health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
                    if (lifeState == 0 && health > 0) {
                        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0);
                        ApplySkin(activeWeapon, skin);
                        lastAppliedWeapon = activeWeapon;
                        lastAppliedKit = skin.paintKit;
                    }
                }
            }
        }

        if (force) forceUpdate.store(false);
    }

    inline void Init() {
        running = true;
        
        // Initialize with some default skins
        weaponSkins[7] = {38, 0.001f, 0, -1, false};   // AK-47 Fade
        weaponSkins[4] = {38, 0.001f, 0, -1, false};   // Glock Fade
        weaponSkins[9] = {344, 0.001f, 0, -1, false};  // AWP Dragon Lore
        weaponSkins[16] = {279, 0.001f, 0, -1, false}; // M4A4 Asiimov
        
        srand((unsigned)(__rdtsc() & 0xFFFFFFFF));
    }

    inline void RandomizeAll() {
        std::lock_guard<std::mutex> lock(configMutex);
        
        cfg.enabled = true;
        
        int kits[] = {12, 38, 44, 77, 135, 279, 309, 344, 409, 415, 417, 418, 433, 475, 524, 597, 637, 735, 811, 846};
        int kitCount = sizeof(kits) / sizeof(kits[0]);
        
        // Randomize knife
        cfg.knifeEnabled = true;
        cfg.knifeModel = 1 + (rand() % (kKnifeCount - 1));
        cfg.knifePaintKit = kits[rand() % kitCount];
        cfg.knifeSeed = rand() % 1000;
        cfg.knifeWear = 0.0001f;
        cfg.knifeStatTrak = (rand() % 3 == 0) ? (rand() % 500) : -1;
        
        // Randomize gloves
        cfg.gloveEnabled = true;
        cfg.gloveModel = 1 + (rand() % (kGloveCount - 1));
        cfg.glovePaintKit = 10006 + (rand() % 50);
        cfg.gloveWear = 0.0001f;
        
        forceUpdate.store(true);
    }

    inline void ForceFullUpdate() {
        forceUpdate.store(true);
        lastAppliedWeapon = 0;
        lastAppliedKit = 0;
    }
}
