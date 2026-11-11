# Builds cefshim.dll (+ import lib) and cefhelper.exe into guidebrowser\cef\bin (CEF's own runtime files are copied
# beside the studio's exe by run-web's build script). Needs the wrapper first (build-wrapper.ps1).
$cef = "C:\src\cef\cef_binary_156.0.28+g57389f0+chromium-156.0.8078.88_windows64_minimal"
$wrap = "C:\src\cef\wrapper-build\libcef_dll_wrapper\libcef_dll_wrapper.lib"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$bin = "$here\bin"
New-Item -ItemType Directory -Force $bin | Out-Null
$bat = "$env:TEMP\cefshim.bat"
@"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "$bin"
cl /nologo /std:c++20 /O2 /MD /EHsc /DWIN32 /D_WINDOWS /DUNICODE /D_UNICODE /DNOMINMAX /DWRAPPING_CEF_SHARED /I"$cef" "$here\src\cefshim.cc" /LD /Fe:cefshim.dll /link "$wrap" "$cef\Release\libcef.lib" user32.lib
cl /nologo /std:c++20 /O2 /MD /EHsc /DWIN32 /D_WINDOWS /DUNICODE /D_UNICODE /DNOMINMAX /DWRAPPING_CEF_SHARED /I"$cef" "$here\src\cefhelper.cc" /Fe:cefhelper.exe /link /SUBSYSTEM:WINDOWS "$wrap" "$cef\Release\libcef.lib" user32.lib
"@ | Set-Content $bat -Encoding ascii
cmd /c $bat 2>&1 | Select-Object -Last 30
