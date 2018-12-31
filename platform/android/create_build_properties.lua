----------------------------------------------------------------------------------------------------
-- Creates a "build.properties" file to be sent to the Corona build server.
-- Used to generate an "AndroidManifest.xml" file.
----------------------------------------------------------------------------------------------------

-- Parse google-services.json and sdd strings to a table
local function infuseStringsWithGoogleServicesJson(android, srcDir, package)
	local json = require("json")

	local jsonFile = android.useGoogleServicesJson

	if type(jsonFile) == "boolean" and jsonFile then
		jsonFile = "google-services.json"
	end
	if type(jsonFile) ~= "string" or jsonFile == '' then
		return
	end
	local jsonFileHandle = io.open( srcDir .. "/" .. jsonFile, "r" )
	if not jsonFileHandle then
		return "The Google Services settings file (usually 'google-services.json') is required by setting.android.useGoogleServicesJson, but file not exist: '" .. tostring( jsonFile ) .. "'. It can download be downloaded from Project Settings in your Firebase control panel."
	end
	local gs = json.decode(jsonFileHandle:read("*a"))
	jsonFileHandle:close()

	if not gs then
		return "Google Service settings file '" .. tostring( jsonFile ) .. "' does not contain valid JSON. Please download a valid google-services.json from Project Settings in your Firebase control panel."
	end

	local client = nil
	if type(gs.client) == "table" then
		for _,c in ipairs(gs.client) do
			if type(c.client_info) == "table" 
				and type(c.client_info.android_client_info) == "table"
				and c.client_info.android_client_info.package_name == package 
			then
				client = c
				break
			end
		end
	end

	if not client then
		return "Unable to find client entry for package '" .. tostring(package) .. "' in Google Service settings file '" .. tostring( jsonFile ) .. "'. Please change application package name or disable useGoogleServicesJson in build.settings"
	end

	android.strings = android.strings or {}
	local strings = android.strings

	local function addString( name, src, path )
		if src == nil then
			return
		end

		local p = path[1]
		if #path == 1 then
			if src[p] then
				-- actual string replacement
				if strings[name] and strings[name] ~= src[p] then
					print(string.format("WARNING: settings.android.strings already contains Google Service value for key '%s': '%s'. Ignoring value from JSON ('%s').", tostring(name), tostring(strings[name]), tostring(src[p])))
				else
					strings[name] = src[p]
				end
			end
		else
			local nextSrc
			if type(p) == "function" then
				nextSrc = p(src)
			else
				nextSrc = src[p]
			end
			-- tail(path)
			addString(name, nextSrc, { select(2, unpack(path)) })
		end
	end

	local function checkClientType( src )
		for _,v in ipairs(src) do
			if type(v) == "table" and v.client_type == 3 then
				return v
			end
		end
	end

	-- values from https://developers.google.com/android/guides/google-services-plugin
	addString("google_app_id", client, {"client_info", "mobilesdk_app_id"})
	addString("gcm_defaultSenderId", gs, {"project_info", "project_number"})
	addString("default_web_client_id", client, {"oauth_client", checkClientType,"client_id"})
	addString("ga_trackingId", client, {"services", "analytics-service", "analytics_property", "tracking_id"})
	addString("firebase_database_url", gs, {"project_info", "firebase_url"})
	addString("google_api_key", client, {"api_key", 1, "current_key"})
	addString("google_crash_reporting_api_key", client, {"api_key", 1, "current_key"})
	addString("google_storage_bucket", gs, {"project_info", "storage_bucket"})

end



