# What re-reading the prefix costs: serve a split world with tools\worldserve.py, run pegasus headless against it, and print per
# run: the reads the Reader did ("reader: prefix read of N bytes took U us" and "whole read"), their sums, and the arrival marks.
#   pwsh tools\prefix-cost.ps1 -World C:\src\zz-w146\w200k-15\w200k-15.world [-Mbit 0] [-LatencyMs 0] [-Runs 5] [-Steps 600]
# Prints one line per run, then the min, median and max of each column. Stops the server when done.
param(
  [Parameter(Mandatory)][string]$World,
  [double]$Mbit = 0,
  [double]$LatencyMs = 0,
  [int]$Runs = 5,
  [int]$Steps = 600
)
$here = Split-Path -Parent $PSCommandPath
$exe = Join-Path $here '..\build\release\pegasus.exe'
$worldPath = (Resolve-Path $World).Path
$root = Split-Path -Parent $worldPath
$name = Split-Path -Leaf $worldPath
$env:PATH += ';C:\libs\SDL3-3.4.16\lib\x64'
$others = Get-Process browser, guidebrowser, pegasus -ErrorAction SilentlyContinue | Where-Object { $_.Path -notlike 'C:\src\pegasus3d-wt-*' }
if ($others) { Write-Output "a browser outside the lane worktrees is running ($($others.Id -join ',')): not running"; exit 2 }
$outLog = Join-Path $env:TEMP "prefixcost-$PID.out"
$errLog = Join-Path $env:TEMP "prefixcost-$PID.err"
$server = Start-Process python -ArgumentList @('-u', (Join-Path $here 'worldserve.py'), $root, '--port', '0', '--mbit', $Mbit, '--latency-ms', $LatencyMs) `
  -RedirectStandardOutput $outLog -RedirectStandardError $errLog -PassThru -WindowStyle Hidden
try {
  $port = 0
  for ($i = 0; $i -lt 50 -and $port -eq 0; $i++) {
    Start-Sleep -Milliseconds 100
    if ((Test-Path $outLog) -and ((Get-Content $outLog -Raw) -match 'on (\d+)')) { $port = [int]$Matches[1] }
  }
  if ($port -eq 0) { Write-Output 'the server did not start'; exit 3 }
  $cacheDir = Join-Path $env:TEMP "prefixcost-cache-$PID"
  $env:PEGASUS_MESH_CACHE = $cacheDir
  $env:PEGASUS_READ_TIMES = '1'      # the Reader prints each read's time
  $rows = @()
  for ($r = 0; $r -lt $Runs; $r++) {
    Remove-Item -Recurse -Force $cacheDir -ErrorAction SilentlyContinue
    $out = & $exe --world "http://127.0.0.1:$port/$name" --keys-from $worldPath --steps $Steps 2>&1 | Out-String
    $ns = { param($pat) if ($out -match $pat) { [double]$Matches[1] } else { [double]::NaN } }
    $t0 = & $ns 'arrival: opened at (\d+) ns'
    $pre = [regex]::Matches($out, 'reader: prefix read of \d+ bytes took (\d+) us')
    $who = [regex]::Matches($out, 'reader: whole read of \d+ bytes took (\d+) us')
    $preUs = ($pre | ForEach-Object { [double]$_.Groups[1].Value } | Measure-Object -Sum).Sum
    $whoUs = ($who | ForEach-Object { [double]$_.Groups[1].Value } | Measure-Object -Sum).Sum
    $row = [pscustomobject]@{
      prefixReads = $pre.Count; prefixMs = [math]::Round($preUs / 1000, 1); wholeMs = [math]::Round($whoUs / 1000, 1)
      firstMs = [math]::Round(((& $ns 'arrival: first part drawn at (\d+) ns') - $t0) / 1e6, 1)
      tenMs = [math]::Round(((& $ns 'arrival: ten parts drawn at (\d+) ns') - $t0) / 1e6, 1)
      allMs = [math]::Round(((& $ns 'arrival: all parts drawn at (\d+) ns') - $t0) / 1e6, 1)
      gapMs = [math]::Round((& $ns 'longest gap between frames up to then (\d+) us') / 1000, 1)
      textInMs = & $ns 'arrival: \d+ bytes of world in at \+(\d+) ms'
      declaredMs = & $ns 'arrival: parts declared at \+(\d+) ms' }
    $rows += $row
    Write-Output ("run {0}: prefix reads {1} = {2} ms, whole read {3} ms, first {4}, all {5}, gap {6}, text in {7}, declared {8}" -f ($r + 1), $row.prefixReads, $row.prefixMs, $row.wholeMs, $row.firstMs, $row.allMs, $row.gapMs, $row.textInMs, $row.declaredMs)
  }
  foreach ($c in 'prefixReads', 'prefixMs', 'wholeMs', 'firstMs', 'tenMs', 'allMs', 'gapMs', 'textInMs', 'declaredMs') {
    $v = @($rows.$c | Where-Object { -not [double]::IsNaN($_) } | Sort-Object)
    if ($v.Count) { Write-Output ("{0,-12} min {1}  median {2}  max {3}" -f $c, $v[0], $v[[int][math]::Floor(($v.Count - 1) / 2)], $v[-1]) }
  }
  Remove-Item -Recurse -Force $cacheDir -ErrorAction SilentlyContinue
}
finally { Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue }
