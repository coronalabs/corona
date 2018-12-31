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

#include "Rtt_AndroidAppPackager.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_TargetAndroidAppStore.h"
#include "Rtt_WebServicesSession.h"
#include "Rtt_FileSystem.h"
#include "Rtt_JavaHost.h"

#include "ListKeyStore.h"

#include <string>

#if defined(Rtt_WIN_ENV) && !defined(Rtt_LINUX_ENV)
#include "stdafx.h"
#include "WinString.h"
#include "Interop/Ipc/CommandLine.h"
#if !defined( Rtt_NO_GUI )
#include "Simulator.h"
#endif
#endif

#if USE_JNI
#include "AntHost.h"
#endif

extern "C"
{
#	include "lfs.h"
}


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#define kDefaultNumBytes 1024

// This is set on the server. This also doesn't belong here.
#define MAX_PERMISSIONS 9

AndroidAppPackagerParams::AndroidAppPackagerParams(
	const char* appName,
	const char* versionName,
	const char* identity,
	const char* provisionFile,
	const char* srcDir,
	const char* dstDir,
	const char* sdkRoot,
	TargetDevice::Platform targetPlatform,
	const char * targetAppStoreName,
	S32 targetVersion,
	const char * customBuildId,
	const char * productId,
	const char * appPackage,
	bool isDistributionBuild,
	const char * keyStore,
	const char * storePassword,
	const char * keyAlias,
	const char * aliasPassword,
    U32 versionCode
)
 :	AppPackagerParams(
		appName, versionName, identity, provisionFile, srcDir, dstDir, sdkRoot,
		targetPlatform, targetAppStoreName, targetVersion, TargetDevice::kAndroidGenericDevice,
		customBuildId, productId, appPackage, isDistributionBuild ),
	fVersionCode(versionCode)
{
	fKeyStore.Set( keyStore );
	fKeyStorePassword.Set( storePassword );
	fKeyAlias.Set( keyAlias );
	fKeyAliasPassword.Set( aliasPassword );
}

void
AndroidAppPackagerParams::Print()
{
	Super::Print();
	fprintf( stderr,
		"\tKeystore path: '%s'\n"
		"\tKeystore alias: '%s'\n"
		"\tAndroid version code: '%x'\n"
		"\tAndroid app package: '%s'\n",
		GetAndroidKeyStore(),
		GetAndroidKeyAlias(),
		GetVersionCode(),
		GetAppPackage() );
}

	
// ----------------------------------------------------------------------------

// TODO: Move create_build_properties.lua out of librtt and into rtt_player in XCode
// Current issue with doing that is this lua file needs to be precompiled into C
// via custom build step --- all .lua files in librtt already do that, so we're
// taking a shortcut for now by putting it under librtt.

// create_build_properties.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in a generated .cpp file. The file also contains the definition of the
// following function which loads the bytecodes via luaL_loadbuffer.
int luaload_create_build_properties(lua_State* L);

AndroidAppPackager::AndroidAppPackager( const MPlatformServices & services, const char * resourcesDir )
:	PlatformAppPackager( services, TargetDevice::kAndroidPlatform ),
	fResourcesDir( & services.Platform().GetAllocator(), resourcesDir ),
	fIsUsingExpansionFile( false )
