------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

----
---- Complete build of iOS app by merging local project assets with app built
---- on Corona servers.
----

local json = require('json')
local lfs = require('lfs')
local simAvail, simulator = pcall(require, "simulator")

-- defines modifyPlist()
local CoronaPListSupport = require("CoronaPListSupport")
local captureCommandOutput = CoronaPListSupport.captureCommandOutput

local coronaLiveBuildAppDir = "_corona_live_build_app" -- "PackageApp" --
local coronaLiveBuildManifest = "_corona_live_build_manifest.txt"
local coronaLiveBuildExclude = "_corona_live_build_exclude.txt"

if not simAvail then
	simulator = nil
end

-- get the numeric value of the "debugBuildProcess" preference or 0 if it's not set (note due to a Lua bug
-- the value is actually the exit code multiplied by 256)
local debugBuildProcess = os.execute("exit $(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null || echo 0)") / 256

local xcodetoolhelper = nil

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
-- sanitize Bundle ID STring
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
local function getCopyResourcesScript( src, dst, should_preserve, live_build, options )

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
	--             --recursive             recurse into directories
	--             --links                 copy symlinks as symlinks
	--             --copy-unsafe-links	   copy external symlinked files to destination
	--             --hard-links            preserve hard links
	--             --times                 preserve times
	--             --perms                 preserve permissions (needed by next option)
	--             --chmod                 specify base level permissions (prevents us generating bad apps)
	--   debug:
	--             --verbose
	--             --itemize-changes       output a change-summary for all updates
	--
	local script = [==[
	exec 2>&1
	# rsync goes haywire if either the source or destination are empty
	if [ -z "$SRC_DIR" -o -z "$DST_DIR" ]
	then
		echo "iPhonePackageApp: invalid SRC_DIR or DST_DIR for rsync"
		exit 1
	fi

	rsync {{DEBUG_FLAGS}} -r --times --delete-excluded {{EXCLUDED_FILES}} {{EXCLUDE_LUA_AND_BUILD_SETTINGS}} --exclude="**/.*"  --delete --prune-empty-dirs --links --copy-unsafe-links --hard-links --perms --chmod="Da+rx,Fa+r" --human-readable "$SRC_DIR/" "$DST_DIR"

	# run pngcrush appropriately
	if [ "$SHOULD_PRESERVE" == "NO" ]
	then
		COPYPNG=$(xcrun -f copypng)
		find "$DST_DIR" -name '*.png' -exec "$COPYPNG" -compress {} {}.copypng \; -exec mv {}.copypng {} \;
	fi
]==]

	local excludedFilesSh = ""

	if options and options.settings then
		-- Generate shell code from the excludeFiles entry in build.settings
		if options.settings.excludeFiles then
			-- we have actual files to exclude
			print("Excluding specified files from build: ")
			for platform,excludes in pairs(options.settings.excludeFiles) do
				if platform == "all" or platform == "ios" or platform == "iphone" then
					for index,pattern in ipairs(excludes) do
						print("   excluding: "..pattern)
						pattern = pattern:gsub("'", "'\\''") -- quote single quotes
						excludedFilesSh = excludedFilesSh .. " --exclude='" .. pattern .."'"
					end
				end
			end
		end
	end

	local args = ""

	if debugBuildProcess > 0 then
		args = args.."set -x\n"
	end

	args = args.."SRC_DIR="..src.."\n"

	if live_build then
		dst = makepath(dst, coronaLiveBuildAppDir)
	end

	args = args.."DST_DIR="..dst.."\n"

	if not should_preserve then
		args = args.."SHOULD_PRESERVE=NO\n"
	end

	-- Replace the placeholders in the script with the generated code (or an empty string if there was none)
	local debugFlags = (debugBuildProcess > 0) and "-v --itemize-changes" or ""
	local excludeLuaAndBuildSettings = (live_build) and "" or "--exclude='**/*.lua' --exclude='build.settings'" -- don't exclude Lua files and build.settings for live builds
	script = script:gsub("{{EXCLUDED_FILES}}", excludedFilesSh)
	script = script:gsub("{{EXCLUDE_LUA_AND_BUILD_SETTINGS}}", excludeLuaAndBuildSettings)
	script = script:gsub("{{DEBUG_FLAGS}}", debugFlags)

	script = args .. script

	if debugBuildProcess > 0 then
		print("getCopyResourcesScript: ".. script)
	end

	return script
end

--
-- Generate the Live Build manifest file generation script
--
-- Format:
--
-- 0 / 1476221154 / /main.lua //
-- 0 / 1476232425 / /subdir/ //
-- 0 / 1476232425 / /subdir/file.lua //
-- 0 / 1476212136 / /world.jpg //
--
local function getLiveBuildManifestScript(appDir, manifestFile)
	-- local genManifestSh = "cd ".. appDir .." && find . -print0 | xargs -0 stat -f '%m %N' > " .. manifestFile

	local genManifestSh = "cd ".. appDir .." &&  find . -print0 | xargs -0 stat -f '0 / %m / %N%T //' | sed -e 's![*@] //$! //!' -e 's!/ \\./!/ /!' > "..manifestFile

	return genManifestSh
end

--
-- Create a script to copy the original apps icons to the correct locations in the live build app
--
local function getLiveBuildCopyIconScript(src, dest, options)
	local script = ""
	local iconFiles = (options.settings and options.settings.iphone and options.settings.iphone.plist and options.settings.iphone.plist.CFBundleIconFiles) or nil;

	local iconsToCopy = 0
	if iconFiles and type(iconFiles) == "table" then
		-- copy all the icon files to the destination folder preserving any subdirectories
		for k, iconFile in pairs(iconFiles) do
			iconsToCopy = iconsToCopy + 1
			script = script .." ".. quoteString(iconFile)
		end

		local debugFlags = (debugBuildProcess > 0) and "-v --itemize-changes " or ""

		script = "cd ".. src .." && rsync --relative --times --links --safe-links --hard-links --perms --chmod='Da+rx,Fa+r' ".. debugFlags .. script .." ".. dest

		if debugBuildProcess > 0 then
			print("getLiveBuildCopyIconScript: ".. script)
		end

	end

	if iconsToCopy > 0 then
		return script
	else
		return nil
	end
end

--------------------------------------------------------------------------------
local function getCodesignScript( entitlements, path, identity, developerBase )

	codesign_allocate = xcodetoolhelper['codesign_allocate']
	codesign = xcodetoolhelper['codesign']

    -- Remove any extended macOS attributes from files in the bundle because
    -- codesign doesn't like them
    local removeXattrs = "/usr/bin/xattr -cr "..quoteString(path) .." && "

	-- quote for spaces
	codesign_allocate = quoteString(codesign_allocate)
	codesign = quoteString(codesign)
	developerBase = quoteString(developerBase)

	local devbase_shell = "DEVELOPER_BASE="..developerBase.."\n"
	local export_path = [==[
export PATH="$DEVELOPER_BASE/Platforms/iPhoneOS.platform/Developer/usr/bin:$DEVELOPER_BASE/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
]==]

	local script = 'export CODESIGN_ALLOCATE=' .. codesign_allocate .. '\n'

	local quotedpath = quoteString( path )
	local cmd = removeXattrs .. codesign .. " --verbose -f -s "..quoteString(identity).." --entitlements "..entitlements.." "..quotedpath

	-- print("getCodesignScript: codesign: ".. codesign)
	-- print("getCodesignScript: codesign_allocate: ".. codesign_allocate)
	-- print("getCodesignScript: ".. devbase_shell .. export_path .. script .. cmd)
	return devbase_shell .. export_path .. script .. cmd
