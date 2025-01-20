------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------


-- Internal Lua functions used by librtt.
-- TODO: Make inaccessible to main.lua code

-- creates an array of scale/suffix pairs sorted in order of increasing scale
function _createImageSuffixTable( suffixes )
	local result = {}

	local numPairs = 0
	
	for k,v in pairs( suffixes ) do
		if ( type(k) == "string" and type(v) == "number" ) then
			local item = { scale=v, suffix=k }
			table.insert( result, item )
			numPairs = numPairs + 1
		end
	end

	if numPairs > 0 then
		table.sort( result, function( a, b ) return a.scale < b.scale end )
	else
		result = nil
	end

	return result
end