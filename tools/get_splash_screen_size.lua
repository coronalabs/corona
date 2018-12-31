-- This script sets the current size of the default splash screen in the "_CoronaSetup_$platform.lua" files
-- making sure that the file it's using has been crunched for the appropriate platform
--
-- Usage: get_splash_screen_size.lua {ios|android} {repo_dir} {template_filename} {output_filename}

local lfs = require('lfs')

local progname = "get_splash_screen_size.lua"
local platform= arg[1]
local repo_dir= arg[2]

if ( platform == nil or repo_dir == nil or lfs.attributes(repo_dir, "mode") ~= "directory" ) then
	print("ERROR: ".. progname ..": Usage: ".. progname .." {ios|android} {repo_dir}")
	os.exit(1)
end

local android_sdk = os.getenv("ANDROID_SDK")
if ( android_sdk == nil or android_sdk == "" or lfs.attributes(android_sdk, "mode") ~= "directory" ) then
	print("ERROR: ".. progname ..": \$ANDROID_SDK is not a directory" )
	os.exit(1)
end

local isOSWindows = function()
	return package.config:sub(1,1) == "\\"
end

local getLatestBuildToolsDir = function(android_sdk)
	local build_tools_path = android_sdk .. "/build-tools"
	if ( lfs.attributes(build_tools_path, "mode") ~= "directory" ) then
		print("ERROR: ".. progname ..": build tools path '".. build_tools_path .."' is not a directory")
		os.exit(1)
	end

	local build_tools_dirs = {}
	for pathname in lfs.dir( build_tools_path ) do
		build_tools_dirs[#build_tools_dirs] = pathname
	end

	table.sort(build_tools_dirs)

	-- Return last item, the one with the  highest version number
	return build_tools_dirs[#build_tools_dirs]
end

local getSplashScreenSize = function(platform, repo_dir)
	local null_device = isOSWindows() and "NUL" or "/dev/null"
	local png_file_android = repo_dir .. "/platform/android/sdk/res/drawable/_corona_splash_screen.png"
	local png_file_ios = repo_dir .. "/platform/iphone/_CoronaSplashScreen.png"
	local crunched_splash_image_file_size = 0
	local current_splash_image_file_size = 0

	if ( isOSWindows() ) then
		tmp_dir = os.getenv("TEMP")
	else
		tmp_dir = "/tmp"
	end

	local tmp_png_file_ios = tmp_dir .. "/_CoronaSplashScreen".. os.time() ..".png"
	local tmp_png_file_android = tmp_dir .. "/_corona_splash_screen".. os.time() ..".png"

	if ( platform == "android" ) then

		-- Note this needs to run on Windows so the os.execute() command needs to be carefully considered
		--
		if ( lfs.attributes(png_file_android, "mode") ~= "file" ) then
			print("ERROR: ".. progname ..": cannot find ".. platform .." splash png file '".. png_file_android .."'")
			os.exit(1)
		end

		-- Get the most recent version of the build tools
		local latest_build_tools_dir = getLatestBuildToolsDir(android_sdk)
		local aapt = android_sdk .."/build-tools/".. latest_build_tools_dir .. "/aapt"
		if ( lfs.attributes(aapt, "mode") ~= "file" ) then
			print("ERROR: ".. progname ..": cannot find a usable version of 'aapt' in '".. aapt .."' (ANDROID_SDK: ".. android_sdk ..")")
			os.exit(1)
		end

		-- building compresses the PNG file so we need to use the processed size
		-- print(aapt .." singleCrunch -i '".. png_file_android .."' -o '".. tmp_png_file_android .."' >" .. null_device)
		os.execute(aapt .." singleCrunch -i '".. png_file_android .."' -o '".. tmp_png_file_android .."' >" .. null_device)
		crunched_splash_image_file_size = lfs.attributes(tmp_png_file_android, "size")
		current_splash_image_file_size = lfs.attributes(png_file_android, "size")

	elseif ( platform == "ios" ) then

		if ( isOSWindows() ) then
			print("ERROR: ".. progname ..": cannot process iOS splash screen on Windows")
			os.exit(1)
		end

		-- Note this will only run on macOS so we can be liberal with the shell syntax in the os.execute()
		if ( lfs.attributes(png_file_ios, "mode") ~= "file" ) then
			print("ERROR: ".. progname ..": cannot find ".. platform .." splash png file '".. png_file_ios .."'")
			os.exit(1)
		end

		os.execute("$(xcrun -f copypng) -compress -strip-PNG-text '".. png_file_ios .."' '".. tmp_png_file_ios .."'")
		crunched_splash_image_file_size = lfs.attributes(tmp_png_file_ios, "size")
		current_splash_image_file_size = lfs.attributes(png_file_ios, "size")

	else

		print("ERROR: ".. progname ..": unsupported platform '".. platform .."'")
		os.exit(1)

	end

	if ( crunched_splash_image_file_size ~= current_splash_image_file_size ) then
		print("ERROR: ".. progname ..": ".. platform .." splash image does not appear to be crunched (CRUNCHED_FILE_SIZE ".. crunched_splash_image_file_size .." != CURRENT_FILE_SIZE ".. current_splash_image_file_size)
		os.exit(1)
	else
		print(current_splash_image_file_size)
	end
end

getSplashScreenSize(platform, repo_dir)
