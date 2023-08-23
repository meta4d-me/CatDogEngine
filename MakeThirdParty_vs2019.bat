@echo off

Set ROOT_PATH=%~dp0
Set BGFX_BUILD_OPTIONS=vs2019
Set CMAKE_IDE_FULL_NAME="Visual Studio 16 2019"

rem VS_VERSION will be used inside assimp makefile
Set VS_VERSION=vs2019

call ./Engine/Auto/MakeThirdParty_Windows.bat