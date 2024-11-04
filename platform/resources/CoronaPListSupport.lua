------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

---- Code to update an iOS app's Info.plist with info from an app's build.settings

local json = require("json")

-- Double quote a string escaping backslashes and any double quotes
local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

-- get the numeric value of the "debugBuildProcess" preference or 0 if it's not set (note due to a Lua bug
-- the value is actually the exit code multiplied by 256)
local debugBuildProcess = os.execute("exit $(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null || echo 0)")

CoronaPListSupport = {}

-- Generate plugin dir names
local function getPluginDirNames( pluginsDir )
	local result = {}
	local lfs = require("lfs")

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
	return result
end

-- Returns a manifest (array of plugin metadata)
local function findPlugins( pluginsDir )
	local result = {}

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
			table.insert( result, plugin )
		end
	end

	return result
end

local function getDelegates(tmpDir)
	local pluginsDir = tmpDir .. "/.build"

	local pluginManifests = {}

	-- required here because local ios builds don't have lfs but simulator builds do.
	-- This function should only be called in simulator builds
	lfs = require("lfs")

	local attributes = lfs.attributes(pluginsDir, "mode")
	if attributes == "directory" then
		pluginManifests = findPlugins(pluginsDir)
	end

	local delegatesSet = {}
	local delegates = {}
	-- Put the delegate classes into a set first to remove dups
	for i=1,#pluginManifests do
		local manifest = pluginManifests[i]
		if manifest.delegates then
			for j=1,#manifest.delegates do
				delegatesSet[manifest.delegates[j]] = true
			end
		end
	end

	-- Reformat the table because this is the way that the json->plist stuff expects it
	for k, v in pairs(delegatesSet) do
		if debugBuildProcess and debugBuildProcess ~= 0 then
			print("Delegate class: ", k)
		end
		table.insert( delegates, k )
	end

	return delegates
end

local function inArray(array, item)
    for key, value in pairs(array) do
        if value == item then return key end
    end
    return nil
