@echo off

Set ROOT_PATH=%~dp0
Set BGFX_BUILD_OPTIONS=--vs=vs2022-clang vs2022
Set CMAKE_IDE_FULL_NAME="Visual Studio 17 2022"

rem VS_VERSION will be used inside assimp makefile
Set VS_VERSION=vs2022

rem Enable Clang toolsets
Set USE_CLANG_TOOLSET=1
Set CMAKE_TOOLSET_OPTION=-T ClangCL

Set CD_BUILD_TYPE=Debug
call "./Engine/Auto/MakeThirdParty_Windows.bat"

Set CD_BUILD_TYPE=Release
call "./Engine/Auto/MakeThirdParty_Windows.bat"

pause