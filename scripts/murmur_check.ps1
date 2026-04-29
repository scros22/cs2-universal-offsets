$bannedNames = @(
  "Player.DamageHeadShot.AttackerFeedback",
  "Player.DamageHeadShotArmor.AttackerFeedback",
  "Player.DamageBody.AttackerFeedback",
  "Player.DamageBodyArmor.AttackerFeedback",
  "Player.DeathHeadShot.AttackerFeedback",
  "Player.DeathHeadShotArmor.AttackerFeedback",
  "Player.DeathBody.AttackerFeedback",
  "Player.DeathBodyArmor.AttackerFeedback"
)
function Murmur2 {
    param([string]$s, [uint32]$seed = 0x31415926)
    $bytes = [Text.Encoding]::ASCII.GetBytes($s)
    $len = $bytes.Length
    $m = 0x5BD1E995
    $r = 24
    $h = ($seed -bxor [uint32]$len) -band 0xFFFFFFFF
    $i = 0
    while ($len - $i -ge 4) {
        $k = [BitConverter]::ToUInt32($bytes, $i)
        $k = ($k * $m) -band 0xFFFFFFFF
        $k = $k -bxor ($k -shr $r)
        $k = ($k * $m) -band 0xFFFFFFFF
        $h = ($h * $m) -band 0xFFFFFFFF
        $h = $h -bxor $k
        $i += 4
    }
    $rem = $len - $i
    if ($rem -ge 3) { $h = $h -bxor ([uint32]$bytes[$i+2] -shl 16) }
    if ($rem -ge 2) { $h = $h -bxor ([uint32]$bytes[$i+1] -shl 8) }
    if ($rem -ge 1) {
        $h = $h -bxor [uint32]$bytes[$i]
        $h = ($h * $m) -band 0xFFFFFFFF
    }
    $h = $h -bxor ($h -shr 13)
    $h = ($h * $m) -band 0xFFFFFFFF
    $h = $h -bxor ($h -shr 15)
    return $h
}

Write-Host "=== Computed hashes (seed=0x31415926) ==="
foreach ($n in $bannedNames) {
    "{0,-50} {1:X8}" -f $n, (Murmur2 $n)
}

Write-Host ""
Write-Host "=== Hashes from latest log session ==="
$log = "$env:TEMP\lucid_sounds.log"
if (Test-Path $log) {
    $content = Get-Content $log
    $starts = @()
    for ($i = 0; $i -lt $content.Count; $i++) {
        if ($content[$i] -like '===*') { $starts += $i }
    }
    if ($starts.Count -gt 0) {
        $content[$starts[-1]..($content.Count - 1)] |
            Where-Object { $_ -like '<hash:*' } |
            Sort-Object -Unique
    }
}
