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
#include "Rtt_WebServicesSession.h"
#include "Rtt_PlatformAppPackager.h"
#include "Rtt_AndroidAppPackager.h"
#include "Rtt_WebAppPackager.h"
#include "Rtt_LinuxAppPackager.h"
#include "Rtt_WinAudioPlayer.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_WinAuthorizationDelegate.h"
#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_MAuthorizationDelegate.h"
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
		case Rtt::WebServicesSession::kNoError:
			resourceId = IDS_BUILD_SUCCEEDED;
			break;
		case Rtt::WebServicesSession::kConnectionError:
			resourceId = IDS_NO_INTERNET;
			break;
		case Rtt::WebServicesSession::kLoginError:
			resourceId = IDS_LOGIN_INVALID;
			break;
		case Rtt::WebServicesSession::kExpiredError:
			resourceId = IDS_SUBSCRIPTION_EXPIRED_FOR_BUILD;
			break;
		case Rtt::WebServicesSession::kBuildVersionMismatchError:
			resourceId = IDS_BUILD_VERSION_MISMATCH_ERROR;
			break;
		case Rtt::WebServicesSession::kTokenExpiredError:
			resourceId = IDS_TOKENEXPIRED;
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

// appAuthorizeInstance - check login/password info in registry, contact server and/or put up login dialog as
// necessary, save created auth objects in GetWinProperties for the running lifetime
// of this whole application. It also restricts availability of Daily Builds to paid subscribers.
bool appAuthorizeInstance()
{
    BOOL hasSucceeded = FALSE;
	int code = 0;

	// Fetch platform services needed to authorize the system down below.
	auto pPlatform = GetWinProperties()->GetPlatform();
	auto pServices = GetWinProperties()->GetServices();
	auto pAuthDelegate = GetWinProperties()->GetAuthDelegate();
	auto pAuth = GetWinProperties()->GetAuth();
	auto ticket = GetWinProperties()->GetTicket();
	Rtt::WebServicesSession session(*pServices);

	// Load authorization information from machine if it exists (don't try to login,
	// we do that later if we don't find a valid ticket)
	hasSucceeded = pAuth->Initialize( false );
	if (!hasSucceeded)
	{
		Rtt_TRACE(("appAuthorizeInstance: Initialize failed\n"));
	}
	else
	{
		hasSucceeded = pAuth->VerifyTicket();
		if (!hasSucceeded)
		{
			Rtt_TRACE(("appAuthorizeInstance: VerifyTicket failed\n"));
		}
	}

	// Ask the use to login if needed
	while (!hasSucceeded)
	{
		pAuth->ClearTicket( );
		
		if ((code = appLoginToServer(&session)) == Rtt::WebServicesSession::kNoError)
		{
			hasSucceeded = true;
			
			continue;
		}

		CMessageDlg dialog;
        CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));

		dialog.SetText(message);
		dialog.SetDefaultText(IDS_LOGIN);
		dialog.SetAltText(IDS_QUIT);
		dialog.SetIconStyle(MB_ICONEXCLAMATION);
		int result = dialog.DoModal();

		if (ID_MSG_BUTTON2 == result)
		{
			// User pressed the quit button indicating they don't want to continue
			PostQuitMessage( 0 );

			return false;
		}

#if 0
        CString message;
	if (Rtt::WebServicesSession::kConnectionError == code)
	{
		GetWinProperties()->GetAnalytics()->Log( "build-could-not-connect", NULL );

		// Unable to connect to the server.
		message.LoadString(GetStatusMessageResourceIdFor(code));
	}
	else if (Rtt::WebServicesSession::kTokenExpiredError == code)
	{
		GetWinProperties()->GetAnalytics()->Log( "build-server-rejection", "TokenExpiredError" );

		// The transaction timed out.
        CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));
	}
	else if (code != Rtt::WebServicesSession::kNoError)
	{
		GetWinProperties()->GetAnalytics()->Log( "build-reenter-password", NULL );

		// Ask the user to enter his/her password and attempt log in again.
		GetWinProperties()->GetAuthDelegate()->Reauthorize( GetWinProperties()->GetAuth() );
		code = appLoginToServer( &session );
		if ( code != Rtt::WebServicesSession::kNoError )
		{
			GetWinProperties()->GetAnalytics()->Log( "build-reenter-password", "failed" );

			CString message;
			message.LoadString(GetStatusMessageResourceIdFor(Rtt::WebServicesSession::kLoginError));
		}
	}
