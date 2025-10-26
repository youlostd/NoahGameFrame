@echo off
set PATH=..\tools;..\tools\Python27;..\tools\scripts;%PATH%
set PYTHONPATH=%PYTHONPATH%;..\tools\scripts;..\tools\scripts\conv
call create_byname pc_warrior
call create_byname pc_shaman
call create_byname pc_sura
call create_byname pc_assassin
call create_byname pc2_warrior
call create_byname pc2_sura
call create_byname pc2_shaman
call create_byname pc2_assassin
call create_byname pc_ridack
call create_byname pc_plechito