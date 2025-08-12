//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

//  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  NOTE:  this class is not used on Windows
//  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
#include "Core/Rtt_Build.h"

#include "Rtt_PlatformSimulator.h"

#include "Rtt_Archive.h"
#include "Display/Rtt_Display.h"   // for AdjustPoint()
#include "Rtt_Event.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaFile.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_PlatformSurface.h"
#include "Rtt_String.h"
#include "Rtt_LuaContext.h"
#include "Rtt_FileSystem.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

int luaload_ValidateSettings(lua_State* L);
int luaload_valid_build_settings(lua_State* L);
int luaload_valid_config_lua(lua_State* L);

const char PlatformSimulator::kPlatformKeyNameIPhone[] = "iphone";
const char PlatformSimulator::kPlatformKeyNameAndroid[] = "android";
const char PlatformSimulator::kPlatformKeyNameMac[] = "mac";
// ----------------------------------------------------------------------------

/*
SimulatorOptions::SimulatorOptions()
:	connectToDebugger( false ),
	isInteractive( false ),
	appPath( NULL )
{

// WARNING: Any non-NULL char *'s will be freed
SimulatorOptions::~SimulatorOptions()
{
	if ( appPath ) free( appPath );
}
*/

// ----------------------------------------------------------------------------

const S32 kDefaultConfigIntValue = -1;

PlatformSimulator::Config::Config( Rtt_Allocator & allocator )
:	configLoaded( false ),
	platform( TargetDevice::kUnknownPlatform ),
	deviceImageFile( & allocator ),
	displayManufacturer( & allocator ),
	displayName( & allocator ),
	supportsScreenRotation( true ),
	supportsExitRequests( true ),
	supportsInputDevices( true ),
	supportsKeyEvents( true ),
	supportsKeyEventsFromKeyboard( true ),
	supportsBackKey( true ),
	supportsMouse( true ),
	supportsMultipleAlerts( false ),
	isAlertButtonOrderRightToLeft( true ),
	statusBarDefaultFile( & allocator ),
	statusBarTranslucentFile( & allocator ),
	statusBarBlackFile( & allocator ),
	statusBarLightTransparentFile( & allocator ),
	statusBarDarkTransparentFile( & allocator ),
	screenDressingFile( & allocator ),
	screenOriginX(0.0f),
	screenOriginY(0.0f),
	screenWidth(0.0f),
	screenHeight(0.0f),
	safeScreenInsetStatusBar(0.0f),
	safeScreenInsetTop(0.0f),
	safeScreenInsetLeft(0.0f),
	safeScreenInsetBottom(0.0f),
	safeScreenInsetRight(0.0f),
	safeLandscapeScreenInsetStatusBar(0.0f),
	safeLandscapeScreenInsetTop(0.0f),
	safeLandscapeScreenInsetLeft(0.0f),
	safeLandscapeScreenInsetBottom(0.0f),
	safeLandscapeScreenInsetRight(0.0f),
	hasAccelerometer( false ),
	hasGyroscope( false ),
	windowTitleBarName( & allocator ),
	defaultFontSize(0.0f),
	iosPointWidth(kDefaultConfigIntValue),
	iosPointHeight(kDefaultConfigIntValue),
	androidDisplayApproximateDpi(kDefaultConfigIntValue)
{
}

PlatformSimulator::Config::~Config()
{
}

S32
PlatformSimulator::Config::GetAdaptiveWidth() const
{
	S32 result = PlatformSurface::kUninitializedVirtualLength;
	
	if ( iosPointWidth > 0 )
	{
		result = iosPointWidth;
	}
	else if ( androidDisplayApproximateDpi > 0 )
	{
		result = PlatformSurface::CalculateVirtualLength( PlatformSurface::kDefaultVirtualDPI, androidDisplayApproximateDpi, screenWidth );
	}
	
	return result;
}

S32
PlatformSimulator::Config::GetAdaptiveHeight() const
{
	S32 result = PlatformSurface::kUninitializedVirtualLength;

	if ( iosPointHeight > 0 )
	{
		result = iosPointHeight;
	}
	else if ( androidDisplayApproximateDpi > 0 )
	{
		result = PlatformSurface::CalculateVirtualLength( PlatformSurface::kDefaultVirtualDPI, androidDisplayApproximateDpi, screenHeight );
	}
	
	return result;
}

