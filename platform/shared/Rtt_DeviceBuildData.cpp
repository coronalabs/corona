//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_DeviceBuildData.h"

#include "Rtt_LuaFrameworks.h"
#include "Rtt_Lua.h"
#include "Rtt_FileSystem.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kPluginsMetadataKey[] = __FILE__ "-plugins";
static const char kSplashScreenMetadataKey[] = __FILE__ "-splashscreen";

static const char kBuildQueueKey[] = "buildQueue";
static const char kBuildBucketKey[] = "buildBucket";

static const char kBuildYearKey[] = "buildYear";
static const char kBuildRevisionKey[] = "buildRevision";

static const char kSplashScreenKey[] = "splashScreen";
static const char kDebugBuildProcessKey[] = "debugBuildProcess";

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
static const char kFuseStagingKey[] = "fuseStaging";
static const char kFuseStagingSuffixKey[] = "fuseStagingSuffix";
static const char kFuseStagingSuffixDefaultValue[] = "-staging";

static const char kFusePluginName[] = "plugin.fuse";
static const char kFuseAdsPluginName[] = "plugin.fuse.ads";
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

// ----------------------------------------------------------------------------

static lua_State *
NewLua()
{
	lua_State *L = Lua::New( true );

#ifdef Rtt_AUTHORING_SIMULATOR
	Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson >, 0 );
#endif
	Lua::RegisterModuleLoader( L, "json", Lua::Open< luaload_json >, 0 );

	// TODO: Is this the best place for this?
	// By default, create an empty table for plugins
	lua_newtable( L );
	lua_setfield( L, LUA_REGISTRYINDEX, kPluginsMetadataKey );

	return L;
}

#if 0
static TargetDevice::Platform
PlatformForString( const char *str )
{
	static const char kAndroidPlatformString[] = "android";
	//static const char kIOSPlatformString[] = "iphone";
	static const char kKindlePlatformString[] = "kindle";
	static const char kNookPlatformString[] = "nook";

	TargetDevice::Platform result = TargetDevice::kIPhonePlatform;

	if ( str )
	{
		if ( 0 == Rtt_StringCompareNoCase( str, kAndroidPlatformString ) )
		{
			result = TargetDevice::kAndroidPlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kKindlePlatformString ) )
		{
			result = TargetDevice::kKindlePlatform;
		}
		else if ( 0 == Rtt_StringCompareNoCase( str, kNookPlatformString ) )
		{
			result = TargetDevice::kNookPlatform;
		}
	}

	return result;
}
#endif // 0
    
// ----------------------------------------------------------------------------

DeviceBuildData::DeviceBuildData(
	Rtt_Allocator *pAllocator,
	const char *appName,
	S32 targetDevice,
	TargetDevice::Platform targetPlatform,
	S32 targetPlatformVersion,
	const char *targetAppStoreName,
	const char *targetCertType, // bool isDistribution
	const char *clientDeviceId,
	const char *clientProductId,
	const char *clientPlatform )
:	fL( NewLua() ),
	fAppName( pAllocator, appName ),
	fTargetDevice( targetDevice ),
	fTargetPlatform( targetPlatform ),
	fTargetPlatformVersion( targetPlatformVersion ),
	fTargetAppStoreName( pAllocator, targetAppStoreName ),
	fTargetCertType( pAllocator, targetCertType ),
	fClientDeviceId( pAllocator, clientDeviceId ),
	fClientProductId( pAllocator, clientProductId ),
	fClientPlatform( pAllocator, clientPlatform ),
	fBuildQueue( pAllocator ),
	fBuildBucket( pAllocator ),
	fBuildYear( Rtt_BUILD_YEAR ),
	fBuildRevision( Rtt_BUILD_REVISION ),
	fDebugBuildProcess( 0 )
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	, fFuseStagingSuffix( pAllocator )
#endif
{
}

DeviceBuildData::~DeviceBuildData()
{
	Lua::Delete( fL );
}

bool
DeviceBuildData::ReadAppSettings( lua_State *L, const char *appSettingsPath )
{
	bool result = true;

	Rtt_LUA_STACK_GUARD( L );

	// Read 'AppSettings' (if it exists)
	if ( appSettingsPath
		 && 0 == luaL_loadfile( L, appSettingsPath )
		 && 0 == lua_pcall( L, 0, 0, 0 ) )
	{
		lua_getglobal( L, kBuildBucketKey );
		if( lua_isstring( L, -1 ) )
		{
			fBuildBucket.Set( lua_tostring( L, -1 ) );
		}
		lua_pop( L, 1 );
	}

	return result;
}

