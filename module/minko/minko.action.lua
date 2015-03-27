minko.action = {}

minko.action.fail = function()
	if os.is('windows') then
		if string.startswith(_ACTION, "gmake") then
			return 'call "' .. path.translate(minko.sdk.path('/tool/win/script/fail.bat')) .. '" ${TARGET}'
		else
			return 'call "' .. path.translate(minko.sdk.path('/tool/win/script/fail.bat')) .. '" "$(Target)"'
		end
	elseif os.is('macosx') then
		return 'bash ' .. minko.sdk.path('/tool/mac/script/fail.sh') .. ' ${TARGET}'		
	else
		return 'bash ' .. minko.sdk.path('/tool/lin/script/fail.sh') .. ' ${TARGET}'
	end
end

minko.action.copy = function(sourcePath)
	local cygwinEnv = false
	if string.startswith(os.getenv('OSTYPE'), 'CYGWIN') then
		cygwinEnv = true
	end

	if os.is('windows') and not cygwinEnv then
		sourcePath = path.translate(sourcePath)

		local targetDir = string.startswith(_ACTION, "gmake") and '$(subst /,\\,$(TARGETDIR))' or '$(TargetDir)'

		if os.isdir(sourcePath) then
			targetDir = targetDir .. '\\' .. path.getbasename(sourcePath)
		end

		local existenceTest = string.find(sourcePath, '*') and '' or ('if exist "' .. sourcePath .. '" ')

		return existenceTest .. 'xcopy /y /i /e "' .. sourcePath .. '" "' .. targetDir .. '"'
	elseif os.is("macosx") then
		local targetDir = '${TARGETDIR}'

		if (_ACTION == "xcode-osx") then
			targetDir = '${TARGET_BUILD_DIR}'
		elseif (_ACTION == "xcode-ios") then
			targetDir = '${TARGET_BUILD_DIR}/${TARGET_NAME}.app'
		end

		local existenceTest = string.find(sourcePath, '*') and '' or ('test -e ' .. sourcePath .. ' && ')

		return existenceTest .. 'cp -R ' .. sourcePath .. ' "' .. targetDir .. '" || :'
	else
		local targetDir = '${TARGETDIR}'
		if cygwinEnv then
			sourcePath = os.capture('cygpath -u "' .. sourcePath .. '"')
			targetDir = os.capture('cygpath -u "' .. targetDir .. '"')
		end

		local existenceTest = string.find(sourcePath, '*') and '' or ('test -e ' .. sourcePath .. ' && ')

		return existenceTest .. 'cp -R ' .. sourcePath .. ' "' .. targetDir .. '" || :'
	end
end

minko.action.link = function(sourcePath)
	local cygwinEnv = false
	if string.startswith(os.getenv('OSTYPE'), 'CYGWIN') then
		cygwinEnv = true
	end

	if os.is('windows') and not cygwinEnv then
		-- fixme: not needed yet
	elseif os.is("macosx") then
		local targetDir = '${TARGETDIR}'

		if (_ACTION == "xcode-osx") then
			targetDir = '${TARGET_BUILD_DIR}'
		elseif (_ACTION == "xcode-ios") then
			targetDir = '${TARGET_BUILD_DIR}/${TARGET_NAME}.app'
		end

		local existenceTest = string.find(sourcePath, '*') and '' or ('test -e ' .. sourcePath .. ' && ')

		return existenceTest .. 'ln -s -f ' .. sourcePath .. ' "' .. targetDir .. '" || :'
	else
		local targetDir = '${TARGETDIR}'
		if cygwinEnv then
			sourcePath = os.capture('cygpath -u "' .. sourcePath .. '"')
			targetDir = os.capture('cygpath -u "' .. targetDir .. '"')
		end

		local existenceTest = string.find(sourcePath, '*') and '' or ('test -e ' .. sourcePath .. ' && ')

		return existenceTest .. 'ln -s -f ' .. sourcePath .. ' "' .. targetDir .. '" || :'
	end
end

minko.action.clean = function()
	if not os.isfile("sdk.lua") then
		error("cannot clean from outside the Minko SDK")
	end

	local cmd = 'git clean -X -d -f'

	os.execute(cmd)
	
	for _, pattern in ipairs { "framework", "plugin/*", "test", "example/*" } do
		local dirs = os.matchdirs(pattern)

		for _, dir in ipairs(dirs) do
			local cwd = os.getcwd()
			os.chdir(dir)
			os.execute(cmd)
			os.chdir(cwd)
		end
	end
end

minko.action.zip = function(directory, archive)
	if os.is('windows') then
		os.execute('7za a "' .. archive .. '" "' .. path.translate(directory) .. '"')
	else
		os.execute('zip -r "' .. archive .. '" "' .. directory .. '"')
	end
end

minko.action.cpjf = function(absoluteSrcDir, relativeDestDir)
	local scriptLocation = MINKO_HOME .. '/tool/lin/script/cpjf.sh';

	if string.startswith(os.getenv('OSTYPE'), 'CYGWIN') then
		scriptLocation = os.capture('cygpath -u "' .. scriptLocation .. '"')
		absoluteSrcDir = os.capture('cygpath -u "' .. absoluteSrcDir .. '"')
	end

	return 'bash ' .. scriptLocation .. ' ' .. absoluteSrcDir .. ' ' .. relativeDestDir .. ' || ' .. minko.action.fail()
end

minko.action.buildandroid = function()
	local minkoHome = MINKO_HOME

	if string.startswith(os.getenv('OSTYPE'), 'CYGWIN') then
		minkoHome = os.capture('cygpath -u "' .. minkoHome .. '"')
	end

	return 'bash ' .. minkoHome .. '/tool/lin/script/build_android.sh ${TARGET} || ' .. minko.action.fail()
end