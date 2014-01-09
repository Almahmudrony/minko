include "lib/googletest"

minko.project.application "minko-tests"
	-- removeplatforms { "html5" } @fixme broken for Windows

	kind "ConsoleApp"
	language "C++"
	files {
		"src/**.hpp",
		"src/**.cpp"
	}
	includedirs { "src" }
	defines { "MINKO_TESTS" }

	-- plugins
	minko.plugin.enable("sdl")
	minko.plugin.enable("serializer")

	-- googletest framework
	links { "googletest" }
	includedirs { "lib/googletest/include" }

	configuration { "linux" }
		links { "pthread" }
