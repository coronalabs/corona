function usage( arg0 )
	print( "USAGE" )
	print( "\t" .. arg0 .. " [-c compiler_path] [-Olevel] [-g] [-o outfile] lua_infile" )
	print( "" )
	print( "SYNOPSIS" )
	print( "\tCreates Loadable Unit (.lu) files" )
	print( "" )
	print( "DESCRIPTION" )
	print( "\t-c compiler_path" )
	print( "\t\tSpecifies the directory path containing the luac compiler." )
	print( "\t\tDefault is the same dir as 'lua_infile'" )
	print( "\t-g" )
	print( "\t\tProduces debugging information" )
	print( "\t-Olevel" )
	print( "\t\tlevel can be either 'Debug' or 'Release'" )
	print( "\t-o outfile" )
	print( "\t\tPlace output in file 'outfile'. Default is to create a .lu file" )
	print( "" )
	print( "OPTIONS" )
	print( "" )
	print( "EXAMPLES" )
	print( "\t" .. arg0 .. " mac Debug /path/to/main.lua /path/to/MyApp.app/main.ro" )

	error( "" )
end

function parseOptions( argv )
	local result = { verbose = false }

	local argc = #argv

	if ( argc < 2 ) then
        print("rcc: not enough arguments ("..tostring(argv[1])..")")
		usage( argv[0] )
	end

	-- NOTE: argv is 0-based!!!
	-- skip i=0 b/c that's just argv[0]
	local i = 1
	while ( i <= argc ) do
		local arg = argv[i]
		local key = nil

		--print( "argv["..i.." of "..argc.."] = "..arg )
		local switchString = string.sub( arg, 1, 2 )
		if ( switchString == "-c" ) then
			key = "compilerDir"
		elseif ( switchString == "-O" ) then
			result.debug = ( string.upper( string.sub( arg, 3, -1 ) ) == "DEBUG" )
		elseif ( switchString == "-g" ) then
			result.debug = true
		elseif ( switchString == "-o" ) then
			key = "outputPath"
		else
			--print( "else:"..arg)
			local f = io.open( arg, "r" )
			if ( f ) then
				result.inputPath = arg
				io.close( f )
			else
                print("rcc: failed to open file '"..tostring(arg).."'")
				usage( argv[0] )
			end
		end

		-- key was set which means the next token contains the value
		if ( key ) then
			i = i + 1
			if ( i < argc ) then
				result[key] = argv[i]
			else
                print("rcc: ran out of arguments ("..tostring(key)..")")
				usage( argv[0] )
			end
		end

		i = i + 1
	end
--	result.compilerDir = argv[1]
--	result.debug = ( string.upper( argv[2] ) == "DEBUG" )
--	result.inputPath = argv[3]
--	result.outputPath = argv[4]

	assert( result.compilerDir ~= nil and type( result.compilerDir ) == "string" )
	assert( result.inputPath ~= nil and type( result.inputPath ) == "string" )
	assert( result.outputPath ~= nil and type( result.outputPath ) == "string" )

	if ( result.verbose ) then
		for i,v in ipairs( arg ) do
			print( "argv[" .. i .. "] = " .. v );
		end

		print( "Using compiler dir: " .. result.compilerDir )
		print( "Using debug " .. tostring( result.debug ) )
		print( "Using input file: " .. result.inputPath )
		print( "Using output file: " .. result.outputPath )
	end

	return result
end

local options = parseOptions( arg )
local debugOptions = ( options.debug and "" ) or "-s"
local commandLine = '"' .. options.compilerDir .. '/luac" ' .. debugOptions .. ' -o "' .. options.outputPath .. '" "' .. options.inputPath .. '"'
if package.config:sub(1,1) == "\\" then -- are we on Windows?
	-- The os.execute() function calls the C language's system() function, where in Windows
	-- it requires the entire command line string to be surrounded by double quotes.
	commandLine = '"' .. commandLine .. '"'
end
local command_result = os.execute( commandLine )
if command_result == 0 then
	os.exit( 0 )
else
	os.exit( 1 )
end
