--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine's packaging
--------------------------------------------------------------

group("Build")

-- Describes the packaging step 
project("com.cdengine.game")
	-- Build an APK file
	kind("Packaging")

	-- Put the APK packaging project to this directory
	location("build/com.cdengine.game")

	-- The dependent project are built first.
	dependson {
		"Engine"
	}

	-- This adds the listed projects into the "references" section in Visual Studio.
	links {
		"Engine",
	}

	-- Files included in the build, some are about configuration, others are sources/resources.
	-- Premake plugin for Android packaging recognizes some special files.
	files {
		-- ANT build configuration.
		path.join(AutoPlatformsPath, "Android/build.xml"),

		-- ANT project configuration.
		path.join(AutoPlatformsPath, "Android/project.properties"),

		-- Manifest file.
		path.join(AutoPlatformsPath, "Android/AndroidManifest.xml"),

		-- Java source files.
		path.join(EngineSourcePath, "Platforms/Android/MainActivity.java"),
	}

group("")