end
local function addLiveBuildsPlist(buildSettingsPlist, options)
	if options.liveBuild then
		buildSettingsPlist = buildSettingsPlist or {}
		buildSettingsPlist.NSLocalNetworkUsageDescription = buildSettingsPlist.NSLocalNetworkUsageDescription or "Solar2D Live Builds are using local network to synchronize the project."
		buildSettingsPlist.NSBonjourServices = buildSettingsPlist.NSBonjourServices or {}
		if not inArray(buildSettingsPlist.NSBonjourServices, "_corona_live._tcp.") then
			buildSettingsPlist.NSBonjourServices[#buildSettingsPlist.NSBonjourServices + 1] = "_corona_live._tcp."
		end
	end
	return buildSettingsPlist
end

function CoronaPListSupport.modifyPlist( options )
	local delegates

	-- The local iOS builds don't have options.tmpDir specified, iOS builds through the simulator do
	if options.tmpDir then
		delegates = getDelegates(options.tmpDir)
	end

	if debugBuildProcess and debugBuildProcess ~= 0 then
		print("CoronaPListSupport.modifyPlist: options: "..json.prettify(options))
	end

	local infoPlistFile
	local tmpJSONFile = os.tmpname()
	local infoPlist = nil

	if options.targetPlatform == "OSX" then
		infoPlistFile = options.appBundleFile .. "/Contents/Info.plist"
	elseif options.targetPlatform == "iOS" or options.targetPlatform == "tvOS" then
		infoPlistFile = options.appBundleFile .. "/Info.plist"
	else
		print("modifyPlist: unknown platform '"..tostring(options.targetPlatform).."', defaulting to 'iOS'")
		infoPlistFile = options.appBundleFile .. "/Info.plist"
		options.targetPlatform = "iOS"
	end

    print("Creating Info.plist...")

	-- Convert the Info.plist to JSON and read it in
	os.execute( "plutil -convert json -o '"..tmpJSONFile.."' "..infoPlistFile)

	local jsonFP, errorMsg = io.open(tmpJSONFile, "r")
	if jsonFP ~= nil then

		local jsonDataStr = jsonFP:read("*a")

		jsonFP:close()

		infoPlist, errorMsg = json.decode( jsonDataStr );

		if infoPlist == nil then
			error("failed to load "..tmpJSONFile..": "..errorMsg.."\nJSON: "..tostring(jsonDataStr))
		end
	else
		error("failed to open modifyPlist input file '"..tmpJSONFile.."': "..errorMsg)
	end

    os.remove(tmpJSONFile)

	-- infoPlist now contains a Lua table representing the Info.plist

	if debugBuildProcess and debugBuildProcess ~= 0 then
		print("Base Info.plist: " .. json.encode(infoPlist, { indent = true }))
	end

	if delegates then
		infoPlist.CoronaDelegates = delegates
	end

	if options.bundledisplayname then
		infoPlist.CFBundleDisplayName = options.bundledisplayname
	end
	if options.bundleexecutable then
		infoPlist.CFBundleExecutable = options.bundleexecutable
	end
	if options.bundleid then
		infoPlist.CFBundleIdentifier = options.bundleid
	end
	if options.bundlename then
		infoPlist.CFBundleName = options.bundlename
	end
	if options.corona_build_id then
		infoPlist.CoronaSDKBuild = options.corona_build_id
	end

	local bundleVersionSource = "not set"
	local bundleShortVersionStringSource = "not set"

	-- We process app Info.plists effectively twice, once when the templates are built and once when
	-- the app itself is built. The meta Info.plist has the place holders prefixed with "TEMPLATE_"
	-- so when we see that we replace it with the normal placeholders.
	if infoPlist.CFBundleVersion == "@TEMPLATE_BUNDLE_VERSION@" then
		infoPlist.CFBundleVersion = "@BUNDLE_VERSION@"
		infoPlist.CFBundleShortVersionString = "@BUNDLE_SHORT_VERSION_STRING@"
	else
		-- Only set these if they are not already set (which may happen in Enterprise builds)
		if infoPlist.CFBundleVersion == nil or infoPlist.CFBundleVersion == "@BUNDLE_VERSION@" then
			-- Apple treats this the build number so, if it hasn't been specified in the build.settings, we
			-- set it to the current date and time which is unique for the app and human readable
			bundleVersionSource = (infoPlist.CFBundleVersion == "@BUNDLE_VERSION@") and "set by Simulator" or "set by Info.plist"
			local datedata = os.date( "!*t")
			infoPlist.CFBundleVersion = datedata.year .. '.' .. datedata.month.."." .. datedata.day .. os.date("!%H%M")
		end

		local version = options.bundleversion or "1.0.0"
		if infoPlist.CFBundleShortVersionString == nil or infoPlist.CFBundleShortVersionString == "@BUNDLE_SHORT_VERSION_STRING@" then
			bundleShortVersionStringSource = (infoPlist.CFBundleShortVersionString == "@BUNDLE_SHORT_VERSION_STRING@") and "set in Build dialog" or "set by Info.plist"
			infoPlist.CFBundleShortVersionString = version
		end
	end

	if options.targetPlatform == "iOS" then
		if infoPlist.UIDeviceFamily == nil or options.targetDevice ~= nil then
			-- iOS case
			-- The behavior here needs to work for both Xcode Enterprise builds and server Simulator builds

			-- If the user specified iPhone or iPad only, modify the plist to not do Universal
			local isolatedTargetDevice = options.targetDevice

			if isolatedTargetDevice == nil then
				isolatedTargetDevice = 127 -- kIOSUniversal, see platform/shared/Rtt_TargetDevice.h
			end

			-- The Obj-C code uses mask bits to flag whether the target is building for device or
			-- Xcode simulator.
			-- Since we don't have bitwise operations in Lua, we do things the hard way.
			if isolatedTargetDevice >= 128 then
				-- removes the Xcode simulator flag
				isolatedTargetDevice = isolatedTargetDevice - 128
			end
			if isolatedTargetDevice == 0 or isolatedTargetDevice == 1 then
				-- Corona uses 0 and 1 for iPhone and iPad, but UIDeviceFamily uses 1 and 2
				infoPlist.UIDeviceFamily = { isolatedTargetDevice+1 }
			else
				-- "universal" just means "both iPhone and iPad"
				infoPlist.UIDeviceFamily = { 1, 2 }
			end
		end

		-- build.settings
		local defaultSettings = {
			orientation =
			{
				default = "portrait"
			}
		}
		local settings = options.settings or defaultSettings
		if settings then
			-- cross-platform settings
			local orientation = settings.orientation or defaultSettings.orientation
			if orientation then
				local defaultOrientation = orientation.default
				local supported = {}
				if defaultOrientation then
					local value = "UIInterfaceOrientationPortrait"
					if "landscape" == defaultOrientation or "landscapeRight" == defaultOrientation then
						value = "UIInterfaceOrientationLandscapeRight"
					elseif "landscapeLeft" == defaultOrientation then
						value = "UIInterfaceOrientationLandscapeLeft"
					end

					table.insert( supported, value )
					infoPlist.UIInterfaceOrientation = value
				end

				infoPlist.ContentOrientation = nil
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
						infoPlist.ContentOrientation = value
					end
				end

				infoPlist.CoronaViewSupportedInterfaceOrientations = nil
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

				infoPlist.CoronaViewSupportedInterfaceOrientations = supported
				infoPlist.UISupportedInterfaceOrientations = supported
			end

			-- add'l custom plist settings specific to iPhone
			local buildSettingsPlist = settings.iphone and settings.iphone.plist
			buildSettingsPlist = addLiveBuildsPlist(buildSettingsPlist, options)

			if buildSettingsPlist then
				--print("Adding custom plist settings: ".. json.encode(buildSettingsPlist))
				--print("buildSettingsPlist: "..json.encode(buildSettingsPlist))
				--print("infoPlist (before): "..json.encode(infoPlist))

				if buildSettingsPlist.CFBundleShortVersionString then
					bundleShortVersionStringSource = "set in build.settings"
				end
				if buildSettingsPlist.CFBundleVersion then
					bundleVersionSource = "set in build.settings"
				end

				for k, v in pairs(buildSettingsPlist) do
					local valuestr = ""
					if type(v) == "table" then
						valuestr = json.encode(v)
					else
						valuestr = tostring(v)
					end
					print("    adding extra plist setting "..k..": "..valuestr)
					infoPlist[k] = v
				end

				--print("infoPlist (after): "..json.encode(infoPlist, {indent = true}))
			end

		end

	elseif options.targetPlatform == "tvOS" then
		-- Only overwrite if doesn't already exist
		if infoPlist.UIDeviceFamily == nil then
			infoPlist.UIDeviceFamily = { 3 }
		end

		-- build.settings
		local defaultSettings = {
			orientation =
			{
				default = "landscape"
			}
		}
		local settings = options.settings or defaultSettings

		if settings then
			-- Force tvOS builds to landscapeRight
			local orientation = defaultSettings.orientation.default
			if orientation then
				local defaultOrientation = "UIInterfaceOrientationLandscapeRight"
				local supported = {
					defaultOrientation
				}

				infoPlist.UIInterfaceOrientation = defaultOrientation
				infoPlist.CoronaViewSupportedInterfaceOrientations = supported
				infoPlist.UISupportedInterfaceOrientations = supported
			end

			-- add'l custom plist settings specific to tvOS
			local buildSettingsPlist = settings.tvos and settings.tvos.plist
			buildSettingsPlist = addLiveBuildsPlist(buildSettingsPlist, options)

			if buildSettingsPlist then
				if buildSettingsPlist.CFBundleShortVersionString then
					bundleShortVersionStringSource = "set in build.settings"
				end
				if buildSettingsPlist.CFBundleVersion then
					bundleVersionSource = "set in build.settings"
				end

				for k, v in pairs(buildSettingsPlist) do
					local valuestr = ""
					if type(v) == "table" then
						valuestr = json.encode(v)
					else
						valuestr = tostring(v)
					end
					print("    adding extra plist setting "..k..": "..valuestr)
					infoPlist[k] = v
				end
			end

		end

	elseif options.targetPlatform == "OSX" then

		local settings = options.settings or defaultSettings
		if settings then
			-- add'l custom plist settings specific to OS X
			local buildSettingsPlist = settings.osx and settings.osx.plist

			if buildSettingsPlist then
				--print("Adding custom plist settings: ".. json.encode(buildSettingsPlist))
				--print("buildSettingsPlist: "..json.encode(buildSettingsPlist))
				--print("infoPlist (before): "..json.encode(infoPlist))

				if buildSettingsPlist.CFBundleShortVersionString then
					bundleShortVersionStringSource = "set in build.settings"
				end
				if buildSettingsPlist.CFBundleVersion then
					bundleVersionSource = "set in build.settings"
				end

				for k, v in pairs(buildSettingsPlist) do
					local valuestr = ""
					if type(v) == "table" then
						valuestr = json.encode(v)
					else
						valuestr = tostring(v)
					end
					print("    adding extra plist setting "..k..": "..valuestr)
					infoPlist[k] = v
				end

				--print("infoPlist (after): "..json.encode(infoPlist, {indent = true}))
			end

		end
	end

	if debugBuildProcess and debugBuildProcess ~= 0 then
		print("Final Info.plist: " .. json.encode(infoPlist, { indent = true }))
	end

	local outFP, errorString = io.open( tmpJSONFile, "w" )
	if outFP ~= nil then
		outFP:write( json.encode(infoPlist, {indent = true}) )
		outFP:close()

		-- Convert the JSON plist into an XML plist
		os.execute("plutil -convert xml1 -o "..infoPlistFile.." '"..tmpJSONFile.."'")
	else
		print("modifyPlist: failed to open output file '"..tmpJSONFile.."': "..errorString)
	end

	os.remove(tmpJSONFile)

	-- Do some trick formatting on the output strings
	local fmt = "%-" .. tostring(string.len(infoPlist.CFBundleVersion)) .. "s"
	print("Application version information:")
	print("    Version: ".. string.format(fmt, tostring(infoPlist.CFBundleShortVersionString)) .." [CFBundleShortVersionString] (".. bundleShortVersionStringSource ..")")
	print("      Build: ".. tostring(infoPlist.CFBundleVersion).." [CFBundleVersion] (".. bundleVersionSource .. ")")
end

function CoronaPListSupport.valueToPlistEntry( t, addDict )
	local ret = ""
	if "boolean" == type(t) then
		if t then
			ret = "<true/>\n"
		else
			ret = "<false/>\n"
		end
	elseif "string" == type(t) then
			ret = "<string>" .. t .. "</string>\n"
	elseif "table" == type(t) then
		if #t > 0 then
			ret = ret .. "<array>\n"
			for i=1,#t do
				ret = ret .. CoronaPListSupport.valueToPlistEntry(t[i], true)
			end
			ret = ret .. "</array>\n"
		else
			if addDict then
				ret = ret .. "<dict>\n"
			end
			for k, v in pairs(t) do
				ret = ret .. '<key>' .. k .. '</key>\n'
				ret = ret .. CoronaPListSupport.valueToPlistEntry(v, true)
			end
			if addDict then
				ret = ret .. "</dict>\n"
			end
		end
	elseif "number" == type(t) then
		ret = "<integer>" .. tostring(t) .. "</integer>\n"
	else
		print("ERROR: invalid entitlements table:", t, type(t) )
	end

	return ret
end

-- Run a command and capture its output in string
function CoronaPListSupport.captureCommandOutput( cmd, debugLevel )
	local debugLevel = debugLevel or 0
	local result = ""

	if debugBuildProcess and debugBuildProcess > debugLevel then
		print("captureCommandOutput: ".. cmd)
	end

	local cmdStdout = io.popen(cmd, "r")

	if cmdStdout ~= nil then
		-- Don't use "*all", it's unreliable with io.popen()
		local line = cmdStdout:read("*line")
		while line ~= nil do
			result = result .. line .. "\n"
			line = cmdStdout:read("*line")
		end
		-- remove trailing newline
		if result ~= "" then
			result = result:sub(0, result:len() - 1)
		end
		cmdStdout:close()
	end
	return result
end

function CoronaPListSupport.generateXcprivacy( settings, platform )
	local platformSettings = settings[platform]
	local ret = ""
	if platformSettings and platformSettings.xcprivacy then
		ret = ret .. CoronaPListSupport.valueToPlistEntry(platformSettings.xcprivacy)
	end

	return ret
end

function CoronaPListSupport.generateEntitlements( settings, platform, provisionProfile )
	local platformSettings = settings[platform]
	local includeProvisioning = false
	local ret = ""
	if platformSettings and platformSettings.iCloud then
		local iCloudEnabled = false
		local provProfileJson = CoronaPListSupport.captureCommandOutput("security cms -D -i '".. provisionProfile .."' | sed -Ee 's#<(\\/)?dat[ae]>#<\\1string>#g' | plutil  -convert json -o - -- - && echo")
		if provisionProfile and provProfileJson then
			local ppEnt = json.decode(provProfileJson)
			if ppEnt then
				ppEnt = ppEnt["Entitlements"]
			end
			local appId = nil
			if ppEnt then
				appId = ppEnt["application-identifier"] or ppEnt["com.apple.application-identifier"] or "*"
				if string.match(appId, "*") then
					print("ERROR: iCloud is enabled but signing with '*' provisioning profile. To use iCloud select provisioning profile with iCloud entitlements.")
					appId = nil
				end
			end
			if ppEnt and appId then
				local t = { ["com.apple.application-identifier"] = platform == 'osx' and appId or nil }
				local kvsContainer = ppEnt["com.apple.developer.ubiquity-kvstore-identifier"]
				if kvsContainer then
					if "table" == type(platformSettings.iCloud) and platformSettings.iCloud["kvstore-identifier"] then
						local kvsId = platformSettings.iCloud["kvstore-identifier"]
						kvsContainer = string.gsub(kvsContainer, "*", kvsId)
					else
						-- usually app id is something like HF56E.com.svoka.flashtest
						-- this code tries to separate App Id from team ID
						local kvsId = string.sub(appId, string.find(appId,"%.")+1 )
						kvsContainer = string.gsub(kvsContainer, "*", kvsId)
					end
					t["com.apple.developer.ubiquity-kvstore-identifier"] = kvsContainer
					print("Using iCloud KVStore identifier: " .. kvsContainer)
					iCloudEnabled = true
				end
				local iCloudServices = nil
				local docContainers = ppEnt["com.apple.developer.ubiquity-container-identifiers"]
				if docContainers and #docContainers>0 then
					t["com.apple.developer.ubiquity-container-identifiers"] = docContainers
					for i=1,#docContainers do
						if string.match(docContainers[i], "*") then
							print("ERROR: invalid entitlements for iCloud. Verify App ID Settings on Developer's portal: Xcode 5 profiles are not supported for Documents in iCloud or CloudKit.")
							t["com.apple.developer.ubiquity-container-identifiers"] = nil
							break
						end
					end
					if t["com.apple.developer.ubiquity-container-identifiers"] then
						if not iCloudServices then iCloudServices = {} end
						iCloudServices[#iCloudServices+1] = "CloudDocuments"
						iCloudEnabled = true
					end
				end
				local cloudKitContainers = ppEnt["com.apple.developer.icloud-container-identifiers"]
				if cloudKitContainers and #cloudKitContainers>0 then
					if not iCloudServices then iCloudServices = {} end
					iCloudServices[#iCloudServices+1] = "CloudKit"
					t["com.apple.developer.icloud-container-identifiers"] = cloudKitContainers
					t["com.apple.developer.icloud-container-environment"] = ppEnt["com.apple.developer.icloud-container-environment"]
					-- Apple doesn't accept to app store with Development container env. even if it is in provisioning profile
					if ppEnt["com.apple.developer.aps-environment"] or ppEnt["aps-environment"]=="production" then
						t["com.apple.developer.icloud-container-environment"] = {"Production",}
					end
					iCloudEnabled = true
				end
				t["com.apple.developer.icloud-services"] = iCloudServices
				if iCloudEnabled and ppEnt['com.apple.developer.team-identifier'] then
					t['com.apple.developer.team-identifier'] = ppEnt['com.apple.developer.team-identifier']
				end
				if iCloudEnabled then
					includeProvisioning = true
					ret = ret .. CoronaPListSupport.valueToPlistEntry(t)
				end
			end
		end
		if not iCloudEnabled then
			print("ERROR: setting." .. platform .. ".iCloud is enabled, but provisioning profile does not have iCloud entitlements." )
		end
	end

	if platformSettings and platformSettings.entitlements then
		local pe = platformSettings.entitlements
		if pe["com.apple.developer.icloud-container-identifiers"]
		or pe["com.apple.developer.ubiquity-container-identifiers"]
		or pe["com.apple.developer.ubiquity-kvstore-identifier"]
		then
			includeProvisioning = true
		end

		ret = ret .. CoronaPListSupport.valueToPlistEntry(pe)
	end

	return ret, includeProvisioning

end

-- compiles xcassets thing into the icons and lanuch images
function CoronaPListSupport.compileXcassets(options, tmpDir, srcAssets, xcassetPlatformOptions, settingsEntry)
	if options.settings and settingsEntry.xcassets then
		local xcassets = settingsEntry.xcassets
		local xcassetsFileName = "Images.xcassets"
		if type(xcassets) == 'string' then
			xcassetsFileName = xcassets
			xcassets = {}
		elseif type(xcassets) == 'table' then
			if type(xcassets.filename) == 'string' and xcassets.filename ~= "" then
				xcassetsFileName = xcassets.filename
				xcassets.filename = nil
			else
				print("WARNING: no xcassets.filename. Defaulting to " .. xcassetsFileName)
			end
		else
			print("Using default '" .. xcassetsFileName .. "' file for assets catalog")
			xcassets = {}
		end

		local iconPlistFile = tmpDir .. '/assetcatalog_generated_info.plist'

		local actoolCMD = "xcrun actool"
		actoolCMD = actoolCMD .. ' --output-format human-readable-text --warnings'
		if debugBuildProcess > 0 then
			actoolCMD = actoolCMD .. ' --notices'
		end
		--Sticker Packaging (Note App Template still needs to updates to include extention)
		if(settingsEntry.stickerPackIdentifierPrefix)then
			actoolCMD = actoolCMD .. ' --include-sticker-content --stickers-icon-role host-app --sticker-pack-identifier-prefix '..settingsEntry.stickerPackIdentifierPrefix
		end
		if(settingsEntry.localizedStickerPackFile)then
			actoolCMD = actoolCMD .. ' --sticker-pack-strings-file ' .. settingsEntry.localizedStickerPackFile
		end
		--Set Alternate Icons
		if(settingsEntry.alternateIcons)then
			for i=1,#settingsEntry.alternateIcons do
					actoolCMD = actoolCMD .. ' --alternate-app-icon '..settingsEntry.alternateIcons[i]
			end
		end
		actoolCMD = actoolCMD .. ' --export-dependency-info ' .. quoteString(tmpDir .. '/assetcatalog_dependencies')
		actoolCMD = actoolCMD .. ' --output-partial-info-plist ' .. quoteString(iconPlistFile)
		actoolCMD = actoolCMD .. ' --compress-pngs'
		actoolCMD = actoolCMD .. ' --enable-on-demand-resources YES'
		actoolCMD = actoolCMD .. ' --product-type com.apple.product-type.application'
		actoolCMD = actoolCMD .. ' --compile ' .. quoteString(options.dstDir .. '/' .. options.dstFile .. ".app")

		local manuallySetParameters = {}

		for k,v in pairs(xcassets) do
			if type(v) == 'table' then
				local param, val = unpack(v)
				if type(param) == 'string' and type(val) == 'string' then
					if #val > 0 then
						actoolCMD = actoolCMD .. ' --' .. param .. ' ' .. val
					end
					manuallySetParameters[param] = true
				else
					print("WARNING: invalid values in xcassets: " .. tostring(k) .. ": " .. tostring(param) .. ", " .. tostring(val))
				end
			elseif type(k) == 'string' and type(v) == 'string' then
				actoolCMD = actoolCMD .. ' --' .. k .. ' ' .. v
				manuallySetParameters[k] = true
			else
				print("WARNING: invalid values in xcassets: " .. tostring(k) .. ": " .. tostring(v))
			end
		end

		for _,v in ipairs(xcassetPlatformOptions) do
			local param, val = unpack(v)
			if not manuallySetParameters[param] then
				actoolCMD = actoolCMD .. ' --' .. param .. ' ' .. val
			end
		end

		actoolCMD = actoolCMD .. ' ' .. quoteString(srcAssets .. '/' .. xcassetsFileName)

		local result, errMsg = runScript( actoolCMD )
		if result ~= 0 then
			print("ERROR: while compiling icons: " .. tostring(errMsg))
			return errMsg
		end

		local iconJson = CoronaPListSupport.captureCommandOutput('plutil  -convert json -o - -- ' .. quoteString(iconPlistFile) .. ' && echo')

		if not settingsEntry.plist then settingsEntry.plist = {} end

		if debugBuildProcess > 0 then
			print("Icon JSON:", iconJson)
		end

		local plist = settingsEntry.plist
		for k,v in pairs(json.decode(iconJson)) do
			if plist[k] then
				print("WARNING: Skipping Plist entry for icon: " .. k)
			else
				plist[k] = v
			end
		end

	end
	return nil
end

return CoronaPListSupport
