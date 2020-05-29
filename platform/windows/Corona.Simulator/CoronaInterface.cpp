//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// All functions start with "app" by convention - not sure why, it was that way when I
// took it over.
// These are functions that are compiled in with Corona Simulator.exe
// CoronaDll\corona.cpp, .h contain functions exported from that DLL.

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "Core\Rtt_Build.h"
#include "Interop\SimulatorRuntimeEnvironment.h"
#include "Rtt_WinPlatformServices.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_PlatformAppPackager.h"
#include "Rtt_AndroidAppPackager.h"
#include "Rtt_WebAppPackager.h"
#include "Rtt_LinuxAppPackager.h"
#include "Rtt_WinAudioPlayer.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaFile.h"
#include "Rtt_LuaResource.h"
#include "Rtt_SimulatorAnalytics.h"
#include "WinString.h"
#include "WinGlobalProperties.h"
#include "CoronaInterface.h"
#include "Simulator.h"
#include "MessageDlg.h"
#include "Rtt_LaunchPad.h"
#include "Rtt_Runtime.h"
#include "Rtt_PlatformSurface.h"

static UINT GetStatusMessageResourceIdFor(int resultCode)
{
	UINT resourceId;

	switch (resultCode)
	{
		case 0:
			resourceId = IDS_BUILD_SUCCEEDED;
			break;
		default:
			resourceId = IDS_BUILD_FAILED;
			break;
	}
	return resourceId;
}

static void LogAnalytics(const char *target, const char *eventName, const char *key = NULL, const char *value = NULL)
{
	Rtt_ASSERT(GetWinProperties()->GetAnalytics() != NULL);
	Rtt_ASSERT(eventName != NULL && strlen(eventName) > 0);

	static std::map<std::string, std::string> keyValues;
	keyValues["target"] = target;

	if (key != NULL && strlen(key) > 0 && value != NULL)
	{
		keyValues[key] = value;
	}

	GetWinProperties()->GetAnalytics()->Log(eventName, keyValues);
}


