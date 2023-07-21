--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine
--------------------------------------------------------------
ChoosePlatform = os.getenv("CD_PLATFORM") or "Windows"
EngineName = "CatDogEngine"
EngineBuildLibKind = "StaticLib" -- "SharedLib"

USE_CLANG_TOOLSET = false
if os.getenv("USE_CLANG_TOOLSET") then
	USE_CLANG_TOOLSET = true
end

DDGI_SDK_PATH = os.getenv("DDGI_SDK_PATH") or ""
if not os.isdir(DDGI_SDK_PATH) then
	DDGI_SDK_PATH = ""
end

ENABLE_SPDLOG = not USE_CLANG_TOOLSET
ENABLE_TRACY = not USE_CLANG_TOOLSET

function IsWindowsPlatform()
	return ChoosePlatform == "Windows"
end

function IsLinuxPlatform()
	return ChoosePlatform == "Andriod" or ChoosePlatform == "Linux"
end

function IsMacPlatform()
	return ChoosePlatform == "OSX" or ChoosePlatform == "IOS"
end

PlatformSettings = {}
PlatformSettings["Windows"] = {
	DisplayName = "Win64",
	MacroName = "CD_PLATFORM_WINDOWS",
}

PlatformSettings["Andriod"] = {
	DisplayName = "Android",
	MacroName = "CD_PLATFORM_ANDROID",
}

PlatformSettings["Linux"] = {
	DisplayName = "Linux",
	MacroName = "CD_PLATFORM_LINUX",
}

PlatformSettings["OSX"] = {
	DisplayName = "OSX",
	MacroName = "CD_PLATFORM_OSX",
}

PlatformSettings["IOS"] = {
	DisplayName = "IOS",
	MacroName = "CD_PLATFORM_IOS",
}

function GetPlatformDisplayName()
	return PlatformSettings[ChoosePlatform].DisplayName
end

function GetPlatformMacroName()
	return PlatformSettings[ChoosePlatform].MacroName
end

IDEConfigs = {}
IDEConfigs.BuildIDEName = os.getenv("BUILD_IDE_NAME")

function SetLanguageAndToolset(projectName)
	language("C++")
	
	if USE_CLANG_TOOLSET then
		toolset("clang")
		cppdialect("C++17")
	else
		cppdialect("C++20")
	end

	location(path.join(IntermediatePath, projectName))
	targetdir(BinariesPath)
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
print("GameSourcePath = "..GameSourcePath)
print("RuntimeSourcePath = "..RuntimeSourcePath)
print("IDEConfigs.BuildIDEName = "..IDEConfigs.BuildIDEName)
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

function CopyDllAutomatically()
	-- copy dll into binary folder automatically.
	filter { "configurations:Debug" }
		postbuildcommands {
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Debug/SDL2d.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/CDProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/CDConsumer.*").."\" \""..BinariesPath.."\"",
		}

		if not USE_CLANG_TOOLSET then
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/GenericProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/TerrainProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/assimp-*-mtd.*").."\" \""..BinariesPath.."\"",
			}
		end
	filter { "configurations:Release" }
		postbuildcommands {
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Release/SDL2.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/CDProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/CDConsumer.*").."\" \""..BinariesPath.."\"",
		}

		if not USE_CLANG_TOOLSET then
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/GenericProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/TerrainProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/assimp-*-mt.*").."\" \""..BinariesPath.."\"",
			}
		end
	filter {}

	filter { "configurations:Release" }
		if DDGI_SDK_PATH ~= "" then
			postbuildcommands {
				"{COPYFILE} \""..path.join(DDGI_SDK_PATH, "bin/*.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(DDGI_SDK_PATH, "bin/ThirdParty/ffmpeg/*.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(DDGI_SDK_PATH, "bin/ThirdParty/zlib/*.*").."\" \""..BinariesPath.."\"",
			}
		end
	filter {}
end

-- thirdparty projects such as sdl
dofile("thirdparty.lua")

-- engine projects
dofile("engine.lua")

-- editor projects
dofile("editor.lua")

-- game projects
dofile("game.lua")

-- regression tests for engine core modules
dofile("test.lua")

-- helper projects to compile shaders/textures, trigger makefiles...
dofile("utility.lua")