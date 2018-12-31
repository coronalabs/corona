local Library = require "CoronaLibrary"

-- Create library
local lib = Library:new{ name='analytics', publisherId='com.coronalabs', usesProviders=true }

local didInitProvider = {}

lib.init = function( providerName, appId )
	-- In the old version, 'appId' was the only parameter so if there is only 
	-- one param, we assume it's the old version for backwards compatibility, 
	if not appId then
		appId = providerName
		providerName = "flurry"
	end

	local result = false
	local shouldInit = not didInitProvider[providerName]

	if shouldInit then
		if lib:setCurrentProvider( providerName ) then
			local hasAppId = type( appId ) == "string"
			if not hasAppId then
				appId = lib.testAppId

				local message = string.format( "No app id was supplied to analytics.init(). Using test app id (%s) instead for analytics provider '%s'", ( appId or '' ), providerName )
---				print( "WARNING: " .. message );
				native.showAlert( "Warning", message, { "OK" } )
			end

			if '' == appId then
				print( "WARNING: An invalid app ID was given to the analytics.init() function. This ID must be set to a non-empty string." )
			else
				result = lib:getProvider().init( providerName, appId, listener )
				didInitProvider[providerName] = result
			end
		end
	else
		if providerName then
			print( "WARNING: analytics.init() was already called for " .. providerName .. "." )
		else
			print( "WARNING: analytics.init() missing parameters." )
		end
	end

	return result
end

-- Return an instance
return lib
