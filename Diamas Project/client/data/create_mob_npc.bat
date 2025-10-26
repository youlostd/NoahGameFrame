@echo off
set PATH=..\tools;..\tools\Python27;..\tools\scripts;%PATH%
set PYTHONPATH=%PYTHONPATH%;..\tools\scripts;..\tools\scripts\conv
call create_byname monster
call create_byname monster2
call create_byname npc
call create_byname npc2
call create_byname npc_pet
call create_byname npc_mount