--------------------------------------------------------------
-- @Description : Makefile of CatDog Engine's dependencies
--------------------------------------------------------------
group "ThirdParty/sdl"
	DeclareExternalProject("SDL2", "SharedLib", "sdl")
		dependson { "SDL2main", "SDL2-static" }
	DeclareExternalProject("SDL2main", "StaticLib", "sdl")
	DeclareExternalProject("SDL2-static", "StaticLib", "sdl")
	DeclareExternalProject("sdl_headers_copy", "Utility", "sdl")

local bgfxProjectsPath = path.join(ThirdPartySourcePath, "bgfx/.build/projects/"..IDEConfigs.BuildIDEName)
group "ThirdParty/bgfx"
	externalproject("bgfx")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("bx")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("bimg")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("bimg_encode")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("bimg_decode")
		kind("StaticLib")
		location(bgfxProjectsPath)

group "ThirdParty/bgfx/examples"

print("Load bgfx examples...")
local allProjects = os.matchfiles(bgfxProjectsPath.."/example-*.vcxproj")
for _, v in ipairs(allProjects) do
	local projectName = path.getbasename(v)
	print(projectName)
	externalproject(projectName)
		kind("ConsoleApp")
		location(bgfxProjectsPath)
end

group "ThirdParty/bgfx/tools"
	externalproject("geometryc")
		kind("ConsoleApp")
		location(bgfxProjectsPath)

	externalproject("geometryv")
		kind("ConsoleApp")
		location(bgfxProjectsPath)

	externalproject("texturec")
		kind("ConsoleApp")
		location(bgfxProjectsPath)

	externalproject("texturev")
		kind("ConsoleApp")
		location(bgfxProjectsPath)

group "ThirdParty/bgfx/tools/shaderc"
	externalproject("fcpp")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("glslang")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("glsl-optimizer")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("shaderc")
		kind("ConsoleApp")
		location(bgfxProjectsPath)

	externalproject("spirv-cross")
		kind("StaticLib")
		location(bgfxProjectsPath)

	externalproject("spirv-opt")
		kind("StaticLib")
		location(bgfxProjectsPath)

group ""
print("================================================================")