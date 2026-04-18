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
#include "combat.h"
#include "chams.h"

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
        f << "aim.showFovCircle=" << Aimbot::config.showFovCircle << "\n";
        f << "aim.smoothing=" << Aimbot::config.smoothing << "\n";
        f << "aim.aimKey=" << Aimbot::config.aimKey << "\n";

        // Triggerbot
        f << "tb.enabled=" << Triggerbot::config.enabled << "\n";
        f << "tb.alwaysOn=" << Triggerbot::config.alwaysOn << "\n";
        f << "tb.key=" << Triggerbot::config.key << "\n";
        f << "tb.teamCheck=" << Triggerbot::config.teamCheck << "\n";
        f << "tb.delayMs=" << Triggerbot::config.delayMs << "\n";

        // ESP
        f << "esp.enabled=" << ESP::config.enabled << "\n";
        f << "esp.bBox=" << ESP::config.bBox << "\n";
        f << "esp.boxType=" << ESP::config.boxType << "\n";
        f << "esp.bHealthBar=" << ESP::config.bHealthBar << "\n";
        f << "esp.bName=" << ESP::config.bName << "\n";
        f << "esp.bDistance=" << ESP::config.bDistance << "\n";
        f << "esp.bSkeleton=" << ESP::config.bSkeleton << "\n";
        f << "esp.teamCheck=" << ESP::config.teamCheck << "\n";
        f << "esp.bGlow=" << ESP::config.bGlow << "\n";
        f << "esp.bChams=" << ESP::config.bChams << "\n";
        f << "esp.chamsVisible=" << ESP::config.chamsVisible << "\n";
        f << "esp.chamsOccluded=" << ESP::config.chamsOccluded << "\n";
        f << "esp.chamsStyleEnemies=" << ESP::config.chamsStyleEnemies << "\n";
        f << "esp.chamsStyleTeam=" << ESP::config.chamsStyleTeam << "\n";
        f << "esp.chamsStyleLocal=" << ESP::config.chamsStyleLocal << "\n";
        f << "esp.chamsColorVisible0=" << ESP::config.chamsColorVisible[0] << "\n";
        f << "esp.chamsColorVisible1=" << ESP::config.chamsColorVisible[1] << "\n";
        f << "esp.chamsColorVisible2=" << ESP::config.chamsColorVisible[2] << "\n";
        f << "esp.chamsColorVisible3=" << ESP::config.chamsColorVisible[3] << "\n";
        f << "esp.chamsColorOccluded0=" << ESP::config.chamsColorOccluded[0] << "\n";
        f << "esp.chamsColorOccluded1=" << ESP::config.chamsColorOccluded[1] << "\n";
        f << "esp.chamsColorOccluded2=" << ESP::config.chamsColorOccluded[2] << "\n";
        f << "esp.chamsColorOccluded3=" << ESP::config.chamsColorOccluded[3] << "\n";
        f << "esp.teamChams=" << ESP::config.teamChams << "\n";
        f << "esp.localChams=" << ESP::config.localChams << "\n";
        f << "esp.bBombTimer=" << ESP::config.bBombTimer << "\n";
        f << "esp.bSpectators=" << ESP::config.bSpectators << "\n";
        f << "esp.bDroppedWeapons=" << ESP::config.bDroppedWeapons << "\n";
        f << "esp.bWeapon=" << ESP::config.bWeapon << "\n";
        f << "esp.bHitmarker=" << ESP::config.bHitmarker << "\n";
        f << "esp.bHitsound=" << ESP::config.bHitsound << "\n";
        f << "esp.enemyColor0=" << ESP::config.enemyColor[0] << "\n";
        f << "esp.enemyColor1=" << ESP::config.enemyColor[1] << "\n";
        f << "esp.enemyColor2=" << ESP::config.enemyColor[2] << "\n";
        f << "esp.enemyColor3=" << ESP::config.enemyColor[3] << "\n";
        f << "esp.previewWidth=" << ESP::config.previewWidthScale << "\n";
        f << "esp.previewHeight=" << ESP::config.previewHeightScale << "\n";
        f << "esp.previewY=" << ESP::config.previewYOffset << "\n";
        f << "esp.bShowCalibration=" << ESP::config.bShowCalibration << "\n";
        f << "esp.teamColor0=" << ESP::config.teamColor[0] << "\n";
        f << "esp.teamColor1=" << ESP::config.teamColor[1] << "\n";
        f << "esp.teamColor2=" << ESP::config.teamColor[2] << "\n";
        f << "esp.teamColor3=" << ESP::config.teamColor[3] << "\n";
        f << "esp.skelColor0=" << ESP::config.skeletonColor[0] << "\n";
        f << "esp.skelColor1=" << ESP::config.skeletonColor[1] << "\n";
        f << "esp.skelColor2=" << ESP::config.skeletonColor[2] << "\n";
        f << "esp.skelColor3=" << ESP::config.skeletonColor[3] << "\n";
        f << "esp.droppedColor0=" << ESP::config.droppedColor[0] << "\n";
        f << "esp.droppedColor1=" << ESP::config.droppedColor[1] << "\n";
        f << "esp.droppedColor2=" << ESP::config.droppedColor[2] << "\n";
        f << "esp.droppedColor3=" << ESP::config.droppedColor[3] << "\n";
        f << "esp.bSpectators=" << ESP::config.bSpectators << "\n";
        f << "esp.bDroppedWeapons=" << ESP::config.bDroppedWeapons << "\n";
        f << "esp.bWeapon=" << ESP::config.bWeapon << "\n";
        f << "esp.espFont=" << ESP::config.espFont << "\n";
        f << "esp.bDroppedDistance=" << ESP::config.bDroppedDistance << "\n";
        f << "models.knife=" << ESP::config.nKnifeModel << "\n";
        f << "models.player=" << ESP::config.nPlayerModel << "\n";
        f << "models.glove=" << ESP::config.nGloveModel << "\n";
        
        // Chams
        f << "chams.enabled=" << Chams::cfg.enabled << "\n";
        f << "chams.wallhack=" << Chams::cfg.wallhack << "\n";
        f << "chams.pVisMat=" << Chams::cfg.playerVis.material << "\n";
        f << "chams.pVisCol0=" << Chams::cfg.playerVis.color[0] << "\n";
        f << "chams.pVisCol1=" << Chams::cfg.playerVis.color[1] << "\n";
        f << "chams.pVisCol2=" << Chams::cfg.playerVis.color[2] << "\n";
        f << "chams.pVisCol3=" << Chams::cfg.playerVis.color[3] << "\n";
        f << "chams.pHidMat=" << Chams::cfg.playerHid.material << "\n";
        f << "chams.pHidCol0=" << Chams::cfg.playerHid.color[0] << "\n";
        f << "chams.pHidCol1=" << Chams::cfg.playerHid.color[1] << "\n";
        f << "chams.pHidCol2=" << Chams::cfg.playerHid.color[2] << "\n";
        f << "chams.pHidCol3=" << Chams::cfg.playerHid.color[3] << "\n";
        f << "chams.handsEnabled=" << Chams::cfg.handsEnabled << "\n";
        f << "chams.hMat=" << Chams::cfg.hands.material << "\n";
        f << "chams.hCol0=" << Chams::cfg.hands.color[0] << "\n";
        f << "chams.hCol1=" << Chams::cfg.hands.color[1] << "\n";
        f << "chams.hCol2=" << Chams::cfg.hands.color[2] << "\n";
        f << "chams.hCol3=" << Chams::cfg.hands.color[3] << "\n";
        f << "chams.weaponsEnabled=" << Chams::cfg.weaponsEnabled << "\n";
        f << "chams.wMat=" << Chams::cfg.weapons.material << "\n";
        f << "chams.wCol0=" << Chams::cfg.weapons.color[0] << "\n";
        f << "chams.wCol1=" << Chams::cfg.weapons.color[1] << "\n";
        f << "chams.wCol2=" << Chams::cfg.weapons.color[2] << "\n";
        f << "chams.wCol3=" << Chams::cfg.weapons.color[3] << "\n";

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

        // Combat
        f << "combat.noRecoil=" << Combat::config.noRecoil << "\n";
        f << "combat.noSpread=" << Combat::config.noSpread << "\n";
        f << "combat.fakeGround=" << Combat::config.fakeGround << "\n";

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
        Aimbot::config.showFovCircle = getBool("aim.showFovCircle", true);
        Aimbot::config.smoothing = getFloat("aim.smoothing", 1.0f);
        Aimbot::config.aimKey = getInt("aim.aimKey", 0);

        // Triggerbot
        Triggerbot::config.enabled = getBool("tb.enabled", false);
        Triggerbot::config.alwaysOn = getBool("tb.alwaysOn", false);
        Triggerbot::config.key = getInt("tb.key", VK_MENU);
        Triggerbot::config.teamCheck = getBool("tb.teamCheck", true);
        Triggerbot::config.delayMs = getInt("tb.delayMs", 15);

        // ESP
        ESP::config.enabled = getBool("esp.enabled", true);
        ESP::config.bBox = getBool("esp.bBox", true);
        ESP::config.boxType = getInt("esp.boxType", 0);
        ESP::config.bHealthBar = getBool("esp.bHealthBar", true);
        ESP::config.bName = getBool("esp.bName", true);
        ESP::config.bDistance = getBool("esp.bDistance", false);
        ESP::config.bSkeleton = getBool("esp.bSkeleton", false);
        ESP::config.teamCheck = getBool("esp.teamCheck", true);
        ESP::config.bGlow = getBool("esp.bGlow", false);
        ESP::config.bChams = getBool("esp.bChams", false);
        ESP::config.chamsVisible = getBool("esp.chamsVisible", true);
        ESP::config.chamsOccluded = getBool("esp.chamsOccluded", true);
        ESP::config.chamsStyleEnemies = getInt("esp.chamsStyleEnemies", 1);
        ESP::config.chamsStyleTeam = getInt("esp.chamsStyleTeam", 1);
        ESP::config.chamsStyleLocal = getInt("esp.chamsStyleLocal", 1);
        ESP::config.chamsColorVisible[0] = getFloat("esp.chamsColorVisible0", 0.0f);
        ESP::config.chamsColorVisible[1] = getFloat("esp.chamsColorVisible1", 1.0f);
        ESP::config.chamsColorVisible[2] = getFloat("esp.chamsColorVisible2", 0.0f);
        ESP::config.chamsColorVisible[3] = getFloat("esp.chamsColorVisible3", 1.0f);
        ESP::config.chamsColorOccluded[0] = getFloat("esp.chamsColorOccluded0", 1.0f);
        ESP::config.chamsColorOccluded[1] = getFloat("esp.chamsColorOccluded1", 0.0f);
        ESP::config.chamsColorOccluded[2] = getFloat("esp.chamsColorOccluded2", 1.0f);
        ESP::config.chamsColorOccluded[3] = getFloat("esp.chamsColorOccluded3", 1.0f);
        ESP::config.teamChams = getBool("esp.teamChams", false);
        ESP::config.localChams = getBool("esp.localChams", false);
        ESP::config.bBombTimer = getBool("esp.bBombTimer", false);
        ESP::config.bSpectators = getBool("esp.bSpectators", false);
        ESP::config.bDroppedWeapons = getBool("esp.bDroppedWeapons", true);
        ESP::config.bWeapon = getBool("esp.bWeapon", true);
        ESP::config.bHitmarker = getBool("esp.bHitmarker", false);
        ESP::config.bHitsound = getBool("esp.bHitsound", false);
        ESP::config.enemyColor[0] = getFloat("esp.enemyColor0", 1.0f);
        ESP::config.enemyColor[1] = getFloat("esp.enemyColor1", 0.0f);
        ESP::config.enemyColor[2] = getFloat("esp.enemyColor2", 0.0f);
        ESP::config.enemyColor[3] = getFloat("esp.enemyColor3", 1.0f);
        ESP::config.previewWidthScale = getFloat("esp.previewWidth", 0.35f);
        ESP::config.previewHeightScale = getFloat("esp.previewHeight", 0.95f);
        ESP::config.previewYOffset = getFloat("esp.previewY", 0.12f);
        ESP::config.bShowCalibration = getBool("esp.bShowCalibration", false);
        ESP::config.teamColor[0] = getFloat("esp.teamColor0", 0.0f);
        ESP::config.teamColor[1] = getFloat("esp.teamColor1", 1.0f);
        ESP::config.teamColor[2] = getFloat("esp.teamColor2", 0.0f);
        ESP::config.teamColor[3] = getFloat("esp.teamColor3", 1.0f);
        ESP::config.skeletonColor[0] = getFloat("esp.skelColor0", 1.0f);
        ESP::config.skeletonColor[1] = getFloat("esp.skelColor1", 1.0f);
        ESP::config.skeletonColor[2] = getFloat("esp.skelColor2", 1.0f);
        ESP::config.skeletonColor[3] = getFloat("esp.skelColor3", 1.0f);
        ESP::config.droppedColor[0] = getFloat("esp.droppedColor0", 1.0f);
        ESP::config.droppedColor[1] = getFloat("esp.droppedColor1", 1.0f);
        ESP::config.droppedColor[2] = getFloat("esp.droppedColor2", 1.0f);
        ESP::config.droppedColor[3] = getFloat("esp.droppedColor3", 0.7f);
        ESP::config.espFont = getInt("esp.espFont", 1);
        ESP::config.bDroppedDistance = getBool("esp.bDroppedDistance", true);

        // Model Changer
        ESP::config.nKnifeModel = getInt("models.knife", 0);
        ESP::config.nPlayerModel = getInt("models.player", 0);
        ESP::config.nGloveModel = getInt("models.glove", 0);

        // Bhop
        Bhop::config.enabled = getBool("bhop.enabled", false);

        // Bullet Tracer
        BulletTracer::config.enabled = getBool("tracer.enabled", false);
        BulletTracer::config.trailLife = getFloat("tracer.trailLife", 2.0f);
        BulletTracer::config.bulletSpeed = getFloat("tracer.bulletSpeed", 8000.0f);
        BulletTracer::config.thickness = getFloat("tracer.thickness", 2.0f);

        // Chams
        Chams::cfg.enabled = getBool("chams.enabled", false);
        Chams::cfg.wallhack = getBool("chams.wallhack", true);
        Chams::cfg.playerVis.material = getInt("chams.pVisMat", 2);
        Chams::cfg.playerVis.color[0] = getFloat("chams.pVisCol0", 1.0f);
        Chams::cfg.playerVis.color[1] = getFloat("chams.pVisCol1", 0.15f);
        Chams::cfg.playerVis.color[2] = getFloat("chams.pVisCol2", 0.15f);
        Chams::cfg.playerVis.color[3] = getFloat("chams.pVisCol3", 1.0f);
        Chams::cfg.playerHid.material = getInt("chams.pHidMat", 1);
        Chams::cfg.playerHid.color[0] = getFloat("chams.pHidCol0", 1.0f);
        Chams::cfg.playerHid.color[1] = getFloat("chams.pHidCol1", 0.5f);
        Chams::cfg.playerHid.color[2] = getFloat("chams.pHidCol2", 0.0f);
        Chams::cfg.playerHid.color[3] = getFloat("chams.pHidCol3", 0.55f);
        Chams::cfg.handsEnabled = getBool("chams.handsEnabled", false);
        Chams::cfg.hands.material = getInt("chams.hMat", 2);
        Chams::cfg.hands.color[0] = getFloat("chams.hCol0", 0.82f);
        Chams::cfg.hands.color[1] = getFloat("chams.hCol1", 0.85f);
        Chams::cfg.hands.color[2] = getFloat("chams.hCol2", 0.95f);
        Chams::cfg.hands.color[3] = getFloat("chams.hCol3", 1.0f);
        Chams::cfg.weaponsEnabled = getBool("chams.weaponsEnabled", false);
        Chams::cfg.weapons.material = getInt("chams.wMat", 3);
        Chams::cfg.weapons.color[0] = getFloat("chams.wCol0", 1.0f);
        Chams::cfg.weapons.color[1] = getFloat("chams.wCol1", 0.95f);
        Chams::cfg.weapons.color[2] = getFloat("chams.wCol2", 0.2f);
        Chams::cfg.weapons.color[3] = getFloat("chams.wCol3", 1.0f);

        // Spinbot
        Spinbot::config.enabled = getBool("spin.enabled", false);
        Spinbot::config.yawSpeed = getFloat("spin.yawSpeed", 15.0f);
        Spinbot::config.pitch = getFloat("spin.pitch", 0.0f);
        Spinbot::config.antiAim = getBool("spin.antiAim", false);

        // Misc
        SkinChanger::thirdPerson = getBool("misc.thirdPerson", false);

        // Combat
        Combat::config.noRecoil = getBool("combat.noRecoil", false);
        Combat::config.noSpread = getBool("combat.noSpread", false);
        Combat::config.fakeGround = getBool("combat.fakeGround", false);

        currentConfig = name;
    }

    inline void Delete(const std::string& name)
    {
        std::filesystem::remove(GetConfigPath(name));
    }
}
