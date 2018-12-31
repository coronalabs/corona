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
	return [[[NSBundle mainBundle] resourcePath] UTF8String];
}
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