end

local function getCodesignAPPXScriptAndPackage(path, identity, entitlements, developerBase, bundleId, isBuildForDistribution)

    local codesign_allocate = xcodetoolhelper['codesign_allocate']
    local codesign = xcodetoolhelper['codesign']

    -- Remove extended attributes that may interfere with codesign
    local appxPath = path:gsub('["\']', "") .. "/PlugIns"
    local removeXattrs = "/usr/bin/xattr -cr " .. quoteString(appxPath)

    -- Quote paths to handle spaces
    codesign_allocate = quoteString(codesign_allocate)
    codesign = quoteString(codesign)
    developerBase = quoteString(developerBase)

    -- Shell script setup
    local devbase_shell = "DEVELOPER_BASE=" .. developerBase .. "\n"
    local export_path = [==[
export PATH="$DEVELOPER_BASE/Platforms/iPhoneOS.platform/Developer/usr/bin:$DEVELOPER_BASE/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
]==]

    local script = 'export CODESIGN_ALLOCATE=' .. codesign_allocate .. '\n'

    -- Start command with attribute cleanup
    local cmd = removeXattrs 

    -- Check if PlugIns folder exists and sign each .appex file
    if lfs.attributes(appxPath, "mode") == "directory" then
        for file in lfs.dir(appxPath) do
            if file:match("%.appex$") then
                local appexPath = appxPath .. "/" .. file
                local infoPlist = quoteString(appexPath .. "/Info.plist")

                -- Extract CFBundleIdentifier and get the last component
                local getBundleIdCmd = "/usr/bin/plutil -extract CFBundleIdentifier xml1 -o - " .. infoPlist .. 
                                       " | sed -n 's/.*<string>\\(.*\\)<\\/string>.*/\\1/p' | awk -F'.' '{print $NF}'"

                -- Set new CFBundleIdentifier
                local setBundleIdCmd = "newBundleId=\"" .. bundleId .. ".$(" .. getBundleIdCmd .. ")\" && " ..
                                       "/usr/bin/plutil -replace CFBundleIdentifier -string \"$newBundleId\" " .. infoPlist

                local bundleIdOutput = io.popen(getBundleIdCmd):read("*a"):gsub("%s+", "")  -- Clean up any extra spaces

                -- Run the command to update CFBundleIdentifier
                runScript(setBundleIdCmd)

                -- Determine which mobile provision profile to use based on `isBuildForDistribution`
                local mobileProvisionPath = path:gsub('["\']', "") .. "/iOSAppxFiles/" .. bundleIdOutput .. "/"
                local selectedProvision = isBuildForDistribution and 
                                          (mobileProvisionPath .. "embedded.mobileprovision") or 
                                          (lfs.attributes(mobileProvisionPath .. "embedded_dev.mobileprovision", "mode") == "file" and 
                                          mobileProvisionPath .. "embedded_dev.mobileprovision" or 
                                          mobileProvisionPath .. "embedded.mobileprovision")

                if not lfs.attributes(selectedProvision, "mode") then
                    print("Warning: Cannot find mobile provision profile for " .. bundleIdOutput ..
                          " at " .. selectedProvision .. " This may cause issues with your app.")
                else
                    -- Copy the mobile provision profile to the .appex bundle and delete the old one
                    local copyMobileProvisionCmd = "cp " .. quoteString(selectedProvision) .. " " ..
                                                   quoteString(appexPath .. "/embedded.mobileprovision") ..
                                                   " && rm -rf " .. quoteString(mobileProvisionPath)
                    runScript(copyMobileProvisionCmd)

                    -- Append signing command for .appex
                    cmd = cmd .. " && " .. codesign .. " --verbose -f -s " .. quoteString(identity) .. 
                          " --entitlements " .. entitlements .. " " .. quoteString(appexPath)
                end
            end
        end    
    end

    return devbase_shell .. export_path .. script .. cmd
end




local function getCodesignFrameworkScript( path, identity, developerBase )

	local codesign_framework = xcodetoolhelper['codesign_framework']

	-- Remove any extended macOS attributes from files in the bundle because
	-- codesign doesn't like them
	local removeXattrs = "/usr/bin/xattr -cr "..quoteString(path) .." && "

	-- quote for spaces
	codesign_framework = quoteString(codesign_framework)
	developerBase = quoteString(developerBase)

	local exports_shell = 'export DEVELOPER_BASE='..developerBase..'\n'
							.. 'export CODE_SIGNING_REQUIRED="YES"\n'
							.. 'export EXPANDED_CODE_SIGN_IDENTITY='..quoteString(identity)..'\n'
							.. 'export BUILT_PRODUCTS_DIR='..quoteString(path)..'\n'
							.. 'export FRAMEWORKS_FOLDER_PATH=Frameworks\n'

	local cmd = removeXattrs .. codesign_framework

	-- print("getCodesignFrameworkScript: codesign_framework: ".. codesign_framework)
	-- print("getCodesignFrameworkScript: ".. exports_shell .. cmd)
	return exports_shell .. cmd
end

local function getProductValidateScript( path, itunesConnectUsername, itunesConnectPassword, applicationLoader )

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
	local cmd = quoteString(altool) .. " --validate-app -f ".. quoteString(path) .." -u '".. itunesConnectUsername .."' -p '".. itunesConnectPassword .."' --output-format xml | grep -v '^<!DOCTYPE' | /usr/bin/xpath '/plist/dict/key[.=\"product-errors\"]/following-sibling::*[1]//key[.=\"message\"]/following-sibling::*[1]/node()' 2>/dev/null  | sed -ne 'p'"

	return cmd
end

local function getProductUploadScript( path, itunesConnectUsername, itunesConnectPassword, applicationLoader )

	-- Apple tells us to use this buried utility to automate Application Loader tasks in
	-- https://itunesconnect.apple.com/docs/UsingApplicationLoader.pdf
	local altool = makepath(applicationLoader, "Contents/Frameworks/ITunesSoftwareService.framework/Support/altool")

	-- Removing the "!DOCTYPE" line from the XML stops xpath trying to access the DTD
	-- The xpath command parses the XML output looking for an error message
	-- The final sed command adds a newline to the last line of output which xpath omits
	local cmd = quoteString(altool) .. " --upload-app -f ".. quoteString(path) .." -u '".. itunesConnectUsername .."' -p '".. itunesConnectPassword .."' --output-format xml | grep -v '^<!DOCTYPE' | /usr/bin/xpath '/plist/dict/key[.=\"product-errors\"]/following-sibling::*[1]//key[.=\"message\"]/following-sibling::*[1]/node()' 2>/dev/null | sed -ne 'p'"

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
		errMsg = captureCommandOutput( "cat " .. tmpFile )
		print( errMsg )
	end

	os.remove( tmpFile )

	return exitCode, errMsg
end

--------------------------------------------------------------------------------

-- check if app has any appx in app folder
local function checkAppHasAPPX( appPath )
	local pluginsPath = appPath:gsub('["\']', "") .. "/PlugIns"

	-- Check if PlugIns folder exists
	if lfs.attributes(pluginsPath, "mode") == "directory" then

		-- Find .appex files inside PlugIns
		for file in lfs.dir(pluginsPath) do
			if file:match("%.appex$") then
				return true
			end
		end
	else
		return false
	end
