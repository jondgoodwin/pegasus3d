# Two-client loopback tests (W117, W125): a server and headless clients, as processes. Build first (build.ps1); output goes to build\run-<scenario>. Usage: net-test.ps1 loop|loss|glide|glideloss|avatar|rejoin|refuse [world]
param([string]$Scenario = "loop", [string]$World = "worlds\first.ddd", [string]$Tag = "", [string]$AfterSteps = "60")
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$pg = Split-Path -Parent $here
$exe = Join-Path $pg "build\release\pegasus.exe"
$env:PATH = "$env:PATH;C:\libs\SDL3-3.4.16\lib\x64"
$out = Join-Path $pg "build\run-$Scenario$Tag"
New-Item -ItemType Directory -Force $out | Out-Null
Remove-Item "$out\*" -Force -ErrorAction SilentlyContinue
$procs = @()
function Start-P($name, $argv) {
  $p = Start-Process -FilePath $exe -ArgumentList $argv -WorkingDirectory $pg -RedirectStandardOutput "$out\$name.log" -RedirectStandardError "$out\$name.err" -PassThru -WindowStyle Hidden
  $script:procs += $p
  $p
}
function Wait-Line($file, $pattern, $secs) {
  $end = (Get-Date).AddSeconds($secs)
  while ((Get-Date) -lt $end) {
    if ((Test-Path $file) -and (Select-String -Path $file -Pattern $pattern -Quiet)) { return $true }
    Start-Sleep -Milliseconds 100
  }
  $false
}
function Wait-Exit($p, $secs) { if (-not $p.WaitForExit($secs * 1000)) { Write-Host "TIMEOUT: pid $($p.Id) did not exit in $secs s"; Stop-Process -Id $p.Id -Force } }

$port = 47117
$srvArgs = @("--serve", "$port", "--loopback", "--world", $World)
$target = "127.0.0.1:$port"
switch ($Scenario) {
  "loop" { $srvArgs += @("--steps", "1800", "--until-empty") }
  "loss" { $srvArgs += @("--steps", "1800", "--until-empty", "--impair", "50", "20", "10") }
  "glide" { $srvArgs += @("--steps", "1800", "--until-empty") }
  "avatar" { $srvArgs += @("--steps", "1800", "--until-empty") }
  "glideloss" { $srvArgs += @("--steps", "1800", "--until-empty", "--impair", "50", "20", "10") }
  "rejoin" { $srvArgs += @("--steps", "900") }
  "refuse" { $srvArgs += @("--steps", "400") }
}
$srv = Start-P "server" $srvArgs
if (-not (Wait-Line "$out\server.log" "Serving on UDP port" 20)) { Write-Host "server did not start"; Stop-Process -Id $srv.Id -Force; exit 1 }
if ($Scenario -eq "loss" -or $Scenario -eq "glideloss") {
  if (-not (Wait-Line "$out\server.log" "Relay: port" 5)) { Write-Host "no relay"; exit 1 }
  $rp = (Select-String -Path "$out\server.log" -Pattern "Relay: port (\d+)").Matches[0].Groups[1].Value
  $target = "127.0.0.1:$rp"
}
$cl = @("--join", $target, "--world", $World)
if ($Scenario -eq "glide" -or $Scenario -eq "glideloss") {
  # B stands and watches; A presses the white rook (a1) and then the square e4; the rook glides there
  $b = Start-P "clientB" ($cl + @("--steps", "640", "--stand"))
  Start-Sleep -Milliseconds 1500
  $a = Start-P "clientA" ($cl + @("--steps", "520", "--press", "220,-175,175", "--press", "270,25,25"))
  Start-Sleep -Milliseconds 500
  $c = Start-P "clientC" ($cl + @("--shot", "$out\clientC.bmp", "--shot-after", "330"))
  Wait-Exit $a 60; Wait-Exit $b 60; Wait-Exit $c 60; Wait-Exit $srv 60
} elseif ($Scenario -eq "avatar") {
  # A walks toward the board; C stands at the start and shoots what it sees (A drawn as the world's avatar)
  $a = Start-P "clientA" ($cl + @("--steps", "420"))
  Start-Sleep -Milliseconds 1500
  $c = Start-P "clientC" ($cl + @("--stand", "--shot", "$out\clientC.bmp", "--shot-after", $AfterSteps))
  Wait-Exit $a 60; Wait-Exit $c 60; Wait-Exit $srv 60
} elseif ($Scenario -eq "loop" -or $Scenario -eq "loss") {
  $b = Start-P "clientB" ($cl + @("--steps", "520", "--stand"))
  Start-Sleep -Milliseconds 1500
  $a = Start-P "clientA" ($cl + @("--steps", "420"))
  Wait-Exit $a 60; Wait-Exit $b 60; Wait-Exit $srv 60
} elseif ($Scenario -eq "rejoin") {
  $a1 = Start-P "clientA1" ($cl + @("--steps", "200"))
  Wait-Exit $a1 40
  $a2 = Start-P "clientA2" ($cl + @("--steps", "200"))
  Wait-Exit $a2 40; Wait-Exit $srv 60
} elseif ($Scenario -eq "refuse") {
  $a = Start-P "clientA" @("--join", $target, "--parts40", "--steps", "200")
  Wait-Exit $a 40; Wait-Exit $srv 60
}
foreach ($p in $procs) { if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
foreach ($f in Get-ChildItem "$out\*.log") {
  Write-Host "== $($f.Name)"
  Get-Content $f | Select-String -Pattern "^(check|net |sim: |pegasus:|Serving|Relay|REFUSED|loader:)" | ForEach-Object { $_.Line }
}
foreach ($p in $procs) { Write-Host "exit pid $($p.Id): $($p.ExitCode)" }
# glide scenarios: every client must end with each moved part where the server has it
if ($Scenario -like "glide*") {
  $srvParts = @(Get-Content "$out\server.log" | Select-String -Pattern "^net server: final part (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
  foreach ($n in "clientA", "clientB", "clientC") {
    $got = @(Get-Content "$out\$n.log" -ErrorAction SilentlyContinue | Select-String -Pattern "^net client: person \d+ final part (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    if ($n -eq "clientC") { continue }
    $same = ($got.Count -eq $srvParts.Count) -and (($got -join "|") -eq ($srvParts -join "|"))
    Write-Host "GLIDE-CHECK $n final parts [$($got -join '; ')] vs server [$($srvParts -join '; ')]: $(if ($same) { 'MATCH' } else { 'DIFFER' })"
  }
}
