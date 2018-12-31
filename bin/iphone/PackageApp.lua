-- Assumes pwd is same as this script's directory

local function escapeString( str )
	return "\"" .. str .. "\""
end

local function generateXcent( options )
	local inFilename
	if ( nil ~= string.find( options.signingIdentity, "Developer", 1, true ) ) then
		inFilename = "template.developer.xcent"
	else
		inFilename = "template.xcent"
	end
	print( "Using xcent template: " .. options.binDir.. "/" ..inFilename )
	local inFile = assert( io.open( options.binDir .. "/" .. inFilename, "rb" ) )

	local filename = options.dstFile .. ".xcent"
	local outFile = assert( io.open( filename, "wb" ) )

	local data = inFile:read( "*all" )
	inFile:close()

	local numMatches = 0
	data, numMatches = string.gsub( data, "ARG0", options.appid )
	assert( numMatches == 1 )

	local len = string.len( data )
	local lenHex = string.format( "%08x", len )

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

function generatePlist( options )
	local inFile = assert( io.open( options.binDir .. "/template.plist", "rb" ) )

	local filename = options.dstFile .. ".plist"
	local outFile = assert( io.open( filename, "wb" ) )

	local data = inFile:read( "*all" )
	inFile:close()

	local numMatches = 0
	data, numMatches = string.gsub( data, "BUNDLEDISPLAYNAME", options.bundledisplayname )
	assert( numMatches == 1 )

	local bundleid, numSubs = string.gsub( options.bundleid, " ", "_" )
	if ( numSubs > 0 ) and options.verbose then
		print( "Bundle id contained white space. Using ("..bundleid..") instead" )
	end
	data, numMatches = string.gsub( data, "BUNDLEID", bundleid )
	assert( numMatches == 1 )

	data, numMatches = string.gsub( data, "BUNDLENAME", options.bundlename )
	assert( numMatches == 1 )

	data, numMatches = string.gsub( data, "BUNDLEEXECUTABLE", options.bundleexecutable )
	assert( numMatches == 1 )

	local version = options.bundleversion or "1.0"
	data, numMatches = string.gsub( data, "BUNDLEVERSION", version )
	assert( numMatches == 1 )

	outFile:write(data)
	assert( outFile:close() )	

	os.execute( "plutil -convert binary1 " .. escapeString( filename ) )

	if ( options.verbose ) then
		print( "Created " .. filename );
	end
end

function generateFiles( options )
	generateXcent( options )
	generatePlist( options )
end

function packageApp( options )
	local baseName = options.dstFile;
--	local appBundleFile = options.srcAssets .. "/" .. baseName .. ".app"
	local appBundleFile = options.dstDir .. "/" .. baseName .. ".app"

	local appBundleFileArg = escapeString( options.dstDir .. "/" .. baseName .. ".app" )
	os.execute( "ditto -X " .. escapeString( options.binDir .. "/template.app" ) .. " " .. appBundleFileArg )
	os.execute( "mv " .. escapeString( baseName..".plist" ) .. " " .. escapeString( appBundleFile.."/Info.plist" ) )
	os.execute( "mv " .. escapeString( appBundleFile.."/template" ) .." " .. escapeString( appBundleFile.."/"..baseName ) )

--[[
	if ( options.iconFile ) then
		os.execute( "cp " .. options.iconFile .. " Icon.png" )
		os.execute( "/Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -resolve-src-symlinks Icon.png " .. appBundleFileArg )
		os.execute( "rm -f Icon.png" )
	end
--]]

--	print( "cp \"" .. options.mobileProvision .. "\" " .. appBundleFile .. "/embedded.mobileprovision" )
	os.execute( "cp " .. escapeString(options.mobileProvision) .. " " .. escapeString( appBundleFile.."/embedded.mobileprovision" ) )

	local runScript = "bash "
	if ( options.verbose ) then
--		runScript = runScript .. "-x "
	end

	local binDir = options.binDir

	-- Replace ditto with CopyResources.sh and BuildSources.sh calls
	os.execute( runScript .. binDir .. "/CopyResources.sh release ".. escapeString( options.srcAssets ) .." ".. appBundleFileArg )
	os.execute( runScript .. binDir .. "/BuildSources.sh release ".. escapeString( options.srcAssets ) .." ".. appBundleFileArg )

	-- copy (actually, copy over the files that CopyResources.sh copied over) replacing
	-- any png's with their stripped down versions
	os.execute( "/Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -resolve-src-symlinks " .. escapeString( options.srcAssets ) .. "/*.png " .. appBundleFileArg )

	-- icon.png should be capitalized Icon.png (capital I)
	os.execute( "mv " .. escapeString( appBundleFile.."/icon.png" ) .. " " .. escapeString( appBundleFile.."/Icon.png" ) )

	-- bundle is now ready to be signed
	local entitlements = escapeString( baseName .. ".xcent" )
	print( runScript .. binDir .. "/codesign.sh " .. entitlements .. " " .. appBundleFileArg .. " " .. escapeString( options.signingIdentity ) )
	os.execute( runScript .. binDir .. "/codesign.sh " .. entitlements .. " " .. appBundleFileArg .. " " .. escapeString( options.signingIdentity ) )
	os.execute( "rm -f " .. entitlements )

	os.execute( "chmod 755 " .. appBundleFileArg )
end

