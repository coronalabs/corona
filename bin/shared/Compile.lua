#!/usr/bin/env lua

--------------------------------------------------------------------------------
-- NOTE: this script is functionally equivalent to bin/mac/CompileScripts.sh
--       If you are changing this one, you probably want to change the other.
--------------------------------------------------------------------------------


local args = {...}

--------------------------------------------------------------------------------
-- Functions
--------------------------------------------------------------------------------

local function fileExists( path )
	local f = io.open( path, "r" )
	if f then
		f:close()
		return true
	end
	return false
end

local function checkError( passesCheck, msg )
	if not passesCheck then
		if not msg then msg = "Exiting due to errors (above)." end
		print( "ERROR: ", msg )
		-- os.exit( -1 )
	end
end

local function execute( ... )
	local args = { ... }
	local cmd = table.concat( args, ' ' )

	local f = io.popen( cmd )
	local result = f:read( "*a" )
	f:close()
	return result
end

local function os_execute( ... )
	local args = { ... }
	local cmd = table.concat( args, ' ' )
	local rc = os.execute( cmd )
	print("execute: ", cmd, "retcode: ", rc)
	return rc
end

local function touch( path )
	io.open( path, "w" ):close()
end

local function escape( param )
	return '"' .. param .. '"'
end

--------------------------------------------------------------------------------
-- Arguments
--------------------------------------------------------------------------------

local PLATFORM
local CORONA_ROOT

if args then
	PLATFORM = args[1]
	CORONA_ROOT = args[2]
end


--[[
-- Platform of caller
local PLATFORM

if os.getenv("WINDIR") then
	PLATFORM = "win32"
else
	local ostype = os.getenv( "OSTYPE" )
print( "OSTYPE,", ostype)
	if nil ~= string.find( ostype:lower(), "darwin", 1, true ) then
		PLATFORM = "mac"
	end
end

checkError( PLATFORM, "Could not determine platform." )
--]]

-- Location of CoronaEnterprise directory
checkError( PLATFORM, "Could not determine the OS this script is running on." )
checkError( CORONA_ROOT, "Could not determine path to 'Native' directory." )

local BIN_DIR = CORONA_ROOT .. "/Corona/" .. PLATFORM .. "/bin"
if PLATFORM == "win" then
	BIN_DIR = CORONA_ROOT .. "\\Corona\\" .. PLATFORM .. "\\bin"
end

local BUILDER_PATH = BIN_DIR .. "/CoronaBuilder.app/Contents/MacOS/CoronaBuilder"
if PLATFORM == "win" then
	BUILDER_PATH = BIN_DIR .. "\\CoronaBuilder.exe"
end

checkError( fileExists( BUILDER_PATH ), "Required CoronaBuilder tool not found at (" .. BUILDER_PATH .. ")" )

--------------------------------------------------------------------------------
-- Environment variables
--------------------------------------------------------------------------------

-- [Required]
TARGET_PLATFORM = os.getenv( "TARGET_PLATFORM" )
checkError( TARGET_PLATFORM, "Missing target device platform." )

-- [Required] Optional on iOS/Mac
PROJECT_DIR = os.getenv( "PROJECT_DIR" )
checkError( PROJECT_DIR, "Missing environment variable 'PROJECT_DIR'." )

-- [Required] Optional on iOS/Mac
CONFIGURATION = os.getenv( "CONFIGURATION" )
if not CONFIGURATION then
	CONFIGURATION = "debug"
	print( "WARNING: Defaulting to 'debug' configuration." )
end
checkError( CONFIGURATION )

-- [Optional] Assuming you're okay with defaults
CORONA_ASSETS_DIR = os.getenv( "CORONA_ASSETS_DIR" )
if not CORONA_ASSETS_DIR then
	CORONA_ASSETS_DIR = PROJECT_DIR .. "/../Corona"
	if PLATFORM == "win" then
	CORONA_ASSETS_DIR = PROJECT_DIR .. "\\..\\Corona"
	end
