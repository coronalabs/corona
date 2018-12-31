local Provider = require "CoronaProvider"

local Class = Provider:newClass( "CoronaProvider.analytics" )

-- Default implementations
local function defaultFunction()
	print( "WARNING: The 'analytics' library is not available on this platform." )
end

Class.isStub = true
Class.init = defaultFunction
Class.logEvent = defaultFunction

-- Return an instance
return Class
