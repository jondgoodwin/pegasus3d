@echo off
rem The incubator incubator on the desktop, with the laptop's own screen as a panel at a desk spot (right of the start, behind the ring).
rem Press L (or click the chip under the panel) to let the mouse control the laptop; off at the start. --desk-dry-run prints the input instead.
rem Runs the copy built by C:\src\studio-work\gbdesk-tools\build-desk.ps1 (the studio-desktop worktree). Extra arguments go on.
set PATH=C:\libs\SDL3-3.4.16\lib\x64;%PATH%
cd /d C:\src\studio-work\gbdesk
build\release\guidebrowser.exe --world incubator --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\incubator-world.md" --desk %*
