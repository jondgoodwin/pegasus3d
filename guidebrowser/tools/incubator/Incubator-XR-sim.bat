@echo off
rem The Incubator in Meta's XR simulator: no headset, no real input, no voice. Arguments go on, e.g. --xr-shots PREFIX
set XR_RUNTIME_JSON=C:\src\meta-xr-simulator\extracted\PFiles\MetaXRSimulator\v207.1\meta_openxr_simulator.json
set META_XRSIM_CONFIG_JSON=C:\src\seaground-work\sim\simcfg.json
set PEGASUS_XR_FAKE=1
set PATH=C:\src\openxr\openxr_loader_windows-1.1.65\x64\bin;C:\libs\SDL3-3.4.16\lib\x64;%PATH%
cd /d C:\src\incubator\build-copy
build\release\guidebrowser.exe --xr --headless --world incubator --doc "C:\Users\jondg\MyDrive\Incubator\3D Web\WI\incubator-world.md" --config C:\src\incubator\config --bridge C:\src\incubator\bridge --desk --desk-dry-run %*
