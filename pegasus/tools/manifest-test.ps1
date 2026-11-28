# The manifest's driven tests, end to end: serve a split world (tools\worldserve.py, over a slowed link), open it with pegasus
# headless, and read the server's request log and the run's own lines. Run from anywhere:
#   pwsh tools\manifest-test.ps1 [-Mbit 1] [-LatencyMs 100]
# 1. a first open fetches the manifest and every file; a second open of the unchanged world, with the same cache, fetches the
#    manifest and NO file;
# 2. a file whose hash is wrong (same size) is left out, said once, and only its subtree is missing (the run's key count, read
#    from a local copy that leaves the same file out, agrees);
# 3. a file whose size is wrong does the same;
# 4. the world mended: only the file that was left out is fetched, the rest are kept by content.
# 5. a manifest path that does not match the file's own first part: that file is left out;
# 6. a manifest that is a fault does not open the world;
# 7. a file left out in the middle of a world cut deeper (a child's file follows the file that opened its parent): the later files keep
#    their places, so only that one file is left out (W144: a file's place is worked out from its own text);
# 8. the last file is slow: the parts of the files that are in are declared and drawn before it arrives, and the world's rules and
#    derives start only after it ("sim: world ..., rules verified" comes after "bytes of world in"). PEGASUS_MUTANT=rules-early starts
#    them with the first file, and makes (8) FAIL.
# Tests 2 and 3 use a fresh cache each: a file kept by its hash is never fetched, so a cache that held the right file would hide
# the wrong one. PEGASUS_MUTANT=manifest-skip-hash makes (2) FAIL (the file is not left out). Prints PASS or FAIL for each; stops
# the server. Exit 0 only if every test passed.
param(
  [double]$Mbit = 1,
  [double]$LatencyMs = 100,
  [string]$World = '',
  [switch]$Tls              # serve https:// with a self-signed certificate (tools\selfsigned.ps1) that pegasus trusts by its test switch
)
$here = Split-Path -Parent $PSCommandPath
$exe = Join-Path $here '..\build\release\pegasus.exe'
if (-not $World) { $World = Join-Path $here '..\worlds\split\first.world' }
$src = Split-Path -Parent (Resolve-Path $World).Path
$name = Split-Path -Leaf $World
$env:PATH += ';C:\libs\SDL3-3.4.16\lib\x64'

$others = Get-Process browser, guidebrowser, pegasus -ErrorAction SilentlyContinue | Where-Object { $_.Path -notlike 'C:\src\pegasus3d-wt-*' }
if ($others) { Write-Output "a browser outside the lane worktrees is running ($($others.Id -join ',')): not running"; exit 2 }

