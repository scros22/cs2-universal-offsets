#pragma once

// ---------------------------------------------------------------
// Auto-Generated Signatures from Enhanced cs2-dumper
// Generated: 2026-04-14 15:52:26 UTC
// Build: Current CS2 build (fresh signatures)
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>

namespace AutoSignatures
{
    // ---------------------------------------------------------------
    // Client.dll - Skinchanger Functions
    // ---------------------------------------------------------------
    
    // Core inventory management
    constexpr const char* EquipItemInLoadout = "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA";
    constexpr const char* GetItemInLoadout = "40 55 48 83 EC ? 49 63 E8";
    
    // Model and mesh functions
    constexpr const char* SetBodyGroup = "85 D2 0F 88 ? ? ? ? 55 57";
    constexpr const char* SetModel = "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 ?";
    constexpr const char* SetMeshGroupMask = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71";
    
    // Paint kit and skin functions
    constexpr const char* CreateNewPaintKit = "48 89 5C 24 10 56 48 83 EC 20 48 8B 01 FF 50 10 48 8B 1D ? ? ? ?";
    constexpr const char* RegenerateWeaponSkin = "40 55 53 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 44 0F B6 FA 48 8B D9 BA ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?";
    constexpr const char* UpdateCompositeMaterial = "48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 44 0F B6 F2 48 8B F1 E8";
    
    // Inventory system
    constexpr const char* GetInventoryManager = "E8 ? ? ? ? 48 8B D3 48 8B C8 4C 8B 00 41 FF 90 00 02";
    constexpr const char* CreateSOSubclassEconItem = "48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85";
    
    // ---------------------------------------------------------------
    // MaterialSystem2.dll - Advanced Material Functions
    // ---------------------------------------------------------------
    
    constexpr const char* CreateMaterial = "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05";
    constexpr const char* PrepareSceneMaterial = "48 89 5C 24 08 48 89 74 24 ? 57 48 83 EC 30 48 8B 59 ? 48 8B F2 48 63 79 ? 48 C1 E7 06";
    constexpr const char* FindParameter = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 20 48";
    constexpr const char* UpdateParameter = "48 89 7C 24 ? 41 56 48 83 EC ? 8B 81";
    
    // ---------------------------------------------------------------
    // Generated Addresses (RVA - add to module base)
    // ---------------------------------------------------------------
    
    namespace Addresses
    {
        namespace Client
        {
            constexpr std::ptrdiff_t EquipItemInLoadout = 0x7C1AD0;
            constexpr std::ptrdiff_t GetItemInLoadout = 0x7C36F0;
            constexpr std::ptrdiff_t SetBodyGroup = 0x14D1BE0;
            constexpr std::ptrdiff_t SetModel = 0x8E19A0;
            constexpr std::ptrdiff_t SetMeshGroupMask = 0xA329C0;
            constexpr std::ptrdiff_t CreateNewPaintKit = 0x10C9E90;
            constexpr std::ptrdiff_t RegenerateWeaponSkin = 0x793080;
            constexpr std::ptrdiff_t UpdateCompositeMaterial = 0x13DB150;
            constexpr std::ptrdiff_t GetInventoryManager = 0x10C26FE;
            constexpr std::ptrdiff_t CreateSOSubclassEconItem = 0x1018A20;
        }
        
        namespace MaterialSystem2
        {
            constexpr std::ptrdiff_t CreateMaterial = 0x3BB70;
            constexpr std::ptrdiff_t PrepareSceneMaterial = 0x11BC0;
            constexpr std::ptrdiff_t FindParameter = 0x11E10;
            constexpr std::ptrdiff_t UpdateParameter = 0x12350;
        }
    }
}