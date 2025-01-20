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

#include "Rtt_IOSAppPackager.h"
#include "Rtt_TVOSAppPackager.h"
#include "Rtt_OSXAppPackager.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#if defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS) || defined(CORONABUILD_OSX)

AppPackagerParams*
AppPackagerFactory::CreatePackagerParamsApple(
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

			if ( ! certificatePath  && (targetDevice == TargetDevice::kIOSUniversal ||
										targetDevice == TargetDevice::kIPhone ||
										targetDevice == TargetDevice::kIPad ))
			{
				fprintf( stderr, "ERROR: Missing 'certificatePath' in build params\n" );
				return NULL;
			}

			NSString *provisionFile = certificatePath?[NSString stringWithUTF8String:certificatePath]:nil;

			bool isDistributionBuild = certificatePath?[AppleSigningIdentityController hasProvisionedDevices:provisionFile]:NO;

			IOSAppPackager packager( fServices );

			const char *appBundleId = certificatePath?packager.GetBundleId( certificatePath, appName ):"com.solar2d.xcodesim";

			if ( ! customBuildId )
			{
				packager.ReadBuildSettings( projectPath );
				customBuildId = packager.GetCustomBuildId();
			}

			NSString *commonName = nil;
			NSString *identity = certificatePath?[AppleSigningIdentityController signingIdentity:provisionFile commonName:&commonName]:nil;

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

		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
	
	if ( ! result )
	{
		fprintf( stderr, "ERROR: Unsupported platform: %s\n", TargetDevice::StringForPlatform( targetPlatform ) );
	}
	
	lua_getfield(L, index, "customTemplate" );
	if(lua_type(L, -1) == LUA_TSTRING)
	{
		result->SetCustomTemplate(lua_tostring(L, -1));
	}
	lua_pop(L, 1);

	lua_getfield(L, index, "includeStandardResources");
	if(lua_type(L, -1) == LUA_TBOOLEAN)
	{
		result->SetIncludeStandardResources(lua_toboolean(L, -1));
	}
	lua_pop(L, 1);

	return result;
}

#endif // defined(CORONABUILDER_IOS) || defined(CORONABUILDER_TVOS) || defined(CORONABUILD_OSX)

#if defined(Rtt_MAC_ENV)
// This is the counterpart of GetResourceDirectoryWin() in Rtt_WIN_ENV builds

// This is a somewhat random place to define this function but we need to query
// the bundle directory in Objective-C code and most of the app is C++
const char *
AppPackagerFactory::GetResourceDirectoryOSX() const
{
	return [[XcodeToolHelper pathForResources] UTF8String];
}
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

