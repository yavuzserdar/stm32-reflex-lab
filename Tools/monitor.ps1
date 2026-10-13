param([string]$Port, [switch]$List)
$ErrorActionPreference = 'Stop'
$ports = @([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object)
if ($List) { $ports; return }
if (!$Port) {
    if ($ports.Count -eq 1) { $Port = $ports[0] }
    elseif ($ports.Count -eq 0) { throw 'No serial port found. Connect the ST-LINK USB data cable.' }
    else { throw "Multiple ports found: $($ports -join ', '). Run Tools/monitor.ps1 -Port COMx with the ST-LINK port." }
}
$resultsDir = Join-Path (Split-Path $PSScriptRoot -Parent) 'results'
New-Item -ItemType Directory -Force -Path $resultsDir | Out-Null
$csv = Join-Path $resultsDir ('session-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.csv')
'session,round,reaction_us,best_us,average_us,early,timeouts' | Set-Content -LiteralPath $csv -Encoding ASCII
$serial = New-Object System.IO.Ports.SerialPort $Port,115200,None,8,one
$serial.ReadTimeout = 250
$serial.DtrEnable = $false
$serial.RtsEnable = $false
try {
    $serial.Open()
    Write-Host "Connected: $Port. Press black RESET once, then blue B1 to start. Ctrl+C to stop."
    Write-Host "CSV: $csv"
    while ($true) {
        try { $line = $serial.ReadLine().Trim() }
        catch [System.TimeoutException] { continue }
        if ($line -match '^RESULT,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)$') {
            $line.Substring(7) | Add-Content -LiteralPath $csv -Encoding ASCII
            $reaction = [int]$Matches[3] / 1000.0
            $best = [int]$Matches[4] / 1000.0
            $average = [int]$Matches[5] / 1000.0
            Write-Host ('Round {0}/10 | {1:F1} ms | Best {2:F1} ms | Average {3:F1} ms' -f $Matches[2],$reaction,$best,$average)
        } else { Write-Host $line }
    }
} finally {
    $serial.Dispose()
    Write-Host "Saved: $csv"
}
