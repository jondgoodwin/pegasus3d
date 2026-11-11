@echo off
rem The same in Meta's XR simulator (no headset, no voice process, no real input). Arguments go on, e.g. --frames 300
set XR_RUNTIME_JSON=C:\src\meta-xr-simulator\extracted\PFiles\MetaXRSimulator\v207.1\meta_openxr_simulator.json
set META_XRSIM_CONFIG_JSON=C:\src\seaground-work\sim\simcfg.json
set PEGASUS_XR_FAKE=1
set PATH=C:\src\openxr\openxr_loader_windows-1.1.65\x64\bin;C:\libs\SDL3-3.4.16\lib\x64;%PATH%
if not defined CFG set CFG=C:\src\studio-work\config
cd /d C:\src\studio-work\gbscroll
build\release\guidebrowser.exe --xr --headless --world warehouse --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\studio-warehouse.md" --desk --desk-dry-run --config %CFG% %*
