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
#include "Rtt_LinuxUtils.h"
#include "wx/aboutdlg.h"

using namespace std;

// file menu items

void LinuxMenuEvents::OnCloneProject(wxCommandEvent &event)
{
	Rtt::LinuxCloneProjectDialog *newCloneDlg = new Rtt::LinuxCloneProjectDialog(wxGetApp().GetFrame(), wxID_ANY, wxEmptyString);

	if (newCloneDlg->ShowModal() == wxID_OK)
	{
	}

	newCloneDlg->Destroy();
}

void LinuxMenuEvents::OnNewProject(wxCommandEvent &event)
{
	Rtt::LinuxNewProjectDialog *newProjectDlg = new Rtt::LinuxNewProjectDialog(wxGetApp().GetFrame(), wxID_ANY, wxEmptyString);

	if (newProjectDlg->ShowModal() == wxID_OK)
	{
		// open project in the simulator
		string projectPath(newProjectDlg->GetProjectFolder().c_str());
		projectPath.append("/").append(newProjectDlg->GetProjectName().c_str());
		projectPath.append("/main.lua");

		wxCommandEvent eventOpen(eventOpenProject);
		eventOpen.SetString(projectPath.c_str());
		wxPostEvent(wxGetApp().GetFrame(), eventOpen);

		// open the project folder in the file browser
		string command("xdg-open \"");
		command.append(newProjectDlg->GetProjectFolder().c_str());
		command.append("/").append(newProjectDlg->GetProjectName().c_str());
		command.append("\"");
		wxExecute(command.c_str());
	}

	newProjectDlg->Destroy();
}

