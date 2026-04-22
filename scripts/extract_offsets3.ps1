$src = 'C:\Users\Samuel\Projects\cs2\sdk\client_dll.hpp'
$content = Get-Content $src -Raw

$queries = @(
    @('C_CSPlayerPawn','m_pClippingWeapon'),
    @('C_CSPlayerPawn','m_aimPunchAngle'),
    @('C_CSPlayerPawn','m_iShotsFired'),
    @('C_CSPlayerPawn','m_ArmorValue'),
    @('C_CSPlayerPawn','m_bIsScoped'),
    @('C_CSPlayerPawn','m_iIDEntIndex'),
    @('C_CSPlayerPawn','m_angEyeAngles'),
    @('C_CSPlayerPawn','m_bGunGameImmunity'),
    @('C_CSPlayerPawn','m_flFlashDuration'),
    @('C_CSPlayerPawn','m_flFlashMaxAlpha'),
    @('C_CSPlayerPawn','m_flLastSmokeOverlayAlpha'),
    @('C_CSPlayerPawn','m_entitySpottedState'),
    @('C_CSPlayerPawn','m_bNeedToReApplyGloves'),
    @('C_CSPlayerPawn','m_EconGloves'),
    @('C_CSPlayerPawn','m_hHudModelArms'),
    @('C_CSPlayerPawn','m_nEconGlovesChanged'),
    @('CSmokeGrenadeProjectile','m_nSmokeEffectTickBegin'),
    @('CSmokeGrenadeProjectile','m_bDidSmokeEffect'),
    @('CSmokeGrenadeProjectile','m_vSmokeColor'),
    @('CSmokeGrenadeProjectile','m_bSmokeEffectSpawned'),
    @('C_Inferno','m_fireCount'),
    @('C_Inferno','m_nFireEffectTickBegin'),
    @('C_BaseEntity','m_vecVelocity'),
    @('C_BaseEntity','m_bRenderWithViewModels'),
    @('C_TonemapController2','m_flAutoExposureMin'),
    @('C_TonemapController2','m_flAutoExposureMax'),
    @('C_BaseModelEntity','m_nRenderMode'),
    @('C_BaseModelEntity','m_clrRender'),
    @('C_BaseModelEntity','m_Glow'),
    @('C_BaseModelEntity','m_flGlowBackfaceMult'),
    @('C_BaseModelEntity','m_ClientOverrideTint'),
    @('C_BaseModelEntity','m_bUseClientOverrideTint'),
    @('C_EconEntity','m_AttributeManager'),
    @('C_EconEntity','m_OriginalOwnerXuidLow'),
    @('C_EconEntity','m_OriginalOwnerXuidHigh'),
    @('C_EconEntity','m_nFallbackPaintKit'),
    @('C_EconEntity','m_nFallbackSeed'),
    @('C_EconEntity','m_flFallbackWear'),
    @('C_EconEntity','m_nFallbackStatTrak'),
    @('C_PlantedC4','m_bBombTicking'),
    @('C_PlantedC4','m_flC4Blow'),
    @('C_PlantedC4','m_bBombDefused'),
    @('C_PlantedC4','m_flDefuseCountDown'),
    @('C_PlantedC4','m_flDefuseLength'),
    @('C_BaseCombatCharacter','m_hMyWearables'),
    @('CCSPlayerController','m_pInventoryServices'),
    @('CCSPlayerController','m_sSanitizedPlayerName'),
    @('CCSPlayerController','m_hPlayerPawn'),
    @('CCSPlayerController','m_bPawnIsAlive'),
    @('CCSPlayerController','m_hObserverPawn'),
    @('C_BasePlayerPawn','m_pWeaponServices'),
    @('C_BasePlayerPawn','m_vecViewOffset'),
    @('C_BasePlayerPawn','m_pObserverServices'),
    @('C_BasePlayerPawn','m_pCameraServices'),
    @('C_BaseEntity','m_pGameSceneNode'),
    @('C_BaseEntity','m_nSubclassID'),
    @('C_BaseEntity','m_hOwnerEntity'),
    @('C_BaseEntity','m_iHealth'),
    @('C_BaseEntity','m_lifeState'),
    @('C_BaseEntity','m_flSimulationTime'),
    @('C_BaseEntity','m_fFlags')
)

foreach ($q in $queries) {
    $cls = $q[0]; $fld = $q[1]
    $rx = [regex]"namespace $cls \{[^}]*?constexpr std::ptrdiff_t $fld = (0x[0-9A-Fa-f]+);"
    $m = $rx.Match($content)
    if ($m.Success) { Write-Host "${cls}::${fld} = $($m.Groups[1].Value)" }
    else { Write-Host "MISSING: ${cls}::${fld}" }
}
