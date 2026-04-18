#pragma once

// ---------------------------------------------------------------
// Enhanced Signatures - Research-based patterns for advanced features
// Based on UC forum research (Raphilaa, koz11, March 2026)
// ---------------------------------------------------------------

namespace EnhancedSignatures
{
    // ----------------------------------------------------------------
    // Skin/Knife/Glove Changer - Inventory System
    // ----------------------------------------------------------------
    
    // CCSInventoryManager::EquipItemInLoadout - Core inventory function
    constexpr const char* EquipItemInLoadout =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA";
    
    // CCSPlayerInventory::GetItemInLoadout - Get item from loadout slot
    constexpr const char* GetItemInLoadout =
        "40 55 48 83 EC ? 49 63 E8";
    
    // CEconItemView::GetPaintKitIndex - Resolve paint kit from econ item
    constexpr const char* GetPaintKitIndex =
        "48 89 5C 24 ? 57 48 83 EC ? 8B 15 ? ? ? ? 48 8B F9 65 48 8B 04 25 ? ? ? ? B9 ? ? ? ? 48 8B 04 D0 8B 04 01 39 05 ? ? ? ? 0F 8F ? ? ? ? E8 ? ? ? ? 8B 58 ? 39 1D ? ? ? ? 74 ? E8 ? ? ? ? 48 8B 15 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 89 05 ? ? ? ? 89 1D ? ? ? ? EB ? 48 8B 05 ? ? ? ? 48 85 C0 74";
    
    // CBaseEntity::SetBodyGroup - Required for glove mesh updates
    constexpr const char* SetBodyGroup =
        "85 D2 0F 88 5C";
    
    // CBaseModelEntity::SetModel - Model changing for knives
    constexpr const char* SetModel =
        "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40";

    // ----------------------------------------------------------------
    // World Effects - Advanced Rendering
    // ----------------------------------------------------------------
    
    // Night Vision / Dark Mode - Tonemap controller manipulation
    constexpr const char* TonemapController =
        "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ?";
    
    // Fog Controller - Atmospheric effects
    constexpr const char* FogController =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 48 8B EA";
    
    // Post Process Controller - Screen effects
    constexpr const char* PostProcessController =
        "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24";
    
    // Material System - Shader parameter modification
    constexpr const char* SetShaderParam =
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B FA 48 8B F1 48 85 D2";

    // ----------------------------------------------------------------
    // Advanced Chams - Material Overrides
    // ----------------------------------------------------------------
    
    // Material Creation - For custom chams materials
    constexpr const char* CreateMaterial =
        "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05";
    
    // Shader Compilation - Runtime shader creation
    constexpr const char* CompileShader =
        "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 54 41 55 41 56 48 81 EC";
    
    // Render Context - Drawing pipeline hook
    constexpr const char* RenderContext =
        "48 8B C4 53 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 45 33 F6";
}