// ----------------------------------------------------------------------------

PlatformSimulator::PlatformSimulator( PlatformFinalizer finalizer )
:	fPlatform( NULL ),
	fPlayer( NULL ),
	fPlatformFinalizer( finalizer ),
	fProperties( 0 ),
	fOrientation( DeviceOrientation::kUpright ),
	fSupportedOrientations( 0 ),
	fLastSupportedOrientation( DeviceOrientation::kUpright ),
	fLastDeviceWidth( -1 ),
	fLastDeviceHeight( -1 ),
	fIsTransparent(false)
{
}

PlatformSimulator::~PlatformSimulator()
{
	WillExit();

	delete fPlayer;

	(*fPlatformFinalizer)( fPlatform );
}

void
PlatformSimulator::Initialize( MPlatform* platform, MCallback *viewCallback )
{
	Rtt_ASSERT( ! fPlatform && ! fPlayer );

	// OpenGL contexts must be available before we call Player c-tor
	PlatformPlayer* player = new PlatformPlayer( * platform, viewCallback );

	fPlatform = platform;
	fPlayer = player;
}

const char *
PlatformSimulator::GetPlatformName() const
{
	return NULL;
}

const char *
PlatformSimulator::GetPlatform() const
{
	return NULL;
}

static bool
BoolForKey( lua_State *L, const char key[], bool defaultValue )
{
    bool result = defaultValue;
    
    lua_getfield( L, -1, key );
    if (lua_isboolean( L, -1 ))
    {
        result = lua_toboolean( L, -1 ) ? true : false;
    }
    lua_pop( L, 1 );
    
    return result;
}

static lua_Number
NumberForKey( lua_State *L, const char key[], lua_Number defaultValue )
{
    lua_Number result = defaultValue;
    
	lua_getfield( L, -1, key );
    if (lua_isnumber( L, -1 ) )
    {
        result = lua_tonumber( L, -1 );
    }
	lua_pop( L, 1 );
    
	return result;
}

static lua_Integer
IntForKey( lua_State *L, const char key[], lua_Integer defaultValue )
{
	lua_Integer result = defaultValue;

	lua_getfield( L, -1, key );
	if (lua_isnumber( L, -1 ) )
	{
		result = lua_tointeger( L, -1 );
	}
	lua_pop( L, 1 );

	return result;
}


static const char*
StringForKey( lua_State *L, const char key[], const char *defaultValue)
{
    const char *result = defaultValue;
    
	lua_getfield( L, -1, key );
    
    if ( lua_isstring( L, -1 ) )
    {
        result = lua_tostring( L, -1 );
    }
	lua_pop( L, 1 );

	return result;
}

#include "CoronaLua.h"

void
PlatformSimulator::LoadConfig(const char deviceConfigFile[], const MPlatform& platform, Config& rConfig)
{
	PlatformSimulator::LoadConfig(deviceConfigFile, rConfig);
}

