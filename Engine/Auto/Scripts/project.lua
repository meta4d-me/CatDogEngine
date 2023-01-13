--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine's example projects
--------------------------------------------------------------

-- Example Projects
ProjectsPath = path.join(RootPath, "Projects")
print("Make example projects : "..ProjectsPath)

function MakeProject(projectName)
	local projectSourcePath = path.join(ProjectsPath, projectName.."/Source")

	project(projectName)
		kind("ConsoleApp")
		language("C++")
		cppdialect("C++latest")
		
		dependson { "Engine" }

		location(path.join(IntermediatePath, "Projects/"..projectName))
		targetdir(BinariesPath)

		files {
			path.join(projectSourcePath, "**.*"),
		}
		
		vpaths {
			["Source"] = { path.join(projectSourcePath, "**.*") },
		}
		
		local projectResourcesPath = RootPath.."/Projects/PBRViewer/Resources/"
		defines {
			"BX_CONFIG_DEBUG",
			"CDENGINE_RESOURCES_ROOT_PATH=\""..projectResourcesPath.."\"",	
		}

		includedirs {
			path.join(EngineSourcePath, "Runtime/"),
			path.join(ThirdPartySourcePath, "AssetPipeline/public"),
			-- TODO : Editor should not include bgfx files.
			path.join(ThirdPartySourcePath, "bgfx/include"),
			path.join(ThirdPartySourcePath, "bgfx/3rdparty"),
			path.join(ThirdPartySourcePath, "bimg/include"),
			path.join(ThirdPartySourcePath, "bimg/3rdparty"),
			path.join(ThirdPartySourcePath, "bx/include"),
			path.join(ThirdPartySourcePath, "bx/include/compat/msvc"),
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
		}
end

group "Projects"

local allProjects = os.matchdirs(ProjectsPath.."/*")
for _, v in ipairs(allProjects) do
	local projectName = path.getname(v)
	print("Make project : "..projectName)
	MakeProject(projectName)
end

group ""
print("================================================================")