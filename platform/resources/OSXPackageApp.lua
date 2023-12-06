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
local simAvail, simulator = pcall(require, "simulator")

-- defines modifyPlist()
local CoronaPListSupport = require("CoronaPListSupport")
local captureCommandOutput = CoronaPListSupport.captureCommandOutput

if not simAvail then
	simulator = nil
end

-- get the numeric value of the "debugBuildProcess" preference or 0 if it's not set (note due to a Lua bug
-- the value is actually the exit code multiplied by 256)
local debugBuildProcess = os.execute("exit $(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null || echo 0)") / 256


-- Read a value from the user's preferences
local function getPreference( prefName )
	return captureCommandOutput("defaults read com.coronalabs.Corona_Simulator "..tostring(prefName))
end

-- Double quote a string escaping backslashes and any double quotes
local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

local function makepath( inPath, ... )
	local path = inPath or ""

	for p=1, select('#', ...) do
		local arg = select(p, ...)

		path = path .. "/"
		path = path .. arg
	end

	path = path:gsub('/+', '/')
	return path
end

local function setStatus( msg )
	print( "Building: " .. msg )
	if simulator then
		simulator.show('buildstatus', msg )
	end
end

--------------------------------------------------------------------------------
-- isAlpha
--
-- val = ASCII character
--
-- Returns true if alpha character, otherwise false
--------------------------------------------------------------------------------

local function isAlpha( val )

	if val >= "A" and val <= "Z" then
		return true
	end
	
	if val >= "a" and val <= "z" then
		return true
	end
	
	return false	-- not Alpha
	
end

--------------------------------------------------------------------------------
-- isNumber
--
-- val = ASCII character
--
-- Returns true if number, otherwise false
--------------------------------------------------------------------------------

local function isNumber( val )

	if val >= "0" and val <= "9" then
		return true
	end
	
	return false	-- not number
	
end

--------------------------------------------------------------------------------
-- isNumber
--
-- val = ASCII character
-- allowDots = true if dots are allowed
--
-- Returns true if number, otherwise false
--------------------------------------------------------------------------------

local function isDotDash( val, allowDots )

	local char2 = "-"		-- default is to not test for dot
	
	if allowDots then
		char2 = "."		-- test for dot
	end
	
	if val == "-" or val == char2 then
		return true
	end
	
	return false	-- not dot or dash
	
end

--------------------------------------------------------------------------------
-- sanitize Bundle ID String
--
-- bundleStr = Bundle string
-- allowDots = true to allow dots in name, otherwise dots converted to dash
--
-- Returns validated Bundle ID string
--
-- This code is used to convert non-alphanumeric (except dash and maybe dot)
-- into dashes for a valid bundle ID.
-- "com.example.Tom's App_Name" returns as "com.example.Tom-s-App-Name"
-- Refer RFC1034 for string specification which is needed to match Xcode (and Apple)
--------------------------------------------------------------------------------

local function sanitizeBundleString( bundleStr, allowDots )

	local newBundleStr	= ""	-- new string
	
	local val
	
	for i = 1, string.len( bundleStr ) do
		val = string.sub( bundleStr, i, i )
		
		if isAlpha( val ) then
			newBundleStr = newBundleStr .. val
		elseif isNumber( val ) then
			newBundleStr = newBundleStr .. val
		elseif isDotDash( val, allowDots ) then
			newBundleStr = newBundleStr .. val
		else
			newBundleStr = newBundleStr .. "-"
		end
		
	end -- do
	
	return newBundleStr

end


--------------------------------------------------------------------------------
local function getCopyResourcesScript( src, dst, options )

	-- Generate a script to copy the project's assets to the built app
	-- rsync is used to speed up building apps with very large numbers of assets
	--
	-- rsync flags:
	--             --delete-excluded       also delete excluded files from dest dirs
	--             --exclude=PATTERN       exclude files matching PATTERN
	--             --delete                delete extraneous files from dest dirs
	--             --force                 force deletion of dirs even if not empty
	--             --prune-empty-dirs      prune empty directory chains from file-list
	--             --human-readable        output numbers in a human-readable format
	--             --itemize-changes       output a change-summary for all updates
	--             --recursive             recurse into directories
	--             --links                 copy symlinks as symlinks
	--             --copy-unsafe-links	   copy external symlinked files to destination
	--             --hard-links            preserve hard links
	--             --times                 preserve times
	--             --perms                 preserve permissions (needed by next option)
	--             --chmod                 specify base level permissions (prevents us generating bad apps)
	--
	local script = [==[
	# rsync goes haywire if either the source or destination are empty
	if [ -z "$SRC_DIR" -o -z "$DST_DIR" ]
	then
		echo "OSXPackageApp: invalid SRC_DIR or DST_DIR for rsync"
		exit 1
	fi

	rsync -rv --times --delete-excluded {{EXCLUDED_FILES}} --exclude="**/*.lua" --exclude="build.settings" --exclude="**/.*"  --delete --prune-empty-dirs --links --copy-unsafe-links --hard-links --perms --chmod="Da+rx,Fa+r" --human-readable --itemize-changes "$SRC_DIR/" "$DST_DIR"

]==]

	local excludedFilesSh = ""

    if options and options.settings then
		-- Generate shell code from the excludeFiles entry in build.settings
		if options.settings.excludeFiles then
			-- we have actual files to exclude
			print("Excluding specified files from build: ")
			for platform,excludes in pairs(options.settings.excludeFiles) do
				if platform == "all" or platform == "osx" or platform == "macos" then
					for index,pattern in ipairs(excludes) do
						print("   excluding: "..pattern)
						pattern = pattern:gsub("'", "'\\''") -- quote single quotes
						excludedFilesSh = excludedFilesSh .. " --exclude='" .. pattern .."'"
					end
				end
			end
        end
	end

	local args = "SRC_DIR="..src.."\nDST_DIR="..dst.."/Contents/Resources/Corona\n"

	-- Replace the placeholders in the script with the generated code (or an empty string if there was none)
	script = args .. script:gsub("{{EXCLUDED_FILES}}", excludedFilesSh)

	if debugBuildProcess > 0 then
		print("script: ".. script)
	end

	return script