// appAndroidBuild - login to server, showing password dialog if necessary,
// put parameters in AppAndroidPackager, set up temp directory, and BUILD APP
CBuildResult appAndroidBuild(
	Interop::SimulatorRuntimeEnvironment *pSim,
	const char *srcDir,
	const char *applicationName, const char *versionName, const char *package,
	const char *keystore, const char *keystore_pwd,
	const char *alias, const char *alias_pwd, const char *dstDir,
	const Rtt::TargetDevice::Platform targetPlatform,
	const char *targetAppStoreName,
	bool isDistribution, int versionCode, bool createLiveBuild 
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	, bool includeFusePlugins,
	bool enableMonetization
#endif
	)
{
	Rtt::WinPlatformServices *pServices = GetWinProperties()->GetServices();
	CSimulatorApp *pApp = ((CSimulatorApp *)AfxGetApp());

	Rtt_ASSERT(pApp != NULL);

	// Create the app packager.
	Rtt::AndroidAppPackager packager( *pServices, GetWinProperties()->GetResourcesDir() );

	// Read the application's "build.settings" file.
	bool wasSuccessful = packager.ReadBuildSettings( srcDir );
	if (!wasSuccessful)
	{
		LogAnalytics( "android", "build-bungled", "reason", "bad-build-settings" );

		CString message;
		message.LoadString(IDS_BUILD_SETTINGS_FILE_ERROR);
		return CBuildResult(5, message);
	}

	// Check if a custom build ID has been assigned.
	// This is typically assigned to daily build versions of Corona.
	const char * customBuildId = packager.GetCustomBuildId();
    if( ! Rtt_StringIsEmpty( customBuildId ) )
	{
		Rtt_Log("\nUsing custom Build Id %s\n", customBuildId );

		LogAnalytics( "android", "build-with-custom-id", "custom-id", customBuildId );
	}

	// these are currently unused
	const char *identity = "";
	const char *provisionFile = "";
	const char *sdkRoot = "";

	// Package build settings parameters.
	Rtt::AndroidAppPackagerParams params(
		applicationName, versionName, identity, provisionFile,	
		srcDir,	dstDir,	sdkRoot,  
		targetPlatform, targetAppStoreName,
		(S32)Rtt::TargetDevice::VersionForPlatform(Rtt::TargetDevice::kAndroidPlatform),
		customBuildId, NULL,
		package, isDistribution, keystore, keystore_pwd, alias, alias_pwd, versionCode );
	params.SetStripDebug( isDistribution );

    // Select build template
	Rtt::Runtime *runtimePointer = pSim->GetRuntime();
	U32 luaModules = runtimePointer->VMContext().GetModules();
	params.InitializeProductId( luaModules );

	const char kBuildSettings[] = "build.settings";
	Rtt::String buildSettingsPath;
	pSim->GetPlatform()->PathForFile(
			kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath );
	params.SetBuildSettingsPath( buildSettingsPath.GetString() );
	params.SetLiveBuild(createLiveBuild);

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	params.SetIncludeFusePlugins( includeFusePlugins );
	params.SetUsesMonetization( enableMonetization );
#endif

	TCHAR user[MAX_PATH];
	DWORD ulen = MAX_PATH;
	GetUserName(user, &ulen);
	bool asciiUserName = true;
	for (size_t i = 0; i < ulen && asciiUserName; i++)
	{
		asciiUserName = iswascii(user[i]);
	}

    // Get Windows temp directory
	TCHAR TempPath[MAX_PATH];
	GetTempPath(MAX_PATH, TempPath);  // ends in '\\'
	size_t tempBaseLen = _tcslen(TempPath);
	if (!asciiUserName)
	{
		GetWindowsDirectory(TempPath, MAX_PATH);
		tempBaseLen = _tcslen(TempPath);
		TCHAR *winTemp = _T("\\Temp\\");
		_tcsncpy_s(TempPath + tempBaseLen, MAX_PATH - tempBaseLen, winTemp, _tcslen(winTemp));
		tempBaseLen = _tcslen(TempPath);
		params.SetWindowsNonAsciiUser(true);
	}

    TCHAR *sCompanyName = _T("Corona Labs");
    _tcsncpy_s(TempPath + tempBaseLen, MAX_PATH - tempBaseLen, sCompanyName, _tcslen(sCompanyName));
    TempPath[ _tcslen( TempPath ) ] = '\0';  // ensure null-termination
	WinString strTempDir;
	strTempDir.SetTCHAR( TempPath );

	// Have the server build the app. (Warning! This is a long blocking call.)
	int code = packager.Build( &params, strTempDir.GetUTF8() );

	// Return the result of the build.
	CString statusMessage;
	if (pApp->IsStopBuildRequested())
	{
		CString message;
		// message.LoadString(IDS_BUILD_STOPPED);
		message = "Build stopped at user's request";
		return CBuildResult(5, message);
	}
	else if (0 == code)
	{
		statusMessage.LoadString(GetStatusMessageResourceIdFor(code));

		LogAnalytics( "android", "build-succeeded" );
	}
	else
	{
		WinString serverMsg;

		serverMsg.SetUTF8(params.GetBuildMessage() ? params.GetBuildMessage() : "Error while building app");

		serverMsg.Append("\r\nMore information may be available in the Simulator console");
		statusMessage.Format(IDS_BUILD_ERROR_FORMAT_MESSAGE, code, serverMsg.GetTCHAR());

		CStringA logMesg;
		logMesg.Format("[%ld] %s", code, params.GetBuildMessage());
		LogAnalytics( "android", "build-failed", "reason", logMesg);
	}
	return CBuildResult(code, statusMessage);
}