end

--------------------------------------------------------------------------------

-- Assumes pwd is same as this script's directory

local templateXcent = [[
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>application-identifier</key>
	<string>{{APPLICATION_IDENTIFIER}}</string>
	<key>keychain-access-groups</key>
	<array>
		<string>{{APPLICATION_IDENTIFIER}}</string>
	</array>
{{OTHER_OPTIONS}}
{{BETA_REPORTS_ACTIVE}}
{{GET_TASK_ALLOW}}
{{CUSTOM_ENTITLEMENTS}}
</dict>
</plist>
]]

local templateBetaReportsActive = [[
	<key>beta-reports-active</key>
	<{{BETA_REPORTS}}/>]]

local templateGetTaskAllow = [[
	<key>get-task-allow</key>
	<{{GET_TASK}}/>]]

local templateOtherOptions = [[
	<key>aps-environment</key>
    <string>{{PUSH_ENVIRONMENT}}</string>]]

local function generateXcent( options )
	-- Skip/escape if there is no signingIdentity (e.g. Xcode Simulator)
	if not options.signingIdentity then
		return
	end

	local filename = options.tmpDir .. "/entitlements.xcent"
	local outFile = assert( io.open( filename, "wb" ) )

	local data = templateXcent

	-- Set "get-task-allow" to the same value as in the provisioning profile
	local get_task_allow_setting = captureCommandOutput("security cms -D -i '".. options.mobileProvision .."' | plutil -p - | fgrep 'get-task-allow'")
	if debugBuildProcess and debugBuildProcess ~= 0 then
		print("get_task_allow_setting: ".. tostring(get_task_allow_setting))
	end

	if get_task_allow_setting ~= "" then
		-- set the value appropriately
		if nil ~= string.find( get_task_allow_setting, "1", 1, true ) then
			templateGetTaskAllow, numMatches = string.gsub( templateGetTaskAllow, "{{GET_TASK}}", "true" )
			assert( numMatches == 1 )
		else
			templateGetTaskAllow, numMatches = string.gsub( templateGetTaskAllow, "{{GET_TASK}}", "false" )
			assert( numMatches == 1 )
		end
		data, numMatches = string.gsub( data, "{{GET_TASK_ALLOW}}", templateGetTaskAllow )
		assert( numMatches == 1 )
	else
		-- Remove the substition marker
		data, numMatches = string.gsub( data, "{{GET_TASK_ALLOW}}", "" )
		assert( numMatches == 1 )
	end

	-- We can only put things in the .xcent file that are in the provisioning profile.  Current distribution profiles all have "beta-reports-active"
	-- but older ones do not and while it's generally set to true if present, this isn't relied upon
	local beta_reports_active_setting = captureCommandOutput("security cms -D -i '".. options.mobileProvision .."' | plutil -p - | fgrep 'beta-reports-active'")
	if debugBuildProcess and debugBuildProcess ~= 0 then
		print("beta_reports_active_setting: ".. tostring(beta_reports_active_setting))
	end

	if beta_reports_active_setting ~= "" then
		-- set the value appropriately
		if nil ~= string.find( beta_reports_active_setting, "1", 1, true ) then
			templateBetaReportsActive, numMatches = string.gsub( templateBetaReportsActive, "{{BETA_REPORTS}}", "true" )
			assert( numMatches == 1 )
		else
			templateBetaReportsActive, numMatches = string.gsub( templateBetaReportsActive, "{{BETA_REPORTS}}", "false" )
			assert( numMatches == 1 )
		end
		data, numMatches = string.gsub( data, "{{BETA_REPORTS_ACTIVE}}", templateBetaReportsActive )
		assert( numMatches == 1 )
	else
		-- Remove the substition marker
		data, numMatches = string.gsub( data, "{{BETA_REPORTS_ACTIVE}}", "" )
		assert( numMatches == 1 )
	end

	local numMatches = 0
	data, numMatches = string.gsub( data, "{{APPLICATION_IDENTIFIER}}", options.appid )
	assert( numMatches == 2 )

	local otherOptions = "" -- default is to assume no other options exist
	if ( "string" == type( options.pushEnvironment ) ) then
		otherOptions, numMatches = string.gsub( templateOtherOptions, "{{PUSH_ENVIRONMENT}}", options.pushEnvironment )
		assert( numMatches == 1 )
	end

	data, numMatches = string.gsub( data, "{{OTHER_OPTIONS}}", otherOptions )
	assert( numMatches == 1 )


	local generatedEntitlements = CoronaPListSupport.generateEntitlements(options.settings, 'iphone', options.mobileProvision)
	data, numMatches = string.gsub( data, "{{CUSTOM_ENTITLEMENTS}}", generatedEntitlements )
	assert( numMatches == 1 )

--[[
	local len = string.len( data )
	local lenHex = string.format( "%08x", len )

	local head = string.char( tonumber( 0xFA ), tonumber( 0xDE ), tonumber( 0x71 ), tonumber( 0x71 ) )
	data = string.gsub( data, "HEAD", head )

	-- translate 8 digit hex number into 4 raw bytes
	lenHex = string.char(	tonumber( lenHex:sub( 1, 2 ), 16 ),
							tonumber( lenHex:sub( 4, 4 ), 16 ),
							tonumber( lenHex:sub( 5, 6 ), 16 ),
							tonumber( lenHex:sub( 7, 8 ), 16 ) )

	assert( lenHex:len() == 4 )
	data = string.gsub( data, "LEN0", lenHex )
--]]
	outFile:write(data)
	assert( outFile:close() )

	print( "Created XCENT: " .. filename );

	if debugBuildProcess and debugBuildProcess ~= 0 then
		runScript("cat "..filename)
	end
end


-- xcprivacy
local templateXcprivacy = [[
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
{{CUSTOM_XCPRIVACY}}
</dict>
</plist>
]]
local function generateXcprivacy( options )
	local filename = options.tmpDir .. "/PrivacyInfo.xcprivacy"
	local outFile = assert( io.open( filename, "wb" ) )

	local data = templateXcprivacy

	local generatedPrivacy = CoronaPListSupport.generateXcprivacy( options.settings, 'iphone')
	if(  generatedPrivacy and generatedPrivacy ~= "" ) then
		data, numMatches = string.gsub( data, "{{CUSTOM_XCPRIVACY}}", generatedPrivacy )
		assert( numMatches == 1 )
	end

	outFile:write(data)
	assert( outFile:close() )

	print( "Created XCPRIACY: " .. filename );

	if debugBuildProcess and debugBuildProcess ~= 0 then
		runScript("cat "..filename)
	end
end


--
-- generateFiles
--
-- Create the .xcent, .xcprivacy, and Info.plist files
--
-- returns an error message or nil on success
--
local function generateFiles( options )
	local result = nil

	result = generateXcprivacy( options )
	if result then
		return result
	end

	result = generateXcent( options )
	if result then
		return result
	end

	result = CoronaPListSupport.modifyPlist( options )
	if result then
		return result
	end

	return result
end

-- True for Ad Hoc or Store builds
local function isBuildForDistribution( options )
	if not options.signingIdentityName then
		return false
	end

	local retval = string.match( options.signingIdentityName, "iPhone Distribution" ) or string.match( options.signingIdentityName, "Apple Distribution" )
	if retval then
		return true
	else
		return false
	end
