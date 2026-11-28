# Builds codegen and runs its tests with the default Cone compiler, then checks every encoding against LLVM.
# The LLVM check is a testing tool only (C:\LLVM\23\bin\llvm-mc.exe); nothing in codegen depends on it.
$snap = "C:\src\cone"
$env:CONGO_HOME = "$PSScriptRoot\..\tools\congo-home"
$env:CONEC = "$snap\build\x64-release\conec.exe"
$env:CONESTD = "$snap\build\x64-release\conestd.lib"
Push-Location $PSScriptRoot
python "$snap\tools\congo\congo.py" test --release 2>&1 | Select-Object -Last 40
python "$PSScriptRoot\tests\check_encodings.py"
# The mutants that crash the process: the test binary run under each must fail (a nonzero exit; a crash is one)
#   CODEGEN_MUTANT=no-trap-switch   a trap's unwind leaves rsp on the native stack: the first trap (the gate refusing) crashes
#   CODEGEN_MUTANT=gg               no gate thunk: the gate runs on the native stack, and a 64 KiB import at depth 1023 runs out of it
$exe = "$PSScriptRoot\build\release\tests\native\native.exe"
foreach ($m in @("no-trap-switch", "gg")) {
  $env:CODEGEN_MUTANT = $m
  & $exe 2>&1 | Select-Object -Last 2 | Out-Null
  $code = $LASTEXITCODE
  Remove-Item Env:CODEGEN_MUTANT
  if ($code -ne 0) { "mutant ${m}: the test binary fails (exit $code), as it must" } else { "MUTANT ${m} NOT CAUGHT" }
}
Pop-Location
