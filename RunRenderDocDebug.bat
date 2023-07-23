@echo off

set "file_path=MoonRender.exe"

if exist "%file_path%" (
    del "%file_path%"
    echo File deleted.
) else (
    echo File not found.
)

set "source_folder=x64\Debug"
set "file_name=MoonRender.exe"
set "destination_folder=%cd%"

xcopy /y "%source_folder%\%file_name%" "%destination_folder%"

"D:\Program Files (x86)\DevTool\RenderDoc\qrenderdoc.exe" capture MoonRender.exe