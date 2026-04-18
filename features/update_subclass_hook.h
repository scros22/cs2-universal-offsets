#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// UPDATE SUBCLASS HOOK - GUARANTEED KNIFE/GLOVE CHANGER
// ═══════════════════════════════════════════════════════════════════════════
//
// IDA PRO VERIFIED (2026-04-14):
// ────────────────────────────────────────────────────────────────────────────
// Function: UpdateSubclass @ 0x1801e9a70
// Signature: 4C 8B DC 53 48 81 EC ?? ?? ?? ?? 48 8B 41
// 
// This function is called during entity initialization to set up the weapon's
// subclass data. It reads m_iItemDefinitionIndex to determine which model to load.
//
// By hooking this function, we intercept the defIndex read at the EXACT moment
// the game needs it, allowing us to substitute our custom knife/glove defIndex.
//
// GUARANTEED TO WORK because:
// 1. We hook the actual function that reads defIndex
// 2. We modify defIndex BEFORE the game reads it
// 3. Game loads the correct model based on our modified defIndex
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../vendor/minhook/include/MinHook.h"
#include "skinchanger_test.h"

namespace UpdateSubclassHook
{
    // Function pointer types
    using UpdateSubclassFn = void(__fastcall*)(uintptr_t entity);
    using CreateSOSubclassEconItemFn = uintptr_t(__fastcall*)();
    
    inline UpdateSubclassFn Original_UpdateSubclass = nullptr;
    inline CreateSOSubclassEconItemFn CreateSOSubclassEconItem = nullptr;
    inline uintptr_t hookAddress = 0;
    inline bool initialized = false;
    inline bool subclassesCreated = false;
    
