----------------------------------------------------------------------------------------------------
-- Creates an "AndroidManifest.xml" using build settings coming from the following files:
-- * A required "build.properties" containing build dialog settings and "build.settings" info.
-- * An optional "output.json" which contains plugin settings.
-- Can also create a "strings.xml" file if the build settings contain a strings table
----------------------------------------------------------------------------------------------------

if not printError then
	printError = print
end

local json = require("json")


-- Fetch arguments.
local manifestTemplateFilePath = arg[1]
local buildPropertiesFilePath = arg[2]
local appName = arg[3]
local newManifestFilePath = arg[4]
local pluginSettingsFilePath = arg[5]
local stringsTemplateFilePath = arg[6]
local newStringsFilePath = arg[7]
local newApkFilesFilePath = arg[8]

-- Do not continue if missing required arguments.
if not manifestTemplateFilePath or not buildPropertiesFilePath or not newManifestFilePath then
	printError( "USAGE: " .. arg[0] .. " src_manifest build.properties new_manifest" )
	os.exit( -1 )
end

-- Load the "build.properties" file.
local buildPropertiesFileHandle = io.open( buildPropertiesFilePath, "r" )
if not buildPropertiesFileHandle then
	printError( "ERROR: The properties file does not exist: ", buildPropertiesFilePath )
	os.exit( -1 )
end
local buildProperties = json.decode(buildPropertiesFileHandle:read("*a"))
buildPropertiesFileHandle:close()

-- Load an array of plugin settings from the given JSON file, if provided.
local pluginSettingsCollection = nil
if pluginSettingsFilePath and pluginSettingsFilePath ~= "" then
	local pluginSettingsFileHandle = io.open(pluginSettingsFilePath, "r")
	if pluginSettingsFileHandle then
		pluginSettingsCollection = json.decode(pluginSettingsFileHandle:read("*a"))
		pluginSettingsFileHandle:close()
	else
		print( "WARNING: Failed to find plugin settings file: ", pluginSettingsFilePath )
	end
end


----------------------------------------------------------------------------------------------------
-- Initialize Android manifest settings.
----------------------------------------------------------------------------------------------------

-- Corona's min and target SDK defaults
local Constants =
{
	MIN_SDK_VERSION = 15,
	TARGET_SDK_VERSION = 23,
}

local minSdkVersion = tostring( Constants.MIN_SDK_VERSION )

local packageName = "com.corona.app"
local defaultOrientation = nil
local supportsOrientationChange = false
local supportsOrientationPortrait = false
local supportsOrientationPortraitUpsideDown = false
local supportsOrientationLandscapeRight = false
local supportsOrientationLandscapeLeft = false
local hasOrientationTable = false
local permissions = {}
local usesPermissions = {}
local supportsScreens = {}
local coronaActivityFlags = {}
local usesFeatures =
{
	{ glEsVersion = "0x00020000" },
	{ name = "android.hardware.telephony", required = false },
}
local mainIntentFilterCategories =
{
	["android.intent.category.LAUNCHER"] = true,
}
local intentFilters = {}
local usesExpansionFile = false
local largeHeap = false
local isGame = false
local installLocation = "auto"
local targetedAppStore = "none"
local manifestChildXmlElements = {}
local applicationChildXmlElements = {}
local googlePlayGamesAppId = false
local facebookAppId = false
local coronaWindowMovesWhenKeyboardAppears = false
local initialSystemUiVisibility = nil
local allowAppsReadOnlyAccessToFiles = true
local strings = {}
local apkFiles = { "...NONE..." } -- necessary due to the way ant treats empty filelists


----------------------------------------------------------------------------------------------------
-- Define functions used to load manifest settings.
----------------------------------------------------------------------------------------------------

-- Fetches the version code from the given argument and copies it to variable "versionCode".
local function fetchVersionCodeFrom(source)
	local numericValue = tonumber(source)
	if "number" == type( numericValue ) then
		versionCode = tostring( math.floor( numericValue ) )
	end
end

-- Fetches a version name from the given argument and copies it to variable "versionName".
local function fetchVersionNameFrom(source)
	if ("string" == type( source )) and (string.len( source ) > 0) then
		versionName = source
	elseif "number" == type( source ) then
		versionName = tostring(source)
	end
end

-- Fetches a min SDK version from the given argument and copies it to variable "minSdkVersion"
local function fetchMinSdkVersionFrom(source)
	if source ~= nil then
		-- A min SDK version was provided
		local numericValue = tonumber(source)
		if "number" == type( numericValue ) then
			-- Ensure the specified minSdkVersion fits our requirements
			if numericValue < Constants.MIN_SDK_VERSION then
				numericValue = Constants.MIN_SDK_VERSION
			elseif numericValue > Constants.TARGET_SDK_VERSION then
				numericValue = Constants.TARGET_SDK_VERSION
			end
			minSdkVersion = tostring( math.floor( numericValue ) )
		end
	end
end

