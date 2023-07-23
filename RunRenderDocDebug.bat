@echo off
set "source_folder=x64\Debug"
set "file_name=MoonRender.exe"
set "destination_folder=%cd%"

xcopy /y "%source_folder%\%file_name%" "%destination_folder%"

"D:\Program Files (x86)\DevTool\RenderDoc\qrenderdoc.exe" capture MoonRender.exe