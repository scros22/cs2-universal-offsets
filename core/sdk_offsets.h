#pragma once

// ---------------------------------------------------------------
// CS2 SDK Auto-Offset Layer
// Pulls all offsets directly from the cs2-dumper generated headers.
// To update: run cs2-dumper, rebuild. No manual editing required.
// ---------------------------------------------------------------

#include <cstddef>
#include <cstdint>

// Dumper-generated SDK headers
#include "../sdk/offsets.hpp"
#include "../sdk/client_dll.hpp"
#include "../sdk/buttons.hpp"

namespace Offsets
{
    // ---- Global offsets (auto-sourced from sdk/offsets.hpp) ----
    namespace Global
    {
        using namespace cs2_dumper::offsets::client_dll;
        using namespace cs2_dumper::offsets::engine2_dll;
    }

    // ---- Button addresses (auto-sourced from sdk/buttons.hpp) ----
    namespace Buttons
    {
        using namespace cs2_dumper::buttons;
    }

    // ---- Entity-system primitives (CGameEntitySystem layout) ----
    // These are reverse-engineered constants — not in the schema dump —
    // but stable across CS2 builds. Use these instead of bare magic
    // numbers so a future layout change can be patched in one place.
    namespace EntitySys
    {
        constexpr std::ptrdiff_t kChunkArrayBase   = 0x10;   // first chunk pointer slot
        constexpr std::ptrdiff_t kChunkPtrStride   = 0x8;    // sizeof(void*)
        constexpr std::ptrdiff_t kChunkEntryStride = 0x70;   // bytes between entity slots inside a chunk
        constexpr uint32_t       kSlotIndexMask    = 0x1FF;  // 512 entries per chunk
        constexpr uint32_t       kHandleIndexMask  = 0x7FFF; // CHandle index bits

        // Per-entity: CEntityInstance layout
        constexpr std::ptrdiff_t kInstanceToIdentity = 0x10; // CEntityInstance + 0x10 -> CEntityIdentity*
        // CEntityIdentity layout
        constexpr std::ptrdiff_t kIdentityDesignerName = 0x20; // CUtlSymbolLarge -> char*
    }

    // ---- CGameSceneNode hand-RE'd offsets (not in schema dump) ----
    namespace SceneNode
    {
        constexpr std::ptrdiff_t kDormant = 0x10B;  // bool m_bDormant (chams/ESP rely on this)
    }

    // ---- Entity schema offsets ----
    // These are tied to CS2 build and should be re-confirmed with each
    // major update. Dumper generates schemas in client_dll.hpp but
    // we define them here as constants for quick access without
    // parsing the full schema namespace at every callsite.

    // C_BaseEntity (build 14152)
    constexpr std::ptrdiff_t m_pGameSceneNode      = 0x330;
    constexpr std::ptrdiff_t m_nSubclassID         = 0x380;
    constexpr std::ptrdiff_t m_hOwnerEntity        = 0x520;
    constexpr std::ptrdiff_t m_iHealth             = 0x34C;
    constexpr std::ptrdiff_t m_lifeState           = 0x354;
    constexpr std::ptrdiff_t m_iTeamNum            = 0x3EB;
    constexpr std::ptrdiff_t m_flSimulationTime    = 0x3B8;
    constexpr std::ptrdiff_t m_fFlags              = 0x3F8;

    // C_BasePlayerPawn
    constexpr std::ptrdiff_t m_pWeaponServices     = 0x11E0;
    constexpr std::ptrdiff_t m_vecViewOffset        = 0xE70;
    constexpr std::ptrdiff_t m_pObserverServices   = 0x11F8;
    constexpr std::ptrdiff_t m_pCameraServices     = 0x1218;

    // CPlayer_WeaponServices
    constexpr std::ptrdiff_t m_hMyWeapons          = 0x48;
    constexpr std::ptrdiff_t m_hActiveWeapon       = 0x60;

