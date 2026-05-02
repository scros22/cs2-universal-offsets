#pragma once
// ---------------------------------------------------------------
// InventoryChanger â€” main-menu locker injection
// Ported from ARCHILIX V2 (features/skins).
//
// Strategy (matches ARCHILIX exactly):
//   1) Resolve CCSInventoryManager::GetInstance via signature.
//   2) GetLocalInventory() vfunc â†’ CCSPlayerInventory*.
//   3) For each item the user wants in the locker:
//        - CEconItem* p = CEconItem::CreateInstance();   (sig)
//        - Set m_ulID / m_unInventory / m_unAccountID / m_unDefIndex / quality / rarity
//        - Optionally set paint kit/seed/wear via SetDynamicAttributeValue (sig)
//        - inventory->AddEconItem(p)  (uses GetSOCache â†’ CreateBaseTypeCache â†’ AddObject + SOCreated)
//   4) Install a MinHook on CCSPlayerInventory::GetItemInLoadout so when the
//      menu fetches our injected item, m_bInitialized is forced true.
//
// All sigs taken from ARCHILIX source; if a sig fails to resolve, the
// system simply no-ops (game stays vanilla, no crash).
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <mutex>
#include <cstdio>
#include <cstdarg>

#include "../../core/memory.h"
#include "../../core/sdk_offsets.h"
#include "../../vendor/minhook/include/MinHook.h"
#include "skinchanger.h"

namespace InventoryChanger
{
    // ---------------------------------------------------------------
    // Tiny logger â€” appends to %TEMP%\inventory_changer.log
    // ---------------------------------------------------------------
    inline void IcLog(const char* fmt, ...)
    {
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "inventory_changer.log");
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
    // Game struct layouts (mirrors ARCHILIX CCPlayerInventory.h / CEconItem.h)
    // ---------------------------------------------------------------
    struct SOID_t
    {
        uint64_t m_id;
        uint32_t m_type;
        uint32_t m_padding;
    };

    // We only touch field offsets we need.  Layout from ARCHILIX:
    //   pad0[0x10]; m_ulID; m_ulOriginalID; m_pCustomDataOptimizedObject;
    //   m_unAccountID; m_unInventory; m_unDefIndex;
    //   uint16_t bitfield (origin/quality/level/rarity/dirty);
    //   m_iItemSet; m_bSOUpdateFrame; m_unFlags;
#pragma pack(push, 1)
    struct CEconItem
    {
        char     pad0[0x10];                        // 0x00
        uint64_t m_ulID;                            // 0x10
        uint64_t m_ulOriginalID;                    // 0x18
        void*    m_pCustomDataOptimizedObject;      // 0x20
        uint32_t m_unAccountID;                     // 0x28
        uint32_t m_unInventory;                     // 0x2C
        uint16_t m_unDefIndex;                      // 0x30
        uint16_t m_bitfield;                        // 0x32  (origin:5,quality:4,level:2,rarity:4,dirty:1)
        int16_t  m_iItemSet;                        // 0x34
        int      m_bSOUpdateFrame;                  // 0x38
        uint8_t  m_unFlags;                         // 0x3C

        void SetQuality(uint8_t q)
        {
            // bits [5..8]
            m_bitfield = (m_bitfield & ~(0xF << 5)) | ((q & 0xF) << 5);
        }
        void SetRarity(uint8_t r)
        {
            // bits [11..14]
            m_bitfield = (m_bitfield & ~(0xF << 11)) | ((r & 0xF) << 11);
        }
    };
#pragma pack(pop)
    static_assert(offsetof(CEconItem, m_ulID)       == 0x10, "CEconItem layout");
    static_assert(offsetof(CEconItem, m_unAccountID) == 0x28, "CEconItem layout");
    static_assert(offsetof(CEconItem, m_unDefIndex) == 0x30, "CEconItem layout");

    // VFunc helper
    template <typename Ret, size_t Idx, typename... Args>
    inline Ret CallVFunc(void* thisptr, Args... args)
    {
        using Fn = Ret(__fastcall*)(void*, Args...);
        return (*reinterpret_cast<Fn**>(thisptr))[Idx](thisptr, args...);
    }

