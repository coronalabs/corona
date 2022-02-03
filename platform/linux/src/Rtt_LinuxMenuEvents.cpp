#include "Rtt_LinuxMenuEvents.h"
#include "Core/Rtt_FileSystem.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxCloneProjectDialog.h"
#include "Rtt_LinuxNewProjectDialog.h"
#include "Rtt_LinuxPreferencesDialog.h"
#include "Rtt_LinuxBuildDialog.h"
#include "Rtt_AndroidBuildDialog.h"
#include "Rtt_WebBuildDialog.h"
#include "Rtt_LinuxClearSandboxDialog.h"
#include "Rtt_ConsoleApp.h"
#include "Rtt_LinuxSimulator.h"
#include "Rtt_LinuxUtils.h"
#include "wx/aboutdlg.h"

using namespace std;

// file menu items

namespace Rtt
{

	void SolarSimulator::OnCloneProject(wxCommandEvent& event)
	{
		Rtt::LinuxCloneProjectDialog* newCloneDlg = new Rtt::LinuxCloneProjectDialog(solarApp, wxID_ANY, wxEmptyString);

		if (newCloneDlg->ShowModal() == wxID_OK)
		{
		}

		newCloneDlg->Destroy();
	}

	void SolarSimulator::OnNewProject(wxCommandEvent& event)
	{
		Rtt::LinuxNewProjectDialog* newProjectDlg = new Rtt::LinuxNewProjectDialog(solarApp, wxID_ANY, wxEmptyString);

		if (newProjectDlg->ShowModal() == wxID_OK)
		{
			// open project in the simulator
			string projectPath(newProjectDlg->GetProjectFolder().c_str());
			projectPath.append("/").append(newProjectDlg->GetProjectName().c_str());
			projectPath.append("/main.lua");

		//	wxCommandEvent eventOpen(eventOpenProject);
		//	eventOpen.SetString(projectPath.c_str());
		//	wxPostEvent(solarApp, eventOpen);

			// open the project folder in the file browser
			string command("xdg-open \"");
			command.append(newProjectDlg->GetProjectFolder().c_str());
			command.append("/").append(newProjectDlg->GetProjectName().c_str());
			command.append("\"");
			wxExecute(command.c_str());
		}

		newProjectDlg->Destroy();
	}