end

-- helper function to figure out if building for App Store distribution
-- returns true or false
local function isBuildForAppStoreDistribution( options )

	if not options.signingIdentityName then
		return false
	end

	local retval = string.match( options.signingIdentityName, "iPhone Distribution" ) or string.match( options.signingIdentityName, "Apple Distribution" )
	if retval then
		-- Adhoc profiles have a 'ProvisionedDevices' section, pure distribution profiles do not
		local hasProvisionedDevices = captureCommandOutput("security cms -D -i '".. options.mobileProvision .."' | fgrep -c 'ProvisionedDevices'")
		if hasProvisionedDevices == "0" then
			return true
		else
			return false
		end
	else
		return false
	end
end

-- this function moves files from main bundle to target OnDemandResources directory
local function generateOdrFileStructure(bundleDir, destDir, odr, appBundleId)
	local err = nil
	if type(odr) ~= "table" then
		return " 'onDemandResources' should be array of tables with tag and resource fields."
	end

	for i = 1,#odr do
		local tag, resource = odr[i].tag, odr[i].resource
		if type(tag)=="string" and type(resource)=="string" then
			local tagBundle = appBundleId .. "." .. tag
			local script = 'SRC_DIR='..quoteString(bundleDir) ..'\n'
			script = script ..'DST_DIR='..quoteString(destDir) .. '\n'
			script = script .. 'BNDL_DIR="$DST_DIR/' .. tagBundle .. '.assetpack"\n'
			script = script .. 'BNDL_REL_SRC="' .. resource .. '"\n'
			script = script .. [==[
BSF="$(dirname "$BNDL_REL_SRC")"
/bin/mkdir -p "$BNDL_DIR/$BSF"
/bin/mv "$SRC_DIR"/"$BNDL_REL_SRC" "$BNDL_DIR/$BSF/"
]==]
			local result, errMsg = runScript( script )
			if result ~= 0 then
				return "error running script " .. tostring(errMsg)
			end

		else
			return " invalid On-Demand Resources Entry: " .. json.encode(odr[i]) .. "; 'onDemandResources' should be array of tables with tag and resource fields."
		end
  end
  return nil
end

-- generates plists for ODR resources. One describing each bundle and some in app bundle to aggregate them
local function generateOdrPlists (bundleDir, odrDir, odr, appBundleId)

	odrDir = odrDir .. "/"

  local tagPacks = {}
  local packResources = {}
	-- still to figure out how to create self hosted ODR. Uncomment related lines to generate AssetPackManifestTemplate.plist
  -- local resources = {}

  -- first generate individual bundles Info.plist
  local c = 1
	for i = 1,#odr do
		local tag, resource = odr[i].tag, odr[i].resource
    local assetBundleDir = odrDir .. appBundleId .. "." .. tag .. '.assetpack'
    local plistPath = assetBundleDir .. '/Info.plist'
    local packBundleId = string.format(appBundleId .. ".asset-pack-%013d", c)
    c = c+1

    tagPacks[tag] = {NSAssetPacks={packBundleId}}
    packResources[packBundleId] = {resource}
    -- resources[#resources+1] = {
    --   URL="http://127.0.0.1" .. assetBundleDir,
    --   uncompressedSize=tonumber(captureCommandOutput('find "'.. assetBundleDir .. '" -type f -exec ls -l {} \\; | awk \'{sum += $5} END {print sum}\'')) or 0,
    --   primaryContentHash={
    --     hash=captureCommandOutput('stat -f"%Sm" -t "%F %T.000" "' .. assetBundleDir .. '"' ),
    --     strategy="modtime"
    --   },
    --   bundleKey=packBundleId,
    --   isStreamable=true,
    -- }

    local f = io.open(plistPath, "w")
		if not f then
			return "unable to create ODR file " .. plistPath
		end
		local resultBundle = {CFBundleIdentifier=packBundleId,Tags={tag}}
		if odr[i].type == "prefetch" then
			resultBundle.Priority = 0.5
		elseif odr[i].type == "install" then
			resultBundle.Priority = 1
		end
    f:write(json.encode(resultBundle))
    f:close()
    local result, errMsg = runScript( '/usr/bin/plutil -convert binary1 '..quoteString(plistPath) )
		if result ~= 0 then
			return "plutil error: " .. errMsg
		end
  end

  -- now aggregate them into OnDemandResources.plist
  local plistPath = bundleDir .. "/OnDemandResources.plist"
  local f = io.open(plistPath, "w")
	if not f then
		return "unable to create ODR file " .. plistPath
	end
  f:write(json.encode( {NSBundleResourceRequestTags=tagPacks, NSBundleResourceRequestAssetPacks=packResources} ))
  f:close()
	local result, errMsg = runScript( '/usr/bin/plutil -convert binary1 '..quoteString(plistPath) )
	if result ~= 0 then
		return "plutil error: " .. errMsg
	end


  -- -- AssetPackManifestTemplate.plist for self hosted ODRs
  -- local plistPath = bundleDir .. "/AssetPackManifestTemplate.plist"
  -- local f = io.open(plistPath, "w")
	-- if not f then
	-- 	return "Error while creating ODR file: " .. plistPath
	-- end
  -- f:write(json.encode( {resources=resources} ))
  -- f:close()
	-- local result, errMsg = runScript( 'plutil -convert binary1 '..quoteString(plistPath) )
	-- if result ~= 0 then
	-- 	return errMsg
	-- end

end

-- this will generate script for signing each bundle individually
local function generateOdrCodesign(destDir, odr, appBundleId, identity, developerBase)

	local codesign_allocate = xcodetoolhelper['codesign_allocate']
	local codesign = xcodetoolhelper['codesign']

	-- quote for spaces
	codesign_allocate = quoteString(codesign_allocate)
	codesign = quoteString(codesign)
	developerBase = quoteString(developerBase)

	local devbase_shell = "export DEVELOPER_BASE="..developerBase.."\n"
	local export_path = [==[
export PATH="$DEVELOPER_BASE/Platforms/iPhoneOS.platform/Developer/usr/bin:$DEVELOPER_BASE/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
]==]

	local allocate_script = 'export CODESIGN_ALLOCATE=' .. codesign_allocate .. '\n\n'

	for i = 1,#odr do
		local script = devbase_shell .. export_path .. allocate_script
		local tag, resource = odr[i].tag, odr[i].resource
		local quotedpath = quoteString(makepath(destDir,appBundleId .. "." .. tag .. ".assetpack"))
		script = script .. codesign .. " --verbose -f -s "..quoteString(identity).." "..quotedpath .. "\n"
		local result, errMsg = runScript( script )
		if result ~= 0 then
			return " codesigning error for  " .. tostring(resource) .. ": "..tostring(errMsg)
		end
	end
  return nil
end