//	,
//	fPermissions( & services.Platform().GetAllocator() ),
//	fVersionCode( & services.Platform().GetAllocator() ),
//	fDefaultOrientation( DeviceOrientation::kUnknown ),
//	fSupportsOrientationChange( false )
{
    Lua::RegisterModuleLoader( fVM, "lfs", luaopen_lfs );
    Lua::RegisterModuleLoader( fVM, "lpeg", luaopen_lpeg ); // json depends on lpeg
	Lua::RegisterModuleLoader( fVM, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( fVM, "json", Lua::Open< luaload_json > );

	Lua::DoBuffer( fVM, & luaload_create_build_properties, NULL );
}

AndroidAppPackager::~AndroidAppPackager()
{
}

static const char *
versionToString( int version )
{
	switch ( version )
	{
		case TargetDevice::kAndroidOS1_5:
			return "1.5";
		case TargetDevice::kAndroidOS1_6:
			return "1.6";
		case TargetDevice::kAndroidOS2_0_1:
			return "2.0";
		case TargetDevice::kAndroidOS2_1:
			return "2.1";
		case TargetDevice::kAndroidOS2_2:
			return "2.2";
		case TargetDevice::kAndroidOS2_3_3:
			return "2.3.3";
		case TargetDevice::kAndroidOS4_0_3:
			return "4.0.3";
		default:
			return "unknown";
	}
}

std::string
AndroidAppPackager::EscapeArgument(std::string arg)
{
	std::string result = arg;

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)

	// On macOS escape shell special characters in the strings by replacing single quotes with "'\''" and
	// then enclosing in single quotes
	ReplaceString(result, "'", "'\\''");	// escape single quotes
	result = "'" + result + "'";

#else // Windows

	// On Windows escape shell special characters in the strings by replacing double quotes with "\"" and
	// then enclosing in double quotes
	ReplaceString(result, "\"", "\\\"");	// escape double quotes
	result = "\"" + result + "\"";

#endif
	
	return result;
}