void LinuxMenuEvents::OnOpenFileDialog(wxCommandEvent &event)
{
//	string startPath(GetHomePath());
//	startPath.append("/Documents/Solar2D Projects");
	wxString startPath(Rtt::LinuxSimulatorView::Config::lastProjectDirectory);

	wxFileDialog openFileDialog(wxGetApp().GetParent(), _("Open"), startPath, wxEmptyString, "Simulator Files (main.lua)|main.lua", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

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
	wxCommandEvent eventOpen(eventOpenProject);
	eventOpen.SetString(path.c_str());
	wxPostEvent(wxGetApp().GetFrame(), eventOpen);
}

void LinuxMenuEvents::OnRelaunchLastProject(wxCommandEvent &event)
{
	if (Rtt::LinuxSimulatorView::IsRunningOnSimulator())
	{
		if (!Rtt::LinuxSimulatorView::Config::lastProjectDirectory.IsEmpty())
		{
			wxCommandEvent eventOpen(eventOpenProject);
			wxString fullPath(Rtt::LinuxSimulatorView::Config::lastProjectDirectory);
			fullPath.append("/main.lua");

			eventOpen.SetInt(ID_MENU_OPEN_LAST_PROJECT);
			eventOpen.SetString(fullPath.c_str());
			wxPostEvent(wxGetApp().GetFrame(), eventOpen);
		}
	}
}

void LinuxMenuEvents::OnOpenInEditor(wxCommandEvent &event)
{
	string command("xdg-open ");
	command.append(wxGetApp().GetFrame()->GetContext()->GetAppPath());
	command.append("/main.lua");

	wxExecute(command.c_str());
}

void LinuxMenuEvents::OnShowProjectFiles(wxCommandEvent &event)
{
	const char *appPath = wxGetApp().GetFrame()->GetContext()->GetAppPath();
	string command("xdg-open ");
	command.append(appPath);

	wxExecute(command.c_str());
}

void LinuxMenuEvents::OnShowProjectSandbox(wxCommandEvent &event)
{
	const char *homeDir = GetHomePath();
	string appName = wxGetApp().GetFrame()->GetContext()->GetAppName();
	string command("xdg-open ");
	command.append(homeDir);
	command.append("/.Solar2D/Sandbox/");
	command.append(appName.c_str());
	command.append("_");
	command.append(CalculateMD5(appName.c_str()));

	wxExecute(command.c_str());
}

void LinuxMenuEvents::OnClearProjectSandbox(wxCommandEvent &event)
{
	Rtt::LinuxClearProjectSandboxDialog *clearProjectSandboxDlg = new Rtt::LinuxClearProjectSandboxDialog(wxGetApp().GetFrame(), wxID_ANY, wxEmptyString);

	if (clearProjectSandboxDlg->ShowModal() == wxID_OK)
	{
		const char *homeDir = GetHomePath();
		string appName = wxGetApp().GetFrame()->GetContext()->GetAppName();
		string command("rm -r ");
		command.append(homeDir);
		command.append("/.Solar2D/Sandbox/");
		command.append(appName);
		command.append("_");
		command.append(CalculateMD5(appName));

		wxExecute(command.c_str());
		// relaunch
		wxCommandEvent relaunchEvent(eventRelaunchProject);
		wxPostEvent(wxGetApp().GetFrame(), relaunchEvent);
	}

	clearProjectSandboxDlg->Destroy();
}

void LinuxMenuEvents::OnAndroidBackButton(wxCommandEvent &event)
{
	Rtt::PlatformInputDevice *dev = NULL;
	Rtt::KeyEvent ke(dev, Rtt::KeyEvent::kDown, "back", 0, false, false, false, false);
	wxGetApp().GetFrame()->GetContext()->GetRuntime()->DispatchEvent(ke);
}
void LinuxMenuEvents::OnOpenPreferences(wxCommandEvent &event)
{
	Rtt::LinuxPreferencesDialog *newPreferencesDialog = new Rtt::LinuxPreferencesDialog(wxGetApp().GetFrame(), wxID_ANY, wxEmptyString);
	newPreferencesDialog->SetProperties(Rtt::LinuxSimulatorView::Config::showRuntimeErrors, Rtt::LinuxSimulatorView::Config::openLastProject, Rtt::LinuxSimulatorView::Config::relaunchOnFileChange);

	if (newPreferencesDialog->ShowModal() == wxID_OK)
	{
		Rtt::LinuxSimulatorView::Config::showRuntimeErrors = newPreferencesDialog->ShouldShowRuntimeErrors();
		Rtt::LinuxSimulatorView::Config::openLastProject = newPreferencesDialog->ShouldOpenLastProject();
		Rtt::LinuxSimulatorView::Config::relaunchOnFileChange = newPreferencesDialog->ShouldRelaunchOnFileChange();
		wxGetApp().GetFrame()->GetContext()->GetPlatform()->fShowRuntimeErrors = Rtt::LinuxSimulatorView::Config::showRuntimeErrors;
		Rtt::LinuxSimulatorView::Config::Save();
		newPreferencesDialog->Destroy();
	}
}

void LinuxMenuEvents::OnQuit(wxCommandEvent &WXUNUSED(event))
{
	// quit the simulator console
	if (Rtt::LinuxSimulatorView::IsRunningOnSimulator())
	{
		ConsoleApp::Quit();
	}

	wxGetApp().GetFrame()->Close(true);
}

// build menu items
void LinuxMenuEvents::OnBuildForAndroid(wxCommandEvent &event)
{
	wxGetApp().GetFrame()->CreateSuspendedPanel();
	Rtt::AndroidBuildDialog *androidBuildDialog = new Rtt::AndroidBuildDialog(wxGetApp().GetFrame(), -1, wxEmptyString, wxDefaultPosition, wxSize(550, 470));
	androidBuildDialog->SetAppContext(wxGetApp().GetFrame()->GetContext());
	androidBuildDialog->ShowModal();
	androidBuildDialog->Destroy();
}

void LinuxMenuEvents::OnBuildForWeb(wxCommandEvent &event)
{
	wxGetApp().GetFrame()->CreateSuspendedPanel();
	Rtt::WebBuildDialog* webBuildDialog = new Rtt::WebBuildDialog(wxGetApp().GetFrame(), -1, wxEmptyString, wxDefaultPosition, wxSize(550, 330));
	webBuildDialog->SetAppContext(wxGetApp().GetFrame()->GetContext());
	webBuildDialog->ShowModal();
	webBuildDialog->Destroy();
}

void LinuxMenuEvents::OnBuildForLinux(wxCommandEvent &event)
{
	wxGetApp().GetFrame()->CreateSuspendedPanel();
	Rtt::LinuxBuildDialog *linuxBuildDialog = new Rtt::LinuxBuildDialog(wxGetApp().GetFrame(), -1, wxEmptyString, wxDefaultPosition, wxSize(550, 330));
	linuxBuildDialog->SetAppContext(wxGetApp().GetFrame()->GetContext());
	linuxBuildDialog->ShowModal();
	linuxBuildDialog->Destroy();
}

// help menu items

void LinuxMenuEvents::OnOpenDocumentation(wxCommandEvent &event)
{
	string command("xdg-open https://docs.coronalabs.com/api/index.html");
	wxExecute(command.c_str());
}

void LinuxMenuEvents::OnOpenSampleProjects(wxCommandEvent &event)
{
	string samplesPath = GetStartupPath(NULL);
	samplesPath.append("/Resources/SampleCode");
	if (!wxDirExists(samplesPath))
	{
		Rtt_LogException("%s\n not found", samplesPath.c_str());
		return;
	}

	wxFileDialog openFileDialog(wxGetApp().GetParent(), _("Open"), samplesPath.c_str(), wxEmptyString, "Simulator Files (main.lua)|main.lua", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
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
	wxCommandEvent eventOpen(eventOpenProject);
	eventOpen.SetString(path.c_str());
	wxPostEvent(wxGetApp().GetFrame(), eventOpen);
}

void LinuxMenuEvents::OnAbout(wxCommandEvent &WXUNUSED(event))
{
	wxAboutDialogInfo info;
	string version("Version: ");
	version.append(to_string(Rtt_BUILD_YEAR)).append(".").append(to_string(Rtt_LOCAL_BUILD_REVISION));
	info.SetName("Solar2D Simulator");
	info.SetVersion(version);
	info.SetCopyright(Rtt_STRING_COPYRIGHT);
	info.AddDeveloper("Danny Glover, Robert Craig. Based on initial port by the CoronaLabs team.");
	info.SetWebSite("https://solar2d.com");
	info.SetLicence(_("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\nEXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\nOF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\nNONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS\nBE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN\nACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN\nCONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\nSOFTWARE."));
	info.SetDescription(_("Solar2D is a cross-platform framework ideal for rapidly creating apps and games for mobile devices, TV, desktop systems and HTML5.\n\nThat means you can create your project once and publish it to multiple types of devices, including Apple iPhone and iPad, Android phones and tablets, Amazon Fire, Mac Desktop, Windows Desktop, Linux, HTML5 and even connected TVs such as Apple TV, Fire TV, and Android TV."));

	string iconPath = GetStartupPath(NULL);
	iconPath.append("/Resources/solar2d.png");
	if (Rtt_FileExists(iconPath.c_str()))
	{
		wxIcon icon = wxIcon(iconPath.c_str(), wxBITMAP_TYPE_PNG, 60, 60);
		info.SetIcon(icon);
	}

	::wxAboutBox(info, wxGetApp().GetFrame());
}
