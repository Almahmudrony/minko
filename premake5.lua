newoption {
	trigger	= 'no-example',
	description = 'Disable examples.'
}

newoption {
	trigger	= 'no-tutorial',
	description = 'Disable tutorials.'
}

newoption {
	trigger	= 'no-framework',
	description = 'Disable plugins.'
}

newoption {
	trigger	= 'no-plugin',
	description = 'Disable plugins.'
}

newoption {
	trigger	= 'no-test',
	description = 'Disable tests.'
}

newoption {
	trigger = 'dist-dir',
	description = 'Output folder for the redistributable SDK built with the \'dist\' action.'
}

newoption {
	trigger = 'platform',
	description = 'Platform for which we want to regroup the binaries with \'regroup\' action.'
}

newoption {
	trigger = 'config',
	description = 'Config for which we want to regroup the binaries with \'regroup\' action.'
}

newoption {
	trigger = 'type',
	description = 'Type of project we want to regroup the binaries with \'regroup\' action (among: example, tutorial, plugin).'
}

newoption {
	trigger = 'regroup-dir',
	description = 'Output folder where we want to regroup the binaries with \'regroup\' action.'
}

solution "minko"
	MINKO_HOME = path.getabsolute(os.getcwd())

	dofile('sdk.lua')

	-- buildable SDK
	MINKO_SDK_DIST = false

	-- framework
	if not _OPTIONS['no-framework'] then
	include 'framework'
	end

	-- tutorial
	if not _OPTIONS['no-tutorial'] then
		include 'tutorial/01-hello-cube'
		include 'tutorial/02-handle-canvas-resizing'
		include 'tutorial/03-rotating-the-camera-around-an-object-with-the-mouse'
		include 'tutorial/04-moving-objects'
		include 'tutorial/05-moving-objects-with-the-keyboard'
		include 'tutorial/06-load-3d-files'
		include 'tutorial/07-loading-scene-files'
		include 'tutorial/08-my-first-script'
		include 'tutorial/09-scripting-mouse-inputs'
		include 'tutorial/10-working-with-the-basic-material'
		include 'tutorial/11-working-with-the-phong-material'
		include 'tutorial/12-working-with-normal-maps'
		include 'tutorial/13-working-with-environment-maps'
		include 'tutorial/14-working-with-specular-maps'
		include 'tutorial/15-loading-and-using-textures'
		include 'tutorial/16-loading-effects'
		include 'tutorial/17-creating-a-custom-effect'
		include 'tutorial/18-creating-custom-materials'
		include 'tutorial/19-binding-the-model-to-world-transform'
		include 'tutorial/20-binding-the-camera'
		include 'tutorial/21-authoring-uber-shaders'
		include 'tutorial/22-creating-a-simple-post-processing-effect'
		include 'tutorial/23-using-external-glsl-code-in-effect-files'
		include 'tutorial/24-working-with-custom-vertex-attributes'
		include 'tutorial/25-working-with-ambient-lights'
		include 'tutorial/26-working-with-directional-lights'
		include 'tutorial/27-working-with-point-lights'
		include 'tutorial/28-working-with-spot-lights'
		include 'tutorial/29-hello-falling-cube'
		include 'tutorial/30-applying-anti-aliasing-effect'

		if os.is("macosx")  and (_ACTION == "xcode-ios" or _ACTION == "xcode-osx") then
			minko.project.library "all-tutorials"
				targetdir "/tmp/minko/bin"
				objdir "/tmp/minko/obj"

				local tutorials = os.matchdirs('tutorial/*')

				for i, basedir in ipairs(tutorials) do
					local tutorialName = path.getbasename(basedir)
					links { "minko-tutorial-" .. tutorialName }
				end
		end
	end

	-- plugin
	if not _OPTIONS['no-plugin'] then
		include 'plugin/android'
		--include 'plugin/lua'
		--include 'plugin/angle'
		include 'plugin/zlib'
		include 'plugin/assimp'
		include 'plugin/devil'
		--include 'plugin/bullet'
		--include 'plugin/fx'
		include 'plugin/html-overlay'
		--include 'plugin/http-loader'
		--include 'plugin/http-worker'
		include 'plugin/jpeg'
		--include 'plugin/leap'
		--include 'plugin/oculus'
		--include 'plugin/offscreen'
		--include 'plugin/particles'
		include 'plugin/png'
		include 'plugin/sdl'
		include 'plugin/serializer'

		-- work around the inability of Xcode to build all projects if no dependency exists between them
		if os.is("macosx")  and (_ACTION == "xcode-ios" or _ACTION == "xcode-osx") then
			minko.project.library "sdk"
				targetdir "/tmp/minko/bin"
				objdir "/tmp/minko/obj"

				links { "minko-framework" }

				local plugins = os.matchdirs('plugin/*')

				for i, basedir in ipairs(plugins) do
					local pluginName = path.getbasename(basedir)
					links { "minko-plugin-" .. pluginName }
				end
		end
	end

	-- example
	if not _OPTIONS['no-example'] then
		--include 'example/assimp'
		-- include 'example/audio'
		include 'example/blending'
		-- include 'example/clone'
		include 'example/cube'
		-- include 'example/devil'
		-- include 'example/effect-config'
		-- include 'example/flares'
		-- include 'example/fog'
		-- include 'example/frustum'
		-- include 'example/hologram'
		-- include 'example/html-overlay'
		-- include 'example/http'
		-- include 'example/jobs'
		-- include 'example/joystick'
		-- include 'example/keyboard'
		-- include 'example/leap-motion'
		-- include 'example/light'
		-- include 'example/line-geometry'
		-- include 'example/lua-scripts'
		-- include 'example/multi-surfaces'
		-- include 'example/oculus'
		-- include 'example/offscreen'
		-- include 'example/particles'
		-- include 'example/physics'
		-- include 'example/picking'
		-- include 'example/raycasting'
		include 'example/serializer'
		-- include 'example/sky-box'
		-- include 'example/stencil'
		-- include 'example/visibility'
		-- include 'example/water'

		if os.is("macosx")  and (_ACTION == "xcode-ios" or _ACTION == "xcode-osx") then
			minko.project.library "all-examples"
				targetdir "/tmp/minko/bin"
				objdir "/tmp/minko/obj"

				local examples = os.matchdirs('example/*')

				for i, basedir in ipairs(examples) do
					local exampleName = path.getbasename(basedir)
					links { "minko-example-" .. exampleName }
				end
		end
	end

	-- test
	if not _OPTIONS['no-test'] then
		include 'test'
	end

