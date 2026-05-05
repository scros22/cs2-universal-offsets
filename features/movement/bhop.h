#pragma once

// ---------------------------------------------------------------
// Bhop v26 â€” Full auto bhop + proper air strafe
//
// Fully automatic: just hold space and move your mouse.
// Auto-jump on ground contact, auto-strafe in air based on
// mouse movement direction.
//
// KEY INSIGHT: In Source engine, holding W (forward) in air kills
// air acceleration. Auto strafe MUST release forward and ONLY
// press A/D to gain speed. This is the fundamental mechanic.
//
// Values: 65537 = press (+cmd), 256 = release (-cmd)
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include "../../core/game_state.h"
#include "../../core/sdk_offsets.h"
#include "../../core/memory.h"
#include "../../core/signatures.h"
#include "../../vendor/minhook/include/MinHook.h"

// Button command offsets are pulled from sdk/buttons.hpp via
// Offsets::Buttons (refreshed from live dumper, build 14154).
// Pre-fix: bhop hardcoded jump=0x2066C70, drift +0x18D40 from live
// 0x204DF30 â€” every press/release was hitting unrelated client.dll
// memory and bhop only "worked" because the user holds Space manually.
namespace ButtonOffsets {
    constexpr std::ptrdiff_t jump    = Offsets::Buttons::jump;
    constexpr std::ptrdiff_t left    = Offsets::Buttons::left;     // +moveleft
    constexpr std::ptrdiff_t right   = Offsets::Buttons::right;    // +moveright
    constexpr std::ptrdiff_t forward = Offsets::Buttons::forward;  // +forward
    constexpr std::ptrdiff_t back    = Offsets::Buttons::back;     // +back
    constexpr std::ptrdiff_t duck    = Offsets::Buttons::duck;     // +duck (for jumpshot crouch-air)
}

constexpr std::uint32_t FL_ONGROUND = (1 << 0);

namespace Bhop
{
    struct Config
    {
        bool  enabled       = false;
        int   key           = VK_SPACE;
        // Soft speed clamp. 0 = OFF (recommended for advanced bhop â€”
        // the only way to climb past 400 ups is to NOT clamp). When >0,
        // autostrafe releases input above this speed so velocity coasts
        // back into the band â€” useful for HvH where 290-315 mimics
        // human pressure but caps the obvious tells.
        // Set in menu. Default 0 = unlimited (gamesense "advanced" mode).
        float maxSpeed      = 0.f;
        float minSpeed      = 30.f;   // kickstart threshold
        bool  autoStrafe    = true;
        bool  showVelocity  = true;   // HUD speed display
        // Strafe mode: 0 = velocity (smoothest, perfect-strafe physics),
        //              1 = mouse-yaw (legacy, follows mouse turn delta)
        int   strafeMode    = 0;
        // Subtick path: queue jump via m_arrForceSubtickMoveWhen +
        // m_nQueuedButtonChangeMask. The ONLY path sv_subtick_legacy_kbinds 0
        // servers honour. Pairs with kbutton fallback for legacy servers.
        bool  subtickJump   = true;

        // Smart-strafe yield: when the player turns their mouse faster
        // than this many degrees per render frame, AutoStrafe stops
        // queueing A/D and lets the engine read the player's mouse
        // rotation natively. Eliminates the "forces one direction and
        // ignores my keyboard" feel.
        float yieldYawDeg   = 1.5f;
        // How long after the last fast mouse-turn frame to keep yielding.
        int   yieldGraceMs  = 120;
    };

    inline Config cfg;

    // Internal state
    inline bool  wasOnGround     = true;
    inline int   hopCount        = 0;
    inline DWORD lastHopTime     = 0;
    inline float savedYaw        = 0.f;
    inline bool  yawInit         = false;
    inline bool  jumpToggle      = false;  // alternates jump press/release
    inline bool  inAutoStrafe    = false;  // true while we're controlling strafe keys in air

    // ----- one-shot-per-landing tracking -----
    // ModernJumpPrime + subtick queue + legacy kbutton all used to fire every
    // render frame. That tripped the engine's spam-penalty timer (sv_jump_
    // spam_penalty_time) after ~5s of continuous bhop, producing the
    // "glitching up/down like the game's trying to stop the jump" symptom.
    // We now forge / queue exactly ONCE per detected landing.
    inline int32_t lastPrimedLanded = INT32_MIN;
    inline int32_t lastQueuedLanded = INT32_MIN;

