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

local function getPluginDirectories(platform, build, pluginsToDownload, buildSettingsPlugins)

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

	local pluginCollector = require "CoronaBuilderPluginCollector"
	local collectorParams = {
	  pluginPlatform = platform,
	  plugins = buildSettingsPlugins or {},
	  destinationDirectory = pluginsDest,
	  build = "9999",
		download = builder.download,
		fetch = builder.fetch
	}
	local res = pluginCollector.collect(collectorParams)
	if(res)then
		print(res)
		return 1
	end

	for _, pd in pairs(pluginsToDownload) do
		local plugin, developer = unpack( pd )
		local pluginArchivePath = pluginsDest .. plugin
		local unpackLocation = pluginsDest .. developer .. '_' .. plugin
		lfs.mkdir(unpackLocation)
		local ret = unpackPlugin(pluginArchivePath..'/data.tgz', unpackLocation)
		if ret ~= 0 then
			print("WARNING: unable to unpack plugin " .. plugin .. ' (' .. developer .. ').')
		else
			table.insert(pluginDirectories, unpackLocation)
		end
		--clean up archives and older folder
		os.remove(pluginArchivePath..'/data.tgz')
		lfs.rmdir(pluginArchivePath)
	end

	return pluginDirectories

end




local function AppleDownloadPlugins( sdk, platform, build, pluginsToDownload, forceLoad, buildSettingsPlugins )
	-- download plugins and unpack them
	local pluginDirectories = getPluginDirectories(platform, build, pluginsToDownload, buildSettingsPlugins )
	if not pluginDirectories then
		return
	end

	local nativePlugins = {}
	-- local luaPlugins = {}
	local additionalResources = {}

	-- local detect Lua plugins and Native plugins

	for _, pluginDir in pairs(pluginDirectories) do

		-- we need to copy .framework files to the resources directory for tvOS
		if platform == 'appletvos' or platform == 'appletvsimulator' then
			local resourcesDir = pluginDir .. '/resources'
			local frameworkDir = resourcesDir .. '/Frameworks'
		
			-- Ensure directories exist before copying
			local function ensureDirectoryExists(path)
				if lfs.attributes(path, "mode") ~= "directory" then
					lfs.mkdir(path)
				end
			end
		
			-- Create the necessary directories
			ensureDirectoryExists(resourcesDir)
			ensureDirectoryExists(frameworkDir)
		
			-- Copy all .framework files
			if lfs.attributes(pluginDir, "mode") == "directory" then
				for file in lfs.dir(pluginDir) do
					if file:match("%.framework$") then
						local src = pluginDir .. '/' .. file
						local dst = frameworkDir .. '/' .. file
						os.execute('cp -r ' .. quoteString(src) .. ' ' .. quoteString(dst))
					end
				end
			end
		end
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
		if(plugin.staticLibs)then
			for _, lib in pairs(plugin.staticLibs) do
				if forceLoad then
					staticLibs[' -force_load ' .. quoteString(plugin.path .. '/lib' .. lib .. '.a')] = true
				else
					staticLibs[' -l' .. lib] = true
					searchPaths[plugin.path] = true
				end
			end
		end

		if(plugin.frameworks)then
			for _, lib in pairs(plugin.frameworks) do
				frameworks[lib] = true
				frameworkSearchPaths[plugin.path] = true
			end
		end

		if(plugin.frameworksOptional)then
			for _, lib in pairs(plugin.frameworksOptional) do
				frameworksWeak[lib] = true
				frameworkSearchPaths[plugin.path] = true
			end
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

-- in offline build `user` is nil
function DownloadPluginsMain(args, user, buildYear, buildRevision)
	if args[1] ~= 'download' then
		print("ERROR: unknows subcommand to 'plugins' command: '" .. tostring(args[1]) .. "'. Only 'download' is currently supported." )
		return 1
	end

	local buildDataPluginEntry = {}
	local buildData = {}
	local forceLoad = false
	for i=#args,1,-1 do
		if args[i] == '--force-load' then
			table.remove(args, i)
			forceLoad = true
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

	for pluginName, pluginTable in pairs(buildDataPluginEntry) do
		local publisherId = pluginTable['publisherId']
		if not addedPluginsToDownload[pluginName .. " " .. publisherId] then
			table.insert( pluginsToDownload, {pluginName, publisherId, pluginTable.supportedPlatforms, pluginTable.marketplaceId} )
		end
	end

	local build = buildYear .. '.' .. buildRevision
	if platform == 'ios' or platform == 'tvos' then

		local platformConfigs = {
			tvos = { dev = { 'appletvos', 'appletvos' }, sim = { 'appletvsimulator', 'appletvsimulator' } },
			default = { dev = { 'iphoneos', 'iphone' }, sim = { 'iphonesimulator', 'iphone-sim' } }
		}
		
		local config = platformConfigs[platform] or platformConfigs.default
		
		local simConfig = AppleDownloadPlugins(config.sim[1], config.sim[2], build, pluginsToDownload, forceLoad, settings.plugins)
		if not simConfig then return 1 end
		
		local devConfig = AppleDownloadPlugins(config.dev[1], config.dev[2], build, pluginsToDownload, forceLoad, settings.plugins)
		if not devConfig then return 1 end
		

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

		if( platform == 'tvos') then
			if forceLoad then			
				config:write('OTHER_LDFLAGS = $(inherited) $(CORONA_CUSTOM_LDFLAGS) -force_load \n')
			else
				config:write('OTHER_LDFLAGS = $(inherited) $(CORONA_CUSTOM_LDFLAGS) -all_load \n')
			end
		else
			if forceLoad then			
				config:write('OTHER_LDFLAGS = $(inherited) $(CORONA_CUSTOM_LDFLAGS) -force_load "$(CORONA_ROOT)/Corona/ios/lib/libplayer.a" \n')
			else
				config:write('OTHER_LDFLAGS = $(inherited) $(CORONA_CUSTOM_LDFLAGS) -all_load -lplayer\n')
			end
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
	else
		print("ERROR: unsupported platform '".. platform .."'.")
		return 1
	end

	if verbosity > 0 then
		print("Done downloading plugins!")
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