#endif
	}

	// Get the logged in ticket
	ticket = pAuth->GetTicket(); Rtt_ASSERT( ticket );

#if 0
	// For now, at least, record all user's analytics (it's a free product, after all)
	// TODO: at some point remove the inactive checkbox from the preferences dialog

	if ( ticket->IsSubscriptionCurrent() )
	{
		// Enable/disable analytics based on the "User Feedback" setting set in the preferences window.
		bool isEnabled = AfxGetApp()->GetProfileInt(REGISTRY_SECTION, REGISTRY_ANALYTICS, REGISTRY_ANALYTICS_DEFAULT) ? true : false;
		GetWinProperties()->GetAnalytics()->SetParticipating(isEnabled);
	}
#endif
	GetWinProperties()->GetAnalytics()->Initialize(
		Rtt::AuthorizationTicket::StringForSubscription( ticket->GetSubscription() ),
		ticket->GetUid() );
	GetWinProperties()->GetAnalytics()->BeginSession(ticket->GetUid());

	// Do not allow trial users to run a daily build version of this software.
	const Rtt::AuthorizationTicket *pTicket = GetWinProperties()->GetTicket();
	hasSucceeded = FALSE;

	while (pTicket && (!pTicket->IsDailyBuildAllowed() && !Rtt::PlatformAppPackager::IsAppSettingsEmpty(*pPlatform)))
	{
		if (!hasSucceeded || !pTicket->IsDailyBuildAllowed())
		{
			// Notify the user that he/she must buy a subscription to use this application.
			CMessageDlg dialog;
			CString mesg;

			mesg.LoadStringW(IDS_DAILY_BUILD_AVAILABLE_TO_SUBSCRIBERS_ONLY);

			if (pTicket == NULL || pTicket->IsPlaceholder())
			{
				CString txt;
				txt.LoadString(IDS_NOT_LOGGED_IN);
				mesg.Append(_T("\n\n"));
				mesg.Append(txt);
			}
			else
			{
				CString txt;
				txt.LoadString(IDS_LOGGED_IN_AS);
				mesg.Append(_T("\n\n"));
				mesg.Append(txt);
				mesg.Append(CString(pTicket->GetUsername()));
			}
			dialog.SetText(mesg);
			dialog.SetDefaultText(IDS_BUYNOW);
			dialog.SetAltText(IDS_QUIT);
			dialog.SetButton3Text(IDS_LOGIN);
			dialog.SetUrl(Rtt::Authorization::kUrlPurchase);
			dialog.SetIconStyle(MB_ICONEXCLAMATION);
			int result = dialog.DoModal();

			if (ID_MSG_BUTTON3 == result)
			{
				// User wants to relogin

				pAuth->ClearTicket( );

				hasSucceeded = appLoginToServer(&session);

				pTicket = GetWinProperties()->GetTicket();
			}
			else if (ID_MSG_BUTTON2 == result)
			{
				// User pressed the quit button indicating they don't want to continue
				PostQuitMessage( 0 );

				return false;
			}
		}

		if (hasSucceeded)
		{				
			// The user needs to be actually logged in if this is a Daily Build 
			if (pAuth->Initialize( true ))
			{
				pTicket = GetWinProperties()->GetTicket();
			}
		}
	}

	if (pTicket == NULL)
	{
		return false;
	}

	// If they couldn't login, we'll have quit before we reach this point

	const char *sub = (ticket == NULL ? "none" : Rtt::AuthorizationTicket::DisplayStringForSubscription( ticket->GetSubscription() ));
	static std::map<std::string, std::string> keyValues;
	keyValues["build-type"] = (Rtt::PlatformAppPackager::IsAppSettingsEmpty(*pPlatform) ? "release" : "daily");
	keyValues["subscription"] = sub;

	GetWinProperties()->GetAnalytics()->Log("authorized", keyValues);

	// This machine is authorized to run this application.
    return true;
}