//	settings =
//	{
//		plugins =
//		{
//			-- key is the name passed to Lua's 'require()'
//			moduleName1 =
//			{
//				-- required
//				publisherId = "string domain value",
//				
//				-- optional (if nil, assumes all platforms are supported)
//				platforms =
//				{
//					ios = true,
//					android = true,
//				}
//				
//				-- optional (for partners)
//				sandboxKey = "string hash value"
//			},
//
//			moduleName2 =
//			{
//			},
//		},
//		buildQueue = "",
//		buildBucket = "",
//      buildYear = 2014,        -- int. default is simulator's daily build year
//      buildRevision = 2511,    -- int. default is simulator's daily build revision
//      fuseStaging = false,     -- default is false
//      fuseStagingSuffix = nil, -- default is kBuildFuseStagingSuffixDefaultValue
//	}
//
bool
DeviceBuildData::ReadBuildSettings( lua_State *L, const char *buildSettingsPath )
{
	bool result = true;

	Rtt_LUA_STACK_GUARD( L );

	// Read 'build.settings'. This is separate from the simulator's
	// b/c we want to support cmd-line build tools like CoronaBuilder.
	if ( buildSettingsPath
		 && Rtt_FileExists( buildSettingsPath )
		 && 0 == luaL_loadfile( L, buildSettingsPath )
		 && 0 == lua_pcall( L, 0, 0, 0 ) )
	{
		lua_getglobal( L, "settings" );
		if ( lua_istable( L, -1 ) )
		{
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
			// Fuse: Detect whether we want to use the staging version of the plugin
			lua_getfield( L, -1, kFuseStagingKey );
			bool isStaging = lua_toboolean( L, -1 );
			lua_pop( L, 1 );

			if ( isStaging )
			{
				// Allow a custom suffix to be used, or use default
				lua_getfield( L, -1, kFuseStagingSuffixKey );
				{
					const char *suffix = lua_tostring( L, -1 );
					if ( ! suffix )
					{
						suffix = kFuseStagingSuffixDefaultValue;
					}
					
					fFuseStagingSuffix.Set( suffix );
				}
				lua_pop( L, 1 );
			}
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

			lua_getfield( L, -1, kBuildYearKey );
			int buildYear = (int)lua_tointeger( L, -1 );
			lua_pop( L, 1 );

			lua_getfield( L, -1, kBuildRevisionKey );
			int buildRevision = (int)lua_tointeger( L, -1 );
			lua_pop( L, 1 );

			SetBuild( buildYear, buildRevision );

			// Find out which plugins are needed
			lua_getfield( L, -1, "plugins" ); // push plugins
			if ( lua_istable( L, -1 ) )
			{
				int index = Lua::Normalize( L, -1 );

				lua_pushnil( L );
				while ( lua_next( L, index ) != 0 )
				{
					// key is at -2. value is at -1
					const char *moduleName = lua_tostring( L, -2 );
					AddPlugin( L, moduleName, -1 );
					lua_pop( L, 1 ); // pop value. keeps key for next iteration
				}
			}
			lua_pop( L, 1 ); // pop plugins

			// settings.buildQueue (test out changes to build queue code)
			lua_getfield( L, -1, kBuildQueueKey ); // push buildQueue
			if ( lua_isstring( L, -1 ) )
			{
				const char *buildQueue = lua_tostring( L, -1 );
				fBuildQueue.Set( buildQueue );
				Rtt_LogException("WARNING: Setting '%s = %s' in build.settings", kBuildQueueKey, buildQueue);
			}
			lua_pop( L, 1 ); // pop buildQueue

			// settings.buildBucket (test out sandboxed templates)
			lua_getfield( L, -1, kBuildBucketKey ); // push buildBucket
			if ( lua_isstring( L, -1 ) )
			{
				const char *buildBucket = lua_tostring( L, -1 );
				fBuildBucket.Set( buildBucket ); // Overrides any existing value
				Rtt_LogException("WARNING: Setting '%s = %s' in build.settings", kBuildBucketKey, buildBucket);
			}
			lua_pop( L, 1 ); // pop buildBucket

			// settings.splashScreen
			lua_getfield( L, -1, kSplashScreenKey );
			if ( lua_istable( L, -1 ) )
			{
				lua_pushvalue(L, -1);
				lua_setfield( L, LUA_REGISTRYINDEX, kSplashScreenMetadataKey );
			}
			lua_pop( L, 1 ); // pop splashScreen
		}
		lua_pop( L, 1 ); // pop settings
	}

	return result;
}

