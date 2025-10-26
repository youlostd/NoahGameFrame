
REM cd /D "%~dp0"
REM cd mysql-windows/bin/
REM start mysqld
REM timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd db/
start vrunner.exe --file=db.exe --pid-path=pid.txt
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd master/
start vrunner.exe --file=master.exe --pid-path=pid.txt
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd auth\
start vrunner.exe --file=auth.exe --pid-path=pid.txt
timeout /t 1 /nobreak > nul

cd /D "%~dp0" 
cd channel1/game1
start vrunner.exe --file=game_server.exe --pid-path=pid.txt
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd channel1/game2
start vrunner.exe --file=game_server.exe --pid-path=pid.txt
timeout /t 1 /nobreak > nul

cd /D "%~dp0"
cd game99/
start vrunner.exe --file=game_server.exe --pid-path=pid.txt

echo Start completed with vrunner.
pause