#pragma once

// ---------------------------------------------------------------
// STABLE SKINCHANGER - Based on friend's working implementation
// Key improvements:
// - Only applies to active weapon (no mass iteration)
// - Uses proper attribute system (temporary attributes)
// - Has regeneration function with proper cleanup
// - Caching to prevent unnecessary applications
// - Exception handling for stability
//
// CS2 KNIFE/GLOVE CHANGING (2026 RESEARCH - CRITICAL DISCOVERY):
// **THE PROBLEM**: Just writing m_iItemDefinitionIndex doesn't work!
// **THE SOLUTION**: Must call EquipItemInLoadout to update inventory system
//
// CS2 uses an inventory manager that tracks equipped items. When you change
// a knife/glove defIndex, the game doesn't see it until you tell the inventory
// system via EquipItemInLoadout(team, slot, itemID).
//
// This is why weapon skins work (they use fallback system) but knives/gloves
// don't (they need inventory system update).
//
// Signature: EquipItemInLoadout @ client.dll
// "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 89 54 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F B7 FA"
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <map>
#include <mutex>
#include <atomic>
#include "../core/game_state.h"
#include "../core/sdk_offsets.h"
#include "../core/memory.h"

namespace SkinChanger
{
    struct SkinConfig
    {
        int paintKit = 0;
        float wear = 0.001f;
        int seed = 0;
        int statTrak = -1;
        bool enabled = false;
    };

    #pragma pack(push, 1)
    struct CEconItemAttribute
    {
        uintptr_t vtable;
        uintptr_t owner;
        char pad_0010[32];
        uint16_t defIndex;
        char pad_0032[2];
        float value;
        float initValue;
        int32_t refundableCurrency;
        bool setBonus;
        char pad_0041[7];
    };

    struct CPtrGameVector
    {
        uint64_t size;
        uintptr_t ptr;
    };
    #pragma pack(pop)

    // ---------------------------------------------------------------
    // Lookup tables for menu compatibility
    // ---------------------------------------------------------------
    struct ItemDef {
        int defIndex;
        const char* name;
    };

    inline constexpr ItemDef kKnives[] = {
        { 0,   "Default" },
        { 500, "Bayonet" },
        { 503, "Classic Knife" },
        { 505, "Flip Knife" },
        { 506, "Gut Knife" },
        { 507, "Karambit" },
        { 508, "M9 Bayonet" },
        { 509, "Huntsman Knife" },
        { 512, "Falchion Knife" },
        { 514, "Bowie Knife" },
        { 515, "Butterfly Knife" },
        { 516, "Shadow Daggers" },
        { 517, "Paracord Knife" },
        { 518, "Survival Knife" },
        { 519, "Ursus Knife" },
        { 520, "Navaja Knife" },
        { 521, "Nomad Knife" },
        { 522, "Stiletto Knife" },
        { 523, "Talon Knife" },
        { 525, "Skeleton Knife" },
        { 526, "Kukri Knife" },
    };
    inline constexpr int kKnifeCount = sizeof(kKnives) / sizeof(kKnives[0]);

    struct WeaponDef {
        int defIndex;
        const char* name;
    };

    inline constexpr WeaponDef kWeapons[] = {
        { 1,  "Desert Eagle" },
        { 2,  "Dual Berettas" },
        { 3,  "Five-SeveN" },
        { 4,  "Glock-18" },
        { 7,  "AK-47" },
        { 8,  "AUG" },
        { 9,  "AWP" },
        { 10, "FAMAS" },
        { 13, "Galil AR" },
        { 14, "M249" },
        { 16, "M4A4" },
        { 17, "MAC-10" },
        { 19, "P90" },
        { 23, "MP5-SD" },
        { 24, "UMP-45" },
        { 25, "XM1014" },
        { 26, "PP-Bizon" },
        { 27, "MAG-7" },
        { 28, "Negev" },
        { 29, "Sawed-Off" },
        { 30, "Tec-9" },
        { 32, "P2000" },
        { 33, "MP7" },
        { 34, "MP9" },
        { 35, "Nova" },
        { 36, "P250" },
        { 39, "SG 553" },
        { 40, "SSG 08" },
        { 60, "M4A1-S" },
        { 61, "USP-S" },
        { 63, "CZ75-Auto" },
        { 64, "R8 Revolver" },
    };
    inline constexpr int kWeaponCount = sizeof(kWeapons) / sizeof(kWeapons[0]);

    // Glove definitions for menu compatibility
    inline constexpr ItemDef kGloves[] = {
        { 0,    "Default" },
        { 5027, "Sport Gloves" },
        { 5028, "Driver Gloves" },
        { 5029, "Hand Wraps" },
        { 5030, "Moto Gloves" },
        { 5031, "Specialist Gloves" },
        { 5032, "Hydra Gloves" },
        { 5033, "Broken Fang Gloves" },
    };
    inline constexpr int kGloveCount = sizeof(kGloves) / sizeof(kGloves[0]);

    // ---------------------------------------------------------------
    // Model path mappings for SetModel
    // ---------------------------------------------------------------
    inline const char* GetKnifeModelPath(int defIndex) {
        switch (defIndex) {
            case 500: return "weapons/models/knife/knife_bayonet/weapon_knife_bayonet.vmdl";
            case 503: return "weapons/models/knife/knife_css/weapon_knife_css.vmdl";
            case 505: return "weapons/models/knife/knife_flip/weapon_knife_flip.vmdl";
            case 506: return "weapons/models/knife/knife_gut/weapon_knife_gut.vmdl";
            case 507: return "weapons/models/knife/knife_karambit/weapon_knife_karambit.vmdl";
            case 508: return "weapons/models/knife/knife_m9_bayonet/weapon_knife_m9_bayonet.vmdl";
            case 509: return "weapons/models/knife/knife_tactical/weapon_knife_tactical.vmdl";
            case 512: return "weapons/models/knife/knife_falchion/weapon_knife_falchion.vmdl";
            case 514: return "weapons/models/knife/knife_survival_bowie/weapon_knife_survival_bowie.vmdl";
            case 515: return "weapons/models/knife/knife_butterfly/weapon_knife_butterfly.vmdl";
            case 516: return "weapons/models/knife/knife_push/weapon_knife_push.vmdl";
            case 517: return "weapons/models/knife/knife_cord/weapon_knife_cord.vmdl";
            case 518: return "weapons/models/knife/knife_canis/weapon_knife_canis.vmdl";
            case 519: return "weapons/models/knife/knife_ursus/weapon_knife_ursus.vmdl";
            case 520: return "weapons/models/knife/knife_gypsy_jackknife/weapon_knife_gypsy_jackknife.vmdl";
            case 521: return "weapons/models/knife/knife_outdoor/weapon_knife_outdoor.vmdl";
            case 522: return "weapons/models/knife/knife_stiletto/weapon_knife_stiletto.vmdl";
            case 523: return "weapons/models/knife/knife_widowmaker/weapon_knife_widowmaker.vmdl";
            case 524: return "weapons/models/knife/knife_kukri/weapon_knife_kukri.vmdl";
            case 525: return "weapons/models/knife/knife_skeleton/weapon_knife_skeleton.vmdl";
            default: return nullptr;
        }
    }

    // ---------------------------------------------------------------
    // Legacy-mesh classification per items_game.txt `use_legacy_model`.
    //
    // CRITICAL: SetMeshGroupMask must match the model being loaded.
    //   2 = Legacy mesh group (CSGO-era models)
    //   1 = Modern mesh group (default CS2)
    // Passing the wrong mask after SetModel makes the mesh fail to bind
    // entirely (per the UC forum post: "if I set the old model to 2 and
    // the new model to 1, my old model fails to load entirely").
    //
    // Among the 500-526 knife range we swap to, only def 503 (Classic
    // Knife) loads `weapon_knife_css.vmdl`, the CSGO classic mesh, which
    // is `use_legacy_model 1` in items_game.txt. All other modern
    // knives (Bayonet, Karambit, M9, Butterfly, Falchion, Huntsman,
    // Bowie, Push, Cord, Canis, Ursus, Gypsy, Outdoor, Stiletto,
    // Widowmaker, Kukri, Skeleton) ship a CS2-era vmdl and use the
    // modern mesh group (mask=1).
    // ---------------------------------------------------------------
    inline bool IsLegacyKnifeMesh(int defIndex) {
        switch (defIndex) {
            case 42:   // default CT knife
            case 59:   // default T knife
            case 503:  // Classic Knife (knife_css.vmdl)
                return true;
            default:
                return false;
        }
    }

