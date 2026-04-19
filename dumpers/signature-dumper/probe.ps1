$out = "C:\Users\Samuel\Projects\cs2\dumpers\signature-dumper\probes.txt"
if (Test-Path $out) { Remove-Item $out }
function Probe($path, $needles) {
  $bytes = [IO.File]::ReadAllBytes($path)
  $name = [IO.Path]::GetFileName($path)
  foreach ($p in $needles) {
    $n = [Text.Encoding]::ASCII.GetBytes($p + "`0")
    $hit = $false
    for ($i = 0; $i -le $bytes.Length - $n.Length; $i++) {
      if ($bytes[$i] -ne $n[0]) { continue }
      $m = $true
      for ($j = 1; $j -lt $n.Length; $j++) { if ($bytes[$i+$j] -ne $n[$j]) { $m = $false; break } }
      if ($m) { $hit = $true; break }
    }
    Add-Content $out "$name $(if($hit){'HIT '}else{'MISS'}) $p"
  }
}
$eng = "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\bin\win64\engine2.dll"
$scn = "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\bin\win64\scenesystem.dll"
$tier = "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\bin\win64\tier0.dll"
$cl  = "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\csgo\bin\win64\client.dll"
Probe $eng @('Engine_GetTime','Host_AccumulateTime','CL_FullyConnected','CNetChan::ProcessMessages','SignOnState','Source2Main','CL_Move','CGameClient','sv_cheats','net_channels','NetworkGameClient','GetLocalClient','CHostStateMgr','HostStateRequest','CClientState','LevelInitPreEntity','LevelShutdown','CL_FrameStageNotify','CL_FullyConnect','Host_NewGame','CSource2Host')
Probe $scn @('CSceneView','CSceneSystem','SkyboxRenderingSystem','SceneSystem','RenderingSystem','SceneObjectDesc','renderable','CSceneObject','CRenderGameSystem','CSceneAnimatableObject','GeneratePrimitives','SkyComposite','RenderSkybox','DrawSky')
Probe $tier @('LoadKV3','KeyValues3','CKeyValues3','Plat_FloatTime','KV3_LoadFromFile','LoadKeyValues3','BinaryToText','LoadKV3FromFile','LoadKeyValues','KV3FromBuffer')
Probe $cl @('CCSGameRules','CHudWeaponSelection','CHudDeathNotice','VAC-Net Detection','vacnet','AcceptInviteToParty','SteamMatchmaking','OnVACNetEvent','C_CSGameRules','HudWeaponSelection','HudDeathNotice','VACBan','C_CSPlayerPawn','C_CSPlayerController','C_CSGameRulesProxy')
"DONE"
