@echo off

Set ROOT_PATH=%~dp0
Set PREMAKE_EXE="%ROOT_PATH%/Engine/Auto/Programs/premake5.exe"
Set CMAKE_EXE="%ROOT_PATH%/Engine/Auto/Programs/CMake/bin/cmake.exe"

rem set variables
call ./Engine/Auto/Configs/vs2019/Config.bat

rem generate
cd Engine/Source/ThirdParty
set ThirdPartyProjectsPath=%cd%
echo %ThirdPartyProjectsPath%

echo [ BGFX ] Start making project...
cd bgfx
"../bx/tools/bin/windows/genie.exe" --with-windows=10.0 --with-examples --with-tools %BUILD_IDE_NAME%
cd %ThirdPartyProjectsPath%
echo\

echo [ SDL ] Start making project...
cd sdl
if not exist build mkdir build
cd build
%CMAKE_EXE% .. -G %CMAKE_IDE_FULL_NAME% -DSDL_FORCE_STATIC_VCRT=ON
%CMAKE_EXE% --build .
cd ..
echo\

pause