--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine Editor
--------------------------------------------------------------

project("Editor")
	kind("ConsoleApp")
	SetLanguageAndToolset("Editor")
	dependson { "Engine" }

	files {
		path.join(EditorSourcePath, "**.*"),
		path.join(ThirdPartySourcePath, "imguizmo/*.h"),
		path.join(ThirdPartySourcePath, "imguizmo/*.cpp"),
		path.join(ThirdPartySourcePath, "imgui-node-editor/*.h"),
		path.join(ThirdPartySourcePath, "imgui-node-editor/*.cpp"),
		path.join(ThirdPartySourcePath, "dmon/*.h"),
	}
	
	vpaths {
		["Source/*"] = { 
			path.join(EditorSourcePath, "**.*"),
		},
		["ImGuizmo"] = {
			path.join(ThirdPartySourcePath, "imguizmo/*.h"),
			path.join(ThirdPartySourcePath, "imguizmo/*.cpp"),
		},
		["imgui-node-editor"] = {
			path.join(ThirdPartySourcePath, "imgui-node-editor/*.h"),
			path.join(ThirdPartySourcePath, "imgui-node-editor/*.cpp"),
		}
	}

	defines {
		"BX_CONFIG_DEBUG",
		"CDENGINE_BUILTIN_SHADER_PATH=\""..BuiltInShaderSourcePath.."\"",
		"CDPROJECT_RESOURCES_SHARED_PATH=\""..ProjectSharedPath.."\"",
		"CDPROJECT_RESOURCES_ROOT_PATH=\""..ProjectResourceRootPath.."\"",
		"CDEDITOR_RESOURCES_ROOT_PATH=\""..EditorResourceRootPath.."\"",
		"CDENGINE_TOOL_PATH=\""..ToolRootPath.."\"",
		"EDITOR_MODE",
		GetPlatformMacroName(),
	}

	includedirs {
		path.join(EngineSourcePath, "Editor/"),
		path.join(EngineSourcePath, "Runtime/"),
		path.join(ThirdPartySourcePath, "AssetPipeline/public"),
		path.join(EnginePath, "BuiltInShaders/shaders"),
		path.join(EnginePath, "BuiltInShaders/UniformDefines"),
		-- TODO : Editor should not include bgfx files.
		path.join(ThirdPartySourcePath, "bgfx/include"),
		path.join(ThirdPartySourcePath, "bimg/include"),
		path.join(ThirdPartySourcePath, "bimg/3rdparty"),
		path.join(ThirdPartySourcePath, "bx/include"),
		path.join(ThirdPartySourcePath, "bx/include/compat/msvc"),
		path.join(ThirdPartySourcePath, "imgui"),
		path.join(ThirdPartySourcePath, "imguizmo"),
		path.join(ThirdPartySourcePath, "imgui-node-editor"),
		path.join(ThirdPartySourcePath, "dmon"),
		ThirdPartySourcePath,
	}

	if ENABLE_SPDLOG then
		defines {
			-- TODO : Remove _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING after spdlog updates the format to the right version.
			"SPDLOG_ENABLE", "SPDLOG_NO_EXCEPTIONS", "FMT_USE_NONTYPE_TEMPLATE_ARGS=0", "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
		}

		includedirs {
			path.join(ThirdPartySourcePath, "spdlog/include"),
		}
	end

	if ENABLE_TRACY then
		defines {
			"TRACY_ENABLE",
		}

		includedirs {
			path.join(ThirdPartySourcePath, "tracy/public"),
		}
	end

	if ENABLE_SUBPROCESS then
		defines {
			"ENABLE_SUBPROCESS"
		}
	end
	
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
	
	-- use /MT /MTd, not /MD /MDd
	staticruntime "on"
	filter { "configurations:Debug" }
		runtime "Debug" -- /MTd
	filter { "configurations:Release" }
		runtime "Release" -- /MT
	filter {}

	libdirs {
		BinariesPath,
		path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/%{cfg.buildcfg}"),
	}

	links {
		"Engine",
		"AssetPipelineCore",
		"CDProducer",
		"CDConsumer",
	}

	if not USE_CLANG_TOOLSET then
		links {
			"GenericProducer",
		}

		defines {
			"ENABLE_GENERIC_PRODUCER",
		}
	end

	if ENABLE_FBX_WORKFLOW then
		links {
			"FbxConsumer",
			"FbxProducer",
		}

		defines {
			"ENABLE_FBX_CONSUMER",
			"ENABLE_FBX_PRODUCER",
		}
	end

	-- Disable these options can reduce the size of compiled binaries.
	justmycode("Off")
	editAndContinue("Off")
	-- Sometimes I don't like to use exception because it will need compiler to generate extra codes about deconstructor callback safely.
	-- Editor application is OK to use as it is in a complex senario which can happen many unexpected user cases.
	exceptionhandling("On")
	rtti("Off")
		
	-- Strict.
	warnings("Default")
	externalwarnings("Off")
	
	-- For msvc, a static library dependends on more than one import library will cause a LINK4006 warning which said
	-- __NULL_IMPORT_DESCRIPTOR redefined. __NULL_IMPORT_DESCRIPTOR is at the end of all the import libraries to mark
	-- the end. So you can only fix it by two ways:
	-- 1. a static library only dependends on a import library.
	-- 2. ignore it and make sure that these two libs are safe to link together.
	-- linkoptions { "-IGNORE:4006" }

	flags {
		"MultiProcessorCompile", -- compiler uses multiple thread
	}

	if not USE_CLANG_TOOLSET then
		flags {
			"FatalWarnings", -- treat warnings as errors
		}
	end

	CopyDllAutomatically()