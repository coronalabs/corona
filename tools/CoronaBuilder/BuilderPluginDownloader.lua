------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------


local json = require('json')
local lfs = require('lfs')
local builder = require('builder')

local serverBackend = 'https://backendapi.coronalabs.com'
local verbosity = 3
local androidBuild = false
local alwaysQuery = false

local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

local windows = (package.config:match("^.") == '\\')
local dirSeparator = package.config:sub(1,1)

local function unpackPlugin( archive, dst )
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" x ' .. quoteString(archive) .. ' -so  2> nul | "%CORONA_PATH%\\7za.exe" x -aoa -si -ttar -o' .. quoteString(dst) .. ' 2> nul "'
		return os.execute(cmd)
	else
		return os.execute('/usr/bin/tar -xzf ' .. quoteString(archive) .. ' -C ' ..  quoteString(dst))
	end
end

local function getPluginDirectories(platform, build, pluginsToDownload)

	local pluginsDest
	if windows then
		-- %APPDATA%\Corona Labs\Corona Simulator\NativePlugins\
		pluginsDest = os.getenv('APPDATA') .. '\\Corona Labs'
		lfs.mkdir(pluginsDest)
		pluginsDest = pluginsDest .. '\\Corona Simulator'
		lfs.mkdir(pluginsDest)
		pluginsDest = pluginsDest .. '\\NativePlugins\\'
		lfs.mkdir(pluginsDest)
		pluginsDest = pluginsDest .. platform .. '\\'
		lfs.mkdir(pluginsDest)
	else
		pluginsDest = os.getenv('HOME') .. '/Library/Application Support/Corona'
		lfs.mkdir(pluginsDest)
		pluginsDest = pluginsDest .. '/Native Plugins/'
		lfs.mkdir(pluginsDest)
		pluginsDest = pluginsDest .. platform .. '/'
		lfs.mkdir(pluginsDest)
	end

	local pluginDirectories = {}

	for _, pd in pairs(pluginsToDownload) do
		local plugin, developer, supportedPlatforms = unpack( pd )
		local supportedPlatform = true
		if supportedPlatforms then
			supportedPlatform = supportedPlatforms[platform]
			if platform == 'iphone' then
				supportedPlatform = supportedPlatform or supportedPlatforms['ios']
			end
		end

		if supportedPlatform then
			local downloadURL
			if type(supportedPlatform) == 'table' and type(supportedPlatform.url) == 'string' then
				downloadURL = supportedPlatform.url
			else
				local downloadInfoURL = serverBackend .. '/v1/plugins/download/' .. developer .. '/' .. plugin .. '/' .. build .. '/' .. platform
				local downloadInfoText, msg = builder.fetch(downloadInfoURL)
				if not downloadInfoText then
					print("ERROR: unable to fetch plugin download location for " .. plugin .. ' ('.. developer.. '). Error message: ' .. msg )
					return
				end

				local downloadInfoJSON = json.decode(downloadInfoText)
				downloadURL = downloadInfoJSON.url
			end
			if not downloadURL then
				print("ERROR: unable to parse plugin download location for " .. plugin .. ' ('.. developer.. ').')
				return
			end

			if androidBuild then
				print("plugin\t" .. plugin .. '_' .. developer .. ".tgz\t"  .. downloadURL)
			else
				local pluginArchivePath = pluginsDest .. plugin .. '_' .. developer .. ".tgz"
				local err, msg = builder.download(downloadURL, pluginArchivePath)
				if msg then
					print("ERROR: unable to download " .. plugin .. ' ('.. developer.. '). Error message: ' .. msg )
					return
				end

				local unpackLocation = pluginsDest .. plugin .. '_' .. developer
				lfs.mkdir(unpackLocation)
				local ret = unpackPlugin(pluginArchivePath, unpackLocation)
				if ret ~= 0 then
					print("ERROR: unable to unpack plugin " .. plugin .. ' (' .. developer .. ').')
					return
				end

				table.insert(pluginDirectories, unpackLocation)
			end

		end
	end

	return pluginDirectories

end


