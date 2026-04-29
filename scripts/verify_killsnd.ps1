param([string]$Dll = "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\csgo\bin\win64\client.dll")
$bytes = [System.IO.File]::ReadAllBytes($Dll)
$peOff = [BitConverter]::ToInt32($bytes,0x3C)
$numSec = [BitConverter]::ToInt16($bytes,$peOff+6)
$optSize = [BitConverter]::ToInt16($bytes,$peOff+0x14)
$secStart = $peOff+0x18+$optSize
$tVA=0;$tRaw=0;$tSize=0
for($i=0;$i -lt $numSec;$i++){$s=$secStart+$i*0x28;$n=[Text.Encoding]::ASCII.GetString($bytes,$s,8).TrimEnd([char]0);if($n -eq ".text"){$tVA=[BitConverter]::ToUInt32($bytes,$s+0x0C);$tRaw=[BitConverter]::ToUInt32($bytes,$s+0x14);$tSize=[BitConverter]::ToUInt32($bytes,$s+0x08)}}
function Find-Pat([string]$hex,[int]$max=8){
  $tk=$hex -split ' '
  $vals=@();$mask=@()
  foreach($t in $tk){if($t -eq '?' -or $t -eq '??'){$vals+=0;$mask+=$true}else{$vals+=[Convert]::ToInt32($t,16);$mask+=$false}}
  $plen=$vals.Count;$end=$tRaw+$tSize-$plen;$hits=@()
  for($i=$tRaw;$i -lt $end;$i++){$ok=$true;for($j=0;$j -lt $plen;$j++){if(-not $mask[$j] -and $bytes[$i+$j] -ne $vals[$j]){$ok=$false;break}};if($ok){$hits+=[uint32]($tVA + ($i-$tRaw));if($hits.Count -ge $max){break}}}
  return ,$hits
}
$h = Find-Pat '48 89 5C 24 08 48 89 74 24 18 48 89 7C 24 20 55 41 56 41 57 48 8B EC 48 81 EC 80 00 00 00 44 8B'
Write-Host ("KillFeedbackEmitter sig hits: {0}" -f $h.Count)
foreach($m in $h){ Write-Host ('  0x{0:X}' -f $m) }
