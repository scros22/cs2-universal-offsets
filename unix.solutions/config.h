#pragma once
#include <Windows.h>
#include <ShlObj.h>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <filesystem>

#include "aimbot.h"
#include "triggerbot.h"
#include "esp.h"
#include "bhop.h"
#include "bullet_tracer.h"
#include "skin_changer.h"
#include "spinbot.h"

namespace Config
{
    inline std::string configDir;
    inline std::string currentConfig = "default";

    inline std::string GetConfigDir()
    {
        if (!configDir.empty()) return configDir;
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
        {
            configDir = std::string(path) + "\\UnixCS2";
            std::filesystem::create_directories(configDir);
        }
        return configDir;
    }

    inline std::string GetConfigPath(const std::string& name)
    {
        return GetConfigDir() + "\\" + name + ".cfg";
    }

    inline std::vector<std::string> GetConfigs()
    {
        std::vector<std::string> configs;
        std::string dir = GetConfigDir();
        if (dir.empty()) return configs;
        for (auto& entry : std::filesystem::directory_iterator(dir))
        {
            if (entry.path().extension() == ".cfg")
                configs.push_back(entry.path().stem().string());
        }
        return configs;
    }

    inline void Save(const std::string& name)
    {
        std::ofstream f(GetConfigPath(name));
        if (!f.is_open()) return;

        // Aimbot
        f << "aim.enabled=" << Aimbot::config.enabled << "\n";
        f << "aim.silentAim=" << Aimbot::config.silentAim << "\n";
        f << "aim.autoShoot=" << Aimbot::config.autoShoot << "\n";
        f << "aim.teamCheck=" << Aimbot::config.teamCheck << "\n";
        f << "aim.visCheck=" << Aimbot::config.visCheck << "\n";
        f << "aim.fov=" << Aimbot::config.fov << "\n";
        f << "aim.screenFov=" << Aimbot::config.screenFov << "\n";
        f << "aim.fovType=" << Aimbot::config.fovType << "\n";
        f << "aim.targetBone=" << Aimbot::config.targetBone << "\n";
        f << "aim.aimKey=" << Aimbot::config.aimKey << "\n";

        // Triggerbot
        f << "tb.enabled=" << Triggerbot::config.enabled << "\n";
        f << "tb.teamCheck=" << Triggerbot::config.teamCheck << "\n";
        f << "tb.delayMs=" << Triggerbot::config.delayMs << "\n";

        // ESP
        f << "esp.enabled=" << ESP::config.enabled << "\n";
        f << "esp.bBox=" << ESP::config.bBox << "\n";
        f << "esp.boxMode=" << ESP::config.boxMode << "\n";
        f << "esp.bHealthBar=" << ESP::config.bHealthBar << "\n";
        f << "esp.bName=" << ESP::config.bName << "\n";
        f << "esp.bDistance=" << ESP::config.bDistance << "\n";
        f << "esp.bSkeleton=" << ESP::config.bSkeleton << "\n";
        f << "esp.teamCheck=" << ESP::config.teamCheck << "\n";
        f << "esp.bGlow=" << ESP::config.bGlow << "\n";
        f << "esp.bChams=" << ESP::config.bChams << "\n";
        f << "esp.bBombTimer=" << ESP::config.bBombTimer << "\n";
        f << "esp.bSpectators=" << ESP::config.bSpectators << "\n";
        f << "esp.boxColor0=" << ESP::config.boxColor[0] << "\n";
        f << "esp.boxColor1=" << ESP::config.boxColor[1] << "\n";
        f << "esp.boxColor2=" << ESP::config.boxColor[2] << "\n";
        f << "esp.boxColor3=" << ESP::config.boxColor[3] << "\n";
        f << "esp.glowColor0=" << ESP::config.glowColor[0] << "\n";
        f << "esp.glowColor1=" << ESP::config.glowColor[1] << "\n";
        f << "esp.glowColor2=" << ESP::config.glowColor[2] << "\n";
        f << "esp.glowColor3=" << ESP::config.glowColor[3] << "\n";

        // Bhop
        f << "bhop.enabled=" << Bhop::config.enabled << "\n";

        // Bullet Tracer
        f << "tracer.enabled=" << BulletTracer::config.enabled << "\n";
        f << "tracer.trailLife=" << BulletTracer::config.trailLife << "\n";
        f << "tracer.bulletSpeed=" << BulletTracer::config.bulletSpeed << "\n";
        f << "tracer.thickness=" << BulletTracer::config.thickness << "\n";

        // Spinbot
        f << "spin.enabled=" << Spinbot::config.enabled << "\n";
        f << "spin.yawSpeed=" << Spinbot::config.yawSpeed << "\n";
        f << "spin.pitch=" << Spinbot::config.pitch << "\n";
        f << "spin.antiAim=" << Spinbot::config.antiAim << "\n";

        // Misc
        f << "misc.thirdPerson=" << SkinChanger::thirdPerson << "\n";

        f.close();
        currentConfig = name;
    }

