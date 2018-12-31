-- Assumes pwd is same as this script's directory

function modifyPlist( options )
	local baseName = options.dstFile;
	local appBundleFile = options.dstDir .. "/" .. baseName .. ".app"
print( "modifying plist: " .. appBundleFile .. " using name: " .. baseName )
	os.execute( "defaults write "..appBundleFile.."/Contents/Info CFBundleExecutable "..options.bundlename )
	os.execute( "defaults write "..appBundleFile.."/Contents/Info CFBundleIdentifier "..options.appid )
	os.execute( "defaults write "..appBundleFile.."/Contents/Info CFBundleName ".. "\"Corona Simulator\"" )
--	os.execute( "defaults write "..appBundleFile.."/Contents/Info CFBundleIconFile "..options.bundlename..".icns" )
end

function packageApp( options )
	local baseName = options.dstFile;
	local appBundleFile = options.dstDir .. "/" .. baseName .. ".app"
	os.execute( "ditto -X " .. options.binDir .. "/template.app " .. appBundleFile )
	os.execute( "mv " .. appBundleFile .. "/Contents/MacOS/template " .. appBundleFile .. "/Contents/MacOS/" .. baseName )
    modifyPlist( options )

--[[
	if ( options.iconFile ) then
		local srcDir = options.srcAssets
		local dstDir = options.dstDir
		os.execute( "sips -z 48 48 s -s format tiff " .. srcDir .. "/Icon.png --out " .. dstDir .. "/Icon.tiff" )
		os.execute( "tiff2icns -noLarge " .. dstDir .. "/Icon.tiff" )
		os.execute( "rm -f " .. dstDir .. "/Icon.tiff" )
	end
--]]

	local runScript = "bash "
	if ( options.verbose ) then
--		runScript = runScript .. "-x "
	end

	local dstDir = appBundleFile .. "/Contents/Resources"
	local binDir = options.binDir

	-- Replace ditto with CopyResources.sh and BuildSources.sh calls
	os.execute( runScript .. binDir .. "/CopyResources.sh release "..options.srcAssets.." "..dstDir .. " --preserve" )
	os.execute( runScript .. binDir .. "/BuildSources.sh release "..options.srcAssets.." "..dstDir )

	os.execute(
		"/usr/bin/codesign -f -s rttplayer " .. appBundleFile )

	os.execute( "chmod 755 " .. appBundleFile )
end

function usage( arg0 )
	print( "USAGE" )
	print( "\t" .. arg0 .. " -a bin_dir -d appid.bundleid [options] src_dir dst_dir " )
	print( "" )
	print( "DESCRIPTION" )
	print( "\tThe parameter 'src_dir' is the path to the assets used by the app." )
	print( "\tAt a minimum, it must include a file called 'main.lua'" )
	print( "\tThe parameter 'dst_dir' is the folder where the built app will be." )
	print( "\tfile assigned to the developer." )
	print( "" )
	print( "OPTIONS" )
	print( "\t-q           quiet mode (suppresses output)" )
	print( "\t-i file.png  png file to be used as application icon" )
	print( "" )
	print( "EXAMPLES" )
	print( "\t" .. arg0 .. " -a ../bin/mac -d com.mycompany.nameofapp path/to/assets/dir" )

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
		local key = nil
		local switchString = string.sub( arg, 1, 2 )
		if ( switchString == "-a" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]
			result.binDir = argv[i]
		elseif ( switchString == "-d" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]
			result.appid = value
			result.bundleid = string.match( value, "[^%.]+%.(.*)" )

			local name = string.match( result.bundleid, "^.*%.([^%.]+)$" )
			if ( nil == name ) then
				name = result.bundleid
			end
			result.bundlename = name
			result.bundledisplayname = name
			result.bundleexecutable = name
			result.dstFile = name
		elseif ( switchString == "-q" ) then
			result.verbose = false
		elseif ( switchString == "-i" ) then
			assert( (i + 1) <= argc )
			i = i + 1
			local value = argv[i]

			result.iconFile = value;
			assert( string.match( value, "^.*%.([^%.]+)$" ) == "png" )
		elseif ( nil == result.srcAssets ) then
			result.srcAssets = argv[i]
		elseif ( nil == result.dstDir ) then
			result.dstDir = argv[i]
		else
			usage( argv[0] )
		end

		i = i + 1
	end

	if ( result.verbose ) then
		print( "Using displayname: " .. result.bundledisplayname )
		print( "Using bundlename " .. result.bundlename )
		print( "Using bundleid: " .. result.bundleid )
		print( "Using appid: " .. result.appid )
		print( "Using icon file: " .. tostring( result.iconFile ) )
		print( "Using source assets dir: " .. result.srcAssets )
		print( "Using dst dir: " .. result.dstDir )
		print( "Using bin dir: " .. result.binDir )
	end

	return result
end

local options = parseOptions( arg )

packageApp( options )
