-- if not minko.platform.supports { "linux32", "linux64" } then
-- 	return
-- end

PROJECT_NAME = path.getname(os.getcwd())

minko.project.application("minko-example-" .. PROJECT_NAME)

	removeplatforms { "ios", "android" }

	files {
		"src/**.hpp",
		"src/**.cpp",
		"asset/**"
	}

	includedirs { "src" }

	minko.plugin.enable("sdl")
	minko.plugin.enable("png")
	minko.plugin.enable("offscreen")
