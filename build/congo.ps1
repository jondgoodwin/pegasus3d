# W148 lane helper: run congo in a folder of this worktree with pegasus\build.ps1's environment. Usage: congo.ps1 FOLDER ARGS...
$snap = "C:\src\cone"
$env:LIB = "$env:LIB;C:\libs\SDL3-3.4.16\lib\x64;C:\src\cone\deps\openssl"
$env:PATH = "$env:PATH;C:\libs\SDL3-3.4.16\lib\x64"
$env:CONGO_HOME = "C:\src\pegasus3d-wt-boids\tools\congo-home"
$env:CONEC = "$snap\build\x64-release\conec.exe"
$env:CONESTD = "$snap\build\x64-release\conestd.lib"
Push-Location $args[0]
$rest = $args[1..($args.Length-1)]
python "$snap\tools\congo\congo.py" @rest 2>&1 | Select-Object -Last 40
Pop-Location
