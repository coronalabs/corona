local M = {}

-------------------------------------------------------------------------------

local lfs = require "lfs"

local BuilderUtils = require "BuilderUtils"
local checkError = BuilderUtils.checkError
local execute = BuilderUtils.execute
local printVerbose = BuilderUtils.printVerbose
local fileExists = BuilderUtils.fileExists

-------------------------------------------------------------------------------

-- Double quote a string escaping backslashes and any double quotes
local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

-------------------------------------------------------------------------------

function M:initialize()
	self.defaults = require "Defaults"
	local defaults = self.defaults
	defaults:initialize()
end

-- Generate plugin dir names
local function getPluginDirNames( pluginsDir )
	local result = {}
	for file in lfs.dir(pluginsDir) do
		-- Ignore hidden files
		if "libtemplate" ~= file and ( string.sub( file, 1, 1 ) ~= '.' ) then
			local filePath = pluginsDir .. '/' .. file
			local attributes = lfs.attributes( filePath )
			if "directory" == attributes.mode then
				table.insert( result, file )
			end
		end
	end

	--[[
	for i=1,#result do
		local path = result[i]
		print( path )
	end
	--]]

	return result
end

function M:copyBundles( options, pluginManifest )
	for i=1, #pluginManifest do
		local baseBundleDir = pluginManifest[i].path .. '/' .. "bundles/"
		local hasBundleDir = lfs.attributes(baseBundleDir, "mode") == "directory"
		if (hasBundleDir) then
			local src = quoteString( baseBundleDir )
			local dst = quoteString( options.dstDir )
			execute( "ditto " .. src .. " " .. dst , options.verbose )
		end
	end
end

function M:checkSwift( options, pluginManifest )
	for i=1, #pluginManifest do
		if pluginManifest[i].usesSwift then
			options.usesSwift = true
			if options.verbose then
				print("Found Swift usage in a plugin: ", pluginManifest[i].path)
			end
		end
	end
end

function M:analyzePlugins( options )
	local pluginManifest = self:findPlugins( options )
	self:updateOptionsWithPluginManifest( options, pluginManifest )
	self:copyBundles( options, pluginManifest )
	self:checkSwift( options, pluginManifest )
end

-- Returns a manifest (array of plugin metadata)
function M:findPlugins( options )
	local result = {}

	local pluginsDir = options.pluginsDir

	local pluginDirNames = getPluginDirNames( pluginsDir )
	for i=1,#pluginDirNames do
		local pluginName = pluginDirNames[i]

		-- Each plugin is expected to have a metadata.lua file on iOS
		-- that contains all the metadata
		local metadataPath = pluginsDir .. '/' .. pluginName .. '/metadata.lua'
		local metadataChunk = loadfile( metadataPath )

		if metadataChunk then
			local metadata = metadataChunk()
			local plugin = metadata.plugin

			-- Store path to plugin folder
			plugin.path = pluginsDir .. '/' .. pluginName

			-- Add plugin metadata to manifest
			print("Found native plugin: "..tostring(plugin.path))
			table.insert( result, plugin )
		end
	end

	return result
end

function M:analyzeLuaPlugins( options )
	local luaPluginDirs = self:findLuaPlugins( options )
	self:copyLuaAssets( options, luaPluginDirs )
end

function M:findLuaPlugins( options )
	local result = {}

	local pluginsDir = options.pluginsDir

	local pluginDirNames = getPluginDirNames( pluginsDir )
	for i=1,#pluginDirNames do
		local pluginName = pluginDirNames[i]

		-- This path signifies a Lua plugin
		local assetPath = pluginsDir .. '/' .. pluginName .. '/lua/lua_51/'
		local assetPath2 = pluginsDir .. '/' .. pluginName .. '/lua_51/'

		if options.verbose then
			print("Examining plugin: "..tostring(pluginName))
		end

		local isLuaPlugin = lfs.attributes( assetPath, "mode" ) == "directory"
		if isLuaPlugin then
			print("Found Lua plugin: "..tostring(pluginName))
			table.insert( result, assetPath )
		end

		local isLuaPlugin = lfs.attributes( assetPath2, "mode" ) == "directory"
		if isLuaPlugin then
			print("Found Lua plugin: "..tostring(pluginName))
			table.insert( result, assetPath2 )
		end
	end

	return result
