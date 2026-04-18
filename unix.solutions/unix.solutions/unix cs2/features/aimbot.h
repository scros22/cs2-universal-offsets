#pragma once
#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include "../sdk/game.h"
#include "../sdk/offsets.h"
#include "bullet_tracer.h"
#include "bhop.h"
#include "triggerbot.h"
#include "spinbot.h"
#include "combat.h"
#include <MinHook.h>

namespace Hooks { extern bool showMenu; }

// ============================================
// CS2 Silent Aim — CreateMove signature hook
// Hooks CCSGOInput::CreateMove via direct signature scan in client.dll.
//
// How it works:
// 1. BEFORE calling original CreateMove: overwrite dwViewAngles with aim angle
// 2. Original CreateMove reads those angles -> builds CUserCmd with aim angles -> server gets them
// 3. AFTER original returns: restore dwViewAngles -> visual camera stays unchanged
// ============================================

namespace Aimbot
{
    using Game::Vector3;
    using Game::QAngle;

    struct AimbotConfig
    {
        bool  enabled    = false;
        int   fovType    = 0;        // 0 = Angle (Degrees), 1 = Screen (Pixels)
        float fov        = 5.0f;     // For Angle FOV
        float screenFov  = 100.0f;   // For Screen FOV
        int   targetBone = 6;        // 6=head
        int   aimKey     = 0;        // 0=auto (Mouse1)
        bool  autoShoot  = false;
        bool  silentAim  = true;
        bool  teamCheck  = true;     // skip same-team (disable for FFA/DM)
        bool  visCheck   = false;    // prefer visible targets (uses m_bSpotted, may be unreliable)
        bool  showFovCircle = true;  // draw FOV circle on screen
        bool  alwaysActive = false;
        float smoothing = 1.0f;      // 1.0 = instant, >1.0 = slow
    };

    inline AimbotConfig config;

    // ===== Debug =====
    inline bool debugEnabled     = true;
    inline int  debugTickCounter = 0;
    inline float drawFovRadius   = 0.0f; // Stores active FOV radius for drawing

    // ===== Render FOV Circle =====
    inline void RenderFOV()
    {
        if (!config.enabled || !config.showFovCircle) return;
        
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!draw) return;
        
        ImVec2 center = ImGui::GetIO().DisplaySize;
        center.x *= 0.5f;
        center.y *= 0.5f;

