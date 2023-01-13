--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine Editor
--------------------------------------------------------------

project("Editor")
	kind("ConsoleApp")
	language("C++")
	cppdialect("C++latest")
	dependson { "Engine" }

	location(path.join(IntermediatePath, "Editor"))
	targetdir(BinariesPath)

	files {
		path.join(EditorSourcePath, "**.*"),
		path.join(ThirdPartySourcePath, "imguizmo/*.h"),
		path.join(ThirdPartySourcePath, "imguizmo/*.cpp"),
	}
	
	vpaths {
		["Source/*"] = { 
			path.join(EditorSourcePath, "**.*"),
		},
		["ImGuizmo"] = {
			path.join(ThirdPartySourcePath, "imguizmo/*.h"),
			path.join(ThirdPartySourcePath, "imguizmo/*.cpp"),
		},
	}

	local editorResourcesPath = RootPath.."/Engine/Source/Editor/Resources/"
	local projectResourcesPath = RootPath.."/Projects/PBRViewer/Resources/"
	defines {
		"BX_CONFIG_DEBUG",
		"CDEDITOR_RESOURCES_ROOT_PATH=\""..editorResourcesPath.."\"",
		"CDENGINE_TOOL_PATH=\""..path.join(EnginePath, "EditorTools").."\"",
		"CDENGINE_RESOURCES_ROOT_PATH=\""..projectResourcesPath.."\"",
	}

	includedirs {
		path.join(EngineSourcePath, "Editor/"),
		path.join(EngineSourcePath, "Runtime/"),
		path.join(ThirdPartySourcePath, "AssetPipeline/public"),
		-- TODO : Editor should not include bgfx files.
		path.join(ThirdPartySourcePath, "bgfx/include"),
		path.join(ThirdPartySourcePath, "bimg/include"),
		path.join(ThirdPartySourcePath, "bimg/3rdparty"),
		path.join(ThirdPartySourcePath, "bx/include"),
		path.join(ThirdPartySourcePath, "bx/include/compat/msvc"),
		path.join(ThirdPartySourcePath, "imgui"),
		path.join(ThirdPartySourcePath, "imguizmo"),
		ThirdPartySourcePath,
	}

	defines {
	}

	-- use /MT /MTd, not /MD /MDd
	staticruntime "on"
	filter { "configurations:Debug" }
		runtime "Debug" -- /MTd
		libdirs {
			BinariesPath,
			path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug"),
		}
	filter { "configurations:Release" }
		runtime "Release" -- /MT
		libdirs {
			BinariesPath,
			path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release"),
		}
	filter {}

	links {
		"Engine",
		"AssetPipelineCore",
		"CDProducer",
		"GenericProducer",
		"CDConsumer",
	}

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
		"FatalWarnings", -- treat warnings as errors
		"MultiProcessorCompile", -- compiler uses multiple thread
	}