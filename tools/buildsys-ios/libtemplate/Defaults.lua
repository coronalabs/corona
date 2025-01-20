local M = {}

-------------------------------------------------------------------------------
--[[

'Defaults' contains all default values

These are organized into 2 properties:

* `Defaults.tools` This contains paths to all the relevant tools. Two related methods:
	+ `Defaults:initialize()` initialized the `tools` property
	+ `Defaults:setSdkType()` allows you to toggle between `"iphoneos"` and `"iphonesimulator"`.
		- This corresponds to the SDKs listed by calling `xcodebuild -showsdks` on the command-line (minus the version)
* `Defaults.options` 
	+ This contains all the options.
	+ If there are changes to the iOS Xcode project (e.g. new frameworks), 
	    you should make corresponding changes here.

--]]
-------------------------------------------------------------------------------

local lfs = require "lfs"

local BuilderUtils = require "BuilderUtils"
local checkError = BuilderUtils.checkError
local execute = BuilderUtils.execute
local xcrun = BuilderUtils.xcrun
local tableDuplicate = BuilderUtils.tableDuplicate

-------------------------------------------------------------------------------

-- Lowest version of iOS that Corona supports
local MIN_VERSION_DEFAULT = "8.0"
local MIN_VERSION_NEW = "11.0"

-- ============================================================================
-- Defaults.tools
-- ============================================================================
local legacySlices, modernSlices

function M:initialize()
	local status, base = checkError( execute( "xcode-select -print-path" ) )

	assert( 0 == status, "ERROR: xcode-select returned with status (" .. status .. ")" )
	assert( nil ~= base, "ERROR: xcode-select failed to find the base path(" .. tostring( base ) .. ")" )

	base = base or "/Applications/Xcode.app/Contents/Developer"

	self.tools =
	{
		-- "/Applications/Xcode.app/Contents/Developer"
		base = base,

		bin = base .. "/Toolchains/XcodeDefault.xctoolchain/usr/bin/",
	}


	-- Default paths in case xcrun fails
	---------------------------------------------------------------------------

	-- "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin:/Applications/Xcode.app/Contents/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
	self.tools.PATH = base .. "/Platforms/iPhoneOS.platform/Developer/usr/bin/:" .. base .. "/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"

	-- "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
	self.tools.compiler = base .. "/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"

	-- "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip"
	self.tools.strip = base .. "/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip"

	-- "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/dsymutil"
	self.tools.dsymutil = base .. "/Toolchains/XcodeDefault.xctoolchain/usr/bin/dsymutil"

	self.tools.lipo = base .. "/Toolchains/XcodeDefault.xctoolchain/usr/bin/lipo"

	---------------------------------------------------------------------------

	-- Use xcrun to find actual paths dynamically
	self:setSdkType()
end

function M:setSdkType( sdkType, minVersion )
	sdkType = sdkType or self.options.sdkType
	self.options.sdkType = sdkType -- update sdktype

	minVersion = minVersion or self.options.minVersion

	local sdkVersion = xcrun( sdkType, "--show-sdk-version" )
	assert( sdkVersion, "ERROR: Could not find iPhone SDK Version" )
	sdkVersion = tonumber(string.match(sdkVersion, '%d+%.?%d*'))
	assert( sdkVersion, "ERROR: Cannot convert iPhone SDK Version:", sdkVersion )

	if sdkVersion >= 16.4 and tonumber(minVersion) < tonumber(MIN_VERSION_NEW) then
		minVersion = MIN_VERSION_NEW
		print("Forcing minVersion to 11.0")
	end
	self.options.minVersion = minVersion

	if tonumber(minVersion) >= 10 or sdkVersion >= 16 then
		modernSlices()
	else
		legacySlices()
	end

	-- Update flags that depend on minVersion
	self:updateFlags( minVersion )

	-- sdkPath
	local sdkPath = xcrun( sdkType, "--show-sdk-path" )
	assert( sdkPath, "ERROR: Could not find iPhone SDK" )
	self.tools.sdkPath = sdkPath

	self:updateOptions( sdkPath )

	-- PATH
	local platformPath = xcrun( sdkType, "--show-sdk-platform-path" )
	if platformPath then
		self.tools.PATH = platformPath .. "/Developer/usr/bin/:" .. self.tools.base .. "/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
	end

	-- Linker Environment
	local linkerEnvArray =
	{
		'unset LIBRARY_PATH; ',
		'export IPHONEOS_DEPLOYMENT_TARGET=' .. minVersion .. '; ',
		'export PATH="' .. self.tools.PATH .. '";', -- note: trailing semicolon
	}
	self.tools.linkerEnv = table.concat( linkerEnvArray )

	-- clang++, strip, dsymutil, lipo
	local function findAndSetTool( toolName, key )
		key = key or toolName -- if nil key, then use toolName as the key

		local path = xcrun( sdkType, "--find " .. toolName )

		if path then
			self.tools[key] = path
		else
			print( "WARNING: xcrun failed to find tool (" .. toolName .. "), sticking with default (" .. tostring( self.tools[key] ) .. ")" )
		end
	end

	findAndSetTool( "clang++", "compiler" )
	findAndSetTool( "strip" )
	findAndSetTool( "dsymutil" )
	findAndSetTool( "lipo" )
