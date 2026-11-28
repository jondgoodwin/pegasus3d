# Worlds over https, end to end on loopback with a self-signed certificate (tools\selfsigned.ps1; pegasus trusts it only through its
# test switch PEGASUS_TEST_TRUST_CA). Run from anywhere:  pwsh tools\https-test.ps1
# 1. first.ddd and the split world open over https, cold (a fresh cache) and warm (the cache filled): the run's checks pass, and the
#    split world's second open fetches the manifest and no file;
# 2. a certificate that is not the trusted one, and no trust switch at all (the system's roots do not know it), each refuse the fetch:
#    the refusal is said once, the world does not open, and no file is fetched;
# 3. connected play: a server runs the world from its https address, a client with no copy of it (--world-from-server) is told the
#    address in the Welcome, fetches the world from there itself, and joins (the definition-sum check holds it to the server's).
# The real-internet figures are tools\arrival-test.ps1 -Url. Prints PASS or FAIL for each; stops what it started; exit 0 only if all passed.
$here = Split-Path -Parent $PSCommandPath
$exe = Join-Path $here '..\build\release\pegasus.exe'
$worlds = Join-Path $here '..\worlds'
$env:PATH += ';C:\libs\SDL3-3.4.16\lib\x64'
$others = Get-Process browser, guidebrowser, pegasus -ErrorAction SilentlyContinue | Where-Object { $_.Path -notlike 'C:\src\pegasus3d-wt-*' }
if ($others) { Write-Output "a browser outside the lane worktrees is running ($($others.Id -join ',')): not running"; exit 2 }

$tmp = Join-Path $env:TEMP "https-test-$PID"
$certs = Join-Path $tmp 'certs'
New-Item -ItemType Directory -Force $tmp | Out-Null
$null = & (Join-Path $here 'selfsigned.ps1') $certs
$failed = 0
function Check($what, $good) { if ($good) { Write-Output "PASS $what" } else { Write-Output "FAIL $what"; $script:failed++ } }
function Count($text, $pat) { @($text -split "`n" | Where-Object { $_ -match $pat }).Count }

$out = Join-Path $tmp 's.out'; $err = Join-Path $tmp 's.err'
$server = Start-Process python -ArgumentList @('-u', (Join-Path $here 'worldserve.py'), (Resolve-Path $worlds).Path, '--port', '0', '--tls', (Join-Path $certs 'cert.pem'), (Join-Path $certs 'key.pem')) `
  -RedirectStandardOutput $out -RedirectStandardError $err -PassThru -WindowStyle Hidden
$srvPegasus = $null
try {
  $port = 0
  for ($i = 0; $i -lt 50 -and $port -eq 0; $i++) {
    Start-Sleep -Milliseconds 100
    if ((Test-Path $out) -and ((Get-Content $out -Raw) -match 'on (\d+)')) { $port = [int]$Matches[1] }
  }
  if ($port -eq 0) { Write-Output 'the server did not start'; exit 3 }
  $base = "https://127.0.0.1:$port"
  $cache = Join-Path $tmp 'cache'
  $env:PEGASUS_MESH_CACHE = $cache
  function Open($path, $local, $extra = @()) {
    (& $exe --world "$base/$path" --keys-from (Join-Path $worlds $local) --steps 300 @extra 2>&1 | Out-String)
  }
  $env:PEGASUS_TEST_TRUST_CA = Join-Path $certs 'cert.pem'

  # 1
  foreach ($w in @(@('first.ddd', 'first.ddd'), @('split/first.world', 'split\first.world'))) {
    Remove-Item -Recurse -Force $cache -ErrorAction SilentlyContinue
    $cold = Open $w[0] $w[1]
    Check "$($w[0]) over https, cold: answered with TLS, the run's checks pass" (($cold -match 'with TLS \(certificate verified\)') -and ($cold -match 'jobs started nearest first: yes: ok') -and ($cold -match 'check resident keys: \d+ \(want \d+\), \d+ released: ok'))
    $warm = Open $w[0] $w[1]
    $okWarm = ($warm -match 'jobs started nearest first: yes: ok') -and ($warm -match '0 built')
    if ($w[0] -like '*.world') { $okWarm = $okWarm -and ($warm -match 'arrival: 15 files: 15 kept, 0 fetched') }
    Check "$($w[0]) over https, warm: nothing built$(if ($w[0] -like '*.world') { ', every file kept, none fetched' })" $okWarm
    if (-not $okWarm) { $warm -split "`n" | Where-Object { $_ -match 'arrival: \d+ files|mesh cache:|fetch:' } | ForEach-Object { Write-Output "   $_" } }
  }

  # --world takes the address on its own (no local copy named for a key count): the run exits ok
  Remove-Item -Recurse -Force $cache -ErrorAction SilentlyContinue
  $null = & $exe --world "$base/split/first.world" --steps 300 2>&1
  Check '--world with an https address and nothing else: exit 0' ($LASTEXITCODE -eq 0)

  # 2
  foreach ($case in @(@('another certificate than the trusted one', (Join-Path $certs 'other.pem')), @('no trust switch (the system roots)', ''))) {
    foreach ($w in @('first.ddd', 'split/first.world')) {
      if ($case[1]) { $env:PEGASUS_TEST_TRUST_CA = $case[1] } else { Remove-Item Env:PEGASUS_TEST_TRUST_CA -ErrorAction SilentlyContinue }
      Remove-Item -Recurse -Force $cache -ErrorAction SilentlyContinue
      $o = Open $w ($w -replace '/', '\')
      Check "$w with $($case[0]): refused and said once, the world not opened" ((Count $o 'REFUSED') -eq 1 -and -not ($o -match 'answered over') -and -not ($o -match 'parts declared'))
    }
  }
  $env:PEGASUS_TEST_TRUST_CA = Join-Path $certs 'cert.pem'

  # 3
  Remove-Item -Recurse -Force $cache -ErrorAction SilentlyContinue
  $srvOut = Join-Path $tmp 'srv.out'
  $srvPegasus = Start-Process $exe -ArgumentList @('--serve', '47713', '--world', "$base/split/first.world", '--keys-from', (Join-Path $worlds 'split\first.world'), '--loopback', '--until-empty', '--steps', '900') `
    -RedirectStandardOutput $srvOut -RedirectStandardError (Join-Path $tmp 'srv.err') -PassThru -WindowStyle Hidden
  Start-Sleep -Seconds 4
  $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache-client'
  $c = & $exe --join 127.0.0.1:47713 --world-from-server --steps 400 2>&1 | Out-String
  Check 'connected: the client (no copy of the world) was told the address, fetched from there, and joined' (($c -match "the server runs the world at $([regex]::Escape($base))/split/first.world") -and ($c -match 'answered over') -and ($c -match 'sim: joined as person 1') -and ($c -match 'check join: .*: ok'))
  Check 'connected: the client opened no file on its disk (no --world was given)' (-not ($c -match 'cannot be read'))
}
finally {
  Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
  if ($srvPegasus) { Stop-Process -Id $srvPegasus.Id -Force -ErrorAction SilentlyContinue }
}
if ($failed -eq 0) { Write-Output 'all passed'; exit 0 } else { Write-Output "$failed FAILED"; exit 1 }
