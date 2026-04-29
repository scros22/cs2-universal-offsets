param(
  [string]$Dll = "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\csgo\bin\win64\client.dll"
)

$bytes = [System.IO.File]::ReadAllBytes($Dll)
$peOff = [BitConverter]::ToInt32($bytes, 0x3C)
$numSections = [BitConverter]::ToInt16($bytes, $peOff + 6)
$optHdrSize  = [BitConverter]::ToInt16($bytes, $peOff + 0x14)
$sectionsStart = $peOff + 0x18 + $optHdrSize

$textVA = 0; $textRaw = 0; $textSize = 0
for ($i = 0; $i -lt $numSections; $i++) {
    $s = $sectionsStart + $i * 0x28
    $name     = [System.Text.Encoding]::ASCII.GetString($bytes, $s, 8).TrimEnd([char]0)
    $virtAddr = [BitConverter]::ToUInt32($bytes, $s + 0x0C)
    $virtSize = [BitConverter]::ToUInt32($bytes, $s + 0x08)
    $rawPtr   = [BitConverter]::ToUInt32($bytes, $s + 0x14)
    if ($name -eq '.text') { $textVA = $virtAddr; $textRaw = $rawPtr; $textSize = $virtSize }
}
Write-Host (".text VA=0x{0:X} RAW=0x{1:X} Size=0x{2:X}" -f $textVA, $textRaw, $textSize)

function Read-AtRva([uint32]$rva, [int]$len) {
    $off = $textRaw + ($rva - $textVA)
    return $bytes[$off..($off + $len - 1)]
}

$rva = 0xAC6EC0
$slice = Read-AtRva $rva 16
Write-Host ("@ 0x{0:X}: {1}" -f $rva, (($slice | ForEach-Object { '{0:X2}' -f $_ }) -join ' '))

# Show bytes at handler RVAs we cached
foreach ($r in @(0xAC8C30, 0xAC8B50)) {
    $sl = Read-AtRva $r 24
    Write-Host ("HANDLER @ 0x{0:X}: {1}" -f $r, (($sl | ForEach-Object { '{0:X2}' -f $_ }) -join ' '))
}

# Wildcard pattern matcher (?? = wildcard)
function Find-Pattern([string]$hex, [int]$max=8) {
    $tokens = $hex -split ' '
    $vals = New-Object 'System.Collections.Generic.List[int]'
    $mask = New-Object 'System.Collections.Generic.List[bool]'
    foreach ($t in $tokens) {
        if ($t -eq '??' -or $t -eq '?') { $vals.Add(0); $mask.Add($true) }
        else { $vals.Add([Convert]::ToInt32($t, 16)); $mask.Add($false) }
    }
    $plen = $vals.Count
    $endIdx = $textRaw + $textSize - $plen
    $hits = New-Object 'System.Collections.Generic.List[uint32]'
    for ($i = $textRaw; $i -lt $endIdx; $i++) {
        $ok = $true
        for ($j = 0; $j -lt $plen; $j++) {
            if (-not $mask[$j] -and $bytes[$i + $j] -ne $vals[$j]) { $ok = $false; break }
        }
        if ($ok) {
            $hits.Add([uint32]($textVA + ($i - $textRaw)))
            if ($hits.Count -ge $max) { break }
        }
    }
    return $hits
}

# Patch site — both register variants
Write-Host '--- Patch site sig (current build, r15b) ---'
$h = Find-Pattern '48 8B 40 08 44 38 38 75 10 44 88 7F 01'
foreach ($m in $h) { Write-Host ('  0x{0:X}' -f $m) }

Write-Host '--- Patch site sig (wildcarded, register-agnostic) ---'
$h = Find-Pattern '48 8B 40 08 44 38 ?? 75 10 44 88 ?? 01'
foreach ($m in $h) { Write-Host ('  0x{0:X}' -f $m) }

Write-Host '--- ThirdPersonOn handler sig ---'
$h = Find-Pattern '48 83 EC 38 48 8B 0D ?? ?? ?? ?? 48 8D 54 24 ?? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ?? 00 0F 85 ?? ?? ?? ?? 4C 8B 05 ?? ?? ?? ?? 41 8B 80 50 0B 00 00'
foreach ($m in $h) { Write-Host ('  0x{0:X}' -f $m) }

Write-Host '--- ThirdPersonOff handler sig ---'
$h = Find-Pattern '48 83 EC 28 48 8B 0D ?? ?? ?? ?? 48 8D 54 24 ?? 48 8B 01 FF 90 08 03 00 00 83 7C 24 ?? 00 75 ?? 48 8B 05 ?? ?? ?? ?? C6 80 29 02 00 00 00 C7 80 A8 06 00 00 00'
foreach ($m in $h) { Write-Host ('  0x{0:X}' -f $m) }

# Extract CInputPtr_RVA from inside the ON handler.
# Handler layout (offset from start):
#   0x24: 4C 8B 05 disp32     ; mov r8, [rip+disp32]  <- CInput global slot
#   next-rip = handler + 0x2B
$onHandler = 0xAC8AF0
$instOff = 0x24
$slice = Read-AtRva ($onHandler + $instOff) 7
Write-Host ('mov r8 instr bytes: {0}' -f (($slice | ForEach-Object { '{0:X2}' -f $_ }) -join ' '))
if ($slice[0] -eq 0x4C -and $slice[1] -eq 0x8B -and $slice[2] -eq 0x05) {
    $disp = [BitConverter]::ToInt32([byte[]]$slice[3..6], 0)
    $cInputRva = [uint32]($onHandler + $instOff + 7 + $disp)
    Write-Host ('LIVE CInputPtr RVA = 0x{0:X}' -f $cInputRva)
} else {
    Write-Host 'CInput mov instruction not at expected offset'
}
