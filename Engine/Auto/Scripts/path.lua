--------------------------------------------------------------
-- Prepare project path for make tool
--------------------------------------------------------------
-- Current working directory is in ./Engine/Auto/Scripts/
CurrentWorkingDirectory = os.getcwd()

-- Ignore case to find the root folder path's start index from current working directory
RootPath = string.sub(CurrentWorkingDirectory, 0, string.len(CurrentWorkingDirectory) - string.len("Engine/Auto/Scripts") - 1)
EnginePath = path.join(RootPath, "Engine")
BinariesPath = path.join(EnginePath, "Binaries/"..GetPlatformDisplayName())
IntermediatePath = path.join(EnginePath, "Intermediate/"..GetPlatformDisplayName())
EngineSourcePath = path.join(EnginePath, "Source")

-- Runtime
RuntimeSourcePath = path.join(EngineSourcePath, "Runtime")

-- Editor
EditorSourcePath = path.join(EngineSourcePath, "Editor")

-- ThirdParty
ThirdPartySourcePath = path.join(EngineSourcePath, "ThirdParty")