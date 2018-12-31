#!/usr/bin/env lua

local template = [[
// ----------------------------------------------------------------------------
// 
// FILENAME.c
// 
// Bytecodes for FILENAME.lua
// 
// ----------------------------------------------------------------------------

#include "CoronaLua.h"
#include "CoronaMacros.h"

// ----------------------------------------------------------------------------

// Bytecodes
static const unsigned char kBuffer[] =
{
BYTECODES
};

// ----------------------------------------------------------------------------

CORONA_EXPORT int
CoronaPluginLuaLoad_VARIABLE( lua_State *L )
{
	return luaL_loadbuffer( L, (const char*)kBuffer, sizeof( kBuffer ), "FILENAME" );
}

// ----------------------------------------------------------------------------
]]

local function toArrayOfCharBuffers( bytecodes, numCols )
	local result = {}

	local numBytes = #bytecodes
	local numFullRows = math.floor( numBytes / numCols )

	--local valueFormat = '%3d, '
	local valueFormat = '0x%02X, '
	local format = string.rep( valueFormat , numCols )

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
	if numColsLastRow > 0 then
		j = i + numColsLastRow
		local format = string.rep( valueFormat , numColsLastRow )
		local str = string.format( format, string.byte( bytecodes, i, j ) )
		table.insert( result, str )
	end

	return result
end

local function toNativeBuffer( luaFile )
	local chunk, errorMsg = loadfile( luaFile )
	if errorMsg then
		print( "[ERROR] Could not convert to native buffer:\n", errorMsg )
		os.exit( 1 )
	end

	local bytecodes = string.dump( chunk )

	-- convert the Lua bytecode to array of char buffers
	local a = toArrayOfCharBuffers( bytecodes, 16 )

	local buffer = '\t' .. table.concat( a, '\n\t' )

	return buffer
end

local function toNativeSource( luaFile, dstFilename )
	local variable = dstFilename:gsub( '%.', '_' )
	local bytecodes = toNativeBuffer( luaFile )

	local contents = template
	contents = contents:gsub( "FILENAME", dstFilename )
	contents = contents:gsub( "VARIABLE", variable )
	contents = contents:gsub( "BYTECODES", bytecodes )
	return contents
end

local luaFile = arg[1]
local dstFilename = arg[2]
local dstPath = arg[3]

if ( luaFile and dstFilename ) then
	if dstPath then
		os.remove( dstPath )
	end
	io.output( dstPath )
	local contents = toNativeSource( luaFile, dstFilename )
	io.write( contents )
else
	print( "Usage: " .. arg[0] .. " luaFile dstFilename [dstPath]" )
	os.exit( 1 )
end
