@echo off
rem The scroll-now build (the picture follows the scroll at once), on the desktop. Voice as run-one.bat: F2 record/pause, F3 send. Your config folder C:\src\studio-work\config. Extra arguments go on.
set PATH=C:\libs\SDL3-3.4.16\lib\x64;%PATH%
if not defined CFG set CFG=C:\src\studio-work\config
start "studiovoice" /D C:\src\studio-voice C:\src\studio-voice\build\release\studiovoice.exe --live --bridge C:\src\studio-work\bridge
cd /d C:\src\studio-work\gbscroll
build\release\guidebrowser.exe --world warehouse --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\studio-warehouse.md" --bridge C:\src\studio-work\bridge --desk --config %CFG% %*