--
-- prePackageApp
--
-- Copy the assets from the project directory to the app bundle
--
-- returns an error message or nil on success
--
local function prePackageApp( bundleDir, options )

	-- If "skipPNGCrush" is specified in the build.settings then set "should_preserve" to its value by default
	local should_preserve = (options.settings ~= nil and options.settings.iphone ~= nil and options.settings.iphone.skipPNGCrush) or false;

	-- options.osPlatform will be nil or 0 for iOS, 2 for Mac
	local deviceType = "iphone" -- iphone (meaning iOS) is default
	if options.osPlatform == 2 then
		should_preserve = true
	elseif options.targetDevice >= 128 then
		should_preserve = true
	end

	-- On developer builds, we also skip pngcrush
	if not should_preserve then
		local isDeveloperBuild = not isBuildForDistribution( options )
		if isDeveloperBuild then
			should_preserve = true
		end
	end

	-- warn if one of the necessary launch screen settings isn't present
	local sectionName = (options.settings ~= nil and options.settings.ios ~= nil) and "ios" or "iphone"
	if options.settings == nil or options.settings.iphone == nil or options.settings.iphone.plist == nil then
		print("WARNING: missing "..sectionName.." / "..sectionName..".plist section in build.settings")
	elseif options.settings.iphone.plist.UILaunchStoryboardName == nil and options.settings.iphone.plist.UILaunchImages == nil then
		print("WARNING: iOS builds require "..sectionName..".plist.UILaunchStoryboardName or "..sectionName..".plist.UILaunchImages in build.settings")
	end

	setStatus("Copying app resources")
	local result, errMsg = runScript( getCopyResourcesScript( quoteString( options.srcAssets ), quoteString(bundleDir), should_preserve, options.liveBuild, options ))

	if result ~= 0 then
		return "ERROR: copying resources failed: "..tostring(errMsg)
	end

	if options.liveBuild then
		setStatus("Generating Live Build manifest")
		local result, errMsg = runScript( getLiveBuildManifestScript( quoteString(makepath(bundleDir, coronaLiveBuildAppDir)), quoteString(makepath(bundleDir, coronaLiveBuildManifest))) )

		if result ~= 0 then
			return "ERROR: generating Live Build manifest failed: "..tostring(errMsg)
		end

		-- Fixup the icon files so that the live build gets the right icon on the device
		local copyIconsScript = getLiveBuildCopyIconScript(quoteString(options.srcAssets), quoteString(bundleDir), options)
			if copyIconsScript then
			local result, errMsg = runScript( copyIconsScript )

			if result ~= 0 then
				print("ERROR: copying Live Build icons failed: "..tostring(errMsg))
			end
		end

		-- Copy exclude patterns to file visible to live build client
		local excludePatterns = ""
		if options and options.settings and options.settings.excludeFiles then
			for platform, excludes in pairs(options.settings.excludeFiles) do
				if platform == "all" or platform == "ios" or platform == "iphone" then
					for index,pattern in ipairs(excludes) do
						excludePatterns = excludePatterns .. pattern .. "\n"
					end
				end
			end
		end
		local f = assert(io.open(makepath(bundleDir, coronaLiveBuildExclude), "wb"))
		f:write( excludePatterns )
		f:close()
	end

	return nil
end

