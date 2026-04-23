@echo off
setlocal

set "ROOT=%~dp0.."
set "CACHE_DIR=%ROOT%\Builds\Cache\CSO"

if exist "%CACHE_DIR%" (
    echo Removing shader cache:
    echo   %CACHE_DIR%
    rmdir /s /q "%CACHE_DIR%"
) else (
    echo Shader cache not found:
    echo   %CACHE_DIR%
)