$tmp = Join-Path $env:TEMP "manifest-test-$PID"
$served = Join-Path $tmp 'served'
New-Item -ItemType Directory -Force $served | Out-Null
Copy-Item (Join-Path $src '*') $served
$outLog = Join-Path $tmp 'server.out'
$errLog = Join-Path $tmp 'server.err'
$scheme = 'http'
$tlsArgs = @()
if ($Tls) {
  $scheme = 'https'
  $certs = Join-Path $tmp 'certs'
  $null = & (Join-Path $here 'selfsigned.ps1') $certs
  $tlsArgs = @('--tls', (Join-Path $certs 'cert.pem'), (Join-Path $certs 'key.pem'))
  $env:PEGASUS_TEST_TRUST_CA = Join-Path $certs 'cert.pem'
}
$server = Start-Process python -ArgumentList (@('-u', (Join-Path $here 'worldserve.py'), $served, '--port', '0', '--mbit', $Mbit, '--latency-ms', $LatencyMs) + $tlsArgs) `
  -RedirectStandardOutput $outLog -RedirectStandardError $errLog -PassThru -WindowStyle Hidden
$failed = 0
function Check($what, $good) {
  if ($good) { Write-Output "PASS $what" } else { Write-Output "FAIL $what"; $script:failed++ }
}
try {
  $port = 0
  for ($i = 0; $i -lt 50 -and $port -eq 0; $i++) {
    Start-Sleep -Milliseconds 100
    if ((Test-Path $outLog) -and ((Get-Content $outLog -Raw) -match 'on (\d+)')) { $port = [int]$Matches[1] }
  }
  if ($port -eq 0) { Write-Output 'the server did not start'; exit 3 }
  $url = "${scheme}://127.0.0.1:$port/$name"
  $files = @(Get-Content (Join-Path $served $name) | Where-Object { $_ -notmatch '^//' -and $_.Trim() }).Count

  function Open-World($keysFrom) {
    $before = (Get-Content $errLog -ErrorAction SilentlyContinue | Measure-Object).Count
    $out = & $exe --world $url --keys-from $keysFrom --steps 300 2>&1 | Out-String
    # (the server's log holds a traceback each time a client drops a connection at exit: only its GET lines are requests)
    $log = @(Get-Content $errLog -ErrorAction SilentlyContinue | Select-Object -Skip $before | Where-Object { $_ -match ' GET /' })
    [pscustomobject]@{ out = $out; requests = $log.Count; files = @($log | Where-Object { $_ -match 'GET /first-' }).Count }
  }
  $okRun = { param($o) ($o -match 'jobs started nearest first: yes: ok') -and ($o -match 'check resident keys: \d+ \(want \d+\), \d+ released: ok') }

  # 1. cold, then warm
  $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache1'
  $a = Open-World (Join-Path $src $name)
  Check "first open: $($a.files) of $files files fetched, none left out, the run's checks pass" (($a.files -eq $files) -and ($a.out -match ' 0 left out') -and (& $okRun $a.out))
  $b = Open-World (Join-Path $src $name)
  Check "second open of the unchanged world: $($b.files) files fetched ($($b.requests) request in all), $files kept" `
    (($b.files -eq 0) -and ($b.requests -eq 1) -and ($b.out -match "$files kept, 0 fetched, 0 left out") -and (& $okRun $b.out))

  # 2. a file with one bit changed (same size, so the hash is wrong), in the served folder and in a local copy that gives the
  # run's key count (it leaves the file out too)
  $lines = Get-Content (Join-Path $served $name)
  $hashFile = ($lines | Where-Object { $_ -match '^first-07' }).Split(' ')[0]
  $sizeFile = ($lines | Where-Object { $_ -match '^first-05' }).Split(' ')[0]
  $local = Join-Path $tmp 'local'
  New-Item -ItemType Directory -Force $local | Out-Null
  Copy-Item (Join-Path $src '*') $local
  $bytes = [System.IO.File]::ReadAllBytes((Join-Path $src $hashFile))
  $bytes[$bytes.Length - 2] = $bytes[$bytes.Length - 2] -bxor 1
  foreach ($d in @($served, $local)) { [System.IO.File]::WriteAllBytes((Join-Path $d $hashFile), $bytes) }
  $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache2'
  $c = Open-World (Join-Path $local $name)
  $said = @([regex]::Matches($c.out, "file $hashFile \(subtree [^)]*\) left out: its hash is")).Count
  Check "wrong hash: $hashFile left out and said once ($said), 1 of $files left out, the run's checks pass" `
    (($said -eq 1) -and ($c.out -match ' 1 left out') -and (& $okRun $c.out))

  # 3. one file a byte longer
  Copy-Item (Join-Path $src $hashFile) (Join-Path $served $hashFile) -Force
  Copy-Item (Join-Path $src $hashFile) (Join-Path $local $hashFile) -Force
  $longer = [System.IO.File]::ReadAllBytes((Join-Path $src $sizeFile)) + [byte]10
  foreach ($d in @($served, $local)) { [System.IO.File]::WriteAllBytes((Join-Path $d $sizeFile), $longer) }
  $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache3'
  $d3 = Open-World (Join-Path $local $name)
  $said = @([regex]::Matches($d3.out, "file $sizeFile \(subtree [^)]*\) left out: its size is")).Count
  Check "wrong size: $sizeFile left out and said once ($said), 1 left out, the run's checks pass" `
    (($said -eq 1) -and ($d3.out -match ' 1 left out') -and (& $okRun $d3.out))

  # 4. mended: with cache3, only the file that was left out is fetched
  Copy-Item (Join-Path $src $sizeFile) (Join-Path $served $sizeFile) -Force
  $e = Open-World (Join-Path $src $name)
  Check "the world mended: $($e.files) file fetched again (the one left out), the rest kept" `
    (($e.files -eq 1) -and ($e.out -match "$($files - 1) kept, 1 fetched, 0 left out") -and (& $okRun $e.out))

  # 5. a manifest line whose path is wrong (first-07 says first.wrong; it begins at first.plinth): that file is left out, said once,
  # the rest stay; the local copy of the manifest says the same, so the run's key count agrees
  Copy-Item (Join-Path $src $sizeFile) (Join-Path $local $sizeFile) -Force   # (test 3 left the local copy's longer)
  $good = Get-Content (Join-Path $src $name)
  $badPath = $good | ForEach-Object { if ($_ -match '^first-07') { ($_ -replace ' \S+$', ' first.wrong') } else { $_ } }
  foreach ($d in @($served, $local)) { Set-Content -Path (Join-Path $d $name) -Value $badPath -Encoding ascii }
  $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache5'
  $f = Open-World (Join-Path $local $name)
  $said = @([regex]::Matches($f.out, "file first-07.ddd \(subtree first.wrong\) left out: its text begins at the part plinth, the manifest says first.wrong")).Count
  Check "wrong path: first-07.ddd left out and said once ($said), 1 left out, the run's checks pass" `
    (($said -eq 1) -and ($f.out -match ' 1 left out') -and (& $okRun $f.out))

  # 6. a manifest that names a file twice, and one that is an error page: each is said once, no file is fetched, the world does not open
  $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache6'
  $dup = @($good) + ($good | Where-Object { $_ -match '^first-03' })
  Set-Content -Path (Join-Path $served $name) -Value $dup -Encoding ascii
  $g = Open-World (Join-Path $src $name)
  $said = @([regex]::Matches($g.out, 'cannot be used: it names the file first-03.ddd twice\. The world does not open\.')).Count
  Check "duplicate file in the manifest: said once ($said), $($g.files) files fetched, no arrival line" `
    (($said -eq 1) -and ($g.files -eq 0) -and -not ($g.out -match 'arrival: \d+ files'))
  Set-Content -Path (Join-Path $served $name) -Value @('<html>', '<body>502 Bad Gateway</body>', '</html>') -Encoding ascii
  $h = Open-World (Join-Path $src $name)
  $said = @([regex]::Matches($h.out, 'cannot be used: it has a line of 1 fields, not four')).Count
  Check "manifest that cannot be read: said once ($said), $($h.files) files fetched, no arrival line" `
    (($said -eq 1) -and ($h.files -eq 0) -and -not ($h.out -match 'arrival: \d+ files'))

  # 7. a world cut deeper (files of a part's children follow the file that opens the part): the file that opens a parent is left out
  $deep = Join-Path $tmp 'deep'
  $deepLocal = Join-Path $tmp 'deeplocal'
  python (Join-Path $here 'splitworld.py') (Join-Path $here '..\worlds\first.ddd') $deep first --max-indent 4 | Out-Null
  Copy-Item $deep $deepLocal -Recurse
  $dl = @(Get-Content (Join-Path $deep 'first.world') | Where-Object { $_ -notmatch '^//' -and $_.Trim() })
  $victim = ''
  for ($k = 0; $k + 1 -lt $dl.Count; $k++) {
    $pa = $dl[$k].Split(' ')[3]; $pb = $dl[$k + 1].Split(' ')[3]
    if ($pb.StartsWith($pa + '.') -and $dl[$k].Split(' ')[0] -notmatch 'first-00') { $victim = $dl[$k].Split(' ')[0]; break }
  }
  $vb = [System.IO.File]::ReadAllBytes((Join-Path $deep $victim))
  $vb[$vb.Length - 2] = $vb[$vb.Length - 2] -bxor 1
  [System.IO.File]::WriteAllBytes((Join-Path $deepLocal $victim), $vb)
  $deepServed = Join-Path $served 'deep'
  Copy-Item $deepLocal $deepServed -Recurse
  $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache7'
  $urlDeep = "${scheme}://127.0.0.1:$port/deep/first.world"
  $outDeep = & $exe --world $urlDeep --keys-from (Join-Path $deepLocal 'first.world') --steps 300 2>&1 | Out-String
  $saidDeep = @([regex]::Matches($outDeep, "file $victim \(subtree [^)]*\) left out: its hash is")).Count
  Check "a file left out in the middle ($victim, which opens a part whose children follow): said once ($saidDeep), the only one left out, no other file's place is wrong" `
    (($saidDeep -eq 1) -and ($outDeep -match ' 1 left out') -and -not ($outDeep -match 'its text begins at'))
  # (what the world is without the parent a file opened is another matter: its children's text then has no parent to be under, the reader
  # refuses it, and nothing draws. Only the file's own place is tested here.)

  # 8. the last file is slow
  $slowOut = Join-Path $tmp 'slow.out'
  $slowErr = Join-Path $tmp 'slow.err'
  $last = ($dl = @(Get-Content (Join-Path $src $name) | Where-Object { $_ -notmatch '^//' -and $_.Trim() }))[-1].Split(' ')[0]
  $slow = Start-Process python -ArgumentList (@('-u', (Join-Path $here 'worldserve.py'), $src, '--port', '0', '--mbit', $Mbit, '--latency-ms', $LatencyMs, '--delay', "$last=1500") + $tlsArgs) `
    -RedirectStandardOutput $slowOut -RedirectStandardError $slowErr -PassThru -WindowStyle Hidden
  try {
    $slowPort = 0
    for ($i = 0; $i -lt 50 -and $slowPort -eq 0; $i++) {
      Start-Sleep -Milliseconds 100
      if ((Test-Path $slowOut) -and ((Get-Content $slowOut -Raw) -match 'on (\d+)')) { $slowPort = [int]$Matches[1] }
    }
    $env:PEGASUS_MESH_CACHE = Join-Path $tmp 'cache8'
    $lineList = @(& $exe --world "${scheme}://127.0.0.1:$slowPort/$name" --keys-from (Join-Path $src $name) --steps 500 2>&1 | Out-String -Stream)
    $early = ($lineList | Select-String -Pattern 'parts declared early' | Select-Object -First 1).LineNumber
    $textIn = ($lineList | Select-String -Pattern 'bytes of world in at' | Select-Object -First 1).LineNumber
    $drawn = ($lineList | Select-String -Pattern 'arrival: first part drawn' | Select-Object -First 1).LineNumber
    $rules = ($lineList | Select-String -Pattern 'sim: world .*rules verified' | Select-Object -First 1).LineNumber
    $joined = $lineList -join "`n"
    Check "the last file is slow: parts declared early (line $early) and the first drawn (line $drawn) before the text is in (line $textIn), the rules start after it (line $rules)" `
      ($early -and $drawn -and $textIn -and $rules -and ($early -lt $textIn) -and ($drawn -lt $textIn) -and ($textIn -lt $rules) -and (& $okRun $joined))
  }
  finally { Stop-Process -Id $slow.Id -Force -ErrorAction SilentlyContinue }
}
finally {
  Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
  Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
if ($failed) { Write-Output "FAILED: $failed"; exit 1 }
Write-Output 'all passed'