void
PlatformSimulator::LoadConfig( const char deviceConfigFile[], Config& rConfig )
{
	lua_State *L = luaL_newstate();
	String errorMesg;

    rConfig.configLoaded = false;
    
	if ( 0 == Lua::DoFile( L, deviceConfigFile, 0, false, &errorMesg ))
	{
		lua_getglobal( L, "simulator" );

		rConfig.platform = TargetDevice::PlatformForDeviceType( StringForKey( L, "device", NULL ) );
		rConfig.supportsExitRequests = ( rConfig.platform != TargetDevice::kIPhonePlatform && rConfig.platform != TargetDevice::kTVOSPlatform );

		// this allows the Simulator to return the simulated OS name in response to system.getInfo("platform")
		switch (rConfig.platform)
		{
			case TargetDevice::kAndroidPlatform:
			case TargetDevice::kKindlePlatform:
			case TargetDevice::kNookPlatform:
				rConfig.osName.Set("android");
				break;
			case TargetDevice::kIPhonePlatform:
				rConfig.osName.Set("ios");
				break;
			case TargetDevice::kOSXPlatform:
				rConfig.osName.Set("macos");
				break;
			case TargetDevice::kTVOSPlatform:
				rConfig.osName.Set("tvos");
				break;
			case TargetDevice::kWin32Platform:
				rConfig.osName.Set("win32");
				break;
			case TargetDevice::kWinPhoneSilverlightPlatform:
				rConfig.osName.Set("winphone");
				break;
			case TargetDevice::kNxSPlatform:
				rConfig.osName.Set("nx64");
				break;
			default:
				rConfig.osName.Set("simulator");
				break;
		}

		switch (rConfig.platform)
		{
			case TargetDevice::kAndroidPlatform:
			case TargetDevice::kIPhonePlatform:
			case TargetDevice::kOSXPlatform:
			case TargetDevice::kTVOSPlatform:
			case TargetDevice::kWin32Platform:
			case TargetDevice::kWinPhoneSilverlightPlatform:
				rConfig.supportsKeyEvents = true;
				break;
			default:
				rConfig.supportsKeyEvents = false;
				break;
		}

		switch (rConfig.platform)
		{
			// In the Simulator we allow keyboard keys in all skins though we
			// warn if the simulated device doesn't support them (see init.lua)
			case TargetDevice::kAndroidPlatform:
			case TargetDevice::kIPhonePlatform:
			case TargetDevice::kOSXPlatform:
			case TargetDevice::kTVOSPlatform:
			case TargetDevice::kWin32Platform:
			case TargetDevice::kWinPhoneSilverlightPlatform:
				rConfig.supportsKeyEventsFromKeyboard = true;
				break;
			default:
				rConfig.supportsKeyEventsFromKeyboard = false;
				break;
		}

		switch (rConfig.platform)
		{
			case TargetDevice::kAndroidPlatform:
			case TargetDevice::kWin32Platform:
			case TargetDevice::kWinPhoneSilverlightPlatform:
				rConfig.supportsBackKey = true;
				break;
			default:
				rConfig.supportsBackKey = false;
				break;
		}

		switch (rConfig.platform)
		{
			case TargetDevice::kAndroidPlatform:
			case TargetDevice::kIPhonePlatform:
			case TargetDevice::kOSXPlatform:
			case TargetDevice::kTVOSPlatform:
			case TargetDevice::kWin32Platform:
			case TargetDevice::kNxSPlatform:
				rConfig.supportsInputDevices = true;
				break;
			default:
				rConfig.supportsInputDevices = false;
				break;
		}

		rConfig.supportsMouse = false;
		switch (rConfig.platform)
		{
			case TargetDevice::kAndroidPlatform:
			case TargetDevice::kOSXPlatform:
			case TargetDevice::kWin32Platform:
				rConfig.supportsMouse = true;
				break;
			default:
				rConfig.supportsMouse = false;
				break;
		}
		
		rConfig.supportsMultipleAlerts = false;
		switch (rConfig.platform)
		{
			case TargetDevice::kWin32Platform:
			case TargetDevice::kWinPhoneSilverlightPlatform:
				rConfig.supportsMultipleAlerts = true;
				break;
			default:
				rConfig.supportsMultipleAlerts = false;
				break;
		}

		rConfig.isAlertButtonOrderRightToLeft = true;
		switch (rConfig.platform)
		{
			case TargetDevice::kWin32Platform:
			case TargetDevice::kWinPhoneSilverlightPlatform:
				rConfig.isAlertButtonOrderRightToLeft = false;
				break;
			default:
				rConfig.isAlertButtonOrderRightToLeft = true;
				break;
		}

		rConfig.screenOriginX = (float) NumberForKey( L, "screenOriginX", 0 );
		rConfig.screenOriginY = (float) NumberForKey( L, "screenOriginY", 0 );
		rConfig.screenWidth = (float) NumberForKey( L, "screenWidth", 400 );
		rConfig.screenHeight = (float) NumberForKey( L, "screenHeight", 400 );

		rConfig.safeScreenInsetStatusBar = (float) NumberForKey( L, "safeScreenInsetStatusBar", 0 );
		rConfig.safeScreenInsetTop = (float) NumberForKey( L, "safeScreenInsetTop", 0 );
		rConfig.safeScreenInsetLeft = (float) NumberForKey( L, "safeScreenInsetLeft", 0 );
		rConfig.safeScreenInsetBottom = (float) NumberForKey( L, "safeScreenInsetBottom", 0 );
		rConfig.safeScreenInsetRight = (float) NumberForKey( L, "safeScreenInsetRight", 0 );
		rConfig.safeLandscapeScreenInsetStatusBar = (float) NumberForKey( L, "safeLandscapeScreenInsetStatusBar", 0 );
		rConfig.safeLandscapeScreenInsetTop = (float) NumberForKey( L, "safeLandscapeScreenInsetTop", 0 );
		rConfig.safeLandscapeScreenInsetLeft = (float) NumberForKey( L, "safeLandscapeScreenInsetLeft", 0 );
		rConfig.safeLandscapeScreenInsetBottom = (float) NumberForKey( L, "safeLandscapeScreenInsetBottom", 0 );
		rConfig.safeLandscapeScreenInsetRight = (float) NumberForKey( L, "safeLandscapeScreenInsetRight", 0 );

		rConfig.deviceImageFile.Set( StringForKey( L, "deviceImage", NULL ) );
		rConfig.displayManufacturer.Set( StringForKey( L, "displayManufacturer", "unknown" ) );
		rConfig.displayName.Set( StringForKey( L, "displayName", "Custom Device" ) );
		rConfig.statusBarDefaultFile.Set( StringForKey( L, "statusBarDefault", NULL ) );
		rConfig.statusBarTranslucentFile.Set( StringForKey( L, "statusBarTranslucent", NULL ) );
		rConfig.statusBarBlackFile.Set( StringForKey( L, "statusBarBlack", NULL ) );
		rConfig.statusBarLightTransparentFile.Set( StringForKey( L, "statusBarLightTransparent", NULL ) );
		rConfig.statusBarDarkTransparentFile.Set( StringForKey( L, "statusBarDarkTransparent", NULL ) );
		rConfig.screenDressingFile.Set( StringForKey( L, "screenDressing", NULL ) );
		rConfig.supportsScreenRotation = BoolForKey( L, "supportsScreenRotation", true );
		rConfig.hasAccelerometer = BoolForKey( L, "hasAccelerometer", true );
		rConfig.isUprightOrientationPortrait = BoolForKey( L, "isUprightOrientationPortrait", (rConfig.screenHeight > rConfig.screenWidth) );
		rConfig.windowTitleBarName.Set( StringForKey( L, "windowTitleBarName", "Custom Device" ) );
		rConfig.defaultFontSize = (float) NumberForKey( L, "defaultFontSize", 0 );

		// iOS skin-specific
		rConfig.iosPointWidth = (S32) IntForKey( L, "iosPointWidth", kDefaultConfigIntValue );
		rConfig.iosPointHeight = (S32) IntForKey( L, "iosPointHeight", kDefaultConfigIntValue );

		// Android skin-specific
		rConfig.androidDisplayApproximateDpi = (S32) IntForKey( L, "androidDisplayApproximateDpi", kDefaultConfigIntValue );

		lua_pop( L, 1 );
        
        rConfig.configLoaded = true;
	}
	else
	{
		Rtt_TRACE(("WARNING: Could not load device config file '%s': %s\n", deviceConfigFile, errorMesg.GetString()));
	}

	lua_close( L );
}

