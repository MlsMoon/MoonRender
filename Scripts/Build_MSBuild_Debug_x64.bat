@echo off
setlocal

set "ROOT=%~dp0.."
set "MSBUILD=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"

if not exist "%MSBUILD%" (
    echo MSBuild not found:
    echo   %MSBUILD%
    exit /b 1
)

pushd "%ROOT%"
"%MSBUILD%" "MoonRender.sln" /t:Build /p:Configuration=Debug /p:Platform=x64
set "ERR=%ERRORLEVEL%"
popd

exit /b %ERR%