    inline void Load(const std::string& name)
    {
        std::ifstream f(GetConfigPath(name));
        if (!f.is_open()) return;

        std::map<std::string, std::string> kv;
        std::string line;
        while (std::getline(f, line))
        {
            auto eq = line.find('=');
            if (eq != std::string::npos)
                kv[line.substr(0, eq)] = line.substr(eq + 1);
        }
        f.close();

        auto getBool = [&](const std::string& key, bool def) -> bool {
            auto it = kv.find(key);
            return it != kv.end() ? (it->second != "0") : def;
        };
        auto getInt = [&](const std::string& key, int def) -> int {
            auto it = kv.find(key);
            return it != kv.end() ? std::stoi(it->second) : def;
        };
        auto getFloat = [&](const std::string& key, float def) -> float {
            auto it = kv.find(key);
            return it != kv.end() ? std::stof(it->second) : def;
        };

        // Aimbot
        Aimbot::config.enabled = getBool("aim.enabled", false);
        Aimbot::config.silentAim = getBool("aim.silentAim", true);
        Aimbot::config.autoShoot = getBool("aim.autoShoot", false);
        Aimbot::config.teamCheck = getBool("aim.teamCheck", true);
        Aimbot::config.visCheck = getBool("aim.visCheck", true);
        Aimbot::config.fov = getFloat("aim.fov", 5.0f);
        Aimbot::config.screenFov = getFloat("aim.screenFov", 150.0f);
        Aimbot::config.fovType = getInt("aim.fovType", 0);
        Aimbot::config.targetBone = getInt("aim.targetBone", 6);
        Aimbot::config.aimKey = getInt("aim.aimKey", 0);

        // Triggerbot
        Triggerbot::config.enabled = getBool("tb.enabled", false);
        Triggerbot::config.teamCheck = getBool("tb.teamCheck", true);
        Triggerbot::config.delayMs = getInt("tb.delayMs", 15);

        // ESP
        ESP::config.enabled = getBool("esp.enabled", true);
        ESP::config.bBox = getBool("esp.bBox", true);
        ESP::config.boxMode = getInt("esp.boxMode", 0);
        ESP::config.bHealthBar = getBool("esp.bHealthBar", true);
        ESP::config.bName = getBool("esp.bName", true);
        ESP::config.bDistance = getBool("esp.bDistance", false);
        ESP::config.bSkeleton = getBool("esp.bSkeleton", false);
        ESP::config.teamCheck = getBool("esp.teamCheck", true);
        ESP::config.bGlow = getBool("esp.bGlow", false);
        ESP::config.bChams = getBool("esp.bChams", false);
        ESP::config.bBombTimer = getBool("esp.bBombTimer", false);
        ESP::config.bSpectators = getBool("esp.bSpectators", false);
        ESP::config.boxColor[0] = getFloat("esp.boxColor0", 1.0f);
        ESP::config.boxColor[1] = getFloat("esp.boxColor1", 0.29f);
        ESP::config.boxColor[2] = getFloat("esp.boxColor2", 0.32f);
        ESP::config.boxColor[3] = getFloat("esp.boxColor3", 1.0f);
        ESP::config.glowColor[0] = getFloat("esp.glowColor0", 1.0f);
        ESP::config.glowColor[1] = getFloat("esp.glowColor1", 0.0f);
        ESP::config.glowColor[2] = getFloat("esp.glowColor2", 0.0f);
        ESP::config.glowColor[3] = getFloat("esp.glowColor3", 0.6f);

        // Bhop
        Bhop::config.enabled = getBool("bhop.enabled", false);

        // Bullet Tracer
        BulletTracer::config.enabled = getBool("tracer.enabled", false);
        BulletTracer::config.trailLife = getFloat("tracer.trailLife", 2.0f);
        BulletTracer::config.bulletSpeed = getFloat("tracer.bulletSpeed", 8000.0f);
        BulletTracer::config.thickness = getFloat("tracer.thickness", 2.0f);

        // Spinbot
        Spinbot::config.enabled = getBool("spin.enabled", false);
        Spinbot::config.yawSpeed = getFloat("spin.yawSpeed", 15.0f);
        Spinbot::config.pitch = getFloat("spin.pitch", 0.0f);
        Spinbot::config.antiAim = getBool("spin.antiAim", false);

        // Misc
        SkinChanger::thirdPerson = getBool("misc.thirdPerson", false);

        currentConfig = name;
    }

    inline void Delete(const std::string& name)
    {
        std::filesystem::remove(GetConfigPath(name));
    }
}
