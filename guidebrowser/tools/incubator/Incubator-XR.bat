@echo off
rem The Incubator in the headset (Quest over Link, Link the active OpenXR runtime), with studiovoice (--live opens the mic and speaker) and the bridge.
set PATH=C:\libs\SDL3-3.4.16\lib\x64;C:\src\openxr\openxr_loader_windows-1.1.65\x64\bin;%PATH%
start "studiovoice" /D C:\src\studio-voice C:\src\incubator\voice\build\release\studiovoice.exe --live --bridge C:\src\incubator\bridge
cd /d C:\src\incubator\build-copy
build\release\guidebrowser.exe --xr --world incubator --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\incubator-world.md" --config C:\src\incubator\config --bridge C:\src\incubator\bridge --desk %*
