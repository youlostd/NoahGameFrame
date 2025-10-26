@echo off
set PATH=..\tools;..\tools\Python27;..\tools\scripts;%PATH%
set PYTHONPATH=%PYTHONPATH%;..\tools\scripts;..\tools\scripts\conv
call create_byname bgm
call create_byname effect
call create_byname guild
call create_byname item
call create_byname icon
call create_byname environment
call create_byname map
call create_byname sound
call create_byname special
call create_byname terrain
call create_byname textureset
call create_byname tree
call create_byname ui
call create_byname zone