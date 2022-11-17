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
		
		defines {
			"BX_CONFIG_DEBUG",
		}

		includedirs {
			path.join(EngineSourcePath, "Runtime/"),
		}
		
		libdirs {
			BinariesPath,
		}
		
		links {
			"Engine"
		}

		-- use /MT /MTd, not /MD /MDd
		staticruntime "on"
		filter { "configurations:Debug" }
			runtime "Debug" -- /MTd
		filter { "configurations:Release" }
			runtime "Release" -- /MT
		filter {}
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