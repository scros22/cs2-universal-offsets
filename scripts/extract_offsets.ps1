$ErrorActionPreference = 'Stop'
$src = 'C:\Users\Samuel\Projects\cs2\sdk\client_dll.hpp'
$content = Get-Content $src -Raw

$names = @(
    'm_pGameSceneNode','m_nSubclassID','m_hOwnerEntity','m_iHealth','m_lifeState',
    'm_iTeamNum','m_flSimulationTime','m_fFlags','m_pWeaponServices','m_vecViewOffset',
    'm_pObserverServices','m_pCameraServices','m_hMyWeapons','m_hActiveWeapon','m_hMyWearables',
    'm_hObserverTarget','m_iObserverMode','m_pInventoryServices','m_sSanitizedPlayerName',
    'm_hPlayerPawn','m_bPawnIsAlive','m_hObserverPawn','m_unMusicID',
    'm_bNeedToReApplyGloves','m_EconGloves','m_hHudModelArms','m_pClippingWeapon',
    'm_nEconGlovesChanged','m_iShotsFired','m_aimPunchAngle','m_angEyeAngles',
    'm_ArmorValue','m_bGunGameImmunity','m_iIDEntIndex','m_flFlashDuration',
    'm_flFlashMaxAlpha','m_flLastSmokeOverlayAlpha','m_bIsScoped','m_entitySpottedState',
    'm_pClientAlphaProperty','m_nAlpha','m_vecVelocity',
    'm_iFOV','m_iFOVStart','m_flFOVTime','m_flFOVRate','m_hZoomOwner','m_flLastShotFOV',
    'm_vecAbsOrigin','m_bRenderWithViewModels','m_flAutoExposureMin','m_flAutoExposureMax',
    'm_materialGroup','m_modelState','m_MeshGroupMask','m_BoneArray',
    'm_nRenderMode','m_clrRender','m_Glow','m_flGlowBackfaceMult','m_ClientOverrideTint','m_bUseClientOverrideTint',
    'm_bGlowing','m_glowColorOverride','m_iGlowType','m_nGlowRange','m_nGlowRangeMin','m_flGlowTime','m_flGlowStartTime',
    'm_AttributeManager','m_OriginalOwnerXuidLow','m_OriginalOwnerXuidHigh',
    'm_nFallbackPaintKit','m_nFallbackSeed','m_flFallbackWear','m_nFallbackStatTrak',
    'm_Item','m_iItemDefinitionIndex','m_iEntityQuality','m_iItemID','m_iItemIDHigh','m_iItemIDLow',
    'm_iAccountID','m_bInitialized','m_bDisallowSOC','m_bRestoreCustomMaterialAfterPrecache',
    'm_AttributeList','m_NetworkedDynamicAttributes','m_szCustomName','m_szCustomNameOverride',
    'm_Attributes','m_iAttributeDefinitionIndex','m_flValue','m_flInitialValue',
    'm_nRefundableCurrency','m_bSetBonus',
    'm_bBombTicking','m_flC4Blow','m_bBombDefused','m_flDefuseCountDown','m_flDefuseLength',
    'm_nSmokeEffectTickBegin','m_bDidSmokeEffect','m_vSmokeColor','m_bSmokeEffectSpawned',
    'm_fireCount','m_nFireEffectTickBegin',
    'm_vTintColor','m_vTintColorLightingOnly','m_flSkyBrightnessScale','m_bEnabled'
)

foreach ($n in $names) {
    $rx = [regex]"constexpr std::ptrdiff_t $n = (0x[0-9A-Fa-f]+);"
    $m = $rx.Match($content)
    if ($m.Success) { Write-Host "$n = $($m.Groups[1].Value)" }
    else { Write-Host "MISSING: $n" }
}