-- Fetches permissions from argument "sourceArray" and inserts them into the "permissions" table.
local function fetchPermissionsFrom(sourceArray)
	if "table" == type(sourceArray) then
		for sourceIndex = 1, #sourceArray do
			-- Get the next permission, but only accept it if it is a table and it contains a "name" key.
			local sourceEntry = sourceArray[sourceIndex]
			if (("table" == type(sourceEntry)) and ("string" == type(sourceEntry["name"]))) then
				-- If the permission name starts with a period, then prefix the package name to it.
				if string.sub(sourceEntry["name"], 1, 1) == "." then
					sourceEntry["name"] = packageName .. sourceEntry["name"]
				end

				-- Check if the permissions array already contains the indexed element.
				local targetEntry = nil
				for targetIndex = 1, #permissions do
					targetEntry = permissions[targetIndex]
					if sourceEntry["name"] == targetEntry["name"] then
						break
					end
					targetEntry = nil
				end

				-- If an existing entry was not found, then insert a new one.
				if not targetEntry then
					targetEntry = {}
					table.insert(permissions, targetEntry)
				end

				-- Copy the permission settings.
				for key, value in pairs(sourceEntry) do
					targetEntry[key] = value
				end
			end
		end
	end
end

-- Fetches the entries in argument "sourceArray" and inserts them into the "usesPermissions" table.
local function fetchUsesPermissionsFrom(sourceArray)
	if "table" == type(sourceArray) then
		for index = 1, #sourceArray do
			local permissionName = sourceArray[index]
			if ("string" == type(permissionName)) then
				print("permissionName: ", permissionName)
				-- If the permission name starts with a period, then prefix the package name to it.
				if string.sub(permissionName, 1, 1) == "." then
					permissionName = packageName .. permissionName
				end

				-- Add the permission name to the given table.
				usesPermissions[permissionName] = true
			elseif "table" == type(permissionName) then
				local permission = permissionName
				permissionName = permission.name
				if ("string" == type(permissionName)) then
					print("permissionName (table): ", permissionName)
					-- If the permission name starts with a period, then prefix the package name to it.
					if string.sub(permissionName, 1, 1) == "." then
						permissionName = packageName .. permissionName
						permission.name = permissionName
					end
					local t = type(usesPermissions[permissionName])
					if 'nil' == t then -- new entry
						usesPermissions[permissionName] = permission
					elseif 'boolean' == t then
						print("Skipping permission table for " .. permissionName .. " because we already have 'true' rule for it")
						-- skip, because we have "always" permission
					elseif 'table' == t then
						local existing = usesPermissions[permissionName]
						if (permission.maxSdkVersion or 999) > (existing.maxSdkVersion or 999) then
							print("WARNING: using permission: " .. json.encode(permission) .. ', ignoring: ' .. json.encode(existing))
							usesPermissions[permissionName] = permission
						else
							print("WARNING: using permission: " .. json.encode(existing) .. ', ignoring: ' .. json.encode(permission))
						end
					end
				end
			end
		end
	end
end

-- Fetches the "uses-features" settings from argument "sourceArray" and inserts them into the "usesFeatures" table.
local function fetchUsesFeaturesFrom(sourceArray)
	if "table" == type(sourceArray) then
		for sourceIndex = 1, #sourceArray do
			-- Get the next feature, but only accept it if it is a table and it contains valid keys and value types.
			-- It is okay if the table is missing the "required" key, because Android allows this attribute to be missing.
			local sourceEntry = sourceArray[sourceIndex]
			if (("table" == type(sourceEntry)) and
			    ("string" == type(sourceEntry["name"])) and
			    ((not sourceEntry["required"]) or ("boolean" == type(sourceEntry["required"])))) then

				-- Check if feature array already contains the "build.settings" element.
				local targetEntry = nil
				for targetIndex = 1, #usesFeatures do
					targetEntry = usesFeatures[targetIndex]
					if sourceEntry["name"] == targetEntry["name"] then
						break
					end
					targetEntry = nil
				end
				if targetEntry then
					-- Update the existing array entry, but only if it is setting "required" to true.
					-- Never change a "required=true" setting to "false".
					if ("boolean" == type(sourceEntry["required"])) then
						targetEntry["required"] = targetEntry["required"] or sourceEntry["required"]
					end
				else
					-- Append new feature to the array.
					-- Do not do a direct table copy. Copy only the keys that we support.
					-- For example, ignore the "glEsVersion" since only Corona is allowed to set the min OpenGL requirement.
					-- Also, it is okay for the "required" attribute to be omitted.
					local newEntry = {name = sourceEntry["name"]}
					if ("boolean" == type(sourceEntry["required"])) then
						newEntry["required"] = sourceEntry["required"]
					end
					table.insert(usesFeatures, newEntry)
				end
			end
		end
	end
end

-- Fetches the entries in argument "sourceArray" and inserts them into the "mainIntentFilterCategories" table.
local function fetchMainIntentFilterCategoriesFrom(sourceArray)
	if "table" == type(sourceArray) then
		for index = 1, #sourceArray do
			local categoryName = sourceArray[index]
			if ("string" == type(categoryName)) and (string.len(categoryName) > 0) then
				if string.sub(categoryName, 1, 1) == "." then
					categoryName = packageName .. categoryName
				end
				mainIntentFilterCategories[categoryName] = true
			end
		end
	end