    // Yaw-delta tracker for smart strafe yield. When the player is actively
    // turning their mouse fast, we DON'T fight them with auto-strafe — their
    // mouse input owns the direction. Resets each tick.
    inline float prevTickYaw = 0.f;
    inline DWORD lastUserMouseMs = 0;
    inline bool  prevWasEnabled  = false;  // edge detect for clean cleanup

    // Last value we wrote to each button slot, so AutoStrafe can avoid
    // hammering the same press/release on every tick. Spamming the same
    // button command bytes 64+ times/sec produces visible micro-jitter
    // and increases our memory-write footprint for no gain.
    inline uint32_t lastBtnLeft    = 0;
    inline uint32_t lastBtnRight   = 0;
    inline uint32_t lastBtnForward = 0;
    inline uint32_t lastBtnBack    = 0;

    inline void WriteBtnIfChanged(uintptr_t base, std::ptrdiff_t off,
                                  uint32_t val, uint32_t& cache)
    {
        if (cache == val) return;
        Mem::Write<uint32_t>(base + off, val);
        cache = val;
    }

    // Movement services offsets â€” canonical, sourced from
    // dumps/latest/offsets/sdk/client_dll.hpp on build 14154.
    // PRE-14154 these were 0x518/0x528/0x530/0x2714 with the services
    // pointer at pawn+0x1418; writing those values now silently lands
    // on unrelated fields and bhop never resets stamina, killing the
    // strafe speed gain.
    using Offsets::m_pMovementServices_pawn;
    using Offsets::m_flStamina_movement;
    using Offsets::m_flStaminaAtJumpStart_movement;
    using Offsets::m_flAccumulatedJumpError_mov;
    using Offsets::m_flLastJumpVelocityZ_movement;
    using Offsets::m_flVelocityModifier_pawn;
    using Offsets::m_ModernJump_movement;
    using Offsets::m_ModernJump_LastActualJumpPressTick_off;
    using Offsets::m_ModernJump_LastActualJumpPressFrac_off;
    using Offsets::m_ModernJump_LastUsableJumpPressTick_off;
    using Offsets::m_ModernJump_LastUsableJumpPressFrac_off;
    using Offsets::m_ModernJump_LastLandedTick_off;
    using Offsets::m_LegacyJump_movement;
    using Offsets::m_LegacyJump_OldJumpPressed_off;
    using Offsets::m_LegacyJump_JumpPressedTime_off;
    using Offsets::m_nLastJumpTick_movement;

    // Modern subtick API offsets (CPlayer_MovementServices base)
    using Offsets::m_nButtons_movement;
    using Offsets::m_nQueuedButtonDownMask_movement;
    using Offsets::m_nQueuedButtonChangeMask_movement;
    using Offsets::m_flCmdForwardMove_movement;
    using Offsets::m_flCmdLeftMove_movement;
    using Offsets::m_flMaxspeed_movement;
    using Offsets::m_arrForceSubtickMoveWhen_movement;
    using Offsets::m_flForwardMove_movement;
    using Offsets::m_flLeftMove_movement;

    inline bool hookInstalled = false;

