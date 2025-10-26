@echo off
set PATH=..\tools;..\tools\Python27;..\tools\scripts;%PATH%
set PYTHONPATH=%PYTHONPATH%;..\tools\scripts;..\tools\scripts\conv
call create_byname game_patch_update1
call create_byname update_monster1
call create_byname update_monster2
call create_byname update_monster12
call create_byname update_npc
call create_byname guild
call create_byname item