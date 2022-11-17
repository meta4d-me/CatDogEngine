--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine's dependencies
--------------------------------------------------------------

-- SDL caused a strange x64 folder generated in the root folder.
-- You can enable these settings if you want to debug conveniently.
--group "ThirdParty/sdl"
--	local sdlBuildPath = path.join(ThirdPartySourcePath, "sdl/build")
--	externalproject("SDL2")
--		kind("SharedLib")
--		location(sdlBuildPath)
--		targetdir(sdlBuildPath)
--		dependson { "SDL2main", "SDL2-static" }
--
--	externalproject("SDL2main")
--		kind("StaticLib")
--		location(sdlBuildPath)
--		targetdir(sdlBuildPath)
--
--	externalproject("SDL2-static")
--		kind("StaticLib")
--		location(sdlBuildPath)
--		targetdir(sdlBuildPath)
--
--	externalproject("sdl_headers_copy")
--		kind("Utility")
--		location(sdlBuildPath)
--		targetdir(sdlBuildPath)

local bgfxProjectsPath = path.join(ThirdPartySourcePath, "bgfx/.build/projects/"..IDEConfigs.BuildIDEName)
group "ThirdParty/bgfx"
	externalproject("bgfx")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("bx")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("bimg")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("bimg_encode")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("bimg_decode")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

group "ThirdParty/bgfx/examples"

print("Load bgfx examples...")
local allProjects = os.matchfiles(bgfxProjectsPath.."/example-*.vcxproj")
for _, v in ipairs(allProjects) do
	local projectName = path.getbasename(v)
	print(projectName)
	externalproject(projectName)
		kind("ConsoleApp")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)
end

group "ThirdParty/bgfx/tools"
	externalproject("geometryc")
		kind("ConsoleApp")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("geometryv")
		kind("ConsoleApp")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("texturec")
		kind("ConsoleApp")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("texturev")
		kind("ConsoleApp")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

group "ThirdParty/bgfx/tools/shaderc"
	externalproject("fcpp")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("glslang")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("glsl-optimizer")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("shaderc")
		kind("ConsoleApp")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("spirv-cross")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

	externalproject("spirv-opt")
		kind("StaticLib")
		location(bgfxProjectsPath)
		targetdir(BinariesPath)

group ""
print("================================================================")