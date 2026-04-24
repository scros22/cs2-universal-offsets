$base = "C:\Users\Samuel\AppData\Roaming\Code\User\workspaceStorage\81b23f477aca009ed47556f1667bd277\GitHub.copilot-chat\chat-session-resources\1fd17f33-f811-4376-a40b-1375b9d3f98b"
$files = @()
$files += Get-ChildItem $base -Recurse -Filter content.json -ErrorAction SilentlyContinue
$files += Get-ChildItem .ida-mcp -Filter *.json -ErrorAction SilentlyContinue
foreach ($f in $files) {
    $j = Get-Content $f.FullName -Raw | ConvertFrom-Json
    Write-Host ""
    Write-Host "=== $($j.addr) ($($j.asm.name)) ==="
    $lines = $j.asm.lines
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $ins = $lines[$i].instruction
        if ($ins -match 'lea\s+\w+,\s*a[A-Z]\w*;\s*"(m_\w+)"') {
            $name = $Matches[1]
            $back = @()
            for ($k = $i - 1; $k -ge [Math]::Max(0, $i - 30) -and $back.Count -lt 4; $k--) {
                $prev = $lines[$k].instruction
                if ($prev -match '^mov\s+(?:dword ptr\s+|qword ptr\s+|byte ptr\s+)?\[(?:rsp|rbp)[^,]+\],\s*([0-9A-Fa-f]+h?|0x[0-9A-Fa-f]+)\s*(?:;.*)?$') {
                    $back += $Matches[1]
                }
            }
            $fwd = @()
            for ($k = $i + 1; $k -le [Math]::Min($lines.Count - 1, $i + 12) -and $fwd.Count -lt 4; $k++) {
                $nxt = $lines[$k].instruction
                if ($nxt -match '^mov\s+(?:dword ptr\s+|qword ptr\s+|byte ptr\s+)?\[(?:rsp|rbp)[^,]+\],\s*([0-9A-Fa-f]+h?|0x[0-9A-Fa-f]+)\s*(?:;.*)?$') {
                    $fwd += $Matches[1]
                }
            }
            $offset = if ($back.Count -ge 2) { $back[1] } elseif ($fwd.Count -ge 1) { $fwd[0] } else { "?" }
            Write-Host ("  {0,-12} {1,-42} off={2,-10} back=[{3}] fwd=[{4}]" -f $lines[$i].addr, $name, $offset, ($back -join ','), ($fwd -join ','))
        }
    }
}
