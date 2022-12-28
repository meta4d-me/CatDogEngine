@echo off

Set ROOT_PATH=%~dp0
call ./Engine/Auto/Configs/vs2022/Config.bat
Set VS_VERSION=vs2022
call ./Engine/Auto/MakeThirdParty_common.bat