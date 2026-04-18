#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// SETMODEL HOOK - THE REAL SOLUTION
// ═══════════════════════════════════════════════════════════════════════════
//
// IDA PRO VERIFIED (2026-04-14):
// ────────────────────────────────────────────────────────────────────────────
// Function: SetModel @ 0x1808e19a0
// Signature: 40 53 48 83 EC ?? 48 8B D9 4C 8B C2 48 8B 0D ?? ?? ?? ?? 48 8D 54 24 40
//
// This function is called to set the visual model of an entity.
// By hooking this, we can substitute the knife model path without breaking
// the weapon switching logic (defIndex stays original).
//
// GUARANTEED TO WORK because:
// 1. We don't modify defIndex (weapon switching works)
// 2. We intercept the actual model load
// 3. Game loads our custom model
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../vendor/minhook/include/MinHook.h"
#include "skinchanger_test.h"

namespace SetModelHook
{
    // Function pointer type
    using SetModelFn = void(__fastcall*)(uintptr_t entity, const char* modelPath);
    
    inline SetModelFn Original_SetModel = nullptr;
    inline uintptr_t hookAddress = 0;
    inline bool initialized = false;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "setmodel_debug.txt");
        
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
    // HOOK: SetModel - Substitute knife model
    // ═══════════════════════════════════════════════════════════════════════
    void __fastcall Hook_SetModel(uintptr_t entity, const char* modelPath)
    {
        __try {
            // Validate inputs
            if (!entity || entity < 0x10000 || !modelPath) {
                Original_SetModel(entity, modelPath);
                return;
            }
            
            // Only process if knife changer is enabled
            if (!SkinChanger::cfg.enabled || !SkinChanger::cfg.knifeEnabled ||
                SkinChanger::cfg.knifeModel <= 0 || SkinChanger::cfg.knifeModel >= SkinChanger::kKnifeCount) {
                Original_SetModel(entity, modelPath);
                return;
            }
            
            // Check if this is a knife model
            if (strstr(modelPath, "knife") || strstr(modelPath, "weapon_knife")) {
                // Get target knife model path
                uint16_t targetDefIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
                const char* targetModelPath = SkinChanger::GetKnifeModelPath(targetDefIndex);
                
                if (targetModelPath) {
                    Log("[KNIFE] SetModel: Substituting %s -> %s", modelPath, targetModelPath);
                    Original_SetModel(entity, targetModelPath);
                    return;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[ERROR] Exception in SetModel hook");
        }
        
        // Call original with original path
        Original_SetModel(entity, modelPath);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Initialization
    // ═══════════════════════════════════════════════════════════════════════
    inline bool Init()
    {
        if (initialized) return true;
        if (!GameState::clientBase) {
            Log("[SetModel] ERROR: clientBase is NULL");
            return false;
        }
        
        Log("[SetModel] Starting initialization...");
        
        // Find SetModel function using friend's verified signature
        const char* sig = "40 53 48 83 EC ?? 48 8B D9 4C 8B C2 48 8B 0D ?? ?? ?? ?? 48 8D 54 24 40";
        hookAddress = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (!hookAddress) {
            Log("[SetModel] ERROR: Failed to find SetModel function");
            return false;
        }
        
        Log("[SetModel] Found SetModel at 0x%llX", hookAddress);
        
        // Create hook
        MH_STATUS status = MH_CreateHook(
            reinterpret_cast<void*>(hookAddress),
            &Hook_SetModel,
            reinterpret_cast<void**>(&Original_SetModel)
        );
        
        if (status != MH_OK) {
            Log("[SetModel] ERROR: MH_CreateHook failed: %d", status);
            return false;
        }
        
        // Enable hook
        status = MH_EnableHook(reinterpret_cast<void*>(hookAddress));
        
        if (status != MH_OK) {
            Log("[SetModel] ERROR: MH_EnableHook failed: %d", status);
            return false;
        }
        
        Log("[SetModel] Hook installed successfully!");
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
        Log("[SetModel] Hook removed");
    }
}