void
PlatformSimulator::ValidateSettings(const MPlatform& platform)
{
	lua_State *L = luaL_newstate();

	luaL_openlibs(L);
	
	String resourcePath;
	platform.PathForFile( NULL, MPlatform::kResourceDir, MPlatform::kTestFileExists, resourcePath );

	//Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson > );
	//Lua::RegisterModuleLoader( L, "json", Lua::Open< luaload_json > );
	//Lua::RegisterModuleLoader( L, "lpeg", luaopen_lpeg );
	//Lua::RegisterModuleLoader( L, "lfs", luaopen_lfs );

	Lua::RegisterModuleLoader( L, "valid_build_settings", Lua::Open< luaload_valid_build_settings > );
	Lua::RegisterModuleLoader( L, "valid_config_lua", Lua::Open< luaload_valid_config_lua > );

	Lua::DoBuffer(L, &luaload_ValidateSettings, NULL);

	lua_getglobal(L, "validateSettings" ); Rtt_ASSERT(lua_isfunction(L, -1 ));

	// First do build.settings

	String buildSettingsPath;
	buildSettingsPath.Set(resourcePath);
	buildSettingsPath.AppendPathComponent("build.settings");

	if (Rtt_FileExists(buildSettingsPath))
	{
		lua_pushstring( L, buildSettingsPath );
		lua_pushstring( L, "valid_build_settings" );

		// This will emit any diagnostics to the console
		Lua::DoCall(L, 2, 1);
	}

	// Now do config.lua

	String configLuaPath;
	configLuaPath.Set(resourcePath);
	configLuaPath.AppendPathComponent("config.lua");

	if (Rtt_FileExists(configLuaPath))
	{
		lua_getglobal(L, "validateSettings" ); Rtt_ASSERT(lua_isfunction(L, -1 ));
		lua_pushstring( L, configLuaPath );
		lua_pushstring( L, "valid_config_lua" );

		// This will emit any diagnostics to the console
		Lua::DoCall(L, 2, 1);
	}

	lua_close( L );
}

