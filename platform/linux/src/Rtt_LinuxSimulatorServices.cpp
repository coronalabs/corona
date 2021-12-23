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
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxMenuEvents.h"
#include "Rtt_LinuxUtils.h"
#include "Core/Rtt_FileSystem.h"

using namespace std;

namespace Rtt
{
	string LinuxSimulatorServices::fCurrentProjectPath;

	LinuxSimulatorServices::LinuxSimulatorServices()
	{
	}

	LinuxSimulatorServices::~LinuxSimulatorServices()
	{
	}

	bool LinuxSimulatorServices::CloneProject() const
	{
		wxCommandEvent cloneProjectEvent(wxEVT_NULL);
		LinuxMenuEvents::OnCloneProject(cloneProjectEvent);
		return true;
	}

	bool LinuxSimulatorServices::NewProject() const
	{
		wxCommandEvent newProjectEvent(wxEVT_NULL);
		LinuxMenuEvents::OnNewProject(newProjectEvent);
		return true;
	}

	bool LinuxSimulatorServices::OpenProject(const char *name) const
	{
		wxString path;

		if (name != NULL)
		{
			path = name;
		}
		else
		{
			wxString startPath(Rtt::LinuxSimulatorView::Config::lastProjectDirectory);
			wxFileDialog openFileDialog(wxGetApp().GetParent(), _("Open"), startPath, "", "Simulator Files (main.lua)|main.lua", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

			if (openFileDialog.ShowModal() == wxID_CANCEL)
			{
				return false;
			}

			path = openFileDialog.GetPath().c_str();
		}

		if (!Rtt_FileExists(path.c_str()))
		{
			return false;
		}

		// update the current project path
		fCurrentProjectPath = path.ToStdString();

		// send open file dialog event
		wxCommandEvent openEvent(eventOpenProject);
		openEvent.SetString(path);
		wxPostEvent(wxGetApp().GetFrame(), openEvent);

		return true;
	}

	bool LinuxSimulatorServices::BuildProject(const char *platformName) const
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
		wxExecute("xdg-open /opt/Solar2D/SampleCode");
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SelectOpenFilename(const char *currDirectory, const char *extn, LuaResource *resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SelectOpenFilename not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SelectSaveFilename(const char *newFilename, const char *currDirectory, const char *extn, LuaResource *resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SelectSaveFilename not available on Linux\n"));
	}

	const char *LinuxSimulatorServices::GetCurrProjectPath() const
	{
		return fCurrentProjectPath.c_str();
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::RunExtension(const char *extName) const
	{
		Rtt_TRACE_SIM(("WARNING: RunExtension not available on Linux\n"));
	}

	// Set the current project resource path
	void LinuxSimulatorServices::SetProjectResourceDirectory(const char *projectResourceDirectory)
	{
		LinuxPlatform *platform = wxGetApp().GetPlatform();
		platform->SetProjectResourceDirectory(projectResourceDirectory);
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetWindowCloseListener(LuaResource *resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SetWindowCloseListener not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::CloseWindow() const
	{
		Rtt_TRACE_SIM(("WARNING: CloseWindow not available on Linux\n"));
	}

	// Get the value of the given preference (user registry setting)
	const char *LinuxSimulatorServices::GetPreference(const char *prefName) const
	{
		//Rtt_ASSERT(0 && "todo");
		return 0;
	}

	// Set the value of the given preference (user registry setting)
	void LinuxSimulatorServices::SetPreference(const char *prefName, const char *prefValue) const
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
	void LinuxSimulatorServices::SetWindowResizeListener(LuaResource *resource) const
	{
		Rtt_TRACE_SIM(("WARNING: SetWindowResizeListener not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetCursorRect(const char *cursorName, int x, int y, int width, int height) const
	{
	}

	// Gets a list of recent projects
	void LinuxSimulatorServices::GetRecentDocs(LightPtrArray<RecentProjectInfo> *list) const
	{
		LinuxSimulatorView::GetRecentDocs(list);
	}

	const char *LinuxSimulatorServices::GetSubscription(S32 *expirationTimestamp) const
	{
		Rtt_ASSERT(0 && "todo");
		return 0;
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetWindowTitle(const char *windowTitle) const
	{
		Rtt_TRACE_SIM(("WARNING: SetWindowTitle not available on Linux\n"));
	}

	void LinuxSimulatorServices::OpenTextEditor(const char *filename) const
	{
		string command("xdg-open ");
		command.append(filename);

		wxExecute(command.c_str());
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::OpenColorPanel(double r, double g, double b, double a, LuaResource *callback) const
	{
		Rtt_TRACE_SIM(("WARNING: OpenColorPanel not available on Linux\n"));
	}

	// stub to match Mac implementation
	void LinuxSimulatorServices::SetBuildMessage(const char *message) const
	{
		Rtt_TRACE_SIM(("WARNING: SetBuildMessage not available on Linux\n"));
	}

	void LinuxSimulatorServices::SendAnalytics(const char *eventName, const char *keyName, const char *value) const
	{
	}

	bool LinuxSimulatorServices::RelaunchProject() const
	{
		wxCommandEvent e(eventRelaunchProject);
		wxPostEvent(wxGetApp().GetFrame(), e);
		return true;
	}

	bool LinuxSimulatorServices::EditProject(const char *name) const
	{
		string command("xdg-open ");
		command.append(name);

		wxExecute(command.c_str());
		return true;
	}

	bool LinuxSimulatorServices::ShowProjectFiles(const char *name) const
	{
		wxString path(name);
		path = path.SubString(0, path.size() - 10); // without main.lua
		string command("xdg-open ");
		command.append(path.c_str());

		wxExecute(command.c_str());
		return true;
	}

	bool LinuxSimulatorServices::ShowProjectSandbox(const char *name) const
	{
		const char *homeDir = GetHomePath();
		string appName = wxGetApp().GetFrame()->GetContext()->GetAppName();
		string command("xdg-open ");
		command.append(homeDir);
		command.append("/.Solar2D/Sandbox/");
		command.append(name);
		command.append("_");
		command.append(CalculateMD5(name));

		wxExecute(command.c_str());
		return true;
	}
}; // namespace Rtt