newaction {
	trigger		= 'dist',
	description	= 'Generate the distributable version of the Minko SDK.',
	execute		= function()

		-- print("Building documentation...")
		-- os.execute("doxygen doc/Doxyfile")

		local distDir = 'dist'

		if _OPTIONS['dist-dir'] then
			distDir = _OPTIONS['dist-dir']
		end

		os.rmdir(distDir)

		os.mkdir(distDir)
		os.copyfile('sdk.lua', distDir .. '/sdk.lua')

		print("Packaging core framework...")

		-- framework
		os.mkdir(distDir .. '/framework')
		os.mkdir(distDir .. '/framework/bin')
		minko.os.copyfiles('framework/bin', distDir .. '/framework/bin')
		os.mkdir(distDir .. '/framework/include')
		minko.os.copyfiles('framework/include', distDir .. '/framework/include')
		os.mkdir(distDir .. '/framework/lib')
		minko.os.copyfiles('framework/lib', distDir .. '/framework/lib')
		os.mkdir(distDir .. '/framework/asset')
		minko.os.copyfiles('framework/asset', distDir .. '/framework/asset')

		-- skeleton
		os.mkdir(distDir .. '/skeleton')
		minko.os.copyfiles('skeleton', distDir .. '/skeleton')

		-- module
		os.mkdir(distDir .. '/module')
		minko.os.copyfiles('module', distDir .. '/module')

		-- -- doc
		-- os.mkdir(distDir .. '/doc')
		-- minko.os.copyfiles('doc/html', distDir .. '/doc')

		-- tool
		os.mkdir(distDir .. '/tool/')
		minko.os.copyfiles('tool', distDir .. '/tool')

		-- plugin
		local plugins = os.matchdirs('plugin/*')

		os.mkdir(distDir .. '/plugin')
		for i, basedir in ipairs(plugins) do
			local pluginName = path.getbasename(basedir)

			print('Packaging plugin "' .. pluginName .. '"...')

			-- plugins
			local dir = distDir .. '/plugin/' .. path.getbasename(basedir)
			local binDir = dir .. '/bin'

			-- plugin.lua
			assert(os.isfile(basedir .. '/plugin.lua'), 'missing plugin.lua')
			os.mkdir(dir)
			os.copyfile(basedir .. '/plugin.lua', dir .. '/plugin.lua')

			if minko.plugin[pluginName] and minko.plugin[pluginName].dist then
				minko.plugin[pluginName]:dist(dir)
			end

			-- bin
			if os.isdir(basedir .. '/bin') then
				os.mkdir(binDir)
				minko.os.copyfiles(basedir .. '/bin', binDir)
			end

			-- includes
			if os.isdir(basedir .. '/include') then
				os.mkdir(dir .. '/include')
				minko.os.copyfiles(basedir .. '/include', dir .. '/include')
			end

			-- assets
			if os.isdir(basedir .. '/asset') then
				os.mkdir(dir .. '/asset')
				minko.os.copyfiles(basedir .. '/asset', dir .. '/asset')
			end
		end

		minko.action.zip(distDir, distDir .. '.zip')
	end
}