    // Returns the SetMeshGroupMask value to pair with a given def-index.
    inline uint64_t MeshMaskForDef(int defIndex) {
        return IsLegacyKnifeMesh(defIndex) ? 2ull : 1ull;
    }

    inline const char* GetGloveModelPath(int defIndex) {
        switch (defIndex) {
            case 5027: return "weapons/models/arms/glove_sporty/glove_sporty.vmdl";
            case 5028: return "weapons/models/arms/glove_slick/glove_slick.vmdl";
            case 5029: return "weapons/models/arms/glove_handwrap_leathery/glove_handwrap_leathery.vmdl";
            case 5030: return "weapons/models/arms/glove_motorcycle/glove_motorcycle.vmdl";
            case 5031: return "weapons/models/arms/glove_specialist/glove_specialist.vmdl";
            case 5032: return "weapons/models/arms/glove_bloodhound/glove_bloodhound.vmdl";
            case 5033: return "weapons/models/arms/glove_sporty/glove_sporty.vmdl"; // Broken Fang uses sporty base
            default: return nullptr;
        }
    }

    // ---------------------------------------------------------------
    // Menu compatibility structure
    // ---------------------------------------------------------------
    struct WeaponSkin {
        bool enabled = false;
        int paintKit = 0;
        int seed = 0;
        float wear = 0.0001f;
        int statTrak = -1;
    };

    struct Config {
        bool enabled = false;
        int activeWeaponSlot = 0;
        
        // Weapon configs (menu compatibility)
        WeaponSkin weapons[32];
        
        // Knife config
        bool knifeEnabled = false;
        int knifeModel = 0;
        int knifePaintKit = 38;  // Default to Fade for testing
        int knifeSeed = 0;
        float knifeWear = 0.0001f;
        int knifeStatTrak = -1;
        
        // Glove config
        bool gloveEnabled = false;
        int gloveModel = 0;
        int glovePaintKit = 10006;
        float gloveWear = 0.0001f;
    };

    // ---------------------------------------------------------------
    // State variables
    // ---------------------------------------------------------------
    inline Config cfg;
    inline std::map<int, SkinConfig> weaponSkins;
    inline std::atomic<bool> forceUpdate = false;
    inline std::atomic<bool> running = false;
    inline std::mutex configMutex;
    inline uintptr_t lastAppliedWeapon = 0;
    inline int lastAppliedKit = 0;
    inline uintptr_t regenAddr = 0;
    inline bool regenPatched = false;
    inline CEconItemAttribute* g_attrBuffer = nullptr;

    // Menu compatibility variables
    inline int lastKnifeDefIdx = 0;
    inline float lastGloveSpawnTime = 0.f;
    inline int gloveRefreshFrames = 0;

    // SetModel function pointer and related functions
    using SetModelFn = void(__fastcall*)(uintptr_t entity, const char* modelPath);
    using SetMeshGroupMaskFn = void(__fastcall*)(uintptr_t entity, uint64_t mask);
    using UpdateSubclassFn = void(__fastcall*)(uintptr_t item);
    using UpdateWeaponDataFn = void(__fastcall*)(uintptr_t weapon);
    using UpdateCompositeFn = void(__fastcall*)(uintptr_t weapon, int param);
    // CBaseModelEntity::SetBodygroup(int group, int value)
    // Required by CS2 to actually refresh the rendered mesh after a model change.
    // Verified via Ghidra @ 0x1808E0610 in CS2 build (April 2026).
    using SetBodyGroupFn = void(__fastcall*)(uintptr_t entity, int group, int value);
    
    inline SetModelFn SetModel = nullptr;
    inline SetMeshGroupMaskFn SetMeshGroupMask = nullptr;
    inline UpdateSubclassFn UpdateSubclass = nullptr;
    inline UpdateWeaponDataFn UpdateWeaponData = nullptr;
    inline UpdateCompositeFn UpdateComposite = nullptr;
    inline SetBodyGroupFn SetBodyGroup = nullptr;

    // Cache so we only call expensive model swaps when the target changes
    inline uintptr_t lastKnifeModelEntity = 0;
    inline int       lastKnifeModelDef   = 0;
    inline uintptr_t lastGloveModelEntity = 0;
    inline int       lastGloveModelDef   = 0;