end

-- copy assets from Lua plugins to app
function M:copyLuaAssets( options, pluginDirs )
	if #pluginDirs > 0 then
		local pluginsDstDir = options.dstDir .. "/corona-plugins"
		checkError( execute( 'mkdir -p ' ..  quoteString(pluginsDstDir), options.verbose ) )

		for i=1, #pluginDirs do
			local pluginDir = pluginDirs[i]

			-- copy all of the plugin files to the destination's .plugin directory
			checkError( execute( 'cp -R ' ..  quoteString(pluginDir) .."/* ".. quoteString(pluginsDstDir), options.verbose ) )
			-- delete all .lua and .lu files from the copy
			checkError( execute( 'find ' .. quoteString(pluginsDstDir) .. " \\( -name '*.lu' -o  -name '*.lua' \\) -delete", options.verbose ) )
			-- optimize pngs for iOS
			checkError( execute( 'COPYPNG=$(xcrun -f copypng); find ' .. quoteString(pluginsDstDir) .. " -name '*.png' -exec \"$COPYPNG\" -compress {} {}.copypng \\; -exec mv {}.copypng {} \\;", options.verbose ) )

		end
	end
end

-- Adding function to 'table' library
table.exists = function( t, item )
	local result

	for i=1,#t do
		if item == t[i] then
			result = true
			break
		end
	end

	return result
end

function M:updateOptionsWithPluginManifest( options, pluginManifest )
	local defaultOptions = self.defaults.options

	local librarySearchPaths = options.librarySearchPaths or defaultOptions.librarySearchPaths
	local staticLibs = options.staticLibs or defaultOptions.staticLibs
	local frameworks = options.frameworks or defaultOptions.frameworks
	local frameworksOptional = options.frameworksOptional or defaultOptions.frameworksOptional

	for i=1,#pluginManifest do
		local plugin = pluginManifest[i]

		-- Determine library searchpaths
		if not table.exists( librarySearchPaths, plugin.path ) then
			table.insert( librarySearchPaths, plugin.path )
		end

		-- Add plugin's static lib
		local libs = plugin.staticLibs or {}
		for j = 1,#libs do
			local item = libs[j]
			if not table.exists( staticLibs, item ) then
				table.insert( staticLibs, item )
			end
		end

		-- Resolve list of required/optional frameworks 
		local required = plugin.frameworks or {}
		local optional = plugin.frameworksOptional or {}

		-- Add any framework in 'required' to 'frameworks' that is not already in
		-- frameworks or frameworksOptional (which are the Corona frameworks)
		for j=1,#required do
			local item = required[j] -- plugin framework

			local exists = table.exists( frameworks, item )
			if not exists then
				exists = table.exists( frameworksOptional, item )
			end

			if not exists then
				table.insert( frameworks, item )
			end
		end

		-- Add any framework in 'optional' to 'frameworksOptional' that is not already in
		-- frameworks or frameworksOptional (which are the Corona frameworks)
		for j=1,#optional do
			local item = optional[j] -- plugin framework

			local exists = table.exists( frameworks, item )
			if not exists then
				exists = table.exists( frameworksOptional, item )
			end

			if not exists then
				table.insert( frameworksOptional, item )
			end
		end
	end
end

function M:getSliceTmpDir( options )
	local defaultOptions = self.defaults.options

	local tmpDir = options.tmpDir
	assert( tmpDir, "ERROR: Temporary directory not specified" )

	local architecture = options.architecture or defaultOptions.architecture

	-- tmpDir should be architecture-dependent
	tmpDir = tmpDir .. '/' .. options.architecture

	if not fileExists( tmpDir ) then
		checkError( execute( 'mkdir "' .. tmpDir .. '"' ) )
	end

	return tmpDir
end