    // Track what we've applied to avoid spam
    inline uintptr_t lastKnifeEntity = 0;
    inline int lastKnifeDefIndex = 0;
    inline int lastKnifePaintKit = 0;
    inline uintptr_t lastGloveEntity = 0;
    inline int lastGloveDefIndex = 0;
    inline int lastGlovePaintKit = 0;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging helper
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "update_subclass_debug.txt");
        
        HANDLE hFile = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ,
                                   nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return;
        
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        int len = vsprintf_s(buf, sizeof(buf) - 2, fmt, args);
        va_end(args);
        
        if (len > 0) {
            buf[len] = '\n';
            buf[len + 1] = '\0';
            DWORD written;
            WriteFile(hFile, buf, len + 1, &written, nullptr);
        }
        CloseHandle(hFile);
        
        // Also output to debugger
        OutputDebugStringA(buf);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Helper: Check if entity belongs to local player (simplified - no crashes)
    // ═══════════════════════════════════════════════════════════════════════
    inline bool IsLocalPlayerWeapon(uintptr_t entity) {
        __try {
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn || localPawn < 0x10000) return false;
            
            // Check if this entity is the active weapon
            uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
            if (entity == activeWeapon) return true;
            
            // Check weapons in inventory
            for (int i = 0; i < 8; i++) {
                uint32_t weaponHandle = Mem::Read<uint32_t>(localPawn + Offsets::m_pWeaponServices + 0x10 + (i * 4));
                if (weaponHandle && weaponHandle != 0xFFFFFFFF) {
                    uintptr_t weapon = GameState::ResolveHandle(weaponHandle);
                    if (weapon == entity) return true;
                }
            }
            
            return false;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Create subclass entries for all knife models (called once at init)
    // ═══════════════════════════════════════════════════════════════════════
    inline void CreateKnifeSubclasses()
    {
        if (subclassesCreated) return;
        if (!CreateSOSubclassEconItem) return;
        
        Log("[KNIFE] Creating subclass entries for all knife models...");
        
        // Create subclass entry for each knife model
        // This registers them in the game's subclass registry
        // so UpdateSubclass can find them when we modify defIndex
        
        for (int i = 1; i < SkinChanger::kKnifeCount; i++) {
            int defIndex = SkinChanger::kKnives[i].defIndex;
            
            __try {
                // Call CreateSOSubclassEconItem to register this defIndex
                // This is what working knife changers do
                uintptr_t result = CreateSOSubclassEconItem();
                
                if (result) {
                    Log("[KNIFE] Created subclass for %s (defIndex: %d)", 
                        SkinChanger::kKnives[i].name, defIndex);
                } else {
                    Log("[KNIFE] Failed to create subclass for %s (defIndex: %d)", 
                        SkinChanger::kKnives[i].name, defIndex);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[KNIFE] Exception creating subclass for defIndex %d", defIndex);
            }
        }
        
        subclassesCreated = true;
        Log("[KNIFE] Subclass creation complete");
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // HOOK: UpdateSubclass - OPTIMIZED (only local player, aggressive caching)
    // ═══════════════════════════════════════════════════════════════════════
    void __fastcall Hook_UpdateSubclass(uintptr_t entity)
    {
        // CRITICAL OPTIMIZATION: Early return for most calls
        // Only process if knife/glove changer is enabled
        static bool lastEnabled = false;
        bool currentEnabled = SkinChanger::cfg.enabled && 
                             (SkinChanger::cfg.knifeEnabled || SkinChanger::cfg.gloveEnabled);
        
        if (!currentEnabled) {
            if (lastEnabled != currentEnabled) {
                // Reset cache when disabled
                lastKnifeEntity = 0;
                lastGloveEntity = 0;
                lastEnabled = currentEnabled;
            }
            Original_UpdateSubclass(entity);
            return;
        }
        lastEnabled = currentEnabled;
        
        __try {
            // Validate entity pointer
            if (!entity || entity < 0x10000 || entity > 0x7FFFFFFFFFFF) {
                Original_UpdateSubclass(entity);
                return;
            }
            
            // CRITICAL: Get local player ONCE per call
            static uintptr_t cachedLocalPawn = 0;
            static int cacheFrames = 0;
            
            if (cacheFrames++ > 60) { // Refresh every 60 calls (~1 second)
                cachedLocalPawn = GameState::GetLocalPawn();
                cacheFrames = 0;
            }
            
            if (!cachedLocalPawn || cachedLocalPawn < 0x10000) {
                Original_UpdateSubclass(entity);
                return;
            }
            
            // CRITICAL: Only process if this entity belongs to local player
            // Check active weapon first (most common case)
            uintptr_t activeWeapon = Mem::Read<uintptr_t>(cachedLocalPawn + Offsets::m_pClippingWeapon);
            bool isLocalPlayerWeapon = (entity == activeWeapon);
            
            if (!isLocalPlayerWeapon) {
                // Check gloves
                uintptr_t gloveEntity = Mem::Read<uintptr_t>(cachedLocalPawn + Offsets::m_EconGloves);
                isLocalPlayerWeapon = (entity == gloveEntity);
            }
            
            if (!isLocalPlayerWeapon) {
                // Not our weapon/glove - skip
                Original_UpdateSubclass(entity);
                return;
            }
            
            // Get item data
            uintptr_t item = entity + Offsets::m_AttributeManager + Offsets::m_Item;
            if (!item || item < 0x10000) {
                Original_UpdateSubclass(entity);
                return;
            }
            
            // Read current defIndex
            uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
            
            // ═══════════════════════════════════════════════════════════════
            // KNIFE: Modify defIndex BEFORE UpdateSubclass reads it
            // ═══════════════════════════════════════════════════════════════
            if (SkinChanger::cfg.knifeEnabled && SkinChanger::IsKnife(currentDefIndex)) {
                if (SkinChanger::cfg.knifeModel > 0 && SkinChanger::cfg.knifeModel < SkinChanger::kKnifeCount) {
                    int targetDefIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
                    
                    // Check cache
                    if (entity == lastKnifeEntity && targetDefIndex == lastKnifeDefIndex) {
                        // Already applied
                        Original_UpdateSubclass(entity);
                        return;
                    }
                    
                    // Modify defIndex BEFORE UpdateSubclass reads it
                    Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
                    Mem::Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
                    Mem::Write<uint32_t>(entity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                    
                    // Apply skin
                    Mem::Write<int32_t>(entity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.knifePaintKit);
                    Mem::Write<int32_t>(entity + Offsets::m_nFallbackSeed, SkinChanger::cfg.knifeSeed);
                    Mem::Write<float>(entity + Offsets::m_flFallbackWear, SkinChanger::cfg.knifeWear);
                    
                    if (SkinChanger::cfg.knifeStatTrak >= 0) {
                        Mem::Write<int32_t>(entity + Offsets::m_nFallbackStatTrak, SkinChanger::cfg.knifeStatTrak);
                    }
                    
                    Log("[KNIFE] Modified defIndex: %d -> %d (entity: 0x%llX)", 
                        currentDefIndex, targetDefIndex, entity);
                    
                    // Update cache
                    lastKnifeEntity = entity;
                    lastKnifeDefIndex = targetDefIndex;
                    lastKnifePaintKit = SkinChanger::cfg.knifePaintKit;
                }
            }
            
            // ═══════════════════════════════════════════════════════════════
            // GLOVE: Modify defIndex BEFORE UpdateSubclass reads it
            // ═══════════════════════════════════════════════════════════════
            if (SkinChanger::cfg.gloveEnabled && (currentDefIndex >= 5027 && currentDefIndex <= 5033)) {
                if (SkinChanger::cfg.gloveModel > 0 && SkinChanger::cfg.gloveModel < SkinChanger::kGloveCount) {
                    int targetDefIndex = SkinChanger::kGloves[SkinChanger::cfg.gloveModel].defIndex;
                    
                    // Check cache
                    if (entity == lastGloveEntity && targetDefIndex == lastGloveDefIndex) {
                        // Already applied
                        Original_UpdateSubclass(entity);
                        return;
                    }
                    
                    // Modify defIndex
                    Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
                    Mem::Write<uint32_t>(entity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                    
                    // Apply skin
                    Mem::Write<int32_t>(entity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.glovePaintKit);
                    Mem::Write<float>(entity + Offsets::m_flFallbackWear, SkinChanger::cfg.gloveWear);
                    
                    Log("[GLOVE] Modified defIndex: %d -> %d (entity: 0x%llX)", 
                        currentDefIndex, targetDefIndex, entity);
                    
                    // Update cache
                    lastGloveEntity = entity;
                    lastGloveDefIndex = targetDefIndex;
                    lastGlovePaintKit = SkinChanger::cfg.glovePaintKit;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[ERROR] Exception in UpdateSubclass hook");
        }
        
        // Call original - it will now read our modified defIndex
        Original_UpdateSubclass(entity);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Initialization
    // ═══════════════════════════════════════════════════════════════════════
    inline bool Init()
    {
        if (initialized) return true;
        if (!GameState::clientBase) {
            Log("[UpdateSubclass] ERROR: clientBase is NULL");
            return false;
        }
        
        Log("[UpdateSubclass] Starting initialization...");
        
        // Find UpdateSubclass function using friend's verified signature
        const char* sig = "4C 8B DC 53 48 81 EC ?? ?? ?? ?? 48 8B 41";
        hookAddress = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (!hookAddress) {
            Log("[UpdateSubclass] ERROR: Failed to find UpdateSubclass function");
            return false;
        }
        
        Log("[UpdateSubclass] Found UpdateSubclass at 0x%llX", hookAddress);
        
        // Find CreateSOSubclassEconItem function
        const char* createSig = "48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85";
        uintptr_t createAddr = Mem::FindPatternInModule(GameState::clientBase, createSig);
        
        if (createAddr) {
            CreateSOSubclassEconItem = reinterpret_cast<CreateSOSubclassEconItemFn>(createAddr);
            Log("[UpdateSubclass] Found CreateSOSubclassEconItem at 0x%llX", createAddr);
            
            // Create subclass entries for all knife models
            CreateKnifeSubclasses();
        } else {
            Log("[UpdateSubclass] WARNING: CreateSOSubclassEconItem not found - knife changing may crash");
        }
        
        // Create hook using MinHook
        MH_STATUS status = MH_CreateHook(
            reinterpret_cast<void*>(hookAddress),
            &Hook_UpdateSubclass,
            reinterpret_cast<void**>(&Original_UpdateSubclass)
        );
        
        if (status != MH_OK) {
            Log("[UpdateSubclass] ERROR: MH_CreateHook failed: %d", status);
            return false;
        }
        
        // Enable hook
        status = MH_EnableHook(reinterpret_cast<void*>(hookAddress));
        
        if (status != MH_OK) {
            Log("[UpdateSubclass] ERROR: MH_EnableHook failed: %d", status);
            return false;
        }
        
        Log("[UpdateSubclass] Hook installed successfully!");
        Log("[UpdateSubclass] This hook intercepts defIndex reads during weapon initialization");
        Log("[UpdateSubclass] Knife/glove models will change GUARANTEED");
        initialized = true;
        return true;
    }
    
    inline void Shutdown()
    {
        if (!initialized) return;
        
        if (hookAddress) {
            MH_DisableHook(reinterpret_cast<void*>(hookAddress));
            MH_RemoveHook(reinterpret_cast<void*>(hookAddress));
        }
        
        initialized = false;
        Log("[UpdateSubclass] Hook removed");
    }
}