local function androidDownloadPlugins( platform, build, pluginsToDownload )

	local pluginDirectories = getPluginDirectories(platform, build, pluginsToDownload)
	if not pluginDirectories then
		return
	end

	if #pluginDirectories > 0 and verbosity>0 then
		print()
		print("Plugins were successfully downloaded")
		print("General guidelines on how to use plugins with Corona Native can be found here: https://docs.coronalabs.com/native/android/index.html")
		print("Plugins may have special instructions for integrating them into an Android Studio project, please refer to the individual plugin documentation before integration")
		print("Plugin download locations:")
		for _, pluginDir in pairs(pluginDirectories) do
			print('\t"' .. pluginDir .. '"')
		end
	end

	return true
end


local function iOSDownloadPlugins( sdk, platform, build, pluginsToDownload, forceLoad )
	-- download plugins and unpack them
	local pluginDirectories = getPluginDirectories(platform, build, pluginsToDownload)
	if not pluginDirectories then
		return
	end

	local nativePlugins = {}
	-- local luaPlugins = {}
	local additionalResources = {}

	-- local detect Lua plugins and Native plugins

	for _, pluginDir in pairs(pluginDirectories) do
		local metadataChunk = loadfile( pluginDir .. '/metadata.lua' )
		if metadataChunk then
			local metadata = metadataChunk()
			local plugin = metadata.plugin
			plugin.path = pluginDir
			table.insert( nativePlugins, plugin )
			local resourcesDir = pluginDir .. '/resources'
			if lfs.attributes( resourcesDir, "mode" ) == "directory" then
				table.insert( additionalResources, resourcesDir )
			end
		end
	end

	for _, pluginDir in pairs(pluginDirectories) do
		local assetPath = pluginDir .. '/lua/lua_51/'
		local isLuaPlugin = lfs.attributes( assetPath, "mode" ) == "directory"
		if isLuaPlugin then
			-- table.insert( luaPlugins, assetPath )
			table.insert( additionalResources, assetPath )
		end
	end

	-- generate config file entries
	local staticLibs = {}
	local searchPaths = {}
	local frameworkSearchPaths = {}
	local fullPathToLib = {}
	local fullPathToLib = {}

	local frameworks = {}
	local frameworksWeak = {}
	local usesSwift = false

	for _, plugin in pairs(nativePlugins) do

		-- Add plugin's static lib
		for _, lib in pairs(plugin.staticLibs) do
			if forceLoad then
				staticLibs[' -force_load ' .. quoteString(plugin.path .. '/lib' .. lib .. '.a')] = true
			else
				staticLibs[' -l' .. lib] = true
				searchPaths[plugin.path] = true
			end
		end

		for _, lib in pairs(plugin.frameworks) do
			frameworks[lib] = true
			frameworkSearchPaths[plugin.path] = true
		end

		for _, lib in pairs(plugin.frameworksOptional) do
			frameworksWeak[lib] = true
			frameworkSearchPaths[plugin.path] = true
		end

		usesSwift = usesSwift or plugin.usesSwift
	end


	-- generate xcconfig entries file
	local configStrings = ""

	local ldFlags = ""

	for lib, _ in pairs(staticLibs) do
		ldFlags = ldFlags .. lib
	end
	for f, _ in pairs(frameworks) do
		ldFlags = ldFlags .. ' -framework ' .. f
	end
	for f, _ in pairs(frameworksWeak) do
		ldFlags = ldFlags .. ' -weak_framework ' .. f
	end

	if #ldFlags > 0 then
		configStrings = configStrings .. 'OTHER_LDFLAGS[sdk=' .. sdk .. '*] = $(inherited)' .. ldFlags .. '\n'
	end

	if usesSwift then
		configStrings = configStrings .. 'ALWAYS_EMBED_SWIFT_STANDARD_LIBRARIES[sdk=' .. sdk .. '*] = YES\n'
	end


	local searchPathsConcat = ""
	for p, _ in pairs(searchPaths) do
		searchPathsConcat = searchPathsConcat .. ' ' .. quoteString(p)
	end
	if #searchPathsConcat > 0 then
		configStrings = configStrings .. 'LIBRARY_SEARCH_PATHS[sdk=' .. sdk .. '*] = $(inherited) ' .. searchPathsConcat .. '\n'
	end

	local frameworkSearchPathConcat = ""
	for p, _ in pairs(frameworkSearchPaths) do
		frameworkSearchPathConcat = frameworkSearchPathConcat .. ' ' .. quoteString(p)
	end
	if #frameworkSearchPathConcat > 0 then
		configStrings = configStrings .. 'FRAMEWORK_SEARCH_PATHS[sdk=' .. sdk .. '*] = $(inherited) ' .. frameworkSearchPathConcat .. '\n'
	end

	-- lua plugin entries
	local luaPluginEntries = ""
	local luaPluginEntries = table.concat( additionalResources, ':' )
	if #luaPluginEntries > 0 then
		configStrings = configStrings .. 'CORONA_PLUGIN_RESOURCES[sdk=' .. sdk .. '*] =' .. luaPluginEntries .. '\n'
	end


	return configStrings

