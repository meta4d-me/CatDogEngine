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
		path.join(ThirdPartySourcePath, "imgui/misc/freetype/imgui_freetype.*"),
	}
	
	vpaths {
		["Source/*"] = { 
			path.join(RuntimeSourcePath, "**.*"),
		},
		["ImGui"] = {
			path.join(ThirdPartySourcePath, "imgui/*.h"),
			path.join(ThirdPartySourcePath, "imgui/*.cpp"),
			path.join(ThirdPartySourcePath, "imgui/misc/freetype/imgui_freetype.*"),
		},
	}
	
	local bgfxBuildBinPath = nil
	local platformDefines = nil
	local platformIncludeDirs = nil
	filter { "system:windows" }
		bgfxBuildBinPath = ThirdPartySourcePath.."/bgfx/.build/win64_"..IDEConfigs.BuildIDEName.."/bin"
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
		path.join(ThirdPartySourcePath, "imgui"),
		path.join(ThirdPartySourcePath, "freetype/include"),
		table.unpack(platformIncludeDirs),
		path.join(EnginePath, "BuiltInShaders/shaders"),
		path.join(EnginePath, "BuiltInShaders/UniformDefines"),
	}

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
			"ENABLE_SPDLOG", "SPDLOG_NO_EXCEPTIONS", "FMT_USE_NONTYPE_TEMPLATE_ARGS=0",
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
			"ENABLE_TRACY",
		}
	end

	filter { "configurations:Debug" }
		platformDefines = {
			"BX_CONFIG_DEBUG",
		}
		libdirs {
			path.join(ThirdPartySourcePath, "sdl/build/Debug"),
			path.join(ThirdPartySourcePath, "freetype/build/Debug"),
			bgfxBuildBinPath,
		}
		links {
			"sdl2d", "sdl2maind",
			"bgfxDebug", "bimgDebug", "bxDebug", "bimg_decodeDebug",
			"freetyped"
		}
	filter { "configurations:Release" }
		libdirs {
			path.join(ThirdPartySourcePath, "sdl/build/Release"),
			path.join(ThirdPartySourcePath, "freetype/build/Release"),
			bgfxBuildBinPath,
		}
		links {
			"sdl2", "sdl2main",
			"bgfxRelease", "bimgRelease", "bxRelease", "bimg_decodeRelease",
			"freetype"
		}
	filter {}
	
	filter { "configurations:Release" }
		if DDGI_SDK_PATH ~= "" then
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
				"DDGI_SDK_PATH=\""..DDGI_SDK_PATH.."\"",
			}
		end
	filter {}
	
	if "SharedLib" == EngineBuildLibKind then
		table.insert(platformDefines, "ENGINE_BUILD_SHARED")
	end
	
	defines {
		"SDL_MAIN_HANDLED", -- don't use SDL_main() as entry point
		"__STDC_LIMIT_MACROS", "__STDC_FORMAT_MACROS", "__STDC_CONSTANT_MACROS",
		"STB_IMAGE_STATIC",
		"IMGUI_ENABLE_FREETYPE",
		GetPlatformMacroName(),
		table.unpack(platformDefines),
		"CDENGINE_BUILTIN_SHADER_PATH=\""..BuiltInShaderSourcePath.."\"",
		"CDPROJECT_RESOURCES_SHARED_PATH=\""..ProjectSharedPath.."\"",
		"CDPROJECT_RESOURCES_ROOT_PATH=\""..ProjectResourceRootPath.."\"",
		"CDEDITOR_RESOURCES_ROOT_PATH=\""..EditorResourceRootPath.."\"",
		"EDITOR_MODE",
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

	if DDGI_SDK_PATH == "" and not USE_CLANG_TOOLSET then
		flags {
			"FatalWarnings", -- treat warnings as errors
		}
	end
	
	CopyDllAutomatically()
