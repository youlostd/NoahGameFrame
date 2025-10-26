@echo off
set PATH=..\tools;..\tools\Python27;..\tools\scripts;%PATH%
set PYTHONPATH=%PYTHONPATH%;..\tools\scripts;..\tools\scripts\conv

python -m make_property_config source > source/property/config.txt
echo Property config created
call create_byname property
pause