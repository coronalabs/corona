//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppPackagerFactory.h"

#if defined(CORONABUILDER_ANDROID)
#include "Rtt_AndroidAppPackager.h"
#endif
#if defined(CORONABUILDER_WIN32)
#include "Rtt_Win32AppPackager.h"
#endif
#if defined(CORONABUILDER_IOS)
#include "Rtt_IOSAppPackager.h"
#endif
#if defined(CORONABUILDER_TVOS)
#include "Rtt_TVOSAppPackager.h"
#endif
#if defined(CORONABUILDER_OSX)
#include "Rtt_OSXAppPackager.h"
#endif
#if defined(CORONABUILDER_HTML5)
#include "Rtt_WebAppPackager.h"
#endif
#if defined(CORONABUILDER_LINUX)
#include "Rtt_LinuxAppPackager.h"
#endif

#ifdef Rtt_WIN_ENV
#include "Rtt_JavaHost.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AppPackagerContext::AppPackagerContext( TargetDevice::Platform platform )
:	fRequiresJava( false )
{
	switch ( platform )
	{
		case TargetDevice::kAndroidPlatform:
		case TargetDevice::kKindlePlatform:
		case TargetDevice::kNookPlatform:
			fRequiresJava = true;
			break;
		default:
			break;
	}

#if defined(USE_JNI) && defined(Rtt_WIN_ENV)
	if ( fRequiresJava )
	{
		JavaHost::Initialize();
	}
#endif
}

AppPackagerContext::~AppPackagerContext()
{
#if defined(USE_JNI) && defined(Rtt_WIN_ENV)
	if ( fRequiresJava )
	{
		JavaHost::Terminate();
	}
#endif
}

// ----------------------------------------------------------------------------

AppPackagerFactory::AppPackagerFactory( const MPlatformServices& services )
:	fServices( services )
{
}

AppPackagerFactory::~AppPackagerFactory()
{
}

static const char kPlatformVersionKey[] = "platformVersion";

static const char kAppNameKey[] = "appName";
static const char kAppVersionKey[] = "appVersion";
//static const char kAppBundleIdKey[] = "appBundleId";

static const char kProjectPathKey[] = "projectPath";
static const char kDstPathKey[] = "dstPath";

static const char kCertificatePathKey[] = "certificatePath";
//static const char kCertificateIdentityKey[] = "certificateIdentity";
//static const char kCertificateTypeKey[] = "certificateType";

static const char kSdkPathKey[] = "sdkPath";

//static const char kDeviceOSVersionKey[] = "deviceOSVersion";
static const char kTemplateIdKey[] = "customBuildId";
static const char kTemplateTypeKey[] = "targetDevice";