function M:getSliceOutputPath( options )
	local tmpDir = self:getSliceTmpDir( options )
	local tmpDstPath = tmpDir .. '/' .. options.dstName
	return tmpDstPath
end

function M:compileSlices( options )
	local status = 0

	local defaultOptions = self.defaults.options

	local sdkType = options.sdkType or defaultOptions.sdkType

	local archSlices = defaultOptions.archSlices[sdkType]
	if archSlices then 
		printVerbose( "Compiling Slices..." )

		local slicePathArray = {}

		for i=1,#archSlices do
			assert( nil == options.architecture )
			assert( nil == options.flagsArchitecture )

			local architecture = archSlices[i]
			options.architecture = architecture
			options.flagsArchitecture = defaultOptions.flagsSlices[architecture]

			printVerbose( "\tCompiling architecture:", options.architecture )
			local sliceDir = self:getSliceTmpDir( options )
			local slicePath = self:getSliceOutputPath( options )
			checkError( self:compile( options, sliceDir, slicePath ) )
			table.insert( slicePathArray, slicePath )
			printVerbose( "\t\tDone.\n")

			if ( 0 ~= status ) then
				return status
			end

			options.architecture = nil
			options.flagsArchitecture = nil
		end
		printVerbose( "\tDone.\n")

		printVerbose( "Creating Universal Binary..." )
		checkError( self:lipo( options, slicePathArray ) )
		printVerbose( "\tDone.\n")
	end

	return status
end

