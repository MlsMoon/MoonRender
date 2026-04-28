@echo off
setlocal
set "ROOT=%~dp0.."
set "EXE_PATH=%ROOT%\Builds\CMakeOutput\Bin\Release\MoonRender.exe"

echo [MoonRender Run - Release]
echo Path: %EXE_PATH%

if not exist "%EXE_PATH%" (
    echo ERROR: MoonRender.exe not found.
    echo Build first: Scripts\build_release.bat
    exit /b 1
)

pushd "%ROOT%"
start "" "%EXE_PATH%"
popd
exit /b 0
