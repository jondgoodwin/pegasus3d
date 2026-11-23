# studiovoice (with the edit lane's dictate mode: C:\src\studio-voice\src as it stands) built from a copy into C:\src\incubator\voice,
# so the exe the running studio uses (C:\src\studio-voice\build) is untouched
robocopy "C:\src\studio-voice\src" "C:\src\incubator\voice\src" /MIR /NFL /NDL /NJH /NJS | Out-Null
Copy-Item "C:\src\studio-voice\congo.toml" "C:\src\incubator\voice\congo.toml" -Force
$env:LIB = "$env:LIB;C:\libs\SDL3-3.4.16\lib\x64;C:\src\cone\deps\openssl"
$env:PATH = "$env:PATH;C:\libs\SDL3-3.4.16\lib\x64"
$env:CONEC = "C:\src\cone-wt-headset\build\x64-release\conec.exe"
$env:CONESTD = "C:\src\cone-wt-headset\build\x64-release\conestd.lib"
$env:CONE_DEPS = "C:\src\cone\deps"
Push-Location "C:\src\incubator\voice"
python "C:\src\cone-wt-headset\tools\congo\congo.py" build --release 2>&1 | Select-String -Pattern "Error|-->|\^---|Unsuccessful|Finished" | Select-Object -First 20
Pop-Location