void
PlatformSimulator::LoadBuildSettings( const MPlatform& platform )
{
	lua_State *L = luaL_newstate();

	// Warn about logical errors in build.settings and config.lua
	ValidateSettings(platform);

	const char kBuildSettings[] = "build.settings";
	
	String filePath( & platform.GetAllocator() );
	platform.PathForFile( kBuildSettings, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath );

	const char *p = filePath.GetString();
	if ( p != NULL ) // build.settings is optional
	{
		if ( 0 == luaL_loadfile( L, p )
		  && 0 == lua_pcall( L, 0, 0, 0 ) )
		{
			lua_getglobal( L, "settings" );
			if ( lua_istable( L, -1 ) )
			{
				lua_getfield( L, -1, "orientation" );
				if ( lua_istable( L, -1 ) )
				{
					const char *orientation = StringForKey( L, "default", NULL );
					DeviceOrientation::Type defaultOrientation = DeviceOrientation::TypeForString( orientation );

					if ( DeviceOrientation::kUnknown == defaultOrientation
					 || DeviceOrientation::kUpsideDown == defaultOrientation )
					{
						Rtt_WARN_SIM(
									 DeviceOrientation::kUpsideDown != defaultOrientation,
									 ( "WARNING: Applications cannot launch upside down. Launching in portrait.\n" ) );

						defaultOrientation = DeviceOrientation::kUpright;
					}
					else
					{
						if ( 0 == Rtt_StringCompare( orientation, "landscape" ) )
						{
							defaultOrientation = DeviceOrientation::kSidewaysRight;
						}
					}

					orientation = StringForKey( L, "content", NULL );
					if ( orientation )
					{
						DeviceOrientation::Type content = DeviceOrientation::TypeForString( orientation );
						if ( DeviceOrientation::kUnknown != content
						 && DeviceOrientation::kUpsideDown != content ) // forbid simulator from launching upside down
						{
							if ( 0 == Rtt_StringCompare( orientation, "landscape" ) )
							{
								content = DeviceOrientation::kSidewaysRight;
							}

							Rtt_WARN_SIM( defaultOrientation == content,
									  ( "WARNING: build.settings has different values for the default and content orientation. "
									   "Using the content orientation for the launch orientation.\n" ) );

							SetProperty( kIsOrientationLocked, true );

							// content orientation overrides default
							defaultOrientation = content;
						}
					}

					if ( DeviceOrientation::IsInterfaceOrientation( defaultOrientation ) )
					{
						fOrientation = defaultOrientation;
						SetOrientationSupported( defaultOrientation );
					}

					lua_getfield( L, -1, "supported" );
					{
						for ( int i = 1, iMax = (int) lua_objlen( L, -1 );
						  i <= iMax;
						  i++ )
						{
							lua_rawgeti( L, -1, i );
							const char *value = lua_tostring( L, -1 );
							DeviceOrientation::Type t = DeviceOrientation::TypeForString( value );
							if ( DeviceOrientation::IsInterfaceOrientation( t ) )
							{
								SetOrientationSupported( t );
							}
							lua_pop( L, 1 );
						}
					}

					lua_pop( L, 1 ); // pop supported
				}
				lua_pop( L, 1 ); // pop orientation
			}
			lua_pop( L, 1 ); // pop settings
		}
		else
		{
			Rtt_TRACE(("WARNING: Could not load 'build.settings': %s\n", lua_tostring( L, -1 )));
			platform.RuntimeErrorNotification( "build.settings error", lua_tostring( L, -1 ), NULL );
		}
	}

	const char kConfigLua[] = "config.lua";
	
	platform.PathForFile( kConfigLua, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath );

	p = filePath.GetString();
	if ( p != NULL ) // config.lua is optional
	{
		if ( 0 == luaL_loadfile( L, p )
		  && 0 == lua_pcall( L, 0, 0, 0 ) )
		{
			lua_getglobal( L, "application" );
			if ( lua_istable( L, -1 ) )
			{
				lua_getfield( L, -1, "isTransparent" );
				
				SetIsTransparent( lua_toboolean( L, -1 ) );
			}
			
			lua_pop( L, 1 ); // pop application
		}
	}
	
	lua_close( L );
}


