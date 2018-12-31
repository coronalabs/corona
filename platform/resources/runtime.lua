------------------------------------------------------------------------------
--
-- Copyright (C) 2018 Corona Labs Inc.
-- Contact: support@coronalabs.com
--
-- This file is part of the Corona game engine.
--
-- Commercial License Usage
-- Licensees holding valid commercial Corona licenses may use this file in
-- accordance with the commercial license agreement between you and 
-- Corona Labs Inc. For licensing terms and conditions please contact
-- support@coronalabs.com or visit https://coronalabs.com/com-license
--
-- GNU General Public License Usage
-- Alternatively, this file may be used under the terms of the GNU General
-- Public license version 3. The license is as published by the Free Software
-- Foundation and appearing in the file LICENSE.GPL3 included in the packaging
-- of this file. Please review the following information to ensure the GNU 
-- General Public License requirements will
-- be met: https://www.gnu.org/licenses/gpl-3.0.html
--
-- For overview and more information on licensing please refer to README.md
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