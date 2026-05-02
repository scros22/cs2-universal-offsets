# Compare hardcoded sdk_offsets.h values vs the latest universal-dumper dump.
# Usage: pwsh scripts\diff_offsets.ps1
$dump = "dumpers\universal-dumper\dumps\latest\offsets\client_dll.json"
$j = Get-Content $dump -Raw | ConvertFrom-Json
$cls = $j."client.dll".classes

function Get-Off($className, $fieldName) {
    $c = $cls.$className
    if (-not $c) { return $null }
    $f = $c.fields.$fieldName
    if ($null -eq $f) { return $null }
    return [int64]$f
}

# (className, fieldName, currentHardcoded)
$pairs = @(
    @("C_BaseEntity",                    "m_pGameSceneNode",        0x330),
    @("C_BaseEntity",                    "m_iHealth",               0x34C),
    @("C_BaseEntity",                    "m_lifeState",             0x354),
    @("C_BaseEntity",                    "m_iTeamNum",              0x3EB),
    @("C_BaseEntity",                    "m_flSimulationTime",      0x3B8),
    @("C_BaseEntity",                    "m_fFlags",                0x3F8),
    @("C_BaseEntity",                    "m_hOwnerEntity",          0x520),
    @("C_BaseEntity",                    "m_MoveType",              0x525),

    @("C_BasePlayerPawn",                "m_pWeaponServices",       0x11E0),
    @("C_BasePlayerPawn",                "m_pObserverServices",     0x11F8),
    @("C_BasePlayerPawn",                "m_pCameraServices",       0x1218),
    @("C_BasePlayerPawn",                "m_pMovementServices",     0x1220),
    @("C_BasePlayerPawn",                "m_vecViewOffset",         0xE70),
    @("C_BasePlayerPawn",                "m_iHideHUD",              $null),
    @("C_BasePlayerPawn",                "v_angle",                 0x1298),

    @("CPlayer_WeaponServices",          "m_hMyWeapons",            0x48),
    @("CPlayer_WeaponServices",          "m_hActiveWeapon",         0x60),

    @("CCSPlayerController",             "m_pInventoryServices",    0x808),
    @("CCSPlayerController",             "m_sSanitizedPlayerName",  0x858),
    @("CCSPlayerController",             "m_hPlayerPawn",           0x904),
    @("CCSPlayerController",             "m_bPawnIsAlive",          0x90C),
    @("CCSPlayerController",             "m_hObserverPawn",         0x908),
    @("CCSPlayerController",             "m_iDesiredFOV",           0x784),

    @("C_CSPlayerPawn",                  "m_angEyeAngles",          0x3300),
    @("C_CSPlayerPawn",                  "m_ArmorValue",            0x1C74),
    @("C_CSPlayerPawn",                  "m_bGunGameImmunity",      0x3278),
    @("C_CSPlayerPawn",                  "m_iIDEntIndex",           0x33DC),
    @("C_CSPlayerPawn",                  "m_flFlashDuration",       0x1400),
    @("C_CSPlayerPawn",                  "m_flFlashMaxAlpha",       0x13FC),
    @("C_CSPlayerPawn",                  "m_flLastSmokeOverlayAlpha", 0x1420),
    @("C_CSPlayerPawn",                  "m_bIsScoped",             0x1C48),
    @("C_CSPlayerPawn",                  "m_entitySpottedState",    0x1C30),
    @("C_CSPlayerPawn",                  "m_bWaitForNoAttack",      0x1C68),
    @("C_CSPlayerPawn",                  "m_bIsDefusing",           0x1C4A),
    @("C_CSPlayerPawn",                  "m_bIsGrabbingHostage",    0x1C4B),
    @("C_CSPlayerPawn",                  "m_iShotsFired",           0x1C5C),
    @("C_CSPlayerPawn",                  "m_flVelocityModifier",    0x1C64),
    @("C_CSPlayerPawn",                  "m_flFlinchStack",         0x1C60),
    @("C_CSPlayerPawn",                  "m_flTimeOfLastInjury",    0x14E4),
    @("C_CSPlayerPawn",                  "m_pAimPunchServices",     0x1490),
    @("C_CSPlayerPawn",                  "m_aimPunchAngle",         $null),
    @("C_CSPlayerPawn",                  "m_vecVelocity",           0x430),
    @("C_CSPlayerPawn",                  "m_flFallVelocity",        0x25C),
    @("C_CSPlayerPawn",                  "m_iHideHUD",              $null),
    @("C_CSPlayerPawn",                  "m_bHud_MiniScoreHidden",  $null),
    @("C_CSPlayerPawn",                  "m_bIsHoldingLookAtWeapon",$null),
    @("C_CSPlayerPawn",                  "m_bWaitForNoAttack",      0x1C68),

    @("C_CSPlayerPawnBase",              "m_iShotsFired",           0x1C5C),

    @("C_EconEntity",                    "m_AttributeManager",      0x1180),
    @("C_EconEntity",                    "m_OriginalOwnerXuidLow",  0x1650),
    @("C_EconEntity",                    "m_OriginalOwnerXuidHigh", 0x1654),
    @("C_EconEntity",                    "m_nFallbackPaintKit",     0x1658),
    @("C_EconEntity",                    "m_nFallbackSeed",         0x165C),
    @("C_EconEntity",                    "m_flFallbackWear",        0x1660),
    @("C_EconEntity",                    "m_nFallbackStatTrak",     0x1664),

    @("CCSPlayer_ItemServices",          "m_bHasDefuser",           $null),
    @("CCSPlayer_ItemServices",          "m_bHasHelmet",            $null),

    @("CCSPlayer_MovementServices",      "m_bDucked",               0x3E0),
    @("CCSPlayer_MovementServices",      "m_flDuckAmount",          0x3E4),
    @("CCSPlayer_MovementServices",      "m_bDucking",              0x3EE),
    @("CCSPlayer_MovementServices",      "m_flStamina",             0x674),
    @("CCSPlayer_MovementServices",      "m_flStaminaAtJumpStart",  0x684),
    @("CCSPlayer_MovementServices",      "m_flAccumulatedJumpError",0x68C),
    @("CCSPlayer_MovementServices",      "m_flLastJumpFrac",        0x6E4),
    @("CCSPlayer_MovementServices",      "m_flLastJumpVelocityZ",   0x6E8),

    @("C_CSWeaponBase",                  "m_flTurningInaccuracyDelta", 0x17BC),
    @("C_CSWeaponBase",                  "m_flTurningInaccuracy",   0x17CC),
    @("C_CSWeaponBase",                  "m_fAccuracyPenalty",      0x17D0),
    @("C_CSWeaponBase",                  "m_flLastAccuracyUpdateTime",0x17D4),
    @("C_CSWeaponBase",                  "m_fAccuracySmoothedForZoom",0x17D8),
    @("C_CSWeaponBase",                  "m_flRecoilIndex",         0x17E0),
    @("C_CSWeaponBase",                  "m_flPostponeFireReadyFrac",0x17F0),
    @("C_CSWeaponBase",                  "m_iRecoilIndex",          0x17DC),
    @("C_CSWeaponBase",                  "m_zoomLevel",             0x1CB0),
    @("C_CSWeaponBase",                  "m_bNeedsBoltAction",      0x1CCD),
    @("C_CSWeaponBase",                  "m_iClip1",                0x16D8),
    @("C_CSWeaponBase",                  "m_bInReload",             0x17F4),
    @("C_CSWeaponBase",                  "m_nNextPrimaryAttackTick",0x16C8),
    @("C_CSWeaponBase",                  "m_flNextPrimaryAttackTickRatio", 0x16CC),
    @("C_CSWeaponBase",                  "m_iItemDefinitionIndex",  $null),

    @("CGameSceneNode",                  "m_vecAbsOrigin",          0xC8),
    @("CGameSceneNode",                  "m_modelState",            0x150),
    @("CSkeletonInstance",               "m_MeshGroupMask",         0x1C8),
    @("CSkeletonInstance",               "m_materialGroup",         0x3C4),
    @("C_BaseModelEntity",               "m_pClientAlphaProperty",  0xF50),
    @("C_BaseModelEntity",               "m_nRenderMode",           0xC78),
    @("C_BaseModelEntity",               "m_clrRender",             0xC98),
    @("C_BaseModelEntity",               "m_Glow",                  0xDD8),
    @("C_BaseModelEntity",               "m_flGlowBackfaceMult",    0xE30),
    @("C_BaseModelEntity",               "m_ClientOverrideTint",    0xF58),
    @("C_BaseModelEntity",               "m_bUseClientOverrideTint",0xF5C),

    @("C_EnvSky",                        "m_vTintColor",            0xFB9),
    @("C_EnvSky",                        "m_vTintColorLightingOnly",0xFBD),
    @("C_EnvSky",                        "m_flBrightnessScale",     0xFC4),
    @("C_EnvSky",                        "m_bEnabled",              0xFDC),

    @("C_PlantedC4",                     "m_bBombTicking",          0x1160),
    @("C_PlantedC4",                     "m_flC4Blow",              0x1190),
    @("C_PlantedC4",                     "m_bBeingDefused",         0x119C),
    @("C_PlantedC4",                     "m_flDefuseLength",        0x11AC),
    @("C_PlantedC4",                     "m_flDefuseCountDown",     0x11B0),
    @("C_PlantedC4",                     "m_bBombDefused",          0x11B4),
    @("C_PlantedC4",                     "m_hBombDefuser",          0x11B8),

    @("C_SmokeGrenadeProjectile",        "m_nSmokeEffectTickBegin", 0x1250),
    @("C_SmokeGrenadeProjectile",        "m_bDidSmokeEffect",       0x1254),
    @("C_SmokeGrenadeProjectile",        "m_VecSmokeColor",         0x125C),
    @("C_SmokeGrenadeProjectile",        "m_vSmokeColor",           0x125C),
    @("C_SmokeGrenadeProjectile",        "m_bSmokeEffectSpawned",   0x1299),

    @("CGlowProperty",                   "m_bGlowing",              0x51),
    @("CGlowProperty",                   "m_glowColorOverride",     0x40),
    @("CGlowProperty",                   "m_iGlowType",             0x30),
    @("CGlowProperty",                   "m_nGlowRange",            0x38),
    @("CGlowProperty",                   "m_nGlowRangeMin",         0x3C),
    @("CGlowProperty",                   "m_flGlowTime",            0x48),
    @("CGlowProperty",                   "m_flGlowStartTime",       0x4C),

    @("C_AttributeContainer",            "m_Item",                  0x50),
    @("CEconItemView",                   "m_iItemDefinitionIndex",  0x1BA),
    @("CEconItemView",                   "m_iEntityQuality",        0x1BC),
    @("CEconItemView",                   "m_iItemID",               0x1C8),
    @("CEconItemView",                   "m_iItemIDHigh",           0x1D0),
    @("CEconItemView",                   "m_iItemIDLow",            0x1D4),
    @("CEconItemView",                   "m_iAccountID",            0x1D8),
    @("CEconItemView",                   "m_bInitialized",          0x1E8),
    @("CEconItemView",                   "m_bDisallowSOC",          0x1E9),
    @("CEconItemView",                   "m_bRestoreCustomMaterialAfterPrecache", 0x1B8),
    @("CEconItemView",                   "m_AttributeList",         0x208),
    @("CEconItemView",                   "m_NetworkedDynamicAttributes", 0x280),
    @("CEconItemView",                   "m_szCustomName",          0x2F8),
    @("CEconItemView",                   "m_szCustomNameOverride",  0x399),

    @("CAttributeList",                  "m_Attributes",            0x8),
    @("CEconItemAttribute",              "m_iAttributeDefinitionIndex", 0x30),
    @("CEconItemAttribute",              "m_flValue",               0x34),
    @("CEconItemAttribute",              "m_flInitialValue",        0x38),
    @("CEconItemAttribute",              "m_nRefundableCurrency",   0x3C),
    @("CEconItemAttribute",              "m_bSetBonus",             0x40),

    @("CCSPlayerBase_CameraServices",    "m_iFOV",                  0x290),
    @("CCSPlayerBase_CameraServices",    "m_iFOVStart",             0x294),
    @("CCSPlayerBase_CameraServices",    "m_flFOVTime",             0x298),
    @("CCSPlayerBase_CameraServices",    "m_flFOVRate",             0x29C),
    @("CCSPlayerBase_CameraServices",    "m_hZoomOwner",            0x2A0),
    @("CCSPlayerBase_CameraServices",    "m_flLastShotFOV",         0x2A4),

    @("CCSGameRulesProxy",               "m_pGameRules",            $null),
    @("CCSGameRules",                    "m_iRoundTime",            0x68),
    @("CCSGameRules",                    "m_fMatchStartTime",       0x6C),
    @("CCSGameRules",                    "m_fRoundStartTime",       0x70),
    @("CCSGameRules",                    "m_gamePhase",             0x84),
    @("CCSGameRules",                    "m_totalRoundsPlayed",     0x88),
    @("CCSGameRules",                    "m_iRoundWinStatus",       0x9AC),
    @("CCSGameRules",                    "m_bFreezePeriod",         0x40),
    @("CCSGameRules",                    "m_bWarmupPeriod",         0x41),

    @("C_CSObserverPawn",                "m_hObserverTarget",       0x4C),
    @("C_CSObserverPawn",                "m_iObserverMode",         0x48),

    @("CPredictableContextManager_PredictableBaseAngles_t", "m_predictableBaseTick", 0x48),
    @("CPredictableContextManager_PredictableBaseAngles_t", "m_predictableBaseAngle", 0x50),
    @("CPredictableContextManager_PredictableBaseAngles_t", "m_unpredictableBaseAngle", 0xA4)
)