    // C_BaseCombatCharacter
    constexpr std::ptrdiff_t m_hMyWearables        = 0x1158;

    // CPlayer_ObserverServices
    constexpr std::ptrdiff_t m_hObserverTarget     = 0x4C;
    constexpr std::ptrdiff_t m_iObserverMode       = 0x48;

    // CCSPlayerController
    constexpr std::ptrdiff_t m_pInventoryServices  = 0x808;
    constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x858;
    constexpr std::ptrdiff_t m_hPlayerPawn         = 0x904;
    constexpr std::ptrdiff_t m_bPawnIsAlive        = 0x90C;
    constexpr std::ptrdiff_t m_hObserverPawn       = 0x908;

    // CCSPlayerController_InventoryServices
    constexpr std::ptrdiff_t m_unMusicID           = 0x58;

    // C_CSPlayerPawn
    // 14154: glove block shifted -0x40 (same drift family as spotted-state).
    // Verified against universal-dumper 2026-04-24 dump (build 14154).
    constexpr std::ptrdiff_t m_bNeedToReApplyGloves = 0x1655;  // 14154 dump 24-04-26
    constexpr std::ptrdiff_t m_EconGloves          = 0x1658;   // 14154 dump 24-04-26
    // NOTE: post-shift fields below verified against a2x/cs2-dumper 2026-04-22
    // (build 14152). Many were off by +0x40 vs the actual schema.
    constexpr std::ptrdiff_t m_hHudModelArms       = 0x1B58;
    // m_pClippingWeapon: schema field removed in build 14152 (Animgraph 2).
    // Skinchanger features that depended on it should derive the active weapon
    // pointer from m_hActiveWeapon via the entity list instead.
    constexpr std::ptrdiff_t m_pClippingWeapon     = 0x0;
    constexpr std::ptrdiff_t m_nEconGlovesChanged  = 0x1AC8;   // 14153 a2x
    // IDA-VERIFIED 2026-04-26 (build 14154): sub_180BB0B10 @ 0x180BB1D98
    // `mov dword [rax+10h], 1C5Ch` after lea "m_iShotsFired".
    constexpr std::ptrdiff_t m_iShotsFired         = 0x1C5C;
    // 14153: m_aimPunchAngle no longer exists on the pawn directly.
    // The punch QAngle now lives inside CCSPlayer_AimPunchServices, pointed to
    // by m_pAimPunchServices on the pawn. Helper accessors live in features that
    // need it. Kept as legacy alias = 0 so any stale write is a no-op (safe).
    constexpr std::ptrdiff_t m_aimPunchAngle       = 0x0;      // DEPRECATED — use indirection
    // IDA-VERIFIED 2026-04-26 (build 14154): sub_180BB0B10 @ 0x180BB1192
    // `mov dword [...var_90], 1490h` after lea "m_pAimPunchServices".
    constexpr std::ptrdiff_t m_pAimPunchServices   = 0x1490;   // CCSPlayer_AimPunchServices*
    constexpr std::ptrdiff_t m_predictableBaseAngle_inAimPunch = 0x50; // QAngle inside services
    constexpr std::ptrdiff_t m_vecCsViewPunchAngle_inCamSvc    = 0x48; // QAngle inside m_pCameraServices
    // v_angle on C_BasePlayerPawn — final view angles AFTER punch is applied.
    // Per recent reversing notes, useful as an alternative read path on builds
    // where m_angEyeAngles drifts. We keep both available.
    constexpr std::ptrdiff_t v_angle               = 0x1298;
    // IDA-VERIFIED 2026-04-26 (build 14154): schema-registration site
    // sub_180BB0B10 @ 0x180BB1F26 emits `mov dword [rax+10h], 3300h`
    // immediately after `lea rdx, "m_angEyeAngles"`. Matches universal-
    // dumper latest. Used for BOTH read (target solver) and write (normal
    // aim path); a wrong value here silently breaks the entire aimbot.
    constexpr std::ptrdiff_t m_angEyeAngles        = 0x3300;
    constexpr std::ptrdiff_t m_ArmorValue          = 0x1C74;
    constexpr std::ptrdiff_t m_bGunGameImmunity    = 0x3278;  // unverified, was 0x32B8 (-0x40)
    constexpr std::ptrdiff_t m_iIDEntIndex         = 0x33DC;
    constexpr std::ptrdiff_t m_flFlashDuration     = 0x1400;
    constexpr std::ptrdiff_t m_flFlashMaxAlpha     = 0x13FC;
    constexpr std::ptrdiff_t m_flLastSmokeOverlayAlpha = 0x1420;
    constexpr std::ptrdiff_t m_bIsScoped           = 0x1C48;
    constexpr std::ptrdiff_t m_entitySpottedState   = 0x1C30;  // was 0x1C70 (-0x40)
    // EntitySpottedState_t::m_bSpottedByMask — uint32[2] bitmask of player
    // slots currently looking at this entity. Anti-detection uses this to
    // soften silent aim when an enemy has us in their PVS / on screen.
    constexpr std::ptrdiff_t m_bSpottedByMask_inSpottedState = 0xC;

