cd /D "%~dp0"

@REM Creating symlinks exe's of binaries

cd auth/
mklink auth.exe C:\m2_rework_project\windows_server\common\binaries\auth.exe
mklink vrunner.exe C:\m2_rework_project\windows_server\common\binaries\vrunner.exe
cd /D "%~dp0" 

cd channel1\game1
mklink game_server.exe C:\m2_rework_project\windows_server\common\binaries\game_server.exe
mklink vrunner.exe C:\m2_rework_project\windows_server\common\binaries\vrunner.exe
cd /D "%~dp0"

cd channel1\game2
mklink game_server.exe C:\m2_rework_project\windows_server\common\binaries\game_server.exe
mklink vrunner.exe C:\m2_rework_project\windows_server\common\binaries\vrunner.exe
cd /D "%~dp0"

cd game99\
mklink game_server.exe C:\m2_rework_project\windows_server\common\binaries\game_server.exe
mklink vrunner.exe C:\m2_rework_project\windows_server\common\binaries\vrunner.exe

cd /D "%~dp0"

cd master\
mklink master.exe C:\m2_rework_project\windows_server\common\binaries\master.exe
mklink vrunner.exe C:\m2_rework_project\windows_server\common\binaries\vrunner.exe

cd /D "%~dp0"

cd db\
mklink db.exe C:\m2_rework_project\windows_server\common\binaries\db.exe
mklink vrunner.exe C:\m2_rework_project\windows_server\common\binaries\vrunner.exe

cd /D "%~dp0"


@


@REM Creating symlinks of channels for the common data

cd /D "%~dp0" @REM Returning to the bash folder

cd channel1\game1
mklink /D data C:\m2_rework_project\windows_server\common\data
cd /D "%~dp0"

cd channel1\game2
mklink /D data C:\m2_rework_project\windows_server\common\data
cd /D "%~dp0"

cd game99\
mklink /D data C:\m2_rework_project\windows_server\common\data
cd /D "%~dp0"

cd db\
mklink /D data C:\m2_rework_project\windows_server\common\data
cd /D "%~dp0"


@REM Creating sylinks of toml files

cd auth\
mklink auth.config.toml C:\m2_rework_project\windows_server\common\binaries\auth.config.toml
cd /D "%~dp0"

@REM Channel's .toml
cd channel1\game1
mklink config.db.toml C:\m2_rework_project\windows_server\common\binaries\config.db.toml
mklink config.global.toml C:\m2_rework_project\windows_server\common\binaries\config.global.toml
cd /D "%~dp0"
cd channel1\game2
mklink config.db.toml C:\m2_rework_project\windows_server\common\binaries\config.db.toml
mklink config.global.toml C:\m2_rework_project\windows_server\common\binaries\config.global.toml
cd /D "%~dp0"
cd game99\
mklink config.db.toml C:\m2_rework_project\windows_server\common\binaries\config.db.toml
mklink config.global.toml C:\m2_rework_project\windows_server\common\binaries\config.global.toml
cd /D "%~dp0"

@REM Db.toml

cd db\
mklink config.db.toml C:\m2_rework_project\windows_server\common\binaries\config.db.toml
cd /D "%~dp0"
cd db\
mklink dbcache.config.toml C:\m2_rework_project\windows_server\common\binaries\dbcache.config.toml
cd /D "%~dp0"

@REM Master .toml

cd master\
mklink master.config.toml C:\m2_rework_project\windows_server\common\binaries\master.config.toml
cd /D "%~dp0"

echo Process completed. Press any key to continue!
pause