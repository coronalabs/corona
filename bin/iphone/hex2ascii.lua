local lenHex = arg[1]

-- translate 8 digit hex number into 4 raw bytes
local c = string.format( "[%c%c%c%c]",
				tonumber( lenHex:sub( 1, 2 ), 16 ),
				tonumber( lenHex:sub( 4, 4 ), 16 ),
				tonumber( lenHex:sub( 5, 6 ), 16 ),
				tonumber( lenHex:sub( 7, 8 ), 16 ) )
print( c )
