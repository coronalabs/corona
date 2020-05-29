//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinSimulatorServices.h"
#include "Core\Rtt_Build.h"
#include "Interop\Storage\RegistryStoredPreferences.h"
#include "SimulatorView.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Simulator.h"
#include "SimulatorView.h"
#include "WinGlobalProperties.h"


namespace Rtt
{

WinSimulatorServices::WinSimulatorServices(CSimulatorView& simulatorView)
:	fSimulatorView(simulatorView)
{
}

WinSimulatorServices::~WinSimulatorServices()
{
}

bool WinSimulatorServices::NewProject() const
{
	fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_NEW);
	return true;
}

bool WinSimulatorServices::OpenProject( const char *name ) const
{
	if (name)
	{
		WinString stringConverter;
		stringConverter.SetUTF8(name);
		CString convertedString = stringConverter.GetTCHAR();
		fSimulatorView.PostOpenWithPath(convertedString);
	}
	else
	{
		// Display the "Open File" dialog for selecting a Corona project to run.
		fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_OPEN);
	}
	return true;
}

bool WinSimulatorServices::BuildProject( const char *platformName ) const
{
	TargetDevice::Platform platformType = TargetDevice::PlatformForString(platformName);
	switch (platformType)
	{
		case TargetDevice::kAndroidPlatform:
			fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_OPENFORBUILD);
			return true;
	}
	return false;
}

void WinSimulatorServices::ShowSampleCode() const
{
	fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_OPEN_SAMPLE_PROJECT);
}

// stub to match Mac implementation
void WinSimulatorServices::SelectOpenFilename(const char* currDirectory, const char* extn, LuaResource* resource) const
{
	Rtt_TRACE_SIM( ( "WARNING: SelectOpenFilename not available on Windows\n" ) );
}

// stub to match Mac implementation
void WinSimulatorServices::SelectSaveFilename(const char* newFilename, const char* currDirectory, const char* extn, LuaResource* resource) const
{
	Rtt_TRACE_SIM( ( "WARNING: SelectSaveFilename not available on Windows\n" ) );
}

// stub to match Mac implementation
const char* WinSimulatorServices::GetCurrProjectPath( ) const
{
	Rtt_TRACE_SIM( ( "WARNING: GetCurrProjectPath not available on Windows\n" ) );
    
    return NULL;
}

// stub to match Mac implementation
void WinSimulatorServices::RunExtension(const char *extName) const
{
	Rtt_TRACE_SIM( ( "WARNING: RunExtension not available on Windows\n" ) );
}

// Set the current project resource path
void WinSimulatorServices::SetProjectResourceDirectory(const char *projectResourceDirectory)
{
	auto runtimeEnvironmentPointer = fSimulatorView.GetRuntimeEnvironment();
	if (runtimeEnvironmentPointer && runtimeEnvironmentPointer->GetPlatform())
	{
		runtimeEnvironmentPointer->GetPlatform()->SetProjectResourceDirectory(projectResourceDirectory);
	}
}

// stub to match Mac implementation
void WinSimulatorServices::SetWindowCloseListener(LuaResource* resource) const
{
	Rtt_TRACE_SIM( ( "WARNING: SetWindowCloseListener not available on Windows\n" ) );
}

// stub to match Mac implementation
void WinSimulatorServices::CloseWindow( ) const
{
	Rtt_TRACE_SIM( ( "WARNING: CloseWindow not available on Windows\n" ) );
}

// Get the value of the given preference (user registry setting)
const char* WinSimulatorServices::GetPreference(const char *prefName) const
{
	static char sStringBuffer[1024];

	// Fetch the Corona Simulator's registry preferences accessing object.
	auto storedPreferencesPointer = Interop::Storage::RegistryStoredPreferences::ForSimulatorPreferences();
	if (!storedPreferencesPointer)
	{
		return nullptr;
	}

	// Attempt to fetch the requested preference from the registry.
	auto fetchResult = storedPreferencesPointer->Fetch(prefName);
	if (fetchResult.HasFailed())
	{
		return nullptr;
	}

	// Attempt to convert the preference value to string, if not already a string.
	auto conversionResult = fetchResult.GetValue().ToString();
	if (conversionResult.HasFailed() || conversionResult.GetValue().IsNull())
	{
		return nullptr;
	}

	// Success! Copy the preference value string to a static buffer and return it.
	sStringBuffer[0] = '\0';
	strcpy_s(sStringBuffer, sizeof(sStringBuffer), conversionResult.GetValue()->c_str());
	return sStringBuffer;
}

// Set the value of the given preference (user registry setting)
void WinSimulatorServices::SetPreference(const char *prefName, const char *prefValue) const
{
	// Validate.
	if (Rtt_StringIsEmpty(prefName))
	{
		return;
	}

	// Fetch the Corona Simulator's registry preferences accessing object.
	auto storedPreferencesPointer = Interop::Storage::RegistryStoredPreferences::ForSimulatorPreferences();
	if (!storedPreferencesPointer)
	{
		return;
	}

	// Write the given prefernce key/value pair to the registry.
	Rtt::Preference preference(prefName, Rtt::PreferenceValue(prefValue));
	storedPreferencesPointer->UpdateWith(preference);
}

// Get the value of the document edited flag
bool WinSimulatorServices::GetDocumentEdited() const
{
	Rtt_TRACE_SIM( ( "WARNING: GetDocumentEdited not available on Windows\n" ) );

    return false;
}