end

--------------------------------------------------------------------------------

local function getCodesignScript( entitlements, path, appIdentity, codesign )

	local quotedpath = quoteString( path )

	-- Remove any extended macOS attributes from files in the bundle because
	-- codesign doesn't like them
	local removeXattrs = "/usr/bin/xattr -cr "..quoteString(path) .." && "

	local entitlementsParam
	if entitlements ~= nil and entitlements ~= "" then
		entitlementsParam = " --entitlements ".. entitlements .. " "
	else
		entitlementsParam = " --deep "
	end
	local verboseParam = ""
	if debugBuildProcess and debugBuildProcess ~= 0 then
		verboseParam = "-".. string.rep("v", debugBuildProcess) .." "
	end

	local cmd = removeXattrs .. codesign.." --options runtime -f -s "..quoteString(appIdentity).." "..entitlementsParam..verboseParam..quotedpath

	return cmd
end

local function getProductBuildScript( path, installerIdentity, productbuild )

	local infoPlist = makepath(path, "Contents/Info.plist")
	local packagePath = path:gsub('.app$', '') .. ".pkg"

	-- productbuild --component "$APP" /Applications --sign "3rd Party Mac Developer Installer: Corona Labs Inc" --product "$APP/Contents/Info.plist" "$APP_BASENAME".pkg
	local cmd = productbuild.." --component ".. quoteString(path) .." /Applications --sign "..quoteString(installerIdentity).." --product "..quoteString(infoPlist) .." "..quoteString(packagePath)

	return cmd
end

local function getProductValidateScript( path, itunesConnectUsername, itunesConnectPassword, applicationLoader )

	local packagePath = path:gsub('.app$', '') .. ".pkg"
	-- Apple tells us to use this buried utility to automate Application Loader tasks in
	-- https://itunesconnect.apple.com/docs/UsingApplicationLoader.pdf
	local altool = makepath(applicationLoader, "Contents/Frameworks/ITunesSoftwareService.framework/Support/altool")

	-- If the "cmd" generated below fails because it's the wrong path that's very hard to detect amongst all the XML parsing so we do it here
	if lfs.attributes( altool ) == nil then
		print("ERROR: cannot find 'altool' utility in "..altool)
		return "echo Failed to find build utilities in expected places (cannot find 'altool' utility in '"..altool.."')"
	end

	-- Removing the "!DOCTYPE" line from the XML stops xpath trying to access the DTD
	-- The xpath command parses the XML output looking for an error message
	-- The final sed command adds a newline to the last line of output which xpath omits
	local cmd = quoteString(altool) .. " --validate-app -f ".. quoteString(packagePath) .." -u '".. itunesConnectUsername .."' -p '".. itunesConnectPassword .."' --output-format xml | grep -v '^<!DOCTYPE' | /usr/bin/xpath '/plist/dict/key[.=\"product-errors\"]/following-sibling::*[1]//key[.=\"message\"]/following-sibling::*[1]/node()' 2>/dev/null  | sed -ne 'p'"

	return cmd
end