    // C_CSGameRules — m_bIsValveDS = true on Valve official matchmaking
    // dedicated servers (the only place Overwatch demos are pulled from
    // and where VAC Live actively scrutinizes input). Anti-detection uses
    // this to throttle silent aim flicks far harder on official servers.
    constexpr std::ptrdiff_t m_bIsValveDS          = 0xA4;
    constexpr std::ptrdiff_t m_bHasMatchStarted    = 0xB0;
    // C_CSGameRules — freeze/warmup gates. Silent aim during freeze time
    // (round-start "armory" where attacks are disabled by the server) is
    // an absolutely free server-side bot-detection signal: the angle write
    // is still serialised even though no shot can fire. Skip rewrites
    // entirely while either flag is set.
    constexpr std::ptrdiff_t m_bFreezePeriod       = 0x40;
    constexpr std::ptrdiff_t m_bWarmupPeriod       = 0x41;

    // C_CSPlayerPawn — m_bWaitForNoAttack is set true by the server
    // immediately after weapon switch / respawn / round-restart and stays
    // true until the player RELEASES attack and re-presses it. Silent
    // firing while this is set produces "fired before client could have
    // pressed M1 again" — pure bot signature, zero false-positive rate.
    // VERIFIED via IDA schema-registration sub_180BB0AB0 @ build 14154:
    // mov [rcx+10h], 1C68h immediately after lea "m_bWaitForNoAttack".
    // Previous value (0x1CA8) was +0x40 high — same drift we already
    // fixed for m_entitySpottedState. Universal-dumper + IDA agree.
    constexpr std::ptrdiff_t m_bWaitForNoAttack    = 0x1C68;

    // C_CSPlayerPawn — defuse / hostage-rescue locks. Server forbids
    // attack inputs while either bool is true; angle desync alone
    // during a defuse is the worst possible silent-aim signature.
    // VERIFIED via IDA schema-registration sub_180BB0AB0 @ build 14154:
    // mov [rcx+10h], 1C4Ah / 1C4Bh after the respective string lea.
    // Previously had 0x1C8A / 0x1C8B (+0x40 drift, same as spotted-state).
    constexpr std::ptrdiff_t m_bIsDefusing         = 0x1C4A;
    constexpr std::ptrdiff_t m_bIsGrabbingHostage  = 0x1C4B;

    // C_BaseEntity — m_MoveType. MoveType_t::WALK = 2 (normal),
    // FLYGRAVITY = 4 (airborne due to gravity). Anything else
    // (LADDER=9, NOCLIP=7, OBSERVER=8, NONE=0) means attack inputs
    // either don't reach FireBullet or are obvious bot signals.
    constexpr std::ptrdiff_t m_MoveType            = 0x525;
    enum MoveType_t : uint8_t {
        MOVETYPE_NONE       = 0,
        MOVETYPE_WALK       = 2,
        MOVETYPE_FLYGRAVITY = 4,
        MOVETYPE_NOCLIP     = 7,
        MOVETYPE_OBSERVER   = 8,
        MOVETYPE_LADDER     = 9,
    };

