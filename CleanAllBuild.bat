@echo off
set "CATDOG_BUILD_FOLDER=%cd%\Engine\Binaries"
set "CATDOG_TEMP_FOLDER=%cd%\Engine\Intermediate"
set "ASSETPIPELINE_BUILD_FOLDER=%cd%\Engine\Source\ThirdParty\AssetPipeline\build"
set "BGFX_BUILD_FOLDER=%cd%\Engine\Source\ThirdParty\bgfx\.build"
set "FREETYPE_BUILD_FOLDER=%cd%\Engine\Source\ThirdParty\freetype\build"
set "SDL_BUILD_FOLDER=%cd%\Engine\Source\ThirdParty\sdl\build"
set "foldersToDelete=CATDOG_BUILD_FOLDER CATDOG_TEMP_FOLDER ASSETPIPELINE_BUILD_FOLDER BGFX_BUILD_FOLDER FREETYPE_BUILD_FOLDER SDL_BUILD_FOLDER"

for %%i in (%foldersToDelete%) do (
    setlocal enabledelayedexpansion
    set "folderPath=!%%i!"
    if exist "!folderPath!" (
        rd /s /q "!folderPath!"
        if exist "!folderPath!" (
            echo Failed to delete folder: !folderPath!
        ) else (
            echo Folder deleted successfully: !folderPath!
        )
    ) else (
        echo Folder not found: !folderPath!
    )
    endlocal
)

pause