int
AndroidAppPackager::Build( AppPackagerParams * params, WebServicesSession & session, const char * tmpDirBase )
{
	int result = WebServicesSession::kBuildError;
	time_t startTime = time(NULL);

    const char tmpTemplate[] = "CLtmpXXXXXX";
	char tmpDir[kDefaultNumBytes]; Rtt_ASSERT( kDefaultNumBytes > ( strlen( tmpDirBase ) + strlen( tmpTemplate ) ) );
	snprintf(tmpDir, kDefaultNumBytes, "%s" LUA_DIRSEP "%s", tmpDirBase, tmpTemplate);

    // This is not as foolproof as mkdtemp() but has the advantage of working on Win32
    if ( mkdir( mktemp(tmpDir) ) )
	{
		char* inputFile = Prepackage( params, tmpDir );

		if ( inputFile )
		{
			const char kOutputName[] = "output.zip";
			size_t tmpDirLen = strlen( tmpDir );
			size_t outputFileLen = tmpDirLen + sizeof(kOutputName) + sizeof( LUA_DIRSEP );
			char* outputFile = (char*)malloc( outputFileLen );
			snprintf( outputFile, outputFileLen, "%s" LUA_DIRSEP "%s", tmpDir, kOutputName );

			// Send params/input.zip via web api
			result = session.BeginBuild( params, inputFile, outputFile );

			if ( WebServicesSession::kNoError == result )
			{
				const AndroidAppPackagerParams * androidParams = (const AndroidAppPackagerParams *) params;

				// Run Android specific post package script

				std::string liveBuildStr;
				std::string liveBuildAssetDirStr;

				if (params->IsLiveBuild())
				{
					liveBuildStr = "YES";
					liveBuildAssetDirStr = "corona_live_build_app_";
				}
				else
				{
					liveBuildStr = "NO";
				}

				std::string javaCmd;
#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
				javaCmd = "/usr/bin/java";
#else // Windows
				std::string jdkPath = Rtt::JavaHost::GetJdkPath();

				javaCmd = jdkPath + "\\bin\\java.exe";
#endif

				const char kCmdFormat[] = "\"%s\" -Djava.class.path=%s org.apache.tools.ant.launch.Launcher %s -DTEMP_DIR=%s -DSRC_DIR=%s -DDST_DIR=%s -DUSER_APP_NAME=%s -DBUNDLE_DIR=%s -DLIVE_BUILD=%s -DLIVE_BUILD_ASSET_DIR=%s -DKS=%s -DKP=%s -DKA=%s -DAP=%s -DAV=%s %s -f %s/build.xml build";

				// const char kCmdFormat[] = "javaw -Djava.class.path=\"%s\" org.apache.tools.ant.launch.Launcher %s -DTEMP_DIR=\"%s\" -DSRC_DIR=\"%s\" -DDST_DIR=\"%s\" -DUSER_APP_NAME=\"%s\" -DBUNDLE_DIR=\"%s\" -DLIVE_BUILD=\"%s\" -DLIVE_BUILD_ASSET_DIR=\"%s\" -DKS=\"%s\" -DKP=\"%s\" -DKA=\"%s\" -DAP=\"%s\" -DAV=\"%s\" %s -f \"%s/build.xml\" build";

				char cmdBuf[20480];
				String appFileName;
				PlatformAppPackager::EscapeFileName( params->GetAppName(), appFileName );

				std::string tmpDirStr = EscapeArgument(tmpDir);
				std::string keystoreStr = EscapeArgument(androidParams->GetAndroidKeyStore());
				std::string keystorePasswordStr = EscapeArgument(androidParams->GetAndroidKeyStorePassword());
				std::string keyaliasStr = EscapeArgument(androidParams->GetAndroidKeyAlias());
				std::string srcDirStr = EscapeArgument(params->GetSrcDir());
				std::string dstDirStr = EscapeArgument(params->GetDstDir());
				std::string appNameStr = EscapeArgument(appFileName.GetString());
				std::string resourcesDirStr = EscapeArgument(fResourcesDir.GetString());
				std::string keyaliasPasswordStr;
				std::string jarPathStr;

				jarPathStr.append(fResourcesDir.GetString());
				jarPathStr.append("/");
				jarPathStr.append("ant.jar");
#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
				jarPathStr.append(":");
#else // Windows
				jarPathStr.append(";");
#endif
				jarPathStr.append(fResourcesDir.GetString());
				jarPathStr.append("/");
				jarPathStr.append("ant-launcher.jar" );
				jarPathStr = EscapeArgument(jarPathStr);

				if (androidParams->GetAndroidKeyAliasPassword() != NULL)
				{
					keyaliasPasswordStr = EscapeArgument(androidParams->GetAndroidKeyAliasPassword());
				}

				const char *antDebugFlag = "-q";
				String debugBuildProcessPref;
				int debugBuildProcess = 0;
				fServices.GetPreference( "debugBuildProcess", &debugBuildProcessPref );

				if (! debugBuildProcessPref.IsEmpty())
				{
					debugBuildProcess = (int) strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
				}
				else
				{
					debugBuildProcess = 0;
				}

				if (debugBuildProcess >= 2)
				{
					antDebugFlag = "-v";
				}
				if (debugBuildProcess > 4)
				{
					antDebugFlag = "-d";
				}

				char expansionFileName[255] = { 0 };
				if (fIsUsingExpansionFile &&
					params->GetTargetAppStoreName() &&
					!strcmp(params->GetTargetAppStoreName(), TargetAndroidAppStore::kGoogle.GetStringId()))
				{
					snprintf(expansionFileName, sizeof(expansionFileName) - 1, "-DEXPANSION_FILE_NAME=\"main.%d.%s.obb\"",
							 ((const AndroidAppPackagerParams*)params)->GetVersionCode(), params->GetAppPackage());
				}

				snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat,
						javaCmd.c_str(),
						jarPathStr.c_str(),
						antDebugFlag,
						tmpDirStr.c_str(),
						srcDirStr.c_str(),
						dstDirStr.c_str(),
						appNameStr.c_str(),
						resourcesDirStr.c_str(),
						liveBuildStr.c_str(),
						liveBuildAssetDirStr.c_str(),
						keystoreStr.c_str(),
						keystorePasswordStr.c_str(),
						keyaliasStr.c_str(),
						keyaliasPasswordStr.c_str(),
						versionToString( params->GetTargetVersion() ),
						expansionFileName,
						resourcesDirStr.c_str() );

				if (debugBuildProcess > 1)
				{
					// Obfuscate passwords
					std::string placeHolder = EscapeArgument("XXXXXX");
					std::string sanitizedCmdBuf = cmdBuf;

					if (keystorePasswordStr.length() > 0)
					{
						ReplaceString(sanitizedCmdBuf, keystorePasswordStr, placeHolder);
					}

					if (keyaliasPasswordStr.length() > 0)
					{
						ReplaceString(sanitizedCmdBuf, keyaliasPasswordStr, placeHolder);
					}

					Rtt_Log("Build: running: %s\n", sanitizedCmdBuf.c_str());
				}

#if defined(Rtt_MAC_ENV)
				result = system( cmdBuf );
#elif defined(Rtt_LINUX_ENV)
				result = system( cmdBuf );
				if (result == 0)
				{
					std::string outputBaseApkStr = tmpDir;
					outputBaseApkStr.append("/MyCoronaActivity-signed-aligned.apk");
					std::string dstDirStrAppName = params->GetDstDir();
					appNameStr = dstDirStrAppName.append("/").append(params->GetAppName()).append(".apk");

					int rc = Rtt_CopyFile(outputBaseApkStr.c_str(), appNameStr.c_str());
					if (rc == false)
					{
						result = 1;
						Rtt_Log("ERROR: Failed to copy built apk to destination directory.");
					}
				}
#else // Windows

				WinString cmdBufStr;
				cmdBufStr.SetUTF8(cmdBuf);
				std::wstring utf16CommandLine(cmdBufStr.GetUTF16());

				Interop::Ipc::CommandLine::SetOutputCaptureEnabled(true);
				Interop::Ipc::CommandLineRunResult cmdResult = Interop::Ipc::CommandLine::RunUntilExit(utf16CommandLine.c_str());
				result = cmdResult.GetExitCode();

				if (result == 0)
				{
					std::string outputBaseApkStr = tmpDir;
					outputBaseApkStr.append("\\MyCoronaActivity-signed-aligned.apk");
					std::string dstDirStrAppName = params->GetDstDir();
					appNameStr = dstDirStrAppName.append("\\").append(params->GetAppName()).append(".apk");

					if (!Rtt_CopyFile(outputBaseApkStr.c_str(), appNameStr.c_str()))
					{
						result = 1;
						Rtt_Log("ERROR: Failed to copy built apk to destination directory.");
					}
				}

#if !defined( Rtt_NO_GUI )
				CSimulatorApp *pApp = ((CSimulatorApp *)AfxGetApp());
				if (pApp != NULL && pApp->IsStopBuildRequested())
				{
					// A request to stop the build was made while the Java was running
					result = -1;
				}
#endif

				if (debugBuildProcess > 1)
				{
					std::string output = cmdResult.GetOutput();

					Rtt_Log("%s", output.c_str());
				}

#endif


#if USE_JNI
				AntHost	antTask;

				char buildFilePath[1024];
				snprintf( buildFilePath, sizeof(buildFilePath), "%s%sbuild.xml", fResourcesDir.GetString(), LUA_DIRSEP );

				String appFileName;
				PlatformAppPackager::EscapeFileName( params->GetAppName(), appFileName );

				antTask.SetProperty( "TEMP_DIR", tmpDir );
				antTask.SetProperty( "SRC_DIR", params->GetSrcDir() );
				antTask.SetProperty( "DST_DIR", params->GetDstDir() );
				antTask.SetProperty( "USER_APP_NAME", appFileName.GetString() );
				antTask.SetProperty( "BUNDLE_DIR", fResourcesDir.GetString() );
				antTask.SetProperty( "LIVE_BUILD", liveBuildStr.c_str() );
				antTask.SetProperty( "LIVE_BUILD_ASSET_DIR", liveBuildAssetDirStr.c_str() );
				antTask.SetProperty( "KS", androidParams->GetAndroidKeyStore() );
				antTask.SetProperty( "KP", androidParams->GetAndroidKeyStorePassword() );
				antTask.SetProperty( "KA", androidParams->GetAndroidKeyAlias() );
				antTask.SetProperty( "AP", androidParams->GetAndroidKeyAliasPassword() );
				antTask.SetProperty( "AV", versionToString( params->GetTargetVersion() ) );

				String debugBuildProcessPref;
				int debugBuildProcess = 0;
				fServices.GetPreference( "debugBuildProcess", &debugBuildProcessPref );

				if (! debugBuildProcessPref.IsEmpty())
				{
					debugBuildProcess = (int) strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
				}
				else
				{
					debugBuildProcess = 0;
				}

				char expansionFileName[255];
				if (fIsUsingExpansionFile &&
				    params->GetTargetAppStoreName() &&
				    !strcmp(params->GetTargetAppStoreName(), TargetAndroidAppStore::kGoogle.GetStringId()))
				{
					snprintf(
						expansionFileName, sizeof(expansionFileName) - 1, "main.%d.%s.obb",
						((const AndroidAppPackagerParams*)params)->GetVersionCode(), params->GetAppPackage());
					antTask.SetProperty( "EXPANSION_FILE_NAME", expansionFileName );
				}

				String antResult;
				int antCode;

				antCode = antTask.AntCall( buildFilePath, "build", debugBuildProcess, &antResult );

				// Translate "antCode" to a "WebServicesSession" return code
				if (antCode != 1)
				{
					Rtt_TRACE_SIM( ( "ANT build failed: %s\n", antResult.GetString() ) );

					result = WebServicesSession::kLocalPackagingError;

					String tmpString;

					tmpString.Set("Failed to build APK\n\n");
					tmpString.Append(antResult.GetString());

					params->SetBuildMessage(tmpString.GetString());
				}
#endif // USE_JNI
			}

			free( outputFile );
			free( inputFile );
		}
		// Clean up intermediate files
        rmdir( tmpDir );
	}
	else
	{
		// Note that the failing mkdir() that brought us here is a member of the AndroidAppPackager class
		String tmpString;

		tmpString.Set("AndroidAppPackager::Build: failed to create temporary directory\n\n");
		tmpString.Append(tmpDir);
		tmpString.Append("\n");

		Rtt_TRACE_SIM( ( "%s", tmpString.GetString() ) );
		params->SetBuildMessage(tmpString.GetString());
	}

    // Indicate status in the console
    if (WebServicesSession::kNoError == result)
    {
		// This isn't an exception but Rtt_Log() is only defined for debug builds
        Rtt_LogException("Android build succeeded in %ld seconds", (time(NULL) - startTime));
    }
    else
    {
		if (params->GetBuildMessage() == NULL)
		{
			// If we don't already have a more precise error, use the XMLRPC layer's error message
			params->SetBuildMessage(&session != NULL ? session.ErrorMessage() : "Failed to Build" );
		}

		Rtt_LogException("Android build failed (%d) after %ld seconds", result, (time(NULL) - startTime));
    }

	return result;
}

