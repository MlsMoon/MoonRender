@echo off
setlocal
set "ROOT=%~dp0.."
set "EXE_PATH=%ROOT%\Builds\CMakeOutput\Bin\Debug\MoonRender.exe"

echo [MoonRender Run - Debug]
echo Path: %EXE_PATH%

if not exist "%EXE_PATH%" (
    echo ERROR: MoonRender.exe not found.
    echo Build first: Scripts\build_debug.bat
    exit /b 1
)

pushd "%ROOT%"
start "" "%EXE_PATH%"
popd
exit /b 0
