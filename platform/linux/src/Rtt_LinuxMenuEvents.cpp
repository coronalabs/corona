#include "Rtt_LinuxMenuEvents.h"
#include "Core/Rtt_FileSystem.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxCloneProjectDialog.h"
#include "Rtt_LinuxClearSandboxDialog.h"
#include "Rtt_LinuxSimulator.h"
#include "Rtt_LinuxUtils.h"

using namespace std;

/*
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

}
*/