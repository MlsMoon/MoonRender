@echo off
setlocal
set "ROOT=%~dp0.."
set "APP=%ROOT%\Builds\CMakeOutput\Bin\Release\MoonRender.exe"
set "RENDERDOC=%RENDERDOC_EXE%"

if not defined RENDERDOC (
    for %%I in (qrenderdoc.exe) do set "RENDERDOC=%%~$PATH:I"
)

if not defined RENDERDOC (
    set "RENDERDOC=D:\Program Files (x86)\DevTool\RenderDoc\qrenderdoc.exe"
)

if not exist "%APP%" (
    echo App not found: %APP%
    echo Build first: Scripts\build_release.bat
    exit /b 1
)

if not exist "%RENDERDOC%" (
    echo RenderDoc not found: %RENDERDOC%
    echo Set RENDERDOC_EXE or add qrenderdoc.exe to PATH.
    exit /b 1
)

pushd "%ROOT%"
"%RENDERDOC%" capture "%APP%"
set "ERR=%ERRORLEVEL%"
popd
exit /b %ERR%
