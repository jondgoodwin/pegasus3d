@echo off
rem The Incubator on the desktop (config, bridge and line under C:\src\incubator). Add --desk for the laptop-screen panel. Extra arguments go on.
set PATH=C:\libs\SDL3-3.4.16\lib\x64;%PATH%
cd /d C:\src\incubator\build-copy2
build\release\guidebrowser.exe --world incubator --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\incubator-world.md" --config C:\src\incubator\config --bridge C:\src\incubator\bridge %*
