--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine's utility projects
--------------------------------------------------------------

group("Build")
--project("CompileShaders")
--	kind("Utility")
--	location(path.join(IntermediatePath, "Engine/Misc"))
--	targetdir(BinariesPath)
--	local shadersFilePath = path.join(EnginePath, "BuiltInShaders")
--	vpaths {
--		["shaders"] = path.join(shadersFilePath, "shaders/**.*"),
--		[""] = path.join(shadersFilePath, "compile_dx11.bat"),
--	}

--	files {
--		path.join(shadersFilePath, "shaders/**.*"),
--		path.join(shadersFilePath, "compile_dx11.bat"),
--	}

--	filter { "system:windows" }
--		prebuildcommands {
--			"cd "..path.join(shadersFilePath),
--			"compile_dx11.bat",
--		}
--	filter {}

--project("CompileTextures")
--	kind("Utility")
--	location(path.join(IntermediatePath, "Engine/Misc"))
--	targetdir(BinariesPath)
--
--	local textureFilePath = path.join(ProjectsPath, "PBRViewer/Resources/Textures")
--	vpaths {
--		["textures"] = path.join(textureFilePath, "textures/**.*"),
--		[""] = path.join(textureFilePath, "texture.bat"),
--	}
--
--	files {
--		path.join(textureFilePath, "textures/**.*"),
--		path.join(textureFilePath, "texture.bat"),
--	}
--
--	filter { "system:windows" }
--		prebuildcommands {
--			"cd "..path.join(ProjectsPath, "PBRViewer/Resources/Textures"),
--			"texture.bat",
--		}
--	filter {}

project("MakeEngine")
	kind("Utility")
	location(path.join(IntermediatePath, "Engine/Misc"))
	targetdir(BinariesPath)

	files {
		path.join(EnginePath, "Auto/Scripts/**.*")
	}

	filter { "system:windows" }
		prebuildcommands {
			"cd "..RootPath,
			"MakeEngine_"..IDEConfigs.BuildIDEName..".bat",
		}
	filter {}

--project("MakeThirdParty")
--	kind("Utility")
--	location(path.join(IntermediatePath, "Engine/Misc"))
--	targetdir(BinariesPath)
--
--	filter { "system:windows" }
--		prebuildcommands {
--			"cd "..RootPath,
--			"MakeThirdParty_"..IDEConfigs.BuildIDEName..".bat",
--		}
--	filter {}

group("")