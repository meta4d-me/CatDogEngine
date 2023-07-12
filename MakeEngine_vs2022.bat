@echo off

call ./Engine/Auto/Configs/vs2022/Config.bat

if exist "./Engine/Auto/commercial_sdk_locations.bat" (
    call "./Engine/Auto/commercial_sdk_locations.bat"
) else (
    echo commercial_sdk_locations does not exist, skipped
)

cd "./Engine/Auto/Scripts"
"../Programs/premake5.exe" %BUILD_IDE_NAME%

pause