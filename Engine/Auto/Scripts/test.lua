--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine's regression tests
--------------------------------------------------------------

-- Example Projects
TestsPath = path.join(RootPath, "Tests")
print("Make tests : "..TestsPath)

function MakeTest(testName)
	local testSourcePath = path.join(TestsPath, testName)

	project(testName)
		kind("ConsoleApp")
		language("C++")
		cppdialect("C++20")
		
		location(path.join(IntermediatePath, "Tests/"..testName))
		targetdir(BinariesPath)

		files {
			path.join(testSourcePath, "**.*"),
		}

		vpaths {
			["Source"] = { path.join(testSourcePath, "**.*") },
		}

		includedirs {
			path.join(EngineSourcePath, "Runtime/"),
			path.join(ThirdPartySourcePath, "AssetPipeline/public"),
			path.join(EnginePath, "BuiltInShaders/UniformDefines"),
		}

		-- convenient to test multiple threads
		openmp("On")

		-- use /MT /MTd, not /MD /MDd
		staticruntime "on"
		filter { "configurations:Debug" }
			runtime "Debug" -- /MTd
		filter { "configurations:Release" }
			runtime "Release" -- /MT
		filter {}
end

group "Tests"

local allTests = os.matchdirs(TestsPath.."/*")
for _, v in ipairs(allTests) do
	local testName = path.getname(v)
	print("Make test : "..testName)
	MakeTest(testName)
end

group ""
print("================================================================")