//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxDialogBuild.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxApp.h"
#include "Rtt_Version.h"
#include "Rtt_FileSystem.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxAppPackager.h"

using namespace std;

namespace Rtt
{


	//
	// DlgAndroidBuild
	//

	void DlgAndroidBuild::Draw()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		const char* title = "Solar2D Simulator";
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Dummy(ImVec2(100, 10));
			ImGui::Text("   Solar2D project has been modified");
			ImGui::Text("Whould you like to relaunch the project?");

			// ok + cancel
			string s = "Relaunch";
			ImGui::Dummy(ImVec2(100, 30));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::SetItemDefaultFocus();

			s = "Ignore";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::EndPopup();
		}
		ImGui::OpenPopup(title);
	}

	//
	// DlgAndroidBuild
	//

	void DlgHTML5Build::Draw()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		const char* title = "Solar2D Simulator";
		if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();

			ImGui::Dummy(ImVec2(100, 10));
			ImGui::Text("   Solar2D project has been modified");
			ImGui::Text("Whould you like to relaunch the project?");

			// ok + cancel
			string s = "Relaunch";
			ImGui::Dummy(ImVec2(100, 30));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::SetItemDefaultFocus();

			s = "Ignore";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::EndPopup();
		}
		ImGui::OpenPopup(title);
	}

	//
	// DlgLinuxdBuild
	//

	DlgLinuxBuild::DlgLinuxBuild()
		: fIncludeStandardResources(false)
	{
		strcpy(fApplicationNameInput, app->GetAppName());
		strcpy(fVersionInput, "1.0.0");
		strcpy(fSaveToFolderInput, app->GetContext()->GetSaveFolder().c_str());
		strcpy(fProjectPathInput, app->GetContext()->GetAppPath());
	}

	void DlgLinuxBuild::Draw()
	{
		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		const char* title = "Linux Build Setup";
		if (ImGui::Begin(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();
			ImGui::Dummy(ImVec2(10, 10));
			ImGui::PushItemWidth(350);		// input field width

			string s = "   Application Name :";
			float label_width = ImGui::CalcTextSize(s.c_str()).x;
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ApplicationName", fApplicationNameInput, sizeof(fApplicationNameInput), ImGuiInputTextFlags_CharsNoBlank);

			s = "   Version Code :";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fVersionInput", fVersionInput, sizeof(fVersionInput), ImGuiInputTextFlags_CharsDecimal);

			s = "   Project Path :";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fProjectPathInput", fProjectPathInput, sizeof(fProjectPathInput), ImGuiInputTextFlags_ReadOnly);

			s = "   Save To Folder: ";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fSaveToFolderInput", fSaveToFolderInput, sizeof(fSaveToFolderInput), 0);
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				fileDialog.SetPwd(fSaveToFolderInput);
				fileDialog.Open();
			}
			ImGui::PopItemWidth();

			ImGui::Checkbox("Include Standard Resources", &fIncludeStandardResources);


			// ok + cancel
			s = "OK";
			ImGui::Dummy(ImVec2(100, 30));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				Build();
			}
			ImGui::SetItemDefaultFocus();

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::End();
		}
		ImGui::OpenPopup(title);
	}

	void DlgLinuxBuild::Build()
	{
		LinuxPlatform* platform = app->GetPlatform();
		MPlatformServices* service = new LinuxPlatformServices(platform);
		LinuxAppPackager packager(*service);
		Rtt::Runtime* runtimePointer = app->GetContext()->GetRuntime();
		string appName(fApplicationNameInput);
		string sourceDir(fProjectPathInput);
		string outputDir(fSaveToFolderInput);
		string appVersion(fVersionInput);
		std::string linuxtemplate(platform->getInstallDir());
		bool runAfterBuild = false; // runAfterBuildCheckbox->GetValue();
		const char* identity = "no-identity";
		const char* customBuildId = packager.GetCustomBuildId();
		const char* bundleId = "bundleId";
		const char* sdkRoot = "";
		int targetVersion = Rtt::TargetDevice::kLinux;
		const TargetDevice::Platform targetPlatform(TargetDevice::Platform::kLinuxPlatform);
		bool isDistribution = true;
		const char kBuildSettings[] = "build.settings";
		Rtt::String buildSettingsPath;
		bool foundBuildSettings = packager.ReadBuildSettings(sourceDir.c_str());
		//	bool checksPassed = foundBuildSettings && !appVersion.empty() && !appName.empty();
	//		wxMessageDialog* resultDialog = new wxMessageDialog(solarApp, wxEmptyString, wxT("Build Error"), wxOK | wxICON_WARNING);

			// setup paths
		const char* TEMPLATE_FILENAME = "linuxtemplate_x64.tgz";
		linuxtemplate.append("/Resources/");
		linuxtemplate.append(TEMPLATE_FILENAME);

		// pre-build validation
		if (!foundBuildSettings)
		{
			//	msgBox("build.settings file not found in project path.");
			return;
		}

		if (appName.empty())
		{
			//resultDialog->SetMessage(wxT("App name cannot be empty."));
			return;
		}

		if (appVersion.empty())
		{
			//resultDialog->SetMessage(wxT("App version cannot be empty."));
			return;
		}

		// ensure we have write access to the target output directory
		if (!Rtt_IsDirectory(outputDir.c_str()) && !Rtt_MakeDirectory(outputDir.c_str()))
		{
			//resultDialog->SetMessage(wxT("Failed to create the selected output directory."));
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
			NULL, fIncludeStandardResources, runAfterBuild, onlyGetPlugins);

		// select build template
		app->GetContext()->GetPlatform()->PathForFile(kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath);
		linuxBuilderParams.SetBuildSettingsPath(buildSettingsPath.GetString());

		// build the app (warning! This is blocking call)
		platform->SetActivityIndicator(true);
		int buildResult = packager.Build(&linuxBuilderParams, "/tmp/Solar2D");
		platform->SetActivityIndicator(false);
		//		EndModal(wxID_OK);
			//	solarSimulator->RemoveSuspendedPanel();

		int dialogResultFlags = buildResult == 0 ? wxOK | wxICON_INFORMATION : wxOK | wxICON_ERROR;
		//resultDialog->SetTitle("Build Result");
		//resultDialog->SetMessage(buildResult == 0 ? "Your application was built successfully." : "Failed to build the application.\nSee the console for more info.");
		//resultDialog->SetMessageDialogStyle(dialogResultFlags);

		platform->SetActivityIndicator(true);
		//resultDialog->ShowModal();
		//wxYield();
		platform->SetActivityIndicator(false);
	}

}	// Rtt