bool
AndroidAppPackager::VerifyConfiguration() const
{
	// TODO: Add code to check existence of various utilities
	return true;
}

/// Called when the "build.settings" file is being read.
/// @param L Pointer to the Lua state that has loaded the build settings table.
/// @param index The index to the "settings" table in the Lua stack.
void
AndroidAppPackager::OnReadingBuildSettings( lua_State *L, int index )
{
	// Fetch the "usesExpansionFile" flag in the "build.settings" file, if provided.
	lua_getfield( L, index, "android" );
	if (lua_istable( L, -1 ))
	{
		lua_getfield( L, -1, "usesExpansionFile" );
		if (lua_isboolean( L, -1 ))
		{
			fIsUsingExpansionFile = lua_toboolean( L, -1 ) ? true : false;
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}

bool
AndroidAppPackager::CreateBuildProperties( const AppPackagerParams& params, const char *tmpDir )
{
	lua_State *L = fVM;
	lua_getglobal( L, "androidCreateProperties" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
	lua_pushstring( L, tmpDir );
	lua_pushstring( L, params.GetAppPackage() );
	lua_pushstring( L, params.GetSrcDir() );
	lua_pushinteger( L, ((AndroidAppPackagerParams&)params).GetVersionCode() );
	lua_pushstring( L, params.GetVersion() );
	lua_pushstring( L, params.GetTargetAppStoreName() );

	bool result = Rtt_VERIFY( 0 == Lua::DoCall( L, 6, 1 ) );
	if ( ! lua_isnil( L, -1 ) )
	{
		Rtt_TRACE_SIM( ( "ERROR: Could not create build.properties:\n\t%s\n", lua_tostring( L, -1 ) ) );
		result = false;
	}
	return result;
}

char *
AndroidAppPackager::Prepackage( AppPackagerParams * params, const char * tmpDir )
{
	char* result = NULL;
	int iresult = -1;

	Rtt_ASSERT( params );

	// Convert build.settings into build.properties
	// And run Android specific pre package script
	String debugBuildProcessPref;
	int debugBuildProcess = 0;
	fServices.GetPreference("debugBuildProcess", &debugBuildProcessPref);

	if (!debugBuildProcessPref.IsEmpty())
	{
		debugBuildProcess = (int)strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
	}
	else
	{
		debugBuildProcess = 0;
	}

	if (debugBuildProcess > 0)
	{
		Rtt_Log("Prepackage: Compiling Lua ...");
	}

	if ( CreateBuildProperties( * params, tmpDir ) && CompileScripts( params, tmpDir ) )
	{
		if (! Rtt_StringIsEmpty(GetSplashImageFile()))
		{
			// Note: this logic (copying the splash screen image to the root of the tmp directory) needs
			// to match the logic in buildsys-worker/tools/build3_output_android.sh on the build server
			String splashImageFilename, tmpSplashPath, tmpFilename, tmpDstFilename;
			splashImageFilename.Set(params->GetSrcDir());
			splashImageFilename.AppendPathComponent(GetSplashImageFile());
			tmpSplashPath.Set(GetSplashImageFile());
			tmpFilename.Set(tmpSplashPath.GetLastPathComponent());
			tmpDstFilename.Set(tmpDir);
			tmpDstFilename.AppendPathComponent(tmpFilename);

			if (! Rtt_CopyFile( splashImageFilename, tmpDstFilename ))
			{
				String tmpString;
				tmpString.Set("ERROR: failed to copy splashScreen.image file: ");
				tmpString.Append(GetSplashImageFile());
				//Rtt_Log("%s", tmpString.GetString());
				params->SetBuildMessage( tmpString );

				return NULL;
			}
		}

		std::string liveBuildStr;
		std::string liveBuildTMPManifestFileStr;

		if (params->IsLiveBuild())
		{
			liveBuildStr = "YES";
			liveBuildTMPManifestFileStr = tmpDir;
			liveBuildTMPManifestFileStr.append("/output/assets/_corona_live_build_manifest.txt");
			liveBuildTMPManifestFileStr = EscapeArgument(liveBuildTMPManifestFileStr);
		}
		else
		{
			liveBuildStr = "NO";
		}

		std::string javaCmd;
#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
		javaCmd = "/usr/bin/java";
#else // Windows
		std::string jdkPath = Rtt::JavaHost::GetJdkPath();

		javaCmd = jdkPath + "\\bin\\java.exe";
#endif

		const char kCmdFormat[] = "\"%s\" -Djava.class.path=%s org.apache.tools.ant.launch.Launcher %s -DTEMP_DIR=%s -DSRC_DIR=%s -DBUNDLE_DIR=%s -DLIVE_BUILD=%s -DLIVE_BUILD_MANIFEST_FILE=%s -f %s/build.xml build-input-zip";

		char cmdBuf[20480];

		std::string jarPathStr;
		std::string tmpDirStr = EscapeArgument(tmpDir);
		std::string srcDirStr = EscapeArgument(params->GetSrcDir());
		std::string resourcesDirStr = EscapeArgument(fResourcesDir.GetString());

		jarPathStr.append(fResourcesDir.GetString());
		jarPathStr.append("/");
		jarPathStr.append("ant.jar");

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
		jarPathStr.append(":");
#else // Windows
		jarPathStr.append(";");
#endif
		jarPathStr.append(fResourcesDir.GetString());
		jarPathStr.append("/");
		jarPathStr.append("ant-launcher.jar" );

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)
		jarPathStr.append(":");
#else // Windows
		jarPathStr.append(";");
#endif
		jarPathStr.append(fResourcesDir.GetString());
		jarPathStr.append("/");
		jarPathStr.append("AntLiveManifest.jar");
		jarPathStr = EscapeArgument(jarPathStr);

		const char *antDebugFlag = "-q";
		if (debugBuildProcess >= 2)
		{
			antDebugFlag = "-v";
		}
		if (debugBuildProcess > 4)
		{
			antDebugFlag = "-d";
		}

		snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat,
				 javaCmd.c_str(),
				 jarPathStr.c_str(),
				 antDebugFlag,
				 tmpDirStr.c_str(),
				 srcDirStr.c_str(),
				 resourcesDirStr.c_str(),
				 liveBuildStr.c_str(),
				 liveBuildTMPManifestFileStr.c_str(),
				 resourcesDirStr.c_str() );
		
		if (debugBuildProcess > 0)
		{
			Rtt_Log("Prepackage: Building server data file");
		}

		if (debugBuildProcess > 1)
		{
			Rtt_Log("Prepackage: running: %s\n", cmdBuf);
		}

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)

		iresult = system( cmdBuf );
		
#else // Windows

		WinString cmdBufStr;
		cmdBufStr.SetUTF8(cmdBuf);

		std::wstring utf16CommandLine(cmdBufStr.GetUTF16());

		Interop::Ipc::CommandLine::SetOutputCaptureEnabled(true);
		Interop::Ipc::CommandLineRunResult cmdResult = Interop::Ipc::CommandLine::RunUntilExit(utf16CommandLine.c_str());

		iresult = cmdResult.GetExitCode();

#if !defined( Rtt_NO_GUI )
		CSimulatorApp *pApp = ((CSimulatorApp *)AfxGetApp());
		if (pApp!= NULL && pApp->IsStopBuildRequested())
		{
			// A request to stop the build was made while the Java was running
			iresult = -1;
		}
#endif

		if (debugBuildProcess > 1)
		{
			std::string output = cmdResult.GetOutput();
			Rtt_Log("%s", output.c_str());
		}

#endif

#if USE_JNI
		AntHost	antTask;

		char buildFilePath[1024];
		snprintf( buildFilePath, sizeof(buildFilePath), "%s%sbuild.xml", fResourcesDir.GetString(), LUA_DIRSEP );

		antTask.SetProperty( "TEMP_DIR", tmpDir );
		antTask.SetProperty( "SRC_DIR", params->GetSrcDir() );
		antTask.SetProperty( "BUNDLE_DIR", fResourcesDir.GetString() );
		antTask.SetProperty( "LIVE_BUILD", liveBuildStr.c_str() );
		antTask.SetProperty( "LIVE_BUILD_MANIFEST_FILE", liveBuildTMPManifestFileStr.c_str() );

		String debugBuildProcessPref;
		int debugBuildProcess = 0;
		fServices.GetPreference( "debugBuildProcess", &debugBuildProcessPref );

		if (! debugBuildProcessPref.IsEmpty())
		{
			debugBuildProcess = (int) strtol(debugBuildProcessPref.GetString(), (char **)NULL, 10);
		}
		else
		{
			debugBuildProcess = 0;
		}

		String antResult;
		int antCode;

		antCode = antTask.AntCall( buildFilePath, "build-input-zip", debugBuildProcess, &antResult );
		Rtt_ASSERT( antCode == 1 );
		if ( antCode == 0 )
		{
			Rtt_TRACE_SIM( ( "ANT build-input-zip failed: %s", antResult.GetString() ) );
		}
		if ( antCode == 1 )
			iresult = 0;
#endif

		if ( iresult == 0 )
		{
			const char kDstName[] = "input.zip";
			size_t resultLen = strlen( tmpDir ) + strlen( kDstName ) + sizeof( LUA_DIRSEP );
			result = (char *) malloc( resultLen + 1 );
			snprintf( result, resultLen, "%s" LUA_DIRSEP "%s", tmpDir, kDstName );
		}
	}

	return result;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