end

function fetchDependenciesForDirectories(root, deps, urlSuffix)
	local fetched = {}
	local toFetch = {}
	for i=1, #deps do
		local dep = deps[i]
		fetched[dep] = true
	end
	for i=1, #deps do
		local dep = deps[i]
		local toDownload = {}
		local metadataFile = root .. dirSeparator .. dep .. dirSeparator .. "metadata.lua"
		pcall( function()
			local metadata = dofile(root .. dirSeparator .. dep .. dirSeparator .. "metadata.lua")
			toDownload = metadata.coronaManifest.dependencies
		end	)
		for plugin, developer in pairs(toDownload) do
			if not fetched[plugin .. '_' .. developer] then
				toFetch[plugin .. '_' .. developer] = {plugin, developer}
			end
		end
	end

	for _, pd in pairs(toFetch) do
		local plugin, developer = unpack(pd)
		local downloadInfoURL = serverBackend .. '/v1/plugins/download/' .. developer .. '/' .. plugin .. urlSuffix
		local downloadInfoText, msg = builder.fetch(downloadInfoURL)
		if not downloadInfoText then
			print("ERROR: unable to fetch plugin download location for " .. plugin .. ' ('.. developer.. '). Error message: ' .. msg )
			return 1
		end

		local downloadInfoJSON = json.decode(downloadInfoText)
		local downloadURL = downloadInfoJSON.url
		if not downloadURL then
			print("ERROR: unable to parse plugin download location for " .. plugin .. ' ('.. developer.. ').')
			return 1
		end
		print("plugin\t" .. plugin .. '_' .. developer .. ".tgz\t"  .. downloadURL)
	end
	return 0
end