AppPackagerParams*
AppPackagerFactory::CreatePackagerParams( lua_State *L, int index, TargetDevice::Platform targetPlatform ) const
{
	AppPackagerParams *result = NULL;
	TargetDevice::Version targetPlatformVersion = TargetDevice::VersionForPlatform( targetPlatform );

#if defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS)
	// Validate or default 'params[platformVersion]' for iOS and tvOS
	if (targetPlatform == TargetDevice::kIPhonePlatform || targetPlatform == TargetDevice::kTVOSPlatform)
	{
		const char *bundleDir = GetResourceDirectory();
		int top = lua_gettop(L);

		// We have to differentiate between the platforms here because 'platform' may not be set in 'params'
		// (we may have defaulted to iOS elsewhere)
		if (targetPlatform == TargetDevice::kTVOSPlatform)
		{
			// Call: CoronaBuilderDetermineTargettvOSVersion(params)
			lua_getglobal( L, "CoronaBuilderDetermineTargettvOSVersion" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
		}
		else
		{
			// Call: CoronaBuilderDetermineTargetiOSVersion(params)
			lua_getglobal( L, "CoronaBuilderDetermineTargetiOSVersion" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
		}

		// Push 'params' as the call parameter
		lua_pushvalue(L, -2);

		lua_pushstring(L, bundleDir);
		lua_pushstring(L, Rtt_STRING_BUILD);

		if (Lua::DoCall( L, 3, 2 ) == 0)
		{
			// Stack is [1] message, [2] boolean indicating success or failure
			if (! lua_toboolean(L, -2))
			{
				printf("ERROR: %s\n", lua_tostring(L, -1));

				return NULL;
			}
			else
			{
				printf("CoronaBuilder: %s\n", lua_tostring(L, -1));
			}
		}

		lua_settop(L, top);
	}
#endif // defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS)

	// WARNING: Change value to lua_checkstack
	// if you increase the number of lua_getfield() calls!!!
	lua_checkstack( L, 11 );
	int top = lua_gettop( L );

	// See if the build spec overrides the default platform version
	lua_getfield( L, index, kPlatformVersionKey );
	if (lua_isnumber( L, -1 ))
	{
		targetPlatformVersion = (TargetDevice::Version) lua_tointeger( L, -1 );
	}

	lua_getfield( L, index, kAppNameKey );
	const char *appName = lua_tostring( L, -1 );
	CHECK_VALUE( appName, kAppNameKey );

	lua_getfield( L, index, kAppVersionKey );
	const char *version = lua_tostring( L, -1 );
	CHECK_VALUE( version, kAppVersionKey );

	lua_getfield( L, index, kCertificatePathKey );
	const char *certificatePath = lua_tostring( L, -1 );

	lua_getfield( L, index, kProjectPathKey );
	const char *projectPath = lua_tostring( L, -1 );
	CHECK_VALUE( projectPath, kProjectPathKey );
	
	lua_getfield( L, index, kDstPathKey );
	const char *dstPath = lua_tostring( L, -1 );
	CHECK_VALUE( dstPath, kDstPathKey );

	const char *sdkPath = NULL;

#if defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS) || defined(CORONABUILD_OSX)
	if (targetPlatform == TargetDevice::kIPhonePlatform
		|| targetPlatform == TargetDevice::kOSXPlatform
		|| targetPlatform == TargetDevice::kTVOSPlatform)
	{
		// Optional
		lua_getfield( L, index, kSdkPathKey );
		sdkPath = lua_tostring( L, -1 );
	}
#endif

//	lua_getfield( L, index, kDeviceOSVersionKey );
//	const char *deviceOSVersion = lua_tostring( L, -1 );

	lua_getfield( L, index, kTemplateIdKey );
	const char *customBuildId = lua_tostring( L, -1 );

	lua_getfield( L, index, kTemplateTypeKey );
	const char *templateType = lua_tostring( L, -1 );

//	lua_getfield( L, index, kAppBundleIdKey );
//	const char *appBundleId = lua_tostring( L, -1 );

#if defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS) || defined(CORONABUILDER_OSX)
	if (targetPlatform == TargetDevice::kIPhonePlatform
		|| targetPlatform == TargetDevice::kOSXPlatform
		|| targetPlatform == TargetDevice::kTVOSPlatform )
	{
		result = CreatePackagerParamsApple(L,
		index,
		targetPlatform,
		targetPlatformVersion,
		appName,
		version,
		certificatePath,
		projectPath,
		dstPath,
		sdkPath,
		customBuildId,
		templateType );
	}
	else
#endif
#if defined(CORONABUILDER_ANDROID)
	if (targetPlatform == TargetDevice::kAndroidPlatform
		|| targetPlatform == TargetDevice::kKindlePlatform
		|| targetPlatform == TargetDevice::kNookPlatform)
	{
		result = CreatePackagerParamsAndroid(L,
											 index,
											 targetPlatform,
											 targetPlatformVersion,
											 appName,
											 version,
											 certificatePath,
											 projectPath,
											 dstPath,
											 sdkPath,
											 customBuildId,
											 templateType );
	}
	else
#endif // CORONABUILDER_ANDROID
#if defined(CORONABUILDER_WIN32)
	if (targetPlatform == TargetDevice::kWin32Platform)
	{
		result = CreatePackagerParamsWin32(L,
											 index,
											 targetPlatform,
											 targetPlatformVersion,
											 appName,
											 version,
											 certificatePath,
											 projectPath,
											 dstPath,
											 sdkPath,
											 customBuildId,
											 templateType );
	}
	else
#endif
#if defined(CORONABUILDER_HTML5)
		if (targetPlatform == TargetDevice::kWebPlatform)
	{
		result = CreatePackagerParamsHTML5(L,
										   index,
										   targetPlatform,
										   targetPlatformVersion,
										   appName,
										   version,
										   certificatePath,
										   projectPath,
										   dstPath,
										   sdkPath,
										   customBuildId,
										   templateType );
	}
	else
#endif
#if defined(CORONABUILDER_LINUX)
		if (targetPlatform == TargetDevice::kLinuxPlatform)
	{
		result = CreatePackagerParamsLinux(L,
										   index,
										   targetPlatform,
										   targetPlatformVersion,
										   appName,
										   version,
										   certificatePath,
										   projectPath,
										   dstPath,
										   sdkPath,
										   customBuildId,
										   templateType );
	}
	else
#endif
	{
		fprintf(stderr, "CoronaBuilder: building for %s is not supported on this operating system\n", TargetDevice::StringForPlatform(targetPlatform));
	}

	lua_settop( L, top );

	return result;
}

PlatformAppPackager*
AppPackagerFactory::CreatePackager( lua_State *L, int index, TargetDevice::Platform targetPlatform ) const
{
	PlatformAppPackager *result = NULL;

	switch ( targetPlatform )
	{
#if defined(CORONABUILDER_IOS)
	case TargetDevice::kIPhonePlatform:
		{
			result = new IOSAppPackager( fServices );
		}
			break;
#endif

#if defined(CORONABUILDER_TVOS)
	case TargetDevice::kTVOSPlatform:
		{
			result = new TVOSAppPackager( fServices );
		}
			break;
#endif

#if defined(CORONABUILDER_ANDROID)
	case TargetDevice::kAndroidPlatform:
		case TargetDevice::kKindlePlatform:
		{
			result = new AndroidAppPackager( fServices, GetResourceDirectory() );
		}
			break;
#endif

#if defined(CORONABUILDER_OSX)
		case TargetDevice::kOSXPlatform:
		{
			result = new OSXAppPackager( fServices );
		}
			break;
#endif

#if defined(CORONABUILDER_WIN32)
		case TargetDevice::kWin32Platform:
		{
			result = new Win32AppPackager( fServices );
		}
			break;
#endif

#if defined(CORONABUILDER_HTML5)
		case TargetDevice::kWebPlatform:
		{
			result = new WebAppPackager( fServices );
		}
			break;
#endif

#if defined(CORONABUILDER_LINUX)
		case TargetDevice::kLinuxPlatform:
		{
			result = new LinuxAppPackager( fServices );
		}
			break;
#endif

		default:
			break;
	}
	
	return result;
}

const char *
AppPackagerFactory::GetResourceDirectory() const
{
#if defined(Rtt_MAC_ENV)
	return GetResourceDirectoryOSX();
#elif defined(Rtt_WIN_ENV)
	return GetResourceDirectoryWin();
#endif
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

