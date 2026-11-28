# The test of the check itself: with NATIVECHECK_MUTANT=add-to-sub (one f64.add lifted as a subtraction) nativecheck must
# exit 1; without it, on the same worlds, it must exit 0. Exits 0 only if both hold.
$exe = "$PSScriptRoot\build\release\nativecheck.exe"
$root = Resolve-Path "$PSScriptRoot\..\.."
$world = "$root\guidebrowser\worlds\guide2.ddd"
$env:NATIVECHECK_MUTANT = "add-to-sub"
& $exe $world | Out-Null
$mutant = $LASTEXITCODE
Remove-Item Env:NATIVECHECK_MUTANT
& $exe $world | Out-Null
$clean = $LASTEXITCODE
"with the fault in the lift: exit $mutant (must be 1); without: exit $clean (must be 0)"
# the stack fault: a runner whose native stack is sized for half the depth stops the fat-frame recursion early
$env:NATIVECHECK_MUTANT = "half-stack"
& $exe $world | Out-Null
$half = $LASTEXITCODE
Remove-Item Env:NATIVECHECK_MUTANT
"with the stack sized for half the depth: exit $half (must be 1)"
if ($mutant -eq 1 -and $clean -eq 0 -and $half -eq 1) { "nativecheck catches a real difference, and a stack too small for the depth limit"; exit 0 }
"SELF-TEST FAILED"; exit 1