    // ---------------------------------------------------------------
    // Resolved game functions
    // ---------------------------------------------------------------
    using FnCreateEconItem        = CEconItem*(__cdecl*)();
    using FnGetCSInvMgr           = void*(__fastcall*)();
    using FnCreateBaseTypeCache   = void*(__thiscall*)(void*, int);
    using FnGetItemInLoadout      = void*(__fastcall*)(void*, int, int);
    using FnSetDynamicAttribute   = uint64_t(__fastcall*)(void* pItem, void* pAttrDef, void* value);

    inline FnCreateEconItem      g_pCreateEconItem      = nullptr;
    inline FnGetCSInvMgr         g_pGetCSInvMgr         = nullptr;
    inline FnCreateBaseTypeCache g_pCreateBaseTypeCache = nullptr;
    inline FnGetItemInLoadout    g_pGetItemInLoadoutOrig = nullptr;
    inline void*                 g_pGetItemInLoadoutTarget = nullptr;
    inline FnSetDynamicAttribute g_pSetDynamicAttribute = nullptr;

    // ---------------------------------------------------------------
    // Helper: GetAbsoluteAddress (resolve `mov rax, [rip+disp]` / call rel32)
    // ---------------------------------------------------------------
    inline uintptr_t GetAbsAddr(uintptr_t addr, int offsetToOperand, int instructionSize = 5)
    {
        if (!addr) return 0;
        int32_t rel = *reinterpret_cast<int32_t*>(addr + offsetToOperand);
        return addr + instructionSize + rel;
    }

    // ---------------------------------------------------------------
    // Inventory wrappers
    // ---------------------------------------------------------------
    inline void* GetLocalInventory(void* invMgr)
    {
        // CCSInventoryManager::GetLocalInventory @ vfunc 70
        return CallVFunc<void*, 70>(invMgr);
    }

    inline SOID_t GetInventoryOwner(void* inventory)
    {
        // member at +0x10
        return *reinterpret_cast<SOID_t*>(reinterpret_cast<uint8_t*>(inventory) + 0x10);
    }

