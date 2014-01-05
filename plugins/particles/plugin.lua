--[[
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
]]--

-- particles plugin
minko.plugin.particles = {}

function minko.plugin.particles:enable()
	minko.plugin.links { "particles" }
	includedirs { minko.sdk.path("plugins/particles/include") }
	defines { "MINKO_PLUGIN_PARTICES" }
	
	configuration { "win" }
		postbuildcommands {
			'xcopy /y /i "' .. minko.sdk.path('/plugins/particles/asset/effect') .. '" "$(TargetDir)\\effect"',
		}
		
	configuration { "linux" }
		postbuildcommands {
			'cp -r ' .. minko.sdk.path('/plugins/particles/asset/effect') .. ' ${TARGETDIR} || ' .. minko.fail(),
		}
	
	configuration { "osx" }
		postbuildcommands {
			'cp -r ' .. minko.sdk.path('/plugins/particles/asset/effect') .. ' ${TARGETDIR} || ' .. minko.fail(),
		}	

	configuration { "html5" }
		prelinkcommands {
			'cp -r ' .. minko.sdk.path('/plugins/particles/asset/effect') .. ' ${TARGETDIR} || ' .. minko.fail(),
		}	
end
