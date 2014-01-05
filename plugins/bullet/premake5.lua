newoption {
	trigger		= "with-bullet",
	description	= "Enable the Minko Bullet plugin."
}

PROJECT_NAME = path.getname(os.getcwd())

minko.project.library("minko-plugin-" .. PROJECT_NAME)

	kind "StaticLib"
	language "C++"

	files {
		"lib/**.hpp",
		"lib/**.h",
		"lib/**.cpp",
		"lib/**.c",
		"src/**.cpp",
		"include/**.hpp"
	}

	includedirs {
		"include",
		"lib/bullet2/src"
	}

	excludes {
		"lib/bullet2/src/BulletMultiThreaded/*.h",
		"lib/bullet2/src/BulletMultiThreaded/*.cpp",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/*.h",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/*.cpp",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/DX11/*.h",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/DX11/*.cpp",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/*.h",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/*.cpp",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/MiniCL/*.h",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/OpenCL/MiniCL/*.cpp",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/Shared/*.h",
		"lib/bullet2/src/BulletMultiThreaded/GpuSoftBodySolvers/Shared/*.cpp",
		"lib/bullet2/src/BulletMultiThreaded/SpuNarrowPhaseCollisionTask/*.h",
		"lib/bullet2/src/BulletMultiThreaded/SpuNarrowPhaseCollisionTask/*.cpp",
		"lib/bullet2/src/BulletMultiThreaded/SpuSampleTask/*.h",
		"lib/bullet2/src/BulletMultiThreaded/SpuSampleTask/*.cpp"
	}

	-- linux
	configuration { "linux" }
		buildoptions { "-Wno-narrowing -Wno-int-to-pointer-cast" }

	-- windows
	configuration { "win" }
		defines {
			"_CRT_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE"
		}

	-- macosx
	configuration { "osx" }
		buildoptions { "-Wno-narrowing -Wno-int-to-pointer-cast" }

	-- emscripten
	configuration { "html5" }
		buildoptions { "-Wno-narrowing -Wno-int-to-pointer-cast" }