function usage( arg0 )
	print( "USAGE" )
	print( "\t" .. arg0 .. " -a bin_dir -s identity [-o appname] [options] provision_file src_dir dst_dir " )
	print( "" )
	print( "DESCRIPTION" )
	print( "\tThe parameter 'src_dir' is the path to the assets used by the app." )
	print( "\tAt a minimum, it must include a file called 'main.lua'" )
	print( "\tThe parameter 'dst_dir' is the folder where the built app will be." )
	print( "\tThe parameter 'provision_file' is the path to the mobile provision" )
	print( "\tfile assigned to the developer." )
	print( "" )
	print( "OPTIONS" )
	print( "\t-q           quiet mode (suppresses output)" )
	print( "\t-i file.png  png file to be used as application icon" )
	print( "" )
	print( "EXAMPLES" )
	print( "\t" .. arg0 .. " -o myapp path/to/assets/dir path/to/file.mobileprovision" )

	error( "" )
end

function parseOptions( argv )
	local result = { verbose = true }

	-- NOTE: argv is 0-based!!!
	local argc = #argv

	-- skip argv[0]
	local i = 1
	if ( argc < 2 ) then
		usage( argv[0] )
	end

	while i <= argc do
		local arg = argv[i]
--print( "argv["..i.."] = "..arg )
		local key = nil
		local switchString = string.sub( arg, 1, 2 )
        if ( switchString == "-a" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]
			result.binDir = argv[i]
		elseif ( switchString == "-s" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]
			result.signingIdentity = value
		elseif ( switchString == "-v" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]
			result.bundleversion = value
		elseif ( switchString == "-o" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]
			local name = value
			--[[
			result.appid = value
			result.bundleid = string.match( value, "[^%.]+%.(.*)" )

			local name = string.match( result.bundleid, "^.*%.([^%.]+)$" )
			if ( nil == name ) then
				name = result.bundleid
			end
			--]]
			result.bundledisplayname = name

			local nameWithoutSpaces = string.gsub( name, " ", "" )
			result.bundlename = nameWithoutSpaces
			result.bundleexecutable = nameWithoutSpaces
			result.dstFile = nameWithoutSpaces
		elseif ( switchString == "-q" ) then
			result.verbose = false
		elseif ( switchString == "-i" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]

			result.iconFile = value;
			assert( string.match( value, "^.*%.([^%.]+)$" ) == "png", "Icon file ("..tostring(value)..") must be a png" )
		elseif ( nil == result.mobileProvision ) then
			-- print( "using provision: ".. argv[i] )
			result.mobileProvision = argv[i]
		elseif ( nil == result.srcAssets ) then
			-- print( "using src assets: ".. argv[i] )
			result.srcAssets = argv[i]
		elseif ( nil == result.dstDir ) then
			-- print( "using dst dir: ".. argv[i] )
			result.dstDir = argv[i]
		else
			-- print( "error using: " .. arg )
			usage( argv[0] )
		end

		i = i + 1
	end

	if ( not result.bundlename ) then
		usage( argv[0] )
		--[[
		local name = "ratatouille"	
		result.bundlename = name
		result.bundledisplayname = name
		result.bundleexecutable = name
		result.dstFile = name
		--]]
	end

	if ( result.mobileProvision ) then
		-- Extract app id from provision file
		local inFile = io.open( result.mobileProvision, "rb" )
		local data = inFile:read( "*all" )
		inFile:close()

		local patternPrefix =
			"<key>application%-identifier</key>%s*<string>([%w%.]*)"
		local patternWithWildcard = patternPrefix .. "%.%*</string>"
		local patternWithoutWildcard = patternPrefix .. "</string>"
		local appid = string.match( data, patternWithWildcard )
		local usesWildcard = true
		if ( not appid ) then
			usesWildcard = false
			appid = string.match( data, patternWithoutWildcard )
		end

		if ( not appid ) then
			local pattern = patternPrefix .. "</string>"
			local appid= string.match( data, pattern )
			assert( appid )
		end
		value = appid

		-- only append if wildcard was used
		if usesWildcard then
			value = value .. "." .. result.bundlename
		end

		result.appid = value
		result.bundleid = string.match( value, "[^%.]+%.(.*)" )

--[[
		-- Extract app id from provision file
		local inFile = io.open( result.mobileProvision, "rb" )
		local data = inFile:read( "*all" )
		inFile:close()

		local patternPrefix =
			"<key>application%-identifier</key>%s*<string>([%w%.]*)"
		local patternWithWildcard = patternPrefix .. "%.%*</string>"
		local appid = string.match( data, patternWithWildcard )
		if ( not appid ) then
			local pattern = patternPrefix .. "</string>"
			local appid= string.match( data, pattern )
			assert( appid )
		end
		value = appid .. "." .. result.bundlename
		result.appid = value
		result.bundleid = string.match( value, "[^%.]+%.(.*)" )
--]]
	else
		print( "Please specify mobile provision file" )
		usage( argv[0] )
	end

	if ( result.verbose ) then
		print( "Using displayname: " .. result.bundledisplayname )
		print( "Using bundlename " .. result.bundlename )
		print( "Using bundleid: " .. result.bundleid )
		print( "Using appid: " .. result.appid )
		print( "Using icon file: " .. result.iconFile )
		print( "Using source assets dir: " .. result.srcAssets )
		print( "Using dst dir: " .. result.dstDir )
		print( "Using bin dir: " .. result.binDir )
		print( "Using mobile provision: " .. result.mobileProvision )
		print( "Using signing identity: " .. result.signingIdentity )
		print( "Using bundle version: " .. result.bundleversion )
	end

	return result
end

local options = parseOptions( arg )

generateFiles( options )
packageApp( options )
