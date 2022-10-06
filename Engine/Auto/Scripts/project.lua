--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine's example projects
--------------------------------------------------------------

-- Example Projects
ProjectsPath = path.join(RootPath, "Projects")
print("Make example projects : "..ProjectsPath)

function MakeProject(projectName)
	local projectSourcePath = path.join(ProjectsPath, projectName.."/Source")
	local projectBinaryPath = path.join(BinariesPath, "Projects/"..projectName)

	project(projectName)
		kind("ConsoleApp")
		language("C++")
		cppdialect("C++latest")
		
		dependson { "Engine" }

		location(path.join(IntermediatePath, "Projects/"..projectName))
		targetdir(projectBinaryPath)

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