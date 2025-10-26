@echo off
set PATH=..\tools;..\tools\Python27;..\tools\scripts;%PATH%
set PYTHONPATH=%PYTHONPATH%;..\tools\scripts;..\tools\scripts\conv
call make_xml
call create_property

python -m create_all

echo Created all paks
pause
