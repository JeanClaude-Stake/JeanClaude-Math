# setup-windows.ps1 - Downloads WebView2 SDK and zstd for Windows build
# Run once from the project root: powershell -ExecutionPolicy Bypass -File setup-windows.ps1

$ErrorActionPreference = "Stop"

function Ensure-Dir($path) {
    if (-not (Test-Path $path)) { New-Item -ItemType Directory -Force -Path $path | Out-Null }
}

Write-Host ""
Write-Host "[1/4] Creating libs directories..." -ForegroundColor Cyan
Ensure-Dir "libs\webview2\include"
Ensure-Dir "libs\webview2\x64"
Ensure-Dir "libs\zstd\include"
Ensure-Dir "libs\zstd\lib"

# WebView2 NuGet package
Write-Host "[2/4] Downloading Microsoft.Web.WebView2..." -ForegroundColor Cyan
$wv2Url = "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2"
$wv2Pkg = "libs\_webview2.nupkg"
Invoke-WebRequest -Uri $wv2Url -OutFile $wv2Pkg -UseBasicParsing

Write-Host "      Extracting..." -ForegroundColor Gray
$wv2Zip = "libs\_webview2.zip"
$wv2Tmp = "libs\_webview2_pkg"
Copy-Item $wv2Pkg $wv2Zip -Force
if (Test-Path $wv2Tmp) { Remove-Item $wv2Tmp -Recurse -Force }
Expand-Archive -Force $wv2Zip $wv2Tmp

# Headers
Copy-Item "$wv2Tmp\build\native\include\*" "libs\webview2\include\" -Recurse -Force

# DLL + LIB — search recursively since structure varies between package versions
$wv2Dll = Get-ChildItem $wv2Tmp -Recurse -Filter "WebView2Loader.dll" | Where-Object { $_.FullName -like "*x64*" } | Select-Object -First 1
$wv2Lib = Get-ChildItem $wv2Tmp -Recurse -Filter "WebView2Loader.lib" | Where-Object { $_.FullName -like "*x64*" } | Select-Object -First 1

if (-not $wv2Dll) { $wv2Dll = Get-ChildItem $wv2Tmp -Recurse -Filter "WebView2Loader.dll" | Select-Object -First 1 }
if (-not $wv2Lib) { $wv2Lib = Get-ChildItem $wv2Tmp -Recurse -Filter "WebView2Loader.lib" | Select-Object -First 1 }

if ($wv2Dll) { Copy-Item $wv2Dll.FullName "libs\webview2\x64\WebView2Loader.dll" -Force }
if ($wv2Lib) { Copy-Item $wv2Lib.FullName "libs\webview2\x64\WebView2Loader.lib" -Force }

if (-not $wv2Lib) {
    Write-Host "      WebView2Loader.lib not in package - generating from DLL..." -ForegroundColor Yellow
    $dllPath = (Resolve-Path "libs\webview2\x64\WebView2Loader.dll").Path
    $defPath = "libs\webview2\x64\WebView2Loader.def"
    $libPath = "libs\webview2\x64\WebView2Loader.lib"

    # WebView2Loader.dll exports are stable across all 1.x versions
    $defContent = @"
LIBRARY WebView2Loader
EXPORTS
  CreateCoreWebView2Environment
  CreateCoreWebView2EnvironmentWithOptions
  GetAvailableCoreWebView2BrowserVersionString
  CompareBrowserVersions
"@
    $defContent | Out-File $defPath -Encoding ASCII
    Write-Host "      Written WebView2Loader.def, generating .lib..." -ForegroundColor Gray
    & llvm-dlltool -m i386:x86-64 -D "WebView2Loader.dll" -d $defPath -l $libPath
    if (Test-Path $libPath) {
        Write-Host "      Generated WebView2Loader.lib" -ForegroundColor Green
    } else {
        Write-Host "      ERROR: llvm-dlltool failed" -ForegroundColor Red
    }
}

Write-Host "      Cleaning up..." -ForegroundColor Gray
Remove-Item $wv2Pkg -Force
Remove-Item $wv2Zip -Force
Remove-Item $wv2Tmp -Recurse -Force

# zstd (static lib + headers)
Write-Host "[3/4] Downloading zstd..." -ForegroundColor Cyan

