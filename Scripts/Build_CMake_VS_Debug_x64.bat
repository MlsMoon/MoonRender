@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0.."
set "CONFIG=Debug"
if /I "%~1"=="Release" set "CONFIG=Release"
if /I "%~1"=="RelWithDebInfo" set "CONFIG=RelWithDebInfo"
if /I "%~1"=="MinSizeRel" set "CONFIG=MinSizeRel"

echo [MoonRender Build]
echo Configuration: %CONFIG%

set "VS_VER="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if exist "%VSWHERE%" (
    for /f "tokens=*" %%a in ('"%VSWHERE%" -latest -products * -property installationPath 2^>nul') do set "VS_PATH=%%a"
    if defined VS_PATH (
        echo Detected VS installation: !VS_PATH!
        echo !VS_PATH! | findstr /C:"\18\" >nul
        if !errorlevel!==0 set "VS_VER=2026"
        echo !VS_PATH! | findstr /C:"2022" >nul
        if !errorlevel!==0 set "VS_VER=2022"
    )
)

if not defined VS_VER (
    if exist "%ProgramFiles%\Microsoft Visual Studio\18" (
        set "VS_VER=2026"
    ) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022" (
        set "VS_VER=2022"
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022" (
        set "VS_VER=2022"
    ) else (
        echo WARNING: Could not detect VS version, defaulting to VS 2026
        set "VS_VER=2026"
    )
)

echo Detected Visual Studio: %VS_VER%

if "%VS_VER%"=="2026" (
    set "PRESET_PREFIX=vs2026"
) else if "%VS_VER%"=="2022" (
    set "PRESET_PREFIX=vs2022"
) else (
    echo WARNING: Unknown VS version '%VS_VER%', defaulting to VS 2026
    set "PRESET_PREFIX=vs2026"
)

if /I "%CONFIG%"=="Debug" (
    set "CONFIGURE_PRESET=%PRESET_PREFIX%-debug-x64"
    set "BUILD_PRESET=build-%PRESET_PREFIX%-debug-x64"
) else (
    set "CONFIGURE_PRESET=%PRESET_PREFIX%-release-x64"
    set "BUILD_PRESET=build-%PRESET_PREFIX%-release-x64"
)

set "BINARY_DIR=%ROOT%\Builds\CMake\%CONFIGURE_PRESET%"

echo Using configure preset: %CONFIGURE_PRESET%
echo Using build preset: %BUILD_PRESET%
echo Build directory: %BINARY_DIR%

pushd "%ROOT%"

if exist "%BINARY_DIR%\CMakeCache.txt" (
    findstr /C:"CMAKE_GENERATOR:INTERNAL=Visual Studio" "%BINARY_DIR%\CMakeCache.txt" >nul
    if !errorlevel!==0 (
        findstr /C:"%PRESET_PREFIX%" "%BINARY_DIR%\CMakeCache.txt" >nul
        if !errorlevel!==1 (
            echo Generator mismatch detected. Cleaning old build directory...
            rmdir /S /Q "%BINARY_DIR%"
        )
    )
)

cmake --preset %CONFIGURE_PRESET%
if errorlevel 1 (
    echo ERROR: CMake configure failed.
    popd
    exit /b 1
)

cmake --build --preset %BUILD_PRESET%
set "ERR=%ERRORLEVEL%"
popd

exit /b %ERR%
