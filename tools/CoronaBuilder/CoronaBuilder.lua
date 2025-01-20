------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

local json = require('json')
local builder = require('builder')

-- These helper functions are called by CoronaBuilder/Rtt_AppPackagerFactory.cpp to determine the
-- correct target SDK version for iOS and tvOS builds
function CoronaBuilderDetermineTargetiOSVersion( params, bundleDir, buildNum )
	local currentSDKsFile = bundleDir .. "/iOS-SDKs.json"
	return CoronaBuilderDetermineTargetSDKVersion( "iphoneos", "ios", currentSDKsFile, params, buildNum )
end

-- Determine we're running in a debug build
function isDebugBuild()
	local debugInfo = debug.getinfo(CoronaBuilderDetermineTargetiOSVersion)
	-- print("debugInfo: ", json.prettify(debugInfo))
	return (debugInfo ~= nil and debugInfo.short_src ~= "?")
end

function debug_print(...)
	if isDebugBuild() then
		print(...)
	end
end

function CoronaBuilderDetermineTargettvOSVersion( params, bundleDir, buildNum )
	local currentSDKsFile = bundleDir .. "/tvOS-SDKs.json"
	return CoronaBuilderDetermineTargetSDKVersion( "appletvos", "tvos", currentSDKsFile, params, buildNum )
end

function CoronaBuilderDetermineTargetSDKVersion( sdkname, platformName, currentSDKsFile, params, buildNum )

	local CoronaPListSupport = require("CoronaPListSupport")
	local captureCommandOutput = CoronaPListSupport.captureCommandOutput

	if not params then
		return false, "params cannot be nil in determineTargetSDKVersion"
	end
	if type(params) ~= "table" then
		return false, "params must be a table in determineTargetSDKVersion (not a "..type(params)..")"
	end

	-- Find currently active version of Xcode
	local xcodeSDKVersion = captureCommandOutput("/usr/bin/xcrun --sdk '"..sdkname.."' --show-sdk-version")
	print("Active "..sdkname.." SDK version: ", xcodeSDKVersion)

	local targetVersion = params['targetPlatformVersion']
	local SDKs, lineno, errorMsg = json.decodeFile(currentSDKsFile) 

	if errorMsg then
		return false, errorMsg
	end

	local coronaVersion = false
	local failMessage = "cannot find a compatible CoronaSDK "..buildNum.." build target for "..sdkname.." SDK "..xcodeSDKVersion
	for idx, sdkParams in ipairs(SDKs[platformName]) do
		-- print(idx, json.prettify(sdkParams))
		if sdkParams['coronaVersion'] == params['platformVersion'] then
			failMessage = sdkParams['failMessage']
			if params["forceVersion"] then
				coronaVersion = sdkParams['coronaVersion']
			end
		end
		if sdkParams['xcodeVersion'] == xcodeSDKVersion then
			coronaVersion = sdkParams['coronaVersion']
		end
	end

	if coronaVersion then
		if not params['platformVersion'] then
			params['platformVersion'] = coronaVersion

			return true, "'platformVersion' defaulted to "..coronaVersion
		elseif params['platformVersion'] == coronaVersion then
			return true, "requested 'platformVersion' of "..coronaVersion.." available"
		else
			return false, "can't build with requested 'platformVersion' of "..params['platformVersion'].." ("..failMessage..")"
		end
	else
		return false, failMessage
	end

end

-- Called by CoronaBuilder/Rtt_BuildParams.cpp to load a JSON parameter file
function CoronaBuilderLoadJSONParams(path)

	debug_print("CoronaBuilderLoadJSONParams: "..tostring(path))
	return json.decodeFile( path )

end


--[[ local testJSONData = 
{
    "data": [
        {
            "bundles": null,
            "expires": "Never",
            "granted": 1481436000,
            "license_by_bundle": 0,
            "plugin_developer": "com.aaronsserver",
            "plugin_name": "plugin.awcolor",
            "status": 2,
            "title": "awcolor"
        },
        {
            "bundles": {
                "ios": "com.head-net.testgame",
                "android": "com.head-net.android.testgame"
            },
            "expires": "Never",
            "granted": 1503706792,
            "license_by_bundle": 1,
            "plugin_developer": "moti",
            "plugin_name": "test plugin-ignore",
            "status": 2,
            "title": "test plugin-ignore"
        },
        {
            "bundles": null,
            "expires": "Never",
            "granted": 1503708007,
            "license_by_bundle": 1,
            "plugin_developer": "prateek",
            "plugin_name": "testing",
            "status": 2,
            "title": "dfdsfds"
        },
		{
            "bundles": {
                "ios": "com.mycompany.MyApp",
                "android": "com.mycompany.myapp",
            },
            "license_by_bundle": 1,
            "expires": 1529650800,
            "granted": 1498121251,
            "plugin_developer": "com.coronalabs",
            "plugin_name": "plugin.CoronaSplashControl",
            "status": 2,
            "title": "Splash Screen Control"
        },
		{
            "bundles": {
                "android": "com.mycompany.customerrorhandler"
            },
            "license_by_bundle": 1,
            "expires": 1529650800,
            "granted": 1498121251,
            "plugin_developer": "com.coronalabs",
            "plugin_name": "plugin.CoronaSplashControl",
            "status": 2,
            "title": "Splash Screen Control"
        }
    ],
    "status": "success"
}
]]