$releaseInfo = Invoke-RestMethod "https://api.github.com/repos/facebook/zstd/releases/latest"
$tag = $releaseInfo.tag_name

# Find the win64 zip asset dynamically from the release assets list
$zstdAsset = $releaseInfo.assets | Where-Object { $_.name -like "*win64*.zip" } | Select-Object -First 1
if (-not $zstdAsset) {
    Write-Host "      No win64 zip found in latest release, trying v1.5.5 fallback..." -ForegroundColor Yellow
    $releaseInfo = Invoke-RestMethod "https://api.github.com/repos/facebook/zstd/releases/tags/v1.5.5"
    $zstdAsset = $releaseInfo.assets | Where-Object { $_.name -like "*win64*.zip" } | Select-Object -First 1
}
$zstdUrl = $zstdAsset.browser_download_url

Write-Host "      Latest zstd: $tag  ->  $($zstdAsset.name)" -ForegroundColor Gray
$zstdZip = "libs\_zstd.zip"
Invoke-WebRequest -Uri $zstdUrl -OutFile $zstdZip -UseBasicParsing

Write-Host "      Extracting..." -ForegroundColor Gray
$zstdTmp = "libs\_zstd_pkg"
if (Test-Path $zstdTmp) { Remove-Item $zstdTmp -Recurse -Force }
Expand-Archive -Force $zstdZip $zstdTmp

$zstdRoot = (Get-ChildItem $zstdTmp -Directory | Select-Object -First 1).FullName

if (Test-Path "$zstdRoot\include") {
    Copy-Item "$zstdRoot\include\*" "libs\zstd\include\" -Recurse -Force
} else {
    Copy-Item "$zstdRoot\*.h" "libs\zstd\include\" -Force
}

$libFiles = Get-ChildItem "$zstdRoot" -Recurse -Include "*.lib","*.dll"
foreach ($f in $libFiles) {
    Copy-Item $f.FullName "libs\zstd\lib\" -Force
}
# Rename static lib to zstd.lib so -lzstd resolves correctly with clang++
$staticLib = Get-ChildItem "libs\zstd\lib" -Filter "*static*.lib" | Select-Object -First 1
if ($staticLib -and $staticLib.Name -ne "zstd.lib") {
    Copy-Item $staticLib.FullName "libs\zstd\lib\zstd.lib" -Force
}

Write-Host "      Cleaning up..." -ForegroundColor Gray
Remove-Item $zstdZip -Force
Remove-Item $zstdTmp -Recurse -Force

# Verify
Write-Host "[4/4] Verifying files..." -ForegroundColor Cyan

$checks = @(
    @{ Path = "libs\webview2\include\WebView2.h";     Label = "WebView2.h" },
    @{ Path = "libs\webview2\x64\WebView2Loader.lib"; Label = "WebView2Loader.lib" },
    @{ Path = "libs\webview2\x64\WebView2Loader.dll"; Label = "WebView2Loader.dll" }
)

$zstdHeader = Get-ChildItem "libs\zstd\include" -Filter "zstd.h" -ErrorAction SilentlyContinue | Select-Object -First 1
$checks += @{ Path = "libs\zstd\include\zstd.h"; Label = "zstd.h" }
$checks += @{ Path = "libs\zstd\lib\zstd.lib";   Label = "zstd.lib (renamed)" }

$ok = $true
foreach ($c in $checks) {
    if ($c.Path -and (Test-Path $c.Path)) {
        Write-Host "  [OK] $($c.Label)" -ForegroundColor Green
    } else {
        Write-Host "  [MISSING] $($c.Label)" -ForegroundColor Red
        $ok = $false
    }
}

if ($ok) {
    Write-Host ""
    Write-Host "Setup complete! Run 'make' to build." -ForegroundColor Green
    Write-Host ""
    Write-Host "NOTE: copy WebView2Loader.dll next to the .exe before running:" -ForegroundColor Yellow
    Write-Host "  copy libs\webview2\x64\WebView2Loader.dll ." -ForegroundColor Yellow
    Write-Host ""
} else {
    Write-Host ""
    Write-Host "Some files are missing - check errors above." -ForegroundColor Red
    Write-Host ""
    exit 1
}