    inline void* GetSOCache(void* inventory)
    {
        return *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(inventory) + 0x68);
    }

    inline void SOCreated(void* inventory, SOID_t owner, void* obj, int eEvent)
    {
        // CCSPlayerInventory::SOCreated @ vfunc 0
        // signature: void(SOID_t owner, CSharedObject* obj, ESOCacheEvent ev)
        using Fn = void(__fastcall*)(void*, SOID_t, void*, int);
        Fn f = (*reinterpret_cast<Fn**>(inventory))[0];
        f(inventory, owner, obj, eEvent);
    }

    inline bool TypeCacheAddObject(void* tc, void* obj)
    {
        // CGCClientSharedObjectTypeCache::AddObject @ vfunc 1
        return CallVFunc<bool, 1>(tc, obj);
    }

    inline void* CreateBaseTypeCache(void* inventory, int classId = 1)
    {
        void* soc = GetSOCache(inventory);
        if (!soc || !g_pCreateBaseTypeCache) return nullptr;
        return g_pCreateBaseTypeCache(soc, classId);
    }

    inline bool AddEconItem(void* inventory, CEconItem* pItem)
    {
        if (!inventory || !pItem) return false;
        void* tc = CreateBaseTypeCache(inventory, 1);
        if (!tc) { IcLog("[InvCh] CreateBaseTypeCache returned null"); return false; }
        if (!TypeCacheAddObject(tc, pItem)) { IcLog("[InvCh] AddObject returned false"); return false; }
        SOID_t owner = GetInventoryOwner(inventory);
        SOCreated(inventory, owner, pItem, /*eSOCacheEvent_Incremental*/4);
        return true;
    }

    // ---------------------------------------------------------------
    // Econ schema â€” to look up attribute definitions for paint kit etc
    // ---------------------------------------------------------------
    // Resolved via Ghidra (April 2026 build):
    //   FUN_1810D5C50 = GetEconItemSystem()  -> returns ptr where [+8] is the
    //                                          CEconItemSchema*.
    //   FUN_18106EDF0 = CEconItemSchema::GetAttributeDefinitionByName(this,name)
    //                                       -> CEconItemAttributeDefinition*
    using FnGetEconItemSystem    = void**(__fastcall*)();
    using FnGetAttrDefByName     = void*(__fastcall*)(void* schema, const char* name);
    inline FnGetEconItemSystem g_pGetEconItemSystem = nullptr;
    inline FnGetAttrDefByName  g_pGetAttrDefByName  = nullptr;
    inline void* g_pEconSchema = nullptr;

    inline void* GetEconItemSchema()
    {
        if (g_pEconSchema) return g_pEconSchema;
        if (!g_pGetEconItemSystem) return nullptr;
        void** sys = nullptr;
        __try { sys = g_pGetEconItemSystem(); }
        __except (EXCEPTION_EXECUTE_HANDLER) { sys = nullptr; }
        if (!sys) { IcLog("[InvCh] GetEconItemSystem returned null"); return nullptr; }
        // Layout (verified): [0]=vtable, [1]=CEconItemSchema*
        void* pSchema = sys[1];
        IcLog("[InvCh] EconItemSchema = %p (sys=%p)", pSchema, sys);
        g_pEconSchema = pSchema;
        return pSchema;
    }

    // CS2 paint-kit attribute names (verified strings present in client.dll):
    //   index 6 â†’ "set item texture prefab"  (paint kit / pattern)
    //   index 7 â†’ "set item texture seed"    (pattern seed)
    //   index 8 â†’ "set item texture wear"    (float wear)
    //   index 80â†’ "kill eater"               (StatTrak kill count)
    //   index 81â†’ "kill eater score type"
    inline const char* AttrNameForIndex(int idx)
    {
        switch (idx) {
            case 6:  return "set item texture prefab";
            case 7:  return "set item texture seed";
            case 8:  return "set item texture wear";
            case 80: return "kill eater";
            case 81: return "kill eater score type";
            default: return nullptr;
        }
    }

    inline void* GetAttributeDef(int index)
    {
        const char* name = AttrNameForIndex(index);
        if (!name) return nullptr;
        void* schema = GetEconItemSchema();
        if (!schema || !g_pGetAttrDefByName) return nullptr;
        void* def = nullptr;
        __try { def = g_pGetAttrDefByName(schema, name); }
        __except (EXCEPTION_EXECUTE_HANDLER) { def = nullptr; }
        return def;
    }

    inline void SetItemAttribute(CEconItem* pItem, int attrIndex, void* valuePtr)
    {
        if (!g_pSetDynamicAttribute) return;
        void* def = GetAttributeDef(attrIndex);
        if (!def) { IcLog("[InvCh] AttrDef[%d] not resolved", attrIndex); return; }
        __try {
            g_pSetDynamicAttribute(pItem, def, valuePtr);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            IcLog("[InvCh] SetDynamicAttribute crashed for index %d", attrIndex);
        }
    }

    inline void ApplyPaintKit(CEconItem* pItem, int paintKit, int seed, float wear, int statTrak)
    {
        if (paintKit > 0) {
            float pk = (float)paintKit;
            float sd = (float)seed;
            SetItemAttribute(pItem, 6, &pk);    // m_nPaintKit
            SetItemAttribute(pItem, 7, &sd);    // m_nPaintSeed
            SetItemAttribute(pItem, 8, &wear);  // m_flPaintWear
        }
        if (statTrak >= 0) {
            int st = statTrak;
            int sttype = 0;
            SetItemAttribute(pItem, 80, &st);     // m_iStatTrakKills
            SetItemAttribute(pItem, 81, &sttype); // m_iStatTrakType
        }
    }

    // ---------------------------------------------------------------
    // EquipItemInLoadout (CCSInventoryManager vfunc 67)
    // ---------------------------------------------------------------
    inline bool EquipItemInLoadout(int team, int slot, uint64_t itemID)
    {
        if (!g_pGetCSInvMgr) return false;
        void* mgr = g_pGetCSInvMgr();
        if (!mgr) return false;
        bool result = false;
        __try {
            result = CallVFunc<bool, 67>(mgr, team, slot, itemID);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            IcLog("[InvCh] EquipItemInLoadout crashed t=%d s=%d", team, slot);
        }
        IcLog("[InvCh] EquipItemInLoadout team=%d slot=%d id=0x%llX -> %d", team, slot, (unsigned long long)itemID, result?1:0);
        return result;
    }

    // ---------------------------------------------------------------
    // Tracking â€” items we injected so we can remove + so the GetItemInLoadout
    // hook knows which to mark m_bInitialized=true on.
    // ---------------------------------------------------------------
    struct InjectedItem
    {
        uint64_t itemID;
        uint16_t defIndex;
        int      paintKit;
        float    wear;
        int      seed;
        int      statTrak;
    };
    inline std::vector<InjectedItem> g_injected;
    inline std::mutex                g_mutex;

    inline bool IsInjectedID(uint64_t id)
    {
        for (auto& it : g_injected) if (it.itemID == id) return true;
        return false;
    }

    // ---------------------------------------------------------------
    // GetItemInLoadout hook â€” when the locker UI asks the inventory
    // "what's equipped for team T slot S?" and the result is one of
    // our injected items, mark it initialized so the panorama UI binds it.
    // ---------------------------------------------------------------
    inline void* __fastcall HkGetItemInLoadout(void* thisptr, int iTeam, int iSlot)
    {
        void* pItemView = g_pGetItemInLoadoutOrig
            ? g_pGetItemInLoadoutOrig(thisptr, iTeam, iSlot)
            : nullptr;
        if (!pItemView) return pItemView;

        __try {
            // m_iItemID at offset 0x1C8 (per sdk_offsets.h verified earlier)
            uint64_t id = *reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(pItemView) + 0x1C8);
            if (IsInjectedID(id))
            {
                // m_bInitialized at 0x1E8
                *reinterpret_cast<bool*>(reinterpret_cast<uint8_t*>(pItemView) + 0x1E8) = true;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}

        return pItemView;
    }

    // ---------------------------------------------------------------
    // ID generation â€” we write synthetic high-bit IDs so they never
    // collide with real Steam-issued item IDs.
    // ---------------------------------------------------------------
    inline uint64_t g_nextItemId  = 0xF000000000000001ull;
    inline uint32_t g_nextInvId   = 0x1000;

    // ---------------------------------------------------------------
    // Build + inject one econ item
    // ---------------------------------------------------------------
    inline bool InjectItem(void* inventory, uint16_t defIndex,
                           int paintKit, float wear, int seed, int statTrak,
                           uint8_t quality = 4 /*Unique*/, uint8_t rarity = 6 /*Covert*/)
    {
        if (!g_pCreateEconItem) { IcLog("[InvCh] CreateEconItem unresolved"); return false; }

        CEconItem* pItem = g_pCreateEconItem();
        if (!pItem) { IcLog("[InvCh] CreateEconItem returned null"); return false; }

        SOID_t owner = GetInventoryOwner(inventory);

        pItem->m_ulID         = g_nextItemId++;
        pItem->m_ulOriginalID = 0;
        pItem->m_unInventory  = ++g_nextInvId;
        pItem->m_unAccountID  = static_cast<uint32_t>(owner.m_id);
        pItem->m_unDefIndex   = defIndex;
        pItem->SetQuality(quality);
        pItem->SetRarity(rarity);
        pItem->m_unFlags      = 0;

        // Apply paint kit / wear / seed / stattrak BEFORE pushing into SO cache
        ApplyPaintKit(pItem, paintKit, seed, wear, statTrak);

        if (!AddEconItem(inventory, pItem))
        {
            IcLog("[InvCh] AddEconItem failed for def=%u", defIndex);
            return false;
        }

        InjectedItem rec{ pItem->m_ulID, defIndex, paintKit, wear, seed, statTrak };
        {
            std::lock_guard<std::mutex> lk(g_mutex);
            g_injected.push_back(rec);
        }
        IcLog("[InvCh] injected def=%u id=0x%llX paint=%d seed=%d wear=%.4f", defIndex, (unsigned long long)pItem->m_ulID, paintKit, seed, wear);
        return true;
    }

    // ---------------------------------------------------------------
    // Public API
    // ---------------------------------------------------------------
    inline void* GetInventory()
    {
        if (!g_pGetCSInvMgr) return nullptr;
        void* mgr = g_pGetCSInvMgr();
        if (!mgr) return nullptr;
        return GetLocalInventory(mgr);
    }

    // ---------------------------------------------------------------
    // Loadout slot constants (from CS2 schema)
    // ---------------------------------------------------------------
    constexpr int SLOT_KNIFE = 0;     // Melee
    constexpr int SLOT_GLOVES = 41;   // Hands
    // Team constants
    constexpr int TEAM_T  = 2;
    constexpr int TEAM_CT = 3;

    // Build a deterministic item ID for a given defIndex so re-injecting the
    // DLL doesn't double up the locker (the game's SO cache keys by m_ulID
    // and ignores duplicates).  High bit set to keep us out of the real-ID
    // range that Steam allocates.
    inline uint64_t MakeItemID(uint16_t defIndex)
    {
        return 0xF000000000000000ull | (uint64_t)defIndex;
    }

    inline uint32_t MakeInvSlot(uint16_t defIndex)
    {
        return 0x10000u | defIndex;
    }

    // Dedup helper â€” true if we have already injected an item with this defIndex
    inline bool HasInjected(uint16_t defIndex)
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        for (const auto& it : g_injected)
            if (it.defIndex == defIndex) return true;
        return false;
    }

    // Inject the chosen knife + glove + every enabled weapon, with paint
    // kits, and equip the knife/glove for both teams.
    inline int AddFromConfig()
    {
        void* inv = GetInventory();
        if (!inv) { IcLog("[InvCh] GetInventory null"); return 0; }

        const auto& cfg = SkinChanger::cfg;
        int n = 0;

        // ---- KNIFE ----
        uint64_t knifeID = 0;
        if (cfg.knifeEnabled
            && cfg.knifeModel >= 0 && cfg.knifeModel < SkinChanger::kKnifeCount)
        {
            uint16_t kdef = static_cast<uint16_t>(SkinChanger::kKnives[cfg.knifeModel].defIndex);
            if (kdef != 0 && !HasInjected(kdef))
            {
                if (g_pCreateEconItem) {
                    CEconItem* p = g_pCreateEconItem();
                    if (p) {
                        SOID_t owner = GetInventoryOwner(inv);
                        p->m_ulID = MakeItemID(kdef);
                        p->m_ulOriginalID = 0;
                        p->m_unInventory = MakeInvSlot(kdef);
                        p->m_unAccountID = (uint32_t)owner.m_id;
                        p->m_unDefIndex = kdef;
                        p->SetQuality(4);
                        p->SetRarity(6);
                        p->m_unFlags = 0;
                        ApplyPaintKit(p, cfg.knifePaintKit, cfg.knifeSeed, cfg.knifeWear, cfg.knifeStatTrak);
                        if (AddEconItem(inv, p)) {
                            knifeID = p->m_ulID;
                            std::lock_guard<std::mutex> lk(g_mutex);
                            g_injected.push_back({ p->m_ulID, kdef, cfg.knifePaintKit, cfg.knifeWear, cfg.knifeSeed, cfg.knifeStatTrak });
                            n++;
                            IcLog("[InvCh] knife def=%u paint=%d seed=%d id=0x%llX",
                                  kdef, cfg.knifePaintKit, cfg.knifeSeed, (unsigned long long)p->m_ulID);
                        }
                    }
                }
            }
        }

        // ---- GLOVE ----
        uint64_t gloveID = 0;
        if (cfg.gloveEnabled
            && cfg.gloveModel >= 0 && cfg.gloveModel < SkinChanger::kGloveCount)
        {
            uint16_t gdef = static_cast<uint16_t>(SkinChanger::kGloves[cfg.gloveModel].defIndex);
            if (gdef != 0 && !HasInjected(gdef))
            {
                if (g_pCreateEconItem) {
                    CEconItem* p = g_pCreateEconItem();
                    if (p) {
                        SOID_t owner = GetInventoryOwner(inv);
                        p->m_ulID = MakeItemID(gdef);
                        p->m_ulOriginalID = 0;
                        p->m_unInventory = MakeInvSlot(gdef);
                        p->m_unAccountID = (uint32_t)owner.m_id;
                        p->m_unDefIndex = gdef;
                        p->SetQuality(4);
                        p->SetRarity(6);
                        p->m_unFlags = 0;
                        ApplyPaintKit(p, cfg.glovePaintKit, 0, cfg.gloveWear, -1);
                        if (AddEconItem(inv, p)) {
                            gloveID = p->m_ulID;
                            std::lock_guard<std::mutex> lk(g_mutex);
                            g_injected.push_back({ p->m_ulID, gdef, cfg.glovePaintKit, cfg.gloveWear, 0, -1 });
                            n++;
                            IcLog("[InvCh] glove def=%u paint=%d id=0x%llX",
                                  gdef, cfg.glovePaintKit, (unsigned long long)p->m_ulID);
                        }
                    }
                }
            }
        }

        // ---- WEAPONS (only inject ones the user has explicitly enabled
        //               with a non-zero paint kit) ----
        for (int i = 0; i < SkinChanger::kWeaponCount && i < 32; ++i)
        {
            const auto& w = cfg.weapons[i];
            uint16_t wdef = static_cast<uint16_t>(SkinChanger::kWeapons[i].defIndex);
            if (wdef == 0 || HasInjected(wdef)) continue;
            // Only inject when the user actually picked a skin for this weapon.
            // Otherwise we end up flooding the locker with vanilla R8/CZ/USP/etc.
            if (!w.enabled || w.paintKit <= 0) continue;
            if (!g_pCreateEconItem) break;
            CEconItem* p = g_pCreateEconItem();
            if (!p) continue;
            SOID_t owner = GetInventoryOwner(inv);
            p->m_ulID = MakeItemID(wdef);
            p->m_ulOriginalID = 0;
            p->m_unInventory = MakeInvSlot(wdef);
            p->m_unAccountID = (uint32_t)owner.m_id;
            p->m_unDefIndex = wdef;
            p->SetQuality(4);
            p->SetRarity(6);
            p->m_unFlags = 0;
            // Always have a paint kit here (loop guard ensures w.paintKit > 0)
            ApplyPaintKit(p, w.paintKit, w.seed, w.wear, w.statTrak);
            if (AddEconItem(inv, p)) {
                std::lock_guard<std::mutex> lk(g_mutex);
                g_injected.push_back({ p->m_ulID, wdef, w.paintKit, w.wear, w.seed, w.statTrak });
                n++;
                IcLog("[InvCh] weapon def=%u paint=%d id=0x%llX", wdef, w.paintKit, (unsigned long long)p->m_ulID);
            }
        }

        // ---- EQUIP knife + glove for both teams ----
        if (knifeID) {
            EquipItemInLoadout(TEAM_T, SLOT_KNIFE, knifeID);
            EquipItemInLoadout(TEAM_CT, SLOT_KNIFE, knifeID);
        }
        if (gloveID) {
            EquipItemInLoadout(TEAM_T, SLOT_GLOVES, gloveID);
            EquipItemInLoadout(TEAM_CT, SLOT_GLOVES, gloveID);
        }

        IcLog("[InvCh] AddFromConfig done: %d items", n);
        return n;
    }

    // Inject one specific knife with a paint kit
    inline bool AddKnife(uint16_t defIndex, int paintKit, float wear, int seed)
    {
        void* inv = GetInventory();
        if (!inv) { IcLog("[InvCh] GetInventory null"); return false; }
        return InjectItem(inv, defIndex, paintKit, wear, seed, -1, 4, 6);
    }
    inline bool AddGlove(uint16_t defIndex, int paintKit, float wear)
    {
        void* inv = GetInventory();
        if (!inv) { IcLog("[InvCh] GetInventory null"); return false; }
        return InjectItem(inv, defIndex, paintKit, wear, 0, -1, 4, 6);
    }
    inline bool AddWeapon(uint16_t defIndex, int paintKit, float wear, int seed, int statTrak)
    {
        void* inv = GetInventory();
        if (!inv) { IcLog("[InvCh] GetInventory null"); return false; }
        return InjectItem(inv, defIndex, paintKit, wear, seed, statTrak, 4, 6);
    }

    // ---------------------------------------------------------------
    // Init â€” resolve sigs + install MinHook on GetItemInLoadout
    // ---------------------------------------------------------------
    inline std::atomic<bool> g_initialized{ false };

    inline bool Init()
    {
        if (g_initialized) return true;

        IcLog("[InvCh] Init begin");

        // Wait for client.dll to be loaded
        HMODULE hClient = nullptr;
        for (int i = 0; i < 30 && !hClient; ++i)
        {
            hClient = GetModuleHandleW(L"client.dll");
            if (!hClient) Sleep(200);
        }
        if (!hClient) { IcLog("[InvCh] client.dll not loaded after 6s"); return false; }

        // 1) CCSInventoryManager::GetInstance â€” it's a `call rel32` site
        //    Pattern points at the CALL instruction; we need the absolute target.
        uintptr_t pCall = Mem::FindPattern(L"client.dll", "E8 ? ? ? ? 48 8B D8 8B F7");
        IcLog("[InvCh] CCSInventoryManager::GetInstance call site: 0x%llX", (unsigned long long)pCall);
        if (pCall)
        {
            uintptr_t target = GetAbsAddr(pCall, 1, 5);
            g_pGetCSInvMgr = reinterpret_cast<FnGetCSInvMgr>(target);
            IcLog("[InvCh]   resolved â†’ 0x%llX", (unsigned long long)target);
        }

        // 2) CGCClientSharedObjectCache::CreateBaseTypeCache â€” direct function
        uintptr_t pCBTC = Mem::FindPattern(L"client.dll", "40 53 48 83 EC ? 4C 8B 49 ? 44 8B D2");
        IcLog("[InvCh] CreateBaseTypeCache: 0x%llX", (unsigned long long)pCBTC);
        if (pCBTC) g_pCreateBaseTypeCache = reinterpret_cast<FnCreateBaseTypeCache>(pCBTC);

        // 3) CEconItem::CreateInstance â€” function preamble pattern
        uintptr_t pCEI = Mem::FindPattern(L"client.dll", "48 83 EC 28 B9 48 00 00 00 E8 ? ? ? ? 48 85");
        IcLog("[InvCh] CreateEconItem: 0x%llX", (unsigned long long)pCEI);
        if (pCEI) g_pCreateEconItem = reinterpret_cast<FnCreateEconItem>(pCEI);

        // 4) CCSPlayerInventory::GetItemInLoadout â€” direct function, hook target
        uintptr_t pGIL = Mem::FindPattern(L"client.dll", "48 89 5C 24 ? 57 48 83 EC ? 8B DA 48 8B F9 85 D2");
        IcLog("[InvCh] GetItemInLoadout: 0x%llX", (unsigned long long)pGIL);
        if (pGIL)
        {
            g_pGetItemInLoadoutTarget = reinterpret_cast<void*>(pGIL);
            MH_STATUS st = MH_CreateHook(g_pGetItemInLoadoutTarget,
                                         reinterpret_cast<void*>(&HkGetItemInLoadout),
                                         reinterpret_cast<void**>(&g_pGetItemInLoadoutOrig));
            IcLog("[InvCh] MH_CreateHook(GetItemInLoadout) â†’ %d", (int)st);
            if (st == MH_OK || st == MH_ERROR_ALREADY_CREATED)
            {
                MH_STATUS st2 = MH_EnableHook(g_pGetItemInLoadoutTarget);
                IcLog("[InvCh] MH_EnableHook â†’ %d", (int)st2);
            }
        }

        // 5) SetDynamicAttributeValue â€” long sig from ARCHILIX
        uintptr_t pSDA = Mem::FindPattern(L"client.dll",
            "48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B FA C7 44 24 ? ? ? ? ? 4D 8B F8");
        IcLog("[InvCh] SetDynamicAttributeValue: 0x%llX", (unsigned long long)pSDA);
        if (pSDA) g_pSetDynamicAttribute = reinterpret_cast<FnSetDynamicAttribute>(pSDA);

        // 6) GetEconItemSystem (Ghidra-verified @ 0x1810D5C50, CS2 April 2026)
        //    Returns void* where [+8] is the CEconItemSchema*.
        uintptr_t pGES = Mem::FindPattern(L"client.dll",
            "48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? 48 89 5C 24");
        IcLog("[InvCh] GetEconItemSystem: 0x%llX", (unsigned long long)pGES);
        if (pGES) g_pGetEconItemSystem = reinterpret_cast<FnGetEconItemSystem>(pGES);

        // 7) CEconItemSchema::GetAttributeDefinitionByName (Ghidra-verified @ 0x18106EDF0)
        //    Looks up an attribute definition pointer by its display name.
        uintptr_t pGAD = Mem::FindPattern(L"client.dll",
            "48 89 5C 24 10 48 89 6C 24 18 57 41 56 41 57 48 83 EC 60 48 8D 05");
        IcLog("[InvCh] GetAttributeDefByName: 0x%llX", (unsigned long long)pGAD);
        if (pGAD) g_pGetAttrDefByName = reinterpret_cast<FnGetAttrDefByName>(pGAD);

        bool ok = g_pCreateEconItem && g_pGetCSInvMgr && g_pCreateBaseTypeCache;
        IcLog("[InvCh] Init end ok=%d", ok ? 1 : 0);
        g_initialized = ok;
        return ok;
    }

    // ---------------------------------------------------------------
    // Apply a full menu-config snapshot:
    //   - inject every knife + glove (so all show in locker)
    //   - inject the chosen weapon with its paint kit (Randomize-friendly)
    // Designed to be called from a menu button so the user sees immediate effect.
    // Safe to call repeatedly; we just keep adding new items (game tolerates it
    // because each gets a unique m_ulID + m_unInventory).
    // ---------------------------------------------------------------
    inline int ApplyMenuConfig()
    {
        IcLog("[InvCh] ApplyMenuConfig CALLED");
        if (!g_initialized && !Init()) { IcLog("[InvCh] ApplyMenuConfig: Init failed"); return 0; }
        return AddFromConfig();
    }

    // ---------------------------------------------------------------
    // Auto-inject support â€” called every frame from SkinChanger::Tick
    // (which runs on the game thread).  Injects exactly once per
    // session as soon as the local inventory becomes available.
    // ---------------------------------------------------------------
    inline std::atomic<bool> g_autoInjected{ false };
    inline std::atomic<bool> g_autoInjectRequested{ true }; // default ON

    inline void Tick()
    {
        if (!g_autoInjectRequested.load() || g_autoInjected.load()) return;
        if (!g_initialized && !Init()) return;
        void* inv = GetInventory();
        if (!inv) return; // wait until inventory loaded
        IcLog("[InvCh] Tick auto-inject firing");
        int n = AddFromConfig();
        if (n > 0) g_autoInjected.store(true);
    }

    // Allow user to re-trigger injection from the menu
    inline void ResetAutoInject()
    {
        g_autoInjected.store(false);
        g_autoInjectRequested.store(true);
        IcLog("[InvCh] ResetAutoInject â€” will re-inject on next Tick");
    }

    // ---------------------------------------------------------------
    // Shutdown â€” must run BEFORE MH_Uninitialize on DLL detach so the
    // GetItemInLoadout trampoline does not call into freed DLL memory
    // when the game continues to invoke the inventory pump.
    // ---------------------------------------------------------------
    inline void Shutdown()
    {
        if (!g_initialized.load()) return;
        IcLog("[InvCh] Shutdown â€” disabling GetItemInLoadout hook");
        if (g_pGetItemInLoadoutTarget) {
            MH_DisableHook(g_pGetItemInLoadoutTarget);
            MH_RemoveHook(g_pGetItemInLoadoutTarget);
            g_pGetItemInLoadoutTarget = nullptr;
            g_pGetItemInLoadoutOrig = nullptr;
        }
        g_autoInjectRequested.store(false);
        g_initialized.store(false);
    }
}
