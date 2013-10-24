solution "minko"
   configurations { "debug", "release" }

dofile('sdk.lua')

-- core framework
include 'framework'

-- plugins
include 'plugins/jpeg'
include 'plugins/png'
include 'plugins/mk'
include 'plugins/bullet'
include 'plugins/particles'
include 'plugins/sdl'
include 'plugins/angle'
if _OPTIONS["platform"] == "emscripten" then
	include 'plugins/webgl'
end

-- examples
include 'examples/sponza'
include 'examples/cube'
include 'examples/light'
--include 'examples/cube-offscreen'

-- tests
if _ACTION ~= "vs2010" and _OPTIONS["platform"] ~= "emscripten" then
	include 'tests'
end
