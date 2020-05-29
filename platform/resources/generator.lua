------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Generator
-------------------------------------------------------------------------------

Generator = {}

function Generator.random( min, max )
	assert( min <= max, "Error: Generator.random requires 1st arg <= 2nd arg" )
	return function() return math.random( min, max ) end
end
