--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine
--------------------------------------------------------------

-- Build options
-- StaticLib is convenient to develop C++ applications which needs to reuse engine codes.
-- SharedLib needs to export APIs by ENGINE_API macro which needs more efforts to have a good design.
-- But it is necessary if you want to combine Engine and applications in other languages, such as C#.
EngineBuildLibKind = "StaticLib" -- "SharedLib"

-- OS Platform
EngineBuildPlatform = "PLATFORM_WINDOWS"
EngineGraphicsBackend = "D3D11"

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

-- Parse folder path
dofile("path.lua")
print("================================================================")
print("EngineBuildLibKind = "..EngineBuildLibKind)
print("CurrentWorkingDirectory = "..CurrentWorkingDirectory)
print("RootPath = "..RootPath)
print("EnginePath = "..EnginePath)
print("BinariesPath = "..BinariesPath)
print("IntermediatePath = "..IntermediatePath)
print("EngineSourcePath = "..EngineSourcePath)
print("RuntimeSourcePath = "..RuntimeSourcePath)
print("IDEConfigs.BuildIDEName = "..IDEConfigs.BuildIDEName)
print("IDEConfigs.VCVersion = "..IDEConfigs.VCVersion)
print("================================================================")

-- workspace means solution in Visual Studio
workspace(EngineName)
	-- location specifies the path for workspace project file, .sln for Visual Studio.
	location(RootPath)
	targetdir(BinariesPath)
	
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

-- thirdparty projects such as sdl
dofile("thirdparty.lua")

-- engine projects
dofile("engine.lua")

-- editor projects
dofile("editor.lua")

-- game projects made by engine
dofile("project.lua")

-- helper projects to compile shaders/textures, trigger makefiles...
dofile("utility.lua")