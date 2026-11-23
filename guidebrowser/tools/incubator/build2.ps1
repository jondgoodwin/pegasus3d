# Builds the incubator worktree's guidebrowser from a copy (C:\src\incubator\build-copy2) with the headset Cone (read only)
robocopy "C:\src\pegasus3d-wt-incubator\guidebrowser" "C:\src\incubator\build-copy2" /MIR /XD build out cache bin /XF run-*.bat /NFL /NDL /NJH /NJS | Out-Null
New-Item -ItemType Directory -Force "C:\src\incubator\build-copy2\cef\bin" | Out-Null
Copy-Item "C:\src\pegasus3d-wt-incubator\guidebrowser\cef\bin\*" "C:\src\incubator\build-copy2\cef\bin" -Force
& "C:\src\pegasus3d-wt-incubator\guidebrowser\shim\build-shim.ps1" -Out "C:\src\incubator\build-copy2\shim\out" | Select-Object -Last 3
$env:LIB = "$env:LIB;C:\libs\SDL3-3.4.16\lib\x64;C:\src\cone\deps\openssl;C:\src\cone\deps\ngtcp2"
$env:PATH = "$env:PATH;C:\libs\SDL3-3.4.16\lib\x64"
$env:CONEC = "C:\src\cone-wt-headset\build\x64-release\conec.exe"
$env:CONESTD = "C:\src\cone-wt-headset\build\x64-release\conestd.lib"
$env:CONE_DEPS = "C:\src\cone\deps"
Push-Location "C:\src\incubator\build-copy2"
python "C:\src\cone-wt-headset\tools\congo\congo.py" build --release 2>&1 | Select-Object -Last 60
Pop-Location
$cef = "C:\src\cef\cef_binary_156.0.28+g57389f0+chromium-156.0.8078.88_windows64_minimal"
$out = "C:\src\incubator\build-copy2\build\release"
if (Test-Path $out) {
Copy-Item "$cef\Release\*" $out -Recurse -Force -Exclude bootstrap.exe,bootstrapc.exe,libcef.lib
Copy-Item "$cef\Resources\*" $out -Recurse -Force
Copy-Item "C:\src\incubator\build-copy2\cef\bin\cefhelper.exe" $out -Force
Copy-Item "C:\src\incubator\build-copy2\shim\out\deskshim.dll" $out -Force
}
