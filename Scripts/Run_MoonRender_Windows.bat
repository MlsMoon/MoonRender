@echo off
setlocal

set "ROOT=%~dp0.."
set "CONFIG=Debug"
if /I "%~1"=="Release" set "CONFIG=Release"
if /I "%~1"=="RelWithDebInfo" set "CONFIG=RelWithDebInfo"
if /I "%~1"=="MinSizeRel" set "CONFIG=MinSizeRel"

set "EXE_PATH=%ROOT%\Builds\CMakeOutput\Bin\%CONFIG%\MoonRender.exe"

echo [MoonRender Run]
echo Configuration: %CONFIG%
echo Expected path: %EXE_PATH%

if not exist "%EXE_PATH%" (
    echo ERROR: MoonRender.exe not found.
    echo.
    echo Please build the project first:
    echo   Scripts\Build_CMake_VS_Debug_x64.bat %CONFIG%
    exit /b 1
)

pushd "%ROOT%"
echo Starting MoonRender...
start "" "%EXE_PATH%"
popd

exit /b 0
