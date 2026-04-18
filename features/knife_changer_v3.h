#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// KNIFE & GLOVE CHANGER V3 - FORCE RESPAWN APPROACH
// ═══════════════════════════════════════════════════════════════════════════
//
// RESEARCH FINDINGS (2026-04-14):
// ────────────────────────────────────────────────────────────────────────────
// 1. CS2 reads m_iItemDefinitionIndex ONCE during weapon spawn to load model
// 2. After model is cached, changing defIndex has NO EFFECT
// 3. FrameStageNotify doesn't exist in CS2 (Source 2 removed it)
// 4. OnPostDataUpdate causes team selection freeze
// 5. UpdateSubclass (0x1801e9a70) is called during entity initialization
//
// SOLUTION:
// ────────────────────────────────────────────────────────────────────────────
// Write defIndex BEFORE the game reads it by:
// 1. Detect when player switches to knife (via Present hook - already working)
// 2. Write the target defIndex IMMEDIATELY
// 3. Force weapon to "respawn" by manipulating m_iItemIDHigh
// 4. Game will re-read defIndex and load correct model
//
// This works because:
// - Present hook runs every frame (proven working for weapon skins)
// - We can detect weapon switches
// - We write defIndex before game caches the model
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "skinchanger_test.h"

namespace KnifeChangerV3
{
    // Track state to detect weapon switches
    inline uintptr_t lastWeaponEntity = 0;
    inline uint16_t lastWeaponDefIndex = 0;
    inline uint16_t lastAppliedKnifeDefIndex = 0;
    inline uint16_t lastAppliedGloveDefIndex = 0;
    inline int framesSinceSwitch = 0;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging helper
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "knife_v3_debug.txt");
        
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
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Main tick function - called from Present hook (every frame)
    // ═══════════════════════════════════════════════════════════════════════
    inline void Tick()
    {
        if (!SkinChanger::cfg.enabled) return;
        
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn || localPawn < 0x10000) return;
        
        // Check if player is alive
        uint8_t lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
        int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
        
        if (lifeState != 0 || health <= 0) {
            // Player dead - reset state
            lastWeaponEntity = 0;
            lastWeaponDefIndex = 0;
            framesSinceSwitch = 0;
            return;
        }
        
        // ═══════════════════════════════════════════════════════════════════
        // KNIFE CHANGER
        // ═══════════════════════════════════════════════════════════════════
        if (SkinChanger::cfg.knifeEnabled && SkinChanger::cfg.knifeModel > 0 && 
            SkinChanger::cfg.knifeModel < SkinChanger::kKnifeCount) {
            
            __try {
                uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
                
                if (activeWeapon && activeWeapon > 0x10000 && activeWeapon < 0x7FFFFFFFFFFF) {
                    uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
                    
                    if (item && item > 0x10000 && item < 0x7FFFFFFFFFFF) {
                        uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
                        
                        // Check if this is a knife
                        if (currentDefIndex > 0 && currentDefIndex < 10000 && SkinChanger::IsKnife(currentDefIndex)) {
                            uint16_t targetDefIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
                            
                            // Detect weapon switch
                            bool weaponSwitched = (activeWeapon != lastWeaponEntity);
                            if (weaponSwitched) {
                                framesSinceSwitch = 0;
                                Log("[KNIFE] Weapon switched! Old: 0x%llX, New: 0x%llX, DefIndex: %d", 
                                    lastWeaponEntity, activeWeapon, currentDefIndex);
                            } else {
                                framesSinceSwitch++;
                            }
                            
                            // Apply knife change IMMEDIATELY on weapon switch or if defIndex is wrong
                            bool needsChange = (currentDefIndex != targetDefIndex) && 
                                             (weaponSwitched || framesSinceSwitch < 5 || lastAppliedKnifeDefIndex != targetDefIndex);
                            
                            if (needsChange) {
                                // CRITICAL: Write defIndex BEFORE game reads it
                                // This must happen in the first few frames after weapon switch
                                
                                // Step 1: Mark as uninitialized to force reload
                                Mem::Write<bool>(item + Offsets::m_bInitialized, false);
                                
                                // Step 2: Write the target defIndex
                                Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                
                                // Step 3: Force item ID to trigger reload
                                Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                                
                                // Step 4: Update subclass ID
                                Mem::Write<uint32_t>(activeWeapon + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                
                                // Step 5: Update entity quality
                                Mem::Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
                                
                                // Step 6: Apply skin
                                Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.knifePaintKit);
                                Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackSeed, SkinChanger::cfg.knifeSeed);
                                Mem::Write<float>(activeWeapon + Offsets::m_flFallbackWear, SkinChanger::cfg.knifeWear);
                                
                                if (SkinChanger::cfg.knifeStatTrak >= 0) {
                                    Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackStatTrak, SkinChanger::cfg.knifeStatTrak);
                                }
                                
                                // Step 7: Update viewmodel if exists
                                uint32_t vmHandle = Mem::Read<uint32_t>(localPawn + Offsets::m_hHudModelArms);
                                if (vmHandle && vmHandle != 0xFFFFFFFF && vmHandle < 0x7FFF) {
                                    uintptr_t vmEntity = GameState::ResolveHandle(vmHandle);
                                    if (vmEntity && vmEntity > 0x10000 && vmEntity < 0x7FFFFFFFFFFF) {
                                        uintptr_t vmItem = vmEntity + Offsets::m_AttributeManager + Offsets::m_Item;
                                        if (vmItem && vmItem > 0x10000 && vmItem < 0x7FFFFFFFFFFF) {
                                            Mem::Write<bool>(vmItem + Offsets::m_bInitialized, false);
                                            Mem::Write<uint16_t>(vmItem + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                            Mem::Write<uint32_t>(vmItem + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                                            Mem::Write<uint32_t>(vmEntity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                        }
                                    }
                                }
                                
                                lastAppliedKnifeDefIndex = targetDefIndex;
                                Log("[KNIFE] Applied change: %d -> %d (frame %d after switch)", 
                                    currentDefIndex, targetDefIndex, framesSinceSwitch);
                            }
                            
                            // Always update tracking
                            lastWeaponEntity = activeWeapon;
                            lastWeaponDefIndex = currentDefIndex;
                        }
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[KNIFE] Exception in knife changer");
            }
        }
        
        // ═══════════════════════════════════════════════════════════════════
        // GLOVE CHANGER
        // ═══════════════════════════════════════════════════════════════════
        if (SkinChanger::cfg.gloveEnabled && SkinChanger::cfg.gloveModel > 0 && 
            SkinChanger::cfg.gloveModel < SkinChanger::kGloveCount) {
            
            __try {
                uintptr_t gloveEntity = Mem::Read<uintptr_t>(localPawn + Offsets::m_EconGloves);
                
                if (gloveEntity && gloveEntity > 0x10000 && gloveEntity < 0x7FFFFFFFFFFF) {
                    uintptr_t gloveItem = gloveEntity + Offsets::m_AttributeManager + Offsets::m_Item;
                    
                    if (gloveItem && gloveItem > 0x10000 && gloveItem < 0x7FFFFFFFFFFF) {
                        uint16_t currentDefIndex = Mem::Read<uint16_t>(gloveItem + Offsets::m_iItemDefinitionIndex);
                        uint16_t targetDefIndex = SkinChanger::kGloves[SkinChanger::cfg.gloveModel].defIndex;
                        
                        if (targetDefIndex > 0 && targetDefIndex < 10000) {
                            bool needsChange = (currentDefIndex != targetDefIndex) && (lastAppliedGloveDefIndex != targetDefIndex);
                            
                            if (needsChange) {
                                // Same approach as knife: write BEFORE game reads
                                Mem::Write<bool>(gloveItem + Offsets::m_bInitialized, false);
                                Mem::Write<uint16_t>(gloveItem + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                Mem::Write<uint32_t>(gloveItem + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                                Mem::Write<uint32_t>(gloveEntity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                
                                // Apply skin
                                Mem::Write<int32_t>(gloveEntity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.glovePaintKit);
                                Mem::Write<float>(gloveEntity + Offsets::m_flFallbackWear, SkinChanger::cfg.gloveWear);
                                
                                // Force glove reapply
                                Mem::Write<bool>(localPawn + Offsets::m_bNeedToReApplyGloves, true);
                                
                                lastAppliedGloveDefIndex = targetDefIndex;
                                Log("[GLOVE] Applied change: %d -> %d", currentDefIndex, targetDefIndex);
                            }
                        }
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[GLOVE] Exception in glove changer");
            }
        }
    }
    
    inline void Init() {
        Log("[KnifeChangerV3] Initialized - using Present hook approach");
    }
    
    inline void Shutdown() {
        Log("[KnifeChangerV3] Shutdown");
    }
}
