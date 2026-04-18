#pragma once
#include <map>
#include <string>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "esp.h" // For access to Config

namespace ModelChanger
{
    struct ModelData
    {
        uint16_t defIndex;
        const char* modelPath;
    };

    // Knives: 1=Kara, 2=Bfly, 3=M9, 4=Bowie
    // Models obtained from common CS2 internal offsets/paths
    inline std::map<int, ModelData> knifeMap = {
        { 1, { 507, "weapons/models/knife/karambit/weapon_knife_karambit.vmdl" } },
        { 2, { 515, "weapons/models/knife/butterfly/weapon_knife_butterfly.vmdl" } },
        { 3, { 508, "weapons/models/knife/m9_bayonet/weapon_knife_m9_bayonet.vmdl" } },
        { 4, { 514, "weapons/models/knife/survival_bowie/weapon_knife_survival_bowie.vmdl" } }
    };

    // Agent Models: 1=Professional, 2=Phoenix, 3=FBI, 4=Ricksaw, 5=Darryl, 6=Ava
    inline std::map<int, const char*> agentModels = {
        { 1, "characters/models/tm_professional_varf.vmdl" },
        { 2, "characters/models/tm_phoenix.vmdl" },
        { 3, "characters/models/ctm_fbi.vmdl" },
        { 4, "characters/models/tm_professional_varg.vmdl" },
        { 5, "characters/models/tm_darryl_vari.vmdl" },
        { 6, "characters/models/ctm_st6_variantk.vmdl" }
    };

    // Gloves: 1=Cobalt Skulls (Wraps), 2=Marble Fade (Specialist), 3=Emerald (Broken Fang)
    inline std::map<int, ModelData> gloveMap = {
        { 1, { 5032, "weapons/models/arms/glove_handwrap_leathery/glove_handwrap_leathery.vmdl" } },
        { 2, { 5034, "weapons/models/arms/glove_specialist/glove_specialist.vmdl" } },
        { 3, { 5033, "weapons/models/arms/glove_bloodhound/glove_bloodhound_brokenfang.vmdl" } }
    };

    inline void ForceRefreshViewmodel(uintptr_t localPawn)
    {
        uintptr_t vmServices = Game::Read<uintptr_t>(localPawn + Offsets::m_pViewModelServices);
        if (!vmServices) return;

        // m_hViewModel is usually an array of handles (0, 1, 2)
        for (int i = 0; i < 3; i++)
        {
            uint32_t vmHandle = Game::Read<uint32_t>(vmServices + Offsets::m_hViewModel + (i * 4));
            if (vmHandle == 0xFFFFFFFF) continue;

            uintptr_t vmEntity = Game::GetEntityFromHandle(vmHandle);
            if (!vmEntity) continue;

            // To force a viewmodel to update its mesh, we can sometimes toggle m_bNeedToReApplyGloves
            // or directly set the model index on the viewmodel.
            // For now, switching weapons is the most reliable way in CS2 to force a mesh reload.
        }
    }

    inline void Run()
    {
        uintptr_t localPawn = Game::GetLocalPlayerPawn();
        if (!localPawn) return;

        // --- Agent Model Changer ---
        if (ESP::config.nPlayerModel > 0)
        {
            auto it = agentModels.find(ESP::config.nPlayerModel);
            if (it != agentModels.end())
            {
                uintptr_t sceneNode = Game::Read<uintptr_t>(localPawn + Offsets::m_pGameSceneNode);
                if (sceneNode)
                {
                    // Internal SetModel call would go here if we had the precise signature
                    // For now, we rely on the scene node update
                }
            }
        }

        // --- Knife & Glove Model Changer ---
        uintptr_t weaponServices = Game::Read<uintptr_t>(localPawn + Offsets::m_pWeaponServices);
        if (weaponServices)
        {
            uintptr_t activeWeapon = Game::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
            if (activeWeapon)
            {
                uintptr_t item = activeWeapon + Offsets::m_AttributeManager + Offsets::m_Item;
                uint16_t defIndex = Game::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);

                bool changed = false;

                // Knife change (Karambit Blue Gem logic)
                if (ESP::config.nKnifeModel == 1 && (defIndex == 42 || defIndex == 59)) // Karambit selected
                {
                    auto it = knifeMap.find(1);
                    Game::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, it->second.defIndex);
                    Game::Write<int>(activeWeapon + Offsets::m_nFallbackPaintKit, 44);
                    Game::Write<int>(activeWeapon + Offsets::m_nFallbackSeed, 387);
                    Game::Write<float>(activeWeapon + Offsets::m_flFallbackWear, 0.001f);
                    Game::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                    changed = true;
                }
                else if (ESP::config.nKnifeModel > 1 && (defIndex == 42 || defIndex == 59))
                {
                    auto it = knifeMap.find(ESP::config.nKnifeModel);
                    if (it != knifeMap.end())
                    {
                        Game::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, it->second.defIndex);
                        Game::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                        changed = true;
                    }
                }

                if (changed) ForceRefreshViewmodel(localPawn);
            }
        }

        // Glove change (Cobalt Skulls Specialist logic)
        if (ESP::config.nGloveModel > 0)
        {
            uintptr_t gloveEntity = Game::Read<uintptr_t>(localPawn + Offsets::m_EconGloves);
            if (gloveEntity)
            {
                auto it = gloveMap.find(ESP::config.nGloveModel);
                if (it != gloveMap.end())
                {
                    uintptr_t item = gloveEntity + Offsets::m_AttributeManager + Offsets::m_Item;
                    Game::Write<uint16_t>(item + Offsets::m_iItemDefinitionIndex, it->second.defIndex);
                    
                    if (ESP::config.nGloveModel == 1) { // Cobalt Skulls specific
                         Game::Write<int>(gloveEntity + Offsets::m_nFallbackPaintKit, 10053);
                         Game::Write<float>(gloveEntity + Offsets::m_flFallbackWear, 0.001f);
                    }

                    Game::Write<uint32_t>(item + Offsets::m_iItemIDHigh, 0xFFFFFFFF);
                    Game::Write<bool>(localPawn + Offsets::m_bNeedToReApplyGloves, true);
                }
            }
        }
    }
}
