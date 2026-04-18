#pragma once
#include <vector>
#include <string>
#include "../imgui/imgui.h"

enum class SkinRarity {
    Consumer = 0,    // Gray
    Industrial,      // Light blue
    MilSpec,         // Blue
    Restricted,      // Purple
    Classified,      // Pink
    Covert,          // Red
    Contraband,      // Gold/Special
    COUNT
};

struct PaintKit {
    int id;
    const char* name;
    SkinRarity rarity;
};

inline ImVec4 GetRarityColor(SkinRarity rarity) {
    switch (rarity) {
        case SkinRarity::Consumer:   return ImVec4(0.69f, 0.75f, 0.78f, 1.0f); // #b0c3d9
        case SkinRarity::Industrial: return ImVec4(0.36f, 0.60f, 0.83f, 1.0f); // #5e98d9
        case SkinRarity::MilSpec:    return ImVec4(0.29f, 0.41f, 0.91f, 1.0f); // #4b69ff
        case SkinRarity::Restricted: return ImVec4(0.53f, 0.28f, 0.83f, 1.0f); // #8847ff
        case SkinRarity::Classified: return ImVec4(0.83f, 0.20f, 0.91f, 1.0f); // #d32ce6
        case SkinRarity::Covert:     return ImVec4(0.92f, 0.30f, 0.30f, 1.0f); // #eb4b4b
        case SkinRarity::Contraband: return ImVec4(0.89f, 0.68f, 0.22f, 1.0f); // #e4ae39
        default:                     return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

inline const std::vector<PaintKit> g_PaintKits = {
    // Contraband / Top Tier
    { 344, "Dragon Lore", SkinRarity::Contraband },
    { 334, "Howl", SkinRarity::Contraband },
    { 44,  "Case Hardened", SkinRarity::Contraband },
    
    // Covert
    { 675, "Bloodsport", SkinRarity::Covert },
    { 696, "Neo-Noir", SkinRarity::Covert },
    { 506, "Kill Confirmed", SkinRarity::Covert },
    { 524, "Fuel Injector", SkinRarity::Covert },
    { 255, "Asiimov", SkinRarity::Covert },
    { 281, "Vulcan", SkinRarity::Covert },
    { 504, "Hyper Beast", SkinRarity::Covert },
    { 639, "Wild Lotus", SkinRarity::Covert },
    { 946, "Printstream", SkinRarity::Covert },
    { 798, "Printstream", SkinRarity::Covert },
    { 14,  "Akihabara Accept", SkinRarity::Covert },
    { 302, "Fire Serpent", SkinRarity::Covert },
    { 688, "Neon Revolution", SkinRarity::Covert },
    { 681, "The Emperor", SkinRarity::Covert },
    { 707, "Neon Rider", SkinRarity::Covert },
    { 342, "Cyrex", SkinRarity::Covert },
    { 664, "Oni Taiji", SkinRarity::Covert },
    { 598, "Chantico's Fire", SkinRarity::Covert },
    { 538, "Mecha Industries", SkinRarity::Covert },
    { 1195, "Head Shot", SkinRarity::Covert },
    { 1145, "Nightwish", SkinRarity::Covert },
    
    // Knives/Rare Patterns
    { 38, "Fade", SkinRarity::Covert },
    { 413, "Marble Fade", SkinRarity::Covert },
    { 568, "Emerald", SkinRarity::Covert },
    { 415, "Ruby", SkinRarity::Covert },
    { 416, "Sapphire", SkinRarity::Covert },
    { 417, "Black Pearl", SkinRarity::Covert },
    { 569, "Lore", SkinRarity::Covert },
    { 573, "Autotronic", SkinRarity::Covert },
    { 561, "Freehand", SkinRarity::Covert },
    { 580, "Freehand", SkinRarity::Covert },

    // Classified
    { 401, "Water Elemental", SkinRarity::Classified },
    { 645, "Corticera", SkinRarity::Classified },
    { 481, "Nemesis", SkinRarity::Classified },
    { 400, "Dragon King", SkinRarity::Classified },
    { 351, "Cartel", SkinRarity::Classified },
    { 1196, "Ice Coaled", SkinRarity::Classified },
    { 1144, "Printstream", SkinRarity::Classified },
    
    // Restricted
    { 60, "Dark Water", SkinRarity::Restricted },
    { 15, "Blizzard Marbleized", SkinRarity::Restricted },
    { 371, "Griffin", SkinRarity::Restricted },
    { 1143, "Ticket to Hell", SkinRarity::Restricted },
    
    // Mil-Spec & Below
    { 6, "Arctic Camo", SkinRarity::MilSpec },
    { 396, "Urban Hazard", SkinRarity::MilSpec },
    { 316, "Abyss", SkinRarity::MilSpec },
    { 5, "Forest DDPAT", SkinRarity::Industrial },
    { 2, "Groundwater", SkinRarity::Consumer },
};
