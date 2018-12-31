-- 
-- Creates new AndroidManifest.xml based on incoming manifest template and build.properties file
-- 

local json = require "json"

-------------------------------------------------------------------------------
-- Helper functions
-------------------------------------------------------------------------------

function string.ends(String,End)
	return End=='' or string.sub(String,-string.len(End))==End
end

-------------------------------------------------------------------------------
-- Parse arguments
-------------------------------------------------------------------------------

local function usage( arg0 )
	print( "USAGE: " .. arg0 .. " path/to/dst/output.json [path1/metadata.lua path2/metadata.lua ...]" )
	os.exit( -1 )
end

if arg[1] == '-h' or not string.ends( arg[1], '.json' ) then
	usage( arg[0] )
end

-- Index of first metadata.lua file
local outputFile = arg[1]
local firstInput = 2

-------------------------------------------------------------------------------
-- Example metadata.lua file
-------------------------------------------------------------------------------
--[==[
local metadata =
{
	plugin =
	{
		format = 'jar',
		manifest =
		{
			usesPermissions =
			{
				"android.permission.INTERNET",
				"android.permission.ACCESS_NETWORK_STATE",
			},
			usesFeatures = {},
			mainIntentFilter =
			{
				categories =
				{
					"tv.ouya.intent.category.GAME"
				},
			},
			applicationChildElements =
			{
				-- Array of strings
				[[
<activity android:name="com.google.ads.AdActivity"
          android:configChanges="keyboard|keyboardHidden|orientation|screenLayout|uiMode|screenSize|smallestScreenSize"/>]],
	        },
		},
	},
}

return metadata
--]==]
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-- Merge metadata.lua files
-------------------------------------------------------------------------------

local outputManifest = {}

-- Iterate through metadata.lua file for each plugin
for i=firstInput,#arg do
	local path = arg[i]
	print( path )
	if string.ends( path, 'metadata.lua' ) then
		local chunk = loadfile( path )
		if chunk then
			local metadata = chunk()
			if type( metadata ) == "table" then
				if metadata.plugin and type( metadata.plugin ) == "table" then
					if metadata.plugin.manifest and type( metadata.plugin.manifest ) == "table" then
						table.insert(outputManifest, metadata.plugin.manifest)
					end
				end
			end
		end
	end
end

local outputString = json.encode( outputManifest )
local f = io.open( outputFile, 'w' )
f:write( outputString )
f:close()

print( "Output written to (" .. outputFile .. ")" )
