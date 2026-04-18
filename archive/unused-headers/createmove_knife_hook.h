#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// CREATEMOVE KNIFE HOOK - MODIFY AT SPAWN TIME (BEFORE GAME CACHES MODEL)
// ═══════════════════════════════════════════════════════════════════════════
//
// THE REAL PROBLEM:
// CS2 caches the knife model at spawn time. Once cached, writing defIndex
// doesn't trigger a model reload. We need to modify BEFORE the cache happens.
//
// THE SOLUTION:
// Hook CreateMove (called every tick BEFORE weapon initialization)
// Modify defIndex BEFORE game reads it for the first time
// This way the game caches our custom knife, not the default
//
// This is the ONLY approach that works without lag or crashes.
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include <atomic>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../vendor/minhook/include/MinHook.h"
#include "skinchanger_test.h"

namespace CreateMoveKnifeHook
{
    // Function pointer type for CreateMove
    using CreateMoveFn = bool(__fastcall*)(void* thisptr, int slot, bool active);
    
    inline CreateMoveFn Original_CreateMove = nullptr;
    inline uintptr_t hookAddress = 0;
    inline bool initialized = false;
    
    // Track what we've modified
    inline std::atomic<bool> hasModifiedKnife{false};
    inline std::atomic<bool> hasModifiedGlove{false};
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging helper
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "createmove_knife_debug.txt");
        
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
    // Modify knife BEFORE game caches it
    // ═══════════════════════════════════════════════════════════════════════
    inline void ModifyKnifeBeforeCache(uintptr_t localPawn)
    {
        if (!SkinChanger::cfg.knifeEnabled) return;
        if (SkinChanger::cfg.knifeModel <= 0 || SkinChanger::cfg.knifeModel >= SkinChanger::kKnifeCount) return;
        
        __try {
            // Iterate through all weapons in inventory
            for (int i = 0; i < 8; i++) {
                uint32_t weaponHandle = Mem::Read<uint32_t>(localPawn + Offsets::m_pWeaponServices + 0x10 + (i * 4));
                if (!weaponHandle || weaponHandle == 0xFFFFFFFF) continue;
                
                uintptr_t weapon = GameState::ResolveHandle(weaponHandle);
                if (!weapon || weapon < 0x10000) continue;
                
                uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;
                if (!item || item < 0x10000) continue;
                
                uint16_t defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
                
                // Only modify knives
                if (!SkinChanger::IsKnife(defIndex)) continue;
                
                int targetDefIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
                
                // Check if already modified
                uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
                if (currentDefIndex == targetDefIndex) continue;
                
                Log("[KNIFE] Modifying BEFORE cache: %d -> %d (weapon: 0x%llX)", 
                    defIndex, targetDefIndex, weapon);
                
                // Modify defIndex BEFORE game caches it
                Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
                Mem::Write<int32_t>(item + Offsets::m_iEntityQuality, 3);
                Mem::Write<uint32_t>(weapon + Offsets::m_nSubclassID, (uint32_t)targetDefIndex);
                
                // Apply skin
                Mem::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.knifePaintKit);
                Mem::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, SkinChanger::cfg.knifeSeed);
                Mem::Write<float>(weapon + Offsets::m_flFallbackWear, SkinChanger::cfg.knifeWear);
                
                if (SkinChanger::cfg.knifeStatTrak >= 0) {
                    Mem::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, SkinChanger::cfg.knifeStatTrak);
                }
                
                Log("[KNIFE] Modified successfully - game will cache custom knife");
                hasModifiedKnife.store(true);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[KNIFE] Exception in ModifyKnifeBeforeCache");
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // HOOK: CreateMove - Called every tick BEFORE weapon initialization
    // ═══════════════════════════════════════════════════════════════════════
    bool __fastcall Hook_CreateMove(void* thisptr, int slot, bool active)
    {
        __try {
            if (SkinChanger::cfg.enabled) {
                uintptr_t localPawn = GameState::GetLocalPawn();
                if (localPawn && localPawn > 0x10000) {
                    // Check if alive
                    uint8_t lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
                    int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
                    
                    if (lifeState == 0 && health > 0) {
                        // Modify knife BEFORE game caches it
                        ModifyKnifeBeforeCache(localPawn);
                    } else {
                        // Reset flag when dead (so we modify again on respawn)
                        hasModifiedKnife.store(false);
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[CreateMove] Exception in hook");
        }
        
        return Original_CreateMove(thisptr, slot, active);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Initialization
    // ═══════════════════════════════════════════════════════════════════════
    inline bool Init()
    {
        if (initialized) return true;
        if (!GameState::clientBase) {
            Log("[CreateMove] ERROR: clientBase is NULL");
            return false;
        }
        
        Log("[CreateMove] Starting initialization...");
        
        // Find CreateMove function
        // Signature from br5rhvh.txt: SetupMove
        const char* sig = "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 48 8B EA 4C 8B F1 E8 ? ? ? ? 48 8D 15";
        hookAddress = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (!hookAddress) {
            Log("[CreateMove] ERROR: Failed to find SetupMove function");
            return false;
        }
        
        Log("[CreateMove] Found SetupMove at 0x%llX", hookAddress);
        
        // Create hook using MinHook
        MH_STATUS status = MH_CreateHook(
            reinterpret_cast<void*>(hookAddress),
            &Hook_CreateMove,
            reinterpret_cast<void**>(&Original_CreateMove)
        );
        
        if (status != MH_OK) {
            Log("[CreateMove] ERROR: MH_CreateHook failed: %d", status);
            return false;
        }
        
        // Enable hook
        status = MH_EnableHook(reinterpret_cast<void*>(hookAddress));
        
        if (status != MH_OK) {
            Log("[CreateMove] ERROR: MH_EnableHook failed: %d", status);
            return false;
        }
        
        Log("[CreateMove] Hook installed successfully!");
        Log("[CreateMove] Will modify knife BEFORE game caches model");
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
        Log("[CreateMove] Hook removed");
    }
}