        if (config.fovType == 1) // Screen FOV
        {
            draw->AddCircle(center, config.screenFov, IM_COL32(255, 255, 255, 60), 64, 1.0f);
        }
        else if (config.fovType == 0) // Angle FOV visualization approximation
        {
            if (drawFovRadius > 0.0f) {
                draw->AddCircle(center, drawFovRadius, IM_COL32(255, 255, 255, 60), 64, 1.0f);
            }
        }
    }

    // ===== Constants =====
    constexpr float PI      = 3.14159265358979323846f;
    constexpr float RAD2DEG = 180.0f / PI;

    // ===== Angle Math =====
    inline void ClampAngles(QAngle& a)
    {
        if (a.pitch >  89.f) a.pitch =  89.f;
        if (a.pitch < -89.f) a.pitch = -89.f;
        while (a.yaw >  180.f) a.yaw -= 360.f;
        while (a.yaw < -180.f) a.yaw += 360.f;
        a.roll = 0.f;
    }

    inline QAngle CalcAngle(const Vector3& src, const Vector3& dst)
    {
        Vector3 d = dst - src;
        float hyp = d.Length2D();
        QAngle a;
        a.pitch = -atan2f(d.z, hyp) * RAD2DEG;
        a.yaw   =  atan2f(d.y, d.x) * RAD2DEG;
        a.roll  = 0.f;
        ClampAngles(a);
        return a;
    }

    inline float GetFOV(const QAngle& view, const QAngle& aim)
    {
        float dp = aim.pitch - view.pitch;
        float dy = aim.yaw - view.yaw;
        while (dy >  180.f) dy -= 360.f;
        while (dy < -180.f) dy += 360.f;
        return sqrtf(dp * dp + dy * dy);
    }

    inline float GetScreenDistance(const Vector3& targetPos)
    {
        float outX, outY;
        float pos[3] = { targetPos.x, targetPos.y, targetPos.z };
        ImVec2 size = ImGui::GetIO().DisplaySize;
        if (Game::WorldToScreen(pos, outX, outY, size.x, size.y))
        {
            float dx = outX - (size.x * 0.5f);
            float dy = outY - (size.y * 0.5f);
            return sqrtf(dx * dx + dy * dy);
        }
        return FLT_MAX;
    }

    // ===== Visibility Check =====
    constexpr std::ptrdiff_t m_entitySpottedState = 0x2288;
    constexpr std::ptrdiff_t m_bSpottedByMask     = 0xC;

    inline bool IsVisible(uintptr_t pawn, int localIndex)
    {
        if (localIndex < 0) return true; // Fallback if index is invalid
        
        uint32_t mask = Game::Read<uint32_t>(pawn + m_entitySpottedState + m_bSpottedByMask);
        return (mask & (1ULL << (localIndex - 1))) != 0;
    }

    inline Vector3 GetEyePosition(uintptr_t pawn)
    {
        return Game::GetEntityOrigin(pawn) + Game::Read<Vector3>(pawn + Offsets::m_vecViewOffset);
    }

    // ===== Target Selection =====
    inline void NormalizeAngle(QAngle& ang)
    {
        if (ang.pitch > 89.0f) ang.pitch = 89.0f;
        if (ang.pitch < -89.0f) ang.pitch = -89.0f;
        while (ang.yaw > 180.0f) ang.yaw -= 360.0f;
        while (ang.yaw < -180.0f) ang.yaw += 360.0f;
    }

    struct AimTarget
    {
        uintptr_t pawn = 0;
        Vector3   pos;
        QAngle    angle;
        float     fov = FLT_MAX;
    };

    inline AimTarget GetBestTarget(const Vector3& eye, const QAngle& viewAngles, bool doDebug)
    {
        AimTarget best;
        float bestScore = FLT_MAX;

        uintptr_t localCtrl = Game::Read<uintptr_t>(Game::clientBase + Offsets::dwLocalPlayerController);
        if (!localCtrl) return best;

        int localIndex = -1;
        uintptr_t identity = Game::Read<uintptr_t>(localCtrl + 0x10);
        if (identity) localIndex = Game::Read<uint32_t>(identity + 0x10) & 0x7FFF;

        uint32_t localHandle = Game::Read<uint32_t>(localCtrl + Offsets::m_hPlayerPawn);
        uintptr_t localPawn = Game::GetEntityByHandle(localHandle);
        if (!localPawn) return best;

        int localTeam = Game::Read<uint8_t>(localPawn + Offsets::m_iTeamNum);

        uintptr_t entList = Game::Read<uintptr_t>(Game::clientBase + Offsets::dwEntityList);
        if (!entList) return best;

        int enemiesFound = 0;

        for (int i = 1; i <= 64; ++i)
        {
            uint32_t chunkIndex = i >> 9;
            uint32_t entryIndex = i & 0x1FF;

            uintptr_t chunkAddr = entList + 0x8 * chunkIndex + 0x10;
            uintptr_t listEntry = Game::Read<uintptr_t>(chunkAddr);
            if (!listEntry) continue;

            uintptr_t ctrl = Game::Read<uintptr_t>(listEntry + Game::ENTITY_IDENTITY_SIZE * entryIndex);
            if (!ctrl) continue;

            uint32_t pH = Game::Read<uint32_t>(ctrl + Offsets::m_hPlayerPawn);
            if (!pH || pH == 0xFFFFFFFF) continue;

            uintptr_t pawn = Game::GetEntityByHandle(pH);
            if (!pawn || pawn == localPawn) continue;

            // Alive check
            int health = Game::Read<int32_t>(pawn + Offsets::m_iHealth);
            uint8_t lifeState = Game::Read<uint8_t>(pawn + Offsets::m_lifeState);
            if (health <= 0 || lifeState != 0) continue;

            // Team check (can be disabled for FFA/DM)
            int team = Game::Read<uint8_t>(pawn + Offsets::m_iTeamNum);
            if (config.teamCheck && team == localTeam) continue;

            enemiesFound++;

            // Get target bone position
            Vector3 bonePos = Game::GetBonePosition(pawn, config.targetBone);
            if (bonePos.IsZero()) continue;

            QAngle aimAng = CalcAngle(eye, bonePos);
            
            float fovVal = FLT_MAX;
            if (config.fovType == 0) // Angle
            {
                fovVal = GetFOV(viewAngles, aimAng);
                if (fovVal > config.fov) continue;
            }
            else // Screen
            {
                fovVal = GetScreenDistance(bonePos);
                if (fovVal > config.screenFov) continue;
            }

            // Calculate distance
            Vector3 delta = bonePos - eye;
            float dist = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

            // Visibility check
            bool visible = true;
            if (config.visCheck)
                visible = IsVisible(pawn, localIndex);

            // Immediately skip if visibility check fails
            if (config.visCheck && !visible)
                continue;

            // Weighted score: lower = better
            float score = fovVal + (dist * 0.01f);

            if (score < bestScore)
            {
                bestScore  = score;
                best.pawn  = pawn;
                best.pos   = bonePos;
                best.angle = aimAng;
                best.fov   = fovVal;
            }
        }

        return best;
    }

    // ===== CreateMove Hook (MinHook) =====
    using CreateMoveFn = double(__fastcall*)(__int64 a1, unsigned int a2, __int64 a3);
    inline CreateMoveFn oCreateMove = nullptr;

    // Frame history layout (from IDA decompilation):
    constexpr int FRAME_HISTORY_COUNT_OFF = 0xBC8;
    constexpr int FRAME_HISTORY_ARRAY_OFF = 0xBD0;
    constexpr int FRAME_ENTRY_SIZE        = 96;   // 0x60
    constexpr int PITCH_OFF_IN_ENTRY      = 0x10;
    constexpr int YAW_OFF_IN_ENTRY        = 0x14;

    inline double ExecuteCreateMoveWithTracing(__int64 a1, unsigned int a2, __int64 a3, bool shotFired, const Vector3& eyePos, float& outPitch, float& outYaw, QAngle& viewAngles, uintptr_t localPawn, bool doDebug)
    {
        float bulletPitch = viewAngles.pitch;
        float bulletYaw   = viewAngles.yaw;

        // --- Synchronized Combat Features (Always active if toggled) ---
        Vector3 currentPunch { 0, 0, 0 };
        if (localPawn)
        {
            // 1. Capture "True" Recoil for calculations (Captured BEFORE visual zeroing)
            currentPunch = Game::Read<Vector3>(localPawn + Offsets::m_aimPunchAngle);

            // 2. Apply Visual No-Recoil synchronously
            if (Combat::config.noRecoil)
            {
                Game::Write<Vector3>(localPawn + Offsets::m_aimPunchAngle, { 0, 0, 0 });
                Game::Write<Vector3>(localPawn + Offsets::m_aimPunchAngleVel, { 0, 0, 0 });
            }

            // 3. Apply No-Spread synchronously
            if (Combat::config.noSpread)
            {
                uintptr_t weaponServices = Game::Read<uintptr_t>(localPawn + Offsets::m_pWeaponServices);
                if (weaponServices)
                {
                    uint32_t weaponHandle = Game::Read<uint32_t>(weaponServices + Offsets::m_hActiveWeapon);
                    uintptr_t activeWeapon = Game::GetEntityByHandle(weaponHandle);
                    if (activeWeapon)
                    {
                        Game::Write<float>(activeWeapon + Offsets::m_fAccuracyPenalty, 0.0f);
                        Game::Write<float>(activeWeapon + Offsets::m_flRecoilIndex, 0.0f);
                        Game::Write<int>(activeWeapon + Offsets::m_iRecoilIndex, 0);
                    }
                }
            }
        }

        // === AIMBOT LOGIC ===
        if (!config.enabled)
        {
            outPitch = bulletPitch;
            outYaw   = bulletYaw;
            return oCreateMove(a1, a2, a3);
        }

        // Check aim key
        bool shouldAim = false;
        if (config.alwaysActive)
            shouldAim = true;
        else if (config.aimKey == 0)
            shouldAim = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        else
            shouldAim = (GetAsyncKeyState(config.aimKey) & 0x8000) != 0;

        if (!shouldAim || !localPawn || eyePos.IsZero())
        {
            outPitch = bulletPitch;
            outYaw   = bulletYaw;
            return oCreateMove(a1, a2, a3);
        }

        // Find best target
        AimTarget target = GetBestTarget(eyePos, viewAngles, doDebug);
        if (target.pawn == 0)
        {
            outPitch = bulletPitch;
            outYaw   = bulletYaw;
            return oCreateMove(a1, a2, a3);
        }

        // Normalize target angles immediately to prevent server rejection
        NormalizeAngle(target.angle);

        // --- Recoil Compensation (Aimbot Internal) ---
        // Uses the 'currentPunch' we captured at the absolute start of the frame.
        if (!currentPunch.IsZero())
        {
            target.angle.pitch -= currentPunch.x * 2.0f;
            target.angle.yaw   -= currentPunch.y * 2.0f;
        }

        // The aimbot will override bullet direction
        // Apply smoothing if enabled
        if (config.smoothing > 1.0f)
        {
            float deltaP = target.angle.pitch - viewAngles.pitch;
            float deltaY = target.angle.yaw - viewAngles.yaw;

            // Normalize yaw delta
            if (deltaY > 180.0f) deltaY -= 360.0f;
            else if (deltaY < -180.0f) deltaY += 360.0f;

            bulletPitch = viewAngles.pitch + (deltaP / config.smoothing);
            bulletYaw = viewAngles.yaw + (deltaY / config.smoothing);

            // Update target entry for silent aim/tracer consistency
            target.angle.pitch = bulletPitch;
            target.angle.yaw = bulletYaw;
        }
        else
        {
            bulletPitch = target.angle.pitch;
            bulletYaw = target.angle.yaw;
        }
        
        outPitch = bulletPitch;
        outYaw   = bulletYaw;

        // --- AUTO SHOOT ---
        if (config.autoShoot && target.pawn != 0 && !Hooks::showMenu)
        {
            static std::chrono::steady_clock::time_point lastShootTime;
            auto now = std::chrono::steady_clock::now();
            
            // Limit rate slightly to allow game to register shots
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShootTime).count() > 30)
            {
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                lastShootTime = now;
            }
        }

        // === SILENT AIM ===
        if (config.silentAim)
        {
            // Read frame history from CCSGOInput (a1)
            int entryCount = *reinterpret_cast<int*>(a1 + FRAME_HISTORY_COUNT_OFF);
            uintptr_t entryArray = *reinterpret_cast<uintptr_t*>(a1 + FRAME_HISTORY_ARRAY_OFF);

            // Save original angles and overwrite BEFORE CreateMove
            struct SavedAngles { float pitch; float yaw; };
            SavedAngles saved[64] = {};
            int safeCount = (entryCount > 0 && entryCount <= 64) ? entryCount : 0;

            if (entryArray && safeCount > 0)
            {
                for (int i = 0; i < safeCount; i++)
                {
                    uintptr_t entry = entryArray + FRAME_ENTRY_SIZE * i;
                    float* pPitch = reinterpret_cast<float*>(entry + PITCH_OFF_IN_ENTRY);
                    float* pYaw   = reinterpret_cast<float*>(entry + YAW_OFF_IN_ENTRY);

                    saved[i].pitch = Game::SafeReadFloat(pPitch);
                    saved[i].yaw   = Game::SafeReadFloat(pYaw);

                    Game::SafeWriteFloat(pPitch, target.angle.pitch);
                    Game::SafeWriteFloat(pYaw,   target.angle.yaw);
                }
            }

            // Call original CreateMove — it reads our overwritten angles
            double result = oCreateMove(a1, a2, a3);

            // Restore original angles — camera stays unchanged
            if (entryArray && safeCount > 0)
            {
                for (int i = 0; i < safeCount; i++)
                {
                    uintptr_t entry = entryArray + FRAME_ENTRY_SIZE * i;
                    float* pPitch = reinterpret_cast<float*>(entry + PITCH_OFF_IN_ENTRY);
                    float* pYaw   = reinterpret_cast<float*>(entry + YAW_OFF_IN_ENTRY);
                    Game::SafeWriteFloat(pPitch, saved[i].pitch);
                    Game::SafeWriteFloat(pYaw,   saved[i].yaw);
                }
            }

            return result;
        }
        else
        {
            // Visible aimbot — set dwViewAngles (using SafeWrite for anti-detection)
            uintptr_t va = Game::clientBase + Offsets::dwViewAngles;
            Game::SafeWrite<QAngle>(va, target.angle);
            return oCreateMove(a1, a2, a3);
        }
    }

    inline double __fastcall hkCreateMove(__int64 a1, unsigned int a2, __int64 a3)
    {
        debugTickCounter++;
        bool doDebug = debugEnabled && (debugTickCounter % 512 == 1);

        if (!Game::clientBase)
            return oCreateMove(a1, a2, a3);

        // === ALWAYS: read local pawn + eye pos for tracer ===
        uintptr_t localPawn = Game::GetLocalPlayerPawn();
        uintptr_t viewAnglesAddr = Game::clientBase + Offsets::dwViewAngles;
        QAngle viewAngles = Game::Read<QAngle>(viewAnglesAddr);
        Vector3 eyePos = localPawn ? GetEyePosition(localPawn) : Vector3{0,0,0};

        // === ALWAYS: detect shots for bullet tracer (works without aimbot) ===
        bool shotFired = localPawn && BulletTracer::DetectShot(localPawn);

        // === MISC: Bunny Hop ===
        if (localPawn) Bhop::Tick(localPawn);

        // === MISC: Triggerbot ===
        if (localPawn) Triggerbot::Tick(localPawn);

        // === MISC: Spinbot (frame history modification, same as silent aim) ===
        bool spinActive = Spinbot::config.enabled && localPawn && !config.enabled;
        if (spinActive)
        {
            float spinYaw = Spinbot::GetSpinYaw();
            float spinPitch = Spinbot::GetSpinPitch();

            if (spinYaw > -998.0f)
            {
                int entryCount = *reinterpret_cast<int*>(a1 + FRAME_HISTORY_COUNT_OFF);
                uintptr_t entryArray = *reinterpret_cast<uintptr_t*>(a1 + FRAME_HISTORY_ARRAY_OFF);
                struct SavedAng { float p; float y; };
                SavedAng spinSaved[64] = {};
                int safeCount = (entryCount > 0 && entryCount <= 64) ? entryCount : 0;

                if (entryArray && safeCount > 0)
                {
                    for (int i = 0; i < safeCount; i++)
                    {
                        uintptr_t entry = entryArray + FRAME_ENTRY_SIZE * i;
                        float* pP = reinterpret_cast<float*>(entry + PITCH_OFF_IN_ENTRY);
                        float* pY = reinterpret_cast<float*>(entry + YAW_OFF_IN_ENTRY);
                        spinSaved[i].p = Game::SafeReadFloat(pP);
                        spinSaved[i].y = Game::SafeReadFloat(pY);
                        Game::SafeWriteFloat(pP, spinPitch);
                        Game::SafeWriteFloat(pY, spinYaw);
                    }
                }

                double result = oCreateMove(a1, a2, a3);

                if (entryArray && safeCount > 0)
                {
                    for (int i = 0; i < safeCount; i++)
                    {
                        uintptr_t entry = entryArray + FRAME_ENTRY_SIZE * i;
                        float* pP = reinterpret_cast<float*>(entry + PITCH_OFF_IN_ENTRY);
                        float* pY = reinterpret_cast<float*>(entry + YAW_OFF_IN_ENTRY);
                        Game::SafeWriteFloat(pP, spinSaved[i].p);
                        Game::SafeWriteFloat(pY, spinSaved[i].y);
                    }
                }

                if (shotFired && !eyePos.IsZero())
                    BulletTracer::AddTraceFromAngles(eyePos.x, eyePos.y, eyePos.z, viewAngles.pitch, viewAngles.yaw);

                return result;
            }
        }

        // Variables that aimbot will assign to if taking over.
        float finalPitch = viewAngles.pitch;
        float finalYaw = viewAngles.yaw;

        double result = ExecuteCreateMoveWithTracing(a1, a2, a3, shotFired, eyePos, finalPitch, finalYaw, viewAngles, localPawn, doDebug);

        // Fire tracer ONLY exactly once, right before exiting
        if (shotFired && !eyePos.IsZero())
        {
            BulletTracer::AddTraceFromAngles(
                eyePos.x, eyePos.y, eyePos.z,
                finalPitch, finalYaw);
        }

        return result;
    }

    // ===== Hook Setup (MinHook-based) =====
    inline bool Init()
    {
#ifdef _DEBUG
        printf("[Aimbot] Scanning for CreateMove in client.dll...\n");
        printf("[Aimbot] clientBase: 0x%IX\n", Game::clientBase);
#endif

        uintptr_t createMoveAddr = Game::FindPattern(
            L"client.dll", Offsets::sig_CreateMove_client);

        if (!createMoveAddr)
        {
#ifdef _DEBUG
            printf("[Aimbot] FAIL: CreateMove signature not found!\n");
#endif
            return false;
        }

#ifdef _DEBUG
        printf("[Aimbot] CreateMove found at: 0x%IX\n", createMoveAddr);
#endif

        MH_STATUS status = MH_CreateHook(
            reinterpret_cast<void*>(createMoveAddr),
            &hkCreateMove,
            reinterpret_cast<void**>(&oCreateMove));

        if (status != MH_OK)
        {
#ifdef _DEBUG
            printf("[Aimbot] FAIL: MH_CreateHook returned %d\n", (int)status);
#endif
            return false;
        }

        status = MH_EnableHook(reinterpret_cast<void*>(createMoveAddr));
        if (status != MH_OK)
        {
#ifdef _DEBUG
            printf("[Aimbot] FAIL: MH_EnableHook returned %d\n", (int)status);
#endif
            return false;
        }

#ifdef _DEBUG
        printf("[Aimbot] CreateMove hooked successfully!\n");
#endif
        return true;
    }

    inline void Shutdown()
    {
        MH_DisableHook(MH_ALL_HOOKS);
#ifdef _DEBUG
        printf("[Aimbot] Shutdown\n");
#endif
    }
}
