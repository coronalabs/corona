------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------


function validateSettings(arg1, arg2)

	-- print("validateSettings: ", tostring(arg1), tostring(arg2))

	-- CoronaSDK doesn't expose loadfile() so we need to do it the hard way
	local function loadfile(path)

		if path then
			local fp = io.open(path, 'r')

			if fp then
				local lua = fp:read( '*a' )
				fp:close()

				return loadstring(lua), ""
			else
				return nil, "Cannot open "..path
			end
		else
			return nil, "Empty path"
		end
	end


	local exemplarSettingsFile = arg2
	local settingsLoaded = require(exemplarSettingsFile)
	if not settingsLoaded then
		print("ERROR: validate-settings: problem loading "..tostring(loadMesg))
		return false
	end
	local validSettings = (settings == nil) and application or settings -- switch between build.settings and config.lua

	local testSettingsFile = arg1
	local testSettings = nil

	local loadSettings, loadMesg = loadfile(testSettingsFile)
	if loadSettings ~= nil then
		local origPrint = print
		print = function (...) end -- disable print() while running file under test

		local ok, ret_or_err = pcall(loadSettings)

		print = origPrint

		if ok then
			testSettings = (settings == nil) and application or settings
			application = nil
			settings = nil
		else
			-- this generally means there are undefined functions being called so bail
			print("WARNING: validate-settings: "..testSettingsFile.." too complex for static analysis")
			-- print("\t"..ret_or_err)
			return false
		end

	else
		print("ERROR: validate-settings: problem loading "..testSettingsFile..": "..tostring(loadMesg))
		return false
	end


	function string:split( inSplitPattern, outResults )

		if not outResults then
			outResults = {}
		end
		local theStart = 1
		local theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
		while theSplitStart do
			table.insert( outResults, string.sub( self, theStart, theSplitStart-1 ) )
			theStart = theSplitEnd + 1
			theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
		end
		table.insert( outResults, string.sub( self, theStart ) )
		return outResults
	end

	local function contains(table, val)
		for i=1,#table do
			if table[i] == val then 
				return true
			end
		end
		return false
	end

	function handleTags(tag)
		if type(tag) == "string" and (tag == "DEPRECATED" or tag == "CORONA") then
			return " "..tag
		else
			return ""
		end
	end
	function genTableOutline(t, stoplist, prefix)
		local prefix = prefix or ""
		local outline = outline or ""

		for key,value in pairs(t) do
			-- print("+++++ ", prefix, key, type(key), value)
			if type(t[key]) == "table" and not contains(stoplist, key) and not (type(key) == "number" and key > 1) then
				item = prefix .. "." .. key
				subItem = genTableOutline(t[key], stoplist, item)
				outline = outline .. subItem
			elseif type(key) == "number" and key == 1 then
				outline = outline .. prefix .."[] (".. type(value)..")".. handleTags(value) .."\n"
			elseif type(key) ~= "number" then
				outline = outline .. prefix ..".".. key .." (".. type(value) .. ")".. handleTags(value) .."\n"
			end
		end

		return outline
	end

	-- These are items we recognize but cannot validate the contents of (generally because they're arbitrary)
	-- (imported with valid settings above)
	local currentStoplist = stoplist or {}

	valid_build_settings_outline = genTableOutline(validSettings, currentStoplist, "settings")
	-- print("validSettings: ", valid_build_settings_outline)

	test_build_settings_outline = genTableOutline(testSettings, currentStoplist, "settings")
	-- print("testSettings: ", test_build_settings_outline)
	test_build_settings_table = test_build_settings_outline:split("\n")

	local filenamePrinted = false
	for k, v in pairs(test_build_settings_table) do
		if v ~= "" then
			local escapedV = v:gsub('%(', '%%(')
			escapedV = escapedV:gsub('%)', '%%)')
			escapedV = escapedV:gsub('%[', '%%[')
			escapedV = escapedV:gsub('%]', '%%]')
			-- print("escapedV: ", escapedV)
			local mesg = ""
			if not valid_build_settings_outline:match(escapedV) then
				mesg = "WARNING: unrecognized key: ".. tostring(v)
				if valid_build_settings_outline:upper():match(escapedV:upper()) ~= nil then
					mesg = mesg .." (incorrect capitalization)"
				end
			elseif valid_build_settings_outline:match(escapedV .. " DEPRECATED") then
				mesg = "WARNING: deprecated key: ".. tostring(v)
			elseif valid_build_settings_outline:match(escapedV .. " CORONA") then
				mesg = "WARNING: key overrides Corona: ".. tostring(v)
			end

			if mesg ~= "" then
				if not filenamePrinted then
					print("WARNING: issue found in ".. testSettingsFile ..": ")
					filenamePrinted = true
				end
				print(mesg)
			end
		end
	end

	return true
end

if arg ~= nil and arg[1] ~= nil then
	validateSettings(arg[1], arg[2])
end