    // ---------------------------------------------------------------
    // Auto Strafe â€” proper Source engine air strafing.
    //
    // The mechanic: in air, RELEASE forward/back, then press A while
    // turning left of velocity OR D while turning right of velocity.
    // This generates the speed gain.
    //
    // We support TWO modes:
    //   strafeMode = 0 (default): velocity-based. Compute the angle
    //     between the player's view yaw and current horizontal velocity
    //     vector. Strafe in the direction that points the velocity
    //     toward the view ("perfect strafe") â€” produces buttery, fluid
    //     turns regardless of mouse speed.
    //   strafeMode = 1: legacy mouse-yaw delta. Follows the user's mouse
    //     turn rate frame-by-frame. Less optimal, more "manual feel".
    //
    // All button writes go through WriteBtnIfChanged so we don't spam
    // the same value over and over (which produces stutter + bloats
    // our memory-write footprint that anti-cheat heuristics scan for).
    // ---------------------------------------------------------------
    inline void AutoStrafe()
    {
        if (!cfg.autoStrafe) return;
        if (!GameState::clientBase) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        uint32_t flags = Mem::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        bool onGround = (flags & FL_ONGROUND) != 0;
        uintptr_t base = GameState::clientBase;

        auto releaseStrafe = [&]() {
            WriteBtnIfChanged(base, ButtonOffsets::left,    256, lastBtnLeft);
            WriteBtnIfChanged(base, ButtonOffsets::right,   256, lastBtnRight);
        };

        if (onGround)
        {
            if (inAutoStrafe)
            {
                releaseStrafe();
                // Don't touch forward/back on ground â€” it's the user's input.
                inAutoStrafe = false;
            }
            yawInit = false;
            return;
        }

        // === IN AIR ===

        // Only auto strafe while the bhop key is held.
        if (!(GetAsyncKeyState(cfg.key) & 0x8000))
        {
            if (inAutoStrafe)
            {
                releaseStrafe();
                inAutoStrafe = false;
            }
            yawInit = false;
            return;
        }

        // RESPECT MANUAL INPUT: if the player is actively pressing A or D
        // themselves, bail â€” they're driving the strafe manually.
        // This is what fixes the "capped at 180 trying to move around" feel:
        // we only kick in when the player has hands off the strafe keys.
        if ((GetAsyncKeyState('A') & 0x8000) ||
            (GetAsyncKeyState('D') & 0x8000))
        {
            if (inAutoStrafe) { releaseStrafe(); inAutoStrafe = false; }
            return;
        }

        // SMART YIELD: when the player is actively turning their mouse
        // (yaw delta > 1.5°/frame), we let THEIR rotation drive the strafe
        // and just watch — we don't queue any A/D press. This kills the
        // "forces one direction and disables keyboard input takeover"
        // feeling: if you're swinging the mouse, the engine reads your
        // turn natively without us picking a competing direction.
        static bool yawTrackerInit = false;
        float curYawCheck = Mem::Read<Math::QAngle>(
            GameState::clientBase + GameState::RVA_dwViewAngles()).yaw;
        float yawDelta = 0.f;
        if (yawTrackerInit) {
            yawDelta = curYawCheck - prevTickYaw;
            if (yawDelta >  180.f) yawDelta -= 360.f;
            if (yawDelta < -180.f) yawDelta += 360.f;
        }
        prevTickYaw = curYawCheck;
        yawTrackerInit = true;
        DWORD nowMs = GetTickCount();
        if (fabsf(yawDelta) > cfg.yieldYawDeg)
            lastUserMouseMs = nowMs;
        // Brief grace window after the user stops turning so we don't
        // immediately re-grab control mid-swing.
        bool userDriving = (nowMs - lastUserMouseMs) < (DWORD)cfg.yieldGraceMs;
        if (userDriving)
        {
            if (inAutoStrafe) { releaseStrafe(); inAutoStrafe = false; }
            return;
        }

        inAutoStrafe = true;

        Math::Vec3 vel = Mem::Read<Math::Vec3>(localPawn + Offsets::m_vecVelocity);
        float speed2D = sqrtf(vel.x * vel.x + vel.y * vel.y);

        // ---- DESCENT-AWARE STRAFE (slope/jump-down momentum) ----
        // When falling fast (steep decline or jump-down) the engine
        // wants to clip our velocity into the slope normal on landing,
        // bleeding speed. Two compensations help here:
        //   1) Don't release W — keeping forward queued means landing
        //      tick wishvel is aligned with horizontal velocity, so
        //      the post-clip projection preserves the parallel speed
        //      component instead of zeroing it.
        //   2) Strafe more aggressively (no dead-zone) so any tiny
        //      mouse drift gets converted into accel, building speed
        //      on the way down before we slap into the slope.
        bool descending = (vel.z < -180.f);

        // ---- OPTIONAL SOFT SPEED CLAMP ----
        // Disabled by default (cfg.maxSpeed == 0). When enabled, releases
        // strafe above the cap so velocity coasts back into the band.
        if (cfg.maxSpeed > 0.f && speed2D > cfg.maxSpeed)
        {
            releaseStrafe();
            return;
        }

        // CRITICAL: in Source, holding W in air kills air acceleration.
        // Only force-release forward when the player ISN'T pressing it
        // themselves (we don't want to fight their input â€” if they hold
        // W intentionally for a jumpshot we leave it alone).
        //
        // EXCEPTION: when descending fast (jump-down / off-ledge / steep
        // ramp) we KEEP W queued. The forward wishdir aligned with
        // horizontal velocity is what survives the slope-clip on
        // landing tick — without it the engine zeroes our forward
        // component when it projects velocity onto the surface plane.
        if (!descending && !(GetAsyncKeyState('W') & 0x8000))
        {
            WriteBtnIfChanged(base, ButtonOffsets::forward, 256, lastBtnForward);
        }
        else if (descending)
        {
            // Keep forward queued through the descent so post-land
            // wishvel preserves horizontal speed.
            WriteBtnIfChanged(base, ButtonOffsets::forward, 65537, lastBtnForward);
        }
        if (!(GetAsyncKeyState('S') & 0x8000))
        {
            WriteBtnIfChanged(base, ButtonOffsets::back,    256, lastBtnBack);
        }

        float curYaw = Mem::Read<Math::QAngle>(
            GameState::clientBase + GameState::RVA_dwViewAngles()).yaw;

        // ---- direction = sign of "turn into desired direction" ----
        // dir < 0  \u2192 strafe LEFT (press A)
        // dir > 0  \u2192 strafe RIGHT (press D)
        // dir == 0 \u2192 no input (hold current speed)
        float dir = 0.f;

        if (cfg.strafeMode == 0)
        {
            // Velocity-vector mode \u2014 true "perfect strafe" math:
            //   in Source's air-accel, addspeed = wishspeed - dot(vel, wishdir)
            //   max gain occurs when wishdir is PERPENDICULAR to vel.
            //   Pressing A gives wishdir = -view_right; pressing D gives
            //   wishdir = +view_right. We pick whichever sign rotates
            //   velocity TOWARD the view yaw, which is also the choice
            //   that produces near-perpendicular wishdir vs vel for any
            //   small angle between vel and view.
            //
            // Edge cases:
            //   * speed too low \u2192 alternate kickstart (need any motion
            //     before the perpendicular math has meaning)
            //   * vel almost aligned with view \u2192 dead-zone, coast
            //   * vel almost opposite view (>150\u00b0) \u2192 still strafe in
            //     the rotation-shorter direction so we curve back
            if (speed2D < 30.f)
            {
                static bool kickRight = false;
                kickRight = !kickRight;
                dir = kickRight ? 1.f : -1.f;
            }
            else
            {
                float velYawDeg = atan2f(vel.y, vel.x) * (180.f / 3.14159265f);
                float diff = curYaw - velYawDeg;
                while (diff >  180.f) diff -= 360.f;
                while (diff < -180.f) diff += 360.f;

                // Tighter dead-zone (0.3\u00b0 vs 1.0\u00b0) so we keep micro-
                // correcting at high speed instead of letting velocity
                // drift off-axis. Zero dead-zone while descending so
                // any micro-yaw delta builds accel into the slope.
                float dz = descending ? 0.0f : 0.3f;
                if      (diff >  dz) dir = -1.f;
                else if (diff < -dz) dir =  1.f;
            }
        }
        else
        {
            // Legacy mouse-yaw delta mode.
            if (!yawInit) { savedYaw = curYaw; yawInit = true; return; }
            float delta = curYaw - savedYaw;
            savedYaw = curYaw;
            if (delta >  180.f) delta -= 360.f;
            if (delta < -180.f) delta += 360.f;

            if      (delta >  0.10f) dir = -1.f;  // turning left
            else if (delta < -0.10f) dir =  1.f;  // turning right
        }

        if (dir < 0.f)
        {
            WriteBtnIfChanged(base, ButtonOffsets::left,  65537, lastBtnLeft);
            WriteBtnIfChanged(base, ButtonOffsets::right,   256, lastBtnRight);
        }
        else if (dir > 0.f)
        {
            WriteBtnIfChanged(base, ButtonOffsets::right, 65537, lastBtnRight);
            WriteBtnIfChanged(base, ButtonOffsets::left,    256, lastBtnLeft);
        }
        else
        {
            WriteBtnIfChanged(base, ButtonOffsets::left,    256, lastBtnLeft);
            WriteBtnIfChanged(base, ButtonOffsets::right,   256, lastBtnRight);
        }
    }

