@echo off

Set ROOT_PATH=%~dp0
call ./Engine/Auto/Configs/vs2022/Config.bat
call ./Engine/Auto/MakeThirdParty_common.bat