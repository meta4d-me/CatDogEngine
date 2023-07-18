@echo off

Set ROOT_PATH=%~dp0
Set BUILD_IDE_NAME="vs2019"
Set CMAKE_IDE_FULL_NAME="Visual Studio 16 2019"
Set VS_VERSION=vs2019
call ./Engine/Auto/MakeThirdParty_common.bat