@echo off
setlocal

set "ROOT=%~dp0.."

pushd "%ROOT%"
cmake --preset vs-debug-x64
if errorlevel 1 (
    popd
    exit /b 1
)

cmake --build --preset build-vs-debug-x64
set "ERR=%ERRORLEVEL%"
popd

exit /b %ERR%