end

-- Fetches the tables from argument "sourceArray" and inserts them into the "intentFilters" table.
local function fetchIntentFiltersFrom(sourceArray)
	if "table" == type(sourceArray) then
		for key, value in pairs(sourceArray) do
			if "table" == type(value) then
				table.insert(intentFilters, value)
			end
		end
	end
end

-- Fetches the entries in argument "source" and inserts them them into the "manifestChildXmlElements" table.
local function fetchManifestChildXmlElementsFrom(source)
	-- Fetch the manifest XML tags to be inserted into the AndroidManifest.xml file.
	if "table" == type(source) then
		for index = 1, #source do
			local nextElement = source[index]
			if ("string" == type(nextElement)) and (string.len(nextElement) > 0) then
				manifestChildXmlElements[nextElement] = true
			end
		end
	elseif "string" == type(source) then
		fetchManifestChildXmlElementsFrom({ source })
	end
end

-- Fetches the entries in argument "source" and inserts them them into the "applicationChildXmlElements" table.
local function fetchApplicationChildXmlElementsFrom(source)
	-- Fetch the application XML tags to be inserted into the AndroidManifest.xml file.
	if "table" == type(source) then
		for index = 1, #source do
			local nextElement = source[index]
			if ("string" == type(nextElement)) and (string.len(nextElement) > 0) then
				applicationChildXmlElements[nextElement] = true
			end
		end
	elseif "string" == type(source) then
		fetchApplicationChildXmlElementsFrom({ source })
	end
end

-- Fetches the strings entry in argument "source" and inserts them them into the "strings" table.
local function fetchStringsFrom(source)
	-- Fetch the strings to be inserted into the strings.xml file.
	if "table" == type(source) then
		for i, v in pairs(source) do
			strings[i] = v
		end
	end
end

-- Fetches the apkFiles entry in argument "source"
local function fetchApkFilesFrom(source)
	-- Fetch the filenames to be inserted into the APK
	if "table" == type(source) then
		apkFiles = source
	else
		apkFiles = { "...NONE..." }  -- necessary due to the way ant treats empty filelists
	end
end

-- Creates an intent filter "data" XML tag from the given argument.
-- Argument "source" is expected to be a table of the data tag's attributes.
-- Returns the XML tag as a string. Returns nil if given an invalid argument.
local function createIntentFilterDataTagFrom(source)
	-- Create the tag's attributes from the given argument's table entries.
	local attributesString = ""
	if "table" == type(source) then
		for key, value in pairs(source) do
			if ("string" == type(key)) and (string.len(key) > 0) and ("string" == type(value)) then
				attributesString = attributesString .. 'android:' .. key .. '="' .. value .. '" '
			end
		end
	end

	-- Create and return the XML tag string.
	local tagString = nil
	if string.len(attributesString) > 0 then
		tagString = "<data " .. attributesString .. "/>"
	end
	return tagString
end


----------------------------------------------------------------------------------------------------
-- Fetch "build.properties" information.
----------------------------------------------------------------------------------------------------

strings["app_name"] = appName
if buildProperties then
	-- Fetch the package name.
	if "string" == type(buildProperties.packageName) then
		packageName = buildProperties.packageName
	end

	-- Fetch version code and version string.
	fetchVersionCodeFrom(buildProperties.versionCode)
	fetchVersionNameFrom(buildProperties.versionName)

	-- Fetch the targeted app store.
	if ("string" == type(buildProperties.targetedAppStore)) and (string.len(buildProperties.targetedAppStore) > 0) then
		targetedAppStore = buildProperties.targetedAppStore
	end

	if "string" == type(buildProperties.appName) then
		strings["app_name"] = buildProperties.appName
	end
end


----------------------------------------------------------------------------------------------------
-- Fetch "build.settings" information.
----------------------------------------------------------------------------------------------------

local buildSettings = nil
if buildProperties then
	buildSettings = buildProperties.buildSettings
