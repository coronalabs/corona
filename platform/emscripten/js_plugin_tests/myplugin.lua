------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

local lib
local platform = system.getInfo("platform")
if platform == 'html5' then

	lib = require("myplugin_js")
else

	-- wrapper for non web platforms

	local CoronaLibrary = require "CoronaLibrary"

	-- Create stub library for simulator
	lib = CoronaLibrary:new{ name='myplugin', publisherId='com.my' }

	-- Default implementations
	local function defaultFunction()
		print( "WARNING: The '" .. lib.name .. "' library is not available on this platform." )
	end

	lib.method1 = defaultFunction
	lib.empty = function() end
	lib.echo = defaultFunction
	lib.get = defaultFunction
	lib.set = defaultFunction
	lib.log = defaultFunction
	lib.callSayHello = defaultFunction
	lib.withCallbacks = defaultFunction
	lib.echo1 = defaultFunction
	
end

-- Return an instance
return lib