end

-- [Required] Optional on iOS/Mac
CORONA_TARGET_RESOURCES_DIR = os.getenv( "CORONA_TARGET_RESOURCES_DIR" )
if not CORONA_TARGET_RESOURCES_DIR then
	if "ios" == TARGET_PLATFORM or "mac" == TARGET_PLATFORM then
		-- $TARGET_BUILD_DIR/$UNLOCALIZED_RESOURCES_FOLDER_PATH
		TARGET_BUILD_DIR = os.getenv( "TARGET_BUILD_DIR" )
		UNLOCALIZED_RESOURCES_FOLDER_PATH = os.getenv( "UNLOCALIZED_RESOURCES_FOLDER_PATH" )
		if TARGET_BUILD_DIR and UNLOCALIZED_RESOURCES_FOLDER_PATH then
			CORONA_TARGET_RESOURCES_DIR = TARGET_BUILD_DIR .. '/' .. UNLOCALIZED_RESOURCES_FOLDER_PATH
		end
	end
end
checkError( CORONA_TARGET_RESOURCES_DIR, "Missing target build resources directory." )

-- [Required] Optional on iOS/Mac
CORONA_TARGET_EXECUTABLE_DIR = os.getenv( "CORONA_TARGET_EXECUTABLE_DIR" )
if not CORONA_TARGET_EXECUTABLE_DIR then
	if "ios" == TARGET_PLATFORM or "mac" == TARGET_PLATFORM then
		-- $TARGET_BUILD_DIR/$EXECUTABLE_FOLDER_PATH
		TARGET_BUILD_DIR = os.getenv( "TARGET_BUILD_DIR" )
		EXECUTABLE_FOLDER_PATH = os.getenv( "EXECUTABLE_FOLDER_PATH" )
		if TARGET_BUILD_DIR and EXECUTABLE_FOLDER_PATH then
			CORONA_TARGET_EXECUTABLE_DIR = TARGET_BUILD_DIR .. '/' .. EXECUTABLE_FOLDER_PATH
		end
	end
end
checkError( CORONA_TARGET_EXECUTABLE_DIR, "Missing target build executable directory." )

-- [Required] Optional on iOS/Mac
BUNDLE_ID = os.getenv( "BUNDLE_ID" )
if not BUNDLE_ID then
	if "ios" == TARGET_PLATFORM or "mac" == TARGET_PLATFORM then
		-- 
		local cmd = '/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" "$TARGET_BUILD_DIR/$INFOPLIST_PATH"'
		BUNDLE_ID = execute( cmd )
    end
end

-- Remove trailing whitespace (including newline)
if BUNDLE_ID then
	BUNDLE_ID = BUNDLE_ID:gsub( '%s', "" )
end

-- [Required] Optional on iOS/Mac
local APP_ID = os.getenv( "APP_ID" )

if not APP_ID then
	if "ios" == TARGET_PLATFORM or "mac" == TARGET_PLATFORM then
		local cmd = '/usr/libexec/PlistBuddy -c "Print :CoronaAppId" "$TARGET_BUILD_DIR/$INFOPLIST_PATH" 2>/dev/null'
		APP_ID = execute( cmd )
	end
end

-- Assign if all else fails
if "" == APP_ID or not APP_ID then
	if BUNDLE_ID then
		APP_ID = BUNDLE_ID
	end
end

-- Remove trailing whitespace (including newline)
if APP_ID then
	APP_ID = APP_ID:gsub( '%s', "" )
end

-- Ignore the given Bundle ID and App ID for Android Enterprise builds.
-- This is because most enterprise users do not know to set the package name in the "build.xml" file.
-- Corona's launchpad analytics will use the Android app's package name at runtime instead.
if "android" == TARGET_PLATFORM then
	BUNDLE_ID = nil
	APP_ID = nil
end

