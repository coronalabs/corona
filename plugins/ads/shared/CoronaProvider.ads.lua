local Provider = require "CoronaProvider"

local Class = Provider:newClass( "CoronaProvider.ads" )

--[[
AdsProvider.initialize = function( self )
	if self.name then
		AdsProvider.register( self )
	end
end

-- Internal variable
-- Ensure AdsProvider instances create a unique name
local providerList = { [AdsProvider.name]=AdsProvider }

AdsProvider.register = function( provider )
	local name = provider.name
	assert( name, "ERROR: Ad provider does not have 'name' property." )

	if not providerList[name] then
		providerList[name] = provider
		return true
	else
		print( "WARNING: Failed to register ad provider because another ad provider with the same name (" .. name .. ") has already been registered." )
	end
	return false
end

AdsProvider.find = function( name )
	assert( type(name) == "string", "Bad argument #1 to 'AdsProvider.find()' (string expected, got " .. type(name) .. ")" )

	return providerList[name]
end
--]]

-- Default implementations
local function defaultFunction()
	print( "WARNING: The 'ads' library is not available on this platform." )
end

Class.isStub = true
Class.init = defaultFunction
Class.show = defaultFunction
Class.hide = defaultFunction
Class.height = defaultFunction
Class.load = defaultFunction
Class.isLoaded = defaultFunction
Class.testAppId = 'No_TestAppId_Exists'

-- Return an instance
return Class