CBuildResult appWebBuild(
	Interop::SimulatorRuntimeEnvironment *pSim,
	const char *srcDir,
	const char *applicationName, const char *versionName,
	const char *dstDir,
	const Rtt::TargetDevice::Platform targetPlatform,
	const char * targetos, bool isDistribution, int versionCode,
	bool useStandartResources, bool createFBInstantArchive)
{
	Rtt::WinPlatformServices *pServices = GetWinProperties()->GetServices();

	// Translate targetOS in BuildAndroidDlg.h into enum from Rtt_PlatformAppPackager.h
	int targetVersion = Rtt::TargetDevice::kWeb1_0;

	// Create the app packager.
	Rtt::WebAppPackager packager( *pServices );

	// Read the application's "build.settings" file.
	bool wasSuccessful = packager.ReadBuildSettings( srcDir );
	if (!wasSuccessful)
	{
		LogAnalytics( "web", "build-bungled", "reason", "bad-build-settings" );

		CString message;
		message.LoadString(IDS_BUILD_SETTINGS_FILE_ERROR);
		return CBuildResult(5, message);
	}

	// Check if a custom build ID has been assigned.
	// This is typically assigned to daily build versions of Corona.
	const char * customBuildId = packager.GetCustomBuildId();
	if( ! Rtt_StringIsEmpty( customBuildId ) )
	{
		Rtt_Log("\nUsing custom Build Id %s\n", customBuildId );

		LogAnalytics( "web", "build-with-custom-id", customBuildId );
	}

	// these are currently unused
	const char *bundleId = "bundleId"; //TODO
	const char *sdkRoot = "";

	// Package build settings parameters.
	Rtt::WebAppPackagerParams params(
		applicationName, versionName, NULL, NULL,	
		srcDir,	dstDir,	NULL,  
		targetPlatform, targetVersion,
		Rtt::TargetDevice::kWebGenericBrowser, customBuildId,
		NULL, bundleId, isDistribution, useStandartResources, NULL, createFBInstantArchive);

	// Select build template
	Rtt::Runtime* runtimePointer = pSim->GetRuntime();
	U32 luaModules = runtimePointer->VMContext().GetModules();
	params.InitializeProductId( luaModules );

	const char kBuildSettings[] = "build.settings";
	Rtt::String buildSettingsPath;
	pSim->GetPlatform()->PathForFile(
			kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath );
	params.SetBuildSettingsPath( buildSettingsPath.GetString() );

	// Get Windows temp directory
	TCHAR TempPath[MAX_PATH];
	GetTempPath(MAX_PATH, TempPath);  // ends in '\\'
	TCHAR *sCompanyName = _T("Corona Labs");
	_tcsncpy_s( TempPath + _tcslen( TempPath ), (MAX_PATH - _tcslen( TempPath )), sCompanyName, _tcslen( sCompanyName ) );
	TempPath[ _tcslen( TempPath ) ] = '\0';  // ensure null-termination
	WinString strTempDir;
	strTempDir.SetTCHAR( TempPath );

	// Have the server build the app. (Warning! This is a long blocking call.)
	int code = packager.Build( &params, strTempDir.GetUTF8());

	// Return the result of the build.
	CString statusMessage;
	if (0 == code)
	{
		statusMessage.LoadString(GetStatusMessageResourceIdFor(code));

		LogAnalytics( "web", "build-succeeded" );
	}
	else
	{
		WinString serverMsg;

		serverMsg.SetUTF8(params.GetBuildMessage() ? params.GetBuildMessage() : "Error while building app");

		serverMsg.Append("\r\nMore information may be available in the Simulator console");		
		statusMessage.Format(IDS_BUILD_ERROR_FORMAT_MESSAGE, code, serverMsg.GetTCHAR());

		CStringA logMesg;
		logMesg.Format("[%ld] %s", code, params.GetBuildMessage());
		LogAnalytics( "web", "build-failed", "reason", logMesg);
	}
	return CBuildResult(code, statusMessage);
}

