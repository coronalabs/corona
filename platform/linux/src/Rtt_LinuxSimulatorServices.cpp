//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxSimulatorServices.h"
#include "Core/Rtt_Build.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_LinuxSimulator.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxMenuEvents.h"
#include "Rtt_LinuxUtils.h"
#include "Core/Rtt_FileSystem.h"

using namespace std;

namespace Rtt
{
	void PushEvent(int evt);

	LinuxSimulatorServices::LinuxSimulatorServices()
	{
	}

	LinuxSimulatorServices::~LinuxSimulatorServices()
	{
	}

	bool LinuxSimulatorServices::CloneProject() const
	{
//		wxCommandEvent cloneProjectEvent(wxEVT_NULL);
//		solarSimulator->OnCloneProject(cloneProjectEvent);
		return true;
	}

	bool LinuxSimulatorServices::NewProject() const
	{
		PushEvent(sdl::OnNewProject);
		return true;
	}

	bool LinuxSimulatorServices::OpenProject(const char* name) const
	{
		SDL_Event e = {};
		e.type = sdl::OnOpenProject;
		e.user.data1 = name ? strdup(name) : NULL;
		SDL_PushEvent(&e);
		return true;
	}

	bool LinuxSimulatorServices::BuildProject(const char* platformName) const
	{
		TargetDevice::Platform platformType = TargetDevice::PlatformForString(platformName);

		switch (platformType)
		{
		case TargetDevice::kAndroidPlatform:
			return true;
		}

		return false;
	}

	void LinuxSimulatorServices::ShowSampleCode() const
	{
		OpenURL("/opt/Solar2D/SampleCode");
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SelectOpenFilename(const char* currDirectory, const char* extn, LuaResource* resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SelectOpenFilename not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SelectSaveFilename(const char* newFilename, const char* currDirectory, const char* extn, LuaResource* resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SelectSaveFilename not available on Linux\n"));
	}

	const char* LinuxSimulatorServices::GetCurrProjectPath() const
	{
		return app->GetContext()->GetAppPath();
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::RunExtension(const char* extName) const
	{
		Rtt_TRACE_SIM(("WARNING: RunExtension not available on Linux\n"));
	}

	// Set the current project resource path
	void LinuxSimulatorServices::SetProjectResourceDirectory(const char* projectResourceDirectory)
	{
		LinuxPlatform* platform = app->GetPlatform();
		platform->SetProjectResourceDirectory(projectResourceDirectory);
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetWindowCloseListener(LuaResource* resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SetWindowCloseListener not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::CloseWindow() const
	{
		Rtt_TRACE_SIM(("WARNING: CloseWindow not available on Linux\n"));
	}

	// Get the value of the given preference (user registry setting)
	const char* LinuxSimulatorServices::GetPreference(const char* prefName) const
	{
		//Rtt_ASSERT(0 && "todo");
		return 0;
	}

	// Set the value of the given preference (user registry setting)
	void LinuxSimulatorServices::SetPreference(const char* prefName, const char* prefValue) const
	{
		Rtt_ASSERT(0 && "todo");
	}

	// Get the value of the document edited flag
	bool LinuxSimulatorServices::GetDocumentEdited() const
	{
		Rtt_TRACE_SIM(("WARNING: GetDocumentEdited not available on Linux\n"));
		return false;
	}

	// Set the value of the document edited flag
	void LinuxSimulatorServices::SetDocumentEdited(bool value) const
	{
		Rtt_TRACE_SIM(("WARNING: SetDocumentEdited not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetWindowResizeListener(LuaResource* resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SetWindowResizeListener not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetCursorRect(const char* cursorName, int x, int y, int width, int height) const
	{
	}

	// Gets a list of recent projects
	void LinuxSimulatorServices::GetRecentDocs(LightPtrArray<RecentProjectInfo>* list) const
	{
		LinuxSimulatorView::GetRecentDocs(list);
	}

	const char* LinuxSimulatorServices::GetSubscription(S32* expirationTimestamp) const
	{
		Rtt_ASSERT(0 && "todo");
		return 0;
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetWindowTitle(const char* windowTitle) const
	{
		Rtt_TRACE_SIM(("WARNING: SetWindowTitle not available on Linux\n"));
	}

	void LinuxSimulatorServices::OpenTextEditor(const char* filename) const
	{
		PushEvent(sdl::OnOpenInEditor);
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::OpenColorPanel(double r, double g, double b, double a, LuaResource* callback) const
	{
		Rtt_TRACE_SIM(("WARNING: OpenColorPanel not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetBuildMessage(const char* message) const
	{
		Rtt_TRACE_SIM(("WARNING: SetBuildMessage not available on Linux\n"));
	}

	void LinuxSimulatorServices::SendAnalytics(const char* eventName, const char* keyName, const char* value) const
	{
	}

	bool LinuxSimulatorServices::RelaunchProject() const
	{
		PushEvent(sdl::OnRelaunchLastProject);
		return true;
	}

	bool LinuxSimulatorServices::EditProject(const char* name) const
	{
		PushEvent(sdl::OnOpenInEditor);
		return true;
	}

	bool LinuxSimulatorServices::ShowProjectFiles(const char* name) const
	{
		PushEvent(sdl::OnShowProjectFiles);
		return true;
	}

	bool LinuxSimulatorServices::ShowProjectSandbox(const char* name) const
	{
		PushEvent(sdl::OnShowProjectSandbox);
		return true;
	}
}; // namespace Rtt
