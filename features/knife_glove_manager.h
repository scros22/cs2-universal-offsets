#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// KNIFE/GLOVE MANAGER - FRAME-BASED MODIFICATION (NO LAG APPROACH)
// ═══════════════════════════════════════════════════════════════════════════
//
// This manager works in conjunction with EquipItemInLoadout hook to provide
// lag-free knife/glove changing.
//
// STRATEGY:
// ────────────────────────────────────────────────────────────────────────────
// 1. EquipItemInLoadout hook intercepts weapon switches (called ONCE per switch)
// 2. This manager applies modifications in Present hook (once per frame)
// 3. Modifications are applied ONLY to active weapon (not all entities)
// 4. Caching prevents redundant writes
//
// WHY THIS WORKS:
// ────────────────────────────────────────────────────────────────────────────
// - No high-frequency function hooks (UpdateSubclass/SetModel)
// - Modifications applied once per frame (60 FPS = 60 writes/sec, not 1000+)
// - Only processes active weapon (not all entities in game)
// - Caching prevents redundant writes
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include <atomic>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "skinchanger_test.h"

namespace KnifeGloveManager
{
    // State tracking
    inline std::atomic<bool> needsUpdate{false};
    inline uintptr_t lastKnifeEntity = 0;
    inline int lastKnifeDefIndex = 0;
    inline int lastKnifePaintKit = 0;
    inline uintptr_t lastGloveEntity = 0;
    inline int lastGloveDefIndex = 0;
    inline int lastGlovePaintKit = 0;
    
