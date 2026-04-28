@echo off
setlocal

set "ROOT=%~dp0.."

for %%D in ("%ROOT%\build-vs" "%ROOT%\build-ninja" "%ROOT%\x64" "%ROOT%\MoonRender" "%ROOT%\Cache") do (
    if exist "%%~fD" (
        echo Removing %%~fD
        rmdir /s /q "%%~fD"
    )
)
