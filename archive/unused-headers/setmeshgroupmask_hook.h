#pragma once

// ═══════════════════════════════════════════════════════════════════════════
// SETMESHGROUPMASK HOOK - MESH VISIBILITY CONTROL FOR KNIFE/GLOVE MODELS
// ═══════════════════════════════════════════════════════════════════════════
//
// IDA PRO VERIFIED (2026-04-14):
// ────────────────────────────────────────────────────────────────────────────
// Function: SetMeshGroupMask @ 0x180A329C0
// Signature: 48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8D 99
// 
// This function controls which mesh groups (parts) of a model are visible.
// For knives/gloves, we need to ensure all mesh groups are visible (full mask).
//
// Offset 896 (0x380) in entity = m_MeshGroupMask in CModelState
// ═══════════════════════════════════════════════════════════════════════════

#include <Windows.h>
#include <cstdint>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"
#include "../vendor/minhook/include/MinHook.h"
#include "skinchanger_test.h"

namespace SetMeshGroupMaskHook
{
    // Function pointer type for SetMeshGroupMask
    using SetMeshGroupMaskFn = void(__fastcall*)(uintptr_t entity, uint64_t mask);
    
    inline SetMeshGroupMaskFn Original_SetMeshGroupMask = nullptr;
    inline uintptr_t hookAddress = 0;
    inline bool initialized = false;
    
    // ═══════════════════════════════════════════════════════════════════════
    // Logging helper
    // ═══════════════════════════════════════════════════════════════════════
    inline void Log(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "setmeshgroupmask_debug.txt");
        
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
    // Helper: Check if entity is a knife
    // ═══════════════════════════════════════════════════════════════════════
    inline bool IsKnifeEntity(uintptr_t entity) {
        __try {
            if (!entity || entity < 0x10000 || entity > 0x7FFFFFFFFFFF)
                return false;
            
            uintptr_t item = entity + Offsets::m_AttributeManager + Offsets::m_Item;
            if (!item || item < 0x10000 || item > 0x7FFFFFFFFFFF)
                return false;
            
            uint16_t defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
            return SkinChanger::IsKnife(defIndex);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Helper: Check if entity is gloves
    // ═══════════════════════════════════════════════════════════════════════
    inline bool IsGloveEntity(uintptr_t entity) {
        __try {
            if (!entity || entity < 0x10000 || entity > 0x7FFFFFFFFFFF)
                return false;
            
            uintptr_t item = entity + Offsets::m_AttributeManager + Offsets::m_Item;
            if (!item || item < 0x10000 || item > 0x7FFFFFFFFFFF)
                return false;
            
            uint16_t defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
            return (defIndex >= 5027 && defIndex <= 5033);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // HOOK: SetMeshGroupMask - Ensure all mesh groups visible for custom models
    // ═══════════════════════════════════════════════════════════════════════
    void __fastcall Hook_SetMeshGroupMask(uintptr_t entity, uint64_t mask)
    {
        __try {
            // Validate entity pointer
            if (!entity || entity < 0x10000 || entity > 0x7FFFFFFFFFFF) {
                Original_SetMeshGroupMask(entity, mask);
                return;
            }
            
            // Only process if knife/glove changer is enabled
            if (!SkinChanger::cfg.enabled) {
                Original_SetMeshGroupMask(entity, mask);
                return;
            }
            
            // Check if this is a knife entity
            if (SkinChanger::cfg.knifeEnabled && IsKnifeEntity(entity))
            {
                // Use full mask for custom knives (show all mesh groups)
                Log("[SetMeshGroupMask] Knife detected - applying full mask (entity: 0x%llX)", entity);
                Original_SetMeshGroupMask(entity, 0xFFFFFFFFFFFFFFFF);
                return;
            }
            
            // Check if this is a glove entity
            if (SkinChanger::cfg.gloveEnabled && IsGloveEntity(entity))
            {
                // Use full mask for custom gloves
                Log("[SetMeshGroupMask] Glove detected - applying full mask (entity: 0x%llX)", entity);
                Original_SetMeshGroupMask(entity, 0xFFFFFFFFFFFFFFFF);
                return;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[SetMeshGroupMask] Exception caught - calling original");
        }
        
        // Call original with unmodified mask
        Original_SetMeshGroupMask(entity, mask);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // Initialization
    // ═══════════════════════════════════════════════════════════════════════
    inline bool Init()
    {
        if (initialized) return true;
        if (!GameState::clientBase) {
            Log("[SetMeshGroupMask] ERROR: clientBase is NULL");
            return false;
        }
        
        Log("[SetMeshGroupMask] Starting initialization...");
        
        // Find SetMeshGroupMask function using friend's verified signature
        const char* sig = "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8D 99";
        hookAddress = Mem::FindPatternInModule(GameState::clientBase, sig);
        
        if (!hookAddress) {
            Log("[SetMeshGroupMask] ERROR: Failed to find SetMeshGroupMask function");
            return false;
        }
        
        Log("[SetMeshGroupMask] Found SetMeshGroupMask at 0x%llX", hookAddress);
        
        // Create hook using MinHook
        MH_STATUS status = MH_CreateHook(
            reinterpret_cast<void*>(hookAddress),
            &Hook_SetMeshGroupMask,
            reinterpret_cast<void**>(&Original_SetMeshGroupMask)
        );
        
        if (status != MH_OK) {
            Log("[SetMeshGroupMask] ERROR: MH_CreateHook failed: %d", status);
            return false;
        }
        
        // Enable hook
        status = MH_EnableHook(reinterpret_cast<void*>(hookAddress));
        
        if (status != MH_OK) {
            Log("[SetMeshGroupMask] ERROR: MH_EnableHook failed: %d", status);
            return false;
        }
        
        Log("[SetMeshGroupMask] Hook installed successfully!");
        Log("[SetMeshGroupMask] Will ensure all mesh groups visible for custom knives/gloves");
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
        Log("[SetMeshGroupMask] Hook removed");
    }
}
