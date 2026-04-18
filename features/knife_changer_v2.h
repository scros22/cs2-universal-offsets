#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// KNIFE CHANGER V2 - INTERCEPT APPROACH
// ═══════════════════════════════════════════════════════════════════════════
//
// DISCOVERY: CS2 reads m_iItemDefinitionIndex when creating/equipping weapons
// to determine which model to load. Once loaded, the model is cached.
//
// SOLUTION: Hook the function that READS defIndex and return our custom value
// BEFORE the model is loaded, not after.
//
// This is similar to how Gemini and other working cheats do it - they intercept
// the defIndex read during weapon creation, not modify it after creation.
//
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../vendor/minhook/include/MinHook.h"
#include "skinchanger_test.h"

namespace KnifeChangerV2
{
    // ═══════════════════════════════════════════════════════════════════════
    // The key insight: We need to hook where CS2 READS the defIndex to decide
    // which weapon model to load. This happens in the weapon creation/equip code.
    // ═══════════════════════════════════════════════════════════════════════
    
    inline bool initialized = false;
    inline uintptr_t lastProcessedWeapon = 0;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Helper: Get the defIndex we want for this weapon
    // ═══════════════════════════════════════════════════════════════════════
    inline uint16_t GetDesiredDefIndex(uintptr_t weaponEntity, uint16_t originalDefIndex)
    {
        // Only modify knives
        if (!SkinChanger::IsKnife(originalDefIndex)) {
            return originalDefIndex;
        }
        
        // Check if knife changer is enabled
        if (!SkinChanger::cfg.knifeEnabled || SkinChanger::cfg.knifeModel <= 0 || SkinChanger::cfg.knifeModel >= SkinChanger::kKnifeCount) {
            return originalDefIndex;
        }
        
        // Return the desired knife defIndex
        return SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // APPROACH: Patch the m_iItemDefinitionIndex memory location itself
    // 
    // Instead of hooking functions, we directly modify the defIndex in memory
    // at the EXACT moment when CS2 is about to read it for model loading.
    // 
    // This is done in the FrameStageNotify or similar early hook.
    // ═══════════════════════════════════════════════════════════════════════
    
    inline void ProcessWeapon(uintptr_t weaponEntity)
    {
        if (!weaponEntity || weaponEntity == lastProcessedWeapon) {
            return;
        }
        
        __try {
            uintptr_t item = weaponEntity + Offsets::m_AttributeManager + Offsets::m_Item;
            if (!item || item < 0x10000) return;
            
            // Read current defIndex
            uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
            
            // Check if it's a knife
            if (!SkinChanger::IsKnife(currentDefIndex)) {
                return;
            }
            
            // Get desired defIndex
            uint16_t desiredDefIndex = GetDesiredDefIndex(weaponEntity, currentDefIndex);
            
            // If it's different, write it IMMEDIATELY
            if (currentDefIndex != desiredDefIndex) {
                // Write the new defIndex
                Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, desiredDefIndex);
                Mem::Write<uint32_t>(weaponEntity + Offsets::m_nSubclassID, (uint32_t)desiredDefIndex);
                
                // Mark as not initialized to force reload
                Mem::Write<bool>(item + Offsets::m_bInitialized, false);
                Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                
                lastProcessedWeapon = weaponEntity;
                
                OutputDebugStringA("[KnifeV2] Changed defIndex before model load\n");
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            OutputDebugStringA("[KnifeV2] Exception in ProcessWeapon\n");
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Main tick function - called VERY EARLY in the frame
    // ═══════════════════════════════════════════════════════════════════════
    inline void Tick()
    {
        if (!SkinChanger::cfg.knifeEnabled) {
            return;
        }
        
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn || localPawn < 0x10000) {
            lastProcessedWeapon = 0;
            return;
        }
        
        // Process all weapons in inventory
        auto weapons = GameState::GetWeapons(localPawn);
        for (uintptr_t weapon : weapons) {
            ProcessWeapon(weapon);
        }
        
        // Also process active weapon
        uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
        if (activeWeapon && activeWeapon > 0x10000) {
            ProcessWeapon(activeWeapon);
        }
    }
    
    inline bool Init()
    {
        if (initialized) return true;
        
        OutputDebugStringA("[KnifeV2] Initialized - using intercept approach\n");
        initialized = true;
        return true;
    }
    
    inline void Shutdown()
    {
        initialized = false;
        lastProcessedWeapon = 0;
    }
}
