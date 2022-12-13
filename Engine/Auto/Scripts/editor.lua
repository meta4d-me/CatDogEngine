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
	defines {
		"BX_CONFIG_DEBUG",
		"CDEDITOR_RESOURCES_ROOT_PATH=\""..editorResourcesPath.."\"",
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
		}
	
		links {
			"Engine",
		}

	filter { "configurations:Release" }
		runtime "Release" -- /MT
		libdirs {
			BinariesPath,
		}
	
		links {
			"Engine",
		}

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