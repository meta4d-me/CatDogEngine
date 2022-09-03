--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine Runtime
--------------------------------------------------------------

project("Engine")
	kind("SharedLib")
	language("C++")
	cppdialect("C++latest")
	--dependson { "SDL2" }

	location(path.join(IntermediatePath, "Engine/Runtime"))
	targetdir(BinariesPath)

	files {
		"*.lua", -- not compiled, just convenient to edit in IDE
		path.join(RuntimeSourcePath, "**.*"),
		path.join(ThirdPartySourcePath, "AssetPipeline/public/**.*"),
		--path.join(ThirdPartySourcePath, "bgfx/3rdparty/dear-imgui/**.*"),
	}
	
	removefiles {
		path.join(RuntimeSourcePath, "Windowing/**.*"),
		path.join(RuntimeSourcePath, "Camera.*"),
	}

	vpaths {
		["Makefile"] = { "*.lua" },
		["Source/*"] = { 
			path.join(RuntimeSourcePath, "**.*"),
		},
		["AssetPipeline"] = {
			path.join(ThirdPartySourcePath, "AssetPipeline", "Public/Producer/CatDogProducer.*"),
		},
		["ThirdParty/ImGui"] = {
			path.join(ThirdPartySourcePath, "bgfx/3rdparty/dear-imgui/**.*"),
		}
	}
	
	local platformDefines = nil
	local platformIncludeDirs = nil

	filter "system:windows"
		platformDefines = {
			"_CRT_SECURE_NO_WARNINGS"
		}

		platformIncludeDirs = { 
			path.join(ThirdPartySourcePath, "bx/include/compat/msvc")
		}
	filter {}

	includedirs {
		path.join(ThirdPartySourcePath, "AssetPipeline/public"),
		path.join(ThirdPartySourcePath, "bgfx/include"),
		path.join(ThirdPartySourcePath, "bgfx/3rdparty"),
		path.join(ThirdPartySourcePath, "bimg/include"),
		path.join(ThirdPartySourcePath, "bimg/3rdparty"),
		path.join(ThirdPartySourcePath, "bx/include"),
		--path.join(ThirdPartySourcePath, "sdl2/include"),
		table.unpack(platformIncludeDirs),
	}

	local configurationDefines = nil
	filter { "configurations:Debug" }
		configurationDefines = {
			"BX_CONFIG_DEBUG",
		}
		libdirs {
			--path.join(ThirdPartyProjectPath, "sdl2/Debug"),
			path.join(ThirdPartyProjectPath, "openal"),
			path.join(ThirdPartyProjectPath, "libsndfile"),
			path.join(ThirdPartyProjectPath, "spdlog"),
			path.join(ThirdPartySourcePathPath, "bgfx/build/win64_"..IDEConfigs.BuildIDEName.."/bin"),
		}
		links {
			--"sdl2d", "sdl2maind",
			"bgfxDebug", "bimgDebug", "bxDebug", "bimg_decodeDebug"
		}
	filter { "configurations:Release" }
		libdirs {
			--path.join(ThirdPartyProjectPath, "sdl2/Release"),
			path.join(ThirdPartyProjectPath, "openal"),
			path.join(ThirdPartyProjectPath, "libsndfile"),
			path.join(ThirdPartyProjectPath, "spdlog"),
			path.join(ThirdPartySourcePathPath, "bgfx/build/win64_"..IDEConfigs.BuildIDEName.."/bin"),
		}
		links {
			--"sdl2", "sdl2main",
			"bgfx", "bimg", "bx", "bimg_decode"
		}
	filter {}

	defines {
		--"SDL_MAIN_HANDLED",
		"__STDC_LIMIT_MACROS", "__STDC_FORMAT_MACROS", "__STDC_CONSTANT_MACROS",
		"STB_IMAGE_STATIC",
		table.unpack(configurationDefines),
		table.unpack(platformDefines),

		"ENGINE_BUILD_SHARED",
	}

	-- put all runtime dlls into this folder
	--debugdir("build/bin/"..buildType.."/runtime/")
	
	-- use /MT /MTd, not /MD /MDd
	staticruntime "on"
	filter { "configurations:Debug" }
		runtime "Debug" -- /MTd
	filter { "configurations:Develop" }
		runtime "Debug" -- /MTd
	filter { "configurations:Release" }
		runtime "Release" -- /MT
	filter {}

	-- seems useless in cpp development.
	-- disable these options can reduce the size of compiled binaries.
	justmycode("Off")
	editAndContinue("Off")
	exceptionhandling("Off")
	rtti("Off")	
		
	-- Be strict to our own codes.
	disablewarnings {
	}
	warnings("Default")
	externalwarnings("Off")
		
	flags {
		"FatalWarnings", -- treat warnings as errors
		"MultiProcessorCompile", -- compiler uses multiple thread
	}

	-- copy dll into binary folder automatically.
	local sourceEngineDllPath = path.join(BinariesPath, "Engine.dll*")
	local EditorBinPath = path.join(CurrentWorkingDirectory, "CatDogEditor")
	if os.isdir(EditorBinPath) then
		local targetEngineDllPath = path.join(EditorBinPath, "Debug/NativePlugin/x64", "Engine.dll*")
		filter { "system:windows" }
			postbuildcommands { 
				"xcopy /c /f /y \""..sourceEngineDllPath.."\" \""..targetEngineDllPath.."\"",
			}
	end