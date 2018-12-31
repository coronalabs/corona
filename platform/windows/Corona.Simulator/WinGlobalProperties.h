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

#pragma once

#include <Windows.h>


// Corona defined messages for activity indicator and alert windows
// Processed by CSimulatorView
#define WMU_ACTIVITYINDICATOR (WM_APP + 100)
#define WMU_NATIVEALERT (WM_APP + 101)

// Processed by CBuildProgressDlg
#define WMU_DOWNLOAD_PROGRESS_UPDATE (WM_APP + 1000)
#define WMU_BUILD_COMPLETE (WM_APP + 1001)

// Struct passed as lParam to WMU_ALERT
// Note: params are char *, expected in UTF8 format
typedef struct wmu_alert_params
{
	const char *sTitle;
	const char *sMsg;
	int nButtonLabels;
	const char **psButtonLabels;
	void *pLuaResource;
	HWND hwnd;
} WMU_ALERT_PARAMS;


namespace Interop
{
	class SimulatorRuntimeEnvironment;
}
namespace Rtt
{
	class WinPlatform;
	class WinPlatformServices;
	class WinAuthorizationDelegate;
	class Authorization;
	class AuthorizationTicket;
	class SimulatorAnalytics;
};


/// <summary>
///  <para>This class mirrors some members of CSimulatorApp for convenience of access.</para>
///  <para>Instead of AfxGetApp()->GetRegistryKey(), use GetWinProperties()->GetRegistryKey().</para>
///  <para>Note that char * strings are expected to be in UTF8 format. Use WinString to convert.</para>
///  <para>It also holds pointers to the four objects needed for authorization.</para>
/// </summary>
class WinGlobalProperties
{
	private:
		WinGlobalProperties();

	public:
		virtual ~WinGlobalProperties();

		const char *GetRegistryKey() { return m_sRegistryKey; }
		void SetRegistryKey(const char *sKey);
		const char *GetRegistryProfile() { return m_sRegistryProfile; }
		void SetRegistryProfile(const char *sProfile);
		const char *GetResourcesDir() { return m_sResourcesDir; }
		void SetResourcesDir(const char *sDir);
		Rtt::WinPlatform *GetPlatform();
		Rtt::WinPlatformServices *GetServices() { return m_pServices; }
		Rtt::WinAuthorizationDelegate *GetAuthDelegate() { return m_pAuthDelegate; }
		Rtt::Authorization *GetAuth() { return m_pAuth; }
		Rtt::SimulatorAnalytics *GetAnalytics() { return m_pAnalytics; }
		const Rtt::AuthorizationTicket *GetTicket();

		static WinGlobalProperties* GetInstance();

	private:
		void DeleteAuthObjects();

		char *m_sRegistryKey;
		char *m_sRegistryProfile;
		char *m_sResourcesDir;
		Interop::SimulatorRuntimeEnvironment *m_pEnvironment;
		Rtt::WinPlatformServices *m_pServices;
		Rtt::WinAuthorizationDelegate *m_pAuthDelegate;
		Rtt::Authorization *m_pAuth;
		Rtt::SimulatorAnalytics *m_pAnalytics;
};

WinGlobalProperties *GetWinProperties();
