@echo off

Set BUILD_IDE_NAME="vs2022"
Set CMAKE_IDE_FULL_NAME="Visual Studio 17 2022"

if exist "./Engine/Auto/commercial_sdk_locations.bat" (
    call "./Engine/Auto/commercial_sdk_locations.bat"
) else (
    echo commercial_sdk_locations does not exist, skipped
)

cd "./Engine/Auto/Scripts"
"../Programs/premake5.exe" %BUILD_IDE_NAME%

pause