$changed = @()
$same = 0
$missing = @()

foreach ($p in $pairs) {
    $cn = $p[0]; $fn = $p[1]; $cur = $p[2]
    $newVal = Get-Off $cn $fn
    if ($null -eq $newVal) {
        $missing += "$cn::$fn (not in dump)"
        continue
    }
    if ($null -eq $cur) {
        Write-Host ("[NEW]   {0}::{1} = 0x{2:X}" -f $cn, $fn, $newVal) -ForegroundColor Yellow
        continue
    }
    if ($newVal -ne $cur) {
        $delta = $newVal - $cur
        $sign = if ($delta -ge 0) { "+" } else { "" }
        Write-Host ("[DIFF]  {0}::{1}  0x{2:X}  ->  0x{3:X}  ({4}0x{5:X})" -f $cn, $fn, $cur, $newVal, $sign, [Math]::Abs($delta)) -ForegroundColor Red
        $changed += $p
    } else {
        $same++
    }
}

Write-Host ""
Write-Host "===================="
Write-Host "Same:    $same"
Write-Host "Changed: $($changed.Count)"
Write-Host "Missing in dump: $($missing.Count)"
if ($missing.Count -gt 0 -and $missing.Count -le 10) { $missing | ForEach-Object { Write-Host "  $_" -ForegroundColor DarkGray } }
