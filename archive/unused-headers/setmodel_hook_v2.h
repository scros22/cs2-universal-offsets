#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// SETMODEL HOOK - KNIFE MODEL SUBSTITUTION WITHOUT BREAKING WEAPON SWITCHING
// ═══════════════════════════════════════════════════════════════════════════
//
// THE PROBLEM:
// - Modifying m_iItemDefinitionIndex breaks weapon switching (game can't find knife in slot 3)
// - UpdateSubclass is called during initialization, but weapon switching uses the ORIGINAL defIndex
//
// THE SOLUTION:
// - DON'T modify defIndex at all - leave it as 42 (CT knife) or 59 (T knife)
// - Hook SetModel to intercept model path and substitute our custom knife model
// - This way weapon switching works (game finds knife by original defIndex)
// - But the MODEL displayed is our custom knife (Karambit, Butterfly, etc.)
//
// IDA PRO VERIFIED:
// SetModel @ 0x1808cc060
// Signature: 40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40
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
    // Function pointer type for SetModel
    using SetModelFn = void(__fastcall*)(uintptr_t entity, const char* modelPath);
    
    inline SetModelFn Original_SetModel = nullptr;
    inline uintptr_t hookAddress = 0;
    inline bool initialized = false;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging helper
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
        OutputDebugStringA(buf);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Helper: Check if this is a knife model path
    // ═══════════════════════════════════════════════════════════════════════
    inline bool IsKnifeModel(const char* modelPath) {
        if (!modelPath) return false;
        
        // Check for default knife models
        return (strstr(modelPath, "weapon_knife.vmdl") != nullptr ||
                strstr(modelPath, "weapon_knife_t.vmdl") != nullptr);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Helper: Get custom knife model path based on config
    // ═══════════════════════════════════════════════════════════════════════
    inline const char* GetCustomKnifeModel() {
        if (!SkinChanger::cfg.knifeEnabled || 
            SkinChanger::cfg.knifeModel <= 0 || 
            SkinChanger::cfg.knifeModel >= SkinChanger::kKnifeCount) {
            return nullptr;
        }
        
        int defIndex = SkinChanger::kKnives[SkinChanger::cfg.knifeModel].defIndex;
        return SkinChanger::GetKnifeModelPath(defIndex);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Helper: Check if this is a glove model path
    // ═══════════════════════════════════════════════════════════════════════
    inline bool IsGloveModel(const char* modelPath) {
        if (!modelPath) return false;
        
        // Check for glove model paths
        return (strstr(modelPath, "glove_") != nullptr ||
                strstr(modelPath, "arms/") != nullptr);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Helper: Get custom glove model path based on config
    // ═══════════════════════════════════════════════════════════════════════
    inline const char* GetCustomGloveModel() {
        if (!SkinChanger::cfg.gloveEnabled || 
            SkinChanger::cfg.gloveModel <= 0 || 
            SkinChanger::cfg.gloveModel >= SkinChanger::kGloveCount) {
            return nullptr;
        }
        
        int defIndex = SkinChanger::kGloves[SkinChanger::cfg.gloveModel].defIndex;
        return SkinChanger::GetGloveModelPath(defIndex);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // HOOK: SetModel - Substitute knife/glove model path
    // ═══════════════════════════════════════════════════════════════════════
    void __fastcall Hook_SetModel(uintptr_t entity, const char* modelPath)
    {
        __try {
            // CRITICAL: Validate ALL pointers before dereferencing
            
            // 1. Validate entity pointer
            if (!entity || entity < 0x10000 || entity > 0x7FFFFFFFFFFF)
            {
                Log("[SetModel] Invalid entity pointer: 0x%llX", entity);
                Original_SetModel(entity, modelPath);
                return;
            }
            
            // 2. Validate modelPath pointer
            if (!modelPath)
            {
                Log("[SetModel] NULL modelPath for entity 0x%llX", entity);
                Original_SetModel(entity, modelPath);
                return;
            }
            
            // 3. Validate modelPath is readable (test read first byte)
            __try {
                volatile char test = modelPath[0];
                (void)test; // Suppress unused warning
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[SetModel] Unreadable modelPath for entity 0x%llX", entity);
                Original_SetModel(entity, modelPath);
                return;
            }
            
            // 4. Only process if knife/glove changer is enabled
            if (!SkinChanger::cfg.enabled) {
                Original_SetModel(entity, modelPath);
                return;
            }
            
            // 5. Check if this is a knife model
            if (SkinChanger::cfg.knifeEnabled && IsKnifeModel(modelPath))
            {
                const char* customModel = GetCustomKnifeModel();
                if (customModel)
                {
                    Log("[SetModel] Substituting knife: %s -> %s (entity: 0x%llX)", 
                        modelPath, customModel, entity);
                    Original_SetModel(entity, customModel);
                    return;
                }
            }
            
            // 6. Check if this is a glove model
            if (SkinChanger::cfg.gloveEnabled && IsGloveModel(modelPath))
            {
                const char* customModel = GetCustomGloveModel();
                if (customModel)
                {
                    Log("[SetModel] Substituting glove: %s -> %s (entity: 0x%llX)", 
                        modelPath, customModel, entity);
                    Original_SetModel(entity, customModel);
                    return;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[SetModel] Exception caught - calling original");
        }
        
        // Call original with unmodified path
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
        const char* sig = "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40";
        hookAddress = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (!hookAddress) {
            Log("[SetModel] ERROR: Failed to find SetModel function");
            return false;
        }
        
        Log("[SetModel] Found SetModel at 0x%llX", hookAddress);
        
        // Create hook using MinHook
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
        Log("[SetModel] Will substitute knife models without modifying defIndex");
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