bool
DeviceBuildData::Initialize(
	const char *appSettingsPath,
	const char *buildSettingsPath,
	bool includeFusePlugins,
	bool usesMonetization,
	bool liveBuild,
	int debugBuildProcess)
{
	bool result = true;

	lua_State *L = fL;

	Rtt_LUA_STACK_GUARD( L );

	ReadAppSettings( L, appSettingsPath );

	// NOTE: build.settings overrides AppSettings
	ReadBuildSettings( L, buildSettingsPath );

	fDebugBuildProcess = debugBuildProcess;

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	// If they want monetization and it's a platform we support that on which excludes the non-Google Android platforms
	// ("kAndroidPlatform" actually means "Google App Store", see Rtt_TargetDevice.h)
	if (includeFusePlugins && (fTargetPlatform == TargetDevice::kIPhonePlatform || fTargetPlatform == TargetDevice::kAndroidPlatform))
	{
		// NOTE: ReadBuildSettings should precede calls to AddRequiredPlugin
		// so that the staging suffix can be read in.
		AddRequiredPlugin( L, kFusePluginName );

		if ( usesMonetization )
		{
			AddRequiredPlugin( L, kFuseAdsPluginName );
		}
	}
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

	if (liveBuild)
	{
		PushCoronaPluginMetadata( L );
		{
			int index = lua_gettop( L );
			lua_pushstring( L, "plugin.liveBuild" );
			{
				const char *name = lua_tostring( L, -1 );
				AddPlugin( L, name, index );
			}
			lua_pop( L, 1 );
		}
		lua_pop( L, 1 );
	}

	return result;
}

void
DeviceBuildData::SetBuild( int buildYear, int buildRevision )
{
	if ( buildYear > 0 && buildRevision > 0 )
	{
		fBuildYear = buildYear;
		fBuildRevision = buildRevision;

		Rtt_TRACE( ( "NOTE: This project's build.settings overrides the default target build. Instead it targets a specific daily build (%d.%d)\n", fBuildYear, fBuildRevision ) );
	}
}