-- Called by CoronaBuilder to determine if the current user has a splash control plugin license
function CoronaBuilderDoesUserOwnSplashScreenControlPlugin(userID, platformName, bundleID)

	debug_print("CoronaBuilderDoesUserOwnSplashScreenControlPlugin: ", userID, platformName, bundleID)

	-- if platformName and bundleID aren't specified try to get them from the environment
	-- (assumes we are started from Xcode; Android builds must specify them on the command line)
	if platformName == nil and os.getenv("PRODUCT_BUNDLE_IDENTIFIER") ~= nil then
		platformName = "ios"

		bundleID = os.getenv("PRODUCT_BUNDLE_IDENTIFIER")

		debug_print("CoronaBuilderDoesUserOwnSplashScreenControlPlugin: using defaults: ", userID, platformName, bundleID)
	end

	if platformName == nil then
		platformName = ""
	end
	if bundleID == nil then
		bundleID = ""
	end

	local apiURL = "https://backendapi.coronalabs.com/v1/plugins/show/";
	local userID = tostring(userID)
	local url = tostring(apiURL..userID)

	debug_print("CoronaBuilderFetchUsersPlugins: "..url)

	local jsonData, errorMsg

	if testJSONData ~= nil then
		jsonData = testJSONData
		errorMsg = nil
	else
		jsonData, errorMsg = builder.fetch(url)
	end

	if errorMsg ~= nil then
		if isDebugBuild() then
			errorMsg = errorMsg .."\nURL: "..tostring(url)
		end

		return false, errorMsg
	end

	debug_print("CoronaBuilderFetchUsersPlugins: JSON: "..tostring(jsonData))
	local plugins, pos, errorMsg = json.decode( jsonData )

	if errorMsg ~= nil then
		if isDebugBuild() then
			errorMsg = errorMsg .."\nInvalid JSON: "..tostring(jsonData)
		end
		return false, errorMsg
	end

	if plugins.data == nil then

		local mesg = "Cannot load plugins for user '"..userID.."'"
		if isDebugBuild() then
			mesg = mesg .."\nURL: "..tostring(url).."\nJSON: "..tostring(jsonData)
		end

		return false, mesg

	end

	local licensedBundleIDs = ""
	local expiryMesg = ""
	local splashControlFound = false

	-- bundleIDs are considered to be case insensitive
	bundleID = bundleID:lower()

	-- Iterate over the plugins array looking for "plugin.CoronaSplashControl"
	for p = 1, #plugins.data do
		debug_print("Checking plugin_name: ", plugins.data[p].plugin_name)
		if plugins.data[p].plugin_name == "plugin.CoronaSplashControl" then
			if tostring(plugins.data[p].expires) == "Never" or os.time() < tonumber(plugins.data[p].expires) then

				splashControlFound = true -- we found at least one instance of a license

				if plugins.data[p].license_by_bundle then
					if plugins.data[p].bundles ~= nil and
						   ((plugins.data[p].bundles[platformName] ~= nil and
						     plugins.data[p].bundles[platformName]:lower() == bundleID) or
						    (plugins.data[p].bundles['all'] ~= nil and
						     plugins.data[p].bundles['all']:lower() == bundleID)) then
						debug_print(">>> Found bundle specific plugin_name for "..platformName.."/"..bundleID..": ", plugins.data[p].plugin_name)
						return true, nil
					else
						-- this is used to make the "not found" error message better below
						if plugins.data[p].bundles ~= nil then
							licensedBundleIDs = licensedBundleIDs .. json.prettify(plugins.data[p].bundles):gsub("[\n{}]", "") .. ", "
						end
					end
				else
					debug_print(">>> Found plugin_name: ", plugins.data[p].plugin_name)
					return true, nil
				end

			else

				expiryMesg = expiryMesg .. " - plugin license expired "..os.date("%Y-%m-%d %H:%M", plugins.data[p].expires) .."\n"

			end
		end
	end

	debug_print("*** Failed to find splash control license")

	local mesg
	if splashControlFound then
		-- we found bundle id specific licenses but not one for the current bundle id
		local bundleIDMesg = (bundleID ~= "" and platformName ~= "") and " and bundle id '"..bundleID.."' on "..platformName.." (current licenses: "..licensedBundleIDs..")" or " (perhaps you need to specify a bundle ID)"
		mesg = "Splash Screen Control plugin not licensed for user '"..userID.."'"..bundleIDMesg
	else
		-- we either didn't find a license, or it's expired
		mesg = "Splash Screen Control plugin not found for user '"..userID.."'\n"
		mesg = mesg .. expiryMesg
		mesg = mesg .. " - if you want to customize the splash screen you can get it here: https://marketplace.coronalabs.com/plugin/splash-screen-control"
	end

	if isDebugBuild() then
		mesg = mesg .."\nJSON: "..tostring(jsonData)
	end

	return false, mesg -- didn't find plugin or user does not have a valid bundle specific license
end

function CoronaBuilderDownloadFile(url, filename)

	debug_print("CoronaBuilderDownloadFile: ", tostring(url), tostring(filename))

	local headers = { } -- { ["Test-Header"] = "first value", second = "second value" }

	local result, errorMesg =  builder.download(url, filename, headers)

	return result, errorMesg
end
