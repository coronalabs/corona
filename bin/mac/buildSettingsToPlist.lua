------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

---- Amend an app's Info.plist with CoronaSDK specific items.

-- add $pwd/../../shared/resource to lua module lookup path
package.path = package.path .. ";" .. arg[0]:match("(.+)/") .. "/../../shared/resource/?.lua"

local json = require "json"

local srcAssets = arg[1]
local appBundleFile = arg[2]
local deviceType = arg[3] or "iOS"

local function osExecute(...)
	print("osExecute: ".. ...)
	return os.execute(...)
end

-- Double quote a string escaping backslashes and any double quotes
local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

-- defaults
local targetDevice = nil -- default is in CoronaPlistSupport

-- Get the current build id from the environment (if set)
local corona_build_id = os.getenv("CORONA_BUILD_ID")

-- init options
local options = {
	appBundleFile = quoteString( appBundleFile ),
	dstDir = dstDir,
	bundleversion = bundleversion,
	signingIdentity = signingIdentity,
	sdkRoot = sdkRoot,
	targetDevice = targetDevice,
	targetPlatform = deviceType,
	verbose = verbose,
	corona_build_id = corona_build_id,
}

local function fileExists( filename )
	local f = io.open( filename, "r" )
	if ( f ) then
		io.close( f )
	end
	return ( nil ~= f )
end

local customSettingsFile = srcAssets .. "/build.settings"
if ( fileExists( customSettingsFile ) ) then
	local customSettings, msg = loadfile( customSettingsFile )
	if ( customSettings ) then
		local status, msg = pcall( customSettings )
		if status then
			print( "Using additional build settings from: " .. customSettingsFile )
			options.settings = _G.settings
		else
			print( "Error: Errors found in build.settings file:" )
			print( "\t".. msg ) 
			os.exit(1)
		end
	else
		print( "Error: Could not load build.settings file:" )
		print( "\t".. msg )
		os.exit(1)
	end
end

-- define modifyPlist()
local CoronaPListSupport = require("CoronaPListSupport")

CoronaPListSupport.modifyPlist( options )

--[[
-- build.settings (if loaded and successfully executed, creates a global settings table)
local settings = _G.settings
if settings then
	-- cross-platform settings
	local orientation = settings.orientation
	if orientation then
		local defaultOrientation = orientation.default
		local supported = {}
		if defaultOrientation then
			local key = "UIInterfaceOrientation"
			local value = "UIInterfaceOrientationPortrait"
			if "landscape" == defaultOrientation or "landscapeRight" == defaultOrientation then
				value = "UIInterfaceOrientationLandscapeRight"
			elseif "landscapeLeft" == defaultOrientation then
				value = "UIInterfaceOrientationLandscapeLeft"
			end

			table.insert( supported, value )
			osExecute( "defaults write '"..appBundleFile.."/Info' " .. key .. " "..value )
		end

		osExecute( "defaults delete '"..appBundleFile.."/Info' ContentOrientation" )
		local contentOrientation = orientation.content
		if contentOrientation then
			local value
			if "landscape" == contentOrientation or "landscapeRight" == contentOrientation then
				value = "UIInterfaceOrientationLandscapeRight"
			elseif "landscapeLeft" == contentOrientation then
				value = "UIInterfaceOrientationLandscapeLeft"
			elseif "portrait" == contentOrientation then
				value = "UIInterfaceOrientationPortrait"
			end

			if value then
				osExecute( "defaults write '"..appBundleFile.."/Info' ContentOrientation "..value )
			end
		end

		osExecute( "defaults delete '"..appBundleFile.."/Info' CoronaViewSupportedInterfaceOrientations" )
		local supportedOrientations = orientation.supported
		if supportedOrientations then
			local toUIInterfaceOrientations =
			{
				landscape = "UIInterfaceOrientationLandscapeRight",
				landscapeRight = "UIInterfaceOrientationLandscapeRight",
				landscapeLeft = "UIInterfaceOrientationLandscapeLeft",
				portrait = "UIInterfaceOrientationPortrait",
				portraitUpsideDown = "UIInterfaceOrientationPortraitUpsideDown",
			}

			for _,v in ipairs( supportedOrientations ) do
				local value = toUIInterfaceOrientations[v]
				if value then
					-- Add only unique values
					local found
					for _,elem in ipairs( supported ) do
						if elem == value then
							found = true
							break
						end
					end

					if not found then
						table.insert( supported, value )
					end
				end
			end
		end
		-- insert escape quotes between each element
		local supportedValue = table.concat( supported, "\" \"" )

		-- escape supportedValue on both ends
		osExecute( "defaults write '"..appBundleFile.."/Info' CoronaViewSupportedInterfaceOrientations -array ".. quoteString( supportedValue ) )
	end


	-- add'l custom plist settings specific to iPhone
	-- defaults write is inadequate to write nested arrays and dictionaries
	local buildSettingsPlist = nil
	if deviceType == "iphone" then
		buildSettingsPlist = settings.iphone and settings.iphone.plist
	elseif deviceType == "mac" then
		buildSettingsPlist = settings.mac and settings.mac.plist
	end

	if buildSettingsPlist then
		print("Adding custom plist settings: ".. json.encode(buildSettingsPlist))

		local infoPlistFile = appBundleFile ..  "/Info.plist"
		local tmpJSONFile = os.tmpname() .. ".json"
		
		-- Convert the Info.plist to JSON and read it in
		osExecute( "plutil -convert json -o '"..tmpJSONFile.."' '"..infoPlistFile.."'")

		local jsonFP = io.open(tmpJSONFile, "r")
		if jsonFP ~= nil then

			local jsonDataStr = jsonFP:read("*a")

			jsonFP:close()

			infoPlist, errorMsg = json.decode( jsonDataStr );

			if infoPlist == nil then
				print("Failed to load "..infoPlistFile..": "..errorMsg)

				os.exit(1)
			end
		end

		-- infoPlist now contains a Lua table representing the Info.plist

		print("================================")
		print("buildSettingsPlist: "..json.encode(buildSettingsPlist))
		print("infoPlist: "..json.encode(infoPlist))
		print("================================")

		for k, v in pairs(buildSettingsPlist) do
			print("Extra plist setting '"..k.."': "..tostring(v))
			infoPlist[k] = v
		end

		print("infoPlist: "..json.encode(infoPlist, {indent = true}))

		local outFP, errorString = io.open( tmpJSONFile, "w" )
		if outFP ~= nil then
			outFP:write( json.encode(infoPlist, {indent = true}) )
			outFP:close()

			-- Convert the JSON plist into an XML plist
			osExecute("plutil -convert xml1 -o '"..infoPlistFile.."' '"..tmpJSONFile.."'")
			osExecute("cat '"..infoPlistFile.."'")
		end
	end
end
--]]
