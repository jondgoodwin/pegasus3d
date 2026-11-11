@echo off
rem The studio warehouse in the headset (Quest over Link, Link the active OpenXR runtime), with the laptop's screen as a panel at a desk spot.
rem Walk to it (left stick); the chip under it ("control laptop") turns the beam into the laptop's pointer; right stick up/down is its wheel.
rem --desk-dry-run prints the input instead of sending it. Extra arguments go on.
set PATH=C:\libs\SDL3-3.4.16\lib\x64;%PATH%
cd /d C:\src\studio-work\gbdesk
build\release\guidebrowser.exe --xr --world warehouse --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\studio-warehouse.md" --desk %*