    // Frame counter for periodic checks
    inline int frameCounter = 0;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging helper
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "knife_glove_manager.txt");
        
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
        OutputDebugStringA(buf);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Apply knife modifications (called once per frame)
    // ═══════════════════════════════════════════════════════════════════════
    inline void ApplyKnifeModifications(uintptr_t localPawn)
    {
        if (!SkinChanger::cfg.knifeEnabled) return;
        if (SkinChanger::cfg.knifeModel <= 0 || SkinChanger::cfg.knifeModel >= SkinChanger::kKnifeCount) return;
        
        // Ensure function pointers are initialized
        if (!SkinChanger::UpdateSubclass || !SkinChanger::SetModel || !SkinChanger::SetMeshGroupMask) {
            // Try to initialize them
            if (!GameState::clientBase) return;
            
            if (!SkinChanger::UpdateSubclass) {
                const char* sig = "4C 8B DC 53 48 81 EC ?? ?? ?? ?? 48 8B 41";
                uintptr_t addr = Mem::FindPatternInModule(GameState::clientBase, sig);
                if (addr) SkinChanger::UpdateSubclass = reinterpret_cast<SkinChanger::UpdateSubclassFn>(addr);
            }
            
            if (!SkinChanger::SetModel) {
                const char* sig = "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40";
                uintptr_t addr = Mem::FindPatternInModule(GameState::clientBase, sig);
                if (addr) SkinChanger::SetModel = reinterpret_cast<SkinChanger::SetModelFn>(addr);
            }
            
            if (!SkinChanger::SetMeshGroupMask) {
                const char* sig = "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8D 99";
                uintptr_t addr = Mem::FindPatternInModule(GameState::clientBase, sig);
                if (addr) SkinChanger::SetMeshGroupMask = reinterpret_cast<SkinChanger::SetMeshGroupMaskFn>(addr);
            }
            
            if (SkinChanger::UpdateSubclass && SkinChanger::SetModel && SkinChanger::SetMeshGroupMask) {
                Log("[KNIFE] Function pointers initialized: UpdateSubclass=0x%llX, SetModel=0x%llX, SetMeshGroupMask=0x%llX",
                    (uintptr_t)SkinChanger::UpdateSubclass, (uintptr_t)SkinChanger::SetModel, (uintptr_t)SkinChanger::SetMeshGroupMask);
            }
        }
        
        __try {
            // Get active weapon
            uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
            if (!activeWeapon || activeWeapon < 0x10000) return;
            
            // Get item data
            uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
            if (!item || item < 0x10000) return;
            
            // Read current defIndex
            uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
            
            // Only process knives
            if (!SkinChanger::IsKnife(currentDefIndex)) return;
            
            // Get target defIndex
            int targetDefIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
            
            // Check if already applied (avoid redundant writes)
            if (activeWeapon == lastKnifeEntity && 
                targetDefIndex == lastKnifeDefIndex && 
                SkinChanger::cfg.knifePaintKit == lastKnifePaintKit) {
                return; // Already applied
            }
            
            Log("[KNIFE] Applying modifications: %d -> %d (entity: 0x%llX)", 
                currentDefIndex, targetDefIndex, activeWeapon);
            
            // Modify item definition
            Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
            Mem::Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
            
            // Write subclass ID
            Mem::Write<uint32_t>(activeWeapon + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
            
            // Apply skin via fallback system
            Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.knifePaintKit);
            Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackSeed, SkinChanger::cfg.knifeSeed);
            Mem::Write<float>(activeWeapon + Offsets::m_flFallbackWear, SkinChanger::cfg.knifeWear);
            
            if (SkinChanger::cfg.knifeStatTrak >= 0) {
                Mem::Write<int32_t>(activeWeapon + Offsets::m_nFallbackStatTrak, SkinChanger::cfg.knifeStatTrak);
            }
            
            // Force weapon to "respawn" by setting ItemIDHigh to -1
            // This triggers the game to reload the weapon entity
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
            
            // Mark as uninitialized to force reload
            Mem::Write<bool>(item + Offsets::m_bInitialized, false);
            
            // Update cache
            lastKnifeEntity = activeWeapon;
            lastKnifeDefIndex = targetDefIndex;
            lastKnifePaintKit = SkinChanger::cfg.knifePaintKit;
            
            Log("[KNIFE] Applied: defIndex=%d, paintKit=%d, wear=%.4f, seed=%d", 
                targetDefIndex, SkinChanger::cfg.knifePaintKit, SkinChanger::cfg.knifeWear, SkinChanger::cfg.knifeSeed);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[KNIFE] Exception in ApplyKnifeModifications");
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Apply glove modifications (called once per frame)
    // ═══════════════════════════════════════════════════════════════════════
    inline void ApplyGloveModifications(uintptr_t localPawn)
    {
        if (!SkinChanger::cfg.gloveEnabled) return;
        if (SkinChanger::cfg.gloveModel <= 0 || SkinChanger::cfg.gloveModel >= SkinChanger::kGloveCount) return;
        
        __try {
            // Get glove entity
            uintptr_t gloveEntity = Mem::Read<uintptr_t>(localPawn + Offsets::m_EconGloves);
            if (!gloveEntity || gloveEntity < 0x10000) return;
            
            // Get item data
            uintptr_t item = gloveEntity + Offsets::m_AttributeManager + Offsets::m_Item;
            if (!item || item < 0x10000) return;
            
            // Read current defIndex
            uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
            
            // Get target defIndex
            int targetDefIndex = SkinChanger::kGloves[SkinChanger::cfg.gloveModel].defIndex;
            
            // Check if already applied
            if (gloveEntity == lastGloveEntity && 
                targetDefIndex == lastGloveDefIndex && 
                SkinChanger::cfg.glovePaintKit == lastGlovePaintKit) {
                return; // Already applied
            }
            
            Log("[GLOVE] Applying modifications: %d -> %d (entity: 0x%llX)", 
                currentDefIndex, targetDefIndex, gloveEntity);
            
            // Modify item definition
            Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
            
            // Write subclass ID
            Mem::Write<uint32_t>(gloveEntity + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
            
            // Apply skin
            Mem::Write<int32_t>(gloveEntity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.glovePaintKit);
            Mem::Write<float>(gloveEntity + Offsets::m_flFallbackWear, SkinChanger::cfg.gloveWear);
            
            // Force model update
            Mem::Write<bool>(item + Offsets::m_bInitialized, false);
            
            // Mark for reapply
            Mem::Write<bool>(localPawn + Offsets::m_bNeedToReApplyGloves, true);
            
            // Update cache
            lastGloveEntity = gloveEntity;
            lastGloveDefIndex = targetDefIndex;
            lastGlovePaintKit = SkinChanger::cfg.glovePaintKit;
            
            Log("[GLOVE] Applied: defIndex=%d, paintKit=%d, wear=%.4f", 
                targetDefIndex, SkinChanger::cfg.glovePaintKit, SkinChanger::cfg.gloveWear);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[GLOVE] Exception in ApplyGloveModifications");
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Main tick function (called from Present hook)
    // ═══════════════════════════════════════════════════════════════════════
    inline void Tick()
    {
        // Only check every 10 frames (6 times per second at 60 FPS)
        // This reduces overhead while still being responsive
        frameCounter++;
        if (frameCounter < 10) return;
        frameCounter = 0;
        
        if (!SkinChanger::cfg.enabled) return;
        if (!GameState::clientBase) return;
        
        __try {
            // Get local player
            uintptr_t localPawn = GameState::GetLocalPawn();
            if (!localPawn || localPawn < 0x10000) {
                // Reset cache when not in game
                lastKnifeEntity = 0;
                lastGloveEntity = 0;
                return;
            }
            
            // Check if alive
            uint8_t lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
            int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
            
            if (lifeState != 0 || health <= 0) {
                // Reset cache when dead
                lastKnifeEntity = 0;
                lastGloveEntity = 0;
                return;
            }
            
            // Apply modifications
            ApplyKnifeModifications(localPawn);
            ApplyGloveModifications(localPawn);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[Manager] Exception in Tick");
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Force update (called when config changes)
    // ═══════════════════════════════════════════════════════════════════════
    inline void ForceUpdate()
    {
        lastKnifeEntity = 0;
        lastKnifeDefIndex = 0;
        lastKnifePaintKit = 0;
        lastGloveEntity = 0;
        lastGloveDefIndex = 0;
        lastGlovePaintKit = 0;
        needsUpdate.store(true);
        Log("[Manager] Force update requested");
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Initialization
    // ═══════════════════════════════════════════════════════════════════════
    inline void Init()
    {
        Log("[Manager] Knife/Glove Manager initialized");
        Log("[Manager] This manager applies modifications once per frame");
        Log("[Manager] NO high-frequency hooks = NO lag");
    }
    
    inline void Shutdown()
    {
        Log("[Manager] Knife/Glove Manager shutdown");
    }
}
