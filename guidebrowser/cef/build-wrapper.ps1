# Builds CEF's libcef_dll_wrapper (static, /MD) once into C:\src\cef\wrapper-build
$cef = "C:\src\cef\cef_binary_156.0.28+g57389f0+chromium-156.0.8078.88_windows64_minimal"
$out = "C:\src\cef\wrapper-build"
$bat = "$env:TEMP\cefwrap.bat"
@"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -S "$cef" -B "$out" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCEF_RUNTIME_LIBRARY_FLAG=/MD -DUSE_SANDBOX=OFF
cmake --build "$out" --target libcef_dll_wrapper
"@ | Set-Content $bat -Encoding ascii
cmd /c $bat 2>&1 | Select-Object -Last 25