local function getProductUploadScript( path, itunesConnectUsername, itunesConnectPassword, applicationLoader )

	local packagePath = path:gsub('.app$', '') .. ".pkg"
	-- Apple tells us to use this buried utility to automate Application Loader tasks in
	-- https://itunesconnect.apple.com/docs/UsingApplicationLoader.pdf
	local altool = makepath(applicationLoader, "Contents/Frameworks/ITunesSoftwareService.framework/Support/altool")
	
	-- Removing the "!DOCTYPE" line from the XML stops xpath trying to access the DTD
	-- The xpath command parses the XML output looking for an error message
	-- The final sed command adds a newline to the last line of output which xpath omits
	local cmd = quoteString(altool) .. " --upload-app -f ".. quoteString(packagePath) .." -u '".. itunesConnectUsername .."' -p '".. itunesConnectPassword .."' --output-format xml | grep -v '^<!DOCTYPE' | /usr/bin/xpath '/plist/dict/key[.=\"product-errors\"]/following-sibling::*[1]//key[.=\"message\"]/following-sibling::*[1]/node()' 2>/dev/null | sed -ne 'p'"

	return cmd
end

local function getCreateDMGScript( path )

	local dmgPath = path:gsub('(.*)%.%w+$', '%1') .. ".dmg"
	local dmgName = path:gsub('.*/(.+)', '%1'):gsub('%.app$', '')

	local verboseParam = ""
	if debugBuildProcess and debugBuildProcess ~= 0 then
		verboseParam = "-verbose"
	end

	local cmd = "/usr/bin/hdiutil create -srcfolder '"..path.."' -volname '"..dmgName.."' -fs HFS+ -format UDZO -imagekey zlib-level=9 '"..dmgPath.."'"

	return cmd
end