end
if "table" == type(buildSettings) then
	-- Fetch the old "androidPermissions" table first for backward compatibility.
	-- Permissions set within the "android" table will be read later.
	fetchUsesPermissionsFrom(buildSettings.androidPermissions)

	-- Fetch orientation settings.
	if "table" == type(buildSettings.orientation) then
		hasOrientationTable = true

		-- Fetch the default orientation.
		local stringValue = buildSettings.orientation.default
		if ("string" == type(stringValue)) then
			if ("portrait" == stringValue) then
				supportsOrientationPortrait = true
				defaultOrientation = "portrait"
			elseif ("portraitUpsideDown" == stringValue) then
				supportsOrientationPortraitUpsideDown = true
				defaultOrientation = "reversePortrait"
			elseif ("landscapeRight" == stringValue) or ("landscape" == stringValue) then
				supportsOrientationLandscapeRight = true
				defaultOrientation = "landscape"
			elseif ("landscapeLeft" == stringValue) then
				supportsOrientationLandscapeLeft = true
				defaultOrientation = "reverseLandscape"
			end
		end

		-- Fetch all supported orientations.
		local supported = buildSettings.orientation.supported
		if "table" == type(supported) then
			for i = 1 , #supported do
				stringValue = supported[i]
				if ("string" == type(stringValue)) then
					if ("portrait" == stringValue) then
						supportsOrientationPortrait = true
					elseif ("portraitUpsideDown" == stringValue) then
						supportsOrientationPortraitUpsideDown = true
					elseif ("landscapeRight" == stringValue) or ("landscape" == stringValue) then
						supportsOrientationLandscapeRight = true
					elseif ("landscapeLeft" == stringValue) then
						supportsOrientationLandscapeLeft = true
					end
				end
			end
		end

		-- Determine if this app supports orientation changes.
		if (supportsOrientationPortrait or supportsOrientationPortraitUpsideDown) and
		   (supportsOrientationLandscapeRight or supportsOrientationLandscapeLeft) then
			supportsOrientationChange = true
		end
	end

	-- Fetch settings within the Android table.
	if "table" == type(buildSettings.android) then
		-- Fetch the version code.
		fetchVersionCodeFrom(buildSettings.android.versionCode)

		-- Fetch version name. This can be either a string or number. (Number is converted to string.)
		fetchVersionNameFrom(buildSettings.android.versionName)

		-- Fetch minimum sdk version.
		fetchMinSdkVersionFrom(buildSettings.android.minSdkVersion)

		-- Fetch the "usesExpansionFile" flag.
		if type(buildSettings.android.usesExpansionFile) == "boolean" then
			usesExpansionFile = buildSettings.android.usesExpansionFile
		end

		-- Fetch the large heap flag.
		if type(buildSettings.android.largeHeap) == "boolean" then
			largeHeap = buildSettings.android.largeHeap
		end

		-- Fetch the "isGame" flag.
		if type(buildSettings.android.isGame) == "boolean" then
			isGame = buildSettings.android.isGame
		end

		-- Fetch install location.
		stringValue = buildSettings.android.installLocation
		if ("string" == type( stringValue )) and (string.len( stringValue ) > 0) then
			installLocation = stringValue
		end

		-- Determine if the "supportsTV" option is enabled.
		if type(buildSettings.android.supportsTV) == "boolean" and buildSettings.android.supportsTV == true then
			-- In order to make this app visible in the Google Play store for puchase by Android TV devices,
			-- the following settings need to be automatically added to the manifest.
			mainIntentFilterCategories["android.intent.category.LEANBACK_LAUNCHER"] = true
			fetchUsesFeaturesFrom(
			{
				{ name = "android.hardware.touchscreen", required = false },
				{ name = "android.software.leanback", required = false },
			})
		end

		-- Fetch Google Play Games App ID if included
		stringValue = buildSettings.android.googlePlayGamesAppId
		if ("string" == type( stringValue )) and (string.len( stringValue ) > 0) then
			googlePlayGamesAppId = stringValue
		elseif "number" == type( stringValue ) then
			googlePlayGamesAppId = tostring(stringValue)
		end

		-- Fetch Facebook App ID if included.
		-- Note: We also support this field with a capitol 'F' and 'D' in case it was copied from iOS plist field.
		stringValue = ""
		if buildSettings.android.facebookAppId then
			stringValue = buildSettings.android.facebookAppId
		elseif buildSettings.android.FacebookAppID then
			stringValue = buildSettings.android.FacebookAppID
		end
		if ("string" == type( stringValue )) and (string.len( stringValue ) > 0) then
			facebookAppId = stringValue
		elseif "number" == type( stringValue ) then
			facebookAppId = tostring(stringValue)
		end

		-- Fetch the "coronaWindowMovesWhenKeyboardAppears" flag used to pan app when keyboard is shown.
		-- Note: We also support this field with a capitol 'C' in case it was copied from iOS plist field.
		if type(buildSettings.android.coronaWindowMovesWhenKeyboardAppears) == "boolean" then
			coronaWindowMovesWhenKeyboardAppears = buildSettings.android.coronaWindowMovesWhenKeyboardAppears
		elseif type(buildSettings.android.CoronaWindowMovesWhenKeyboardAppears) == "boolean" then
			coronaWindowMovesWhenKeyboardAppears = buildSettings.android.CoronaWindowMovesWhenKeyboardAppears
		end

		-- Fetch the "initialSystemUiVisibility" flag used to set the systemUiVisibility before the splashScreen is shown.
		if type(buildSettings.android.initialSystemUiVisibility) == "string" then
			initialSystemUiVisibility = buildSettings.android.initialSystemUiVisibility
		end

		-- Fetch a flag indicating if Corona's FileContentProvider should provide public read-only access to files.
		if type(buildSettings.android.allowAppsReadOnlyAccessToFiles) == "boolean" then
			allowAppsReadOnlyAccessToFiles = buildSettings.android.allowAppsReadOnlyAccessToFiles
		end

		-- Fetch permissions to be added as <permission> tags to the AndroidManifest.xml file.
		-- These are used to create new permissions, not to use/request permissions.
		fetchPermissionsFrom(buildSettings.android.permissions)

		-- Fetch permissions to be added as <uses-permission> tags to the AndroidManifest.xml file.
		fetchUsesPermissionsFrom(buildSettings.android.usesPermissions)

		-- Fetch screen sizes supported.
		if "table" == type(buildSettings.android.supportsScreens) then
			for settingName, settingValue in pairs(buildSettings.android.supportsScreens) do
				supportsScreens[settingName] = tostring(settingValue)
			end
		end

		-- Fetch Corona activity flags
		if "table" == type(buildSettings.android.coronaActivityFlags) then
			for flagName, flagValue in pairs(buildSettings.android.coronaActivityFlags) do
				coronaActivityFlags[flagName] = flagValue
			end
		end

		-- Fetch "usesFeatures" settings.
		fetchUsesFeaturesFrom(buildSettings.android.usesFeatures)

		-- Fetch the "mainIntentFilter" settings.
		if "table" == type(buildSettings.android.mainIntentFilter) then
			fetchMainIntentFilterCategoriesFrom(buildSettings.android.mainIntentFilter.categories)
		end

		-- Fetch custom intent filters.
		fetchIntentFiltersFrom(buildSettings.android.intentFilters)

		-- Fetch custom raw XML elements
		fetchManifestChildXmlElementsFrom(buildSettings.android.manifestChildElements)
		fetchApplicationChildXmlElementsFrom(buildSettings.android.applicationChildElements)

		fetchStringsFrom(buildSettings.android.strings)
		fetchApkFilesFrom(buildSettings.android.apkFiles)
	end
