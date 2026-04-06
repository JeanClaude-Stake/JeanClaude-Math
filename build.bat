@echo off
setlocal

:: Find vcvarsall.bat via vswhere (works for VS 2017+)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VCVARS="

if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do (
        set "VCVARS=%%i\VC\Auxiliary\Build\vcvarsall.bat"
    )
)

:: Fallback: scan common BuildTools paths
if not defined VCVARS (
    for /d %%i in ("%ProgramFiles(x86)%\Microsoft Visual Studio\*\BuildTools") do (
        if exist "%%i\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%%i\VC\Auxiliary\Build\vcvarsall.bat"
    )
)
if not defined VCVARS (
    for /d %%i in ("%ProgramFiles(x86)%\Microsoft Visual Studio\*\Community") do (
        if exist "%%i\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%%i\VC\Auxiliary\Build\vcvarsall.bat"
    )
)
if not defined VCVARS (
    for /d %%i in ("%ProgramFiles(x86)%\Microsoft Visual Studio\*\Professional") do (
        if exist "%%i\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=%%i\VC\Auxiliary\Build\vcvarsall.bat"
    )
)

if not defined VCVARS (
    echo [ERROR] Could not find vcvarsall.bat. Install Visual Studio Build Tools.
    exit /b 1
)

echo [build] Using: %VCVARS%
call "%VCVARS%" amd64 >nul

echo [build] Running make %*
make %*
