# Build nativecheck with the default Cone compiler; prints the last lines of the build
$snap = "C:\src\cone"
$env:LIB = "$env:LIB;C:\libs\SDL3-3.4.16\lib\x64;C:\src\cone\deps\openssl"
$env:PATH = "$env:PATH;C:\libs\SDL3-3.4.16\lib\x64"
$env:CONGO_HOME = "$PSScriptRoot\..\congo-home"
$env:CONEC = "$snap\build\x64-release\conec.exe"
$env:CONESTD = "$snap\build\x64-release\conestd.lib"
Push-Location $PSScriptRoot
python "$snap\tools\congo\congo.py" build --release 2>&1 | Select-Object -Last 40
Pop-Location
