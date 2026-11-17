# The heavy run, as one command:  powershell -File browser\tools\heavy-run.ps1 [-Exe PATH] [-Log PATH]
# Runs 'browser.exe --heavy --no-audio' (a window, the synthetic clock, a fixed walk, load / churn / steady phases; heavy.cone),
# keeps the whole stdout in -Log, echoes the phase marks to the console and prints the one JSON line of its measures last.
# Exit code: the browser's, or 1 when no JSON came out. Never passes --xr. Frame medians of two runs are meant to agree to 5%.
param(
  [string]$Exe = (Join-Path $PSScriptRoot "..\build\release\browser.exe"),
  [string]$Log = (Join-Path $env:TEMP "heavy-run.log")
)
$out = & $Exe --heavy --no-audio 2>&1
$code = $LASTEXITCODE
$out | Out-File -Encoding utf8 $Log
$out | Where-Object { $_ -match '^HEAVY (mark|scene)' } | ForEach-Object { Write-Host $_ }
$json = $out | Where-Object { $_ -match '^HEAVY_JSON ' } | Select-Object -Last 1
if (-not $json) { Write-Host "heavy-run: no HEAVY_JSON line; see $Log"; exit 1 }
($json -replace '^HEAVY_JSON ', '')
exit $code
