# Builds the studio-desktop worktree's guidebrowser from a copy (C:\src\studio-work\gbdesk) with the headset Cone (cone-wt-headset)
robocopy "C:\src\pegasus3d-wt-incubatordesk\guidebrowser" "C:\src\studio-work\gbdesk" /MIR /XD build out /XF run-*.bat /NFL /NDL /NJH /NJS | Out-Null
& "C:\src\pegasus3d-wt-incubatordesk\guidebrowser\shim\build-shim.ps1" -Out "C:\src\studio-work\gbdesk\shim\out" | Select-Object -Last 3
$env:LIB = "$env:LIB;C:\libs\SDL3-3.4.16\lib\x64;C:\src\cone\deps\openssl;C:\src\cone\deps\ngtcp2"
$env:PATH = "$env:PATH;C:\libs\SDL3-3.4.16\lib\x64"
$env:CONEC = "C:\src\cone-wt-headset\build\x64-release\conec.exe"
$env:CONESTD = "C:\src\cone-wt-headset\build\x64-release\conestd.lib"
$env:CONE_DEPS = "C:\src\cone\deps"
Push-Location "C:\src\studio-work\gbdesk"
python "C:\src\cone-wt-headset\tools\congo\congo.py" build --release 2>&1 | Select-Object -Last 60
Pop-Location
