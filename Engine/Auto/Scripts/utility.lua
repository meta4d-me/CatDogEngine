--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine's utility projects
--------------------------------------------------------------

group("Build")

project("MakeEngine")
	kind("Utility")
	location(path.join(IntermediatePath, "Engine/Misc"))
	targetdir(BinariesPath)

	files {
		path.join(EnginePath, "Auto/Scripts/**.*")
	}

	if IsWindowsPlatform() then
		if USE_CLANG_TOOLSET then
			prebuildcommands {
				"cd "..RootPath,
				"Set USE_CLANG_TOOLSET=1",
				"MakeEngine_"..IDEConfigs.BuildIDEName..".bat",
			}
		else
			prebuildcommands {
				"cd "..RootPath,
				"MakeEngine_"..IDEConfigs.BuildIDEName..".bat",
			}
		end
	end

group("")