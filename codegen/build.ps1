# Build and test codegen with the default Cone compiler
$snap = "C:\src\cone"
$env:CONGO_HOME = "$PSScriptRoot\..\tools\congo-home"
$env:CONEC = "$snap\build\x64-release\conec.exe"
$env:CONESTD = "$snap\build\x64-release\conestd.lib"
Push-Location $PSScriptRoot
python "$snap\tools\congo\congo.py" build --release 2>&1 | Select-Object -Last 40
Pop-Location