end

-- Refine the default orientation assignment using info from the supported orientations.
if (supportsOrientationPortrait or supportsOrientationPortraitUpsideDown
	or supportsOrientationLandscapeRight or supportsOrientationLandscapeLeft) then

	if (supportsOrientationPortrait and
		not supportsOrientationPortraitUpsideDown and
		not supportsOrientationLandscapeRight and
		not supportsOrientationLandscapeLeft) then

		defaultOrientation = "portrait"

	elseif (not supportsOrientationPortrait and
		not supportsOrientationPortraitUpsideDown and
		supportsOrientationLandscapeRight and
		not supportsOrientationLandscapeLeft) then

		defaultOrientation = "landscape"

	elseif (supportsOrientationPortrait and
		supportsOrientationPortraitUpsideDown and
		not supportsOrientationLandscapeRight and
		not supportsOrientationLandscapeLeft) then

		defaultOrientation = "sensorPortrait"

	elseif (not supportsOrientationPortrait and
		not supportsOrientationPortraitUpsideDown and
		supportsOrientationLandscapeRight and
		supportsOrientationLandscapeLeft) then

		defaultOrientation = "sensorLandscape"

	elseif (not supportsOrientationPortrait and
		supportsOrientationPortraitUpsideDown and
		not supportsOrientationLandscapeRight and
		not supportsOrientationLandscapeLeft) then

		defaultOrientation = "reversePortrait"

	elseif (not supportsOrientationPortrait and
		not supportsOrientationPortraitUpsideDown and
		not supportsOrientationLandscapeRight and
		supportsOrientationLandscapeLeft) then

		defaultOrientation = "reverseLandscape"

	end

elseif (not defaultOrientation) then
	-- No default orientation has been set and no supported
	-- orientations are provided. So make something!
	defaultOrientation = "portrait"
	supportsOrientationPortrait = true
end -- The case of defaultOrientation being assigned, but no supported orientations is a programmer error!


----------------------------------------------------------------------------------------------------
-- Fetch plugin settings information.
----------------------------------------------------------------------------------------------------

if "table" == type(pluginSettingsCollection) then
	for index = 1, #pluginSettingsCollection do
		-- Merge the next plugin's settings with the above build settings.
		local pluginSettings = pluginSettingsCollection[index]
		if "table" == type(pluginSettings) then
			fetchPermissionsFrom(pluginSettings.permissions)
			fetchUsesPermissionsFrom(pluginSettings.usesPermissions)
			fetchUsesFeaturesFrom(pluginSettings.usesFeatures)
			if "table" == type(pluginSettings.mainIntentFilter) then
				fetchMainIntentFilterCategoriesFrom(pluginSettings.mainIntentFilter.categories)
			end
			fetchIntentFiltersFrom(pluginSettings.intentFilters)
			fetchApplicationChildXmlElementsFrom(pluginSettings.applicationChildElements)
			fetchManifestChildXmlElementsFrom(pluginSettings.manifestChildElements)
		end
	end
end


----------------------------------------------------------------------------------------------------
-- Create XML tag and attribute strings from the given build settings.
----------------------------------------------------------------------------------------------------

local stringBuffer
local stringArray
local manifestKeys = {}
local stringsKeys = {}

-- Store basic app settings.
manifestKeys.USER_APP_NAME = appName
manifestKeys.USER_ACTIVITY_PACKAGE = packageName
manifestKeys.USER_VERSION_CODE = tostring(versionCode)
manifestKeys.USER_VERSION_NAME = versionName
manifestKeys.USER_INSTALL_LOCATION = installLocation
manifestKeys.USER_MIN_SDK_VERSION = tostring(minSdkVersion)
manifestKeys.USER_FILE_CONTENT_PROVIDER_EXPORTED = tostring(allowAppsReadOnlyAccessToFiles)