--
-- packageApp
--
-- Sign the app and create the IPA (if required)
--
-- returns an error message or nil on success
--
local function packageApp( options )
	-- Introducing an unquoted string because we should be quoting as the final set after appending strings, not before.
	-- See casenum: 1008
	local appBundleFileUnquoted = makepath(options.dstDir, options.dstFile) .. ".app"
	-- Quoting should happen around the final string, otherwise you get quotes in multiple places which might not be what you expect.
	local appBundleFile = quoteString( appBundleFileUnquoted )

	local iPhoneSDKRoot = options.sdkRoot or "/Applications/Xcode.app/Contents/Developer"
	local builtForAppStore = isBuildForAppStoreDistribution( options )

	-- package on demand resources
	local odrOutputDir = nil
	if options.settings and options.settings.iphone and options.settings.iphone.onDemandResources and isBuildForAppStoreDistribution( options ) then
		setStatus("Generating On-Demand Resource bundles")
		local odrData = options.settings.iphone.onDemandResources

		-- step 1: move resources into appropriate folders
		odrOutputDir = captureCommandOutput('mktemp -d -t CLtmpXXXXXX_ODR') .. "/OnDemandResources"
		local errMsg = generateOdrFileStructure(appBundleFileUnquoted, odrOutputDir, odrData, options.bundleid)
		if errMsg then
			return "ERROR: error while copying On-Demand Resources: " .. tostring(errMsg)
		end

		-- step 2: generate necessary Plists
		errMsg = generateOdrPlists(appBundleFileUnquoted, odrOutputDir, odrData, options.bundleid)
		if errMsg then
			return "ERROR: error while generating On-Demand Resources medatada: "..errMsg
		end

		-- step 3: codesign ODRs
		errMsg = generateOdrCodesign(odrOutputDir, odrData, options.bundleid, options.signingIdentity, iPhoneSDKRoot )
		if errMsg then
			return "ERROR: codesignig On-Demand Resources: "..errMsg
		end
	end


	-- bundle swift libraries if required
	local bundleSwiftSupportDir
	local bundleSwiftSupportParentDir
	if options.usesSwift then
		setStatus("Adding Swift support for plugins")

		local bundleOptions = {
			exe=options.bundleexecutable,
			sdkBase=iPhoneSDKRoot,
			app=appBundleFileUnquoted,
			identity=options.signingIdentity,
			platform="iphoneos"
		}
		local sdkVersion = captureCommandOutput("xcrun --sdk iphoneos --show-sdk-version")
		if not sdkVersion then
			return "Unable to get iOS SDK version"
		end
		sdkVersion = tonumber(string.match(sdkVersion, '%d+%.?%d*'))
		if not sdkVersion then
			return "Unable to parse SDK version"
		end

		local bundleScript
		if sdkVersion >= 16.4 then
			bundleScript = '$(xcrun -f swift-stdlib-tool) --copy --verbose --scan-executable "{app}/{exe}" --scan-folder "{app}/Frameworks" --platform {platform} --destination "{app}/Frameworks" --strip-bitcode '
		else
			bundleScript = '$(xcrun -f swift-stdlib-tool) --copy --verbose --scan-executable "{app}/{exe}" --scan-folder "{app}/Frameworks" --platform {platform} --toolchain "{sdkBase}/Toolchains/XcodeDefault.xctoolchain" --destination "{app}/Frameworks" --strip-bitcode '
		end

		if not options.signingIdentity then
			bundleOptions.identity = "-"
			bundleOptions.platform = "iphonesimulator"
		end

		if builtForAppStore then
			-- generate SwiftSupport folder
			local suffix = "_SwiftSupport"
			local ssDir = captureCommandOutput("mktemp -d -t CLtmpXXXXXX"..suffix) -- creates a temporary directory
			-- be paranoid because we'll "rm -r" this later
			if not ssDir:sub(-string.len(suffix)) == suffix then
				return "Problem with temporary directory: "..tostring(ssDir)
			end
			bundleSwiftSupportParentDir = ssDir
			bundleSwiftSupportDir = ssDir.."/SwiftSupport"
			bundleOptions.SwiftSupport = bundleSwiftSupportDir
			bundleScript = bundleScript .. '--unsigned-destination "{SwiftSupport}" '
		else
			-- just embed swift
			bundleScript = bundleScript .. '--resource-destination "{app}" --resource-library libswiftRemoteMirror.dylib '
		end
		bundleScript = bundleScript .. ' && find "{app}/Frameworks" -iname "libSwift*.dylib" -exec $(xcrun -f lipo) {} -remove arm64e -output {} \\;'

		bundleScript = bundleScript:gsub("({([^}]+)})",
		function(whole,i) return bundleOptions[i] or whole end)

		local result, errMsg = runScript( bundleScript )
		if result ~= 0 then
			errMsg = "ERROR: bundling Swift libraries required by plugins failed: "..tostring(errMsg)
			return errMsg
		end
	end

	--remove standard resources(Corona Resources Bundle) if users selects
	if options.includeStandardResources == false then
		runScript("rm -rf "..quoteString(makepath(appBundleFileUnquoted, "CoronaResources.bundle")))
	end
	
	--add xcprivacy file to the bundle
	if options.settings.iphone and options.settings.iphone.xcprivacy then
		runScript("cp -v " .. quoteString(options.tmpDir .. "/PrivacyInfo.xcprivacy") .. " " .. quoteString(makepath(appBundleFileUnquoted, "PrivacyInfo.xcprivacy")))
	end
	

	-- bundle is now ready to be signed (don't sign if we don't have a signingIdentity, e.g. Xcode Simulator)
	if options.signingIdentity then
		-- codesign embedded frameworks before signing the .app
		local result, errMsg = runScript( getCodesignFrameworkScript( appBundleFileUnquoted, options.signingIdentity, iPhoneSDKRoot ) )
		if result ~= 0 then
			errMsg = "ERROR: code signing embedded frameworks failed: "..tostring(errMsg)
			return errMsg
		end

		local entitlements = quoteString( options.tmpDir .. "/entitlements.xcent" )
		setStatus("Signing application with "..tostring(options.signingIdentityName))

		-- codesign embedded appx (iOS Extension Files)
		if(checkAppHasAPPX(options.appBundleFile)) then
			
			local result, errMsg = runScript( getCodesignAPPXScriptAndPackage( options.appBundleFile:gsub('["\']', ""), options.signingIdentity, entitlements, iPhoneSDKRoot, options.bundleid, builtForAppStore ) )
			if result ~= 0 then
				errMsg = "ERROR: code signing embedded appx failed: "..tostring(errMsg)
				return errMsg
			end
		end
				

		local result, errMsg = runScript( getCodesignScript( entitlements, appBundleFileUnquoted, options.signingIdentity, iPhoneSDKRoot ) )

		if result ~= 0 then
			errMsg = "ERROR: code signing failed: "..tostring(errMsg)
			return errMsg
		end

		if debugBuildProcess > 1 then
			runScript( "cp -v " .. entitlements .. " /tmp/")
		end
	else
		setStatus("Signing application for Xcode Simulator")
		local result, errMsg = runScript( "/usr/bin/codesign --force --sign - --timestamp=none "..appBundleFile )

		if result ~= 0 then
			errMsg = "ERROR: code signing for Xcode Simulator failed: "..tostring(errMsg)
			return errMsg
		end
	end

	runScript( "chmod 755 " .. appBundleFile )

	-- If building with a distribution identity, create an IPA of the .app which can be used by Application Loader
	local appBundleFileIPA = quoteString(makepath(options.dstDir, options.dstFile) .. ".ipa")
	-- remove old IPA for extra cleanliness, even if we are not building a new IPA for distribution (it's stale so let's remove it)
	runScript( "rm -f " .. appBundleFileIPA )

	if builtForAppStore then
		setStatus("Creating application bundle IPA for distribution: ".. appBundleFileIPA)
		local quotedDestDir = quoteString( options.dstDir )
		local baseBundleName = quoteString( options.dstFile .. ".app" )
		local suffix = "_ipa"
		local ipaTmpDir = captureCommandOutput("mktemp -d -t CLtmpXXXXXX"..suffix) -- creates a temporary directory

		-- be paranoid because we'll "rm -r" this later
		if not ipaTmpDir:sub(-string.len(suffix)) == suffix then
			return "Problem with temporary directory: "..tostring(ipaTmpDir)
		end

		local mkTmpDirResult, errorMesg = lfs.mkdir(makepath(ipaTmpDir, "Payload"))

		if mkTmpDirResult then
			setStatus("Creating IPA for store submission...")
			-- note we move the app to the "Payload" directory to preserve permissions and for speed which means the .app doesn't exist anymore
			runScript( "mv " .. quoteString(makepath(options.dstDir, options.dstFile..".app")) .." ".. quoteString(makepath(ipaTmpDir, "Payload")) )

			--move odr resources to "Payload" folder
			if odrOutputDir then
				runScript( "mv " .. quoteString(makepath(odrOutputDir)) .." ".. quoteString(makepath(ipaTmpDir, "Payload")) )
			end

			if bundleSwiftSupportDir then
				runScript( "mv " .. bundleSwiftSupportDir .." ".. quoteString(ipaTmpDir) )
			end

			runScript( "cd " .. quoteString(ipaTmpDir) .. "; zip --symlinks -r " .. appBundleFileIPA .. " *" )

			runScript( "rm -r " .. quoteString(ipaTmpDir) )

			if bundleSwiftSupportParentDir then
				runScript( "rm -r " .. quoteString(bundleSwiftSupportParentDir) )
			end
		else
			return "ERROR: failed to create IPA temporary directory '"..ipaTmpDir .."': " ..errorMesg
		end
	end

	return nil
end

--------------------------------------------------------------------------------

local function getNames( name, dstFile, result )
	local err

	result.bundledisplayname = name
	result.bundlename = name
	result.bundleexecutable = dstFile

	return err
end

-- determines appid and bundleid from provision file
-- if wildcard is present, then requires bundlename to be supplied as input
local function getIDs( mobileProvision, result )
	local err

	-- This is really a logic issue but better to gracefully error
	if not mobileProvision or string.len(mobileProvision) == 0 then
		return "Error: missing provision file"
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
		"<key>application%-identifier</key>%s*<string>([%w%.%-]*)"
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
		value = appid

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

function getBundleId( mobileProvision, bundledisplayname, dstFile )
	local options = {}

	local err = getNames( bundledisplayname, dstFile, options )
	if ( not err ) then
		err = getIDs( mobileProvision, options )

		if ( not err ) then
			return options.bundleid
		end
	end

	return nil
end

