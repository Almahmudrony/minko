newoption {
	trigger		= "with-serializer",
	description	= "Enable the Minko SERIALIZER plugin."
}

PROJECT_NAME = path.getname(os.getcwd())

minko.project.library("minko-plugin-" .. PROJECT_NAME)
	kind "StaticLib"
	language "C++"
	files { "**.hpp", "**.h", "**.cpp", "**.c", "include/**.hpp", "lib/msgpack-c/include/**.hpp" }
	includedirs {
		"include",
		"src",
		"lib/msgpack-c/include",
		"lib/msgpack-c/src"
	}

	configuration { "windows" }
		-- msgpack
		defines {
			"_LIB",
			"_CRT_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE",
			"__STDC_VERSION__=199901L",
			"__STDC__",
			"WIN32"
		}
		buildoptions {
			"/wd4028",
			"/wd4244",
			"/wd4267",
			"/wd4996",
			"/wd4273",
			"/wd4503"
		}

	configuration { "linux" }
		buildoptions {
			"-Wno-deprecated-declarations"
		}
		defines {
			"__STDC_FORMAT_MACROS"
		}
