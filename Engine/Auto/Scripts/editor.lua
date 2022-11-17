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
	}
	
	vpaths {
		["Source/*"] = { 
			path.join(EditorSourcePath, "**.*"),
		},
		["ThirdParty/ImGui"] = {
			path.join(ThirdPartySourcePath, "bgfx/3rdparty/dear-imgui/**.*"),
		}
	}

	includedirs {
		EditorSourcePath,
	}

	libdirs {

	}
	
	links {

	}

	defines {
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