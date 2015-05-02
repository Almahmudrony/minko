PROJECT_NAME = path.getname(os.getcwd())

minko.project.application("minko-example-" .. PROJECT_NAME)

	files {
		"src/**.cpp",
		"src/**.hpp",
		"asset/**"
	}

	minko.package.assets {
		['**.effect'] = { 'embed' },
		['**.glsl'] = { 'embed' },
		['**.scene'] = { 'embed' }
	}

	includedirs { "src" }

	-- plugins
	minko.plugin.enable("assimp")
	minko.plugin.enable("sdl")
	--minko.plugin.enable("bullet")
	--minko.plugin.enable("jpeg")
	minko.plugin.enable("serializer")
	--minko.plugin.enable("particles")
	minko.plugin.enable("png")
	minko.plugin.enable("fx")
