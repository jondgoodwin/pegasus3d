# Build the Pegasus3D browser (release mode) and pack the downloadable Windows zip.
#
#   powershell -File tools\release\release.ps1            (from anywhere; the repo is found from this script's place)
#   powershell -File tools\release\release.ps1 -SkipBuild  (pack what browser\build\release already holds)
#
# Output: <OutDir>\pegasus3d-browser-win64-<short git hash>.zip, and its SHA-256 printed.
#
# Machine paths (the defaults are Jon's machine; override with the parameters):
#   -Compiler   folder holding conec.exe and conestd.lib (the 3D Web compiler snapshot)
#   -Congo      Cone's congo.py (packages come from the Cone checkout it sits in)
#   -Sdl        folder holding SDL3.dll and SDL3.lib, and above it LICENSE.txt (SDL 3.x)
#   -Cone       the Cone checkout (its LICENSE goes in the zip)
#   -VcRedist   folder holding the Visual C++ runtime's vcruntime140.dll (x64), for app-local deployment
#
# What goes in the zip is listed below and nowhere else: nothing is copied by pattern, so a stray file in the
# build folder (or reference images, measuring tools, PDBs, objects) cannot get in.
param(
  [string]$OutDir   = "C:\src\releases",
  [string]$Compiler = "C:\src\cone-wt-3dweb-compiler\build\x64-release",
  [string]$Congo    = "C:\src\cone\tools\congo\congo.py",
  [string]$Sdl      = "C:\libs\SDL3-3.4.16\lib\x64",
  [string]$Cone     = "C:\src\cone",
  [string]$VcRedist = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\x64\Microsoft.VC143.CRT",
  [switch]$SkipBuild
)
$ErrorActionPreference = "Stop"
$repo    = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$browser = Join-Path $repo "browser"
$rel     = Join-Path $browser "build\release"
$tpl     = $PSScriptRoot

$dirty = git -C $repo status --porcelain --untracked-files=no
if ($dirty) { Write-Warning "The working tree has uncommitted changes; the zip's hash names the last commit only." }
$hash = (git -C $repo rev-parse --short=8 HEAD).Trim()

if (-not $SkipBuild) {
  $env:LIB      = "$Sdl;$env:LIB"
  $env:CONEC    = Join-Path $Compiler "conec.exe"
  $env:CONESTD  = Join-Path $Compiler "conestd.lib"
  Push-Location $browser
  try { python $Congo build --release; if ($LASTEXITCODE -ne 0) { throw "congo build failed" } } finally { Pop-Location }
}

# The files, as (source, name inside the zip). Each is checked to exist.
$files = @(
  @{ src = "$rel\browser.exe";                         dst = "browser.exe" },
  @{ src = "$rel\browser.spv";                         dst = "browser.spv" },
  @{ src = "$rel\sdfmesh.spv";                         dst = "sdfmesh.spv" },
  @{ src = "$Sdl\SDL3.dll";                            dst = "SDL3.dll" },
  @{ src = "$VcRedist\vcruntime140.dll";               dst = "vcruntime140.dll" },
  @{ src = "$browser\fonts\Inter-Regular.ttf";         dst = "fonts\Inter-Regular.ttf" },
  @{ src = "$browser\fonts\Inter-SemiBold.ttf";        dst = "fonts\Inter-SemiBold.ttf" },
  @{ src = "$tpl\README.txt";                          dst = "README.txt" },
  @{ src = "$tpl\THIRD-PARTY.txt";                     dst = "THIRD-PARTY.txt" },
  @{ src = "$repo\LICENSE";                            dst = "licenses\pegasus3d-LICENSE.txt" },
  @{ src = "$repo\COPYRIGHT";                          dst = "licenses\pegasus3d-COPYRIGHT.txt" },
  @{ src = "$Cone\LICENSE";                            dst = "licenses\cone-LICENSE.txt" },
  @{ src = "$Cone\COPYRIGHT";                          dst = "licenses\cone-COPYRIGHT.txt" },
  @{ src = (Join-Path (Split-Path $Sdl -Parent | Split-Path -Parent) "LICENSE.txt"); dst = "licenses\SDL3-LICENSE.txt" },
  @{ src = "$browser\fonts\Inter-LICENSE.txt";         dst = "licenses\Inter-OFL-LICENSE.txt" }
)
foreach ($f in $files) { if (-not (Test-Path $f.src)) { throw "missing: $($f.src)" } }

$stage = Join-Path ([IO.Path]::GetTempPath()) "pegasus3d-release-$hash"
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
$root = Join-Path $stage "pegasus3d-browser"
foreach ($f in $files) {
  $to = Join-Path $root $f.dst
  New-Item -ItemType Directory -Force (Split-Path $to -Parent) | Out-Null
  Copy-Item $f.src $to
}
(Get-Content (Join-Path $root "README.txt") -Raw).Replace("@HASH@", $hash) | Set-Content (Join-Path $root "README.txt") -NoNewline

New-Item -ItemType Directory -Force $OutDir | Out-Null
$zip = Join-Path $OutDir "pegasus3d-browser-win64-$hash.zip"
if (Test-Path $zip) { Remove-Item $zip }
Compress-Archive -Path $root -DestinationPath $zip -CompressionLevel Optimal
Remove-Item -Recurse -Force $stage

$sha = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLower()
"zip     $zip"
"sha256  $sha"
"size    $((Get-Item $zip).Length) bytes"
"pegasus3d $hash"
