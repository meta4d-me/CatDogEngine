--------------------------------------------------------------
-- Prepare project path for make tool
--------------------------------------------------------------
-- Current working directory is in ./Engine/Auto/Scripts/
CurrentWorkingDirectory = os.getcwd()

-- Root
EngineName = "CatDogEngine"
PlatformName = "Win64"

-- Ignore case to find the root folder path's start index from current working directory
RootPath = string.sub(CurrentWorkingDirectory, 0, string.len(CurrentWorkingDirectory) - string.len("Engine/Auto/Scripts") - 1)
EnginePath = path.join(RootPath, "Engine")
BinariesPath = path.join(EnginePath, "Binaries/"..PlatformName)
IntermediatePath = path.join(EnginePath, "Intermediate/"..PlatformName)
EngineSourcePath = path.join(EnginePath, "Source")

-- ThirdParty
ThirdPartySourcePath = path.join(EngineSourcePath, "ThirdParty")

-- Runtime
RuntimeSourcePath = path.join(EngineSourcePath, "Runtime")