    // ---------------------------------------------------------------
    // Auto Bhop â€” fully automatic jump on ground contact
    // Alternates between +jump and -jump each call to ensure
    // the game registers the input properly.
    // ---------------------------------------------------------------
    inline void DoBhop()
    {
        if (!cfg.enabled || !GameState::clientBase) return;

        // When the modern subtick path is owning the jump (default), we do
        // NOT touch the legacy kbutton slot. Three competing jump-driving
        // paths racing each tick is what produced the "jumping up/down
        // glitching" symptom. Subtick is the canonical path on 14155+.
        if (cfg.subtickJump) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        // Only bhop when the configured key is held
        if (!(GetAsyncKeyState(cfg.key) & 0x8000))
        {
            jumpToggle = false;
            return;
        }

        int32_t health = Mem::Read<int32_t>(localPawn + Offsets::m_iHealth);
        if (health <= 0) return;

        uint32_t flags = Mem::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        uintptr_t forceJump = GameState::clientBase + ButtonOffsets::jump;

        if (flags & FL_ONGROUND)
        {
            // On ground — alternate between press and release
            // This ensures the game sees a fresh key press each landing
            if (!jumpToggle)
            {
                Mem::Write<uint32_t>(forceJump, 65537); // +jump
                jumpToggle = true;
            }
            else
            {
                Mem::Write<uint32_t>(forceJump, 256);   // -jump
                jumpToggle = false;
            }
            hopCount++;
            lastHopTime = GetTickCount();
        }
        else
        {
            // In air — always release jump so we can re-press on landing
            Mem::Write<uint32_t>(forceJump, 256);
            jumpToggle = false;
        }
    }

