local templateXcent = [[
HEADLEN0<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>application-identifier</key>
	<string>ARG0</string>
	<key>get-task-allow</key>
	<ALLOW/>
</dict>
</plist>
]]

local function generateXcent( options )
	local allow
	if ( nil ~= string.find( options.signingIdentity, "Developer", 1, true ) ) then
		allow = "true"
	else
		allow = "false"
	end

	local filename = options.filename
	local outFile = assert( io.open( filename, "wb" ) )

	local data = templateXcent

	local numMatches = 0
	data, numMatches = string.gsub( data, "ARG0", options.appid )
	assert( numMatches == 1 )

	data, numMatches = string.gsub( data, "ALLOW", allow )
	assert( numMatches == 1 )

	local len = string.len( data )
	local lenHex = string.format( "%08x", len )

	local head = string.char( tonumber( 0xFA ), tonumber( 0xDE ), tonumber( 0x71 ), tonumber( 0x71 ) )
	data = string.gsub( data, "HEAD", head )

	-- translate 8 digit hex number into 4 raw bytes
	lenHex = string.char(	tonumber( lenHex:sub( 1, 2 ), 16 ),
							tonumber( lenHex:sub( 4, 4 ), 16 ),
							tonumber( lenHex:sub( 5, 6 ), 16 ),
							tonumber( lenHex:sub( 7, 8 ), 16 ) )

	assert( lenHex:len() == 4 )
	data = string.gsub( data, "LEN0", lenHex )

	outFile:write(data)
	assert( outFile:close() )

	if ( options.verbose ) then
		print( "Created " .. filename );
	end
end

options = {
	filename = arg[1],
	signingIdentity=arg[2],
	appid = "77GJDRNYVB." ..arg[3],
}

generateXcent( options )