-- Create a meta-data tag for the targeted app store if provided.
stringBuffer = ""
if ("string" == type(targetedAppStore)) and (string.len(targetedAppStore) > 0) then
	stringBuffer = '<meta-data android:name="targetedAppStore" android:value="' .. targetedAppStore .. '" />'
end
manifestKeys.USER_TARGETED_APP_STORE = stringBuffer

-- Create a meta-data tag for the "usesExpansionFile" setting if provided and only if targeting Google Play.
stringBuffer = ""
if usesExpansionFile and (targetedAppStore == "google") then
	stringBuffer = '<meta-data android:name="usesExpansionFile" android:value="true" />'
end
manifestKeys.USER_USES_EXPANSION_FILE = stringBuffer

stringBuffer = ""
if googlePlayGamesAppId then
	stringBuffer = '<meta-data android:name="com.google.android.gms.games.APP_ID" android:value="@string/corona_app_gsm_id" />'
	strings["corona_app_gsm_id"] = googlePlayGamesAppId
end
manifestKeys.USER_USES_GOOGLE_PLAY_GAMES = stringBuffer

-- Create a meta-data tags for Facebook integration if provided an App Id and Display Name.
stringBuffer = ""
if facebookAppId then
	stringBuffer = '<meta-data android:name="com.facebook.sdk.ApplicationId" android:value="@string/corona_app_facebook_id" />'
	strings["corona_app_facebook_id"] = facebookAppId
end
manifestKeys.USER_USES_FACEBOOK = stringBuffer

-- Create a meta-data tag for the "coronaWindowMovesWhenKeyboardAppears" setting, if provided.
stringBuffer = ""
if coronaWindowMovesWhenKeyboardAppears then
	stringBuffer = '<meta-data android:name="coronaWindowMovesWhenKeyboardAppears" android:value="true" />'
end
manifestKeys.USER_CORONA_WINDOW_MOVES_WHEN_KEYBOARD_APPEARS = stringBuffer

-- Create a meta-data tag for the "initialSystemUiVisibility" setting, if provided.
stringBuffer = ""
if initialSystemUiVisibility then
	stringBuffer = '<meta-data android:name="initialSystemUiVisibility" android:value="' .. initialSystemUiVisibility .. '" />'
end
manifestKeys.USER_INITIAL_SYSTEM_UI_VISIBILITY = stringBuffer

-- Create a "largeHeap" application tag attribute if set.
stringBuffer = ""
if largeHeap == true then
	stringBuffer = 'android:largeHeap="true"'
end
manifestKeys.USER_LARGE_HEAP = stringBuffer

-- Create an "isGame" application tag attribute if set.
stringBuffer = ""
if isGame == true then
	stringBuffer = 'android:isGame="true"'
end
manifestKeys.USER_IS_GAME = stringBuffer

-- Only create a "screenOrientation" attribute if the app does NOT support orientation changes.
-- This allows the OS to handle the orientation and respects system preferences lke auto-rotate.
stringBuffer = ""
if (not supportsOrientationChange) then
	stringBuffer = 'android:screenOrientation="' .. defaultOrientation .. '"'
else
	-- Our default orientation is unspecified, so the OS can do as it pleases.
	defaultOrientation = "unspecified"
end

-- Set the default orientation and create a meta-data tag for it as well.
manifestKeys.USER_DEFAULT_ORIENTATION = stringBuffer
manifestKeys.USER_REQUESTED_DEFAULT_ORIENTATION =
		'<meta-data android:name="requestedDefaultOrientation" android:value="' ..
		defaultOrientation .. '" />'

-- Create "permission" tags.
stringBuffer = ""
for index = 1, #permissions do
	local attributesString = ""
	for settingName, settingValue in pairs(permissions[index]) do
		attributesString = attributesString .. 'android:' .. settingName .. '="' .. tostring(settingValue) .. '" '
	end
	if (string.len(attributesString) > 0) then
		stringBuffer = stringBuffer .. "<permission " .. attributesString .. "/>\n\t"
	end
end
manifestKeys.USER_PERMISSIONS = stringBuffer

-- Create "uses-permission" tags.
stringBuffer = ""
for permissionName, permission in pairs(usesPermissions) do
	local attributesString = ""
	if 'boolean' == type(permission) then
		permission = {name=permissionName}
	end
	for settingName, settingValue in pairs(permission) do
		attributesString = attributesString .. 'android:' .. settingName .. '="' .. tostring(settingValue) .. '" '
	end
	if (string.len(attributesString) > 0) then
		stringBuffer = stringBuffer .. "<uses-permission " .. attributesString .. "/>\n\t"
	end
end
manifestKeys.USER_USES_PERMISSIONS = stringBuffer

-- Create a "supports-screens" tag if settings were provided.
stringBuffer = ""
for settingName, settingValue in pairs(supportsScreens) do
	stringBuffer = stringBuffer .. 'android:' .. settingName .. '="' .. settingValue .. '" '
