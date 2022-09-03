@echo off

Set CURRENT_WORK_DIRECTORY=%~dp0
Set PREMAKE_EXE="%CURRENT_WORK_DIRECTORY%/Engine/Auto/Programs/premake5.exe"
Set CMAKE_EXE="%CURRENT_WORK_DIRECTORY%/Engine/Auto/Programs/CMake/bin/cmake.exe"

rem set variables
call ./Engine/Auto/Configs/vs2019/Config.bat

rem generate
cd Engine
if not exist ThirdPartyProjects\. mkdir ThirdPartyProjects
cd ThirdPartyProjects
set ThirdPartyProjectsPath=%cd%

echo [ BGFX ] Start making project...
rem TODO : let GENie generate projects under ThirdPartyProjectsPath/bgfx
cd ../Source/ThirdParty/bgfx
"../bx/tools/bin/windows/genie.exe" --with-windows=10.0 --with-examples --with-tools %BUILD_IDE_NAME%
rem xcopy ".build" "..\..\..\ThirdPartyProjects\bgfx" /E /H /C /I /Y
cd %ThirdPartyProjectsPath%
echo\

echo [ SDL ] Start making project...
if not exist sdl\. mkdir sdl
cd sdl
%CMAKE_EXE% ../../Source/ThirdParty/sdl -G %CMAKE_IDE_FULL_NAME% -A x64 -DSDL_FORCE_STATIC_VCRT=ON
cd ..
echo\

pause