    // Simple file logger — writes to %TEMP%\skinchanger.log
    inline void SkLog(const char* fmt, ...) {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "skinchanger.log");
        HANDLE h = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                               nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return;
        char buf[1024];
        va_list ap; va_start(ap, fmt);
        int n = vsprintf_s(buf, sizeof(buf) - 2, fmt, ap);
        va_end(ap);
        if (n > 0) { buf[n] = '\n'; buf[n+1] = 0; DWORD w; WriteFile(h, buf, n+1, &w, nullptr); }
        CloseHandle(h);
    }

    // ---------------------------------------------------------------
    // Helper functions
    // ---------------------------------------------------------------
    inline bool IsKnife(uint16_t defIndex) {
        return defIndex == 42 || defIndex == 59 || (defIndex >= 500 && defIndex <= 526);
    }

    inline CEconItemAttribute MakeAttribute(uint16_t defIndex, float value) {
        CEconItemAttribute attr{};
        attr.defIndex = defIndex;
        attr.value = value;
        attr.initValue = value;
        return attr;
    }

    inline void CreateAttributes(uintptr_t item, int paintKit, int seed, float wear) {
        if (paintKit <= 0) return;

        uintptr_t attrListAddr = item + Offsets::m_AttributeList + Offsets::m_Attributes;
        CPtrGameVector existing = Mem::Read<CPtrGameVector>(attrListAddr);
        if (existing.size > 0 || existing.ptr != 0) return;

        if (!g_attrBuffer) {
            g_attrBuffer = reinterpret_cast<CEconItemAttribute*>(
                VirtualAlloc(nullptr, sizeof(CEconItemAttribute) * 3,
                    MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            if (!g_attrBuffer) return;
        }

        g_attrBuffer[0] = MakeAttribute(6, static_cast<float>(paintKit));
        g_attrBuffer[1] = MakeAttribute(7, static_cast<float>(seed));
        g_attrBuffer[2] = MakeAttribute(8, wear);

        CPtrGameVector newList;
        newList.size = 3;
        newList.ptr = reinterpret_cast<uintptr_t>(g_attrBuffer);
        Mem::Write<CPtrGameVector>(attrListAddr, newList);
    }

    inline void RemoveAttributes(uintptr_t item) {
        uintptr_t attrListAddr = item + Offsets::m_AttributeList + Offsets::m_Attributes;
        CPtrGameVector existing = Mem::Read<CPtrGameVector>(attrListAddr);
        if (existing.size == 0) return;

        CPtrGameVector empty{};
        Mem::Write<CPtrGameVector>(attrListAddr, empty);
    }

    // ---------------------------------------------------------------
    // RegenerateWeaponSkin direct resolver (build 14154+).
    //
    // Previously this patched +0x52 of an outer wrapper to fix up an
    // attribute-list offset — fragile across builds and broken in 14154.
    //
    // Reverse-engineered via IDA on client.dll build 14154:
    //   sub_18078C050 = void RegenerateWeaponSkin(C_BasePlayerWeapon*, bool)
    // Verified by universal-dumper signatures.hpp (RVA 0x78C050).
    //
    // We resolve the function directly by RVA — no code patching needed.
    // The fallback fields (m_nFallbackPaintKit/Seed/Wear/StatTrak) and
    // m_iItemIDHigh=0xFFFFFFFF must already be set on the weapon entity
    // before calling. The function reads those, builds a paint material
    // via materialsystem2, and binds it to the weapon's render slots.
    // ---------------------------------------------------------------
    using RegenerateWeaponSkinFn = void(__fastcall*)(uintptr_t weapon, bool refresh);
    inline RegenerateWeaponSkinFn RegenerateWeaponSkin = nullptr;

    // ---------------------------------------------------------------
    // ApplyEconCustomization (sub_1807A8A00 in build 14155).
    //
    // This is the REAL paint-apply entry point. RegenerateWeaponSkin
    // (sub_18078C050) only handles a legacy static paint table indexed
    // by entity hash — it does NOT consume m_nFallbackPaintKit. The
    // modern path is:
    //
    //   sub_1807A8A00(weapon, 1)
    //     -> sub_181079790(weapon, 1)
    //          -> sub_18105AAF0(weapon)
    //               // Reads m_nFallbackPaintKit / m_nFallbackSeed /
    //               // m_flFallbackWear, calls SetOrAddAttribute with
    //               // "set item texture prefab" / "seed" / "wear"
    //               // on weapon.m_AttributeManager.m_Item.m_AttributeList
    //     -> queues "clientside_reload_custom_econ" delayed think
    //          // This is what actually rebuilds the composite material
    //          // and binds it to the weapon's render slots — i.e. the
    //          // step that makes the skin VISIBLE.
    //
    // Trigger condition (inside sub_18105AAF0):
    //   if EconItemView lookup by m_iItemID FAILS -> apply fallbacks.
    // We force this by writing m_iItemIDHigh = m_iItemIDLow = 0xFFFFFFFF
    // (ID 0xFFFFFFFFFFFFFFFF will not resolve to any real inventory item).
    // ---------------------------------------------------------------
    using ApplyEconCustomizationFn = void(__fastcall*)(uintptr_t weapon, char applyFallbacks);
    inline ApplyEconCustomizationFn ApplyEconCustomization = nullptr;
    inline constexpr std::ptrdiff_t kApplyEconCustomization_RVA = 0x7A8A00;

    // ---------------------------------------------------------------
    // ANIMGRAPH FORCE-REBUILD (build 14155, IDA-verified 2026-04-28)
    // ---------------------------------------------------------------
    // Schema layout (from sdk/client_dll.hpp + IDA decompile of
    // sub_1808AD5F0):
    //   CBaseAnimGraph::m_pMainGraphController  @ entity + 0x1058
    //                                             (CAnimGraphControllerBase*)
    //   CBaseAnimGraphController::m_hGraphDefinitionAG2 @ ctrl + 0x370
    //                                             (CStrongHandle<CNmGraphDefinition>)
    //   CBaseAnimGraphController::m_pGraphInstanceAG2  @ ctrl + 0x448
    //                                             (CNmGraphInstance*)
    //
    // sub_1808AD5F0(controller, char arg2):
    //   arg2 == 2 -> tear down the existing CNmGraphInstance (clears
    //                ctrl+0x448, walks m_vecExternalGraphs at ctrl+0x660
    //                to release each binding via sub_18120D900, calls the
    //                global graph manager's vtable[+424] dtor on the
    //                instance), then recreates bindings via
    //                sub_1808AF930 (CAnimGraphControllerManager::
    //                CreateControllerToGraphBindings) using the manager
    //                lookup chain sub_180A18100(controller).
    //
    // After ApplyKnifeModelSwap spoofs m_nSubclassID + UpdateWeaponData,
    // the controller still references the OLD knife's graph instance.
    // Calling sub_1808AD5F0(controller, 2) destroys that instance and
    // makes the manager re-bind from the (now-updated) vdata's animgraph.
    using AnimGraphRebuildFn = void(__fastcall*)(uintptr_t controller, char mode);
    inline AnimGraphRebuildFn AnimGraphRebuild = nullptr;
    inline constexpr std::ptrdiff_t kAnimGraphRebuild_RVA = 0x8AD5F0;
    inline constexpr std::ptrdiff_t kMainGraphController_OFF = 0x1058;
    inline constexpr std::ptrdiff_t kGraphInstanceAG2_OFF    = 0x448;
    inline constexpr std::ptrdiff_t kGraphDefinitionAG2_OFF  = 0x370;

    // RVA is unchanged (post-14154 build, IDA-verified 2026-04-25).
    // Backup sig captures the unique current-build prologue:
    //   40 55 53 41 57 48 8D AC 24 00 FE FF FF 48 81 EC
    //   = push rbp/rbx/r15; lea rbp,[rsp-1F8h]; sub rsp,...
    // Old prologue check (48 8B C4) was for a DIFFERENT compiler build of
    // this function and silently failed on every current build, leaving
    // RegenerateWeaponSkin = nullptr and ALL skins/knife paint not applying.
    inline constexpr std::ptrdiff_t kRegenerateWeaponSkin_RVA = 0x78C050;

    inline void InitRegen() {
        if (RegenerateWeaponSkin != nullptr) return;
        if (!GameState::clientBase) return;

        // Primary path: direct RVA. Validate prologue first — if drifted,
        // sig scan recovers.
        uintptr_t addr = GameState::clientBase + kRegenerateWeaponSkin_RVA;
        __try {
            // Current-build prologue: 40 55 53 41 57 48 8D AC 24 00 FE FF FF
            uint8_t b0 = *reinterpret_cast<uint8_t*>(addr + 0);
            uint8_t b1 = *reinterpret_cast<uint8_t*>(addr + 1);
            uint8_t b2 = *reinterpret_cast<uint8_t*>(addr + 2);
            uint8_t b3 = *reinterpret_cast<uint8_t*>(addr + 3);
            uint8_t b4 = *reinterpret_cast<uint8_t*>(addr + 4);
            bool prologueOk = (b0 == 0x40 && b1 == 0x55 && b2 == 0x53 &&
                                b3 == 0x41 && b4 == 0x57);
            if (!prologueOk) addr = 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            addr = 0;
        }

        if (!addr) {
            // Fallback sig — current-build prologue, IDA-verified unique.
            const char* sig =
                "40 55 53 41 57 48 8D AC 24 00 FE FF FF 48 81 EC";
            addr = Mem::FindPatternInModule(GameState::clientBase, sig);
        }

        if (addr) {
            RegenerateWeaponSkin = reinterpret_cast<RegenerateWeaponSkinFn>(addr);
            regenAddr = addr;          // legacy state var, kept for compat
            regenPatched = true;       // no patch needed but downstream checks this flag
            SkLog("[Init] RegenerateWeaponSkin resolved @ 0x%llX (RVA 0x%llX)",
                  (unsigned long long)addr,
                  (unsigned long long)(addr - GameState::clientBase));
        } else {
            SkLog("[Init] FAILED to resolve RegenerateWeaponSkin");
        }

        // ----- ApplyEconCustomization (sub_1807A8A00) -----
        // Prologue (current build): 48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 E8
        uintptr_t aecAddr = GameState::clientBase + kApplyEconCustomization_RVA;
        __try {
            uint8_t b0 = *reinterpret_cast<uint8_t*>(aecAddr + 0);
            uint8_t b5 = *reinterpret_cast<uint8_t*>(aecAddr + 5);
            uint8_t b10 = *reinterpret_cast<uint8_t*>(aecAddr + 10);
            uint8_t b11 = *reinterpret_cast<uint8_t*>(aecAddr + 11);
            // 48 89 5C 24 ?? 57 ?? ?? ?? ?? 8B FA
            if (b0 != 0x48 || b5 != 0x57 || b10 != 0x8B || b11 != 0xFA) aecAddr = 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            aecAddr = 0;
        }
        if (!aecAddr) {
            const char* sig =
                "48 89 5C 24 ? 57 48 83 EC ? 8B FA 48 8B D9 E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 48 85 C0 74";
            aecAddr = Mem::FindPatternInModule(GameState::clientBase, sig);
        }
        if (aecAddr) {
            ApplyEconCustomization = reinterpret_cast<ApplyEconCustomizationFn>(aecAddr);
            SkLog("[Init] ApplyEconCustomization resolved @ 0x%llX (RVA 0x%llX)",
                  (unsigned long long)aecAddr,
                  (unsigned long long)(aecAddr - GameState::clientBase));
        } else {
            SkLog("[Init] FAILED to resolve ApplyEconCustomization");
        }
    }

    inline void InitModelFunctions() {
        if (SetModel != nullptr) return;
        if (!GameState::clientBase) return;
        
        // SetModel signature - EXACT from your friend (IDA verified: 0x1808cc060)
        const char* setModelSig = "40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40";
        uintptr_t setModelAddr = Mem::FindPatternInModule(GameState::clientBase, setModelSig);
        if (setModelAddr) {
            SetModel = reinterpret_cast<SetModelFn>(setModelAddr);
        }
        
        // SetMeshGroupMask signature — user-provided (build 14155 verified).
        //   48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71
        const char* meshMaskSig = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? 48 8B 71";
        uintptr_t meshMaskAddr = Mem::FindPatternInModule(GameState::clientBase, meshMaskSig);
        if (meshMaskAddr) {
            SetMeshGroupMask = reinterpret_cast<SetMeshGroupMaskFn>(meshMaskAddr);
        }        
        // UpdateSubclass — INTENTIONALLY UNRESOLVED.
        // The historic sig (4C 8B DC 53 48 81 EC ?? ?? ?? ?? 48 8B 41)
        // matches sub_1801FA880 in current build, which is the
        // "missing subclass data" error logger (calls sub_180364720 with
        // entity-deletion side effects). Calling it on a knife will
        // wipe the weapon. We leave UpdateSubclass = nullptr so callers
        // skip it. ApplyKnifeModelSwap no longer requires it.
        UpdateSubclass = nullptr;
        
        // UpdateWeaponData - NEW! This is what we're missing
        // Signature from forum post analysis
        const char* updateWeaponDataSig = "48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B D8";
        uintptr_t updateWeaponDataAddr = Mem::FindPatternInModule(GameState::clientBase, updateWeaponDataSig);
        if (updateWeaponDataAddr) {
            UpdateWeaponData = reinterpret_cast<UpdateWeaponDataFn>(updateWeaponDataAddr);
        }
        
        // UpdateComposite - NEW! This refreshes the weapon model
        // Signature from forum post analysis
        const char* updateCompositeSig = "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B EA";
        uintptr_t updateCompositeAddr = Mem::FindPatternInModule(GameState::clientBase, updateCompositeSig);
        if (updateCompositeAddr) {
            UpdateComposite = reinterpret_cast<UpdateCompositeFn>(updateCompositeAddr);
        }

        // CBaseModelEntity::SetBodygroup(int, int) — best-effort. The schema
        // string "SetBodygroup" lives in C_BaseModelEntity's metadata table and
        // does NOT directly resolve to an executable export; in build 14152 it
        // is most likely an inline that resolves the bodygroup index then
        // updates the mesh-group mask. Since SetMeshGroupMask above already
        // refreshes the rendered mesh, this call is non-critical — if the
        // pattern fails to resolve we just skip it and rely on the mesh-mask
        // write to refresh the visual.
        const char* setBodyGroupSig =
            "85 D2 0F 88 ? ? ? ? 53 55 56 48 83 EC 70 41 8B F0 8B DA 48 8B E9";
        uintptr_t setBodyGroupAddr = Mem::FindPatternInModule(GameState::clientBase, setBodyGroupSig);
        if (setBodyGroupAddr) {
            SetBodyGroup = reinterpret_cast<SetBodyGroupFn>(setBodyGroupAddr);
        }

        // AnimGraphRebuild = sub_1808AD5F0 in client.dll @ build 14155.
        // Resolved by RVA only (no published sig — function is internal,
        // not exported). RVA verified via IDA decompile this session.
        // See ANIMGRAPH FORCE-REBUILD comment block above for layout.
        if (GameState::clientBase) {
            uintptr_t rebuildAddr = GameState::clientBase + kAnimGraphRebuild_RVA;
            AnimGraphRebuild = reinterpret_cast<AnimGraphRebuildFn>(rebuildAddr);
        }

        // ---- one-time diagnostic so we can verify each sig actually resolved
        SkLog("[Init] sigs: SetModel=0x%llX SetMeshGroupMask=0x%llX UpdateSubclass=0x%llX UpdateWeaponData=0x%llX UpdateComposite=0x%llX SetBodyGroup=0x%llX",
            (unsigned long long)(uintptr_t)SetModel,
            (unsigned long long)(uintptr_t)SetMeshGroupMask,
            (unsigned long long)(uintptr_t)UpdateSubclass,
            (unsigned long long)(uintptr_t)UpdateWeaponData,
            (unsigned long long)(uintptr_t)UpdateComposite,
            (unsigned long long)(uintptr_t)SetBodyGroup);
        char buf[256];
        wsprintfA(buf,
            "[SkinChanger] sig resolve: SetModel=0x%llX SetMeshGroupMask=0x%llX UpdateSubclass=0x%llX\n",
            (unsigned long long)(uintptr_t)SetModel,
            (unsigned long long)(uintptr_t)SetMeshGroupMask,
            (unsigned long long)(uintptr_t)UpdateSubclass);
        OutputDebugStringA(buf);
    }

    // ---------------------------------------------------------------
    // Direct call to RegenerateWeaponSkin(weapon, false).
    //
    // The caller MUST have set up the weapon's fallback fields and
    // m_iItemIDHigh = 0xFFFFFFFF before invoking. The function is
    // self-contained — it walks the weapon's paint data, resolves the
    // vmdl/vcompmat path via materialsystem2, builds the composite
    // material, and binds it to the weapon's render slots.
    //
    // Bypasses the bulk-iterator gate at weapon[0xAA8]/[0xAC0] which
    // would otherwise skip our weapon (those flags are only set on
    // genuinely-owned items by the inventory system).
    // ---------------------------------------------------------------
    inline void CallRegen(uintptr_t weapon = 0) {
        if (!weapon) return;
        __try {
            // Zero the bulk-iterator gate (legacy regen).
            Mem::Write<int32_t>(weapon + 0xAA8, 0);
            Mem::Write<int32_t>(weapon + 0xAC0, 0);

            // Modern path: convert m_nFallback* into real EconItemView
            // attributes and queue clientside_reload_custom_econ.
            // This is what actually makes the skin VISIBLE in build 14155.
            if (ApplyEconCustomization) {
                ApplyEconCustomization(weapon, 1);
            }

            // Legacy path (covers older code that still consults the
            // static paint table — harmless if no-op).
            if (RegenerateWeaponSkin) {
                RegenerateWeaponSkin(weapon, false);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            SkLog("[Regen] EXCEPTION on weapon=0x%llX",
                  (unsigned long long)weapon);
        }
    }

    inline void ApplyAndRegen(uintptr_t weapon, const SkinConfig& skin, uint16_t targetDefIndex) {
        uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;

        if (RegenerateWeaponSkin || ApplyEconCustomization) {
            // Direct-call path (build 14154+): write fallbacks, regen, leave them in place.
            //
            // Unlike the old "patch + flash + cleanup" approach, the fallback fields
            // are PERSISTENT — m_iItemIDHigh=0xFFFFFFFF tells the renderer to read
            // m_nFallbackPaintKit/Seed/Wear/StatTrak directly. Cleaning them up after
            // regen would just put the weapon back into "no skin" state on the next
            // tick. So we leave them set; ApplyGloveSkin / weapon iteration ensures
            // they are re-written when the user picks a different paint.
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
            // Force item-ID lookup to fail so ApplyEconCustomization enters its
            // fallback-apply branch (sub_18105AAF0). m_iItemIDLow lives at
            // m_iItemIDHigh + 4 in the EconItemView.
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh + 4, 0xFFFFFFFF);
            Mem::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, skin.paintKit);
            Mem::Write<float>(weapon + Offsets::m_flFallbackWear, skin.wear);
            Mem::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, skin.seed);
            Mem::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, skin.statTrak);

            CallRegen(weapon);

            static int s_paintLogCount = 0;
            if (s_paintLogCount < 8) {
                SkLog("[Paint] regen: weapon=0x%llX def=%u kit=%d",
                      (unsigned long long)weapon, (unsigned)targetDefIndex, skin.paintKit);
                s_paintLogCount++;
            }
        } else {
            // Persistent fallback approach: keep values written
            // Works without regen — game reads fallbacks when m_iItemIDHigh == 0xFFFFFFFF
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
            Mem::Write<int32_t>(weapon + Offsets::m_nFallbackPaintKit, skin.paintKit);
            Mem::Write<float>(weapon + Offsets::m_flFallbackWear, skin.wear);
            Mem::Write<int32_t>(weapon + Offsets::m_nFallbackSeed, skin.seed);
            Mem::Write<int32_t>(weapon + Offsets::m_nFallbackStatTrak, skin.statTrak);
        }
    }

    // ---------------------------------------------------------------
    // Glove skin helper — SEH-safe, no C++ objects with destructors
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ARCHILIX-style subclass-id magic table per knife def-index.
    // Server validates this hash; without it the swap is rejected.
    // ---------------------------------------------------------------
    inline uint64_t GetKnifeSubclassID(int defIndex) {
        switch (defIndex) {
            case 500: return 3933374535ull;
            case 503: return 3787235507ull;
            case 505: return 4046390180ull;
            case 506: return 2047704618ull;
            case 507: return 1731408398ull;
            case 508: return 1638561588ull;
            case 509: return 2282479884ull;
            case 512: return 3412259219ull;
            case 514: return 2511498851ull;
            case 515: return 1353709123ull;
            case 516: return 4269888884ull;
            case 517: return 1105782941ull;
            case 518: return 275962944ull;
            case 519: return 1338637359ull;
            case 520: return 3230445913ull;
            case 521: return 3206681373ull;
            case 522: return 2595277776ull;
            case 523: return 4029975521ull;
            case 524: return 2463111489ull;
            case 525: return 365028728ull;
            case 526: return 3845286452ull;
            default:  return 0ull;
        }
    }

    // ---------------------------------------------------------------
    // Force-load knife model on the equipped weapon entity.
    // Mirrors ARCHILIX: spoof ownership (account id + item id), set
    // the per-knife subclass id hash, then SetModel + mesh mask.
    // ---------------------------------------------------------------
    inline void ApplyKnifeModelSwap(uintptr_t weapon, int targetDefIndex)
    {
        if (!weapon || targetDefIndex <= 0) return;
        // UpdateSubclass is OPTIONAL — the published "UpdateSubclass" sig
        // (4C 8B DC 53 48 81 EC ...) actually resolves to a subclass-data
        // ERROR LOGGER (sub_1801FA880 in current build) that can DELETE
        // the entity. We refuse to gate on it. SetModel + SetMeshGroupMask
        // alone are sufficient for the world-mesh swap.
        if (!SetModel || !SetMeshGroupMask) return;

        const char* modelPath = GetKnifeModelPath(targetDefIndex);
        if (!modelPath) return;

        uint64_t subclassId = GetKnifeSubclassID(targetDefIndex);
        if (!subclassId) return;

        __try {
            uintptr_t item = weapon + Offsets::m_AttributeManager + Offsets::m_Item;

            // Pull local player's account id from any owned weapon (XuidLow)
            uint32_t accountId = Mem::Read<uint32_t>(weapon + Offsets::m_OriginalOwnerXuidLow);
            if (!accountId) accountId = 0xFFFFFFFFu; // fallback

            // 1. Item identity \u2014 spoof ownership so the game accepts the swap
            Mem::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
            Mem::Write<int32_t>(item  + Offsets::m_iEntityQuality, 3);
            Mem::Write<uint64_t>(item + Offsets::m_iItemID,     0xF000000000000000ull | (uint64_t)targetDefIndex);
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFFu);
            Mem::Write<uint32_t>(item + Offsets::m_iItemIDLow,  0xFFFFFFFFu);
            Mem::Write<uint32_t>(item + Offsets::m_iAccountID,  accountId);
            Mem::Write<bool>(item + Offsets::m_bRestoreCustomMaterialAfterPrecache, true);
            Mem::Write<bool>(item + Offsets::m_bDisallowSOC,    false);
            Mem::Write<bool>(item + Offsets::m_bInitialized,    true);

            // 2. Subclass id hash \u2014 critical, server validates this
            Mem::Write<uint64_t>(weapon + Offsets::m_nSubclassID, subclassId);

            // 3. SetModel(weapon, vmdl)
            SetModel(weapon, modelPath);

            // 4. SetMeshGroupMask(scene, mask) -- mask MUST match the
            //    model loaded above. Legacy CSGO-era meshes (def 503
            //    Classic Knife / 42 / 59) need mask=2; all modern CS2
            //    knives use mask=1. Per the UC forum post: passing the
            //    wrong mask makes the new mesh fail to bind entirely.
            const uint64_t kMeshMask = MeshMaskForDef(targetDefIndex);
            uintptr_t sceneNode = Mem::Read<uintptr_t>(weapon + Offsets::m_pGameSceneNode);
            if (sceneNode)
                SetMeshGroupMask(sceneNode, kMeshMask);

            // 5. UpdateWeaponData — refreshes the weapon's bound VData
            //    (CCSWeaponBaseVData) to match the new m_nSubclassID we
            //    just wrote. Without this the world+view weapon keep the
            //    PREVIOUS knife's animgraph (m_szAnimExtension /
            //    m_szAnimClass) so e.g. a Karambit plays default-knife
            //    inspect/deploy/swing animations. Calling it forces the
            //    animation system to re-resolve sequences against the
            //    new subclass.
            if (UpdateWeaponData) UpdateWeaponData(weapon);

            // 6. SetBodygroup(weapon, 0, 0) — forces the renderer to drop the
            //    cached mesh from the previous def-index and rebind to the new
            //    model loaded by SetModel. Without this the world+view weapon
            //    keeps the original mesh even though SetModel "succeeded" and
            //    the animation/pose comes from the new subclass id.
            //    Ghidra @ 0x1808E0610: void CBaseModelEntity::SetBodygroup(int,int)
            if (SetBodyGroup) SetBodyGroup(weapon, 0, 0);

            // 7. Viewmodel-attachment best-effort.
            //    C_EconEntity has a m_hViewmodelAttachment @ 0x1688 pointing at
            //    the third-person attached mesh entity used while inspecting/
            //    holstering. Forcing SetModel on it picks up the new vmdl too,
            //    which fixes the "knife flips like a Karambit but stays default
            //    in the holster" half of the prior bug. The actual first-person
            //    viewmodel is owned by CCSPlayer_ViewModelServices and only
            //    rebinds its mesh on a fresh Deploy — see step 8.
            constexpr std::ptrdiff_t kViewmodelAttachment = 0x1688; // C_EconEntity::m_hViewmodelAttachment
            uint32_t vmaHandle = Mem::Read<uint32_t>(weapon + kViewmodelAttachment);
            if (vmaHandle && vmaHandle != 0xFFFFFFFFu) {
                uintptr_t vma = GameState::ResolveHandle(vmaHandle);
                if (vma && vma > 0x10000) {
                    SetModel(vma, modelPath);
                    uintptr_t vmaScene = Mem::Read<uintptr_t>(vma + Offsets::m_pGameSceneNode);
                    // Same mask as the world weapon -- the viewmodel
                    // attachment loads the SAME vmdl, so it must use
                    // the SAME legacy bit, otherwise its mesh fails
                    // to bind exactly like the world model would.
                    if (vmaScene) SetMeshGroupMask(vmaScene, kMeshMask);
                    if (SetBodyGroup) SetBodyGroup(vma, 0, 0);
                }
            }

            // 8. Force a re-deploy so the animgraph rebinds against the
            //    new spoofed subclass.
            //
            //    These deploy-tick clears force the active weapon think
            //    to treat the next tick as a fresh deploy:
            Mem::Write<int32_t>(weapon + 0x17F8, 0);  // m_nDeployTick = 0 (force re-deploy think)
            Mem::Write<bool>(weapon + 0x1804, false); // m_bIsHauledBack = false

            // Also re-arm the third-person regen gate so the world material
            // rebinds on the same tick (otherwise FPV catches up but the
            // 3rd-person view still shows the previous skin).
            Mem::Write<int32_t>(weapon + 0xAA8, 0);
            Mem::Write<int32_t>(weapon + 0xAC0, 0);

            // 9. ANIMGRAPH FORCE-REBUILD (build 14155, IDA-verified).
            //
            //    After UpdateWeaponData refreshes the vdata pointer the
            //    weapon's CBaseAnimGraphController STILL holds the old
            //    knife's CNmGraphInstance (e.g. default-knife idle/grip).
            //    That is why the karambit shows but is held in the wrong
            //    fist position with default-knife inspect / swing anims.
            //
            //    Fix: tear down the existing graph instance and let the
            //    AnimGraphControllerManager rebind from the now-current
            //    vdata. See ANIMGRAPH FORCE-REBUILD comment block above
            //    for the full reverse-engineering rationale.
            //
            //    Defensive checks:
            //      * controller pointer non-null and looks valid
            //      * existing graph instance non-null (no point destroying
            //        a graph that doesn't exist; also avoids triggering
            //        the rebuild path on a partially-initialised entity)
            //      * SEH wrap so any internal manager-state mismatch
            //        downgrades to "no rebuild" instead of crashing.
            if (AnimGraphRebuild) {
                __try {
                    uintptr_t controller = Mem::Read<uintptr_t>(weapon + kMainGraphController_OFF);
                    if (controller > 0x10000) {
                        uintptr_t graphInstance = Mem::Read<uintptr_t>(controller + kGraphInstanceAG2_OFF);
                        if (graphInstance > 0x10000) {
                            AnimGraphRebuild(controller, 2);
                            SkLog("[Knife] AnimGraphRebuild OK ctrl=0x%llX inst=0x%llX",
                                  (unsigned long long)controller,
                                  (unsigned long long)graphInstance);
                        } else {
                            SkLog("[Knife] AnimGraphRebuild SKIP: no graph instance (ctrl=0x%llX)",
                                  (unsigned long long)controller);
                        }
                    } else {
                        SkLog("[Knife] AnimGraphRebuild SKIP: bad controller ptr 0x%llX",
                              (unsigned long long)controller);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    SkLog("[Knife] AnimGraphRebuild EXCEPTION on weapon=0x%llX",
                          (unsigned long long)weapon);
                }
            }

            char buf[160];
            wsprintfA(buf, "[SkinChanger] knife model swap: def=%d path=%s scene=0x%llX\n",
                targetDefIndex, modelPath, (unsigned long long)sceneNode);
            OutputDebugStringA(buf);
            SkLog("[Knife] SetModel(weapon=0x%llX, '%s'); MeshMask(scene=0x%llX,1); UpdateSubclass(item=0x%llX); newDef=%u",
                (unsigned long long)weapon, modelPath,
                (unsigned long long)sceneNode, (unsigned long long)item,
                (unsigned)Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex));
        } __except (EXCEPTION_EXECUTE_HANDLER) { SkLog("[Knife] EXCEPTION in swap"); }
    }

    // ---------------------------------------------------------------
    // Force-load glove model on the local pawn (build 14154 approach).
    //
    // ROOT CAUSE OF PRIOR BREAKAGE:
    // The visible glove on your hands is NOT the wearable in m_hMyWearables.
    // It is a *dynamically spawned* C_WorldModelGloves entity that is
    // bonemerged onto the pawn each time pawn->m_bNeedToReApplyGloves
    // flips true. The code that does the spawn (sub_180BBFAA0) reads
    // the GLOVE IDENTITY from the EMBEDDED CEconItemView at pawn+0x1658
    // (m_EconGloves) — NOT from any wearable entity.
    //
    // So the correct (and only reliable) way to swap gloves is:
    //   1. Write the desired def-index into pawn + m_EconGloves + 0x1BA
    //      (m_iItemDefinitionIndex inside the embedded EconItemView).
    //   2. Set the identity-spoof flags on that embedded view so the
    //      game treats it as a real owned item.
    //   3. Set pawn[m_bNeedToReApplyGloves] = true.
    //   4. Do NOTHING else. Next tick, the game's per-tick orchestrator
    //      (sub_180BC2620 → sub_180BBFAA0) will:
    //        - destroy any existing C_WorldModelGloves
    //        - read the new def-index from m_EconGloves
    //        - resolve gloves/paints/<...>.vmdl path
    //        - spawn a fresh C_WorldModelGloves with parentName=<pawn>,
    //          parentAttachmentName="!bonemerge", useLocalOffset=true
    //        - apply g_flWearAmount, g_nRandomSeed, g_nRandomSeedAlt,
    //          econ_instance material params
    //
    // We also still walk m_hMyWearables and update any pre-existing
    // wearable entity's def-index for consistency (so that any code
    // reading the wearable list sees the new identity), but the
    // visible mesh comes from the spawn pipeline above.
    // ---------------------------------------------------------------
    inline void ApplyGloveModelSwap(uintptr_t localPawn, int targetDefIndex)
    {
        if (!localPawn || targetDefIndex <= 0) return;

        __try {
            // Layout inside the embedded CEconItemView at pawn + m_EconGloves:
            //   +0x1BA  m_iItemDefinitionIndex   (uint16)
            //   +0x1B8  m_bRestoreCustomMaterialAfterPrecache
            //   +0x1D0  m_iItemIDHigh
            //   +0x1D8  m_iAccountID
            //   +0x1E8  m_bInitialized
            //   +0x208  m_AttributeList (for paint kit attributes)
            uintptr_t econGloves = localPawn + Offsets::m_EconGloves;

            // Write target glove identity directly into the embedded view.
            Mem::Write<uint16_t>(econGloves + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
            Mem::Write<int32_t>(econGloves + Offsets::m_iEntityQuality, 3);
            Mem::Write<uint64_t>(econGloves + Offsets::m_iItemID,
                                 0xF000000000000000ull | (uint64_t)targetDefIndex);
            Mem::Write<uint32_t>(econGloves + Offsets::m_iItemIDHigh, 0xFFFFFFFFu);
            Mem::Write<uint32_t>(econGloves + Offsets::m_iItemIDLow,  0xFFFFFFFFu);

            uint32_t accountId = Mem::Read<uint32_t>(econGloves + Offsets::m_iAccountID);
            if (!accountId) accountId = 0xFFFFFFFFu;
            Mem::Write<uint32_t>(econGloves + Offsets::m_iAccountID, accountId);
            Mem::Write<bool>(econGloves + Offsets::m_bRestoreCustomMaterialAfterPrecache, true);
            Mem::Write<bool>(econGloves + Offsets::m_bDisallowSOC, false);
            Mem::Write<bool>(econGloves + Offsets::m_bInitialized, true);

            // Trigger flag — game's per-tick orchestrator (sub_180BBFAA0)
            // reads this byte, destroys the old C_WorldModelGloves entity,
            // and spawns a fresh one bonemerged to the pawn using the
            // identity we just wrote above.
            Mem::Write<uint8_t>(localPawn + Offsets::m_bNeedToReApplyGloves, 1);

            // ALSO update any wearable entity in m_hMyWearables so any
            // code reading the list sees the new identity. The wearable
            // is NOT what's rendered (the spawned C_WorldModelGloves is)
            // but keeping them in sync avoids server-side desync warnings.
            uintptr_t wearablesBase = localPawn + Offsets::m_hMyWearables;
            int32_t   wCount = Mem::Read<int32_t>(wearablesBase + 0x00);
            uintptr_t wData  = Mem::Read<uintptr_t>(wearablesBase + 0x08);
            if (wCount > 0 && wCount < 16 && wData) {
                for (int i = 0; i < wCount; ++i) {
                    uint32_t h = Mem::Read<uint32_t>(wData + i * 4);
                    uintptr_t e = GameState::ResolveHandle(h);
                    if (!e || e < 0x10000) continue;
                    uintptr_t it = e + Offsets::m_AttributeManager + Offsets::m_Item;
                    uint16_t  d  = Mem::Read<uint16_t>(it + Offsets::m_iItemDefinitionIndex);
                    bool isGlove = (d >= 5027 && d <= 5035);
                    if (isGlove) {
                        Mem::Write<uint16_t>(it + Offsets::m_iItemDefinitionIndex, (uint16_t)targetDefIndex);
                        Mem::Write<int32_t>(it  + Offsets::m_iEntityQuality, 3);
                        Mem::Write<uint64_t>(it + Offsets::m_iItemID,
                                             0xF000000000000000ull | (uint64_t)targetDefIndex);
                        Mem::Write<uint32_t>(it + Offsets::m_iItemIDHigh, 0xFFFFFFFFu);
                        Mem::Write<uint32_t>(it + Offsets::m_iAccountID, accountId);
                        Mem::Write<bool>(it + Offsets::m_bInitialized, true);
                        break;
                    }
                }
            }

            static int s_gloveSwapLog = 0;
            if (s_gloveSwapLog < 5) {
                SkLog("[Glove] m_EconGloves write: pawn=0x%llX def=%d (m_bNeedToReApplyGloves set)",
                      (unsigned long long)localPawn, targetDefIndex);
                s_gloveSwapLog++;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            SkLog("[Glove] EXCEPTION in ApplyGloveModelSwap");
        }
    }

    inline void ApplyGloveSkin(uintptr_t localPawn, int paintKit, float wear)
    {
        __try {
            // m_hMyWearables is CNetworkUtlVectorBase<CHandle> on C_BaseCombatCharacter
            uintptr_t wearablesBase = localPawn + Offsets::m_hMyWearables;
            int32_t wearableCount = Mem::Read<int32_t>(wearablesBase + 0x00);
            uintptr_t wearablesData = Mem::Read<uintptr_t>(wearablesBase + 0x08);

            if (wearableCount > 0 && wearableCount < 16 && wearablesData) {
                for (int w = 0; w < wearableCount; w++) {
                    uint32_t wearableHandle = Mem::Read<uint32_t>(wearablesData + w * 4);
                    uintptr_t wearable = GameState::ResolveHandle(wearableHandle);
                    if (!wearable) continue;

                    uintptr_t wearItem = wearable + Offsets::m_AttributeManager + Offsets::m_Item;
                    uint16_t wearDefIdx = Mem::Read<uint16_t>(wearItem + Offsets::m_iItemDefinitionIndex);
                    bool isGlove = (wearDefIdx >= 5027 && wearDefIdx <= 5033);

                    if (isGlove || w == 0) {
                        // Persistent fallback — same as weapon skins
                        Mem::Write<uint32_t>(wearItem + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                        Mem::Write<int32_t>(wearable + Offsets::m_nFallbackPaintKit, paintKit);
                        Mem::Write<float>(wearable + Offsets::m_flFallbackWear, wear);
                        Mem::Write<int32_t>(wearable + Offsets::m_nFallbackSeed, 0);
                        Mem::Write<int32_t>(wearable + Offsets::m_nFallbackStatTrak, -1);
                        break;
                    }
                }
            }

            // Signal game to reapply gloves
            Mem::Write<bool>(localPawn + Offsets::m_bNeedToReApplyGloves, true);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Sync function to convert menu config to internal system
    // ---------------------------------------------------------------
    inline void SyncConfigs() {
        std::lock_guard<std::mutex> lock(configMutex);
        
        // Sync weapon configs from menu to internal system
        for (int i = 0; i < kWeaponCount && i < 32; ++i) {
            int defIndex = kWeapons[i].defIndex;
            const auto& oldSkin = cfg.weapons[i];
            
            if (oldSkin.enabled) {
                weaponSkins[defIndex] = {
                    oldSkin.paintKit,
                    oldSkin.wear,
                    oldSkin.seed,
                    oldSkin.statTrak,
                    true
                };
            } else {
                weaponSkins[defIndex].enabled = false;
            }
        }
    }

    // ---------------------------------------------------------------
    // Main functions
    // ---------------------------------------------------------------
    inline void Init() {
        running = true;
        
        // Initialize with some default skins for testing
        weaponSkins[7] = {38, 0.001f, 0, -1, true};   // AK-47 Fade (ENABLED)
        weaponSkins[4] = {38, 0.001f, 0, -1, true};   // Glock Fade (ENABLED)
        weaponSkins[9] = {344, 0.001f, 0, -1, true};  // AWP Dragon Lore (ENABLED)
        weaponSkins[16] = {279, 0.001f, 0, -1, true}; // M4A4 Asiimov (ENABLED)
        
        // Enable knife changer by default for testing
        cfg.enabled = true;
        cfg.knifeEnabled = true;
        cfg.knifeModel = 7; // Karambit (index 7 in kKnives array)
        cfg.knifePaintKit = 38; // Fade
        cfg.knifeSeed = 0;
        cfg.knifeWear = 0.001f;
        cfg.knifeStatTrak = -1;
        
        srand((unsigned)(__rdtsc() & 0xFFFFFFFF));
    }

    inline void Tick() {
        // ---- early-bail diagnostic: per-reason cap so one noisy reason
        //      doesn't suppress logs of later (different) bail reasons.
        static int s_bailLogCfg     = 0;
        static int s_bailLogClient  = 0;
        static int s_bailLogPawn    = 0;
        static int s_bailLogHealth  = 0;
        static int s_bailLogWeapon  = 0;
        static int s_bailLogIsWep   = 0;
        // Track previous values so we always log VALUE TRANSITIONS even
        // after the cap is hit -- otherwise a transient state at startup
        // hides the real per-tick state once gameplay starts.
        static int s_lastHealth = -999;
        static int s_lastLife   = -1;
        static int s_lastDef    = -1;
        auto bail = [&](const char* why, int& counter) {
            if (counter < 30) {
                SkLog("[Tick-bail] %s cfg.enabled=%d running=%d clientBase=0x%llX",
                      why, cfg.enabled ? 1 : 0, running.load() ? 1 : 0,
                      (unsigned long long)GameState::clientBase);
                counter++;
            }
        };

        if (!cfg.enabled || !running) { bail("cfg/running", s_bailLogCfg); return; }
        if (!GameState::clientBase) { bail("clientBase", s_bailLogClient); return; }

        // Sync menu configs to internal system
        SyncConfigs();

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) {
            bail("localPawn", s_bailLogPawn);
            lastAppliedWeapon = 0;
            lastAppliedKit = 0;
            return;
        }

        uint8_t lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
        int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);

        // Always log VALUE TRANSITIONS (even past the bail cap)
        if ((int)lifeState != s_lastLife || health != s_lastHealth) {
            SkLog("[Tick] life=%u hp=%d (was life=%d hp=%d)",
                  (unsigned)lifeState, health, s_lastLife, s_lastHealth);
            s_lastLife = (int)lifeState;
            s_lastHealth = health;
        }

        if (lifeState != 0 || health <= 0) {
            bail("life/health", s_bailLogHealth);
            lastAppliedWeapon = 0;
            lastAppliedKit = 0;
            return;
        }

        InitRegen();
        InitModelFunctions();

        bool force = forceUpdate.load();
        std::lock_guard<std::mutex> lock(configMutex);

        // ---------------------------------------------------------------
        // GLOVE CHANGER — force model load + skin
        // ---------------------------------------------------------------
        if (cfg.gloveEnabled && cfg.gloveModel > 0 && cfg.gloveModel < kGloveCount) {
            int targetGloveDef = kGloves[cfg.gloveModel].defIndex;
            // Re-apply every tick until it sticks (game may revert)
            ApplyGloveModelSwap(localPawn, targetGloveDef);
            lastGloveModelEntity = localPawn;
            lastGloveModelDef    = targetGloveDef;
            if (cfg.glovePaintKit > 0)
                ApplyGloveSkin(localPawn, cfg.glovePaintKit, cfg.gloveWear);
        } else {
            lastGloveModelEntity = 0;
            lastGloveModelDef    = 0;
        }

        // ---------------------------------------------------------------
        // WEAPON/KNIFE CHANGER
        // ---------------------------------------------------------------
        // Build 14152+ removed m_pClippingWeapon. Resolve the active weapon
        // via WeaponServices->m_hActiveWeapon (centralised in game_state.h).
        uintptr_t activeWeapon = GameState::GetActiveWeapon(localPawn);
        if (!activeWeapon) { bail("activeWeapon", s_bailLogWeapon); return; }

        uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
        uint16_t defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);

        // Always log defIndex transitions even past the bail cap
        if ((int)defIndex != s_lastDef) {
            SkLog("[Tick] weapon=0x%llX def=%u (was %d)",
                  (unsigned long long)activeWeapon, (unsigned)defIndex, s_lastDef);
            s_lastDef = (int)defIndex;
        }

        bool isWeapon = (defIndex > 0 && defIndex < 70) || (defIndex >= 500 && defIndex < 600);
        if (!isWeapon || defIndex == 31) { bail("isWeapon", s_bailLogIsWep); return; } // Skip grenades

        int lookupIndex = defIndex;

        // ---- one-shot diagnostic: log the active weapon + cfg state on first
        //      tick where we have a valid pawn + weapon. Lets us pinpoint why
        //      the apply paths might not be triggering (cfg.enabled false, no
        //      weapon match in map, etc.)
        {
            static int s_diagOnce = 0;
            if (s_diagOnce < 6) {
                bool isKnife = IsKnife(defIndex);
                int  wsCount = 0;
                bool wsHit   = false;
                int  wsKit   = 0;
                for (auto& kv : weaponSkins) {
                    if (kv.second.enabled && kv.second.paintKit > 0) ++wsCount;
                    if (kv.first == lookupIndex && kv.second.enabled) {
                        wsHit = true; wsKit = kv.second.paintKit;
                    }
                }
                SkLog("[Diag] activeWeapon=0x%llX def=%u isKnife=%d cfg.enabled=%d "
                      "cfg.knifeEnabled=%d cfg.knifeModel=%d cfg.knifePaintKit=%d "
                      "weaponSkins.size=%zu enabledCount=%d hit=%d hitKit=%d",
                      (unsigned long long)activeWeapon, (unsigned)defIndex, isKnife ? 1 : 0,
                      cfg.enabled ? 1 : 0, cfg.knifeEnabled ? 1 : 0,
                      cfg.knifeModel, cfg.knifePaintKit,
                      weaponSkins.size(), wsCount, wsHit ? 1 : 0, wsKit);
                s_diagOnce++;
            }
        }
        
        // ---------------------------------------------------------------
        // KNIFE SKIN — apply paint kit to equipped knife via ApplyAndRegen
        // NOTE: Knife MODEL changing (e.g. Karambit) is server-authoritative
        // and cannot be done client-side. Only knife SKINS work.
        // ---------------------------------------------------------------
        if (IsKnife(defIndex) && cfg.knifeEnabled) {
            // Resolve target knife def-index from menu selection
            int targetKnifeDef = (cfg.knifeModel > 0 && cfg.knifeModel < kKnifeCount)
                ? kKnives[cfg.knifeModel].defIndex
                : 0;

            // 1. MODEL SWAP — re-swap every tick until on-entity def matches target.
            //    Game's network update can revert def-index, so caching is unsafe.
            if (targetKnifeDef > 0 && (int)defIndex != targetKnifeDef) {
                static int s_swapLogCount = 0;
                if (s_swapLogCount < 5) {
                    SkLog("[Knife] swap attempt: weapon=0x%llX curDef=%u targetDef=%d",
                        (unsigned long long)activeWeapon, (unsigned)defIndex, targetKnifeDef);
                    s_swapLogCount++;
                }
                ApplyKnifeModelSwap(activeWeapon, targetKnifeDef);
                lastKnifeModelEntity = activeWeapon;
                lastKnifeModelDef    = targetKnifeDef;
                // Re-read after swap
                defIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
            }

            // 2. SKIN APPLY (paint-kit/wear/seed) — only when paint kit is set
            if (cfg.knifePaintKit > 0) {
                SkinConfig knifeSkin;
                knifeSkin.paintKit = cfg.knifePaintKit;
                knifeSkin.wear     = cfg.knifeWear;
                knifeSkin.seed     = cfg.knifeSeed;
                knifeSkin.statTrak = cfg.knifeStatTrak;
                knifeSkin.enabled  = true;

                bool needsApply = force || (activeWeapon != lastAppliedWeapon) || (cfg.knifePaintKit != lastAppliedKit);
                if (needsApply) {
                    lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
                    health    = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
                    if (lifeState == 0 && health > 0) {
                        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0);
                        ApplyAndRegen(activeWeapon, knifeSkin, defIndex);
                        lastAppliedWeapon = activeWeapon;
                        lastAppliedKit    = cfg.knifePaintKit;
                    } else {
                        static int s_kSkipLog = 0;
                        if (s_kSkipLog < 5) {
                            SkLog("[Paint-skip] knife life=%u hp=%d", (unsigned)lifeState, health);
                            s_kSkipLog++;
                        }
                    }
                } else {
                    static int s_kCacheLog = 0;
                    if (s_kCacheLog < 3) {
                        SkLog("[Paint-skip] knife cached weapon=0x%llX kit=%d (last=0x%llX/%d)",
                              (unsigned long long)activeWeapon, cfg.knifePaintKit,
                              (unsigned long long)lastAppliedWeapon, lastAppliedKit);
                        s_kCacheLog++;
                    }
                }
            }
        }
        else {
            // Handle regular weapons — use proven ApplyAndRegen approach
            auto it = weaponSkins.find(lookupIndex);
            if (it == weaponSkins.end() || !it->second.enabled || it->second.paintKit <= 0) {
                static int s_wMissLog = 0;
                if (s_wMissLog < 8) {
                    SkLog("[Paint-skip] weapon def=%d not in enabled skin map (size=%zu)",
                          lookupIndex, weaponSkins.size());
                    s_wMissLog++;
                }
            } else {
                const SkinConfig& skin = it->second;
                bool needsApply = force || (activeWeapon != lastAppliedWeapon) || (skin.paintKit != lastAppliedKit);

                if (needsApply) {
                    lifeState = Mem::Read<uint8_t>(localPawn + Offsets::m_lifeState);
                    health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
                    if (lifeState == 0 && health > 0) {
                        // Reset ItemIDHigh before apply (reference approach)
                        Mem::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0);
                        // Write-regen-cleanup cycle
                        ApplyAndRegen(activeWeapon, skin, defIndex);
                        lastAppliedWeapon = activeWeapon;
                        lastAppliedKit = skin.paintKit;
                    }
                }
            }
        }

        if (force) forceUpdate.store(false);
    }

    // ---------------------------------------------------------------
    // Menu interface functions
    // ---------------------------------------------------------------
    inline void RandomizeAll() {
        std::lock_guard<std::mutex> lock(configMutex);
        
        cfg.enabled = true;
        
        // Popular paint kits
        int kits[] = {12, 38, 44, 77, 135, 279, 309, 344, 409, 415, 417, 418, 433, 475, 524, 597, 637, 735, 811, 846};
        int kitCount = sizeof(kits) / sizeof(kits[0]);
        
        // Randomize weapons in menu config
        for (int i = 0; i < kWeaponCount && i < 32; ++i) {
            cfg.weapons[i].enabled = true;
            cfg.weapons[i].paintKit = kits[rand() % kitCount];
            cfg.weapons[i].seed = rand() % 1000;
            cfg.weapons[i].wear = 0.0001f;
            cfg.weapons[i].statTrak = (rand() % 4 == 0) ? (rand() % 500) : -1;
        }
        
        // Randomize knife
        cfg.knifeEnabled = true;
        cfg.knifeModel = 1 + (rand() % (kKnifeCount - 1));
        cfg.knifePaintKit = kits[rand() % kitCount];
        cfg.knifeSeed = rand() % 1000;
        cfg.knifeWear = 0.0001f;
        cfg.knifeStatTrak = (rand() % 3 == 0) ? (rand() % 500) : -1;
        
        forceUpdate.store(true);
    }

    inline void ForceFullUpdate() {
        forceUpdate.store(true);
        lastAppliedWeapon = 0;
        lastAppliedKit = 0;
    }
}

// Namespace alias for compatibility
namespace SkinChangerTest = SkinChanger;