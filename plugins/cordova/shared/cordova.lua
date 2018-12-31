local Library = require "CoronaLibrary"

-- Create library
local lib = Library:new{ name='cordova', publisherId='com.coronalabs' }

lib.newCleaver = function()
	print( "WARNING: The 'cordova' library is not available on this platform." )
end

-- Return an instance
return lib
