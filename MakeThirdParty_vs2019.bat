@echo off

Set ROOT_PATH=%~dp0
call ./Engine/Auto/Configs/vs2019/Config.bat
Set VS_VERSION=vs2019
call ./Engine/Auto/MakeThirdParty_common.bat