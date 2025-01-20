//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinGlobalProperties.h"
#include "Core\Rtt_Build.h"
#include "Interop\SimulatorRuntimeEnvironment.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_WinPlatformServices.h"
#include <string.h>


WinGlobalProperties* WinGlobalProperties::GetInstance()
{
	static WinGlobalProperties sGlobalProperties;
	return &sGlobalProperties;
}

WinGlobalProperties *GetWinProperties()
{
	return WinGlobalProperties::GetInstance();
}

WinGlobalProperties::WinGlobalProperties()
:	m_sRegistryKey(nullptr),
	m_sRegistryProfile(nullptr),
	m_sResourcesDir(nullptr),
	m_pEnvironment(nullptr),
	m_pServices(nullptr),
	m_pAnalytics(nullptr)
{
	// Create a new runtime environment assigned the given window handle. (Used as a parent for native alert dialogs.)
	// We do not set up the runtime environment to create an Rtt::Runtime object since there is no project to run.
	// We only need an Rtt::MPlatform derived object for authorization purposes.
	Interop::SimulatorRuntimeEnvironment::CreationSettings settings;
	settings.IsRuntimeCreationEnabled = false;
	auto result = Interop::SimulatorRuntimeEnvironment::CreateUsing(settings);
	m_pEnvironment = result.GetPointer();
	if (nullptr == m_pEnvironment)
	{
		return;
	}
	auto platformPointer = m_pEnvironment->GetPlatform();
	if (nullptr == platformPointer)
	{
		return;
	}

	// Create authorization objects using the above runtime environment's platform.
	m_pServices = new Rtt::WinPlatformServices(*platformPointer);

	// Create an analytics object for the given platform.
	m_pAnalytics = new Rtt::SimulatorAnalytics(*platformPointer, m_pEnvironment->GetLuaPackageLoaderCallback());
}

WinGlobalProperties::~WinGlobalProperties()
{
	if (m_sRegistryKey)
	{
		delete m_sRegistryKey;
		m_sRegistryKey = nullptr;
	}
	if (m_sRegistryProfile)
	{
		delete m_sRegistryProfile;
		m_sRegistryProfile = nullptr;
	}
	if (m_sResourcesDir)
	{
		delete m_sResourcesDir;
		m_sResourcesDir = nullptr;
	}
	if (m_pAnalytics)
	{
		delete m_pAnalytics;
		m_pAnalytics = nullptr;
	}
	if (m_pServices)
	{
		delete m_pServices;
		m_pServices = nullptr;
	}
	if (m_pEnvironment)
	{
		Interop::SimulatorRuntimeEnvironment::Destroy(m_pEnvironment);
		m_pEnvironment = nullptr;
	}
}

void WinGlobalProperties::SetRegistryKey(const char *sKey)
{
	if (m_sRegistryKey)
	{
		delete m_sRegistryKey;
		m_sRegistryKey = nullptr;
	}

	if (sKey)
	{
		int length = strlen(sKey) + 1;
		m_sRegistryKey = new char[length];
		strcpy_s(m_sRegistryKey, length, sKey);
	}
}

void WinGlobalProperties::SetRegistryProfile(const char *sProfile)
{
	if (m_sRegistryProfile)
	{
		delete m_sRegistryProfile;
		m_sRegistryProfile = nullptr;
	}

	if (sProfile)
	{
		int length = strlen(sProfile) + 1;
		m_sRegistryProfile = new char[length];
		strcpy_s(m_sRegistryProfile, length, sProfile);
	}
}

void WinGlobalProperties::SetResourcesDir(const char *sDir)
{
	if (m_sResourcesDir)
	{
		delete m_sResourcesDir;
		m_sResourcesDir = nullptr;
	}

	if (sDir)
	{
		int length = strlen(sDir) + 1;
		m_sResourcesDir = new char[length];
		strcpy_s(m_sResourcesDir, length, sDir);
	}
}

Rtt::WinPlatform* WinGlobalProperties::GetPlatform()
{
	if (m_pEnvironment)
	{
		return m_pEnvironment->GetPlatform();
	}
	return nullptr;
}
