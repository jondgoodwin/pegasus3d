# Arrival over a slowed loopback link: serve a world with tools\worldserve.py, run pegasus headless against its URL, and print
# the arrival marks in milliseconds from the world's opening. Run from anywhere:
#   pwsh tools\arrival-test.ps1 -World worlds\split\first.world [-Mbit 1] [-LatencyMs 100] [-Cache cold|warm] [-Runs 3]
# -Mbit 0 and -LatencyMs 0 is the full speed of the loopback. The mesh cache is a fresh folder for 'cold', and for 'warm' one
# filled by an unmeasured run first. Prints one line per run and the median. Stops the server when it is done.
param(
  [Parameter(Mandatory)][string]$World,
  [double]$Mbit = 0,
  [double]$LatencyMs = 0,
  [ValidateSet('cold', 'warm')][string]$Cache = 'cold',
  [int]$Runs = 3,
  [int]$Steps = 600
)
$here = Split-Path -Parent $PSCommandPath
$exe = Join-Path $here '..\build\release\pegasus.exe'
$worldPath = (Resolve-Path $World).Path
$root = Split-Path -Parent $worldPath
$name = Split-Path -Leaf $worldPath
$env:PATH += ';C:\libs\SDL3-3.4.16\lib\x64'

# the headset check: a browser of Jon's is running, so nothing runs
$others = Get-Process browser, guidebrowser, pegasus -ErrorAction SilentlyContinue | Where-Object { $_.Path -notlike 'C:\src\pegasus3d-wt-*' }
if ($others) { Write-Output "a browser outside the lane worktrees is running ($($others.Id -join ',')): not running"; exit 2 }

$outLog = Join-Path $env:TEMP "worldserve-$PID.out"
$errLog = Join-Path $env:TEMP "worldserve-$PID.err"
$server = Start-Process python -ArgumentList @('-u', (Join-Path $here 'worldserve.py'), $root, '--port', '0', '--mbit', $Mbit, '--latency-ms', $LatencyMs) `
  -RedirectStandardOutput $outLog -RedirectStandardError $errLog -PassThru -WindowStyle Hidden
try {
  $port = 0
  for ($i = 0; $i -lt 50 -and $port -eq 0; $i++) {
    Start-Sleep -Milliseconds 100
    if ((Test-Path $outLog) -and ((Get-Content $outLog -Raw) -match 'on (\d+)')) { $port = [int]$Matches[1] }
  }
  if ($port -eq 0) { Write-Output 'the server did not start'; exit 3 }
  $url = "http://127.0.0.1:$port/$name"
  $cacheDir = Join-Path $env:TEMP "arrival-cache-$PID"
  $env:PEGASUS_MESH_CACHE = $cacheDir
  function Run-Once {
    $out = & $exe --world $url --keys-from $worldPath --steps $Steps 2>&1 | Out-String
    $ns = { param($pat) if ($out -match $pat) { [double]$Matches[1] } else { [double]::NaN } }
    $t0 = & $ns 'arrival: opened at (\d+) ns'
    $first = (& $ns 'arrival: first part drawn at (\d+) ns') - $t0
    $ten = (& $ns 'arrival: ten parts drawn at (\d+) ns') - $t0
    $all = (& $ns 'arrival: all parts drawn at (\d+) ns') - $t0
    $gap = & $ns 'longest gap between frames up to then (\d+) us'
    $files = & $ns 'arrival: \d+ bytes of world in at \+(\d+) ms'
    $declared = & $ns 'arrival: parts declared at \+(\d+) ms'
    $landed = & $ns 'all \d+ keys landed (\d+) ms'
    $ok = ($out -match 'jobs started nearest first: yes: ok') -and ($out -match 'check resident keys: \d+ \(want \d+\), \d+ released: ok')
    [pscustomobject]@{ firstMs = [math]::Round($first / 1e6, 1); tenMs = [math]::Round($ten / 1e6, 1); allMs = [math]::Round($all / 1e6, 1);
      gapMs = [math]::Round($gap / 1000, 1); textInMs = $files; declaredMs = $declared; keysLandedMs = $landed; ok = $ok }
  }
  if ($Cache -eq 'warm') { Remove-Item -Recurse -Force $cacheDir -ErrorAction SilentlyContinue; $null = Run-Once }
  $rows = @()
  for ($r = 0; $r -lt $Runs; $r++) {
    if ($Cache -eq 'cold') { Remove-Item -Recurse -Force $cacheDir -ErrorAction SilentlyContinue }
    $row = Run-Once
    $rows += $row
    Write-Output ("run {0}: first {1} ms, ten {2} ms, all {3} ms, longest frame gap {4} ms, text in {5} ms, declared {6} ms, keys landed {7} ms, checks ok {8}" -f `
      ($r + 1), $row.firstMs, $row.tenMs, $row.allMs, $row.gapMs, $row.textInMs, $row.declaredMs, $row.keysLandedMs, $row.ok)
  }
  function Median($v) { $s = @($v | Sort-Object); $s[[int][math]::Floor(($s.Count - 1) / 2)] }
  Write-Output ("MEDIAN {0} {1} Mbit {2} ms {3}: first {4} ms, ten {5} ms, all {6} ms, longest frame gap {7} ms (text in {8} ms)" -f `
    $name, $Mbit, $LatencyMs, $Cache, (Median $rows.firstMs), (Median $rows.tenMs), (Median $rows.allMs), (Median $rows.gapMs), (Median $rows.textInMs))
  Remove-Item -Recurse -Force $cacheDir -ErrorAction SilentlyContinue
}
finally {
  Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
}
