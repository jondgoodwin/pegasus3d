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
  "light" { $srvArgs += @("--steps", "1800", "--until-empty"); $env:PEGASUS_LIGHT_CHANGE = "400:sun:25000" }
  "lamp" { $srvArgs += @("--steps", "480") }       # W145: lamp.ddd; A walks into the lantern's area (B stands away); the server ends while A is still there
  "avatar" { $srvArgs += @("--steps", "1800", "--until-empty") }
  "glideloss" { $srvArgs += @("--steps", "1800", "--until-empty", "--impair", "50", "20", "10") }
  "rejoin" { $srvArgs += @("--steps", "900") }
  "refuse" { $srvArgs += @("--steps", "400") }
}
$srv = Start-P "server" $srvArgs
Remove-Item Env:PEGASUS_LIGHT_CHANGE -ErrorAction SilentlyContinue   # only the server changes a light
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
} elseif ($Scenario -eq "loop" -or $Scenario -eq "loss" -or $Scenario -eq "light") {
  $b = Start-P "clientB" ($cl + @("--steps", "520", "--stand"))
  Start-Sleep -Milliseconds 1500
  $a = Start-P "clientA" ($cl + @("--steps", "420"))
  Wait-Exit $a 60; Wait-Exit $b 60; Wait-Exit $srv 60
} elseif ($Scenario -eq "lamp") {
  $b = Start-P "clientB" ($cl + @("--steps", "420", "--stand"))
  Start-Sleep -Milliseconds 1000
  $a = Start-P "clientA" ($cl + @("--steps", "520"))
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
  Get-Content $f | Select-String -Pattern "^(check|net |sim: |pegasus:|Serving|Relay|REFUSED|loader:|render: light)" | ForEach-Object { $_.Line }
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
# light scenario (W142): the server changes the sun's lux at its step 400; each client's Render must end with the server's light
if ($Scenario -eq "light") {
  $srvLight = (Get-Content "$out\server.log" | Select-String -Pattern "^sim: light: (.*?), \d+ look changes").Matches | Select-Object -First 1
  Write-Host "LIGHT-CHECK server: $($srvLight.Groups[1].Value)"
  foreach ($n in "clientA", "clientB") {
    $r = (Get-Content "$out\$n.log" -ErrorAction SilentlyContinue | Select-String -Pattern "^render: light: (.*)$").Matches | Select-Object -First 1
    $s = (Get-Content "$out\$n.log" -ErrorAction SilentlyContinue | Select-String -Pattern "^sim: light: (.*?), \d+ look changes").Matches | Select-Object -First 1
    $lux = [regex]::Match($srvLight.Groups[1].Value, "lux ([\d.]+)").Groups[1].Value
    $rlux = [regex]::Match($r.Groups[1].Value, "lux ([\d.]+)").Groups[1].Value
    $slux = [regex]::Match($s.Groups[1].Value, "lux ([\d.]+)").Groups[1].Value
    Write-Host "LIGHT-CHECK $n render: $($r.Groups[1].Value) | sim: $($s.Groups[1].Value): $(if (($lux -eq $rlux) -and ($lux -eq $slux)) { 'MATCH' } else { 'DIFFER' })"
  }
}
# lamp scenario (W145): the world's rule (lamp.ddd's derive of the glass's tint, run on the server) lights the lamp when client A is near;
# the server, client A and client B must each end with the same lamp light in Sim and in Render
if ($Scenario -eq "lamp") {
  $srvLamp = (Get-Content "$out\server.log" | Select-String -Pattern "^sim: light: .*?(\d+) lamps \(first ([\d.]+) cd\)").Matches | Select-Object -First 1
  $cd = $srvLamp.Groups[2].Value
  Write-Host "LAMP-CHECK server: lamp $cd cd, $((Get-Content "$out\server.log" | Select-String -Pattern 'tint set to').Count) tint changes"
  foreach ($n in "clientA", "clientB") {
    $r = (Get-Content "$out\$n.log" -ErrorAction SilentlyContinue | Select-String -Pattern "^render: light: .*?\(first ([\d.]+) cd\)").Matches | Select-Object -First 1
    $s = (Get-Content "$out\$n.log" -ErrorAction SilentlyContinue | Select-String -Pattern "^sim: light: .*?\(first ([\d.]+) cd\)").Matches | Select-Object -First 1
    $ch = [regex]::Match((Get-Content "$out\$n.log" | Select-String -Pattern "^sim: light: .* (\d+) look changes").Line, "(\d+) look changes").Groups[1].Value
    $sch = [regex]::Match((Get-Content "$out\server.log" | Select-String -Pattern "^sim: light: .* (\d+) look changes").Line, "(\d+) look changes").Groups[1].Value
    Write-Host "LAMP-CHECK $n render $($r.Groups[1].Value) cd | sim $($s.Groups[1].Value) cd, $ch look changes (server $sch): $(if (($cd -eq $r.Groups[1].Value) -and ($cd -eq $s.Groups[1].Value) -and ($ch -eq $sch) -and ([int]$ch -ge 2)) { 'MATCH' } else { 'DIFFER' })"
  }
}
