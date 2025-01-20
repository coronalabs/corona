------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

local name, decodeBase64, rawVerify = ...

--print( "ccdata_loader", name, decodeBase64, rawVerify )

local errorFormats =
{
	signature = "License file \"%s\" is corrupted.",
	signatureType = "License file \"%s\" was found, but it cannot be verified.",
	containerType = "License file \"%s\" is malformed.",
	dataType = "License file \"%s\" has an invalid or unexpected file format.",
	missing = "License file \"%s\" was not found. This is needed to use this software product. Please contact Corona Labs to acquire a valid license.",
}

local result = nil

-- Module dependencies
local json = require "json"
local io = require "io"

-- Load file and read contents
local fileName = name .. ".ccdata"
local path = system.pathForFile( fileName )

if path then
--print( "ccdata_loader", path )

	local f = io.open( path )
	local contents = f:read( "*a" )
	f:close()

	local payload = json.decode( contents )
	if type( payload ) == "table" then
		if type( payload.metadata ) == "table" then
			local signature = payload["metadata"].signature
			if type( signature ) == "string" then

				if type( payload["data"] ) == "string" then

					local base64Data = payload["data"]
					local dataPayload = decodeBase64(base64Data)

					--Verify signature
					local verified = rawVerify( signature, dataPayload )

					if (verified) then
						local chunkResult = loadstring( dataPayload )
						payload.data = chunkResult
						result = payload
					else
						result = string.format( errorFormats.signature, fileName )
					end
				else
					result = string.format( errorFormats.dataType, fileName )
				end
			else
				result = string.format( errorFormats.signatureType, fileName )
			end
		else
			result = string.format( errorFormats.containerType, fileName )
		end
	end
else
	result = string.format( errorFormats.missing, fileName )
end

return result;