    // C_CSWeaponBaseGun — m_bNeedsBoltAction: AWP / SSG-08 / Scout
    // post-shot animation lockout while the bolt cycles. Server drops
    // any attack until this flips back to false. Suppress.
    constexpr std::ptrdiff_t m_bNeedsBoltAction    = 0x1CCD;

    // C_CSWeaponBaseGun — m_zoomLevel: 0 = unscoped, 1 = first zoom,
    // 2 = second zoom (AWP only). No-scope silent-fire on AWP/SSG
    // is one of the loudest reportable patterns in the game; no human
    // hits no-scopes consistently. Refuse silent-aim rewrites while
    // holding a sniper that isn't currently scoped.
    constexpr std::ptrdiff_t m_zoomLevel           = 0x1CB0;

    // C_BasePlayerWeapon — m_nNextPrimaryAttackTick is the absolute
    // server tick at which the weapon is next allowed to fire. Compare
    // against CBasePlayerController::m_nTickBase to know whether a
    // silent shot can actually convert into a bullet THIS tick. Firing
    // silently before this tick reaches its value cannot produce a
    // bullet (the server discards the attack) but the angle desync IS
    // still serialised to the demo — pure liability, suppress.
    constexpr std::ptrdiff_t m_nNextPrimaryAttackTick      = 0x16C8;
    constexpr std::ptrdiff_t m_flNextPrimaryAttackTickRatio = 0x16CC;

    // C_BasePlayerWeapon — m_iClip1: bullets currently loaded. Silent
    // firing with clip == 0 is a guaranteed bot signal; the server
    // discards the attack but the angle desync still hits the demo.
    constexpr std::ptrdiff_t m_iClip1                       = 0x16D8;

    // C_CSWeaponBase — m_bInReload: weapon is mid-reload animation.
    // No bullet can leave the gun until reload completes; silent
    // angle writes during this window are pure liability.
    constexpr std::ptrdiff_t m_bInReload                    = 0x17F4;

    // CBasePlayerController — m_nTickBase: the local tick counter the
    // server uses for all of the player's weapon/movement timers.
    constexpr std::ptrdiff_t m_nTickBase            = 0x6B8;

    // C_CSPlayerPawn — m_iShotsFired increments per bullet (auto-spray)
    // and is reset by the server when the player stops shooting AND
    // recoil decays. Useful for: post-shot suppression timing, rate-of-
    // fire sanity (firing > magazine size in <X ticks = bot signal).
    // (Already defined as 0x1C5C earlier in this header — value here is
    //  cross-checked against the schema dump @ 0x1C9C; the prior entry
    //  is the live one. Anti-detection code uses Offsets::m_iShotsFired.)

    // C_CSWeaponBase — m_iRecoilIndex is the server-tracked shot count
    // in the current burst (pair to m_flRecoilIndex defined later).
    // Server-vs-client divergence here is a known anti-cheat heuristic.
    constexpr std::ptrdiff_t m_iRecoilIndex         = 0x17DC;

    // C_EconEntity (parent of C_BasePlayerWeapon) — attribute container
    // chain to the item definition index.  m_iItemDefinitionIndex sits
    // inside m_AttributeManager.m_Item, so the absolute offset on a
    // weapon entity is 0x13B8 (m_AttributeManager) + 0x50 (m_Item) +
    // 0x1BA (m_iItemDefinitionIndex) = 0x15C2.
    // (m_AttributeManager / m_Item / m_iItemDefinitionIndex are defined
    //  later in this file under C_AttributeContainer / C_EconItemView.)
    constexpr std::ptrdiff_t kWeaponItemDefIndexOffset = 0x13B8 + 0x50 + 0x1BA; // 0x15C2