local RESOURCE_EXT = ".car"
if "ios" == TARGET_PLATFORM then
	RESOURCE_EXT = ".corona-archive"
end

-- [Optional]
USER_ID = os.getenv( "USER_ID" )
if not USER_ID then
	USER_ID = '345'
end

if BUNDLE_ID then
	print( "Using Bundle Id: (" .. BUNDLE_ID .. ")" )
end
if APP_ID then
	print( "Using AppId: (" .. APP_ID .. ")" )
end

--------------------------------------------------------------------------------
-- Main
--------------------------------------------------------------------------------

-- Copy project resources, compile Lua scripts, etc
-- "$BIN_DIR/CopyResources.sh" $CONFIGURATION "$CORONA_ASSETS_DIR" "$CORONA_TARGET_RESOURCES_DIR" "$BIN_DIR"

local CopyResourcesScript = escape( BIN_DIR .. "/CopyResources.sh" )
if PLATFORM == "win" then
	CopyResourcesScript = 'cmd /Q/C "' 
		.. escape( BIN_DIR .. "\\CopyResources.bat" ) .. ' '
		.. CONFIGURATION .. ' '
		.. escape( CORONA_ASSETS_DIR ) .. ' '
		.. escape( CORONA_TARGET_RESOURCES_DIR ) .. ' '
		.. escape( BIN_DIR ) 
		.. '"'

	local result = os_execute(CopyResourcesScript)
	checkError( 0 == tonumber( result ) )
else
	local result = os_execute(
		CopyResourcesScript,
		CONFIGURATION,
		escape( CORONA_ASSETS_DIR ),
		escape( CORONA_TARGET_RESOURCES_DIR ),
		escape( BIN_DIR ) )
	checkError( 0 == tonumber( result ) )
end

-- Config.lua
TMP_FILE = os.tmpname()
if PLATFORM == "win" then
	TMP_FILE = os.getenv('TMP')..TMP_FILE
end

CONFIG_META = TMP_FILE -- "$TARGET_BUILD_DIR/config.metadata.lua"
local pl = require "pl"
local pl_dir = require "pl.dir"
local pl_path = require "pl.path"
local pl_file = require "pl.file"
PROJECT_NAME = pl_path.basename( PROJECT_DIR ) -- `basename "$PROJECT_DIR"`

MODE="distribution"
if nil ~= string.find( CONFIGURATION:lower(), "Debug", 1, true ) then
	MODE="developer"
end

print("Creating application.metadata ...")
local config_meta_format = [[
-- Ensure existence of "application" table
if not application or type( application ) ~= "table" then
    application = {}
end

application.metadata = {
    appName = "%s",
    appVersion = "1.0",
    %s
    %s
    userId = "%s",
    subscription = "enterprise",
    mode = "%s",
}
]]

local bundleIdTableEntry = ""
if BUNDLE_ID then
	bundleIdTableEntry = 'appPackageId = "' .. tostring(BUNDLE_ID) .. '",'
end
local appIdTableEntry = ""
if APP_ID then
	appIdTableEntry = 'appId = "' .. tostring(APP_ID) .. '",'
end
local config_meta = string.format( config_meta_format, PROJECT_NAME, bundleIdTableEntry, appIdTableEntry, USER_ID, MODE )

local config_meta_file = io.open( CONFIG_META, "w+" )
print("CONFIG_META=", CONFIG_META, "config_meta_file=", config_meta_file, config_meta)

config_meta_file:write( config_meta )
config_meta_file:close()


-- Concatenate config.metadata.lua into config.lu overwriting original config.lu

CONFIG_DST = CORONA_TARGET_EXECUTABLE_DIR .. "/config.lu"
CONFIG_SRC = CORONA_TARGET_EXECUTABLE_DIR .. "/config.orig.lu"

if not fileExists( CONFIG_DST ) then
	touch( CONFIG_SRC )
