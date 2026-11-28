# Build pegasus-server (this folder) with the default Cone compiler: the server program, Sim in the server role with a Loader
# that builds only Sim's products, and the network (no Render, window, GPU, SDL of its own; sources in ..\pegasuscore).
# Congo names the executable for the package (pegasusserver.exe); it is copied to build\release\pegasus-server.exe.
$snap = "C:\src\cone"
$env:LIB = "$env:LIB;C:\libs\SDL3-3.4.16\lib\x64;C:\src\cone\deps\openssl"
$env:PATH = "$env:PATH;C:\libs\SDL3-3.4.16\lib\x64"
$env:CONGO_HOME = "$PSScriptRoot\..\tools\congo-home"
$env:CONEC = "$snap\build\x64-release\conec.exe"
$env:CONESTD = "$snap\build\x64-release\conestd.lib"
Push-Location $PSScriptRoot
python "$snap\tools\congo\congo.py" build --release 2>&1 | Select-Object -Last 40
if (Test-Path build\release\pegasusserver.exe) { Copy-Item build\release\pegasusserver.exe build\release\pegasus-server.exe -Force }
Pop-Location
