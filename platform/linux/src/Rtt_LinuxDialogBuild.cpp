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
#include "Rtt_AndroidBuildDialog.h"
#include "Rtt_AndroidAppPackager.h"
#include "Rtt_TargetAndroidAppStore.h"
#include "ListKeyStore.h"

using namespace std;

namespace Rtt
{

	//
	// DlgAndroidBuild
	//

	DlgAndroidBuild::DlgAndroidBuild()
		: fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir)
		, fCreateLiveBuild(false)
	{
		char uname[256] = { 0 };
		int rc = getlogin_r(uname, sizeof(uname));
		std::string package("com.solar2d.");
		package.append(uname).append(".");
		package.append(app->GetAppName());

		std::string keystorePath(GetStartupPath(NULL));
		keystorePath.append("/Resources/debug.keystore");

		string keystorePassword = "android";
		ReadKeystore(keystorePath, keystorePassword);

		*fBuildResult = 0;
		strncpy(fApplicationNameInput, app->GetAppName(), sizeof(fApplicationNameInput));
		strncpy(fVersionCodeInput, "1", sizeof(fVersionCodeInput));
		strncpy(fVersionNameInput, "1.0.0", sizeof(fVersionNameInput));
		strncpy(fPackageInput, package.c_str(), sizeof(fPackageInput));
		strncpy(fSaveToFolderInput, app->GetContext()->GetSaveFolder().c_str(), sizeof(fSaveToFolderInput));
		strncpy(fProjectPathInput, app->GetContext()->GetAppPath(), sizeof(fProjectPathInput));
		strncpy(fKeyStoreInput, app->GetContext()->GetAppPath(), sizeof(fKeyStoreInput));

		fileDialog.SetTitle("Browse For Folder");
	}

	bool DlgAndroidBuild::ReadKeystore(std::string& keystorePath, std::string& password)
	{
		fKeyAliases.clear();
		ListKeyStore listKeyStore;  // uses Java to read keystore
		if (listKeyStore.GetAliasList(keystorePath.c_str(), password.c_str()))
		{
			for (int i = 0; i < listKeyStore.GetSize(); i++)
			{
				fKeyAliases.push_back(listKeyStore.GetAlias(i));
			}

			// Succeeded and there is at least one alias - add aliases to alias dropdown
			if (fKeyAliases.empty())
			{
				Rtt_LogException("The selected keystore doesn't have any aliases.\n");
				return false;
			}
		}
		else // didn't get valid password, or keystore bad format
		{
			Rtt_LogException("The selected keystore is either invalid or an incorrect password was entered\n");
			return false;
		}
		return true;
	}

	void DlgAndroidBuild::Draw()
	{
		const ImVec2& window_size = ImGui::GetWindowSize();
		LinuxPlatform* platform = app->GetPlatform();

		fileDialog.Display();
		if (fileDialog.HasSelected())
		{
			strncpy(fSaveToFolderInput, fileDialog.GetSelected().string().c_str(), sizeof(fSaveToFolderInput));
			fileDialog.ClearSelected();
		}

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::Begin("Android Build Setup", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			string s;
			ImGui::Dummy(ImVec2(10, 10));
			ImGui::PushItemWidth(350);		// input field width

			s = "   Application Name :";
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
			ImGui::InputText("##fVersionCodeInput", fVersionCodeInput, sizeof(fVersionCodeInput), ImGuiInputTextFlags_CharsDecimal);

			s = "   Version Name :";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fVersionNameInput", fVersionNameInput, sizeof(fVersionNameInput), ImGuiInputTextFlags_CharsDecimal);

			s = "   Package :";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fPackageInput", fPackageInput, sizeof(fPackageInput), ImGuiInputTextFlags_None);

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
				fileDialog.SetTitle("Browse For Folder");
				fileDialog.SetPwd(fSaveToFolderInput);
				fileDialog.Open();
			}
			ImGui::PopItemWidth();

			ImGui::Checkbox("Create Live Build", &fCreateLiveBuild);

			// ok + cancel
			s = "Build";
			ImGui::Dummy(ImVec2(70, 40));
			int ok_width = 100;
			ImGui::SetItemDefaultFocus();
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				platform->SetActivityIndicator(true);
				fThread = new mythread();
				fThread->start([this]() { Build(); });
			}

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::End();
		}

		// builder ended ?
		if (fThread && fThread->is_running() == false)
		{
			platform->SetActivityIndicator(false);
			fThread = NULL;
		}

		if (*fBuildResult != 0)
		{
			// display result
			const char* title = "Solar2D Simulator";
			if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Dummy(ImVec2(100, 10));
				ImGui::Text(fBuildResult);

				string s = "View";
				ImGui::Dummy(ImVec2(100, 30));
				int ok_width = 100;
				ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
				if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
				{
					OpenURL(fSaveToFolderInput);
					PushEvent(sdl::onClosePopupModal);
				}
				ImGui::SetItemDefaultFocus();

				s = "Done";
				ImGui::SameLine();
				if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
				{
					PushEvent(sdl::onClosePopupModal);
				}
				ImGui::EndPopup();
			}
			ImGui::OpenPopup(title);
		}
	}

	// running in separate thread!
	void DlgAndroidBuild::Build()
	{
		lock_guard<mutex> lock(app->EngineMutex());
		/*
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

				// setup paths
				const char* TEMPLATE_FILENAME = "linuxtemplate_x64.tgz";
				linuxtemplate.append("/Resources/");
				linuxtemplate.append(TEMPLATE_FILENAME);

				// pre-build validation
				if (!foundBuildSettings)
				{
					strncpy(fBuildResult, "build.settings file not found in project path.", sizeof(fBuildResult));
					return;
				}

				if (appName.empty())
				{
					strncpy(fBuildResult, "App name cannot be empty.", sizeof(fBuildResult));
					return;
				}

				if (appVersion.empty())
				{
					strncpy(fBuildResult, "App version cannot be empty.", sizeof(fBuildResult));
					return;
				}

				// ensure we have write access to the target output directory
				if (!Rtt_IsDirectory(outputDir.c_str()) && !Rtt_MakeDirectory(outputDir.c_str()))
				{
					strncpy(fBuildResult, "Failed to create the selected output directory.", sizeof(fBuildResult));
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
				int buildResult = packager.Build(&linuxBuilderParams, "/tmp/Solar2D");

				strncpy(fBuildResult, buildResult == 0 ? "Your application was built successfully." : "Failed to build the application.\nSee the console for more info.", sizeof(fBuildResult));
				*/
	}

	//
	// DlgHTML5Build
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
		: fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir)
		, fIncludeStandardResources(false)
	{
		*fBuildResult = 0;
		strncpy(fApplicationNameInput, app->GetAppName(), sizeof(fApplicationNameInput));
		strncpy(fVersionInput, "1.0.0", sizeof(fVersionInput));
		strncpy(fSaveToFolderInput, app->GetContext()->GetSaveFolder().c_str(), sizeof(fSaveToFolderInput));
		strncpy(fProjectPathInput, app->GetContext()->GetAppPath(), sizeof(fProjectPathInput));

		fileDialog.SetTitle("Browse For Folder");
	}

	void DlgLinuxBuild::Draw()
	{
		const ImVec2& window_size = ImGui::GetWindowSize();
		LinuxPlatform* platform = app->GetPlatform();

		fileDialog.Display();
		if (fileDialog.HasSelected())
		{
			strncpy(fSaveToFolderInput, fileDialog.GetSelected().string().c_str(), sizeof(fSaveToFolderInput));
			fileDialog.ClearSelected();
		}

		// Always center this window when appearing
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::Begin("Linux Build Setup", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			string s;
			ImGui::Dummy(ImVec2(10, 10));
			ImGui::PushItemWidth(350);		// input field width

			s = "   Application Name :";
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

			s = "   Save To Folder: ";
			ImGui::Text(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fSaveToFolderInput", fSaveToFolderInput, sizeof(fSaveToFolderInput), 0);
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				fileDialog.SetTitle("Browse For Folder");
				fileDialog.SetPwd(fSaveToFolderInput);
				fileDialog.Open();
			}
			ImGui::PopItemWidth();

			ImGui::Checkbox("Include Standard Resources", &fIncludeStandardResources);

			// ok + cancel
			s = "Build";
			ImGui::Dummy(ImVec2(70, 40));
			int ok_width = 100;
			ImGui::SetItemDefaultFocus();
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				platform->SetActivityIndicator(true);
				fThread = new mythread();
				fThread->start([this]() { Build(); });
			}

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onClosePopupModal);
			}
			ImGui::End();
		}

		// builder ended ?
		if (fThread && fThread->is_running() == false)
		{
			platform->SetActivityIndicator(false);
			fThread = NULL;
		}

		if (*fBuildResult != 0)
		{
			// display result
			const char* title = "Solar2D Simulator";
			if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Dummy(ImVec2(100, 10));
				ImGui::Text(fBuildResult);

				string s = "View";
				ImGui::Dummy(ImVec2(100, 30));
				int ok_width = 100;
				ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
				if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
				{
					OpenURL(fSaveToFolderInput);
					PushEvent(sdl::onClosePopupModal);
				}
				ImGui::SetItemDefaultFocus();

				s = "Done";
				ImGui::SameLine();
				if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
				{
					PushEvent(sdl::onClosePopupModal);
				}
				ImGui::EndPopup();
			}
			ImGui::OpenPopup(title);


		}
	}

	// running in separate thread!
	void DlgLinuxBuild::Build()
	{
		lock_guard<mutex> lock(app->EngineMutex());

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

		// setup paths
		const char* TEMPLATE_FILENAME = "linuxtemplate_x64.tgz";
		linuxtemplate.append("/Resources/");
		linuxtemplate.append(TEMPLATE_FILENAME);

		// pre-build validation
		if (!foundBuildSettings)
		{
			strncpy(fBuildResult, "build.settings file not found in project path.", sizeof(fBuildResult));
			return;
		}

		if (appName.empty())
		{
			strncpy(fBuildResult, "App name cannot be empty.", sizeof(fBuildResult));
			return;
		}

		if (appVersion.empty())
		{
			strncpy(fBuildResult, "App version cannot be empty.", sizeof(fBuildResult));
			return;
		}

		// ensure we have write access to the target output directory
		if (!Rtt_IsDirectory(outputDir.c_str()) && !Rtt_MakeDirectory(outputDir.c_str()))
		{
			strncpy(fBuildResult, "Failed to create the selected output directory.", sizeof(fBuildResult));
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
		int buildResult = packager.Build(&linuxBuilderParams, "/tmp/Solar2D");

		strncpy(fBuildResult, buildResult == 0 ? "Your application was built successfully." : "Failed to build the application.\nSee the console for more info.", sizeof(fBuildResult));
	}


}	// Rtt

