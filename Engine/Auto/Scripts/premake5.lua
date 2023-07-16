--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine
--------------------------------------------------------------

EngineName = "CatDogEngine"

-- Build options
-- StaticLib is convenient to develop C++ applications which needs to reuse engine codes.
-- SharedLib needs to export APIs by ENGINE_API macro which needs more efforts to have a good design.
-- But it is necessary if you want to combine Engine and applications in other languages, such as C#.
EngineBuildLibKind = "StaticLib" -- "SharedLib"

PlatformSettings = {}
PlatformSettings["Windows"] = {
	DisplayName = "Win64",
	MacroName = "CD_PLATFORM_WINDOWS",
}

PlatformSettings["Andriod"] = {
	DisplayName = "Android",
	MacroName = "CD_PLATFORM_ANDROID",
}

PlatformSettings["OSX"] = {
	DisplayName = "OSX",
	MacroName = "CD_PLATFORM_OSX",
}

PlatformSettings["IOS"] = {
	DisplayName = "IOS",
	MacroName = "CD_PLATFORM_IOS",
}

ChoosePlatform = "Windows"
function GetPlatformDisplayName()
	return PlatformSettings[ChoosePlatform].DisplayName
end

function GetPlatformMacroName()
	return PlatformSettings[ChoosePlatform].MacroName
end

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

BUILD_WITH_LLVM_CLANG_CL = false
function SetLanguageAndToolset(projectName)
	language("C++")
	cppdialect("C++20")

	if BUILD_WITH_LLVM_CLANG_CL then
		toolset("clang")
	end

	location(path.join(IntermediatePath, projectName))
	targetdir(BinariesPath)
end

DDGI_SDK_PATH = os.getenv("DDGI_SDK_PATH") or ""
if not os.isdir(DDGI_SDK_PATH) then
	DDGI_SDK_PATH = ""
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
print("DDGI_SDK_PATH = "..DDGI_SDK_PATH)
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
		symbols("On")
		optimize("Full")
	filter {}

	filter "system:Windows"
		-- For Windows OS, we want to use latest Windows SDK installed in the PC.
		systemversion("latest")

		-- You can use the /utf-8 option to specify both the source and execution character sets as encoded by using UTF-8.
		-- Avoid compiler warnings about non-utf8 characters which cannot be present.
		-- https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
		buildoptions { "/utf-8" }
	filter {}

	startproject("Editor")

-- thirdparty projects such as sdl
dofile("thirdparty.lua")

-- engine projects
dofile("engine.lua")

-- editor projects
dofile("editor.lua")

-- regression tests for engine core modules
dofile("test.lua")

-- helper projects to compile shaders/textures, trigger makefiles...
dofile("utility.lua")