#pragma once

// ---------------------------------------------------------------
// WeaponIcons — Font-based weapon icon rendering
// Uses a custom weapon icon font where letters map to weapon glyphs
// ---------------------------------------------------------------

#include <cstring>
#include <string>

namespace WeaponIcons
{
    inline ImFont* g_weaponFont = nullptr;

    inline void Init(ImFont* weaponFont)
    {
        g_weaponFont = weaponFont;
    }

    inline bool HasFont()
    {
        return g_weaponFont != nullptr;
    }

    inline ImFont* GetFont()
    {
        return g_weaponFont;
    }

    inline const char* GetWeaponIcon(const char* weaponName)
    {
        if (!weaponName) return "W";
        
        // Use string find like unix.solutions for more flexible matching
        std::string d = weaponName;
        
        // Rifles
        if (d.find("ak47") != std::string::npos) return "W";
        if (d.find("m4a1_s") != std::string::npos || d.find("m4a1_silencer") != std::string::npos) return "T";
        if (d.find("m4a4") != std::string::npos) return "S";
        if (d.find("m4a1") != std::string::npos) return "S"; // fallback for m4a4
        if (d.find("aug") != std::string::npos) return "U";
        if (d.find("famas") != std::string::npos) return "R";
        if (d.find("galilar") != std::string::npos || d.find("galil") != std::string::npos) return "Q";
        if (d.find("sg556") != std::string::npos) return "V";
        
        // Snipers
        if (d.find("awp") != std::string::npos) return "Z";
        if (d.find("ssg08") != std::string::npos) return "a";
        if (d.find("g3sg1") != std::string::npos) return "X";
        if (d.find("scar20") != std::string::npos) return "Y";
        
        // SMGs
        if (d.find("mac10") != std::string::npos) return "K";
        if (d.find("mp5sd") != std::string::npos || d.find("mp5") != std::string::npos) return "x";
        if (d.find("mp7") != std::string::npos) return "N";
        if (d.find("mp9") != std::string::npos) return "O";
        if (d.find("bizon") != std::string::npos) return "M";
        if (d.find("p90") != std::string::npos) return "P";
        if (d.find("ump45") != std::string::npos) return "L";
        
        // Shotguns
        if (d.find("nova") != std::string::npos) return "e";
        if (d.find("sawedoff") != std::string::npos) return "c";
        if (d.find("xm1014") != std::string::npos) return "b";
        if (d.find("mag7") != std::string::npos) return "d";
        
        // Heavy
        if (d.find("m249") != std::string::npos) return "g";
        if (d.find("negev") != std::string::npos) return "f";
        
        // Pistols
        if (d.find("deagle") != std::string::npos) return "A";
        if (d.find("elite") != std::string::npos) return "B";
        if (d.find("fiveseven") != std::string::npos) return "C";
        if (d.find("glock") != std::string::npos) return "D";
        if (d.find("hkp2000") != std::string::npos || d.find("p2000") != std::string::npos) return "E";
        if (d.find("usp_s") != std::string::npos || d.find("usp_silencer") != std::string::npos) return "G";
        if (d.find("usps") != std::string::npos) return "G";
        if (d.find("p250") != std::string::npos) return "F";
        if (d.find("cz75a") != std::string::npos || d.find("cz75") != std::string::npos) return "I";
        if (d.find("tec9") != std::string::npos) return "H";
        if (d.find("revolver") != std::string::npos) return "J";
        
        // Utility
        if (d.find("taser") != std::string::npos) return "h";
        if (d.find("c4") != std::string::npos) return "o";
        if (d.find("knife") != std::string::npos) return "[";
        
        // Grenades
        if (d.find("hegrenade") != std::string::npos) return "j";
        if (d.find("flashbang") != std::string::npos) return "i";
        if (d.find("smokegrenade") != std::string::npos) return "k";
        if (d.find("molotov") != std::string::npos) return "l";
        if (d.find("incgrenade") != std::string::npos || d.find("incendiary") != std::string::npos) return "n";
        if (d.find("decoy") != std::string::npos) return "m";
        
        return "W"; // Default fallback
    }

    inline void Shutdown()
    {
        g_weaponFont = nullptr;
    }
}