// Set the value of the document edited flag
void WinSimulatorServices::SetDocumentEdited(bool value) const
{
	Rtt_TRACE_SIM( ( "WARNING: SetDocumentEdited not available on Windows\n" ) );
}

// stub to match Mac implementation
void WinSimulatorServices::SetWindowResizeListener(LuaResource* resource) const
{
	Rtt_TRACE_SIM( ( "WARNING: SetWindowResizeListener not available on Windows\n" ) );
}

// stub to match Mac implementation
void WinSimulatorServices::SetCursorRect(const char *cursorName, int x, int y, int width, int height) const
{
	// Fetch the currently active Corona runtime environment.
	auto runtimeEnvironmentPointer = fSimulatorView.GetRuntimeEnvironment();
	if (!runtimeEnvironmentPointer)
	{
		return;
	}

	// Floor the width and height to valid values.
	if (width < 0)
	{
		width = 0;
	}
	if (height < 0)
	{
		height = 0;
	}

	// Copy the given cursor bounds to a Win32 rect sructure.
	RECT region{};
	region.left = x;
	region.top = y;
	region.right = x + width;
	region.bottom = y + height;

	// Identify the given mouse cursor style.
	bool isCursorNameValid = true;
	auto cursorStyle = WinInputDeviceManager::CursorStyle::kDefaultArrow;
	if (Rtt_StringCompare(cursorName, "arrow") == 0)
	{
		cursorStyle = WinInputDeviceManager::CursorStyle::kDefaultArrow;
	}
	else if (Rtt_StringCompare(cursorName, "crosshair") == 0)
	{
		cursorStyle = WinInputDeviceManager::CursorStyle::kCrosshair;
	}
	else if (Rtt_StringCompare(cursorName, "notAllowed") == 0)
	{
		cursorStyle = WinInputDeviceManager::CursorStyle::kSlashedCircle;
	}
	else if (Rtt_StringCompare(cursorName, "pointingHand") == 0)
	{
		cursorStyle = WinInputDeviceManager::CursorStyle::kPointingHand;
	}
	else
	{
		isCursorNameValid = false;
	}

	// Add or remove the given mouse cursor region.
	// Note: Mac version of Corona removes the given region if it could not identify the cursor name.
	if (isCursorNameValid)
	{
		runtimeEnvironmentPointer->AddMouseCursorRegion(cursorStyle, region);
	}
	else
	{
		runtimeEnvironmentPointer->RemoveMouseCursorRegion(region);
	}
}

// Gets a list of recent projects
void WinSimulatorServices::GetRecentDocs(LightPtrArray<RecentProjectInfo> *list) const
{
	fSimulatorView.GetRecentDocs(list);
}

// stub to match Mac implementation
void WinSimulatorServices::SetWindowTitle(const char *windowTitle) const
{
	Rtt_TRACE_SIM( ( "WARNING: SetWindowTitle not available on Windows\n" ) );
}

// stub to match Mac implementation
void WinSimulatorServices::OpenTextEditor(const char *filename) const
{
	Rtt_TRACE_SIM( ( "WARNING: OpenTextEditor not available on Windows\n" ) );
}

// stub to match Mac implementation
void WinSimulatorServices::OpenColorPanel(double r, double g, double b, double a, LuaResource* callback) const
{
	Rtt_TRACE_SIM( ( "WARNING: OpenColorPanel not available on Windows\n" ) );
}

// stub to match Mac implementation
void WinSimulatorServices::SetBuildMessage(const char *message) const
{
	Rtt_TRACE_SIM( ( "WARNING: SetBuildMessage not available on Windows\n" ) );
}

void WinSimulatorServices::SendAnalytics(const char *eventName, const char *keyName, const char *value) const
{
	if (GetWinProperties()->GetAnalytics() != NULL)
	{
		GetWinProperties()->GetAnalytics()->Log(eventName, keyName, value);
	}
}

bool WinSimulatorServices::RelaunchProject() const
{
	fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_RELAUNCH);

	return true;
}

bool WinSimulatorServices::EditProject(const char *name) const
{
	if (name != NULL && *name != 0)
	{
		WinString stringConverter;
		stringConverter.SetUTF8(name);
		CString convertedString = stringConverter.GetTCHAR();
		fSimulatorView.PostOpenWithPath(convertedString);

		fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_OPENINEDITOR);

		return true;
	}
	else
	{
		return false;
	}
}

bool WinSimulatorServices::ShowProjectFiles(const char *name) const
{
	if (name != NULL && *name != 0)
	{
		WinString stringConverter;
		stringConverter.SetUTF8(name);
		CString convertedString = stringConverter.GetTCHAR();
		fSimulatorView.PostOpenWithPath(convertedString);

		fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_SHOW_PROJECT_FILES);

		return true;
	}
	else
	{
		return false;
	}
}

bool WinSimulatorServices::ShowProjectSandbox(const char *name) const
{
	if (name != NULL && *name != 0)
	{
		WinString stringConverter;
		stringConverter.SetUTF8(name);
		CString convertedString = stringConverter.GetTCHAR();
		fSimulatorView.PostOpenWithPath(convertedString);

		fSimulatorView.PostMessage(WM_COMMAND, ID_FILE_SHOWPROJECTSANDBOX);

		return true;
	}
	else
	{
		return false;
	}
}

} // namespace Rtt