U32
PlatformSimulator::GetModulesFromBuildSettings( const char* platformkeyname, bool& isdefined, bool& iserror )
{
	U32 moduleflags = 0;
	iserror = false;
	isdefined = false;
	
	lua_State *L = luaL_newstate();
	
	const char kBuildSettings[] = "build.settings";
	
	String filePath( & fPlatform->GetAllocator() );
	fPlatform->PathForFile( kBuildSettings, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath );

	const char *p = filePath.GetString();
	if ( p
		 && 0 == luaL_loadfile( L, p )
		 && 0 == lua_pcall( L, 0, 0, 0 ) )
	{
		lua_getglobal( L, "settings" );
		if ( lua_istable( L, -1 ) )
		{
			lua_getfield( L, -1, platformkeyname ); // "iphone" or "android" or "mac", etc
			if ( lua_istable( L, -1 ) )
			{
				lua_getfield( L, -1, "components" ); // This must be a standard Lua array, not an associative array
				if ( lua_istable( L, -1 ) )
				{
					isdefined = true;
					// Must be a convential array or things won't work right.
					int number_of_elements = (int) lua_objlen(L, -1);  /* get length of array */
					for ( int i=1; i<=number_of_elements; i++ )
					{
						lua_rawgeti(L, -1, i);  /* array is at -1, pushes value at array[i] onto top of stack */
						if ( lua_type( L, -1 ) == LUA_TSTRING )
						{
							// Add other keys like this here:
							/*
							if ( 0 == Rtt_StringCompareNoCase( lua_tostring( L, -1 ), "papaya" ) )
							{
								moduleflags = moduleflags | LuaContext::kPapayaModuleMask;
							}
							else
							*/
							{
								// Intentionally ignoring unknown keys so people using later versions (daily builds)
								// don't trip errors when reverting to an older build that doesn't know about the key.
								// Warning might be okay, but don't fail the process.
								Rtt_TRACE (("Warning: Unknown component in build.settings component array for device=%s at element at index=%d (%s).\n", platformkeyname, i, lua_tostring( L, -1 ) ) );
							}
						}
						else
						{
							Rtt_TRACE (("Error: Invalid type in build.settings component array for device=%s. The element at index=%d is not a string.\n", platformkeyname, i) );
						}
						lua_pop( L, 1 ); // pop array[i] off the top of the stack. array is now on top of stack again.
					}

				}
				lua_pop( L, 1 ); // pop components
			}
			lua_pop( L, 1 ); // pop platformkeyname
		}
		lua_pop( L, 1 ); // pop settings
	}
	else
	{
		iserror = true;
	}
	
	lua_close( L );
	
	return moduleflags;
}


void
PlatformSimulator::Start( const SimulatorOptions& options )
{
	Rtt_ASSERT( fPlayer );

	Runtime& runtime = fPlayer->GetRuntime();

	// Currently, enterprise is required to have the Lua parser available
	bool isLuaParserAvailable = true;
	runtime.SetProperty( Runtime::kIsLuaParserAvailable, isLuaParserAvailable );

 	// Read plugins from build.settings
	runtime.FindDownloadablePlugins( GetPlatformName() );

	const char* appFilePath =
		( options.isInteractive
			? PlatformPlayer::InteractiveFilePath()
			: PlatformPlayer::DefaultAppFilePath() );

	fPlayer->Start( appFilePath, options.connectToDebugger, GetOrientation() );

	fLastSupportedOrientation = GetOrientation();
	fLastDeviceWidth = runtime.GetDisplay().DeviceWidth();
	fLastDeviceHeight = runtime.GetDisplay().DeviceHeight();
	
	DidStart();
}

void
PlatformSimulator::SetProperty( U32 mask, bool value )
{
	if ( Rtt_VERIFY( mask > kUninitializedMask ) )
	{
		const U32 p = fProperties;
		fProperties = ( value ? p | mask : p & ~mask );
	}
}

