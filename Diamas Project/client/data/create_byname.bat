@echo off
set PATH=..\tools;..\tools\Python27;..\tools\scripts;%PATH%
set PYTHONPATH=%PYTHONPATH%;..\tools\scripts;..\tools\scripts\conv

if %1!==! goto build_with_param
set arg1=%1
"archiver" -X make_xml/%1_create.xml
"archiver" -X xml/%1_create.xml
echo Done
goto end
:build_with_param
set /p id="Enter File to make: " %=%
"archiver" -X make_xml/%id%_create.xml
"archiver" -X xml/%id%_create.xml
echo Done press any button to continue
pause >nul
cls
goto build_with_param
:end
echo Finished Execution