// Contacts the Corona Labs server to deauthorize Corona on this machine.
// Displays a progress window while this is happening and a message box if it succeeded or failed.
// Returns true if deauthorization succeeded. Returns false if not.
bool appDeauthorize()
{
	CString message;
	int resultCode = Rtt::Authorization::kAuthorizationError;
	bool wasDeauthorized = false;

	// Display a progress window and wait cursor.
    CSimulatorApp *pApp = (CSimulatorApp*)AfxGetApp();
	if (pApp)
	{
		pApp->ShowProgressWnd(true);
	}
    CWaitCursor wait; 

	// Deauthorize this machine using currently loaded ticket.
	Rtt::Authorization *pAuth = GetWinProperties()->GetAuth();
	const Rtt::AuthorizationTicket *pTicket = GetWinProperties()->GetTicket();
	Rtt::String ticketData, pwd;
	Rtt::WinPlatformServices *pServices = GetWinProperties()->GetServices();
	pServices->GetPreference( Rtt::Authorization::kTicketKey, &ticketData );

	if (pAuth && pTicket)
	{
		const char *user = pTicket->GetUsername();
		pServices->GetPreference(pTicket->GetUsername(), &pwd);
		resultCode = pAuth->Deauthorize(user, pwd.GetString(), ticketData.GetString());
	}

	// Close the progress window.
	// The wait cursor will be cleared when we leave this function via its destructor.
	if (pApp)
	{
		pApp->ShowProgressWnd(false);
	}

	// Display a message box indicating if deauthorization succeeded or failed.
	switch (resultCode)
	{
		case Rtt::Authorization::kAuthorizationDeauthorizeSuccess:
			message.LoadString(IDS_DEAUTHORIZED);
			wasDeauthorized = true;
			break;
		case Rtt::Authorization::kAuthorizationTokenExpired:
			message.LoadString(IDS_TOKENEXPIRED);
			break;
		case Rtt::Authorization::kAuthorizationConnectionError:
			message.LoadString(IDS_NO_INTERNET);
			break;
		default:
			message.LoadString(IDS_DEAUTHERROR);
			break;
	}
	::AfxMessageBox(message, MB_OK | (wasDeauthorized ? MB_ICONINFORMATION : MB_ICONWARNING));

	if (wasDeauthorized)
	{
		GetWinProperties()->GetAnalytics()->Log("deauthorize", "result", "success");
	}
	else
	{
		GetWinProperties()->GetAnalytics()->Log("deauthorize", "result", "failure");

		if (pAuth != NULL)
		{
			// Rather than leave them in a weird state, zap the local info and claim victory
			pAuth->ClearTicket();

			wasDeauthorized = true;
		}
	}

	// Returns true if machine was successfully deauthorized.
    return wasDeauthorized;
}

// appAllowFullBuild - Check user's permissions, return true if allowed full build.
// If not, put up message notifying user of limitations.
// Trial users and ios-only users are limited on Windows for Android builds
bool appAllowFullBuild(const Rtt::TargetDevice::Platform targetPlatform)
{
	const Rtt::AuthorizationTicket *pTicket = GetWinProperties()->GetTicket();
	Rtt::Authorization *pAuth = GetWinProperties()->GetAuth();
	Rtt::WinAuthorizationDelegate *pAuthDelegate = GetWinProperties()->GetAuthDelegate();

    if( NULL == pTicket )
	{
		return false;
	}

    if( pTicket->IsTrial() )
	{
		static time_t nextUpsellTime = 0;

		if (time(NULL) > nextUpsellTime)
		{
			// Show upsell roughly once a day
			nextUpsellTime = time(NULL) + 86400;
			
			pAuthDelegate->ShowTrialBuild( *pAuth, *pTicket );
		}
	}
	else if( ! pTicket->IsAllowedToBuild( targetPlatform ) )
	{
		pAuthDelegate->ShowDeviceBuildUpsell( *pAuth, *pTicket, targetPlatform );
	}

	// Get ticket again, in case re-allocated by purchase
	pTicket = pAuth->GetTicket();

	// Now are they allowed to build?
	return pTicket->IsAllowedToBuild( targetPlatform );
}