    // CS2 sniper weapon item-definition indices (stable since 2014).
    // Used by anti-detection to refuse silent firing while no-scoped.
    constexpr uint16_t kItemDefAWP    = 9;
    constexpr uint16_t kItemDefSSG08  = 40;
    constexpr uint16_t kItemDefG3SG1  = 11;
    constexpr uint16_t kItemDefSCAR20 = 38;

    // C_BaseModelEntity — alpha property
    constexpr std::ptrdiff_t m_pClientAlphaProperty = 0xF50;

    // CClientAlphaProperty
    constexpr std::ptrdiff_t m_nAlpha              = 0x17;

    // CSmokeGrenadeProjectile
    constexpr std::ptrdiff_t m_nSmokeEffectTickBegin = 0x1250;
    constexpr std::ptrdiff_t m_bDidSmokeEffect     = 0x1254;
    constexpr std::ptrdiff_t m_vSmokeColor         = 0x125C;  // Vector (3 floats)
    constexpr std::ptrdiff_t m_bSmokeEffectSpawned = 0x1299;

    // C_Inferno
    constexpr std::ptrdiff_t m_fireCount           = 0x1958;
    constexpr std::ptrdiff_t m_nFireEffectTickBegin = 0x196C;

    // C_BaseEntity velocity
    constexpr std::ptrdiff_t m_vecVelocity         = 0x430;

    // CPlayer_CameraServices
    constexpr std::ptrdiff_t m_iFOV                = 0x290;   // uint32, NOT float
    constexpr std::ptrdiff_t m_iFOVStart           = 0x294;   // uint32
    constexpr std::ptrdiff_t m_flFOVTime           = 0x298;   // GameTime_t
    constexpr std::ptrdiff_t m_flFOVRate           = 0x29C;   // float32
    constexpr std::ptrdiff_t m_hZoomOwner          = 0x2A0;   // CHandle
    constexpr std::ptrdiff_t m_flLastShotFOV       = 0x2A4;   // float32

    // CBasePlayerController — desired-FOV (lives on the CONTROLLER, not the pawn).
    // Writing this on the pawn corrupts random fields and crashes the game.
    constexpr std::ptrdiff_t m_iDesiredFOV_OnController = 0x784;   // uint32 (build 14152)

    // C_EnvSky (renamed in 14152: m_flSkyBrightnessScale -> m_flBrightnessScale)
    constexpr std::ptrdiff_t m_vTintColor          = 0xFB9;   // Color (RGBA bytes)
    constexpr std::ptrdiff_t m_vTintColorLightingOnly = 0xFBD; // Color
    constexpr std::ptrdiff_t m_flSkyBrightnessScale = 0xFC4;  // float32 (m_flBrightnessScale)
    constexpr std::ptrdiff_t m_bSkyEnabled         = 0xFDC;   // bool (m_bEnabled on C_EnvSky)

    // CGameSceneNode
    constexpr std::ptrdiff_t m_vecAbsOrigin        = 0xC8;

    // C_BaseEntity — viewmodel detection
    constexpr std::ptrdiff_t m_bRenderWithViewModels = 0x56A;

    // C_TonemapController2 — exposure control
    constexpr std::ptrdiff_t m_flAutoExposureMin   = 0x600;
    constexpr std::ptrdiff_t m_flAutoExposureMax   = 0x604;

    // CSkeletonInstance
    constexpr std::ptrdiff_t m_materialGroup       = 0x3C4;
    constexpr std::ptrdiff_t m_modelState          = 0x150;

    // CModelState (m_BoneArray is hand-RE'd, not in schema)
    constexpr std::ptrdiff_t m_MeshGroupMask       = 0x1C8;
    constexpr std::ptrdiff_t m_BoneArray           = 0x80;
    // Model handle / name on CModelState (schema-verified, build 14154,
    // dumps/latest/sdk/client_dll.hpp lines 4585-4586). Used by the
    // model-changer feature to copy a donor pawn's model resource handle
    // onto our local pawn (cheap client-side agent skin swap — no hooks,
    // pure memory write). m_ModelName is read-only diagnostics.
    constexpr std::ptrdiff_t m_hModel              = 0xA0;  // CStrongHandle<InfoForResourceTypeCModel>
    constexpr std::ptrdiff_t m_ModelName_state     = 0xA8;  // CUtlSymbolLarge (interned string ptr)

