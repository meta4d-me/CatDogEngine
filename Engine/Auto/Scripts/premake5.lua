--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine
--------------------------------------------------------------
EngineName = "CatDogEngine"
EngineBuildLibKind = "StaticLib" -- "SharedLib"

ChoosePlatform = os.getenv("CD_PLATFORM") or "Windows"
function IsWindowsPlatform()
	return ChoosePlatform == "Windows"
end

function IsAndroidPlatform()
	return ChoosePlatform == "Android"
end

function IsLinuxPlatform()
	return ChoosePlatform == "Linux"
end

function IsMacPlatform()
	return ChoosePlatform == "OSX"
end

function IsIOSPlatform()
	return ChoosePlatform == "IOS"
end

USE_CLANG_TOOLSET = false
if os.getenv("USE_CLANG_TOOLSET") then
	USE_CLANG_TOOLSET = true
end

DDGI_SDK_PATH = os.getenv("DDGI_SDK_PATH") or ""
if not os.isdir(DDGI_SDK_PATH) then
	DDGI_SDK_PATH = ""
end

ENABLE_FREETYPE = not USE_CLANG_TOOLSET and not IsLinuxPlatform() and not IsAndroidPlatform()
ENABLE_SPDLOG = not USE_CLANG_TOOLSET and not IsLinuxPlatform() and not IsAndroidPlatform()
ENABLE_SUBPROCESS = not USE_CLANG_TOOLSET and not IsLinuxPlatform() and not IsAndroidPlatform()
ENABLE_TRACY = not USE_CLANG_TOOLSET and not IsLinuxPlatform() and not IsAndroidPlatform()
ENABLE_DDGI = DDGI_SDK_PATH ~= ""

ShouldTreatWaringAsError = not (ENABLE_DDGI or USE_CLANG_TOOLSET)

PlatformSettings = {}
PlatformSettings["Windows"] = {
	DisplayName = "Win64",
	MacroName = "CD_PLATFORM_WINDOWS",
}

PlatformSettings["Android"] = {
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
	end
	
	cppdialect("C++20")
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
	
	filter "system:Windows"
		architecture "x64"
		system("windows")
	filter "system:Android"
		architecture "ARM64"
		androidapilevel(21)
		system("android")
	filter {}
	
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
	if IsAndroidPlatform() then
		return
	end

	-- copy dll into binary folder automatically.
	filter { "configurations:Debug" }
		if IsAndroidPlatform() then
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Debug/SDL2d.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/ARM64/Debug/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/ARM64/Debug/CDProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/ARM64/Debug/CDConsumer.*").."\" \""..BinariesPath.."\"",
			}
		else
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Debug/SDL2d.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/CDProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/CDConsumer.*").."\" \""..BinariesPath.."\"",
			}
		end

		if not USE_CLANG_TOOLSET then
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/GenericProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/assimp-*-mtd.*").."\" \""..BinariesPath.."\"",
			}
		end
	filter { "configurations:Release" }
		if IsAndroidPlatform() then
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Release/SDL2.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/ARM64/Release/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/ARM64/Release/CDProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/ARM64/Release/CDConsumer.*").."\" \""..BinariesPath.."\"",
			}
		else
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Release/SDL2.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/CDProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/CDConsumer.*").."\" \""..BinariesPath.."\"",
			}
		end

		if not USE_CLANG_TOOLSET then
			postbuildcommands {
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/GenericProducer.*").."\" \""..BinariesPath.."\"",
				"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/assimp-*-mt.*").."\" \""..BinariesPath.."\"",
			}
		end
	filter {}

	if ENABLE_DDGI then
		postbuildcommands {
			"{COPYFILE} \""..path.join(DDGI_SDK_PATH, "bin/*.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(DDGI_SDK_PATH, "bin/ThirdParty/ffmpeg/*.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(DDGI_SDK_PATH, "bin/ThirdParty/zlib/*.*").."\" \""..BinariesPath.."\"",
		}
	end
end

-- thirdparty projects
if not IsAndroidPlatform() then
	dofile("thirdparty.lua")
end

-- engine projects
dofile("engine.lua")

-- editor projects
if not IsAndroidPlatform() then
	dofile("editor.lua")
end

-- game projects
--dofile("game.lua")

-- regression tests for engine core modules
dofile("test.lua")

-- helper projects to compile shaders/textures, trigger makefiles...
dofile("utility.lua")

-- package
if IsAndroidPlatform() then
	dofile("package.lua")
end