// appLoginToServer - Helper function for appAndroidBuild - login to server
int appLoginToServer( Rtt::WebServicesSession *pSession )
{
	const char *username = "";
	Rtt::WinPlatformServices *pServices = GetWinProperties()->GetServices();
	const Rtt::AuthorizationTicket *pTicket = NULL;
	Rtt::Authorization *pAuth = GetWinProperties()->GetAuth();

	// Force a login if we are not already logged in
	if (! pAuth->Initialize( true ))
	{
		return Rtt::WebServicesSession::kLoginError;
	}
	
	pTicket = GetWinProperties()->GetTicket();

	if( NULL != pTicket )
	{
		username = pTicket->GetUsername();
	}

	Rtt::String pwd;

    pServices->GetSecurePreference( username, &pwd );
    if( pwd.GetString() == NULL )
        pServices->GetPreference( username, &pwd );

	int code = pSession->LoginWithEncryptedPassword( Rtt::WebServicesSession::CoronaServerUrl(*pServices), username, pwd.GetString() );

    return code;
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
	Rtt::WebServicesSession session( *pServices );
	CSimulatorApp *pApp = ((CSimulatorApp *)AfxGetApp());

	Rtt_ASSERT(pApp != NULL);

	pApp->SetCurrentWebServicesSession(&session);

    // Try to log in.
    int code = appLoginToServer( &session );
	if (pApp->IsStopBuildRequested())
	{
		CString message;
		// message.LoadString(IDS_BUILD_STOPPED);
		message = "Build stopped at user's request";
		return CBuildResult(Rtt::WebServicesSession::kBuildError, message);
	}
	else if (Rtt::WebServicesSession::kConnectionError == code)
	{
		LogAnalytics( "android", "build-failed", "reason", "could-not-connect" );

		// Unable to connect to the server.
        CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));
		message += "\n\n";
		message += session.ErrorMessage();

		return CBuildResult(code, message);
	}
	else if (Rtt::WebServicesSession::kTokenExpiredError == code)
	{
		LogAnalytics( "android", "build-failed", "reason", "TokenExpiredError" );

		// The transaction timed out.
        CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));
		return CBuildResult(code, message);
	}
	else if (code != Rtt::WebServicesSession::kNoError)
	{
		LogAnalytics( "android", "build-bungled", "reason", "reenter-password" );

		// Ask the user to enter his/her password and attempt log in again.
		GetWinProperties()->GetAuthDelegate()->Reauthorize( GetWinProperties()->GetAuth() );
		code = appLoginToServer( &session );
		if ( code != Rtt::WebServicesSession::kNoError )
		{
			LogAnalytics( "android", "build-bungled", "reason", "reenter-password-failed" );

			CString message;
			message.LoadString(GetStatusMessageResourceIdFor(Rtt::WebServicesSession::kLoginError));
			return CBuildResult(code, message);
		}
	}

	// Create the app packager.
	Rtt::AndroidAppPackager packager( *pServices, GetWinProperties()->GetResourcesDir() );

	// Read the application's "build.settings" file.
	bool wasSuccessful = packager.ReadBuildSettings( srcDir );
	if (!wasSuccessful)
	{
		LogAnalytics( "android", "build-bungled", "reason", "bad-build-settings" );

		CString message;
		message.LoadString(IDS_BUILD_SETTINGS_FILE_ERROR);
		return CBuildResult(Rtt::WebServicesSession::kBuildError, message);
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

    // Get Windows temp directory
	TCHAR TempPath[MAX_PATH];
	GetTempPath(MAX_PATH, TempPath);  // ends in '\\'
    TCHAR *sCompanyName = _T("Corona Labs");
    _tcsncpy_s( TempPath + _tcslen( TempPath ), (MAX_PATH - _tcslen( TempPath )), sCompanyName, _tcslen( sCompanyName ) );
    TempPath[ _tcslen( TempPath ) ] = '\0';  // ensure null-termination
	WinString strTempDir;
	strTempDir.SetTCHAR( TempPath );

	// Have the server build the app. (Warning! This is a long blocking call.)
	code = packager.Build( &params, session, strTempDir.GetUTF8() );

	// Return the result of the build.
	CString statusMessage;
	if (pApp->IsStopBuildRequested())
	{
		CString message;
		// message.LoadString(IDS_BUILD_STOPPED);
		message = "Build stopped at user's request";
		return CBuildResult(Rtt::WebServicesSession::kBuildError, message);
	}
	else if (Rtt::WebServicesSession::kNoError == code)
	{
		statusMessage.LoadString(GetStatusMessageResourceIdFor(code));

		LogAnalytics( "android", "build-succeeded" );
	}
	else
	{
		WinString serverMsg;

		serverMsg.SetUTF8(params.GetBuildMessage());

		if (session.ErrorMessage() != NULL)
		{
			if (serverMsg.GetLength() > 0)
			{
				serverMsg.Append("\r\n\r\n");
			}

			serverMsg.Append("Communications error:\r\n\r\n\t");
			serverMsg.Append(session.ErrorMessage());
			serverMsg.Append("\r\n");
		}

		if (Rtt::WebServicesSession::kConnectionError == code)
		{
			serverMsg.Append("\r\nIf you are not connecting to the internet directly (for example, you connect via a proxy server)\r\nyou might want to try a direct connection to see if that solves the problem.\r\n");
		}

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
	Rtt::WebServicesSession session( *pServices );

	// Try to log in.
	int code = appLoginToServer( &session );
	if (Rtt::WebServicesSession::kConnectionError == code)
	{
		LogAnalytics( "web", "build-failed", "reason", "could-not-connect" );

		// Unable to connect to the server.
		CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));
		return CBuildResult(code, message);
	}
	else if (Rtt::WebServicesSession::kTokenExpiredError == code)
	{
		LogAnalytics( "web", "build-failed", "reason", "server-rejection (TokenExpiredError)" );

		// The transaction timed out.
		CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));
		return CBuildResult(code, message);
	}
	else if (code != Rtt::WebServicesSession::kNoError)
	{
		LogAnalytics( "web", "build-bungled", "reason", "reenter-password" );

		// Ask the user to enter his/her password and attempt log in again.
		GetWinProperties()->GetAuthDelegate()->Reauthorize( GetWinProperties()->GetAuth() );
		code = appLoginToServer( &session );
		if ( code != Rtt::WebServicesSession::kNoError )
		{
			LogAnalytics( "web", "build-bungled", "reason", "reenter-password-failed" );

			CString message;
			message.LoadString(GetStatusMessageResourceIdFor(Rtt::WebServicesSession::kLoginError));
			return CBuildResult(code, message);
		}
	}

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
		return CBuildResult(Rtt::WebServicesSession::kBuildError, message);
	}

	// Check if a custom build ID has been assigned.
	// This is typically assigned to daily build versions of Corona.
	const char * customBuildId = packager.GetCustomBuildId();
	if( ! Rtt_StringIsEmpty( customBuildId ) )
	{
		Rtt_Log("\nUsing custom Build Id %s\n", customBuildId );

		LogAnalytics( "web", "build-with-custom-id", customBuildId );
	}

	const Rtt::AuthorizationTicket *pTicket = GetWinProperties()->GetTicket();
	const char *identity = pTicket ? pTicket->GetUsername() : "";

	// these are currently unused
	const char *bundleId = "bundleId"; //TODO
	const char *sdkRoot = "";

	// Package build settings parameters.
	Rtt::WebAppPackagerParams params(
		applicationName, versionName, identity, NULL,	
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
	code = packager.Build( &params, session, strTempDir.GetUTF8());

	// Return the result of the build.
	CString statusMessage;
	if (Rtt::WebServicesSession::kNoError == code)
	{
		statusMessage.LoadString(GetStatusMessageResourceIdFor(code));

		LogAnalytics( "web", "build-succeeded" );
	}
	else
	{
		WinString serverMsg;

		serverMsg.SetUTF8(params.GetBuildMessage());

		if (session.ErrorMessage() != NULL)
		{
			if (serverMsg.GetLength() > 0)
			{
				serverMsg.Append("\r\n\r\n");
			}

			serverMsg.Append("Communications error:\r\n\r\n\t");
			serverMsg.Append(session.ErrorMessage());
			serverMsg.Append("\r\n");
		}

		if (Rtt::WebServicesSession::kConnectionError == code)
		{
			serverMsg.Append("\r\nIf you are not connecting to the internet directly (for example, you connect via a proxy server)\r\nyou might want to try a direct connection to see if that solves the problem.\r\n");
		}

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
	Rtt::WebServicesSession session( *pServices );

	// Try to log in.
	int code = appLoginToServer( &session );
	if (Rtt::WebServicesSession::kConnectionError == code)
	{
		LogAnalytics( "linux", "build-failed", "reason", "could-not-connect" );

		// Unable to connect to the server.
		CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));
		return CBuildResult(code, message);
	}
	else if (Rtt::WebServicesSession::kTokenExpiredError == code)
	{
		LogAnalytics( "linux", "build-failed", "reason", "server-rejection (TokenExpiredError)" );

		// The transaction timed out.
		CString message;
		message.LoadString(GetStatusMessageResourceIdFor(code));
		return CBuildResult(code, message);
	}
	else if (code != Rtt::WebServicesSession::kNoError)
	{
		LogAnalytics( "linux", "build-bungled", "reason", "reenter-password" );

		// Ask the user to enter his/her password and attempt log in again.
		GetWinProperties()->GetAuthDelegate()->Reauthorize( GetWinProperties()->GetAuth() );
		code = appLoginToServer( &session );
		if ( code != Rtt::WebServicesSession::kNoError )
		{
			LogAnalytics( "linux", "build-bungled", "reason", "reenter-password-failed" );

			CString message;
			message.LoadString(GetStatusMessageResourceIdFor(Rtt::WebServicesSession::kLoginError));
			return CBuildResult(code, message);
		}
	}

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
		return CBuildResult(Rtt::WebServicesSession::kBuildError, message);
	}

	// Check if a custom build ID has been assigned.
	// This is typically assigned to daily build versions of Corona.
	const char * customBuildId = packager.GetCustomBuildId();
	if( ! Rtt_StringIsEmpty( customBuildId ) )
	{
		Rtt_Log("\nUsing custom Build Id %s\n", customBuildId );

		LogAnalytics( "linux", "build-with-custom-id", customBuildId );
	}

	const Rtt::AuthorizationTicket *pTicket = GetWinProperties()->GetTicket();
	const char *identity = pTicket ? pTicket->GetUsername() : "";

	// these are currently unused
	const char *bundleId = "bundleId"; //TODO
	const char *sdkRoot = "";

	// Package build settings parameters.
	Rtt::LinuxAppPackagerParams params(
		applicationName, versionName, identity, NULL,	
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
	code = packager.Build( &params, session, strTempDir.GetUTF8());

	// Return the result of the build.
	CString statusMessage;
	if (Rtt::WebServicesSession::kNoError == code)
	{
		statusMessage.LoadString(GetStatusMessageResourceIdFor(code));

		LogAnalytics( "linux", "build-succeeded" );
	}
	else
	{
		WinString serverMsg;

		serverMsg.SetUTF8(params.GetBuildMessage());

		if (session.ErrorMessage() != NULL)
		{
			if (serverMsg.GetLength() > 0)
			{
				serverMsg.Append("\r\n\r\n");
			}

			serverMsg.Append("Communications error:\r\n\r\n\t");
			serverMsg.Append(session.ErrorMessage());
			serverMsg.Append("\r\n");
		}

		if (Rtt::WebServicesSession::kConnectionError == code)
		{
			serverMsg.Append("\r\nIf you are not connecting to the internet directly (for example, you connect via a proxy server)\r\nyou might want to try a direct connection to see if that solves the problem.\r\n");
		}

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
