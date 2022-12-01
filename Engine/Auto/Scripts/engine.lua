--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine Runtime
--------------------------------------------------------------

project("Engine")
	kind(EngineBuildLibKind)
	language("C++")
	cppdialect("C++latest")
	dependson { "bx", "bimg", "bimg_decode", "bgfx" } -- sdl is pre-built in makefile.
	
	location(path.join(IntermediatePath, "Engine/Runtime"))
	targetdir(BinariesPath)

	files {
		path.join(RuntimeSourcePath, "**.*"),
		path.join(ThirdPartySourcePath, "AssetPipeline/public/**.*"),
		path.join(ThirdPartySourcePath, "rapidxml/**.hpp"),
	}
	
	vpaths {
		["Source/*"] = { 
			path.join(RuntimeSourcePath, "**.*"),
		},
		["AssetPipeline"] = {
			path.join(ThirdPartySourcePath, "AssetPipeline", "Public/Producer/CatDogProducer.*"),
		}
	}
	
	local bgfxBuildBinPath = nil
	local platformDefines = nil
	local platformIncludeDirs = nil

	filter { "system:windows" }
		bgfxBuildBinPath = ThirdPartySourcePath.."/bgfx/.build/win64_"..IDEConfigs.BuildIDEName.."/bin"
		platformDefines = {
			"_CRT_SECURE_NO_WARNINGS"
		}

		platformIncludeDirs = { 
			path.join(ThirdPartySourcePath, "bx/include/compat/msvc")
		}
	filter {}

	includedirs {
		RuntimeSourcePath,
		ThirdPartySourcePath,
		path.join(ThirdPartySourcePath, "AssetPipeline/public"),
		path.join(ThirdPartySourcePath, "bgfx/include"),
		path.join(ThirdPartySourcePath, "bgfx/3rdparty"),
		path.join(ThirdPartySourcePath, "bimg/include"),
		path.join(ThirdPartySourcePath, "bimg/3rdparty"),
		path.join(ThirdPartySourcePath, "bx/include"),
		path.join(ThirdPartySourcePath, "sdl/include"),
		table.unpack(platformIncludeDirs),
	}

	filter { "configurations:Debug" }
		platformDefines = {
			"BX_CONFIG_DEBUG",
		}
		libdirs {
			path.join(ThirdPartySourcePath, "sdl/build/Debug"),
			bgfxBuildBinPath,
		}
		links {
			"sdl2d", "sdl2maind",
			"bgfxDebug", "bimgDebug", "bxDebug", "bimg_decodeDebug"
		}
	filter { "configurations:Release" }
		libdirs {
			path.join(ThirdPartySourcePath, "sdl/build/Release"),
			bgfxBuildBinPath,
		}
		links {
			"sdl2", "sdl2main",
			"bgfxRelease", "bimgRelease", "bxRelease", "bimg_decodeRelease"
		}
	filter {}

	if "SharedLib" == EngineBuildLibKind then
		table.insert(platformDefines, "ENGINE_BUILD_SHARED")
	end

	local editorResourcesPath = RootPath.."/Engine/Source/Editor/Resources/"
	local projectResourcesPath = RootPath.."/Projects/SponzaBaseScene/Resources/"
	defines {
		"SDL_MAIN_HANDLED",
		"__STDC_LIMIT_MACROS", "__STDC_FORMAT_MACROS", "__STDC_CONSTANT_MACROS",
		"STB_IMAGE_STATIC",
		EngineBuildPlatform,
		table.unpack(platformDefines),
		"CDENGINE_RESOURCES_ROOT_PATH=\""..projectResourcesPath.."\"",
		"CDEDITOR_RESOURCES_ROOT_PATH=\""..editorResourcesPath.."\"",
	}

	-- use /MT /MTd, not /MD /MDd
	staticruntime "on"
	filter { "configurations:Debug" }
		runtime "Debug" -- /MTd
	filter { "configurations:Release" }
		runtime "Release" -- /MT
	filter {}

	-- Disable these options can reduce the size of compiled binaries.
	justmycode("Off")
	editAndContinue("Off")
	exceptionhandling("Off")
	rtti("Off")	
		
	-- Strict.
	warnings("Default")
	externalwarnings("Off")
		
	flags {
		"FatalWarnings", -- treat warnings as errors
		"MultiProcessorCompile", -- compiler uses multiple thread
	}
	
	-- copy dll into binary folder automatically.
	filter { "configurations:Debug" }
		postbuildcommands {
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Debug/SDL2d.dll").."\" \""..BinariesPath.."\"",
		}
	filter { "configurations:Release" }
		postbuildcommands {
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Release/SDL2.dll").."\" \""..BinariesPath.."\"",
		}
	filter {}