    // ---------------------------------------------------------------
    // Speed preservation â€” zero stamina/velocity penalties
    // ---------------------------------------------------------------
    inline void PreserveSpeed()
    {
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        Mem::Write<float>(localPawn + m_flVelocityModifier_pawn, 1.0f);

        uintptr_t moveSvc = Mem::Read<uintptr_t>(localPawn + m_pMovementServices_pawn);
        if (moveSvc)
        {
            Mem::Write<float>(moveSvc + m_flStamina_movement,           0.f);
            Mem::Write<float>(moveSvc + m_flStaminaAtJumpStart_movement, 0.f);
            Mem::Write<float>(moveSvc + m_flAccumulatedJumpError_mov,    0.f);
        }
    }

    // ---------------------------------------------------------------
    // ModernJumpPrime â€” build 14158 added CCSPlayerModernJump with a
    // subtick press-tick guard. Writing the legacy +jump kbutton slot
    // (which the universal-dumper exposes as Buttons::jump) no longer
    // drives a jump on default servers (sv_legacy_jump 0): the press
    // event the server now consults lives in m_ModernJump on the local
    // movement services struct.
    //
    // We can't easily call the +iv_jump ConCommand from here without
    // resolving the engine's command dispatcher, so instead we forge
    // the press server-side state every frame the bhop key is held and
    // we're on the ground: bump m_nLastActualJumpPressTick to one tick
    // past the last landed tick. This is the same value the engine
    // would write if a real subtick press arrived right after touchdown,
    // so the spam-penalty timer (sv_jump_spam_penalty_time) sees a
    // fresh press every landing and the jump fires.
    //
    // We also keep the legacy struct fields in sync as a belt-and-
    // braces fallback for sv_legacy_jump 1 servers.
    // ---------------------------------------------------------------
    inline void ModernJumpPrime(bool keyHeld, bool onGround)
    {
        if (!cfg.enabled) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;
        uintptr_t moveSvc = Mem::Read<uintptr_t>(localPawn + m_pMovementServices_pawn);
        if (!moveSvc) return;

        uintptr_t modern = moveSvc + m_ModernJump_movement;
        uintptr_t legacy = moveSvc + m_LegacyJump_movement;

        if (keyHeld && onGround)
        {
            // CRITICAL: forge ONCE per landing, not every render frame.
            //
            // The previous unconditional write hammered LastActualJumpPress
            // Tick = landed+1 every frame. After ~5–6s the engine's spam
            // penalty timer (sv_jump_spam_penalty_time) flagged it as a
            // stuck-press pattern and started rejecting jumps, producing
            // the "glitching up/down" symptom. Gating on landedTick change
            // means the press is forged exactly when a real player would
            // press: once per touchdown.
            int32_t landedTick = Mem::Read<int32_t>(modern + m_ModernJump_LastLandedTick_off);
            if (landedTick != lastPrimedLanded)
            {
                int32_t pressTick = landedTick + 1;
                Mem::Write<int32_t>(modern + m_ModernJump_LastActualJumpPressTick_off, pressTick);
                Mem::Write<int32_t>(modern + m_ModernJump_LastUsableJumpPressTick_off, pressTick);
                Mem::Write<float>  (modern + m_ModernJump_LastActualJumpPressFrac_off, 0.f);
                Mem::Write<float>  (modern + m_ModernJump_LastUsableJumpPressFrac_off, 0.f);

                // Legacy fallback (sv_legacy_jump 1 servers)
                Mem::Write<bool> (legacy + m_LegacyJump_OldJumpPressed_off,  true);
                Mem::Write<float>(legacy + m_LegacyJump_JumpPressedTime_off, 0.f);

                lastPrimedLanded = landedTick;
            }
        }
        else if (!keyHeld)
        {
            // Drop legacy press flag while key released, reset prime tracker
            Mem::Write<bool>(legacy + m_LegacyJump_OldJumpPressed_off, false);
            lastPrimedLanded = INT32_MIN;
        }
    }

