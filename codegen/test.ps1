# Builds codegen and runs its tests with the default Cone compiler, then checks every encoding against LLVM.
# The LLVM check is a testing tool only (C:\LLVM\23\bin\llvm-mc.exe); nothing in codegen depends on it.
$snap = "C:\src\cone"
$env:CONGO_HOME = "$PSScriptRoot\..\tools\congo-home"
$env:CONEC = "$snap\build\x64-release\conec.exe"
$env:CONESTD = "$snap\build\x64-release\conestd.lib"
Push-Location $PSScriptRoot
python "$snap\tools\congo\congo.py" test --release 2>&1 | Select-Object -Last 40
python "$PSScriptRoot\tests\check_encodings.py"
Pop-Location
