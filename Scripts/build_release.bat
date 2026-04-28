@echo off
setlocal enabledelayedexpansion
set "ROOT=%~dp0.."

echo [MoonRender Build - Release]

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VS_VER="

if exist "%VSWHERE%" (
    for /f "tokens=*" %%a in ('"%VSWHERE%" -latest -products * -property installationPath 2^>nul') do set "VS_PATH=%%a"
    if defined VS_PATH (
        echo !VS_PATH! | findstr /C:"\18\" >nul && set "VS_VER=2026"
        echo !VS_PATH! | findstr /C:"2022" >nul && set "VS_VER=2022"
    )
)

if not defined VS_VER set "VS_VER=2026"
echo VS version: %VS_VER%

if "%VS_VER%"=="2022" (
    set "CONFIGURE_PRESET=vs2022-release-x64"
    set "BUILD_PRESET=build-vs2022-release-x64"
) else (
    set "CONFIGURE_PRESET=vs2026-release-x64"
    set "BUILD_PRESET=build-vs2026-release-x64"
)

pushd "%ROOT%"
cmake --preset %CONFIGURE_PRESET%
if errorlevel 1 ( popd & exit /b 1 )
cmake --build --preset %BUILD_PRESET%
set "ERR=%ERRORLEVEL%"
popd
exit /b %ERR%