    // C_BaseModelEntity (render / visual)
    constexpr std::ptrdiff_t m_nRenderMode         = 0xC78;
    constexpr std::ptrdiff_t m_clrRender           = 0xC98;
    constexpr std::ptrdiff_t m_Glow                = 0xDD8;
    constexpr std::ptrdiff_t m_flGlowBackfaceMult  = 0xE30;
    constexpr std::ptrdiff_t m_ClientOverrideTint  = 0xF58;
    constexpr std::ptrdiff_t m_bUseClientOverrideTint = 0xF5C;

    // CGlowProperty
    constexpr std::ptrdiff_t m_bGlowing            = 0x51;
    constexpr std::ptrdiff_t m_glowColorOverride   = 0x40;
    constexpr std::ptrdiff_t m_iGlowType           = 0x30;
    constexpr std::ptrdiff_t m_nGlowRange          = 0x38;
    constexpr std::ptrdiff_t m_nGlowRangeMin       = 0x3C;
    constexpr std::ptrdiff_t m_flGlowTime          = 0x48;
    constexpr std::ptrdiff_t m_flGlowStartTime     = 0x4C;

    // C_EconEntity
    constexpr std::ptrdiff_t m_AttributeManager    = 0x13B8;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidLow  = 0x1650;
    constexpr std::ptrdiff_t m_OriginalOwnerXuidHigh = 0x1654;
    constexpr std::ptrdiff_t m_nFallbackPaintKit   = 0x1658;
    constexpr std::ptrdiff_t m_nFallbackSeed       = 0x165C;
    constexpr std::ptrdiff_t m_flFallbackWear      = 0x1660;
    constexpr std::ptrdiff_t m_nFallbackStatTrak   = 0x1664;

    // C_AttributeContainer
    constexpr std::ptrdiff_t m_Item                = 0x50;

    // C_EconItemView
    constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
    constexpr std::ptrdiff_t m_iEntityQuality      = 0x1BC;
    constexpr std::ptrdiff_t m_iItemID             = 0x1C8;
    constexpr std::ptrdiff_t m_iItemIDHigh         = 0x1D0;
    constexpr std::ptrdiff_t m_iItemIDLow          = 0x1D4;
    constexpr std::ptrdiff_t m_iAccountID          = 0x1D8;
    constexpr std::ptrdiff_t m_bInitialized        = 0x1E8;
    constexpr std::ptrdiff_t m_bDisallowSOC       = 0x1E9;
    constexpr std::ptrdiff_t m_bRestoreCustomMaterialAfterPrecache = 0x1B8;
    constexpr std::ptrdiff_t m_AttributeList       = 0x208;
    constexpr std::ptrdiff_t m_NetworkedDynamicAttributes = 0x280;
    constexpr std::ptrdiff_t m_szCustomName        = 0x2F8;
    constexpr std::ptrdiff_t m_szCustomNameOverride = 0x399;

    // CAttributeList
    constexpr std::ptrdiff_t m_Attributes          = 0x8;

    // CEconItemAttribute
    constexpr std::ptrdiff_t m_iAttributeDefinitionIndex = 0x30;
    constexpr std::ptrdiff_t m_flValue             = 0x34;
    constexpr std::ptrdiff_t m_flInitialValue      = 0x38;
    constexpr std::ptrdiff_t m_nRefundableCurrency = 0x3C;
    constexpr std::ptrdiff_t m_bSetBonus           = 0x40;

