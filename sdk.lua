if not MINKO_HOME then
	if os.getenv('MINKO_HOME') then
		MINKO_HOME = os.getenv('MINKO_HOME');
	else
		print(color.fg.red .. 'You must define the environment variable MINKO_HOME.' .. color.reset)
		os.exit(1)
	end
end

if not os.isfile(MINKO_HOME .. '/sdk.lua') then
	print(color.fg.red ..'MINKO_HOME does not point to a valid Minko SDK.' .. color.reset)
	os.exit(1)
end

print('Minko SDK home directory: ' .. MINKO_HOME)

package.path = MINKO_HOME .. "/modules/?/?.lua;".. package.path

configurations { "debug", "release" }
platforms { "linux32", "linux64", "windows32", "windows64", "osx64", "html5", "ios", "android" }

require 'emscripten'
require 'android'
require 'vs2013ctp'

local insert = require 'insert'

insert.insert(premake.tools.gcc, 'cxxflags.system', {
	linux = { "-MMD", "-MP", "-std=c++11" },
	macosx = { "-MMD", "-MP", "-std=c++11" },
	emscripten = { "-MMD", "-MP", "-std=c++11" }
})

insert.insert(premake.tools.gcc, 'tools.linux', {
	ld = MINKO_HOME .. '/tools/lin/bin/g++-ld.sh',
	cxx = MINKO_HOME .. '/tools/lin/bin/g++-ld.sh'
})

insert.insert(premake.tools.clang, 'cxxflags.system', {
	macosx = { "-MMD", "-MP", "-std=c++11", "-stdlib=libc++" }
})

insert.insert(premake.tools.clang, 'ldflags.system.macosx', {
	macosx = { "-stdlib=libc++" }
})

configuration { "windows32" }
	system "windows"
	architecture "x32"

configuration { "windows64" }
	system "windows"
	architecture "x64"

configuration { "linux32" }
	system "linux"
	architecture "x32"

configuration { "linux64" }
	system "linux"
	architecture "x64"

configuration { "osx64" }
	system "macosx"

configuration { "html5" }
	system "emscripten"

configuration { "android"}
	system "android"

configuration {}

-- print(table.inspect(premake.tools.clang))

-- distributable SDK
MINKO_SDK_DIST = true

-- import build system utilities
dofile(MINKO_HOME .. '/tools/all/lib/minko.lua')
dofile(MINKO_HOME .. '/tools/all/lib/minko.sdk.lua')
dofile(MINKO_HOME .. '/tools/all/lib/minko.os.lua')
dofile(MINKO_HOME .. '/tools/all/lib/minko.path.lua')
dofile(MINKO_HOME .. '/tools/all/lib/minko.plugin.lua')
dofile(MINKO_HOME .. '/tools/all/lib/minko.vs.lua')
dofile(MINKO_HOME .. '/tools/all/lib/minko.project.lua')

newoption {
	trigger	= "no-stencil",
	description = "Disable all stencil operations."
}

if _OPTIONS["no-stencil"] then
	defines { "MINKO_NO_STENCIL" }
end