CBuildResult appLinuxBuild(
	Interop::SimulatorRuntimeEnvironment *pSim,
	const char *srcDir,
	const char *applicationName, const char *versionName,
	const char *dstDir,
	const Rtt::TargetDevice::Platform targetPlatform,
	const char * targetos, bool isDistribution, int versionCode, bool useStandartResources)
{
	Rtt::WinPlatformServices *pServices = GetWinProperties()->GetServices();


	// Translate targetOS in BuildAndroidDlg.h into enum from Rtt_PlatformAppPackager.h
	int targetVersion = Rtt::TargetDevice::kLinux;

	// Create the app packager.
	Rtt::LinuxAppPackager packager( *pServices );

	// Read the application's "build.settings" file.
	bool wasSuccessful = packager.ReadBuildSettings( srcDir );
	if (!wasSuccessful)
	{
		LogAnalytics( "linux", "build-bungled", "reason", "bad-build-settings" );

		CString message;
		message.LoadString(IDS_BUILD_SETTINGS_FILE_ERROR);
		return CBuildResult(5, message);
	}

	// Check if a custom build ID has been assigned.
	// This is typically assigned to daily build versions of Corona.
	const char * customBuildId = packager.GetCustomBuildId();
	if( ! Rtt_StringIsEmpty( customBuildId ) )
	{
		Rtt_Log("\nUsing custom Build Id %s\n", customBuildId );

		LogAnalytics( "linux", "build-with-custom-id", customBuildId );
	}

	// these are currently unused
	const char *bundleId = "bundleId"; //TODO
	const char *sdkRoot = "";

	// Package build settings parameters.
	Rtt::LinuxAppPackagerParams params(
		applicationName, versionName, NULL, NULL,	
		srcDir,	dstDir,	NULL,  
		targetPlatform, targetVersion,
		Rtt::TargetDevice::kLinux, customBuildId,
		NULL, bundleId, isDistribution, NULL, useStandartResources);

	// Select build template
	Rtt::Runtime* runtimePointer = pSim->GetRuntime();
	U32 luaModules = runtimePointer->VMContext().GetModules();
	params.InitializeProductId( luaModules );

	const char kBuildSettings[] = "build.settings";
	Rtt::String buildSettingsPath;
	pSim->GetPlatform()->PathForFile(kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath );
	params.SetBuildSettingsPath( buildSettingsPath.GetString() );

	// Get Windows temp directory
	TCHAR TempPath[MAX_PATH];
	GetTempPath(MAX_PATH, TempPath);  // ends in '\\'
	TCHAR *sCompanyName = _T("Corona Labs");
	_tcsncpy_s( TempPath + _tcslen( TempPath ), (MAX_PATH - _tcslen( TempPath )), sCompanyName, _tcslen( sCompanyName ) );
	TempPath[ _tcslen( TempPath ) ] = '\0';  // ensure null-termination
	WinString strTempDir;
	strTempDir.SetTCHAR( TempPath );

	// Have the server build the app. (Warning! This is a long blocking call.)
	int code = packager.Build( &params, strTempDir.GetUTF8());

	// Return the result of the build.
	CString statusMessage;
	if (0 == code)
	{
		statusMessage.LoadString(GetStatusMessageResourceIdFor(code));

		LogAnalytics( "linux", "build-succeeded" );
	}
	else
	{
		WinString serverMsg;

		serverMsg.SetUTF8(params.GetBuildMessage() ? params.GetBuildMessage() : "Error while building app");

		serverMsg.Append("\r\nMore information may be available in the Simulator console");		
		statusMessage.Format(IDS_BUILD_ERROR_FORMAT_MESSAGE, code, serverMsg.GetTCHAR());

		CStringA logMesg;
		logMesg.Format("[%ld] %s", code, params.GetBuildMessage());
		LogAnalytics( "linux", "build-failed", "reason", logMesg);
	}
	return CBuildResult(code, statusMessage);
}

void appEndNativeAlert(void *resource, int nButtonIndex, bool bCanceled)
{
	// Validate.
	if (!resource)
	{
		return;
	}

	// Invoke the Lua listener.
	Rtt::LuaResource *pLuaResource = (Rtt::LuaResource *)resource;
	Rtt::LuaLibNative::AlertComplete(*pLuaResource, nButtonIndex, bCanceled);

	// Delete the Lua resource.
	Rtt_DELETE(pLuaResource);
}
