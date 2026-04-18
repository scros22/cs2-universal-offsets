#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// FRAME STAGE NOTIFY HOOK - BETTER TIMING FOR KNIFE CHANGER
// ═══════════════════════════════════════════════════════════════════════════
//
// This hook runs at FRAME_NET_UPDATE_END which is the perfect time to modify
// weapon data before rendering but after network updates. This won't interfere
// with UI or input systems like OnPostDataUpdate did.
//
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../vendor/minhook/include/MinHook.h"
#include "skinchanger_test.h"

namespace FrameStageHook
{
    enum FrameStage
    {
        FRAME_UNDEFINED = -1,
        FRAME_START,
        FRAME_NET_UPDATE_START,
        FRAME_NET_UPDATE_POSTDATAUPDATE_START,
        FRAME_NET_UPDATE_POSTDATAUPDATE_END,
        FRAME_NET_UPDATE_END,
        FRAME_RENDER_START,
        FRAME_RENDER_END
    };
    
    using FrameStageNotifyFn = void(__fastcall*)(void* thisptr, FrameStage stage);
    inline FrameStageNotifyFn Original_FrameStageNotify = nullptr;
    inline uintptr_t hookAddress = 0;
    inline bool initialized = false;
    
    // Track last applied values to avoid spam
    inline uint16_t lastKnifeDefIndex = 0;
    inline uint16_t lastGloveDefIndex = 0;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging helper
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "knife_frame_debug.txt");
        
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
    // Hook implementation
    // ═══════════════════════════════════════════════════════════════════════
    void __fastcall Hook_FrameStageNotify(void* thisptr, FrameStage stage)
    {
        // Only process at FRAME_NET_UPDATE_END - perfect timing for weapon modifications
        if (stage == FRAME_NET_UPDATE_END) {
            uintptr_t localPawn = GameState::GetLocalPawn();
            
            if (localPawn && localPawn > 0x10000 && localPawn < 0x7FFFFFFFFFFF) {
                // Check if player is alive
                uint8_t lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
                int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
                
                if (lifeState == 0 && health > 0) {
                    // ═══════════════════════════════════════════════════════════
                    // KNIFE CHANGER - FRAME_NET_UPDATE_END timing
                    // ═══════════════════════════════════════════════════════════
                    if (SkinChanger::cfg.knifeEnabled && SkinChanger::cfg.knifeModel > 0 && SkinChanger::cfg.knifeModel < SkinChanger::kKnifeCount) {
                        __try {
                            uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
                            
                            if (activeWeapon && activeWeapon > 0x10000 && activeWeapon < 0x7FFFFFFFFFFF) {
                                uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
                                
                                if (item && item > 0x10000 && item < 0x7FFFFFFFFFFF) {
                                    uint16_t defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
                                    
                                    if (defIndex > 0 && defIndex < 10000 && SkinChanger::IsKnife(defIndex)) {
                                        uint16_t targetDefIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
                                        
                                        // Check if we need to update
                                        bool isDefaultKnife = (defIndex == 42 || defIndex == 59);
                                        bool needsUpdate = (defIndex != targetDefIndex) && (isDefaultKnife || lastKnifeDefIndex != targetDefIndex);
                                        
                                        if (needsUpdate) {
                                            // Apply knife changes
                                            Mem::Write<bool>(item + Offsets::m_bInitialized, false);
                                            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                                            Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                            Mem::Write<uint32_t>(activeWeapon + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                            
                                            // Update viewmodel if exists
                                            uint32_t vmHandle = Mem::Read<uint32_t>(localPawn + Offsets::m_hHudModelArms);
                                            if (vmHandle && vmHandle != 0xFFFFFFFF && vmHandle < 0x7FFF) {
                                                uintptr_t vmEntity = GameState::ResolveHandle(vmHandle);
                                                if (vmEntity && vmEntity > 0x10000 && vmEntity < 0x7FFFFFFFFFFF) {
                                                    uintptr_t vmItem = vmEntity + Offsets::m_AttributeManager + Offsets::m_Item;
                                                    if (vmItem && vmItem > 0x10000 && vmItem < 0x7FFFFFFFFFFF) {
                                                        Mem::Write<bool>(vmItem + Offsets::m_bInitialized, false);
                                                        Mem::Write<uint32_t>(vmItem + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                                                        Mem::Write<uint16_t>(vmItem + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                                        Mem::Write<uint32_t>(vmEntity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                                    }
                                                }
                                            }
                                            
                                            lastKnifeDefIndex = targetDefIndex;
                                            Log("[KNIFE] FrameStage: Changed %d -> %d", defIndex, targetDefIndex);
                                        }
                                        
                                        // Always apply skin
                                        Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.knifePaintKit);
                                        Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackSeed, SkinChanger::cfg.knifeSeed);
                                        Mem::Write<float>(activeWeapon + Offsets::m_flFallbackWear, SkinChanger::cfg.knifeWear);
                                        
                                        if (SkinChanger::cfg.knifeStatTrak >= 0) {
                                            Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackStatTrak, SkinChanger::cfg.knifeStatTrak);
                                        }
                                    }
                                }
                            }
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            Log("[KNIFE] Exception in FrameStage knife changer");
                        }
                    }
                    
                    // ═══════════════════════════════════════════════════════════
                    // GLOVE CHANGER - FRAME_NET_UPDATE_END timing
                    // ═══════════════════════════════════════════════════════════
                    if (SkinChanger::cfg.gloveEnabled && SkinChanger::cfg.gloveModel > 0 && SkinChanger::cfg.gloveModel < SkinChanger::kGloveCount) {
                        __try {
                            uintptr_t gloveEntity = Mem::Read<uintptr_t>(localPawn + Offsets::m_EconGloves);
                            
                            if (gloveEntity && gloveEntity > 0x10000 && gloveEntity < 0x7FFFFFFFFFFF) {
                                uintptr_t gloveItem = gloveEntity + Offsets::m_AttributeManager + Offsets::m_Item;
                                
                                if (gloveItem && gloveItem > 0x10000 && gloveItem < 0x7FFFFFFFFFFF) {
                                    uint16_t currentDefIndex = Mem::Read<uint16_t>(gloveItem + Offsets::m_iItemDefinitionIndex);
                                    uint16_t targetDefIndex = SkinChanger::kGloves[SkinChanger::cfg.gloveModel].defIndex;
                                    
                                    if (targetDefIndex > 0 && targetDefIndex < 10000) {
                                        bool needsUpdate = (currentDefIndex != targetDefIndex) && (lastGloveDefIndex != targetDefIndex);
                                        
                                        if (needsUpdate) {
                                            Mem::Write<uint32_t>(gloveItem + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                                            Mem::Write<uint16_t>(gloveItem + Offsets::m_iItemDefinitionIndex, targetDefIndex);
                                            Mem::Write<uint32_t>(gloveEntity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                                            
                                            lastGloveDefIndex = targetDefIndex;
                                            Log("[GLOVE] FrameStage: Changed %d -> %d", currentDefIndex, targetDefIndex);
                                        }
                                        
                                        // Always apply skin
                                        Mem::Write<int32_t>(gloveEntity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.glovePaintKit);
                                        Mem::Write<float>(gloveEntity + Offsets::m_flFallbackWear, SkinChanger::cfg.gloveWear);
                                    }
                                }
                            }
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            Log("[GLOVE] Exception in FrameStage glove changer");
                        }
                    }
                }
            }
        }
        
        // Call original function
        Original_FrameStageNotify(thisptr, stage);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Initialization
    // ═══════════════════════════════════════════════════════════════════════
    inline bool Init()
    {
        if (initialized) return true;
        if (!GameState::clientBase) {
            Log("[FrameStage] ERROR: clientBase is NULL");
            return false;
        }
        
        Log("[FrameStage] Starting initialization...");
        
        // Find FrameStageNotify function - this is a common signature for Source engine
        // We need to find this function in client.dll
        const char* sig = "48 89 5C 24 ? 56 48 83 EC ? 8B F2 48 8B D9 83 FE ?";
        hookAddress = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (!hookAddress) {
            // Try alternative signature
            const char* altSig = "40 53 48 83 EC ? 8B DA 48 8B F1 83 FB ?";
            hookAddress = Mem::FindPatternInModule(GameState::clientBase, altSig);
        }
        
        if (!hookAddress) {
            Log("[FrameStage] ERROR: Failed to find FrameStageNotify function");
            return false;
        }
        
        Log("[FrameStage] Found FrameStageNotify at 0x%llX", hookAddress);
        
        // Create hook
        MH_STATUS status = MH_CreateHook(
            reinterpret_cast<void*>(hookAddress),
            &Hook_FrameStageNotify,
            reinterpret_cast<void**>(&Original_FrameStageNotify)
        );
        
        if (status != MH_OK) {
            Log("[FrameStage] ERROR: MH_CreateHook failed: %d", status);
            return false;
        }
        
        // Enable hook
        status = MH_EnableHook(reinterpret_cast<void*>(hookAddress));
        
        if (status != MH_OK) {
            Log("[FrameStage] ERROR: MH_EnableHook failed: %d", status);
            return false;
        }
        
        Log("[FrameStage] Hook installed successfully!");
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
        Log("[FrameStage] Hook removed");
    }
}