function buildExe( options )
	local appBundleFileUnquoted = makepath(options.dstDir, options.dstFile) .. ".app"
	local dstDir = appBundleFileUnquoted

	local buildDir = dstDir .. '/.build'
	local libtemplateDir = buildDir .. '/libtemplate'

	local result = nil

	local path = libtemplateDir .. '/Builder.lua'
	local builderCtor = loadfile( path )

	-- Builder.lua only exists when we have plugins.
	-- Otherwise we assume there is an exe already available.
	if builderCtor then
		-- Temporarily modify Lua require path
		local pathOld = package.path or ""
		package.path = libtemplateDir .. '/?.lua;' .. pathOld

		-- Instantiate Builder
		local builder = builderCtor()

		-- Params to builder
		local dstName = options.bundleexecutable

		local buildOptions =
		{
			dstDir = dstDir,
			dstName = dstName,
			dstPath = dstDir .. '/' .. dstName,
			librarySearchPaths = { libtemplateDir, },
			pluginsDir = buildDir,
			verbose = options.verbose or debugBuildProcess,
			sdkType = options.sdkType,
			tmpDir = options.tmpDir,
			settings = options.settings
		}

		setStatus("Adding plugins")
		if debugBuildProcess > 1 then
			print("builder: buildOptions: "..json.prettify(buildOptions))
		end

		-- Perform build
		local buildCallSucceeded, buildResult = pcall( function() builder:build( buildOptions ) end )
		options.usesSwift = buildOptions.usesSwift

		if debugBuildProcess and debugBuildProcess > 1 then
			print("builder: buildCallSucceeded: "..tostring(buildCallSucceeded).."; buildResult: "..tostring(buildResult))
		end

		-- Did the Builder code assert()?
		if not buildCallSucceeded then
			-- extract the JSON from the XML response
			local buildError = json.decode(buildResult:gsub(".*<error>(.*)</error>", "%1"))
			if buildError and buildError.message then
				-- emit the error to the console in an easy to read format
				print("ERROR: Builder failed: " .. tostring(buildError.message))
			else
				-- we got a plain string back as the error message
				print("ERROR: Builder failed: " .. tostring(buildResult))
			end
		end

		-- We have to handle either an assert in the Builder code or a non-zero return code
		if not buildCallSucceeded or (buildResult ~= nil and buildResult ~= 0) then
			-- this gets prefixed with "BUILD ERROR: " in the console
			return "There was a problem linking the app.\n\nCheck the console for more information."
		end

		-- Move the helper files/plugin libs out of the .app bundle into the tmp dir
		buildResult = runScript( 'mv "' .. dstDir .. '/.build" "' .. options.tmpDir .. '"' )

		if buildResult ~= 0 then
			return "ERROR: build failed to move helper files/plugin libs"
		end

		-- Restore old Lua require path
		package.path = pathOld
	end

	return result
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

