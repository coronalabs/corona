#include "Rtt_LinuxBuildDialog.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaFile.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PlatformAppPackager.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_RenderingStream.h"
#include "Rtt_Runtime.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxSimulatorServices.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxBuildDialog.h"
#include "Rtt_SimulatorRecents.h"
#include "Rtt_WebAppPackager.h"
#include "Rtt_LinuxAppPackager.h"
#include "Rtt_AndroidAppPackager.h"
#include "Core/Rtt_FileSystem.h"
#include <string.h>

namespace Rtt
{
	LinuxBuildDialog::LinuxBuildDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style):
		wxDialog(parent, id, title, pos, size, wxCAPTION)
	{
		appNameTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
		appVersionTextCtrl = new wxTextCtrl(this, wxID_ANY, wxT("1.0.0"));
		appPathTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		appBuildPathTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		appBuildPathButton = new wxButton(this, wxID_OPEN, wxT("..."));
		includeWidgetResourcesCheckbox = new wxCheckBox(this, wxID_ANY, wxEmptyString);
		runAfterBuildCheckbox = new wxCheckBox(this, wxID_ANY, wxEmptyString);
		buildButton = new wxButton(this, wxID_OK, wxT("Build"));
		cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));

		SetProperties();
		DoLayout();
	}

	void LinuxBuildDialog::SetProperties()
	{
		SetTitle(wxT("Build For Linux"));
		appNameTextCtrl->SetToolTip(wxT("The name to give your projects output binary."));
		appVersionTextCtrl->SetToolTip(wxT("Your projects version number/string."));
		appPathTextCtrl->SetToolTip(wxT("The path to your projects main folder."));
		appBuildPathTextCtrl->SetToolTip(wxT("The path to your projects generated binary."));
		appBuildPathButton->SetToolTip(wxT("Click here to set a new path for your projects ouput binary."));
		includeWidgetResourcesCheckbox->SetToolTip(wxT("If your project uses the widget library, you should set this to on."));
		runAfterBuildCheckbox->SetToolTip(wxT("Enabling this will launch your built project after a successful build."));
		appBuildPathButton->SetMinSize(wxSize(40, 30));
		buildButton->SetDefault();
	}

	void LinuxBuildDialog::DoLayout()
	{
		wxBoxSizer *parentGridSizer = new wxBoxSizer(wxVERTICAL);
		wxFlexGridSizer *buttonGridSizer = new wxFlexGridSizer(1, 2, 0, 0);
		wxFlexGridSizer *optionsGridSizer = new wxFlexGridSizer(6, 3, 4, 10);
		wxStaticText *appNameLabel = new wxStaticText(this, wxID_ANY, wxT("Application Name:"));
		optionsGridSizer->Add(appNameLabel, 0, wxLEFT, 10);
		optionsGridSizer->Add(appNameTextCtrl, 0, wxEXPAND | wxRIGHT, 10);
		optionsGridSizer->Add(0, 0, 0, 0, 0);
		wxStaticText *appVersion = new wxStaticText(this, wxID_ANY, wxT("Application Version:"));
		optionsGridSizer->Add(appVersion, 0, wxLEFT, 10);
		optionsGridSizer->Add(appVersionTextCtrl, 0, wxEXPAND | wxRIGHT, 10);
		optionsGridSizer->Add(0, 0, 0, 0, 0);
		wxStaticText *appPathLabel = new wxStaticText(this, wxID_ANY, wxT("Application Path:"));
		optionsGridSizer->Add(appPathLabel, 0, wxLEFT, 10);
		optionsGridSizer->Add(appPathTextCtrl, 0, wxEXPAND | wxRIGHT, 10);
		optionsGridSizer->Add(0, 0, 0, 0, 0);
		wxStaticText *appBuildPath = new wxStaticText(this, wxID_ANY, wxT("Build Ouput Path:"));
		optionsGridSizer->Add(appBuildPath, 0, wxLEFT, 10);
		optionsGridSizer->Add(appBuildPathTextCtrl, 0, wxEXPAND | wxRIGHT, 10);
		optionsGridSizer->Add(appBuildPathButton, 0, 0, 0);
		wxStaticText *includeWidgetResourcesLabel = new wxStaticText(this, wxID_ANY, wxT("Include Widget Resources?"));
		optionsGridSizer->Add(includeWidgetResourcesLabel, 0, wxLEFT, 10);
		optionsGridSizer->Add(includeWidgetResourcesCheckbox, 0, 0, 0);
		optionsGridSizer->Add(0, 0, 0, 0, 0);
		wxStaticText *runAfterBuildLabel = new wxStaticText(this, wxID_ANY, wxT("Launch App After Build?"));
		optionsGridSizer->Add(runAfterBuildLabel, 0, wxLEFT, 10);
		optionsGridSizer->Add(runAfterBuildCheckbox, 0, 0, 0);
		optionsGridSizer->Add(0, 0, 0, 0, 0);
		optionsGridSizer->AddGrowableCol(1);
		parentGridSizer->Add(optionsGridSizer, 0, wxALL | wxEXPAND, 10);
		wxStaticLine *staticLineSeparator = new wxStaticLine(this, wxID_ANY);
		parentGridSizer->Add(staticLineSeparator, 0, wxBOTTOM | wxEXPAND | wxTOP, 5);
		buttonGridSizer->Add(buildButton, 1, wxRIGHT | wxTOP, 10);
		buttonGridSizer->Add(cancelButton, 1, wxRIGHT | wxTOP, 10);
		buttonGridSizer->AddGrowableCol(0);
		buttonGridSizer->AddGrowableCol(1);
		parentGridSizer->Add(buttonGridSizer, 1, wxALIGN_CENTER_HORIZONTAL, 0);
		SetSizer(parentGridSizer);
		Layout();
		Centre();
	}

	void LinuxBuildDialog::SetAppContext(SolarAppContext *appContext)
	{
		fAppContext = appContext;
		appNameTextCtrl->SetValue(fAppContext->GetAppName());
		appPathTextCtrl->SetValue(fAppContext->GetAppPath());
		appBuildPathTextCtrl->SetValue(fAppContext->GetSaveFolder());
	}

	BEGIN_EVENT_TABLE(LinuxBuildDialog, wxDialog)
		EVT_BUTTON(wxID_OPEN, LinuxBuildDialog::OnSelectOutputPathClicked)
		EVT_BUTTON(wxID_OK, LinuxBuildDialog::OnBuildClicked)
		EVT_BUTTON(wxID_CANCEL, LinuxBuildDialog::OnCancelClicked)
	END_EVENT_TABLE();

	void LinuxBuildDialog::OnSelectOutputPathClicked(wxCommandEvent &event)
	{
		wxDirDialog openDirDialog(this, _("Choose Output Directory"), "", 0, wxDefaultPosition);

		if (openDirDialog.ShowModal() == wxID_OK)
		{
			appBuildPathTextCtrl->SetValue(openDirDialog.GetPath());
		}
	}

	void LinuxBuildDialog::OnBuildClicked(wxCommandEvent &event)
	{
		LinuxPlatform *platform = wxGetApp().GetPlatform();
		MPlatformServices *service = new LinuxPlatformServices(platform);
		LinuxAppPackager packager(*service);
		Rtt::Runtime *runtimePointer = fAppContext->GetRuntime();
		wxString appName(appNameTextCtrl->GetValue());
		wxString sourceDir(appPathTextCtrl->GetValue());
		wxString outputDir(appBuildPathTextCtrl->GetValue());
		wxString appVersion(appVersionTextCtrl->GetValue());
		std::string linuxtemplate(platform->getInstallDir());
		bool useWidgetResources = includeWidgetResourcesCheckbox->GetValue();
		bool runAfterBuild = runAfterBuildCheckbox->GetValue();
		const char *identity = "no-identity";
		const char *customBuildId = packager.GetCustomBuildId();
		const char *bundleId = "bundleId";
		const char *sdkRoot = "";
		int targetVersion = Rtt::TargetDevice::kLinux;
		const TargetDevice::Platform targetPlatform(TargetDevice::Platform::kLinuxPlatform);
		bool isDistribution = true;
		const char kBuildSettings[] = "build.settings";
		Rtt::String buildSettingsPath;
		bool foundBuildSettings = packager.ReadBuildSettings(sourceDir.c_str());
		bool checksPassed = foundBuildSettings && !appVersion.IsEmpty() && !appName.IsEmpty();
		wxMessageDialog *resultDialog = new wxMessageDialog(wxGetApp().GetFrame(), wxEmptyString, wxT("Build Error"), wxOK | wxICON_WARNING);

		// setup paths
		const char* TEMPLATE_FILENAME = "linuxtemplate_x64.tgz";
		linuxtemplate.append("/Resources/");
		linuxtemplate.append(TEMPLATE_FILENAME);

		// pre-build validation
		if (!foundBuildSettings)
		{
			resultDialog->SetMessage(wxT("build.settings file not found in project path."));
		}

		if (appName.IsEmpty())
		{
			resultDialog->SetMessage(wxT("App name cannot be empty."));
		}

		if (appVersion.IsEmpty())
		{
			resultDialog->SetMessage(wxT("App version cannot be empty."));
		}

		// ensure we have write access to the target output directory
		
		if (wxDirExists(outputDir))
		{
			if (!wxFileName::IsDirWritable(outputDir))
			{
				resultDialog->SetMessage(wxT("No write access to the selected output directory."));
				checksPassed = false;
			}
		}
		else
		{
			if (!Rtt_MakeDirectory(outputDir))
			{
				resultDialog->SetMessage(wxT("Failed to create the selected output directory."));
				checksPassed = false;
			}
		}

		// checks failed, show failure popup
		if (!checksPassed)
		{
			resultDialog->SetTitle("Invalid Settings");
			resultDialog->ShowModal();
			return;
		}

		// check if a custom build ID has been assigned
		if (!Rtt_StringIsEmpty(customBuildId))
		{
			Rtt_Log("\nUsing custom Build Id %s\n", customBuildId);
		}

		bool onlyGetPlugins = false;
		LinuxAppPackagerParams linuxBuilderParams(
		    appName.c_str(), appVersion.c_str(), identity, NULL, sourceDir.c_str(),
		    outputDir.c_str(), NULL, targetPlatform, targetVersion,
		    Rtt::TargetDevice::kLinux, customBuildId, NULL, bundleId, isDistribution,
		    NULL, useWidgetResources, runAfterBuild, onlyGetPlugins);

		// select build template
		fAppContext->GetPlatform()->PathForFile(kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath);
		linuxBuilderParams.SetBuildSettingsPath(buildSettingsPath.GetString());

		// build the app (warning! This is blocking call)
		platform->SetActivityIndicator(true);
		int buildResult = packager.Build(&linuxBuilderParams, "/tmp/Solar2D");
		platform->SetActivityIndicator(false);
		EndModal(wxID_OK);
		wxGetApp().GetFrame()->RemoveSuspendedPanel();

		int dialogResultFlags = buildResult == 0 ? wxOK | wxICON_INFORMATION : wxOK | wxICON_ERROR;
		resultDialog->SetTitle("Build Result");
		resultDialog->SetMessage(buildResult == 0 ? "Your application was built successfully." : "Failed to build the application.\nSee the console for more info.");
		resultDialog->SetMessageDialogStyle(dialogResultFlags);

		platform->SetActivityIndicator(true);
		resultDialog->ShowModal();
		wxYield();
		platform->SetActivityIndicator(false);

		if (buildResult == 0 && runAfterBuild)
		{
			wxString command("\"");
			command.Append(outputDir);
			command.Append("/").Append(appName);
			command.Append("/").Append(appName);
			command.Append("\"");
			wxExecute(command);
		}
	}

	void LinuxBuildDialog::OnCancelClicked(wxCommandEvent &event)
	{
		wxGetApp().GetFrame()->RemoveSuspendedPanel();
		EndModal(wxID_CLOSE);
	}
};