end

-- ============================================================================
-- Defaults.options
-- ============================================================================

local options = {}
M.options = options

options.minVersion = MIN_VERSION_DEFAULT

-- This is called once by M:initialize() => M:setSdkType()
function M:updateFlags( minVersion )
	assert( nil ~= minVersion )

	self.options.flags = {
		'-ObjC',
		'-all_load',
		'-ld_classic',
		'-fobjc-link-runtime',
		'-miphoneos-version-min=' .. minVersion,
		'-std=c++11',
		'-stdlib=libc++',
		'-Xlinker -rpath -Xlinker /usr/lib/swift -Xlinker -rpath -Xlinker @executable_path/Frameworks',
	}
end

local TOKEN_sdkPath = '{{IPHONE_SDK}}'
options.flagsSlicesTemplate = {
	i386 = {
		'-arch',
		'i386',
		'-isysroot',
		TOKEN_sdkPath,
	},
	x86_64 = {
		'-arch',
		'x86_64',
		'-isysroot',
		TOKEN_sdkPath,
	},
	armv7 = {
		'-stdlib=libstdc++',
		'-arch',
		'armv7',
		'-isysroot',
		TOKEN_sdkPath,
	},
	arm64 = {
		'-fexceptions',	-- by default, arm64 does not generate stack unwind info
		'-stdlib=libstdc++',
		'-arch',
		'arm64',
		'-isysroot',
		TOKEN_sdkPath,
	},
}

function M:updateOptions( sdkPath )
	local flagsSlices = tableDuplicate( self.options.flagsSlicesTemplate )
	for slice,params in pairs(flagsSlices) do
		for i=1,#params do
			local value = params[i]

			-- Replace token with correct path
			if TOKEN_sdkPath == value then
				params[i] = '"'.. sdkPath ..'"'
			end
		end
	end

	self.options.flagsSlices = flagsSlices
	self.options.flagsArchitecture = flagsSlices[options.architecture]
end


--'-F/Users/wluh/Library/Developer/Xcode/DerivedData/ratatouille-fsolxbgsiczraldzhktzzkjjdowc/Build/Products/Release-iphoneos',
--'-F/Volumes/rttmain/main/platform/iphone/../../plugins/sdk-facebook/ios',

-- The set of slices organized by SDK type
legacySlices = function ()
	options.archSlices = {
		iphonesimulator = {
			"i386",
			"x86_64",
		},
		iphoneos = {
			"armv7",
			"arm64",
		},
	}
	options.architecture = "armv7" -- default
end

modernSlices = function ()
	options.archSlices = {
		iphonesimulator = {
			"x86_64",
		},
		iphoneos = {
			"arm64",
		},
	}
	options.architecture = "arm64" -- default
end

legacySlices()
options.sdkType = "iphoneos" -- default (see keys in options.archSlices for other values)

-- TODO: We are removing these frameworks from the core list
-- so we should check that respective plugins have these as required
-- in the metadata: 
-- 
-- Accounts.framework
-- AdSupport.framework
-- AddressBook.framework
-- AddressBookUI.framework
-- Accounts.framework
-- Social.framework
-- iAds.framework
-- 
options.frameworks = {
	'AudioToolbox',
	'AVFoundation',
	'CFNetwork',
	'CoreGraphics',
	'CoreLocation',
	'CoreMedia',
	'CoreMotion',
	'CoreVideo',
	'CoreText',
	'GameKit',
	'GLKit',
	'ImageIO',
	'MapKit',
	'MediaPlayer',
	'MessageUI',
	'MobileCoreServices',
	'OpenAL',
	'QuartzCore',
	'Security',
	'StoreKit',
	'SystemConfiguration',
	'UIKit',
	'WebKit',
}

options.frameworksOptional = {
	'GameController',
	'AssetsLibrary',
	'Photos',
	'Foundation', -- NOTE: This is optional b/c libFuseAds.a references an iOS7 API
	'Twitter',
}

options.staticLibs = {
	'objc',
	'sqlite3',
	'template',
}

options.librarySearchPaths = {
	'.',
}

return M
