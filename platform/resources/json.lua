------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

---- Wrapper for dkjson.lua that implements the defaults we want (and improves
---- a common error behavior)

local dkjson = require "dkjson"

-- Give ourselves a different version
dkjson.version = dkjson.version .. "-1"

local decode_base = dkjson.decode
local encode_base = dkjson.encode

local function decode_override(str, pos, nullval, ...)
	-- Check for common error that gives an inscrutable message and
	-- provide a better one
	if str == nil then
		error("json.decode called with nil string", 2)
	else
		return decode_base(str, pos, nullval, ...)
	end
end

local function encode_override(value, state)
	-- Enforce a default exception handler for data that JSON 
	-- cannot encode.  To restore the default behavior define
	-- an exception handler that calls error()
	if state == nil then
		state = {}
	end
	if state.exception == nil then
		state.exception = dkjson.encodeexception
	end

	return encode_base(value, state)
end

local function json_prettify(obj)
	-- Return a human readable JSON string representing the
	-- given Lua object or JSON string (top-level keys are sorted)

	local pos, errorMsg

	if obj == nil then
		error("json.prettify called with nil string", 2)
	elseif type(obj) == "string" then
		-- Assume we were given a JSON string
		local len = obj:len()
		obj, pos, errorMsg = decode_base(obj)

		if errorMsg ~= nil then
			error("json.prettify: invalid input: ".. tostring(errorMsg), 2)
			return nil
		elseif pos < len then
			print("Warning: json.prettify: extra data at end of JSON")
			return nil
		end

	elseif type(obj) ~= "table" then
		error("json.prettify takes a string or a table (got "..type(obj)..")", 2)
		return nil
	end

	local keyorder = {}
	for k, v in pairs(obj) do
		keyorder[#keyorder + 1] = k
	end
	table.sort(keyorder)

	return encode_override(obj, { indent = true, keyorder = keyorder })
end

local function json_decode_file(filename, pos, nullval, ...)
	local decodedData = nil
	local fp, fileOpenErrorMsg = io.open(filename, 'r')

	if not fp then
		-- Couldn't open file
		return nil, 0, "Cannot open file "..tostring(fileOpenErrorMsg)
	else
		local str = fp:read( '*a' )
		fp:close()
		decodedData, lineno, errorMsg = decode_override( str, pos, nullval, ... )
		if errorMsg ~= nil then
			return nil, lineno, errorMsg
		end
	end

	return decodedData, 0, nil
end

dkjson.decode = decode_override
dkjson.decodeFile = json_decode_file
dkjson.encode = encode_override
dkjson.prettify = json_prettify

return dkjson
