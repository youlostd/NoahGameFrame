
REM cd /D "%~dp0"
REM cd mysql-windows/bin/
REM start mysqld
REM timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd db/
start db.exe
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd master/
start master.exe
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd auth\
start auth.exe
timeout /t 1 /nobreak > nul

cd /D "%~dp0" 
cd channel1/game1
start game_server.exe
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd channel1/game2
start game_server.exe
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd game99/
start game_server.exe

echo Start completed.
pause