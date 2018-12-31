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

#include "Rtt_Win32AppPackager.h"
#include "Rtt_Win32AppPackagerParams.h"

#include "WinString.h"

#include <string.h>
#include <ctype.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#if defined(CORONABUILDER_ANDROID)

AppPackagerParams*
AppPackagerFactory::CreatePackagerParamsWin32(
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

		case TargetDevice::kWin32Platform:
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

			bool isDistributionBuild = false;

			const char *versionString = NULL;
			lua_getfield(L, index, "versionString");
			versionString = lua_tostring(L, -1);
			if (versionString == NULL)
			{
				versionString = "";
			}

			const char *dstPath = NULL;
			lua_getfield(L, index, "dstPath");
			dstPath = lua_tostring(L, -1);
			if (dstPath == NULL)
			{
				dstPath = "";
			}

			const char *projectPath = NULL;
			lua_getfield(L, index, "projectPath");
			projectPath = lua_tostring(L, -1);
			if (projectPath == NULL)
			{
				projectPath = "";
			}

			const char *exeFileName = NULL;
			lua_getfield(L, index, "exeFileName");
			exeFileName = lua_tostring(L, -1);
			if (exeFileName == NULL)
			{
				exeFileName = "";
			}

			const char *companyName = NULL;
			lua_getfield(L, index, "companyName");
			companyName = lua_tostring(L, -1);
			if (companyName == NULL)
			{
				companyName = "";
			}

			const char *copyright = NULL;
			lua_getfield(L, index, "copyright");
			copyright = lua_tostring(L, -1);
			if (copyright == NULL)
			{
				copyright = "";
			}

			const char *appDescription = NULL;
			lua_getfield(L, index, "appDescription");
			appDescription = lua_tostring(L, -1);
			if (appDescription == NULL)
			{
				appDescription = "";
			}
			Win32AppPackager packager( fServices );
			if ( ! customBuildId )
			{
				packager.ReadBuildSettings( projectPath );
				customBuildId = packager.GetCustomBuildId();
			}
			
			//Temporary hack just to get build working
			const char *storeTarget = NULL;

			// Configure the Win32 app packager's parameters.
			Rtt::Win32AppPackagerParams::CoreSettings paramsSettings{};
			WinString appNameTranscoder(appName);
			paramsSettings.AppName = appNameTranscoder.GetUTF8();

			WinString versionStringTranscoder(versionString);
			paramsSettings.VersionString = versionStringTranscoder.GetUTF8();

			WinString destinationDirectoryPathTranscoder(dstPath);
			String destinationDirectoryPathStr;
			destinationDirectoryPathStr.Set(destinationDirectoryPathTranscoder.GetUTF8());
			String lastPathComponent = destinationDirectoryPathStr.GetLastPathComponent();
			String appDirName = paramsSettings.AppName;
			appDirName.Append(".win32");
			if (Rtt_StringCompareNoCase(appDirName, lastPathComponent) != 0)
			{
				destinationDirectoryPathStr.AppendPathComponent(appDirName);
			}
			paramsSettings.DestinationDirectoryPath = destinationDirectoryPathStr;

			WinString sourceDirectoryPathTranscoder(projectPath);
			paramsSettings.SourceDirectoryPath = sourceDirectoryPathTranscoder.GetUTF8();

			WinString stringTranscoder;
			Rtt::Win32AppPackagerParams *params = new Win32AppPackagerParams(paramsSettings);

			stringTranscoder.SetUTF8(exeFileName);
			params->SetExeFileName(stringTranscoder.GetUTF8());

			stringTranscoder.SetUTF8(companyName);
			params->SetCompanyName(stringTranscoder.GetUTF8());

			stringTranscoder.SetUTF8(copyright);
			params->SetCopyrightString(stringTranscoder.GetUTF8());

			stringTranscoder.SetUTF8(appDescription);
			params->SetAppDescription(stringTranscoder.GetUTF8());

			params->SetIncludeBuildSettings(true);
			params->SetStripDebug(true);

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
			params.SetIncludeFusePlugins(false);
			params.SetUsesMonetization(false);
#endif
			params->SetRuntime(NULL); // NEEDSWORK:  runtimeEnvironmentPointer->GetRuntime());

			WinString projectDirectoryPath = projectPath;
			String utf8BuildSettingsPath;

			utf8BuildSettingsPath.Set(projectDirectoryPath.GetUTF8());
			utf8BuildSettingsPath.AppendPathComponent("build.settings");
			params->SetBuildSettingsPath(utf8BuildSettingsPath.GetString());

			result = params;

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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

