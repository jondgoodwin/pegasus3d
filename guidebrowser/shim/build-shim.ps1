# Builds deskshim.dll and deskshim.lib with MSVC into the folder given (default: 'out' beside this script)
param([string]$Out = "$PSScriptRoot\out")
New-Item -ItemType Directory -Force $Out | Out-Null
$vc = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmd /c "`"$vc`" >nul && cl /nologo /O2 /LD /W3 `"$PSScriptRoot\deskshim.c`" /Fo`"$Out\\`" /Fe`"$Out\deskshim.dll`" /link /IMPLIB:`"$Out\deskshim.lib`" d3d11.lib dxgi.lib dxguid.lib user32.lib"