    // C_CSWeaponBase — server-replicated firing state.
    // Used by the seeded triggerbot to predict whether the next shot
    // will land inside the target's hitbox cone before pulling the
    // trigger. Verified against build 14154 schema dump (Apr 2026).
    // Schema reg site: sub_18078F9B0 (client.dll). Cross-checked
    // against dumps/latest/offsets/sdk/client_dll.hpp.
    constexpr std::ptrdiff_t m_flTurningInaccuracyDelta = 0x17BC; // float
    constexpr std::ptrdiff_t m_flTurningInaccuracy      = 0x17CC; // float — extra cone from view turn rate
    constexpr std::ptrdiff_t m_fAccuracyPenalty         = 0x17D0; // float — cumulative inaccuracy
    constexpr std::ptrdiff_t m_flLastAccuracyUpdateTime = 0x17D4; // GameTime_t
    constexpr std::ptrdiff_t m_fAccuracySmoothedForZoom = 0x17D8; // float
    constexpr std::ptrdiff_t m_flRecoilIndex            = 0x17E0; // float — current recoil step
    constexpr std::ptrdiff_t m_flPostponeFireReadyFrac  = 0x17F0; // float — between-shot delay frac

    // CCSPlayer_MovementServices — bhop / autostrafe critical fields.
    // The pawn-side pointer to this services block is m_pMovementServices
    // at pawn+0x1220 (was 0x1418 pre-14154, now drifted -0x1F8). Writing
    // stamina to the wrong block silently nukes bhop without any obvious
    // symptom — the speed reset just doesn't take effect.
    constexpr std::ptrdiff_t m_pMovementServices_pawn       = 0x1220; // CCSPlayer_MovementServices*
    constexpr std::ptrdiff_t m_bDucked_movement             = 0x3E0;  // bool
    constexpr std::ptrdiff_t m_flDuckAmount_movement        = 0x3E4;  // float
    constexpr std::ptrdiff_t m_bDucking_movement            = 0x3EE;  // bool
    constexpr std::ptrdiff_t m_flStamina_movement           = 0x674;  // float — bhop kills speed if non-zero
    constexpr std::ptrdiff_t m_flStaminaAtJumpStart_movement = 0x684; // float
    constexpr std::ptrdiff_t m_flAccumulatedJumpError_mov   = 0x68C;  // float
    constexpr std::ptrdiff_t m_flLastJumpFrac_movement      = 0x6E4;  // float
    constexpr std::ptrdiff_t m_flLastJumpVelocityZ_movement = 0x6E8;  // float — for jumpshot apex detection
    // Pawn-direct (not via services pointer):
    constexpr std::ptrdiff_t m_flVelocityModifier_pawn      = 0x1C64; // float — server caps speed via this
    constexpr std::ptrdiff_t m_iShotsFired_pawn             = 0x1C5C; // int — increments per bullet
    constexpr std::ptrdiff_t m_flFallVelocity_pawn          = 0x25C;  // float

    // C_PlantedC4 — full timing block. m_flC4Blow / m_flDefuseCountDown
    // are GameTime_t (server-clock seconds since epoch); subtract local
    // GlobalVars::curtime to get a remaining-seconds countdown.
    constexpr std::ptrdiff_t m_bBombTicking        = 0x1160;
    constexpr std::ptrdiff_t m_flC4Blow            = 0x1190;  // GameTime_t — when bomb explodes
    constexpr std::ptrdiff_t m_bBeingDefused       = 0x119C;  // bool — defuse in progress
    constexpr std::ptrdiff_t m_flDefuseLength      = 0x11AC;  // float — total seconds for current defuser (5 nokit / 10 kit)
    constexpr std::ptrdiff_t m_flDefuseCountDown   = 0x11B0;  // GameTime_t — when defuse completes
    constexpr std::ptrdiff_t m_bBombDefused        = 0x11B4;  // bool — defused (post-completion)
    constexpr std::ptrdiff_t m_hBombDefuser        = 0x11B8;  // CHandle<C_CSPlayerPawn>