-- in offline build `user` is nil
function DownloadPluginsMain(args, user, buildYear, buildRevision)
	if args[1] ~= 'download' then
		print("ERROR: unknows subcommand to 'plugins' command: '" .. tostring(args[1]) .. "'. Only 'download' is currently supported." )
		return 1
	end

	local buildDataPluginEntry = {}
	local buildData = {}
	local forceLoad = false
	local fetchDependencies = false
	for i=#args,1,-1 do
		if args[i] == '--force-load' then
			table.remove(args, i)
			forceLoad = true
		elseif args[i] == '--android-build' then -- this switches into android build mode. Emits output instead of downloading anything
			table.remove(args, i)
			verbosity = 0
			androidBuild = true
		elseif args[i] == '--build-data' then -- build data contains info about additional plugins and metadata
			table.remove(args, i)
			buildData = json.decode(io.read('*all')) or {}
			buildDataPluginEntry = buildData.plugins or {}
		elseif args[i] == '--fetch-dependencies' then -- scans directory for dependencies
			table.remove(args, i)
			verbosity = 0
			fetchDependencies = true
		elseif args[i] == '--always-query' then -- forces always to query for available plugins, for test purposes mostly
			table.remove(args, i)
			alwaysQuery = true
		elseif args[i] == '--build' then --verrides buildYear and buildRevision
			table.remove(args, i)
			local build = args[i]
			table.remove(args, i)
			build = (build or ""):gmatch('(%d+)%.(%d+)')
			local y,b = build()
			if y and b then
				buildYear = y
				buildRevision = b
			end
		elseif args[i] == '--' then
			break
		end
	end

	local platform = args[2]

	if fetchDependencies then
		table.remove(args, 1)
		table.remove(args, 1)
		local root = args[1]
		table.remove(args, 1)
		local urlSuffix = ('/%s.%s/%s/'):format(buildYear, buildRevision, platform)
		return fetchDependenciesForDirectories(root, args, urlSuffix)
	end

	if type(platform) ~= 'string' then
		print("ERROR: missing platform parameter to 'plugins download' subcommand.")
		return 1
	end
	platform = platform:lower()
	if platform == 'iphone' then
		platform = 'ios'
		print("NOTICE: please, use modern 'ios' platform instead legacy 'iphone'.")
	end


	local buildSettingsFile = args[3]
	-- parse build settings and form "pluginsToDownload" containing { ['plugin.name']='com.coronalabs', }
	if not buildSettingsFile then
		print("ERROR: no build settings file specified.")
		return 1;
	end

	local settings
	if buildSettingsFile:sub(-#"build.properties") == "build.properties" then
		local props, err = io.open( buildSettingsFile, "r" )
		if not props then
			print("ERROR: unable to open build.properties file, error: " .. tostring(err))
			return 1
		end
		settings = json.decode(props:read("*a") or '{"buildSettings":{}}').buildSettings or {}

		props:close()
	else
		local oldSettings = _G['settings']
		_G['settings'] = nil
		pcall( function(  )
			dofile(buildSettingsFile)
		end  )
		settings = _G['settings']
		_G['settings'] = oldSettings
	end

	if type(settings) ~= 'table' then
		print("ERROR: Couldn't read 'build.settings' file at path: '" .. buildSettingsFile .. "'")
		return 1
	end

	if type(settings.plugins) ~= 'table' then
		settings.plugins = {}
	end

	local pluginsToDownload = {}

	for pluginName, pluginTable in pairs(settings.plugins) do
		local publisherId = pluginTable['publisherId']
		table.insert( pluginsToDownload, {pluginName, publisherId, pluginTable.supportedPlatforms} )
	end

	local splashScreenEnabled = true
	local splashScreenImage = nil
	if buildData and buildData.splashScreen then
		local splash = buildData.splashScreen
		if splash ~= nil then
			if splash.enable ~= nil then
				splashScreenEnabled = splash.enable
			end
			if type(splash.image) == "string" then
				splashScreenImage = splash.image
			end
		end
		local splash = buildData.splashScreen[platform]
		if splash ~= nil then
			if splash.enable ~= nil then
				splashScreenEnabled = splash.enable
			end
			if type(splash.image) == "string" then
				splashScreenImage = splash.image
			end
		end
	end
	if splashScreenImage ~= nil or not splashScreenEnabled then
		print("SPLASH\t" .. tostring(splashScreenImage))
	end
	local addedPluginsToDownload = {}
	if #pluginsToDownload > 0 or alwaysQuery then
		local authorisedPlugins = {}

		if user then
			local authURL = serverBackend .. '/v1/plugins/show/' .. user

			local authorisedPluginsText, msg = builder.fetch(authURL)

			if not authorisedPluginsText then
				print("ERROR: Unable to retrieve authorised plugins list (" .. msg .. ").")
				return 1
			end

			local authPluginsJson = json.decode( authorisedPluginsText )
			if not authPluginsJson then
				print("ERROR: Unable to parse authorised plugins list.")
				return 1
			end

			if authPluginsJson.status ~= 'success' then
				print("ERROR: Retrieving authorised plugins was unsuccessful. Info: " .. authorisedPluginsText)
				return 1
			end

			if not authPluginsJson.data then
				print("ERROR: received empty data for authorised plugins.")
				return 1
			end

			for _, ap in pairs(authPluginsJson.data) do -- ap : authorisedPlugin
				authorisedPlugins[ tostring(ap['plugin_name']) .. ' ' .. tostring(ap['plugin_developer'])] = ap['status']
			end
		end

		local authErrors = false
		for _, pd in pairs(pluginsToDownload) do
			local plugin, developer, supportedPlatforms = unpack( pd )
			addedPluginsToDownload[plugin .. " " .. developer] = true
			local supportedPlatform = true
			if supportedPlatforms then
				supportedPlatform = supportedPlatforms[platform]
				if platform == 'iphone' then
					supportedPlatform = supportedPlatform or supportedPlatforms['ios']
				end
			end
			if supportedPlatform then
				if type(supportedPlatform) == 'table' and type(supportedPlatform.url) == 'string' then
					if supportedPlatform.url == "" then
						print("ERROR: empty custom URL for: " .. plugin .. " (" .. developer .. ")")
						authErrors = true
					end
				else
					local status = authorisedPlugins[plugin .. ' ' .. developer] or 0
					if status == 0 then
						print("ERROR: plugin could not be validated: " .. plugin .. " (" .. developer .. ")")
						print("ERROR: Activate plugin at: https://marketplace.coronalabs.com/plugin/" .. developer .. "/" .. plugin)
						authErrors = true
					end
				end
			end
		end
		if authErrors then
			print("ERROR: exiting due to plugins above not being activated.")
			return 1
		end
	else
		print("No build.settings plugins to download")
	end


	for pluginName, pluginTable in pairs(buildDataPluginEntry) do
		local publisherId = pluginTable['publisherId']
		if not addedPluginsToDownload[pluginName .. " " .. publisherId] then
			table.insert( pluginsToDownload, {pluginName, publisherId, pluginTable.supportedPlatforms} )
		end
	end	

	local build = buildYear .. '.' .. buildRevision
	if platform == 'ios' then

		-- config for native plugins

		local simConfig =  iOSDownloadPlugins('iphoneos', 'iphone', build, pluginsToDownload, forceLoad )
		if not simConfig then
			return 1
		end
		local devConfig = iOSDownloadPlugins('iphonesimulator', 'iphone-sim', build, pluginsToDownload, forceLoad )
		if not devConfig then
			return 1
		end


		local xcconfig = args[4]
		if not xcconfig then
			print("ERROR: no output config file specified.")
			return 1
		end

		local config, err = io.open( xcconfig, "w" )
		if not config then
			print("ERROR: unable to write config file " .. configToGenerate .. " error: " .. tostring(err))
			return 1
		end
		config:write([[// This file is generated by Corona.
// All changes is overwritten when Download Plugins target is built

// import Corona native location and basic settings
#include "CoronaNative.xcconfig"

]])

		if forceLoad then
			config:write('OTHER_LDFLAGS = $(inherited) $(CORONA_CUSTOM_LDFLAGS) -force_load "$(CORONA_ROOT)/Corona/ios/lib/libplayer.a" \n')
		else
			config:write('OTHER_LDFLAGS = $(inherited) $(CORONA_CUSTOM_LDFLAGS) -all_load -lplayer\n')
		end

		if #devConfig > 0 then
			config:write('// device entries\n')
			config:write(devConfig)
		end
		if #simConfig > 0 then
			config:write('\n// simulator entries\n')
			config:write(simConfig)
		end
		config:close()


	elseif platform == 'android' or platform == 'android-kindle' or platform == 'android-nongoogle' then
		local androidConfSuccess = androidDownloadPlugins(platform, build, pluginsToDownload)
		if not androidConfSuccess then
			return 1
		end
	else
		print("ERROR: unsupported platform '".. platform .."'.")
		return 1
	end

	if verbosity > 0 then
		print("Done downloading plugins!")
	end

	if androidBuild then
		if user then
			local downloadInfoText, msg = builder.fetch(serverBackend.. "/v1/buildid/native/" .. user)
			if not downloadInfoText then
				print("ERROR: unable to fetch build ID: ", msg )
				return 1
			end
			print("BUILD\t" .. downloadInfoText)
		else
			print("BUILD\tOffline")
		end
	end

	return 0
end


function DownloadAndroidOfflinePlugins(args, user, buildYear, buildRevision)
	local buildData
	table.remove(args, 1)
	local inputFile = string.match(args[1] or "", "^builderInput=(.+)")
	if inputFile
	then
	    local f = assert(io.open(inputFile, "rb"))
		table.remove(args, 1)
		buildData = json.decode(f:read("*all"))
		f:close()
  	else
		buildData = json.decode(io.read('*all'))
	end
	assert(buildData)
	buildData.build = buildData.build or buildRevision
	buildData.user = buildData.user or user
	for i=1, #args do
		local k,v = args[i]:match('(.+)=(.+)')
		if k and v then
			buildData[k] = v
		end
	end

	local pluginCollector = require "CoronaBuilderPluginCollector"
	local result = pluginCollector.collect(buildData)
	if type(result) == 'string' then
		print("ERROR: occured while collecting plugins for Android. ", result)
        return 1
	end
	return 0
end
