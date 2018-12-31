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

#include "Rtt_AndroidAppPackager.h"

#include "Rtt_TargetAndroidAppStore.h"
#include "Rtt_AndroidSupportTools.h"

#include <string.h>
#include <ctype.h>

#if defined(Rtt_WIN_ENV)
#include "WinString.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#if defined(CORONABUILDER_ANDROID)

AppPackagerParams*
AppPackagerFactory::CreatePackagerParamsAndroid(
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

	switch ( targetPlatform )
	{
		case TargetDevice::kIPhonePlatform:
		break;
			
		case TargetDevice::kTVOSPlatform:
		break;

		case TargetDevice::kAndroidPlatform:
		case TargetDevice::kKindlePlatform:
		{
			// WARNING: Change value to lua_checkstack
			// if you increase the number of lua_getfield() calls!!!
			lua_checkstack( L, 5 );
			int top = lua_gettop( L );

			const char *resourcePath = NULL;

#if defined(Rtt_MAC_ENV)
			resourcePath = GetResourceDirectoryOSX();
#elif defined(Rtt_WIN_ENV)
			resourcePath = GetResourceDirectoryWin();
#endif

			// Default to Debug.keystore
			String keystorePathStr;
			keystorePathStr.Set(resourcePath);
			keystorePathStr.Append("/debug.keystore");
			const char *keystorePath = keystorePathStr.GetString();
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
				for (size_t i = 0; i < strlen(androidAppPackage); i++)
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

				String scriptPathStr;
#if defined(Rtt_WIN_ENV)
				scriptPathStr.Set(GetResourceDirectoryWin());
#elif defined(Rtt_MAC_ENV)
				scriptPathStr.Set(GetResourceDirectoryOSX());
#endif
				scriptPathStr.Append("/AndroidValidation.lua");
				lua_State* L1 = Rtt_AndroidSupportTools_NewLuaState( scriptPathStr.GetString() );
				if (L1 == NULL)
				{
					fprintf( stderr, "ERROR: Could not find script file '%s'\n", scriptPathStr.GetString() );
					return NULL;
				}

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

			lua_getfield( L, index, "androidStore" );
			const char* customStore = lua_tostring( L, -1 );
			if (customStore != NULL)
			{
				if(Rtt::TargetAndroidAppStore::GetByStringId(customStore) != NULL)
				{
					storeTarget = customStore;
				}
				else
				{
					fprintf( stderr, "WARNING: uknown androidStore: %s, defaulting to %s\n", customStore, storeTarget );
				}
			}

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

#endif // defined(CORONABUILDER_ANDROID)

#if defined(Rtt_WIN_ENV)
// This is the counterpart of GetResourceDirectoryOSX() in Rtt_MAC_ENV builds
const char *
AppPackagerFactory::GetResourceDirectoryWin() const
{
	static WinString resourceDir;
	if (resourceDir.GetLength() == 0)
	{
		resourceDir = _wgetenv(_T("CORONA_PATH"));
		resourceDir.Append("\\Resources");
	}
	return resourceDir.GetUTF8();
}
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