    // C_CSGameRules — round/match clocks. Used for round-time HUD and
    // for the freeze-aware silent-aim suppression we already do.
    // m_iRoundTime is the round duration in SECONDS. m_fRoundStartTime
    // is the GameTime_t when the live phase began (round_start). Time
    // remaining = (m_fRoundStartTime + m_iRoundTime) - curtime.
    constexpr std::ptrdiff_t m_iRoundTime          = 0x68;
    constexpr std::ptrdiff_t m_fMatchStartTime     = 0x6C;
    constexpr std::ptrdiff_t m_fRoundStartTime     = 0x70;
    constexpr std::ptrdiff_t m_gamePhase           = 0x84;  // 0=init,1=pregame,2=startgame,3=preround,4=teamwin,5=restart,6=stalemate,7=gameover
    constexpr std::ptrdiff_t m_totalRoundsPlayed   = 0x88;
    constexpr std::ptrdiff_t m_iRoundWinStatus     = 0x9AC;

    // C_Inferno — molotov / incendiary fire entity.
    // m_BurnNormal is up to 64 vec3 fire-spawn world positions; pair with
    // m_fireCount (typically right next to it) to know how many slots
    // are populated. Used by world_effects to render the danger area
    // so the player can see safe paths through fire.
    constexpr std::ptrdiff_t m_BurnNormal          = 0x1658;  // Vector[64]

    // C_CSPlayerPawn — server-tracked damage state. These are READ-ONLY
    // values the server uses to gate aim accuracy server-side; using them
    // as soft-throttle multipliers on silent-aim flick magnitude makes
    // our behaviour line up exactly with what the server already expects:
    //   * m_flFlinchStack       — 0..N stacks of flinch (decays per tick).
    //                             Bot-perfect aim while > 0 is one of the
    //                             cleanest detection signals.
    //   * m_flVelocityModifier  — 0.0..1.0 multiplier the server applies
    //                             to the local pawn's max speed for ~1s
    //                             after taking damage. < 1.0 means we
    //                             were just shot.
    //   * m_flTimeOfLastInjury  — GameTime_t of the last damage event.
    //                             curtime - this < 0.5s = still recovering.
    // IDA-VERIFIED 2026-04-26 (build 14154): sub_180BB0B10 @ 0x180BB1DE9
    // `mov dword [rax+10h], 1C60h` after lea "m_flFlinchStack" and
    // @ 0x180BB1E3A `mov dword [rax+10h], 1C64h` after "m_flVelocityModifier".
    constexpr std::ptrdiff_t m_flFlinchStack         = 0x1C60;
    constexpr std::ptrdiff_t m_flVelocityModifier    = 0x1C64;
    constexpr std::ptrdiff_t m_flTimeOfLastInjury    = 0x14E4;

    // CCSPlayer_AimPunchServices @ pawn + 0x1490. The server keeps the
    // aim-punch state here as a *predictable* base (recoil-pattern based,
    // applied on FireBullet) plus an *unpredictable* base (damage punch).
    // Reading m_predictableBaseAngle every tick lets us:
    //   1) Subtract it from our silent-aim shoot-angles → bullets fly
    //      to the *uncompensated* aim point even mid-spray.
    //   2) Render an inverse-recoil HUD crosshair so the player can
    //      see where the gun is *actually* pointing.
    constexpr std::ptrdiff_t m_predictableBaseTick           = 0x48;
    constexpr std::ptrdiff_t m_predictableBaseTickInterpAmount = 0x4C;
    constexpr std::ptrdiff_t m_predictableBaseAngle          = 0x50;  // QAngle (pitch,yaw,roll)
    constexpr std::ptrdiff_t m_predictableBaseAngleVel       = 0x5C;  // QAngle
    constexpr std::ptrdiff_t m_unpredictableBaseTick         = 0xA0;
    constexpr std::ptrdiff_t m_unpredictableBaseAngle        = 0xA4;  // QAngle (damage punch)
    // m_pAimPunchServices already defined earlier (0x1490).
}