-- Creates a "build.properties" file with the given settings and the "build.settings" file.
-- Returns nil if succeeded. Returns an error message string if there was a failure.
function androidCreateProperties(destinationDirectory, packageName, projectDirectory, versionCode, versionName, targetedAppStore)
	local json = require("json")
	local lfs = require('lfs')
	local errorMessage = nil

	-- Make sure that the optional arguments are set to valid values.
	if not versionCode then
		versionCode = 1
	end
	if not versionName then
		versionName = "1.0"
	end
	if not targetedAppStore then
		targetedAppStore = "none"
	end

	-- Load the "build.settings" file, if it exists.
	if projectDirectory and projectDirectory ~= "" then
		errorMessage = nil
		local buildSettingsFilePath = projectDirectory .. "/build.settings"
		local buildSettingsFileHandle = io.open( buildSettingsFilePath, "r" )
		local chunk = nil
		local errorMessage = nil
		local result = false

		if buildSettingsFileHandle then
			chunk, errorMessage = loadfile( buildSettingsFilePath )
			if chunk then
				result, errorMessage = pcall(chunk)
			end
			buildSettingsFileHandle:close()
		end
		if result == false or errorMessage then
			if ("string" == type(errorMessage)) and (string.len(errorMessage) > 0) then
				errorMessage = 'The "build.settings" file contains the following error: ' .. errorMessage
			else
				errorMessage = 'Failed to access the "build.settings" file.'
			end
			return errorMessage
		end
	end

	-- Create the build properties from the given arguments and "build.settings".
	local buildProperties = {}
	buildProperties.packageName = packageName
	buildProperties.versionCode = versionCode
	buildProperties.versionName = versionName
	buildProperties.targetedAppStore = targetedAppStore
	if type(settings) == "table" then
		buildProperties.buildSettings = settings

		if type(settings.android) == "table" then
			local err = infuseStringsWithGoogleServicesJson(settings.android, projectDirectory, packageName)
			if err then
				return err
			end
		end
	end

	-- Create the "build.properties" file.
	errorMessage = nil
	local buildPropertiesFilePath = destinationDirectory .. "/build.properties"
	local buildPropertiesFileHandle, errorMessage = io.open( buildPropertiesFilePath, "wb" )
	if not buildPropertiesFileHandle then
		if ("string" == type(errorMessage)) and (string.len(errorMessage) > 0) then
			return errorMessage
		else
			return "Failed to create file: "..buildPropertiesFilePath
		end
	end
	buildPropertiesFileHandle:write(json.encode(buildProperties))
	buildPropertiesFileHandle:close()

	-- Create the build.xml file exclusion list
	-- (note that build.xml needs the file to exist so we always create it
	-- even if it's empty)
	errorMessage = nil
	local excludesFilePath = destinationDirectory .. "/excludesfile.properties"
	local excludesFileHandle, errorMessage = io.open( excludesFilePath, "wb" )
	if not excludesFileHandle then
		if ("string" == type(errorMessage)) and (string.len(errorMessage) > 0) then
			return errorMessage
		else
			return "Failed to create file: "..excludesFilePath
		end
	end

	local defaultExcludes = {
			"*.config",
			"*.lu",
			"*.bak",
			"*.orig",
			"*.swp",
			"*.DS_Store",
			"*.apk",
			"*.obb",
			"*.obj",
			"*.o",
			"*.lnk",
			"*.class",
			"*.log",
			".*",
			".*/**",
			"._*",
			"build.properties",
		}

	if settings and settings.excludeFiles then
		-- we have actual files to exclude
		print("Excluding specified files from build: ")
		for platform,excludes in pairs(settings.excludeFiles) do
			if platform == "all" or platform == "android" then
				for index,pattern in ipairs(excludes) do
					print("   excluding: "..pattern)
					-- Add '/**' to directories to better meet user expectations of globbing behavior
					attrs = lfs.attributes(projectDirectory.."/"..pattern)
					if attrs and attrs.mode == "directory" then
						pattern = pattern .. "/**"
					end
					-- Prepend directory wildcard so internal app structure is hidden
					pattern = "**/" .. pattern
					excludesFileHandle:write(pattern.."\n")
				end
			end
		end

	end

	-- add in the default exclusions
	for index,pattern in ipairs(defaultExcludes) do
		-- Prepend directory wildcard so internal app structure is hidden
		pattern = "**/" .. pattern
		excludesFileHandle:write(pattern.."\n")
	end

	excludesFileHandle:close()

	-- Return nil to indicate that we succeeded in creating the file.
	return nil
end


-- If "arg" was defined, then this script was called from the command line for local Android builds.
if arg then
	-- If no valid arguments have been received, then explain how to use this script.
	if (arg[1] == nil) or (arg[2] == nil) then
		print("USAGE: " .. arg[0] .. " destinationDirectory packageName [projectDirectory [versionCode [versionName [targetedAppStore]]]]")
		os.exit( -1 )
	end

	-- Create the "build.properties" file.
	local errorMessage = androidCreateProperties(arg[1], arg[2], arg[3], arg[4], arg[5], arg[6])
	if errorMessage then
		local logMessage = 'ERROR: Failed to create the "build.properties" file. '
		if ("string" == type(errorMessage)) and (string.len(errorMessage) > 0) then
			logMessage = logMessage .. errorMessage
		end
		print(logMessage)
		os.exit(-1)
	end
end

