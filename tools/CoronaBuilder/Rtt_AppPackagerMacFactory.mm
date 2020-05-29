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

#import <Foundation/Foundation.h>
#import "AppleSigningIdentityController.h"
#import "XcodeToolHelper.h"

#include "Rtt_AndroidAppPackager.h"
#include "Rtt_IOSAppPackager.h"
#include "Rtt_TVOSAppPackager.h"
#include "Rtt_OSXAppPackager.h"

#include "Rtt_TargetAndroidAppStore.h"
#include "Rtt_AndroidSupportTools.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#ifdef NOT_USED
static const char kAttachmentKey[] = "attachment";
static const char kDeviceHardwareKey[] = "deviceHardware";
static const char kDeviceOSKey[] = "deviceOS";

static const char kClientDeviceIdKey[] = "clientDeviceId";
static const char kClientVersionKey[] = "clientVersion";
static const char kClientOSKey[] = "clientOS";
static const char kUsesLaunchPadKey[] = "usesLaunchPad";
static const char kLanguageKey[] = "language";
#endif // NOT_USED
	
AppPackagerParams*
AppPackagerFactory::CreatePackagerParamsInternal(
	lua_State *L,
	int index,
	TargetDevice::Platform targetPlatform,
	TargetDevice::Version targetPlatformVersion,
	const char *appName,
	const char *version,
	const char *certificatePath,
	const char *projectPath,
	const char *dstPath,
	const char *sdkPath,
	const char *customBuildId,
	const char *templateType ) const
{
	AppPackagerParams *result = NULL;

	if ( certificatePath )
	{
		NSString *path = [NSString stringWithUTF8String:certificatePath];
		if ( ! [[NSFileManager defaultManager] fileExistsAtPath:path] )
		{
			fprintf( stderr, "ERROR: Could not find certificate file: '%s'\n", [path UTF8String] );
			return NULL;
		}
	}

	if (sdkPath == NULL || *sdkPath == '\0')
	{
		sdkPath = [[XcodeToolHelper getXcodePath] UTF8String];
	}

	switch ( targetPlatform )
	{
#if defined(CORONABUILDER_IOS)
		case TargetDevice::kIPhonePlatform:
		{
			S32 targetDevice = TargetDevice::kIOSUniversal;
			if ( templateType )
			{
				if ( 0 == strcasecmp( templateType, "iphone" ) )
				{
					targetDevice = TargetDevice::kIPhone;
				}
				else if ( 0 == strcasecmp( templateType, "ipad" ) )
				{
					targetDevice = TargetDevice::kIPad;
				}
				else if ( 0 == strcasecmp( templateType, "ios-simulator" ) )
				{
					targetDevice = TargetDevice::kIOSUniversalXCodeSimulator;
				}
				else if ( 0 == strcasecmp( templateType, "iphone-simulator" ) )
				{
					targetDevice = TargetDevice::kIPhoneXCodeSimulator;
				}
				else if ( 0 == strcasecmp( templateType, "ipad-simulator" ) )
				{
					targetDevice = TargetDevice::kIPadXCodeSimulator;
				}
			}

			if ( ! certificatePath )
			{
				fprintf( stderr, "ERROR: Missing 'certificatePath' in build params\n" );
				return NULL;
			}

			NSString *provisionFile = [NSString stringWithUTF8String:certificatePath];

			bool isDistributionBuild = [AppleSigningIdentityController hasProvisionedDevices:provisionFile];

			IOSAppPackager packager( fServices );

			const char *appBundleId = packager.GetBundleId( certificatePath, appName );

			if ( ! customBuildId )
			{
				packager.ReadBuildSettings( projectPath );
				customBuildId = packager.GetCustomBuildId();
			}

			NSString *commonName = nil;
			NSString *identity = [AppleSigningIdentityController signingIdentity:provisionFile commonName:&commonName];

			result = new IOSAppPackagerParams(
											  appName,
											  version,
											  [identity UTF8String],
											  certificatePath,
											  projectPath,
											  dstPath,
											  sdkPath,
											  targetPlatform,
											  targetPlatformVersion,
											  targetDevice,
											  customBuildId,
											  NULL,
											  appBundleId,
											  isDistributionBuild );
		}
		break;
#endif // defined(CORONABUILDER_IOS)
			
#if defined(CORONABUILDER_TVOS)
		case TargetDevice::kTVOSPlatform:
		{
			S32 targetDevice = TargetDevice::kAppleTV;

			if ( ! certificatePath )
			{
				fprintf( stderr, "ERROR: Missing 'certificatePath' in build params\n" );
				return NULL;
			}

			NSString *provisionFile = [NSString stringWithUTF8String:certificatePath];

			bool isDistributionBuild = [AppleSigningIdentityController hasProvisionedDevices:provisionFile];

			TVOSAppPackager packager( fServices );

			const char *appBundleId = packager.GetBundleId( certificatePath, appName );

			if ( ! customBuildId )
			{
				packager.ReadBuildSettings( projectPath );
				customBuildId = packager.GetCustomBuildId();
			}

			NSString *commonName = nil;
			NSString *identity = [AppleSigningIdentityController signingIdentity:provisionFile commonName:&commonName];

			result = new TVOSAppPackagerParams(
											  appName,
											  version,
											  [identity UTF8String],
											  certificatePath,
											  projectPath,
											  dstPath,
											  sdkPath,
											  targetPlatform,
											  targetPlatformVersion,
											  targetDevice,
											  customBuildId,
											  NULL,
											  appBundleId,
											  isDistributionBuild );
		}
		break;
#endif // defined(CORONABUILDER_TVOS)

#if defined(CORONABUILDER_ANDROID)
		case TargetDevice::kAndroidPlatform:
		case TargetDevice::kKindlePlatform:
		{
			// WARNING: Change value to lua_checkstack
			// if you increase the number of lua_getfield() calls!!!
			lua_checkstack( L, 5 );
			int top = lua_gettop( L );

			// Default to Debug.keystore
			NSString *path = [[NSBundle mainBundle] pathForResource:@"debug" ofType:@"keystore"];
			const char *keystorePath = [path UTF8String];
			const char *keystorePassword = "android";
			const char *keystoreAlias = "androiddebugkey";
			const char *keystoreAliasPassword = "android";

			bool isDistributionBuild = false;

			if ( certificatePath )
			{
				isDistributionBuild = true;
				keystorePath = certificatePath;

				lua_getfield( L, index, "keystorePassword" );
				keystorePassword = lua_tostring( L, -1 );
				if (keystorePassword == NULL)
				{
					keystorePassword = "";
				}

				lua_getfield( L, index, "keystoreAlias" );
				keystoreAlias = lua_tostring( L, -1 );
				if (keystoreAlias == NULL)
				{
					keystoreAlias = "";
				}

				lua_getfield( L, index, "keystoreAliasPassword" );
				keystoreAliasPassword = lua_tostring( L, -1 );
				if (keystoreAliasPassword == NULL)
				{
					keystoreAliasPassword = "";
				}

			}

			lua_getfield( L, index, "androidVersionCode" );
			int androidVersionCode = (int) lua_tointeger( L, -1 );

			lua_getfield( L, index, "androidAppPackage" );
			const char *origAndroidAppPackage = lua_tostring( L, -1 );
			char *androidAppPackage = NULL;
			if (origAndroidAppPackage != NULL)
			{
				androidAppPackage = strdup(origAndroidAppPackage);
				CHECK_VALUE( androidAppPackage, "androidAppPackage" );

				// Sanitize the package name by replacing bad characters with underscores
				for (int i = 0; i < strlen(androidAppPackage); i++)
				{
					if (! isalnum(androidAppPackage[i]) && androidAppPackage[i] != '.')
					{
						androidAppPackage[i] = '_';
					}
				}

				if (strcmp(origAndroidAppPackage, androidAppPackage) != 0)
				{
					fprintf( stderr, "WARNING: androidAppPackage '%s' sanitized to '%s'\n", origAndroidAppPackage, androidAppPackage );
				}

				NSString *script = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"AndroidValidation.lua"];
				lua_State* L1 = Rtt_AndroidSupportTools_NewLuaState( [script fileSystemRepresentation] );
				bool isValidPackageName = Rtt_AndroidSupportTools_IsAndroidPackageName( L1, androidAppPackage );
				Rtt_AndroidSupportTools_CloseLuaState(L1);

				if (! isValidPackageName)
				{
					fprintf( stderr, "ERROR: androidAppPackage '%s' is not a valid Android package name - see http://en.wikipedia.org/wiki/Java_package#Package_naming_conventions\n", origAndroidAppPackage );

					return NULL;
				}
			}
			else
			{
				fprintf(stderr, "ERROR: missing required 'androidAppPackage' setting\n");

				return NULL;
			}

			AndroidAppPackager packager( fServices, NULL );
			if ( ! customBuildId )
			{
				packager.ReadBuildSettings( projectPath );
				customBuildId = packager.GetCustomBuildId();
			}
			
			//Temporary hack just to get build working
			const char *storeTarget = Rtt::TargetAndroidAppStore::kGoogle.GetStringId();
			if (targetPlatform == TargetDevice::kKindlePlatform)
				storeTarget = Rtt::TargetAndroidAppStore::kAmazon.GetStringId();

			result = new AndroidAppPackagerParams(
				appName,
				version,
				NULL,
				NULL,
				projectPath,
				dstPath,
				NULL,
				targetPlatform,
				storeTarget,
				targetPlatformVersion, // TargetDevice::kAndroidOS2_2,
				customBuildId,
				NULL,
				androidAppPackage,
				isDistributionBuild,
				keystorePath,
				keystorePassword,
				keystoreAlias,
				keystoreAliasPassword,
				androidVersionCode );

			lua_settop( L, top );

		}
		break;
#endif // defined(CORONABUILDER_ANDROID)

#if defined(CORONABUILDER_OSX)
		case TargetDevice::kOSXPlatform:
		{
			S32 targetDevice = -1;		// TODO: add OSX/Windows devices
			OSXAppPackager packager( fServices );
			bool isDistributionBuild = false;
			NSString *identity = nil;
			const char *appBundleId = packager.GetBundleId( certificatePath, appName );

			if ( certificatePath != NULL )
			{
				NSString *provisionFile = [NSString stringWithUTF8String:certificatePath];
				isDistributionBuild = [AppleSigningIdentityController hasProvisionedDevices:provisionFile];

				NSString *commonName = nil;
				identity = [AppleSigningIdentityController signingIdentity:provisionFile commonName:&commonName];
			}

			result = new OSXAppPackagerParams(
											  appName,
											  version,
											  [identity UTF8String],
											  certificatePath,
											  projectPath,
											  dstPath,
											  sdkPath,
											  targetPlatform,
											  targetPlatformVersion,
											  targetDevice,
											  customBuildId,
											  NULL,
											  appBundleId,
											  isDistributionBuild );
		}
		break;
#endif // defined(CORONABUILDER_TVOS)

		default:
			// Rtt_ASSERT_NOT_REACHED();
			break;
	}

	if ( ! result )
	{
		fprintf( stderr, "ERROR: Platform %s is not supported by this version of CoronaBuilder\n", TargetDevice::StringForPlatform( targetPlatform ) );
	}

	return result;
}

PlatformAppPackager*
AppPackagerFactory::CreatePackager( lua_State *L, int index, TargetDevice::Platform targetPlatform ) const
{
	PlatformAppPackager *result = NULL;

	switch ( targetPlatform )
	{
		case TargetDevice::kIPhonePlatform:
		{
			result = new IOSAppPackager( fServices );
		}
		break;
			
		case TargetDevice::kTVOSPlatform:
		{
			result = new TVOSAppPackager( fServices );
		}
		break;

		case TargetDevice::kAndroidPlatform:
		case TargetDevice::kKindlePlatform:
		{
			NSString *resourcesDir = [[NSBundle mainBundle] resourcePath];
			result = new AndroidAppPackager( fServices, [resourcesDir UTF8String] );
		}
		break;

		case TargetDevice::kOSXPlatform:
		{
			result = new OSXAppPackager( fServices );
		}
		break;

		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

// This is a somewhat random place to define this functions but we need to query
// the bundle directory in Objective-C code and most of the app is C++
const char *
AppPackagerFactory::GetBundleDirectory() const
{
	return [[[NSBundle mainBundle] resourcePath] UTF8String];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