    // ---------------------------------------------------------------
    // GetLastJumpVelZ â€” reads m_flLastJumpVelocityZ from the movement
    // services block.  Aimbot's jumpshot/triggerbot use this to find
    // the actual apex of the current jump rather than guessing from
    // the noisy live m_vecVelocity.z (which oscillates because the
    // physics integrator updates between client+server ticks).
    // ---------------------------------------------------------------
    inline float GetLastJumpVelZ()
    {
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return 0.f;
        uintptr_t moveSvc = Mem::Read<uintptr_t>(localPawn + m_pMovementServices_pawn);
        if (!moveSvc) return 0.f;
        return Mem::Read<float>(moveSvc + m_flLastJumpVelocityZ_movement);
    }

    // ---------------------------------------------------------------
    // ApplySubtickInputs â€” the MODERN bhop path (build 14155+).
    //
    // The kbutton path (writing 65537/256 to client.dll button slots)
    // works but is timing-fragile: the slot is consumed once per render
    // frame, the engine debounces fresh presses, and on slopes/stairs
    // FL_ONGROUND flickers between true/false sub-tick which causes
    // missed jumps. The MODERN path the engine itself uses for the
    // "subtick input" (sv_subtick_legacy_kbinds 0) treats every input
    // as a fractional tick event:
    //
    //   m_arrForceSubtickMoveWhen[slot] = fractional position 0..1
    //   m_nQueuedButtonDownMask        |= bit       (set DOWN)
    //   m_nQueuedButtonChangeMask      |= bit       (mark CHANGED)
    //
    // This is what gamesense / NL / pandora bhop call directly. The
    // engine then synthesises a sub-tick press event at exactly that
    // fraction of the upcoming tick, which:
    //   * always fires on the same tick (no edge-case landed-tick race)
    //   * is sub-tick precise so it survives slopes/stair-step where
    //     FL_ONGROUND is true for only 1-2ms per land-bounce
    //   * is the ONLY path sv_legacy_jump 0 servers (the default) honour
    //
    // For the autostrafe we ALSO write directly to:
    //   m_flCmdLeftMove (raw input axis)  AND
    //   m_flLeftMove    (post-clip applied axis)
    // This bypasses the kbutton +moveleft/+moveright entirely so the
    // strafe responds in the same tick we wrote it (kbutton has a
    // 1-tick latency through CInput::JoyStickMove). It also means the
    // bhop is INDEPENDENT of whether the user is holding any WASD key
    // â€” true hands-free.
    //
    // Subtick slot indices:
    //   0 = ATTACK
    //   1 = ATTACK2
    //   2 = JUMP   <- our slot
    //   3 = DUCK
    // ---------------------------------------------------------------
    inline void ApplySubtickInputs(bool keyHeld, bool onGround)
    {
        if (!cfg.enabled || !cfg.subtickJump) return;
        if (!keyHeld) return;

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;
        uintptr_t moveSvc = Mem::Read<uintptr_t>(localPawn + m_pMovementServices_pawn);
        if (!moveSvc) return;

        // --- Subtick JUMP press ---
        //
        // Gate: must be effectively grounded AND must NOT have already
        // jumped since the latest landing. The engine spam-guards back-
        // to-back press queues; without this gate we'd flood the queue
        // every render frame and the engine would discard our presses.
        //
        // "Effectively grounded" means either FL_ONGROUND right now, OR
        // we landed more recently than we last jumped (covers slope/
        // stair flicker where FL_ONGROUND drops for 1-2 sub-ticks).
        int32_t lastLanded = Mem::Read<int32_t>(
            moveSvc + m_ModernJump_movement + m_ModernJump_LastLandedTick_off);
        int32_t lastJump   = Mem::Read<int32_t>(
            moveSvc + m_nLastJumpTick_movement);

        bool effectiveGround = onGround || (lastLanded > lastJump);
        bool freshLanding    = (lastLanded >= lastJump);

        // ONE QUEUE PER LANDING. Without this gate we re-stamped the
        // subtick "when" + change-mask every render frame (240+ Hz),
        // and the engine would consume the press, then immediately see
        // the SAME press queued again on the very next frame — producing
        // the rapid micro-jump glitching the user reported.
        if (effectiveGround && freshLanding && lastLanded != lastQueuedLanded)
        {
            // Tiny positive fraction = press at the very start of the
            // upcoming tick. 0.0 is treated as "unset".
            Mem::Write<float>(moveSvc + m_arrForceSubtickMoveWhen_movement
                              + sizeof(float) * Offsets::kSubtickSlot_Jump,
                              0.001f);

            // Engine ignores subtick "when" entries whose bit isn't ALSO
            // present in change-mask â€” it's the change-mask that drives
            // subtick recognition.
            uint64_t down = Mem::Read<uint64_t>(moveSvc + m_nQueuedButtonDownMask_movement);
            uint64_t chng = Mem::Read<uint64_t>(moveSvc + m_nQueuedButtonChangeMask_movement);
            Mem::Write<uint64_t>(moveSvc + m_nQueuedButtonDownMask_movement,
                                 down | Offsets::IN_BTN_JUMP);
            Mem::Write<uint64_t>(moveSvc + m_nQueuedButtonChangeMask_movement,
                                 chng | Offsets::IN_BTN_JUMP);

            lastQueuedLanded = lastLanded;
        }

        // NOTE: subtick STRAFE removed. Writing m_flCmdLeftMove + zeroing
        // m_flCmdForwardMove every render frame caused the engine to
        // renormalize wishvel against m_flMaxspeed, capping speed at
        // ~180 ups when the player was just trying to move around.
        // The kbutton AutoStrafe() path is the correct strafe driver â€”
        // it lets CInput produce the sidemove value AFTER engine clamps.
    }