local function getShowFileInFinderScript( path )

	local cmd = [[osascript -e 'set thePath to "__PATH__"
	tell application "Finder"
	    reveal POSIX file thePath as text
	    activate
	end tell']]

	cmd = cmd:gsub("__PATH__", path)

	return cmd
end

--------------------------------------------------------------------------------

function runScript( script, debugLevel )
	local debugLevel = debugLevel or 0
	local errMsg = nil

	if debugBuildProcess and debugBuildProcess > debugLevel then
		print("Running: ".. tostring(script))
	end

	-- Run the command capturing any output
	local tmpFile = os.tmpname()
	local exitCode = os.execute( '( ' .. script .. ' ) 2>' .. tmpFile )

	if exitCode ~= 0 then
		print("ERROR: build command failed: ".. script)
		-- divide exitcode by 256 to work around Lua bug
		print("ERROR: with exit code ".. (exitCode / 256) .." and output: ")
		errMsg = captureCommandOutput( "/bin/cat " .. tmpFile )
		print( errMsg )
	end

	os.remove( tmpFile )

	return exitCode, errMsg
end

--------------------------------------------------------------------------------

local templateXcent = [[
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.app-sandbox</key>
    <true/>
{{CUSTOM_ENTITLEMENTS}}
</dict>
</plist>
]]

local function generateOSXEntitlements(filename, settings, provisionFile)

	local outFile = io.open( filename, "wb" )

	if outFile == nil then
		return "failed to open xcent file for writing: "..tostring(filename)
	end

	local data = templateXcent
	local entitlementsStr = ""

	local generatedEntitlements, includeProvisioning = CoronaPListSupport.generateEntitlements(settings, 'osx', provisionFile)
	data, numMatches = string.gsub( data, "{{CUSTOM_ENTITLEMENTS}}", generatedEntitlements )
	assert( numMatches == 1 )

	outFile:write(data)
	outFile:close()

	if debugBuildProcess and debugBuildProcess ~= 0 then
		print( "Created entitlements file: " .. filename );
		runScript("/bin/cat "..filename)
	end

	return "", includeProvisioning
end

--
-- generateFiles
--
-- Create the Info.plist file
--
-- returns an error message or nil on success
--
local function generateFiles( options )
	local result = nil

	result = CoronaPListSupport.modifyPlist( options )
	if result then
		return result
	end

	return result
end

-- helper function to figure out if building for App Store distribution
-- returns true or false
local function isBuildForAppStoreDistribution( options )
	
	if not options.signingIdentity then
		return false
	end

	-- FIXME: Should handle App Store vs. Ad-hoc
	local retval = string.match( options.signingIdentityName, "3rd Party Mac Developer" ) or string.match( options.signingIdentityName, "Apple Distribution" )
	if retval then
		return true
	else
		return false
	end
end

--
-- packageApp
--
-- Copy the assets from the project directory to the app bundle
--
-- returns an error message or nil on success
--
local function packageApp( options )
	local baseName = options.dstFile;
	-- Introducing an unquoted string because we should be quoting as the final set after appending strings, not before.
	-- See casenum: 1008
	local appBundleFileUnquoted = options.dstDir .. "/" .. baseName .. ".app"
	-- Quoting should happen around the final string, otherwise you get quotes in multiple places which might not be what you expect.
	local appBundleFile = quoteString( appBundleFileUnquoted )

	local binDir = options.binDir

	setStatus("Copying changed app resources")
	local result, errMsg = runScript( getCopyResourcesScript( quoteString( options.srcAssets ), appBundleFile, options ), 2 )

	if result ~= 0 then
		return "ERROR: copying resources failed: "..tostring(errMsg)
	end

	runScript( "/bin/chmod 755 " .. appBundleFile )

	return nil
end

--------------------------------------------------------------------------------

local function getNames( name, result )
	local err

	result.bundledisplayname = name
	result.bundlename = name
	result.bundleexecutable = name
	result.dstFile = name

	return err
end

-- determines appid and bundleid from provision file
-- if wildcard is present, then requires bundlename to be supplied as input
local function getIDs( mobileProvision, result )
	local err

	if debugBuildProcess and debugBuildProcess ~= 0 then
		print("getIDs: mobileProvision '"..tostring(mobileProvision).."'")
	end

	if not mobileProvision then
		result.mobileProvision = nil
		result.appid = sanitizeBundleString( result.bundlename, false )
		result.bundleid = result.appid -- string.match( result.appid, "[^%.]+%.(.*)" )
		result.pushEnvironment = nil

		return nil
	end

	result.mobileProvision = mobileProvision

	-- Extract app id from provision file
	local data = nil
	local inFile, errorMsg = io.open( mobileProvision, "rb" )
	if inFile then
		data = inFile:read( "*all" )
		inFile:close()
	else
		return "Error: Cannot open '"..mobileProvision.."' for reading ("..tostring(errorMsg)..")"
	end

	if not data or string.len(data) == 0 then
		return "Error: '"..mobileProvision.."' is not a valid provision file"
	end

	local patternPrefix =
		"<key>com.apple.application%-identifier</key>%s*<string>([%w%.%-]*)"
	local patternWithWildcard = patternPrefix .. "%.%*</string>"
	local patternWithoutWildcard = patternPrefix .. "</string>"
	local appid = string.match( data, patternWithWildcard )
	local usesWildcard = true
	if ( not appid ) then
		usesWildcard = false
		appid = string.match( data, patternWithoutWildcard )
	end

	if ( not appid ) then
		local pattern = patternPrefix .. "</string>"
		local appid= string.match( data, pattern )
		if ( not appid ) then
			--assert( appid )
			err = "Error: Could not obtain application identifier from '"..mobileProvision.."'"
		end
	end

	if ( appid ) then
		local value = appid

		assert( result )

		-- only append if wildcard was used
		if usesWildcard then
			-- Convert the bundlename into alphanumeric and dash characters only
			value = value .. "." .. sanitizeBundleString( result.bundlename, false )
		end

		result.appid = value
		result.bundleid = string.match( value, "[^%.]+%.(.*)" )
	end

	-- Determine push notification aps-environment (default to nil)
	local pattern =
		"<key>aps%-environment</key>%s*<string>([%w]*)</string>"
	local pushEnvironment = string.match( data, pattern )
	result.pushEnvironment = pushEnvironment

	return err
end

local function fileExists( filename )
	local f = io.open( filename, "r" )
	if ( f ) then
		io.close( f )
	end
	return ( nil ~= f )
end

function getBundleId( mobileProvision, bundledisplayname )
	local options = {}

	local err = getNames( bundledisplayname, options )
	if debugBuildProcess and debugBuildProcess ~= 0 then
		print("getBundleId: bundledisplayname '"..tostring(bundledisplayname).."'; options: "..json.prettify(options))
	end
	if ( not err ) then
		err = getIDs( mobileProvision, options )

		if ( not err ) then
			return options.bundleid
		end
	end
	
	return nil
end

-- Corona table extension that we want to use here
table.indexOf = function( t, object )
	local result

	if "table" == type( t ) then
		for i=1,#t do
			if object == t[i] then
				result = i
				break
			end
		end
	end

	return result
end

function signAllPlugins(pluginsDir, signingIdentity, codesign)
	if lfs.attributes( pluginsDir ) == nil then
		return "" -- no plugins dir
	end

	for file in lfs.dir(pluginsDir) do
		-- Only sign files in the Plugins directory (directories will contain assets that don't need to be signed)
		local pluginFile = makepath(pluginsDir, file)
		if lfs.attributes( pluginFile ).mode ~= "directory" then

			local result, errMsg = runScript( getCodesignScript( nil, pluginFile, signingIdentity, codesign ) )

			if result ~= 0 then
				errMsg = "ERROR: plugin code signing for '"..pluginFile.."' failed: "..tostring(errMsg)
				return errMsg
			end
		end
	end

	return ""
end

--
-- OSXPostPackage
--
-- Process the app bundle return by the build server (including code signing, Info.plist processing, etc)
--
-- returns an error message or nil on success
--
function OSXPostPackage( params )

	local srcAssets = params.srcAssets
	local compiledLuaArchive = params.compiledLuaArchive
	local tmpDir = params.tmpDir
	local dstDir = params.dstDir
	local bundledisplayname = params.bundledisplayname
	local bundleversion = params.bundleversion
	local provisionFile = params.provisionFile
	local signingIdentity = params.signingIdentity
	local targetDevice = params.targetDevice
	local targetPlatform = params.targetPlatform
	local verbose = ( debugBuildProcess and debugBuildProcess > 1 )
	local osPlatform = params.osPlatform
	local sdkRoot = params.xcodetoolhelper.sdkRoot
	local err = nil

	setStatus("Building for macOS")

	-- init options
	local options = {
		dstDir=dstDir,
		srcAssets=srcAssets,
		compiledLuaArchive=compiledLuaArchive,
		bundleversion=bundleversion,
		signingIdentity=signingIdentity,
		tmpDir=tmpDir,
		sdkRoot=sdkRoot,
		targetDevice=targetDevice,
		targetPlatform=targetPlatform,
		verbose=verbose,
		osPlatform=osPlatform,
		sdkType=params.sdkType,
		sdkRoot=sdkRoot,
		osxAppTemplate=params.osxAppTemplate,
		tmpPluginsDir=params.tmpPluginsDir,
		corona_build_id=params.corona_build_id,
		xcodetoolhelper = params.xcodetoolhelper,
	}

	local customSettingsFile = srcAssets .. "/build.settings"
	if ( fileExists( customSettingsFile ) ) then
		local customSettings, msg = loadfile( customSettingsFile )
		if ( customSettings ) then
			local status, msg = pcall( customSettings )
			if status then
				print( "Using additional build settings from: " .. customSettingsFile )
				-- forwards compatibility
				if settings.macos ~= nil then
					if settings.osx ~= nil then
						print( "WARNING: settings.macos overrides settings.osx in "..customSettingsFile )
					end
					settings.osx = settings.macos
				end
				options.settings = settings
			else
				err = "Warning: Error found in build.settings file:\n\t".. msg
				print(err)
			end
		else
			err = "Warning: Could not load build.settings file:\n\t".. msg
			print(err)
		end
	end

	if ( not err ) then
		local err = getNames( bundledisplayname, options )
		if ( not err ) then

			-- Special case for no provision profile (e.g. Xcode simulator)
			if not provisionFile then
				options.bundleid = "com.example." .. sanitizeBundleString(bundledisplayname)
			else
				err = getIDs( provisionFile, options )
			end
		end
	end


	if ( not err ) then

		-- Make sure the build directory exists
		runScript( "/bin/mkdir -p "..quoteString( dstDir ) )

		local appBundleFileUnquoted = options.dstDir .. "/" .. options.dstFile .. ".app"
		appBundleFileUnquoted = appBundleFileUnquoted:gsub('//', '/')
		options.appBundleFile = quoteString( appBundleFileUnquoted )

		-- If the output bundle directory exists but is not writeable, error
		-- (a common cause of this is testing a .pkg install which will replace the original app
		-- with an installed copy owned by root)
		if lfs.attributes( appBundleFileUnquoted ) ~= nil and not lfs.touch(  appBundleFileUnquoted ) then
			return "ERROR: app bundle '".. appBundleFileUnquoted .."' is not writeable (you may need to remove it before building)"
		end

		-- If an iOS app with the same name as this macOS app exists in the build directory, tell
		-- the user and refuse to overwrite it (this also works around an apparent bug in macOS which
		-- causes a macOS app that replaces an iOS app to not be runnable)
		if lfs.attributes( appBundleFileUnquoted.."/"..options.dstFile ) ~= nil then
			return "ERROR: an iOS application with the same name exists in the output folder and will not be overwritten"
		end

		setStatus("Unpacking template")
		-- extract template into dstDir
		local result, errMsg = runScript( "/usr/bin/ditto -x -k "..quoteString( options.osxAppTemplate ).." "..options.appBundleFile )
		runScript("find " .. options.appBundleFile .. " -name _CodeSignature -exec rm -vr {} +")

		if result ~= 0 then
			return "ERROR: unzipping template failed: "..tostring(errMsg)
		end

		-- cleanup signature from the template
		runScript( "cd ".. options.appBundleFile .. " ; /usr/bin/codesign --remove-signature --deep . " )

		-- If "bundleResourcesDirectory" is set, copy the contents of that directory to the
		-- application's Resource directory
		if options and options.settings and options.settings.osx and options.settings.osx.bundleResourcesDirectory then
			local customBundleResources = makepath(srcAssets, options.settings.osx.bundleResourcesDirectory)
			if ( fileExists( customBundleResources ) ) then
				local resourceDir = makepath(options.appBundleFile, "/Contents/Resources/")
				setStatus("Copying 'bundleResourcesDirectory' resources")
				runScript("/bin/cp -av "..quoteString(customBundleResources).."/ "..resourceDir)
			else
				print("WARNING: 'bundleResourcesDirectory' does not exist: "..customBundleResources)
			end
		end

		-- rename the internal binary
		runScript( "/bin/mv -v "..options.appBundleFile.."/Contents/MacOS/CoronaShell "..options.appBundleFile.."/Contents/MacOS/"..quoteString(options.dstFile) )

		runScript( "/bin/mkdir -p "..options.appBundleFile.."/Contents/Resources/Corona/" )

		-- We get the signingIdentity as a cert fingerprint but we also need the plaintext name
		if options.signingIdentity then
			options.signingIdentityName = captureCommandOutput("security find-identity -p codesigning -v | grep '".. options.signingIdentity .."' | sed -e 's/.*\\(\".*\"\\).*/\\1/'")
			if not options.signingIdentityName or options.signingIdentityName == "" then
				print("WARNING: identity '"..options.signingIdentity.."' does not appear to be valid for codesigning.  You should open Keychain Access and verify that your signing identities are valid")
			else
				print("Code signing identity: ".. options.signingIdentityName .." (".. options.signingIdentity ..")")
			end
		end

		if options and debugBuildProcess and debugBuildProcess ~= 0 then
			print("====================================")
			print("OSXPostPackage: options: "..json.prettify(options))
			print("====================================")
		end

		-- finalize package
		setStatus("Creating Info.plist")
		result = generateFiles( options )

		if result then
			return result
		end

		setStatus("Copying application assets")
		result = packageApp( options )

		if result then
			return result
		end

		-- If we have a plugins dir ...
		if lfs.attributes( options.tmpPluginsDir ) ~= nil then
			setStatus("Processing plugins")
			local appPluginsDirUnquoted = makepath(appBundleFileUnquoted, "Contents/Plugins")
			local appPluginsDir = quoteString(appPluginsDirUnquoted)

			-- Remove Lua files from tmpPluginsDir
			runScript( "/usr/bin/find "..quoteString(options.tmpPluginsDir).." \\( -name '*.lua' -o -name '*.lu' \\) -delete" )

			-- Make sure the plugins directory exists
			runScript( "/bin/mkdir -p "..appPluginsDir )

			-- Copy all files from tmpPluginsDir to the app's Plugins directory (that way we get Lua plugin assets)
			runScript( 'cp -Rv ' ..  quoteString(options.tmpPluginsDir) .."/* ".. appPluginsDir )

			-- Prune all empty directories
			runScript( 'find ' ..  appPluginsDir .. ' -type d -empty -delete' )

			-- sign all the plugins
			if options.signingIdentity then

				local result = signAllPlugins(appPluginsDirUnquoted, options.signingIdentity, options.xcodetoolhelper.codesign)
				if result ~= "" then
					return tostring(result)
				end

			end
		end

		-- Copy the resource.car to the app's Corona resource directory
		runScript( "/bin/cp -v "..quoteString(options.compiledLuaArchive).." "..options.appBundleFile.."/Contents/Resources/Corona/resource.car" )

		-- If we have a macOS icon with a new or old name ...
		local iconPath = makepath(options.srcAssets, "Icon-macos.icns")
		if lfs.attributes( iconPath ) == nil then
			iconPath = makepath(options.srcAssets, "Icon-osx.icns")
		end

		if lfs.attributes( iconPath ) ~= nil then
			-- Copy the macOS (if any) to the app's resource directory
			-- calling it Icon-osx.icns for backwards compatibility
			runScript( "/bin/cp -v "..quoteString(iconPath).." "..options.appBundleFile.."/Contents/Resources/Icon-osx.icns" )
		else
			print("WARNING: no Icon-macos.icns / Icon-osx.icns file found, using default icon")
		end

		setStatus("Cleaning up")
		-- Mark app as updated
		runScript( "/usr/bin/touch "..options.appBundleFile )

		-- bundle is now ready to be signed
		if options.signingIdentity then
			local entitlements_filename = os.tmpname() .. "_entitlements.xcent"
			local entitlements = entitlements_filename
			local result, includeProvisioning = generateOSXEntitlements( entitlements_filename, settings, provisionFile )
			if result ~= "" then
				entitlements = nil
			end

			-- Copy provisioning profile if we need it
			if includeProvisioning then
				runScript( "/bin/cp " .. quoteString(provisionFile) .. " " .. quoteString(makepath(appBundleFileUnquoted, "Contents/embedded.provisionprofile")) )
			end

			if entitlements ~= nil then
				setStatus("Sign application plugins")
			    runScript( getCodesignScript( nil, appBundleFileUnquoted, options.signingIdentity, options.xcodetoolhelper.codesign ) )
			end
			setStatus("Signing application with "..tostring(options.signingIdentityName))
			local result, errMsg = runScript( getCodesignScript( entitlements, appBundleFileUnquoted, options.signingIdentity, options.xcodetoolhelper.codesign ) )
			runScript( "/bin/rm -f " .. entitlements_filename )

			if result ~= 0 then
				errMsg = "ERROR: code signing failed: "..tostring(errMsg)
				runScript( "/bin/ls -Rl '"..appBundleFileUnquoted.."'")
				return errMsg
			end
		end

		-- Sometimes macOS gets confused about whether the apps we build are opennable, this
		-- tells macOS to flush its cache of such things
		runScript( "/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -u "..options.appBundleFile)
	end

	return err
end

--
-- OSXPackageForAppStore
--
-- Create an App Store package and optionally send it to iTunes Connect
--
-- returns an error message or nil on success
--
function OSXPackageForAppStore( params )

	local srcAssets = params.srcAssets
	local appSigningIdentity = params.appSigningIdentity
	local installerSigningIdentity = params.installerSigningIdentity
	local dstDir = params.dstDir
	local dstFile = params.bundledisplayname
	local sendToAppStore = params.sendToAppStore
	local osxAppEntitlementsFile = params.osxAppEntitlements
	local itunesConnectUsername = params.itc1
	local itunesConnectPassword = params.itc2
	local sdkRoot = params.xcodetoolhelper.sdkRoot
	local codesign = params.xcodetoolhelper.codesign
	local applicationLoader = params.xcodetoolhelper.applicationLoader
	local productbuild = params.xcodetoolhelper.productbuild
	local err = nil
	local appSigningIdentityName
	local installerSigningIdentityName
	local provisionFile = params.provisionFile

	print("Using Xcode utilities from "..sdkRoot)

	local customSettingsFile = srcAssets .. "/build.settings"
	if ( fileExists( customSettingsFile ) ) then
		local customSettings, msg = loadfile( customSettingsFile )
		if ( customSettings ) then
			-- this creates a global called "settings"
			local status, msg = pcall( customSettings )
			if status then
				print( "Using additional build settings from: " .. customSettingsFile )
			else
				err = "Warning: Error found in build.settings file:\n\t".. msg
				print(err)
			end
			-- forwards compatibility
			if settings.macos ~= nil then
				if settings.osx ~= nil then
					print( "WARNING: settings.macos overrides settings.osx in "..customSettingsFile )
				end
				settings.osx = settings.macos
			end
		else
			err = "Warning: Could not load build.settings file:\n\t".. msg
			print(err)
		end
	end

	local appBundleFile = makepath(dstDir, dstFile) .. ".app"

	if not appSigningIdentity or not installerSigningIdentity then
		return "ERROR: building an App Store package requires valid app and installer signing identities (if you've made changes to these recently you may need to restart the Corona Simulator to pick them up)"
	end

	-- We get the {app|installer}SigningIdentity as a cert fingerprint but we also need the plaintext name
	if appSigningIdentity then
		appSigningIdentityName = captureCommandOutput("security find-identity -p codesigning -v | grep '".. appSigningIdentity .."' | sed -e 's/.*\\(\".*\"\\).*/\\1/'")
		if not appSigningIdentityName or appSigningIdentityName == "" then
			print("WARNING: application identity '"..appSigningIdentity.."' does not appear to be valid for codesigning.  You should open Keychain Access and verify that your signing identities are valid")
		else
			print("Application signing identity: ".. appSigningIdentityName .." (".. appSigningIdentity ..")")
		end
	end
	if installerSigningIdentity then
		installerSigningIdentityName = captureCommandOutput("security find-identity -p basic -v | grep '".. installerSigningIdentity .."' | sed -e 's/.*\\(\".*\"\\).*/\\1/'")
		if not installerSigningIdentityName or installerSigningIdentityName == "" then
			print("WARNING: installer identity '"..installerSigningIdentity.."' does not appear to be valid for codesigning.  You should open Keychain Access and verify that your signing identities are valid")
		else
			print("Installer signing identity: ".. installerSigningIdentityName .." (".. installerSigningIdentity ..")")
		end
	end

	--
	-- Sign application with production identity
	--
	local entitlements_filename = os.tmpname() .. "_entitlements.xcent"
	local result, includeProvisioning = generateOSXEntitlements( entitlements_filename, settings, provisionFile )
	if result ~= "" then
		return "ERROR: generating entitlements file failed: "..tostring(result)
	end

	-- Copy provisioning profile if we need it
	if includeProvisioning then
		runScript( "/bin/cp " .. quoteString(provisionFile) .. " " .. quoteString(makepath(appBundleFile, "Contents/embedded.provisionprofile")) )
	end

	-- sign all the plugins
	local appPluginsDirUnquoted = makepath(appBundleFile, "Contents/Plugins")
	local result = signAllPlugins(appPluginsDirUnquoted, appSigningIdentity, codesign)
	if result ~= "" then
		return tostring(result)
	end

	setStatus("Sign application deep")
	runScript( getCodesignScript( nil, appBundleFile, appSigningIdentity, codesign ) )
	setStatus("Signing application with "..tostring(appSigningIdentityName))
	local result, errMsg = runScript( getCodesignScript( entitlements_filename, appBundleFile, appSigningIdentity, codesign ) )

	runScript( "/bin/rm -f " .. entitlements_filename )

	if result ~= 0 then
		errMsg = "ERROR: code signing failed: "..tostring(errMsg)
		runScript( "/bin/ls -Rl '"..appBundleFile.."'")
		return errMsg
	end

	--
	-- Create package
	--
	setStatus("Packaging application with "..tostring(appSigningIdentityName))
	local productFile = appBundleFile:gsub('.app$', '') .. ".pkg"
	local result, errMsg = runScript( getProductBuildScript( appBundleFile, installerSigningIdentity, productbuild ) )

	if result ~= 0 then
		errMsg = "ERROR: creating installation package failed: "..tostring(errMsg)
		return errMsg
	end

	if itunesConnectUsername and itunesConnectUsername ~= "" then
		--
		-- Validate package with iTunes Connect
		--
		setStatus("Validating application package with iTunes Connect")
		local result = captureCommandOutput( getProductValidateScript( appBundleFile, itunesConnectUsername, itunesConnectPassword, applicationLoader ), 2 )

		if result ~= "" then
			if result:match('Unable to validate archive') then
				-- Validation requires an internet connection but the error message is somewhat inscrutable
				result = result .. "\n\n(make sure you are connected to the internet)"
			end

			errMsg = "ERROR: validation failed: "..tostring(result)
			return errMsg
		end
	end

	if sendToAppStore then
		--
		-- Upload package to iTunes Connect
		--
		setStatus("Uploading application package to iTunes Connect")
		local result = captureCommandOutput( getProductUploadScript( appBundleFile, itunesConnectUsername, itunesConnectPassword, applicationLoader ), 2 )

		if result ~= "" then
			errMsg = "ERROR: upload failed: "..tostring(result)
			return errMsg
		end
	else
		-- Not sending to App Store so just show what we built in the Finder
		runScript( getShowFileInFinderScript( productFile ) )
	end

	return nil
end

--
-- OSXPackageForSelfDistribution
--
-- Sign app for self distribution and optionally create an installer and DMG
--
-- returns an error message or nil on success
--
function OSXPackageForSelfDistribution( params )

	local appBundle = params.appBundle
	local appSigningIdentity = params.appSigningIdentity
	local installerSigningIdentity = params.installerSigningIdentity
	local tmpDir = params.tmpDir
	local dstDir = params.dstDir
	local dstFile = params.bundledisplayname
	local createInstaller = params.createInstaller
	local createDMG = params.createDMG
	local sdkRoot = params.xcodetoolhelper.sdkRoot
	local codesign = params.xcodetoolhelper.codesign
	local err = nil

	local appBundleFile = makepath(dstDir, dstFile) .. ".app"
	local productFile = appBundleFile
	local appSigningIdentityName = ""

	-- We get the appSigningIdentity as a cert fingerprint but we also need the plaintext name (unless it's "None")
	if appSigningIdentity and appSigningIdentity ~= "None" then
		appSigningIdentityName = captureCommandOutput("security find-identity -p codesigning -v | grep '".. appSigningIdentity .."' | sed -e 's/.*\\(\".*\"\\).*/\\1/'")
		if not appSigningIdentityName or appSigningIdentityName == "" then
			print("WARNING: application identity '"..appSigningIdentity.."' does not appear to be valid for codesigning.  You should open Keychain Access and verify that your signing identities are valid")
		else
			print("Application signing identity: ".. appSigningIdentityName .." (".. appSigningIdentity ..")")
		end

		--
		-- Sign application with production identity
		--
		-- sign all the plugins
		local appPluginsDirUnquoted = makepath(appBundleFile, "Contents/Plugins")
		local result = signAllPlugins(appPluginsDirUnquoted, appSigningIdentity, codesign)
		if result ~= "" then
			return tostring(result)
		end

		setStatus("Signing application with "..tostring(appSigningIdentityName))
		local result, errMsg = runScript( getCodesignScript( nil, appBundleFile, appSigningIdentity, codesign ) )

		if result ~= 0 then
			errMsg = "ERROR: code signing failed: "..tostring(errMsg)
			runScript( "/bin/ls -Rl '"..appBundleFile.."'") -- to aid debugging
			return errMsg
		end
	end

	if createDMG then
		--
		-- Create DMG
		--
		setStatus("Creating DMG for application")
		productFile = appBundleFile:gsub('.app$', '.dmg')

		if lfs.attributes( productFile ) ~= nil then
			-- Remove existing DMG file as hdiutil wont overwrite it
			local result, errMsg = os.remove(productFile)
			if not result then
				errMsg = "ERROR: removing old DMG failed: "..tostring(errMsg)
				return errMsg
			end
		end

		local result, errMsg = runScript( getCreateDMGScript( appBundleFile ) )

		if result ~= 0 then
			errMsg = "ERROR: creating DMG failed: "..tostring(errMsg)
			return errMsg
		end
	end

	-- Show what we built in the Finder
	runScript( getShowFileInFinderScript( productFile ) )

	return nil
end


