#!/usr/bin/env lua

local function toArrayOfCharBuffers( bytecodes, numCols )
	local result = {}

	local numBytes = #bytecodes
	local numFullRows = math.floor( numBytes / numCols )

	local format = string.rep( '0x%02X, ' , numCols )

	local i = 1
	local j = i + numCols
	for row=1,numFullRows do
		local str = string.format( format, string.byte( bytecodes, i, j ) )
		table.insert( result, str )

		i = i + numCols
		j = j + numCols
	end

	-- last row
	local numColsLastRow = numBytes - (numFullRows * numCols)
	j = i + numColsLastRow
	local format = string.rep( '0x%02X, ' , numColsLastRow )
	local str = string.format( format, string.byte( bytecodes, i, j ) )
	table.insert( result, str )

	return result
end


local function toNativeBuffer( luaFile, dstfile )
	local chunk, errorMsg = loadfile( luaFile )
	if errorMsg then
		print( "[ERROR] Could not convert to native buffer:\n", errorMsg )
		os.exit( 1 )
	end

	local bytecodes = string.dump( chunk )

	-- convert the Lua bytecode to array of char buffers
	local a = toArrayOfCharBuffers( bytecodes, 16 )

	local buffer = '\t' .. table.concat( a, '\n\t' )

	io.output( dstfile )
	io.write( buffer )
	io.write( '\n' )
end

local luaFile = arg[1]
local dstFile = arg[2]

if ( luaFile ) then
	toNativeBuffer( luaFile, dstFile )
else
	print( "Usage: " .. arg[0] .. " luaFile [dstFile]" )
	os.exit( 1 )
end