newaction {
	trigger			= "doc",
	description		= "Create developer reference.",
	execute			= function()
		os.execute("doxygen doc/Doxyfile")
	end
}

newaction {
	trigger			= "clean",
	description		= "Remove generated files.",
	execute			= function()
		minko.action.clean()
	end
}

newaction {
	trigger			= "regroup",
	description		= "Regroup all binaries into a single folder",
	execute			= function()

		function Set (list)
		  local set = {}
		  for _, l in ipairs(list) do set[l] = true end
		  return set
		end

		local availablePlatforms = Set { "windows32", "windows64", "linux32", "linux64", "osx64", "html5", "android", "ios" }
		local availableConfig = Set { "debug", "release"}
		local availableType = Set { "example", "tutorial", "plugin" }

		if _OPTIONS['platform'] and availablePlatforms[_OPTIONS['platform']] and
		   _OPTIONS['type'] and availableType[_OPTIONS['type']] and
		   _OPTIONS['config'] and availableConfig[_OPTIONS['config']] and
		   _OPTIONS['regroup-dir'] then

			local platform = _OPTIONS['platform']
			local projectType = _OPTIONS['type']
			local config = _OPTIONS['config']
			local outputDir = _OPTIONS['regroup-dir']

			local completeOutputDir = outputDir .. '/' .. platform .. '/' .. config .. '/' .. projectType
			os.mkdir(completeOutputDir)

			local dirs = os.matchdirs(projectType .. '/*')

			for i, basedir in ipairs(dirs) do

			    local dirName = path.getbasename(basedir)
				local sourceDir = projectType .. '/' .. dirName .. '/bin/' .. platform .. '/' .. config
				if os.isdir(sourceDir) then
					print(dirName)

					os.mkdir(completeOutputDir .. '/' .. dirName)

					if platform == 'android' then
						if os.isdir(sourceDir .. '/bin/artifacts') then
							minko.os.copyfiles(sourceDir .. '/bin/artifacts', completeOutputDir .. '/' .. dirName)
						end
					else
						minko.os.copyfiles(sourceDir, completeOutputDir .. '/' .. dirName)
					end
				end
			end
		else
			print "Error: Some arguments are missing or are not correct. Please follow the usage."
			print "Usage: regroup --platform=$1 --config=$2 --type=$3 --regroup-dir=$4"
			print " \$1: the platform (windows32, windows64, linux32, linux64, osx64, html5, android or ios)"
			print " \$2: the configuration (debug or release)"
			print " \$3: the type of projects you want to pack (example, tutorial or plugin)"
			print " \$4: the output directory where you want to copy all files"
		end
	end
}
