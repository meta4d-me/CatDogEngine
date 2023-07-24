@echo off

Set BUILD_IDE_NAME=vs2022_android
Set CD_PLATFORM=Android
Set CMAKE_IDE_FULL_NAME="Visual Studio 17 2022"

if exist "./Engine/Auto/commercial_sdk_locations.bat" (
    call "./Engine/Auto/commercial_sdk_locations.bat"
) else (
    echo commercial_sdk_locations does not exist, skipped
)

cd "./Engine/Auto/Scripts"
Set USE_CLANG_TOOLSET=1
"../Programs/Windows/premake5.exe" --os=android vs2022

pause