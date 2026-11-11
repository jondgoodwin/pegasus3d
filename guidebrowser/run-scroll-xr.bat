@echo off
rem The studio build two in the headset (Link active as the OpenXR runtime). Palette on the left hand: grip lifts it off, the pin docks it; trigger presses. Voice as run-one-xr.bat.
set PATH=C:\src\openxr\openxr_loader_windows-1.1.65\x64\bin;C:\libs\SDL3-3.4.16\lib\x64;%PATH%
if not defined CFG set CFG=C:\src\studio-work\config
start "studiovoice" /D C:\src\studio-voice C:\src\studio-voice\build\release\studiovoice.exe --live --bridge C:\src\studio-work\bridge
cd /d C:\src\studio-work\gbscroll
build\release\guidebrowser.exe --xr --world warehouse --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\studio-warehouse.md" --bridge C:\src\studio-work\bridge --desk --config %CFG% %*