	void SolarSimulator::OnOpenFileDialog(wxCommandEvent& event)
	{
		//	string startPath(GetHomePath());
		//	startPath.append("/Documents/Solar2D Projects");
		wxString startPath(solarSimulator->ConfigStr("lastProjectDirectory"));

		wxFileDialog openFileDialog(solarApp, _("Open"), startPath, wxEmptyString, "Simulator Files (main.lua)|main.lua", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (openFileDialog.ShowModal() == wxID_CANCEL)
		{
			return;
		}

		wxString path = openFileDialog.GetPath();
		if (!Rtt_FileExists(path.c_str()))
		{
			return;
		}

		// open project
	//	wxCommandEvent eventOpen(eventOpenProject);
	//	eventOpen.SetString(path.c_str());
	//	wxPostEvent(solarApp, eventOpen);
	}

	void SolarSimulator::OnRelaunchLastProject(wxCommandEvent& event)
	{
		const string& lastProjectDirectory = solarSimulator->ConfigStr("lastProjectDirectory");
		if (lastProjectDirectory.size() > 0)
		{
	//		wxCommandEvent eventOpen(eventOpenProject);
	//		wxString fullPath(lastProjectDirectory);
	//		fullPath.append("/main.lua");

	//		eventOpen.SetInt(ID_MENU_OPEN_LAST_PROJECT);
	//		eventOpen.SetString(fullPath.c_str());
	//		wxPostEvent(solarApp, eventOpen);
		}
	}

	void SolarSimulator::OnOpenInEditor()
	{
		string command("xdg-open ");
		command.append(GetContext()->GetAppPath());
		command.append("/main.lua");

		wxExecute(command.c_str());
	}

	void SolarSimulator::OnShowProjectFiles(wxCommandEvent& event)
	{
		const char* appPath = GetContext()->GetAppPath();
		string command("xdg-open ");
		command.append(appPath);

		wxExecute(command.c_str());
	}

	void SolarSimulator::OnShowProjectSandbox(wxCommandEvent& event)
	{
		const char* homeDir = GetHomePath();
		const string& appName = GetContext()->GetAppName();
		string command("xdg-open ");
		command.append(homeDir);
		command.append("/.Solar2D/Sandbox/");
		command.append(appName);
		command.append("_");
		command.append(CalculateMD5(appName));

		wxExecute(command.c_str());
	}

	void SolarSimulator::OnClearProjectSandbox(wxCommandEvent& event)
	{
		Rtt::LinuxClearProjectSandboxDialog* clearProjectSandboxDlg = new Rtt::LinuxClearProjectSandboxDialog(solarApp, wxID_ANY, wxEmptyString);

		if (clearProjectSandboxDlg->ShowModal() == wxID_OK)
		{
			const char* homeDir = GetHomePath();
			string appName = GetContext()->GetAppName();
			string command("rm -r ");
			command.append(homeDir);
			command.append("/.Solar2D/Sandbox/");
			command.append(appName);
			command.append("_");
			command.append(CalculateMD5(appName));

			wxExecute(command.c_str());
			// relaunch
	//		wxCommandEvent relaunchEvent(eventRelaunchProject);
	//		wxPostEvent(solarApp, relaunchEvent);
		}

		clearProjectSandboxDlg->Destroy();
	}

	void SolarSimulator::OnAndroidBackButton(wxCommandEvent& event)
	{
		Rtt::PlatformInputDevice* dev = NULL;
		Rtt::KeyEvent ke(dev, Rtt::KeyEvent::kDown, "back", 0, false, false, false, false);
		GetContext()->GetRuntime()->DispatchEvent(ke);
	}
	void SolarSimulator::OnOpenPreferences(wxCommandEvent& event)
	{
		Rtt::LinuxPreferencesDialog* newPreferencesDialog = new Rtt::LinuxPreferencesDialog(solarApp, wxID_ANY, wxEmptyString);
		//		newPreferencesDialog->SetProperties(Rtt::LinuxSimulatorView::Config::showRuntimeErrors, Rtt::LinuxSimulatorView::Config::openLastProject, Rtt::LinuxSimulatorView::Config::relaunchOnFileChange);
		newPreferencesDialog->SetProperties(true, solarSimulator->ConfigInt("openLastProject"), solarSimulator->ConfigStr("relaunchOnFileChange"));

		if (newPreferencesDialog->ShowModal() == wxID_OK)
		{
			solarSimulator->ConfigSet("showRuntimeErrors", true); // newPreferencesDialog->ShouldShowRuntimeErrors();
			solarSimulator->ConfigSet("openLastProject", newPreferencesDialog->ShouldOpenLastProject());
			solarSimulator->ConfigSet("relaunchOnFileChange", newPreferencesDialog->ShouldRelaunchOnFileChange());
			GetContext()->GetPlatform()->fShowRuntimeErrors = true; // Rtt::LinuxSimulatorView::Config::showRuntimeErrors;
			solarSimulator->ConfigSave();
			newPreferencesDialog->Destroy();
		}
	}

	void SolarSimulator::OnQuit(wxCommandEvent& WXUNUSED(event))
	{
		// quit the simulator console
		ConsoleApp::Quit();

		//Close(true);
	}

	// build menu items
	void SolarSimulator::OnBuildForAndroid(wxCommandEvent& event)
	{
		CreateSuspendedPanel();
		Rtt::AndroidBuildDialog* androidBuildDialog = new Rtt::AndroidBuildDialog(solarApp, -1, wxEmptyString, wxDefaultPosition, wxSize(550, 470));
		androidBuildDialog->SetAppContext(GetContext());
		androidBuildDialog->ShowModal();
		androidBuildDialog->Destroy();
	}

	void SolarSimulator::OnBuildForWeb(wxCommandEvent& event)
	{
		CreateSuspendedPanel();
		Rtt::WebBuildDialog* webBuildDialog = new Rtt::WebBuildDialog(solarApp, -1, wxEmptyString, wxDefaultPosition, wxSize(550, 330));
		webBuildDialog->SetAppContext(GetContext());
		webBuildDialog->ShowModal();
		webBuildDialog->Destroy();
	}

	void SolarSimulator::OnBuildForLinux(wxCommandEvent& event)
	{
		CreateSuspendedPanel();
		Rtt::LinuxBuildDialog* linuxBuildDialog = new Rtt::LinuxBuildDialog(solarApp, -1, wxEmptyString, wxDefaultPosition, wxSize(550, 330));
		linuxBuildDialog->SetAppContext(GetContext());
		linuxBuildDialog->ShowModal();
		linuxBuildDialog->Destroy();
	}

	// help menu items

	void SolarSimulator::OnOpenDocumentation()
	{
		string command("xdg-open https://docs.coronalabs.com/api/index.html");
		wxExecute(command.c_str());
	}

	void SolarSimulator::OnOpenSampleProjects()
	{
		string samplesPath = GetStartupPath(NULL);
		samplesPath.append("/Resources/SampleCode");
		if (!wxDirExists(samplesPath))
		{
			Rtt_LogException("%s\n not found", samplesPath.c_str());
			return;
		}

		wxFileDialog openFileDialog(solarApp, _("Open"), samplesPath.c_str(), wxEmptyString, "Simulator Files (main.lua)|main.lua", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (openFileDialog.ShowModal() == wxID_CANCEL)
		{
			return;
		}

		wxString path = openFileDialog.GetPath();
		if (!Rtt_FileExists(path.c_str()))
		{
			Rtt_LogException("%s\n not found", path.c_str());
			return;
		}

		// open project
		SDL_Event e = {};
		e.type = sdl::OnOpen;
		e.user.data1 = strdup(path.c_str());
		SDL_PushEvent(&e);
	}

}
