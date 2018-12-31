local Library = require "CoronaLibrary"

-- Create library
local lib = Library:new{ name='ads', publisherId='com.coronalabs', usesProviders=true }

local didInitProvider = {}

lib.init = function( providerName, appId, listener )
	local result = false
	local shouldInit = not didInitProvider[providerName]

	if shouldInit then
		if lib:setCurrentProvider( providerName ) then
			local hasAppId = type( appId ) == "string"
			if not hasAppId then
				appId = lib.testAppId

				local message = string.format( "No app id was supplied to ads.init(). Using test app id (%s) instead for ad network '%s'", ( appId or '' ), providerName )
---				print( "WARNING: " .. message );
				native.showAlert( "Warning", message, { "OK" } )
			end

			if '' == appId then
				print( "WARNING: An invalid app ID was given to the ads.init() function. This ID must be set to a non-empty string." )
			else
				result = lib:getProvider().init( providerName, appId, listener )
				didInitProvider[providerName] = result
			end
		end
	else
		if providerName then
			print( "WARNING: ads.init() was already called for " .. providerName .. "." )
		else
			print( "WARNING: ads.init() missing parameters." )
		end
	end

	return result
end

-- Return an instance
---print( "ads", lib )
return lib
