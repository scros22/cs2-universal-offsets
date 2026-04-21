$src = 'C:\Users\Samuel\Projects\cs2\sdk\client_dll.hpp'
$content = Get-Content $src -Raw

# Pairs: ClassName, FieldName
$queries = @(
    @('CGlowProperty','m_iGlowType'),
    @('CGlowProperty','m_nGlowRange'),
    @('CGlowProperty','m_nGlowRangeMin'),
    @('CGlowProperty','m_glowColorOverride'),
    @('CGlowProperty','m_bGlowing'),
    @('CGlowProperty','m_flGlowTime'),
    @('CGlowProperty','m_flGlowStartTime'),
    @('CSkeletonInstance','m_modelState'),
    @('CSkeletonInstance','m_materialGroup'),
    @('CModelState','m_MeshGroupMask'),
    @('CModelState','m_BoneArray'),
    @('CGameSceneNode','m_vecAbsOrigin'),
    @('CCSPlayerBase_CameraServices','m_iFOV'),
    @('CPlayer_CameraServices','m_iFOV'),
    @('C_BaseModelEntity','m_pClientAlphaProperty'),
    @('CClientAlphaProperty','m_nAlpha'),
    @('C_PlayerPawnComponent','m_pPawn'),
    @('C_BaseEntity','m_iTeamNum'),
    @('C_CSWeaponBase','m_iClip1'),
    @('C_BasePlayerWeapon','m_iClip1'),
    @('C_EnvSky','m_bEnabled'),
    @('C_EnvSky','m_vTintColor'),
    @('C_EnvSky','m_vTintColorLightingOnly')
)

foreach ($q in $queries) {
    $cls = $q[0]; $fld = $q[1]
    $rx = [regex]"namespace $cls \{[^}]*?constexpr std::ptrdiff_t $fld = (0x[0-9A-Fa-f]+);"
    $m = $rx.Match($content)
    if ($m.Success) { Write-Host "${cls}::${fld} = $($m.Groups[1].Value)" }
    else { Write-Host "MISSING: ${cls}::${fld}" }
}