function M:compile( options, sliceDir, slicePath )
	local defaultOptions = self.defaults.options

	local flags = options.flags or defaultOptions.flags
	local librarySearchPaths = options.librarySearchPaths or defaultOptions.librarySearchPaths
	local frameworks = options.frameworks or defaultOptions.frameworks
	local frameworksOptional = options.frameworksOptional or defaultOptions.frameworksOptional
	local staticLibs = options.staticLibs or defaultOptions.staticLibs

	local sep = ' '
	local flagsArchitecture = options.flagsArchitecture or defaultOptions.flagsArchitecture
	local flagArgs = sep .. table.concat( flagsArchitecture, sep )

	flagArgs = flagArgs .. sep .. table.concat( flags, sep )

	local frameworkSearchPathsArgs = ""
	if lfs.attributes( options.dstDir .. "/Frameworks", "mode" ) == "directory" then
		frameworkSearchPathsArgs = " -F" .. quoteString(options.dstDir .. "/Frameworks")
	end


	-- Put quotes around library search paths
	local librarySearchPathsQuoted = {}
	for i=1,#librarySearchPaths do
		local item = librarySearchPaths[i]
		table.insert( librarySearchPathsQuoted, quoteString( item ) )
	end

	sep = ' -L'
	local librarySearchPathsArgs = sep .. table.concat( librarySearchPathsQuoted, sep )

	sep = ' -F'
	frameworkSearchPathsArgs = frameworkSearchPathsArgs .. sep .. table.concat( librarySearchPathsQuoted, sep )

	sep = ' -framework '
	local frameworksArgs = ''
	if ( #frameworks > 0 ) then
		frameworksArgs = sep .. table.concat( frameworks, sep )
	end

	sep = ' -weak_framework '
	local frameworksOptionalArgs = ''
	if ( #frameworksOptional > 0 ) then
		frameworksOptionalArgs = sep .. table.concat( frameworksOptional, sep )
	end

	sep = ' -l'
	local staticLibsArgs = ''
	if ( #staticLibs > 0 ) then
		staticLibsArgs = sep .. table.concat( staticLibs, sep )
	end

	local args = flagArgs .. librarySearchPathsArgs .. frameworkSearchPathsArgs .. frameworksArgs .. frameworksOptionalArgs .. staticLibsArgs

	local tools = self.defaults.tools
	local chdir = 'cd ' .. quoteString( sliceDir ) .. ';'
	local env = tools.linkerEnv
	local cmd = chdir .. env .. quoteString( tools.compiler ) .. args .. ' -o ' .. quoteString( slicePath )
	return execute( cmd, options.verbose )
end

function M:lipo( options, slicePathArray )
	local defaultOptions = self.defaults.options
	local dstPath = options.dstPath or defaultOptions.dstPath

	local tools = self.defaults.tools
	local chdir = 'cd ' .. quoteString( options.dstDir ) .. ';'
	local env = 'export PATH=' .. quoteString( tools.PATH ) .. ';' -- note: trailing semicolon

	-- Flatten slices into a single string, quoting each path separately
	local slices = ''
	local sep = ' '
	for i=1,#slicePathArray do
		slices = slices .. sep .. quoteString( slicePathArray[i] )
	end

	local cmd = chdir .. env .. quoteString( tools.lipo ) .. ' -create ' .. slices .. ' -output ' .. quoteString( dstPath )
	return execute( cmd, options.verbose )
end

function M:emitDebug( options )
	local defaultOptions = self.defaults.options

	local sep = ' '
	local dstPath = options.dstPath or defaultOptions.dstPath
	local args = sep .. quoteString( dstPath )

	local dstDir = options.dstDir

	local tools = self.defaults.tools
	local chdir = 'cd ' .. quoteString( options.dstDir ) .. ';'
	local env = 'export PATH=' .. quoteString( tools.PATH ) .. ';' -- note: trailing semicolon
	local cmd = chdir .. env .. quoteString( tools.dsymutil ) .. args .. ' -o ' .. quoteString( dstDir .. '.dSYM' )
	return execute( cmd, options.verbose )
end

function M:strip( options )
	local defaultOptions = self.defaults.options

	local dstPath = options.dstPath or defaultOptions.dstPath

	local tools = self.defaults.tools
	local chdir = 'cd ' .. quoteString( options.dstDir ) .. ';'
	local env = 'export PATH=' .. quoteString( tools.PATH ) .. ';' -- note: trailing semicolon
	local cmd = chdir .. env .. quoteString( tools.strip ) .. " -x " .. quoteString( dstPath )
	return execute( cmd, options.verbose )
end

local function getMinVersion( settings )
	local result -- default is nil

	if settings then
		local iphone = settings.iphone
		if iphone then
			local buildSettingsPlist = iphone.plist
			if buildSettingsPlist then
				if "string" == type( buildSettingsPlist.MinimumOSVersion ) then
					result = buildSettingsPlist.MinimumOSVersion
				end
			end
		end
	end

	return result
end

function M:build( options )
	local verbose = options.verbose
	BuilderUtils.setVerbose( verbose )

	local defaultOptions = self.defaults.options
	local sdkType = options.sdkType or defaultOptions.sdkType
	local minVersion = getMinVersion( options.settings )
	self.defaults:setSdkType( sdkType, minVersion )

	if options.pluginsDir then
		printVerbose( "Analyzing plugins..." )
		self:analyzePlugins( options )
		printVerbose( "\tDone.\n")

		printVerbose( "Analyzing Lua plugins..." )
		self:analyzeLuaPlugins( options )
		printVerbose( "\tDone.\n")
	end

	if options.usesSwift then
		local librarySearchPaths = options.librarySearchPaths or defaultOptions.librarySearchPaths
		local usrLibSwift = "/usr/lib/swift"
		if not table.exists( librarySearchPaths, usrLibSwift ) then
			table.insert( librarySearchPaths, usrLibSwift )
		end
		local swiftLibsDst = self.defaults.tools.base  .. "/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift/" .. sdkType
		if not table.exists( librarySearchPaths, swiftLibsDst ) then
			table.insert( librarySearchPaths, swiftLibsDst )
		end
	end

	local status = checkError( self:compileSlices( options ) )

	printVerbose( "Emit debug symbols..." )
	status = checkError( self:emitDebug( options ) )
	printVerbose( "\tDone.\n")

	printVerbose( "Stripping..." )
	status = checkError( self:strip( options ) )
	printVerbose( "\tDone.\n")

	if 0 == status then
		printVerbose( "Build Succeeded." )
	end

	return status
end

-- TODO: Remove this if we make this derive from CoronaPrototype
M:initialize()

return M