static PlatformSimulator::PropertyMask
PropertyMaskForEventType( MPlatformDevice::EventType type )
{
	PlatformSimulator::PropertyMask mask = PlatformSimulator::kUninitializedMask;

	switch( type )
	{
		case MPlatformDevice::kOrientationEvent:
			mask = PlatformSimulator::kOrientationEventMask;
			break;
		case MPlatformDevice::kAccelerometerEvent:
			mask = PlatformSimulator::kAccelerometerEventMask;
			break;
		case MPlatformDevice::kGyroscopeEvent:
			mask = PlatformSimulator::kGyroscopeEventMask;
			break;
		case MPlatformDevice::kLocationEvent:
			mask = PlatformSimulator::kLocationEventMask;
			break;
		case MPlatformDevice::kHeadingEvent:
			mask = PlatformSimulator::kHeadingEventMask;
			break;
		case MPlatformDevice::kMultitouchEvent:
			mask = PlatformSimulator::kMultitouchEventMask;
			break;
		case MPlatformDevice::kMouseEvent:
			mask = PlatformSimulator::kMouseEventMask;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return mask;
}

void
PlatformSimulator::BeginNotifications( MPlatformDevice::EventType type ) const
{
	PropertyMask mask = PropertyMaskForEventType( type );

	switch( mask )
	{
		case kAccelerometerEventMask:
			Rtt_TRACE_SIM( ( "WARNING: Simulator does not support accelerometer events\n" ) );
			break;
		case kGyroscopeEventMask:
			Rtt_TRACE_SIM( ( "WARNING: Simulator does not support gyroscope events\n" ) );
			break;
		case kLocationEventMask:
			break;
		case kHeadingEventMask:
			Rtt_TRACE_SIM( ( "WARNING: Simulator does not support heading events\n" ) );
			break;
		case kMultitouchEventMask:
			Rtt_TRACE_SIM( ( "WARNING: Simulator does not support multitouch events\n" ) );
			break;
		default:
			break;
	}

	const_cast< Self* >( this )->SetProperty( mask, true );
}

void
PlatformSimulator::EndNotifications( MPlatformDevice::EventType type ) const
{
	const_cast< Self* >( this )->SetProperty( PropertyMaskForEventType( type ), false );
}

void
PlatformSimulator::Rotate( bool clockwise )
{
	DeviceOrientation::Type oldOrientation = (DeviceOrientation::Type)fOrientation;
	DeviceOrientation::Type orientation = oldOrientation;
	orientation = (DeviceOrientation::Type)(orientation + ( clockwise ? -1 : 1 ));
	if ( DeviceOrientation::kUnknown == orientation )
	{
		orientation = DeviceOrientation::kSidewaysLeft;
	}
	else if ( DeviceOrientation::kFaceUp == orientation )
	{
		orientation = DeviceOrientation::kUpright;
	}

	fOrientation = orientation;

	DidRotate( clockwise, oldOrientation, orientation );

	Rtt::Runtime& runtime = GetPlayer()->GetRuntime();

	// Raise an orientation event, if enabled.
	if ( IsProperty( kOrientationEventMask ) )
	{
		OrientationEvent event( orientation, oldOrientation );
		runtime.DispatchEvent( event );
	}

	// Determine if the content width and height has changed.
	// This is determined if the orientation has changed from portrait to landscape or vice-versa.
	// Note: We have to ignore the render stream's content width and height because it wrongly swaps
	//       these values when rotating to an orientation the app does not support.
	bool hasContentWidthHeightChanged = false;
	if (IsOrientationSupported(orientation))
	{
		if (DeviceOrientation::IsSideways(orientation) != DeviceOrientation::IsSideways(fLastSupportedOrientation))
		{
			hasContentWidthHeightChanged = true;
		}
		fLastSupportedOrientation = orientation;
	}

	// Raise a resize event if the simulated app's view has changed size.
	// This is expected to be raised after the orientation event.
	S32 currentDeviceWidth = runtime.GetDisplay().DeviceWidth();
	S32 currentDeviceHeight = runtime.GetDisplay().DeviceHeight();
	if (hasContentWidthHeightChanged ||
	    (fLastDeviceWidth != currentDeviceWidth) ||
	    (fLastDeviceHeight != currentDeviceHeight))
	{
		runtime.DispatchEvent( ResizeEvent() );
	}
	fLastDeviceWidth = currentDeviceWidth;
	fLastDeviceHeight = currentDeviceHeight;

	// On a real device, any orientation change would be accompanied by at least one accelerometer event
	PlatformSimulator::Shake();
}

void
PlatformSimulator::Shake()
{
	double gravity[] = { 0., 0., 0. };
	switch ( fOrientation )
	{
		case DeviceOrientation::kUpright:		gravity[1] = -1.; break;
		case DeviceOrientation::kUpsideDown:	gravity[1] = 1.; break;
		case DeviceOrientation::kSidewaysLeft:	gravity[0] = 1.; break;
		case DeviceOrientation::kSidewaysRight:	gravity[0] = -1.; break;
	}

	double instant[] = { 1.5, 1.5, 1.5 };
	double deltaTime = 0.1;

	AccelerometerEvent event( gravity, instant, instant, true, deltaTime );
	GetPlayer()->GetRuntime().DispatchEvent( event );	
}

bool
PlatformSimulator::IsOrientationSupported( DeviceOrientation::Type orientation )
{
	Rtt_ASSERT( DeviceOrientation::IsInterfaceOrientation( orientation ) );
	return 0 != ( fSupportedOrientations & ( 1 << orientation ) );
}

void
PlatformSimulator::SetOrientationSupported( DeviceOrientation::Type orientation )
{
	Rtt_ASSERT( DeviceOrientation::IsInterfaceOrientation( orientation ) );
	fSupportedOrientations |= ( 1 << orientation );
}

void
PlatformSimulator::WillSuspend()
{
	
}

void 
PlatformSimulator::DidSuspend()
{
	
}

void
PlatformSimulator::WillResume()
{
	
}
	
void
PlatformSimulator::DidResume()
{
	
}
	
void
PlatformSimulator::ToggleSuspendResume(bool sendApplicationEvents /* = true */)
{
//Rtt_TRACE( ( "DidResume\n" ) );
	
	// This assert interferes with suspending the app if a syntax error is thrown during
	// app load.  Since it's only active in debug builds removing it should be benign for
	// the shipping product.
	// Rtt_ASSERT( IsProperty( kIsAppStartedMask ) );

	PlatformPlayer* player = GetPlayer();

	if ( player )
	{
		Runtime& runtime = player->GetRuntime();
		bool isSuspended = runtime.IsSuspended();

		if ( isSuspended )
		{
			// System is suspending app, e.g. phone call
			WillResume();
			runtime.Resume(sendApplicationEvents);
			DidResume();
		}
		else
		{
			WillSuspend();
			runtime.Suspend(sendApplicationEvents);
			DidSuspend();
		}
	}
}

void
PlatformSimulator::DidStart()
{
//Rtt_TRACE( ( "DidStart\n" ) );
	SetProperty( kIsAppStartedMask, true );
}

void
PlatformSimulator::WillExit()
{
}

void
PlatformSimulator::DidChangeScale(float scalefactor)
{
}

// TODO: This function should be called from adjustPoint() in GLview.mm as well as Windows code
// Until then, the two functions need to be kept in sync.
void
PlatformSimulator::AdjustPoint( float& ptX, float& ptY, float viewWidth, float viewHeight, float zoomFactor )
{
	Rtt::Runtime *pRuntime = & (GetPlayer()->GetRuntime());
	Rtt::Display& display = pRuntime->GetDisplay();

    // Change point origin based on view orientation relative to screen orientation
	const Rtt::DeviceOrientation::Type orientation = display.GetRelativeOrientation();
	float x = ptX;
	if ( Rtt::DeviceOrientation::kUpright == orientation )
	{
         // point origin unchanged
	}
	else if ( Rtt::DeviceOrientation::kSidewaysRight == orientation )
	{
		ptX = ptY;
		ptY = viewWidth - x;
	}
	else if ( Rtt::DeviceOrientation::kUpsideDown == orientation )
	{
		ptX = viewWidth - ptX;
		ptY = viewHeight - ptY;
	}
	else if ( Rtt::DeviceOrientation::kSidewaysLeft == orientation )
	{
		ptX = viewHeight - ptY;
		ptY = x;
	}
	else 
	{
		Rtt_ASSERT_NOT_REACHED();
	}

    // Restore point's scale relative to current zoom factor
    ptX = ptX / zoomFactor;
    ptY = ptY / zoomFactor;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

