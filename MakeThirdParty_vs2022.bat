@echo off

Set ROOT_PATH=%~dp0
Set BUILD_IDE_NAME="vs2022"
Set CMAKE_IDE_FULL_NAME="Visual Studio 17 2022"
Set VS_VERSION=vs2022
call ./Engine/Auto/MakeThirdParty_common.bat