    // ---------------------------------------------------------------
    // Called from SilentAim::hkCreateMove (tick-synced)
    // ---------------------------------------------------------------
    inline void OnCreateMove(__int64 /*inputPtr*/)
    {
        if (!cfg.enabled) return;
        __try {
            DoBhop();
            AutoStrafe();
            PreserveSpeed();
            uintptr_t lp = GameState::GetLocalPawn();
            if (lp) {
                uint32_t flags = Mem::Read<uint32_t>(lp + Offsets::m_fFlags);
                bool og = (flags & FL_ONGROUND) != 0;
                bool kh = (GetAsyncKeyState(cfg.key) & 0x8000) != 0;
                ModernJumpPrime(kh, og);
                ApplySubtickInputs(kh, og);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Legacy API compatibility
    inline void Init() { hookInstalled = true; }
    inline void Shutdown() { hookInstalled = false; }

    // ---------------------------------------------------------------
    // Tick â€” called from PresentCore (render loop, every frame)
    // Runs full auto bhop + auto strafe + speed preservation.
    // This ensures bhop works even if CreateMove hook is not installed.
    // ---------------------------------------------------------------
    inline void Tick()
    {
        // Edge-detect disable: when bhop is turned off, release every
        // button slot we may have stamped. Without this the kbutton
        // cache stays at 65537 / 256 from the last frame and the next
        // time the user manually moves, the engine sees a stale press
        // bit on +moveleft / +moveright — producing the symptom of
        // "input being held against my will after disabling".
        if (!cfg.enabled)
        {
            if (prevWasEnabled && GameState::clientBase)
            {
                uintptr_t base = GameState::clientBase;
                Mem::Write<uint32_t>(base + ButtonOffsets::jump,    256);
                Mem::Write<uint32_t>(base + ButtonOffsets::left,    256);
                Mem::Write<uint32_t>(base + ButtonOffsets::right,   256);
                Mem::Write<uint32_t>(base + ButtonOffsets::forward, 256);
                Mem::Write<uint32_t>(base + ButtonOffsets::back,    256);
                lastBtnLeft = lastBtnRight = lastBtnForward = lastBtnBack = 256;
                inAutoStrafe = false;
                jumpToggle = false;
                lastPrimedLanded = INT32_MIN;
                lastQueuedLanded = INT32_MIN;
                yawInit = false;
                prevWasEnabled = false;
            }
            return;
        }
        prevWasEnabled = true;

        if (!GameState::clientBase) return;

        __try {

        DoBhop();
        AutoStrafe();
        PreserveSpeed();

        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn) return;

        uint32_t flags = Mem::Read<uint32_t>(localPawn + Offsets::m_fFlags);
        bool onGround = (flags & FL_ONGROUND) != 0;
        bool keyHeld  = (GetAsyncKeyState(cfg.key) & 0x8000) != 0;

        // 14158 Modern Jump primer â€” forge a fresh press tick each
        // ground frame so the subtick spam guard accepts our jump.
        ModernJumpPrime(keyHeld, onGround);

        // Modern subtick path â€” sub-tick precise jump + direct sidemove
        // axis writes. This is what survives slope/stair landings and
        // pins the glide to the configured speed band.
        ApplySubtickInputs(keyHeld, onGround);

        if (onGround && !wasOnGround)
        {
            hopCount++;
            lastHopTime = GetTickCount();
        }
        wasOnGround = onGround;

        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    // ---------------------------------------------------------------
    // Velocity Display â€” rendered as HUD overlay showing current
    // horizontal speed, peak speed, and hop count.
    // ---------------------------------------------------------------
    inline float currentSpeed  = 0.f;
    inline float peakSpeed     = 0.f;
    inline DWORD peakResetTime = 0;

    inline void RenderVelocity()
    {
        if (!cfg.showVelocity) return;
        if (!GameState::clientBase) return;

        __try {
            uintptr_t lp = GameState::GetLocalPawn();
            if (!lp) return;

            int32_t hp = Mem::Read<int32_t>(lp + Offsets::m_iHealth);
            if (hp <= 0) return;

            // Read velocity (horizontal only: x,y)
            Math::Vec3 vel = Mem::Read<Math::Vec3>(lp + Offsets::m_vecVelocity);
            currentSpeed = sqrtf(vel.x * vel.x + vel.y * vel.y);

            // Track peak speed (reset after 3 seconds of no new peak)
            DWORD now = GetTickCount();
            if (currentSpeed > peakSpeed)
            {
                peakSpeed = currentSpeed;
                peakResetTime = now;
            }
            if (now - peakResetTime > 3000)
                peakSpeed = currentSpeed;

            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            if (!dl) return;

            ImVec2 disp = ImGui::GetIO().DisplaySize;
            float cx = disp.x * 0.5f;
            float cy = disp.y * 0.82f;

            // Speed color: green < 250, yellow 250-300, red > 300
            ImU32 speedCol;
            if (currentSpeed > 300.f)
                speedCol = IM_COL32(255, 60, 60, 255);
            else if (currentSpeed > 250.f)
                speedCol = IM_COL32(255, 200, 60, 255);
            else
                speedCol = IM_COL32(180, 255, 180, 255);

            // Main speed number
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0f u/s", currentSpeed);
            ImVec2 sz = ImGui::CalcTextSize(buf);
            dl->AddText(ImVec2(cx - sz.x * 0.5f + 1, cy + 1), IM_COL32(0,0,0,180), buf);
            dl->AddText(ImVec2(cx - sz.x * 0.5f, cy), speedCol, buf);

            // Peak + hops (smaller, below)
            if (cfg.enabled && hopCount > 0)
            {
                snprintf(buf, sizeof(buf), "peak: %.0f | hops: %d", peakSpeed, hopCount);
                ImVec2 sz2 = ImGui::CalcTextSize(buf);
                float y2 = cy + sz.y + 2;
                dl->AddText(ImVec2(cx - sz2.x * 0.5f + 1, y2 + 1), IM_COL32(0,0,0,120), buf);
                dl->AddText(ImVec2(cx - sz2.x * 0.5f, y2), IM_COL32(200,200,200,200), buf);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
}
