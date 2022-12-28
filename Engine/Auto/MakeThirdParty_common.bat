@echo off

Set PREMAKE_EXE="%ROOT_PATH%/Engine/Auto/Programs/premake5.exe"
Set CMAKE_EXE=%ROOT_PATH%/Engine/Auto/Programs/CMake/bin/cmake.exe

rem Find msbuild.exe through vswhere.exe. For some make systems, we need to do vswhere by ourselves.
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe') do (
	SET MSBUILD_PATH=%%i
)
SET MSBUILD_PATH=%MSBUILD_PATH:\=/%
SET MSBUILD_FOLDER=%MSBUILD_PATH:MsBuild.exe=%

rem Generate
cd Engine/Source/ThirdParty
set ThirdPartyProjectsPath=%cd%
echo %ThirdPartyProjectsPath%

echo [ BGFX ] Start making project...
cd bgfx
"../bx/tools/bin/windows/genie" --with-windows=10.0 --with-examples --with-tools %BUILD_IDE_NAME%
cd %ThirdPartyProjectsPath%
echo\

echo [ SDL ] Start making project...
cd sdl
if not exist build mkdir build
cd build
%CMAKE_EXE% .. -G %CMAKE_IDE_FULL_NAME% -DSDL_FORCE_STATIC_VCRT=ON -D CMAKE_CONFIGURATION_TYPES="Debug;Release"
start /b %CMAKE_EXE% --build . --config Debug
start /b %CMAKE_EXE% --build . --config Release
cd %ThirdPartyProjectsPath%
echo\

echo [ FreeType ] Start making project...
cd freetype
if not exist build mkdir build
cd build
%CMAKE_EXE% .. -G %CMAKE_IDE_FULL_NAME% -DCMAKE_CXX_FLAGS="/MT"
start /b %CMAKE_EXE% --build . --config Debug
start /b %CMAKE_EXE% --build . --config Release
cd %ThirdPartyProjectsPath%
echo\

echo [ AssetPipeline ] Start making project...
cd AssetPipeline
if %VS_VERSION% == vs2019 call ./make_win64_vs2019.bat
if %VS_VERSION% == vs2022 call ./make_win64_vs2022.bat
cd %MSBUILD_FOLDER%
"%MSBUILD_PATH%" -m %ThirdPartyProjectsPath%/AssetPipeline/AssetPipeline.sln /p:Configuration=Debug /p:Platform=x64
"%MSBUILD_PATH%" -m %ThirdPartyProjectsPath%/AssetPipeline/AssetPipeline.sln /p:Configuration=Release /p:Platform=x64
cd %ThirdPartyProjectsPath%
echo\

pause