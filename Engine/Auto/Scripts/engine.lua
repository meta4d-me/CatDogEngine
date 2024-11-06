--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine Runtime
--------------------------------------------------------------

project("Engine")
	kind(EngineBuildLibKind)
	SetLanguageAndToolset("Engine/Runtime")
	dependson { "bx", "bimg", "bimg_decode", "bgfx" } -- sdl is pre-built in makefile.

	files {
		path.join(RuntimeSourcePath, "**.*"),
		path.join(ThirdPartySourcePath, "rapidxml/**.hpp"),
		path.join(ThirdPartySourcePath, "imgui/*.h"),
		path.join(ThirdPartySourcePath, "imgui/*.cpp"),
	}
	
	vpaths {
		["Source/*"] = { 
			path.join(RuntimeSourcePath, "**.*"),
		},
		["ImGui"] = {
			path.join(ThirdPartySourcePath, "imgui/*.h"),
			path.join(ThirdPartySourcePath, "imgui/*.cpp"),
		},
	}

	if ENABLE_FREETYPE then
		files {
			path.join(ThirdPartySourcePath, "imgui/misc/freetype/imgui_freetype.*"),
		}

		vpaths {
			["ImGui"] = {
				path.join(ThirdPartySourcePath, "imgui/misc/freetype/imgui_freetype.*"),
			},
		}
	end

	local bgfxBuildBinPath = nil
	local platformDefines = {}
	local platformIncludeDirs = {}
	if IsWindowsPlatform() then
		bgfxBuildBinPath = ThirdPartySourcePath.."/bgfx/.build/win64_"..IDEConfigs.BuildIDEName.."/bin"
		table.insert(platformIncludeDirs, path.join(ThirdPartySourcePath, "bx/include/compat/msvc"))
	elseif IsLinuxPlatform() then
		bgfxBuildBinPath = ThirdPartySourcePath.."/bgfx/.build/linux_"..IDEConfigs.BuildIDEName.."/bin"
		table.insert(platformIncludeDirs, path.join(ThirdPartySourcePath, "bx/include/compat/linux"))
	elseif IsAndroidPlatform() then
		bgfxBuildBinPath = ThirdPartySourcePath.."/bgfx/.build/android_arm64/bin"
		table.insert(platformIncludeDirs, path.join(ThirdPartySourcePath, "bx/include/compat/android"))
	end

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
		path.join(ThirdPartySourcePath, "imgui"),
		table.unpack(platformIncludeDirs),
		path.join(EnginePath, "BuiltInShaders/shaders"),
		path.join(EnginePath, "BuiltInShaders/UniformDefines"),
	}

	if ENABLE_FREETYPE then
		includedirs {
			path.join(ThirdPartySourcePath, "freetype/include"),
		}

		defines {
			"IMGUI_ENABLE_FREETYPE",
		}

		filter { "configurations:Debug" }
			libdirs {
				path.join(ThirdPartySourcePath, "freetype/build/Debug"),
			}
			links {
				"freetyped"
			}
		filter { "configurations:Release" }
			libdirs {
				path.join(ThirdPartySourcePath, "freetype/build/Release"),
			}
			links {
				"freetype"
			}
		filter {}
	end

	if ENABLE_SPDLOG then
		files {
			path.join(ThirdPartySourcePath, "spdlog/include/spdlog/**.*"),
		}
	
		vpaths {
			["spdlog/*"] = { 
				path.join(ThirdPartySourcePath, "spdlog/include/spdlog/**.*"),
			},
		}

		includedirs {
			path.join(ThirdPartySourcePath, "spdlog/include"),
		}

		defines {
			"SPDLOG_ENABLE", "SPDLOG_NO_EXCEPTIONS",
		}
	end

	if ENABLE_TRACY then
		files {
			path.join(ThirdPartySourcePath, "tracy/public/TracyClient.cpp"),
		}

		vpaths {
			["Tracy"] = {
				path.join(ThirdPartySourcePath, "tracy/public/TracyClient.cpp"),
			}
		}

		includedirs {
			path.join(ThirdPartySourcePath, "tracy/public"),
		}

		defines {
			"TRACY_ENABLE",
		}
	end

	if ENABLE_SUBPROCESS then
		defines {
			"ENABLE_SUBPROCESS"
		}
	end

	filter { "configurations:Debug" }
		table.insert(platformDefines, "BX_CONFIG_DEBUG")

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
	
	if ENABLE_DDGI then
		includedirs {
			path.join(DDGI_SDK_PATH, "include"),
		}
		libdirs {
			path.join(DDGI_SDK_PATH, "lib"),
		}
		links {
			"ddgi_sdk", "mright_sdk", "DDGIProbeDecoderBin"
		}
		defines {
			"ENABLE_DDGI",
			"DDGI_SDK_PATH=\""..DDGI_SDK_PATH.."\"",
		}
	else
		excludes {
			path.join(RuntimeSourcePath, "ECWorld/DDGIComponent.*"),
			path.join(RuntimeSourcePath, "Rendering/DDGIRenderer.*"),
		}
	end
	
	if "SharedLib" == EngineBuildLibKind then
		table.insert(platformDefines, "ENGINE_BUILD_SHARED")
	end
	
	defines {
		"SDL_MAIN_HANDLED", -- don't use SDL_main() as entry point
		"__STDC_LIMIT_MACROS", "__STDC_FORMAT_MACROS", "__STDC_CONSTANT_MACROS",
		"STB_IMAGE_STATIC",
		GetPlatformMacroName(),
		table.unpack(platformDefines),
		"CDENGINE_BUILTIN_SHADER_PATH=\""..BuiltInShaderSourcePath.."\"",
		"CDPROJECT_RESOURCES_SHARED_PATH=\""..ProjectSharedPath.."\"",
		"CDPROJECT_RESOURCES_ROOT_PATH=\""..ProjectResourceRootPath.."\"",
		"CDEDITOR_RESOURCES_ROOT_PATH=\""..EditorResourceRootPath.."\"",
		"EDITOR_MODE", -- TODO : remove it
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
		"MultiProcessorCompile", -- compiler uses multiple thread
	}

	filter { "action:vs*" }
		disablewarnings {
			-- MSVC : "needs to have dll-interface to be used by clients of class".
			-- This warning is not accurate indeed.
			"4251"
		}
	filter {}

	if ShouldTreatWaringAsError then
		flags {
			"FatalWarnings", -- treat warnings as errors
		}
	end
	
	CopyDllAutomatically()