--
-- populateUIAppFonts
--
-- Populate UIAppFonts with font files found in the project
-- The table may already have entries because in the past we required the developer to populate the
-- list themselves so we just merge all the font files we find at the top level of the project into
-- the table.  Or there may be no build.settings at all and everything needs to be created.
function populateUIAppFonts( options )

	-- Everything in options.settings is optional
	if options.settings == nil then
		options.settings = {}
	end
	if options.settings.iphone == nil then
		options.settings.iphone = {}
	end
	if options.settings.iphone.plist == nil then
		options.settings.iphone.plist = {}
	end

	local tmpUIAppFonts = options.settings.iphone.plist.UIAppFonts

	if tmpUIAppFonts ~= nil then
		if debugBuildProcess and debugBuildProcess > 1 then
			print("populateUIAppFonts: BEFORE: tmpUIAppFonts: "..json.prettify(tmpUIAppFonts))
		end

		-- Check that each font already in UIAppFonts exists in the project
		for i, fontfile in ipairs(tmpUIAppFonts) do
			if lfs.attributes( options.srcAssets .."/".. fontfile ) == nil then
				print("Warning: font '"..tostring(fontfile).."' is in build.settings UIAppFonts but not in the project")
			end
		end
	end

	-- Find any fonts that are in the project but not in UIAppFonts and add them
	for filename in lfs.dir( options.srcAssets ) do
		if string.match(filename, "%.ttf$") or string.match(filename, "%.otf$") or string.match(filename, "%.ttc$") then
			if debugBuildProcess and debugBuildProcess > 0 then
				print( "populateUIAppFonts: Found font file: " .. filename )
			end

			-- If the font file is not already in UIAppFonts, add it
			if table.indexOf( tmpUIAppFonts, filename ) == nil then
				if tmpUIAppFonts == nil then
					tmpUIAppFonts = {}
				end

				table.insert( tmpUIAppFonts, filename )
			end
		end
	end

	if tmpUIAppFonts ~= nil and #tmpUIAppFonts > 0 then
		if debugBuildProcess and debugBuildProcess > 1 then
			print("populateUIAppFonts:  AFTER: #"..#tmpUIAppFonts..": tmpUIAppFonts: "..json.prettify(tmpUIAppFonts))
		end

		options.settings.iphone.plist.UIAppFonts = tmpUIAppFonts
	end
end


--
-- iPhonePostPackage
--
-- Process the app bundle return by the build server (including code signing, Info.plist processing, etc)
--
-- returns an error message or nil on success
--
function iPhonePostPackage( params )

	local srcAssets = params.srcAssets
	local tmpDir = params.tmpDir
	local dstDir = params.dstDir
	local dstFile = params.dstFile
	local bundledisplayname = params.bundledisplayname
	local bundleversion = params.bundleversion
	local provisionFile = params.provisionFile
	local signingIdentity = params.signingIdentity
	local sdkRoot = params.sdkRoot
	local targetDevice = params.targetDevice
	local targetPlatform = params.targetPlatform
	local liveBuild = params.liveBuild
	local includeStandardResources = params.includeStandardResources
	local verbose = ( debugBuildProcess and debugBuildProcess > 1 )
	local osPlatform = params.osPlatform
	local err = nil

	local iPhoneSDKRoot = sdkRoot or "/Applications/Xcode.app/Contents/Developer"
    -- Make available globally
    xcodetoolhelper = params.xcodetoolhelper

	-- init options
	local options = {
		dstDir=dstDir,
		dstFile=dstFile,
		srcAssets=srcAssets,
		bundleversion=bundleversion,
		signingIdentity=signingIdentity,
		tmpDir=tmpDir,
		sdkRoot=sdkRoot,
		targetDevice=targetDevice,
		targetPlatform=targetPlatform,
		verbose=verbose,
		osPlatform=osPlatform,
		sdkType=params.sdkType,
		liveBuild=liveBuild,
		includeStandardResources=includeStandardResources,
	}

	local customSettingsFile = srcAssets .. "/build.settings"
	if ( fileExists( customSettingsFile ) ) then
		local customSettings, msg = loadfile( customSettingsFile )
		if ( customSettings ) then
			local status, msg = pcall( customSettings )
			if status then
				print( "Using additional build settings from: " .. customSettingsFile )
				options.settings = settings
			else
				err = "Warning: Error found in build.settings file:\n\t".. msg
				print(err)
			end
			-- forwards compatibility
			if settings.ios ~= nil then
				if settings.iphone ~= nil then
					print( "WARNING: settings.ios overrides settings.iphone in "..customSettingsFile )
				end
				settings.iphone = settings.ios
			end
		else
			err = "Warning: Could not load build.settings file:\n\t".. msg
			print(err)
		end
	end

	if ( not err ) then
		local err = getNames( bundledisplayname, dstFile, options )
		if ( not err ) then

			-- Special case for no provision profile (e.g. Xcode simulator)
			if not provisionFile then
				options.bundleid = "com.example." .. sanitizeBundleString(bundledisplayname, false)
			else
				err = getIDs( provisionFile, options )
			end
		end
	end

	if ( not err ) then

		options.appBundleFile = quoteString(makepath(dstDir, options.dstFile) .. ".app" )

		runScript( "rm -rf ".. options.appBundleFile .. ".dSYM" )

		-- if we've changed build styles from regular to live or vice-versa, start with a clean slate
		if (not options.liveBuild and fileExists(makepath(options.appBundleFile, coronaLiveBuildAppDir))) or
		   (options.liveBuild and not fileExists(makepath(options.appBundleFile, coronaLiveBuildAppDir))) then
			runScript( "rm -rf "..  options.appBundleFile )
		end

		-- The server may have removed spaces and/or Unicode characters from the app name, fix it up
		--
		-- We used to look in output.zip to determine what the server might have done to the app name but
		-- as of macOS 10.12 that is no longer reliable due to unzip munging Unicode names
		-- local serverDirName = captureCommandOutput("unzip -Z -1 "..tmpDir.."/output.zip | head -1 | sed -e 's!/$!!'")
		-- local serverAppName = serverDirName:gsub(".app$", "")

		-- default to the name of the project
		local serverDirName = options.dstFile..".app"
		local serverAppName = options.dstFile

		-- the server removes spaces so we do too (this is reversed later on)
		serverDirName = serverDirName:gsub(' ', '')
		serverAppName = serverAppName:gsub(' ', '')

		if debugBuildProcess > 0 then
			print("Server dir name: "..tostring(serverDirName))
			print("Server app name: "..tostring(serverAppName) .. " (local name: "..tostring(options.dstFile)..")")
		end

		runScript("mkdir -p "..  quoteString(makepath(dstDir, serverDirName)))

		-- Copy app resources
		result = prePackageApp(makepath(dstDir, serverDirName), options)

		if result then
			return result
		end

		-- extract output.zip into dstDir
		if debugBuildProcess > 1 then
			print("Contents of ZIP file from server:")
			runScript( "unzip -l "..tmpDir.."/output.zip" )
			-- uncomment to save a copy of the server zip file
			-- runScript( "cp "..tmpDir.."/output.zip /tmp/" )
		end

		setStatus("Unpacking build with plugins")

		-- The file 'Default-568h@2x.png' is a special case: if there is one in the project,
		-- don't overwrite it with the one from the template
		local zipExtra = ""
		if fileExists(makepath(dstDir, serverDirName, "Default-568h@2x.png")) then
			zipExtra = " -x '"..serverDirName.."/Default-568h@2x.png'"
		end

		-- We use the '-V' option to unzip to stop it stripping semi-colons from the end of filenames
		local result, errMsg = runScript( "unzip -q -V -o "..tmpDir.."/output.zip -d "..quoteString( dstDir )..zipExtra )

		if result ~= 0 then
			return "ERROR: unzipping output.zip failed: "..tostring(errMsg)
		end

		-- Rename the "resource.car" file to "resource.corona-archive" because Apple chokes on random ".car" files
		runScript( "mv -f "..quoteString(makepath(dstDir, serverDirName, "resource.car")).." ".. quoteString(makepath(dstDir, serverDirName, "resource.corona-archive")) )

		-- If the server's app name differs from ours, use ours
		if serverAppName ~= options.dstFile then

			-- The ".build" directory will exist if this is a plugin build (which will create the executable later)
			if not fileExists(makepath(dstDir, serverDirName, ".build")) then
				-- Rename the executable in the bundle
				runScript( "mv -f "..quoteString(makepath(dstDir, serverDirName, serverAppName)).." ".. quoteString(makepath(dstDir, serverDirName, options.dstFile)))
			end

			-- Rename the bundle itself
			runScript( "rm -rf ".. options.appBundleFile ) -- remove any old bundle
			runScript( "mv -f "..quoteString(makepath(dstDir, serverDirName)).." ".. options.appBundleFile )
		end

		-- for plugins, build exe
		result = buildExe( options )

		if result then
			return result
		end

		-- compile Xcode assets for icon
		if options.settings and options.settings.iphone then
			setStatus("Compiling Xcode assets catalog")
			local xcassetPlatformOptions = {
				{ "target-device", "iphone" },
				{ "target-device", "ipad" },
				{ "minimum-deployment-target", "8.0" },
				{ "platform", options.signingIdentity and "iphoneos" or "iphonesimulator" },

				 {"app-icon", "AppIcon"},
			}
			err = CoronaPListSupport.compileXcassets(options, tmpDir, srcAssets, xcassetPlatformOptions, options.settings.iphone)
			if err then
				return err
			end
		end

		setStatus("Packaging app")
		-- We get the signingIdentity as a cert fingerprint but we also need the plaintext name
		if options.signingIdentity then
			options.signingIdentityName = captureCommandOutput("security find-identity -p codesigning | grep '".. options.signingIdentity .."' | sed -e 's/.*\\(\".*\"\\).*/\\1/'")
			if not options.signingIdentityName or options.signingIdentityName == "" then
				print("WARNING: identity '"..options.signingIdentity.."' does not appear to be valid for codesigning.  You should open Keychain Access and verify that your signing identities are valid")
			else
				print("Code signing identity: ".. options.signingIdentityName .." (".. options.signingIdentity ..")")
			end
		end


		-- inject live build settings
		if options.liveBuild then
			-- 1. set options.settings.iph one.plist.NSAppTransportSecurity.NSAllowsArbitraryLoads
			if not options.settings then options.settings = {} end
			if not options.settings.iphone then options.settings.iphone = {} end
			if not options.settings.iphone.plist then options.settings.iphone.plist = {} end
			if not options.settings.iphone.plist.NSAppTransportSecurity then options.settings.iphone.plist.NSAppTransportSecurity = {} end
			options.settings.iphone.plist.NSAppTransportSecurity.NSAllowsArbitraryLoads = true;
		end


		populateUIAppFonts( options )

		if options and debugBuildProcess and debugBuildProcess ~= 0 then
			print("====================================")
			print("iPhonePostPackage: options: "..json.prettify(options))
			print("====================================")
		end

		-- finalize package
		result = generateFiles( options )

		if result then
			return result
		end


		result = packageApp( options )
		
		if result then
			return result
		end

	end

	return err
end

--
-- IOSSendToAppStore
--
-- Send an app to the App Store via iTunes Connect
--
-- returns an error message or nil on success
--
function IOSSendToAppStore( params )

	local tmpDir = params.tmpDir
	local dstDir = params.dstDir
	local dstFile = params.dstFile
	local bundledisplayname = params.bundledisplayname
	local itunesConnectUsername = params.itc1
	local itunesConnectPassword = params.itc2
	local sdkRoot = params.xcodetoolhelper.sdkRoot
	local codesign = params.xcodetoolhelper.codesign
	local applicationLoader = params.xcodetoolhelper.applicationLoader
	local productbuild = params.xcodetoolhelper.productbuild
	local err = nil
	local appSigningIdentityName
	local installerSigningIdentityName

	print("Using Xcode utilities from "..sdkRoot)

	local appArchiveFile = makepath(dstDir, dstFile) .. ".ipa"

	-- logically at this point the IPA must exist
	if lfs.attributes( appArchiveFile ) == nil then
		return "ERROR: cannot locate IPA file at '"..appArchiveFile.."'"
	end

	--
	-- Validate package with iTunes Connect
	--
	setStatus("Validating application with iTunes Connect")
	local result = captureCommandOutput( getProductValidateScript( appArchiveFile, itunesConnectUsername, itunesConnectPassword, applicationLoader ), 2 )

	if result ~= "" then
		if result:match('Unable to validate archive') then
			-- Validation requires an internet connection but the error message is somewhat inscrutable
			result = result .. "\n\n(make sure you are connected to the internet)"
		end

		errMsg = "ERROR: validation failed: "..tostring(result:gsub('\n', '\n\n'))
		return errMsg
	end

	--
	-- Upload package to iTunes Connect
	--
	setStatus("Uploading application to iTunes Connect")
	local result = captureCommandOutput( getProductUploadScript( appArchiveFile, itunesConnectUsername, itunesConnectPassword, applicationLoader ), 2 )

	if result ~= "" then
		errMsg = "ERROR: upload failed: "..tostring(result)
		return errMsg
	end

	return nil
end