end
if (string.len(stringBuffer) > 0) then
	stringBuffer = "<supports-screens " .. stringBuffer .. "/>"
end
manifestKeys.USER_SUPPORTS_SCREENS = stringBuffer

-- Add Corona activity flags if provided
stringBuffer = ""
for settingName, settingValue in pairs(coronaActivityFlags) do
	stringBuffer = stringBuffer .. 'android:' .. settingName .. '="' .. tostring(settingValue) .. '"\n'
end
-- set default (attributes can't appear multiple times)
if string.find(stringBuffer, "resizeableActivity") == nil then
	stringBuffer = stringBuffer .. 'android:resizeableActivity="false"\n'
end

manifestKeys.USER_CORONA_ACTIVITY_ATTRIBUTES = stringBuffer

-- Create "uses-feature" tags.

-- If the application is landscape only then we don't want the uses portrait feature since it will remove the landscape only devices
-- which is caused by the CameraActivity's screenOrientation property in the manifest

-- Check for a user setting so it won't be overridden
local hasUseLandScapeFeature = false
local hasUsePortraitFeature = false
for index = 1, #usesFeatures do
	if usesFeatures[index].name == "android.hardware.screen.portrait" then
		hasUsePortraitFeature = true
	end
	if usesFeatures[index].name == "android.hardware.screen.landscape" then
		hasUseLandScapeFeature = true
	end
end

if not hasUsePortraitFeature then
	-- If the app only supports portrait then its required or theres no settings and everything defaults to portrait
	local required = not supportsOrientationLandscapeRight and
					 not supportsOrientationLandscapeLeft
	table.insert(usesFeatures, {name = "android.hardware.screen.portrait", required = required})
end

if not hasUseLandScapeFeature then
	-- If the app only supports landscape then its required
	local required = (supportsOrientationLandscapeRight or supportsOrientationLandscapeLeft) and
					 hasOrientationTable and
					 not supportsOrientationPortrait and
					 not supportsOrientationPortraitUpsideDown
	table.insert(usesFeatures, {name = "android.hardware.screen.landscape", required = required})
end

stringBuffer = ""
for index = 1, #usesFeatures do
	local attributesString = ""
	for settingName, settingValue in pairs(usesFeatures[index]) do
		attributesString = attributesString .. 'android:' .. settingName .. '="' .. tostring(settingValue) .. '" '
	end
	if (string.len(attributesString) > 0) then
		stringBuffer = stringBuffer .. "<uses-feature " .. attributesString .. "/>\n\t"
	end
end
manifestKeys.USER_USES_FEATURES = stringBuffer

-- Create the main intent filter's "category" tags.
stringArray = {}
for categoryName, categoryEnabled in pairs(mainIntentFilterCategories) do
	if categoryEnabled then
		stringBuffer = string.format('<category android:name="%s"/>', categoryName)
		table.insert(stringArray, stringBuffer)
	end
end
manifestKeys.USER_MAIN_INTENT_FILTER_CATEGORIES = table.concat(stringArray, "\n\t\t\t\t")

-- Create all of the custom intent filter tags and concatenate them into one string.
stringBuffer = ""
for index = 1, #intentFilters do
	local nextIntentFilter = intentFilters[index]

	-- Append the start tag.
	stringBuffer = stringBuffer .. "<intent-filter "
	if ("string" == type(nextIntentFilter.label)) and (string.len(nextIntentFilter.label) > 0) then
		stringBuffer = stringBuffer .. 'android:label="' .. nextIntentFilter.label .. '"'
	end
	stringBuffer = stringBuffer .. ">\n"

	-- Append the action tags.
	if "table" == type(nextIntentFilter.actions) then
		for actionIndex = 1, #nextIntentFilter.actions do
			local actionName = nextIntentFilter.actions[actionIndex]
			if ("string" == type(actionName)) and (string.len(actionName) > 0) then
				if string.sub(actionName, 1, 1) == "." then
					actionName = packageName .. actionName
				end
				stringBuffer = stringBuffer .. '\t\t\t\t<action android:name="' .. actionName .. '"/>\n'
			end
		end
	end

	-- Append the category tags.
	if "table" == type(nextIntentFilter.categories) then
		for categoryIndex = 1, #nextIntentFilter.categories do
			local categoryName = nextIntentFilter.categories[categoryIndex]
			if ("string" == type(categoryName)) and (string.len(categoryName) > 0) then
				if string.sub(categoryName, 1, 1) == "." then
					categoryName = packageName .. categoryName
				end
				stringBuffer = stringBuffer .. '\t\t\t\t<category android:name="' .. categoryName .. '"/>\n'
			end
		end
	end

	-- Append the data tags.
	if "table" == type(nextIntentFilter.data) then
		stringArray = {}

		-- Attempt to fetch one data tag at the root level of this table.
		local dataTagString = createIntentFilterDataTagFrom(nextIntentFilter.data)
		if dataTagString then
			table.insert(stringArray, "\t\t\t\t" .. dataTagString)
		end

		-- Attempt to fetch an array of data tags within this table.
		for key, value in pairs(nextIntentFilter.data) do
			dataTagString = createIntentFilterDataTagFrom(value)
			if dataTagString then
				table.insert(stringArray, "\t\t\t\t" .. dataTagString)
			end
		end

		-- Append all of the data tags to the end of the string buffer.
		if #stringArray > 0 then
			stringBuffer = stringBuffer .. table.concat(stringArray, "\n") .. "\n"
		end
	end

	-- Append the end tag.
	stringBuffer = stringBuffer .. "\t\t\t</intent-filter>\n\t\t\t"
end
manifestKeys.USER_INTENT_FILTERS = stringBuffer

-- Create the XML elements to be inserted as children within the "manifest" block.
stringArray = {}
for elementString, elementEnabled in pairs(manifestChildXmlElements) do
	if elementEnabled then
		table.insert(stringArray, elementString)
	end
end
manifestKeys.USER_MANIFEST_CHILD_XML_ELEMENTS = table.concat(stringArray, "\n\t\t")

-- Create the XML elements to be inserted as children within the "application" block.
stringArray = {}
for elementString, elementEnabled in pairs(applicationChildXmlElements) do
	if elementEnabled then
		table.insert(stringArray, elementString)
	end
end
manifestKeys.USER_APP_CHILD_XML_ELEMENTS = table.concat(stringArray, "\n\t\t")

local function escapeStringsValue(str)
	-- &   &amp;
	-- <   &lt;
	-- >   &gt;
	-- "   &quot;
	-- '   &apos;
	local escapedValue = tostring(str):gsub("&", "&amp;")
	escapedValue = escapedValue:gsub("'", "\\'")
	escapedValue = escapedValue:gsub('"', '\\"')
	escapedValue = escapedValue:gsub(">", "&gt;")
	escapedValue = escapedValue:gsub("<", "&lt;")

	return escapedValue
end

-- Create the XML elements to be inserted as children within the "strings.xml" file.
stringArray = {}
for name, value in pairs(strings) do
	table.insert(stringArray, '<string name="'..tostring(name)..'">'..escapeStringsValue(value)..'</string>')
end
stringsKeys.USER_STRINGS = table.concat(stringArray, "\n\t")

-- Recursively finds @KEY@ in the value where it will also be replaced as well
local function replace(line, keys)
	for key, value in pairs(keys) do
		key = "@" .. key .. "@"
		local wasKeyFound = string.find( line, key, 1, true )
		if wasKeyFound then
			local value1 = replace(value, keys)
			line = string.gsub( line, key, value1 )
			break
		end
	end
	return line
end

----------------------------------------------------------------------------------------------------
-- Create a new "AndroidManifest.xml" with the given build settings.
----------------------------------------------------------------------------------------------------

-- Open the "AndroidManifest.xml" template file.
-- This file contains @KEY@ strings where we'll insert the given build settings to.
local manifestTemplateFileHandle = io.open( manifestTemplateFilePath, "r" )
if not manifestTemplateFileHandle then
	printError( "ERROR: The AndroidManifest.xml template file does not exist: ", manifestTemplateFilePath )
	os.exit( -1 )
end

-- Copy the template's contents to a new "AndroidManifest.xml" file containing the given build settings.
-- Note: We currently only support one @KEY@ string substitution per line.
local newManifestFileHandle = io.open( newManifestFilePath, "w" )
for line in manifestTemplateFileHandle:lines() do
	local replacedLine = replace(line, manifestKeys)
	newManifestFileHandle:write( replacedLine, "\n" )
end

-- Close the files.
newManifestFileHandle:close()
manifestTemplateFileHandle:close()

----------------------------------------------------------------------------------------------------
-- Create a new "strings.xml" file with the given build settings.
----------------------------------------------------------------------------------------------------

-- Open the "strings.xml" template file.
-- This file contains @KEY@ placeholders where we'll insert the given build settings to.
local stringsTemplateFileHandle = io.open( stringsTemplateFilePath, "r" )
if not stringsTemplateFileHandle then
	printError( "ERROR: The strings.xml template file does not exist: ", stringsTemplateFilePath )
	os.exit( -1 )
end

-- Copy the template's contents to a new "AndroidManifest.xml" file containing the given build settings.
-- Note: We currently only support one @KEY@ string substitution per line.
print("Opening: ", newStringsFilePath)
local newStringsFileHandle = io.open( newStringsFilePath, "w" )
for line in stringsTemplateFileHandle:lines() do
	local replacedLine = replace(line, stringsKeys)
	newStringsFileHandle:write( replacedLine, "\n" )
end

-- Close the files.
newStringsFileHandle:close()
stringsTemplateFileHandle:close()


----------------------------------------------------------------------------------------------------
-- Create a new "copy-files-to-apk.properties" file
----------------------------------------------------------------------------------------------------

print("Opening: ", newApkFilesFilePath)
local newApkFilesFileHandle = io.open( newApkFilesFilePath, "w" )
newApkFilesFileHandle:write( table.concat(apkFiles, "\n"), "\n" )
newApkFilesFileHandle:close()
