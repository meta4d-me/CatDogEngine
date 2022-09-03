@echo off

call ./Engine/Auto/Configs/vs2022/Config.bat

cd "./Engine/Auto/Scripts"
"../Programs/premake5.exe" %BUILD_IDE_NAME%

pause