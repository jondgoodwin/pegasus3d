# Runs nativecheck over every guide world (the names in tools\mesh-hashes, from guidebrowser\worlds) and every world in
# pegasus\worlds; exits 1 on any difference. Extra arguments are passed on (a world file instead of the default list).
#   tools\nativecheck\run.ps1                         the merge gate
#   $env:NATIVECHECK_MUTANT = "add-to-sub"            the test of the check itself: one FADD lifted as an FSUB, so it must exit 1
$root = Resolve-Path "$PSScriptRoot\..\.."
$exe = "$PSScriptRoot\build\release\nativecheck.exe"
$files = @()
if ($args.Count -gt 0) { $files = $args }
else {
  foreach ($f in Get-ChildItem "$root\tools\mesh-hashes\*.txt" | Sort-Object Name) {
    $w = "$root\guidebrowser\worlds\$($f.BaseName).ddd"
    if (Test-Path $w) { $files += $w }
  }
  foreach ($f in Get-ChildItem "$root\pegasus\worlds\*.ddd" | Sort-Object Name) { $files += $f.FullName }
}
& $exe @files
exit $LASTEXITCODE
