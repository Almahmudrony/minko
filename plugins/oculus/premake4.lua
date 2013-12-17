newoption {
	trigger		= "with-oculus",
	description	= "Enable the Minko OCULUS plugin."
}

PROJECT_NAME = path.getname(os.getcwd())

minko.project.library("minko-plugin-" .. PROJECT_NAME)
	kind "StaticLib"
	language "C++"
	files { 
		"**.hpp", 
		"**.h", 
		"**.cpp", 
		"**.c", 
		"include/**.hpp",
		"lib/LibOVR/Src/**"
	}
	includedirs {
		"include",
		"src",
		"lib/LibOVR/Include",
		"lib/LibOVR/Src"
	}
	
	excludes { "lib/LibOVR/Include/OVRVersion.h" }
	
	configuration { "windows" }
		excludes {
			"lib/LibOVR/Src/OVR_Linux_*",
			"lib/LibOVR/Src/OVR_OSX_*",
			"lib/LibOVR/Src/Kernel/OVR_ThreadsPthread.cpp"
		}
		defines { "UNICODE", "_UNICODE" } -- should also undefine _MCBS
		
	configuration { "linux" }
		excludes {
			"lib/LibOVR/Src/OVR_Win32_*",
			"lib/LibOVR/Src/OVR_OSX_*",
			"lib/LibOVR/Src/Kernel/OVR_ThreadsWinAPI.cpp"
		}
	
	configuration { "macosx" }
		excludes {
			"lib/LibOVR/Src/OVR_Win32_*",
			"lib/LibOVR/Src/OVR_Linux_*",
			"lib/LibOVR/Src/Kernel/OVR_ThreadsWinAPI.cpp"
		}
		
	configuration { }
