newoption {
	trigger			= "with-webgl",
	description		= "Enable the Minko WebGL plugin."
}

PROJECT_NAME = path.getname(os.getcwd())

minko.project.library("minko-plugin-" .. PROJECT_NAME)

	removeplatforms { "osx", "win", "ios", "linux", "android" }

	kind "StaticLib"
	language "C++"

	files {
		"lib/**.hpp",
		"lib/**.h",
		"lib/**.cpp",
		"lib/**.c",
		"include/**.hpp",
		"src/**.cpp"
	}

	includedirs { "include" }

	minko.plugin.import("sdl")
