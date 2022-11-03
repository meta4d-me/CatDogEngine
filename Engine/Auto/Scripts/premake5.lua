--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine
--------------------------------------------------------------

-- Parse folder path
dofile("path.lua")
print("================================================================")
print("CurrentWorkingDirectory = "..CurrentWorkingDirectory)
print("RootPath = "..RootPath)
print("EnginePath = "..EnginePath)
print("BinariesPath = "..BinariesPath)
print("IntermediatePath = "..IntermediatePath)
print("EngineSourcePath = "..EngineSourcePath)
print("RuntimeSourcePath = "..RuntimeSourcePath)
print("================================================================")

IDEConfigs = {}
local buildIDEName = os.getenv("BUILD_IDE_NAME")
buildIDEName = string.gsub(buildIDEName, "\"", "")
IDEConfigs.BuildIDEName = buildIDEName
if "vs2022" == buildIDEName then
	IDEConfigs.VCVersion = "vc143"
elseif "vs2019" == buildIDEName then
	IDEConfigs.VCVersion = "vc142"
else
	print(buildIDEName.." : No ide compiler version!")
	return
end

-- workspace means solution in Visual Studio
workspace(EngineName)
	-- location specifies the path for workspace project file, .sln for Visual Studio.
	location(RootPath)
	
	architecture "x64"
	
	-- Set build configs
	configurations { "Debug", "Release" }
	
	-- Debug is a strict debug mode. No optimization will be performed.
	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols("On")
		optimize("Off")
	-- Full optimization.
	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols("Off")
		optimize("Full")
	filter {}

	filter "system:Windows"
		-- For Windows OS, we want to use latest Windows SDK installed in the PC.
		systemversion("latest")
	filter {}

-- utils
function DeclareExternalProject(projectName, projectKind, projectPath)
	-- Same with projectName by default
	projectPath = projectPath or projectName

	externalproject(projectName)
		kind(projectKind)
		location(path.join(ThirdPartyProjectPath, projectPath))
end

-- Make projects
dofile("thirdparty.lua")
dofile("engine.lua")
dofile("project.lua")

project("AutoMake")
	kind("Utility")
	location(path.join(IntermediatePath, "Engine/Misc"))
	targetdir(BinariesPath)	
	files {
		path.join(EnginePath, "Auto/Scripts/**.*")
	}

	filter { "system:windows" }
		prebuildcommands {
			"cd "..RootPath,
			"MakeEngine_"..IDEConfigs.BuildIDEName..".bat",
		}
	filter {}