// Pushes metadata table needed by AddPlugin() to top of stack.
//
// { publisherId = "com.coronalabs" }
void
DeviceBuildData::PushCoronaPluginMetadata( lua_State *L )
{
	lua_newtable( L );
	lua_pushstring( L, "com.coronalabs" );
	lua_setfield( L, -2, "publisherId" );
}

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
void
DeviceBuildData::AddRequiredPlugin( lua_State *L, const char *name )
{
	Rtt_ASSERT( name );

	PushCoronaPluginMetadata( L );
	{
		int index = lua_gettop( L );
		const char *suffix = fFuseStagingSuffix.GetString();
		if ( ! suffix ) { suffix = ""; }
		lua_pushfstring( L, "%s%s", name, suffix );
		{
			const char *name = lua_tostring( L, -1 );
			AddPlugin( L, name, index );
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

void
DeviceBuildData::AddPlugin( lua_State *L, const char *moduleName, int index )
{
	Rtt_LUA_STACK_GUARD( L );

	index = Lua::Normalize( L, index );

	if ( lua_istable( L, index ) )
	{
#if 0
		// Filter out unsupported platforms
		bool isPlatformSupported = false;
		TargetDevice::Platform platform = (TargetDevice::Platform)fTargetPlatform;

		lua_getfield( L, index, "platforms" );
		if ( lua_istable( L, -1 ) )
		{
			for ( int i = 1, iMax = lua_objlen( L, -1 );
				  i <= iMax && ! isPlatformSupported;
				  i++ )
			{
				lua_rawgeti( L, -1, i ); // push platforms[i]
				{
					const char *value = lua_tostring( L, -1 );
					TargetDevice::Platform p = PlatformForString( value );
					isPlatformSupported = ( platform == p );
				}
				lua_pop( L, 1 ); // push platforms[i]
			}
		}
		lua_pop( L, 1 );

		// Store reference to plugin metadata
		if ( isPlatformSupported )
#endif // 0

		{
			lua_getfield( L, LUA_REGISTRYINDEX, kPluginsMetadataKey ); // push registry[kPluginsMetadataKey]
			{
				lua_pushvalue( L, index );
				lua_setfield( L, -2, moduleName );
			}
			lua_pop( L, 1 ); // pop registry[kPluginsMetadataKey]
		}
	}
	else
	{
		Rtt_PRINT( ( "Plugin (%s) in build.settings not specified correctly. Got '%s'. Expected 'table'", moduleName, lua_typename( L, index ) ) );
	}
}

//	{
//		appName = ,
//		targetDevice = , -- device
//		targetPlatform = , -- platform
//		targetPlatformVersion = , -- version
//		targetCertType = , -- certType
//		clientDeviceId = , -- deviceId
//		clientProductId = , -- productId
//		clientPlatform = ,
//		dailyBuildYear = , -- [new]
//		dailyBuildRevision = , -- [new]
//		isLaunchPadActive = , -- launchPad
//
//		plugins = {
//			-- enforce uniqueness of name on a per-project basis
//			name1 = {
//				publisherId = ,
//				supportedPlatforms = { "ios", "android", } // optional
//				sandboxKey = "3473298abde99", // optional
//			},
//			name2 = {
//				publisherId = ,
//			},
//		}
//
//		-- (separate arg in POST) inputProject = , -- file
//
//
//		-- (?) appVersion = ,
//		-- (?) packageName = , -- appPackage
//		-- (X) customBuildId = ,
//		-- (X) simVersionTimestamp = , -- timestamp
//	}

void
DeviceBuildData::PushTable( lua_State *L ) const
{
	lua_newtable( L );

    // Don't guard the stack until we've pushed the table as that's the point of this
	Rtt_LUA_STACK_GUARD( L );
    
	lua_pushstring( L, fAppName.GetString() );
	lua_setfield( L, -2, "appName" );

	lua_pushinteger( L, fTargetDevice );
	lua_setfield( L, -2, "targetDevice" );

	lua_pushinteger( L, fTargetPlatform );
	lua_setfield( L, -2, "targetPlatform" );

	lua_pushinteger( L, fTargetPlatformVersion );
	lua_setfield( L, -2, "targetPlatformVersion" );

	lua_pushstring( L, fTargetAppStoreName.GetString() );
	lua_setfield( L, -2, "targetAppStore" );

	lua_pushstring( L, fTargetCertType.GetString() );
	lua_setfield( L, -2, "targetCertType" );

	lua_pushstring( L, fClientDeviceId.GetString() );
	lua_setfield( L, -2, "clientDeviceId" );

	lua_pushstring( L, fClientProductId.GetString() );
	lua_setfield( L, -2, "clientProductId" );

	lua_pushstring( L, fClientPlatform.GetString() );
	lua_setfield( L, -2, "clientPlatform" );

	lua_pushinteger( L, fBuildYear );
	lua_setfield( L, -2, "dailyBuildYear" );

	lua_pushinteger( L, fBuildRevision );
	lua_setfield( L, -2, "dailyBuildRevision" );

	lua_pushstring( L, fBuildQueue.GetString() );
	lua_setfield( L, -2, kBuildQueueKey );

	lua_pushstring( L, fBuildBucket.GetString() );
	lua_setfield( L, -2, kBuildBucketKey );

	lua_pushinteger( L, fDebugBuildProcess );
	lua_setfield( L, -2, kDebugBuildProcessKey );

	// TODO: Remove server dependency then remove this
	lua_pushboolean( L, false );
	lua_setfield( L, -2, "isLaunchPadActive" );

	// Plugins
	lua_getfield( L, LUA_REGISTRYINDEX, kPluginsMetadataKey );
	lua_setfield( L, -2, "plugins" );

	// Splash screen
	lua_getfield( L, LUA_REGISTRYINDEX, kSplashScreenMetadataKey );
	lua_setfield( L, -2, "splashScreen" );
}

void
DeviceBuildData::GetJSON( String& result ) const
{
	lua_State *L = fL;

	Rtt_LUA_STACK_GUARD( fL );

	Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( L, "lpeg", luaopen_lpeg );
	Lua::PushModule( L, "json" ); // push 'json'

	// Call: 'json.encode( t )'
	lua_getfield( L, -1, "encode" ); // push 'json.encode'
	PushTable( L ); // push 't'
	Lua::DoCall( L, 1, 1 );

	const char *jsonValue = lua_tostring( L, -1 );
	if (fDebugBuildProcess)
	{
		Rtt_TRACE_SIM( ( "DeviceBuildData: %s\n", jsonValue ) );
	}
	result.Set( jsonValue );

	lua_pop( L, 2 );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

