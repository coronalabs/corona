local M = {}

-------------------------------------------------------------------------------

local json = require "json"

-------------------------------------------------------------------------------

-- BuilderUtils.checkError( status, msg, command )
M.checkError = function( status, msg, command )
	if 0 ~= status then
		local error =
		{
			command = command,
			status = status,
			message = msg,
		}

		local errorJson = json.encode( error )
		assert( false, "<error>" .. errorJson .. "</error>" )
	end

	return status, msg
end

-- BuilderUtils.execute( cmd, verbose, keepTrailingNewline )
M.execute = function( cmd, verbose, keepTrailingNewline )
	-- Eliminate buffering
	io.stdout:setvbuf( "no" )
	io.stderr:setvbuf( "no" )

	local path = os.tmpname()
	os.remove( path )
	local status = os.execute( cmd .. ' >' .. path .. ' 2>&1' )

	local f = io.open( path, 'r' )
	local msg = f:read( "*all" )
	f:close()

	os.remove( path )

	-- Remove trailing newline
	if msg and not keepTrailingNewline then
		local i1,i2 = msg:find( '\n$' )
		if i1 then
			msg = msg:sub( 1, i1 - 1 )
		end
	end

	if verbose then
		print( "", "<execute>" .. cmd .. "</execute>" )
		print( "", "<result>" .. tostring(msg) .. "</result>" )
	end

	return status, msg, cmd
--[[
	local f = io.popen( cmd .. ' 2>&1; echo "-retcode:$?"', 'r' ) 
--	local f = io.popen( cmd .. '; echo "-retcode:$?"', 'r' )
	f:flush()
	local msg = f:read( '*a' )
	if verbose then
		print( "", "<execute>" .. cmd .. "</execute>" )
		print( "", "<result>" .. tostring(msg) .. "</result>" )
	end
	f:close()

	local i1,i2,status = msg:find('%-retcode:(%d+)\n$')
	if not keepTrailingNewline and i1 > 1 then i1 = i1 - 1 end 
	msg = msg:sub(1,i1-1)
	return tonumber(status), msg
--]]
end

M.xcrun = function( sdkType, cmd )
	-- discard stderr because 'xcrun' sometimes prints warnings there
	local status, result = M.checkError( M.execute( "xcrun --sdk " .. sdkType .. " " .. cmd .. " 2>/dev/null" ) )

	if 0 ~= status then
		print( "WARNING: xcrun failed (sdkType, cmd, result): ", sdkType, cmd, result )
		result = nil
	end

	return result
end

M.tableDuplicate = function( source )
	local result

	if source then
		result = {}
		for k,v in pairs( source ) do
			if type( v ) == "table" then
				v = M.tableDuplicate( v )
			end

			result[k] = v
		end
	end

	return result
end

M.setVerbose = function( newValue )
	M._verbose = newValue
end

M.printVerbose = function( ... )
	if M._verbose then
		print( ... )
	end
end

M.fileExists = function( path )
	local f = io.open( path,"r" )

	local result = ( nil ~= f )
	if result then
		f:close()
	end

	return result
end

return M
