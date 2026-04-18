#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// KNIFE CHANGER - FINAL WORKING SOLUTION
// ═══════════════════════════════════════════════════════════════════════════
//
// RESEARCH CONCLUSION (2026-04-14):
// ────────────────────────────────────────────────────────────────────────────
// UpdateSubclass hook works BUT only called during initial spawn, not weapon switch.
// 
// ACTUAL WORKING SOLUTION:
// 1. Write defIndex in Present hook (every frame)
// 2. Force weapon to "respawn" by setting m_iItemIDHigh = -1
// 3. This triggers UpdateSubclass to be called again
// 4. UpdateSubclass hook reads our modified defIndex
// 5. Model changes
//
// This is what Gemini and other working cheats do.
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "skinchanger_test.h"

namespace KnifeChangerFinal
{
    // Track state
    inline uintptr_t lastKnifeEntity = 0;
    inline uint16_t lastAppliedKnifeDefIndex = 0;
    inline uintptr_t lastGloveEntity = 0;
    inline uint16_t lastAppliedGloveDefIndex = 0;
    inline int frameCounter = 0;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "knife_final_debug.txt");
        
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
    // Main tick - called from Present hook
    // ═══════════════════════════════════════════════════════════════════════
    inline void Tick()
    {
        frameCounter++;
        
        // Log every 300 frames (about every 5 seconds at 60fps)
        if (frameCounter % 300 == 0) {
            Log("[DEBUG] Frame %d: enabled=%d, knifeEnabled=%d, knifeModel=%d", 
                frameCounter, SkinChanger::cfg.enabled, SkinChanger::cfg.knifeEnabled, SkinChanger::cfg.knifeModel);
        }
        
        if (!SkinChanger::cfg.enabled) return;
        
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn || localPawn < 0x10000) return;
        
        // Check if alive
        uint8_t lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
        int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
        
        if (lifeState != 0 || health <= 0) {
            lastKnifeEntity = 0;
            lastAppliedKnifeDefIndex = 0;
            return;
        }
        
        // ═══════════════════════════════════════════════════════════════════
        // KNIFE CHANGER - Force respawn approach
        // ═══════════════════════════════════════════════════════════════════
        if (SkinChanger::cfg.knifeEnabled && 
            SkinChanger::cfg.knifeModel > 0 && 
            SkinChanger::cfg.knifeModel < SkinChanger::kKnifeCount) {
            
            __try {
                uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
                
                if (activeWeapon && activeWeapon > 0x10000 && activeWeapon < 0x7FFFFFFFFFFF) {
                    uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
                    
                    if (item && item > 0x10000 && item < 0x7FFFFFFFFFFF) {
                        uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
                        
                        // Log knife detection
                        if (frameCounter % 300 == 0 && currentDefIndex > 0) {
                            Log("[DEBUG] Active weapon defIndex: %d, IsKnife: %d", 
                                currentDefIndex, SkinChanger::IsKnife(currentDefIndex));
                        }
                        
                        if (currentDefIndex > 0 && currentDefIndex < 10000 && SkinChanger::IsKnife(currentDefIndex)) {
                            uint16_t targetDefIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
                            
                            // Check if we need to apply
                            bool weaponChanged = (activeWeapon != lastKnifeEntity);
                            bool defIndexWrong = (currentDefIndex != targetDefIndex);
                            bool notApplied = (lastAppliedKnifeDefIndex != targetDefIndex);
                            
                            if (defIndexWrong && (weaponChanged || notApplied)) {
                                // STEP 1: Write the target defIndex
                                Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                
                                // STEP 2: Force respawn by setting ItemIDHigh to -1
                                // This triggers UpdateSubclass to be called again
                                Mem::Write<int32_t>(item + Offsets::m_iItemIDHigh, -1);
                                
                                // STEP 3: Mark as uninitialized
                                Mem::Write<bool>(item + Offsets::m_bInitialized, false);
                                
                                // STEP 4: Update subclass ID
                                Mem::Write<uint32_t>(activeWeapon + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                
                                // STEP 5: Set quality
                                Mem::Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
                                
                                // STEP 6: Apply skin
                                Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.knifePaintKit);
                                Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackSeed, SkinChanger::cfg.knifeSeed);
                                Mem::Write<float>(activeWeapon + Offsets::m_flFallbackWear, SkinChanger::cfg.knifeWear);
                                
                                if (SkinChanger::cfg.knifeStatTrak >= 0) {
                                    Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackStatTrak, SkinChanger::cfg.knifeStatTrak);
                                }
                                
                                // STEP 7: Update viewmodel
                                uint32_t vmHandle = Mem::Read<uint32_t>(localPawn + Offsets::m_hHudModelArms);
                                if (vmHandle && vmHandle != 0xFFFFFFFF && vmHandle < 0x7FFF) {
                                    uintptr_t vmEntity = GameState::ResolveHandle(vmHandle);
                                    if (vmEntity && vmEntity > 0x10000 && vmEntity < 0x7FFFFFFFFFFF) {
                                        uintptr_t vmItem = vmEntity + Offsets::m_AttributeManager + Offsets::m_Item;
                                        if (vmItem && vmItem > 0x10000 && vmItem < 0x7FFFFFFFFFFF) {
                                            Mem::Write<uint16_t>(vmItem + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                            Mem::Write<int32_t>(vmItem + Offsets::m_iItemIDHigh, -1);
                                            Mem::Write<bool>(vmItem + Offsets::m_bInitialized, false);
                                            Mem::Write<uint32_t>(vmEntity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                        }
                                    }
                                }
                                
                                lastKnifeEntity = activeWeapon;
                                lastAppliedKnifeDefIndex = targetDefIndex;
                                
                                Log("[KNIFE] Applied: %d -> %d (frame %d, entity 0x%llX)", 
                                    currentDefIndex, targetDefIndex, frameCounter, activeWeapon);
                            }
                        }
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[KNIFE] Exception");
            }
        }
        
        // ═══════════════════════════════════════════════════════════════════
        // GLOVE CHANGER
        // ═══════════════════════════════════════════════════════════════════
        if (SkinChanger::cfg.gloveEnabled && 
            SkinChanger::cfg.gloveModel > 0 && 
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
                                Mem::Write<uint16_t>(gloveItem + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                Mem::Write<int32_t>(gloveItem + Offsets::m_iItemIDHigh, -1);
                                Mem::Write<bool>(gloveItem + Offsets::m_bInitialized, false);
                                Mem::Write<uint32_t>(gloveEntity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                
                                Mem::Write<int32_t>(gloveEntity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.glovePaintKit);
                                Mem::Write<float>(gloveEntity + Offsets::m_flFallbackWear, SkinChanger::cfg.gloveWear);
                                
                                Mem::Write<bool>(localPawn + Offsets::m_bNeedToReApplyGloves, true);
                                
                                lastGloveEntity = gloveEntity;
                                lastAppliedGloveDefIndex = targetDefIndex;
                                
                                Log("[GLOVE] Applied: %d -> %d", currentDefIndex, targetDefIndex);
                            }
                        }
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[GLOVE] Exception");
            }
        }
    }
    
    inline void Init() {
        Log("[KnifeChangerFinal] Initialized - using Present hook + forced respawn");
    }
    
    inline void Shutdown() {
        Log("[KnifeChangerFinal] Shutdown");
    }
}