else
	pl_file.move( CONFIG_DST, CONFIG_SRC )
end

-- "$LUAC" -s -o "$CONFIG_DST" "$CONFIG_SRC" "$CONFIG_META"
if PLATFORM == "win" then
	LUAC = 'cmd /Q/C "' 
		.. escape( BIN_DIR .. "\\luac.exe" ) .. ' '
		.. '-s -o' .. ' '
		.. escape( CONFIG_DST ) .. ' '
		.. escape( CONFIG_SRC ) .. ' '
		.. escape( CONFIG_META ) 
		.. '"'

	local result = os_execute(LUAC)

	checkError( 0 == result, "Failed to compile Lua files." )
else
	LUAC = BIN_DIR .. "/luac"
	local result = os_execute( 
		escape(LUAC),
		'-s -o',
		escape( CONFIG_DST ),
		escape( CONFIG_SRC ),
		escape( CONFIG_META ) )
	checkError( 0 == result, "Failed to compile Lua files." )
end

pl_file.delete( CONFIG_SRC )
pl_file.delete( CONFIG_META )

-- Create resource.{car,corona-archive} from *.lu
-- "$BUILDER_PATH" car "$CORONA_TARGET_EXECUTABLE_DIR/resource.corona-archive" "$CORONA_TARGET_EXECUTABLE_DIR"/*.lu

-- If Mac native we need to fix the compile path
IS_MAC_NATIVE = os.getenv( "IS_MAC_NATIVE" )
if IS_MAC_NATIVE then 
	local newPath = string.sub(CORONA_TARGET_EXECUTABLE_DIR, 1, #CORONA_TARGET_EXECUTABLE_DIR - 6) .. '/Resources'
	os_execute("mv ", CORONA_TARGET_EXECUTABLE_DIR.."/config.lu", newPath, " 2>/dev/null")	
	CORONA_TARGET_EXECUTABLE_DIR = newPath
end

print( 'Archiving ' .. CORONA_TARGET_EXECUTABLE_DIR ..'/*.lu into ' .. CORONA_TARGET_EXECUTABLE_DIR .. '/resource' .. RESOURCE_EXT )
if PLATFORM == "win" then
	local result = os_execute(
		'cmd /Q/C "dir /s/b',
		escape( CORONA_TARGET_EXECUTABLE_DIR .. "\\*.lu"),
		'|',
		escape( BUILDER_PATH ),
		'car',
		'-f -', -- read file list from stdin
		escape( CORONA_TARGET_EXECUTABLE_DIR .. "/resource" .. RESOURCE_EXT ),
		'"')
	checkError( 0 == result, "Failed to generate resource" .. RESOURCE_EXT )
else
	local result = os_execute( 
	    'find -H',
		escape( CORONA_TARGET_EXECUTABLE_DIR ),
		'-maxdepth 1 -name "*.lu" -print', -- avoid cmd line length issues with "*.lu" 
		'|',
		escape( BUILDER_PATH ),
		'car',
		'-f -', -- read file list from stdin
		escape( CORONA_TARGET_EXECUTABLE_DIR .. "/resource" .. RESOURCE_EXT ) )
	checkError( 0 == result, "Failed to generate resource" .. RESOURCE_EXT )
end


-- rm -f "$CORONA_TARGET_EXECUTABLE_DIR"/*.lu
if PLATFORM == "win" then
	print( 'Removing: ' .. CORONA_TARGET_EXECUTABLE_DIR .. '\\*.lu' )
	local result = os_execute('del', CORONA_TARGET_EXECUTABLE_DIR .. "\\*.lu")
else
	print( 'Removing: ' .. CORONA_TARGET_EXECUTABLE_DIR .. '/*.lu' )
	local result = os_execute( 
    'find -H',
	escape( CORONA_TARGET_EXECUTABLE_DIR ),
	'-maxdepth 1 -name "*.lu" -exec rm -f {} \\;')
end

