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
#include "Rtt_AndroidAppPackager.h"
#include "Rtt_WebAppPackager.h"
#include "Rtt_TargetAndroidAppStore.h"
#include "ListKeyStore.h"

using namespace std;

namespace Rtt
{

	DlgBuild::DlgBuild(const std::string& title, int w, int h)
		: Window(title, w, h)
	{
		app->Pause();
	}

	DlgBuild::~DlgBuild()
	{
		app->Resume();
	}

	//
	// DlgAndroidBuild
	//

	DlgAndroidBuild::DlgAndroidBuild(const std::string& title, int w, int h)
		: DlgBuild(title, w, h)
		, fileDialogKeyStore(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CloseOnEsc)
		, fileDialogSaveTo(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_CloseOnEsc)
		, fBuildResult(NULL)
		, fBuildSuccessed(0)
		, fCreateLiveBuild(false)
		, fAppStoreIndex(1)		// google play
		, fKeyAliases(NULL)
		, fKeyAliasesSize(0)
		, fKeyAliasIndex(0)
	{
		char uname[256] = { 0 };
		int rc = getlogin_r(uname, sizeof(uname));
		std::string package("com.solar2d.");
		package.append(uname).append(".");
		package.append(app->GetAppName());

		// defaults
		strncpy(fApplicationNameInput, app->GetAppName(), sizeof(fApplicationNameInput));
		strncpy(fVersionCodeInput, "1", sizeof(fVersionCodeInput));
		strncpy(fVersionNameInput, "1.0.0", sizeof(fVersionNameInput));
		strncpy(fPackageInput, package.c_str(), sizeof(fPackageInput));
		strncpy(fSaveToFolderInput, app->GetContext()->GetSaveFolder().c_str(), sizeof(fSaveToFolderInput));
		strncpy(fProjectPathInput, app->GetContext()->GetAppPath(), sizeof(fProjectPathInput));

		std::string keystorePath(GetStartupPath(NULL));
		keystorePath.append("/Resources/debug.keystore");
		strncpy(fKeyStoreInput, keystorePath.c_str(), sizeof(fKeyStoreInput));

		fStorePassword = "android";
		fAliasPassword = fStorePassword;
		ReadKeystore(keystorePath, fStorePassword);
		ReadVersion();

		fileDialogKeyStore.SetTitle("Browse For Keystore");
		fileDialogKeyStore.SetWindowSize(w, h);
		fileDialogSaveTo.SetTitle("Browse For Folder");
		fileDialogSaveTo.SetWindowSize(w, h);
	}

	DlgAndroidBuild::~DlgAndroidBuild()
	{
		ClearKeyAliases();
	}

	void DlgAndroidBuild::ClearKeyAliases()
	{
		if (fKeyAliases)
		{
			for (int i = 0; i < fKeyAliasesSize; i++)
			{
				free(fKeyAliases[i]);
			}
			delete[] fKeyAliases;
		}
		fKeyAliasesSize = 0;
		fKeyAliasIndex = 0;
	}

	bool DlgAndroidBuild::ReadKeystore(const std::string& keystorePath, const std::string& password)
	{
		ClearKeyAliases();
		ListKeyStore listKeyStore;  // uses Java to read keystore
		if (listKeyStore.GetAliasList(keystorePath.c_str(), password.c_str()))
		{
			// Succeeded and there is at least one alias - add aliases to alias dropdown
			if (listKeyStore.GetSize() == 0)
			{
				Rtt_LogException("The selected keystore doesn't have any aliases.\n");
				return false;
			}
			else
			{
				fKeyAliasesSize = listKeyStore.GetSize();
				fKeyAliases = new char* [fKeyAliasesSize];
				for (int i = 0; i < listKeyStore.GetSize(); i++)
				{
					const char* s = listKeyStore.GetAlias(i);
					fKeyAliases[i] = (char*)malloc(strlen(s) + 1);
					strcpy(fKeyAliases[i], s);
				}
			}
		}
		else // didn't get valid password, or keystore bad format
		{
			Rtt_LogException("The selected keystore is either invalid or an incorrect password was entered\n");
			return false;
		}
		return true;
	}

	void DlgAndroidBuild::ReadVersion()
	{
		// Get the version code from build.settings
		const char kBuildSettings[] = "build.settings";
		String filePath;
		app->GetPlatform()->PathForFile(kBuildSettings, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath);
		lua_State* L = app->GetRuntime()->VMContext().L();
		const char* buildSettingsPath = filePath.GetString();

		if (buildSettingsPath && 0 == luaL_loadfile(L, buildSettingsPath) && 0 == lua_pcall(L, 0, 0, 0))
		{
			lua_getglobal(L, "settings");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "android");
				if (lua_istable(L, -1))
				{
					lua_getfield(L, -1, "versionCode");
					if (lua_isstring(L, -1))
					{
						strncpy(fVersionCodeInput, lua_tostring(L, -1), sizeof(fVersionCodeInput));
					}
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
			}
		}
	}

	void DlgAndroidBuild::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgAndroidBuild", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();
			LinuxPlatform* platform = app->GetPlatform();

			fileDialogKeyStore.Display();
			if (fileDialogKeyStore.HasSelected())
			{
				strncpy(fKeyStoreInput, fileDialogKeyStore.GetSelected().string().c_str(), sizeof(fKeyStoreInput));
				fileDialogKeyStore.ClearSelected();
			}

			fileDialogSaveTo.Display();
			if (fileDialogSaveTo.HasSelected())
			{
				strncpy(fSaveToFolderInput, fileDialogSaveTo.GetSelected().string().c_str(), sizeof(fSaveToFolderInput));
				fileDialogSaveTo.ClearSelected();
			}

			string s;
			ImGui::Dummy(ImVec2(10, 10));
			ImGui::PushItemWidth(350);		// input field width

			s = "   Application Name :";
			float label_width = ImGui::CalcTextSize(s.c_str()).x;
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ApplicationName", fApplicationNameInput, sizeof(fApplicationNameInput), ImGuiInputTextFlags_CharsNoBlank);

			s = "   Version Code :";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fVersionCodeInput", fVersionCodeInput, sizeof(fVersionCodeInput), ImGuiInputTextFlags_CharsDecimal);

			s = "   Version Name :";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fVersionNameInput", fVersionNameInput, sizeof(fVersionNameInput), ImGuiInputTextFlags_CharsDecimal);

			s = "   Package :";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fPackageInput", fPackageInput, sizeof(fPackageInput), ImGuiInputTextFlags_None);

			s = "A unique Java-style package identifier for your app\n(e.g. com.acme.games.myframegame)";
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::TextUnformatted(s.c_str());
			ImGui::Dummy(ImVec2(70, 10));

			s = "   Project Path :";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fProjectPathInput", fProjectPathInput, sizeof(fProjectPathInput), ImGuiInputTextFlags_ReadOnly);

			// Target App Store
			ImGui::TextUnformatted("   Target App Store :");
			ImGui::SameLine();
			const char* appstores[] = { "Amazon", "Google Play", "Samsung" };
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::Combo("##TargetAppStore", &fAppStoreIndex, appstores, IM_ARRAYSIZE(appstores));

			s = "   Keystore: ";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fSaveToFolderInput", fKeyStoreInput, sizeof(fKeyStoreInput), 0);
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				fileDialogKeyStore.SetPwd(fKeyStoreInput);
				fileDialogKeyStore.Open();
			}

			// Key Alias
			ImGui::TextUnformatted("   Key Alias :");
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::Combo("##TargetAppStore", &fKeyAliasIndex, fKeyAliases, fKeyAliasesSize);

			s = "   Save To Folder: ";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fSaveToFolderInput", fSaveToFolderInput, sizeof(fSaveToFolderInput), 0);
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				fileDialogSaveTo.SetPwd(fSaveToFolderInput);
				fileDialogSaveTo.Open();
			}

			ImGui::PopItemWidth();

			ImGui::Dummy(ImVec2(70, 10));
			ImGui::Checkbox("Create Live Build", &fCreateLiveBuild);

			// ok + cancel
			s = "Build";
			ImGui::Dummy(ImVec2(70, 40));
			int ok_width = 100;
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				fThread = new mythread();
				fThread->start([this]() { Build(); });
			}
			ImGui::SetItemDefaultFocus();

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onCloseDialog);
			}

			if (fBuildResult)
			{
				// display result
				const char* title = "Solar2D Simulator";
				if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Dummy(ImVec2(100, 10));
					ImGui::TextUnformatted(fBuildResult);

					string s;
					ImGui::Dummy(ImVec2(100, 30));
					int ok_width = 100;
					ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
					if (fBuildSuccessed == 0)
					{
						s = "View";
						if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
						{
							OpenURL(fSaveToFolderInput);
							PushEvent(sdl::onCloseDialog);
						}
						ImGui::SameLine();
					}

					s = "Done";
					if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
					{
						PushEvent(sdl::onCloseDialog);
					}
					ImGui::SetItemDefaultFocus();
					ImGui::EndPopup();
				}
				ImGui::OpenPopup(title);
			}
			ImGui::End();
		}

		// builder ended ?
		if (fThread)
		{
			DrawActivity();
			if (fThread->is_running() == false)
			{
				fThread = NULL;
			}
		}

		end();
	}

	// thread function
	void DlgAndroidBuild::Build()
	{
		fBuildSuccessed = false;
		LinuxPlatform* platform = app->GetPlatform();
		MPlatformServices* service = new LinuxPlatformServices(platform);
		Rtt::Runtime* runtimePointer = app->GetRuntime();
		std::string androidTemplate(platform->getInstallDir());
		std::string tmp = Rtt_GetSystemTempDirectory();
		std::string targetAppStoreName(TargetAndroidAppStore::kGoogle.GetStringId());
		const char* identity = "no-identity";
		const char* bundleId = "bundleId";
		const char* provisionFile = "";
		int versionCode = atoi(fVersionCodeInput);
		bool installAfterBuild = false; // installToDeviceCheckbox->GetValue() == true;
		const TargetDevice::Platform targetPlatform(TargetDevice::Platform::kAndroidPlatform);
		bool isDistribution = true;
		const char kBuildSettings[] = "build.settings";
		Rtt::String buildSettingsPath;

		// setup paths
		androidTemplate.append("/Resources");
		tmp.append("/CoronaLabs");

		AndroidAppPackager packager(*service, androidTemplate.c_str());
		bool foundBuildSettings = packager.ReadBuildSettings(fProjectPathInput);
		const char* customBuildId = packager.GetCustomBuildId();

		// pre-build validation
		if (!foundBuildSettings)
		{
			fBuildResult = "build.settings file not found in project path.";
			return;
		}

		if (*fApplicationNameInput == 0)
		{
			fBuildResult = "App name cannot be empty.";
			return;
		}

		if (*fVersionNameInput == 0)
		{
			fBuildResult = "App version cannot be empty.";
			return;
		}

		if (fVersionNameInput[0] == '.' || fVersionNameInput[strlen(fVersionNameInput)] == '.')
		{
			fBuildResult = "App version cannot start or end with a period (dot).";
			return;
		}

		if (*fVersionCodeInput == 0)
		{
			fBuildResult = "App version code cannot be empty.";
			return;
		}

		if (versionCode <= 0)
		{
			fBuildResult = "App version code must be an integer.";
			return;
		}

		if (*fPackageInput == 0)
		{
			fBuildResult = "App package name cannot be empty.";
			return;
		}

		if (strchr(fPackageInput, '.') == NULL)
		{
			fBuildResult = "App package name must contain at least one period (dot).";
			return;
		}

		if (fPackageInput[0] == '.' || fPackageInput[strlen(fPackageInput)] == '.')
		{
			fBuildResult = "App package name cannot start or end with a period (dot).";
			return;
		}

		if (fKeyAliasesSize == 0)
		{
			fBuildResult = "The keystore password is invalid.";
			return;
		}

		// ensure unzip is installed on the users system
		if (!Rtt_FileExists("/usr/bin/unzip"))
		{
			fBuildResult = "/usr/bin/unzip not found";
			return;
		}

		string exePath = GetStartupPath(NULL);

		// ensure Solar2DBuilder exists at the correct location
		string solar2DBuilderPath = exePath;
		solar2DBuilderPath.append("/Solar2DBuilder");
		if (!Rtt_FileExists(solar2DBuilderPath.c_str()))
		{
			fBuildResult = "Solar2DBuilder is not found";
			return;
		}

		// ensure Resource is not a link
		string resourcesPath = exePath;
		resourcesPath.append("/Resources");
		struct stat buf;
		if (lstat(resourcesPath.c_str(), &buf) == 0)
		{
			if (S_ISLNK(buf.st_mode))
			{
				fBuildResult = "Resources path is a link, it must be regular folder";
				return;
			}
		}
		else
		{
			fBuildResult = "No Resources found";
			return;
		}

		string outputDir(fSaveToFolderInput);
		outputDir.append("/");
		outputDir.append(fApplicationNameInput);
		outputDir.append(".Android");

		// ensure we have write access to the target output directory
		if (!Rtt_IsDirectory(outputDir.c_str()))
		{
			if (!Rtt_MakeDirectory(outputDir.c_str()))
			{
				fBuildResult = "Failed to create the selected output directory.";
				return;
			}
		}

		Rtt_Log("\nStarting Android build...\n");

		// check if a custom build ID has been assigned
		if (!Rtt_StringIsEmpty(customBuildId))
		{
			Rtt_Log("Using custom Build Id %s\n", customBuildId);
		}

		// set the target app store
		if (fAppStoreIndex == 1) // GOOGLE_PLAY_STORE_TARGET
		{
			targetAppStoreName = TargetAndroidAppStore::kGoogle.GetStringId();
		}
		else if (fAppStoreIndex == 0) // AMAZON_STORE_TARGET
		{
			targetAppStoreName = TargetAndroidAppStore::kAmazon.GetStringId();
		}
		else if (fAppStoreIndex == 2) // Samsung
		{
			targetAppStoreName = TargetAndroidAppStore::kSamsung.GetStringId();
		}

		Rtt_ASSERT(fKeyAliasIndex > 0 && fKeyAliasIndex < fKeyAliasesSize);
		const char* keyAlias = fKeyAliases[fKeyAliasIndex];

		AndroidAppPackagerParams androidBuilderParams(fApplicationNameInput, fVersionCodeInput, identity, provisionFile,
			fProjectPathInput, outputDir.c_str(), androidTemplate.c_str(), targetPlatform, targetAppStoreName.c_str(),
			(S32)Rtt::TargetDevice::VersionForPlatform(Rtt::TargetDevice::kAndroidPlatform), customBuildId, NULL,
			fPackageInput, isDistribution, fKeyStoreInput, fStorePassword.c_str(), keyAlias, fAliasPassword.c_str(), versionCode);

		// select build template
		app->GetPlatform()->PathForFile(kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath);
		androidBuilderParams.SetBuildSettingsPath(buildSettingsPath.GetString());

		// build the app (warning! This is blocking call)
		fBuildSuccessed = packager.Build(&androidBuilderParams, tmp.c_str());
		fBuildResult = fBuildSuccessed == 0 ? "Android build succeeded" : "Android build failed. Check the log for more details";

		// install after build
		/*if (buildResult == 0 && installAfterBuild)
		{
			const char* adbPath = "/opt/Solar2D/Android/platform-tools/adb";
			if (Rtt_FileExists(adbPath))
			{
				std::string cmd(adbPath);
				cmd.append(" install -r \"");
				cmd.append(outputDir.ToStdString().c_str());
				cmd.append("/").append(appName.ToStdString().c_str());
				cmd.append(".apk").append("\"");
				system(cmd);
			}
			else
			{
				Rtt_LogException("adb not found at the expected location of %s\n. Not copying to device as a result.\n", adbPath);
			}
		}*/
	}

	//
	// DlgHTML5Build
	//

	DlgHTML5Build::DlgHTML5Build(const std::string& title, int w, int h)
		: DlgBuild(title, w, h)
		, fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir)
		, fBuildResult(NULL)
		, fIncludeStandardResources(true)
	{
		strncpy(fApplicationNameInput, app->GetAppName(), sizeof(fApplicationNameInput));
		strncpy(fVersionInput, "1", sizeof(fVersionInput));
		strncpy(fSaveToFolderInput, app->GetContext()->GetSaveFolder().c_str(), sizeof(fSaveToFolderInput));
		strncpy(fProjectPathInput, app->GetContext()->GetAppPath(), sizeof(fProjectPathInput));

		fileDialog.SetTitle("Save To Folder");
		fileDialog.SetWindowSize(w, h);
	}

	DlgHTML5Build::~DlgHTML5Build()
	{
	}

	void DlgHTML5Build::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgHTML5Build", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();
			LinuxPlatform* platform = app->GetPlatform();

			fileDialog.Display();
			if (fileDialog.HasSelected())
			{
				strncpy(fSaveToFolderInput, fileDialog.GetSelected().string().c_str(), sizeof(fSaveToFolderInput));
				fileDialog.ClearSelected();
			}

			string s;
			ImGui::Dummy(ImVec2(10, 10));
			ImGui::PushItemWidth(350);		// input field width

			s = "   Application Name :";
			float label_width = ImGui::CalcTextSize(s.c_str()).x;
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ApplicationName", fApplicationNameInput, sizeof(fApplicationNameInput), ImGuiInputTextFlags_CharsNoBlank);

			s = "   Version Code :";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fVersionInput", fVersionInput, sizeof(fVersionInput), ImGuiInputTextFlags_CharsDecimal);

			s = "   Save To Folder: ";
			ImGui::TextUnformatted(s.c_str());
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
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				fThread = new mythread();
				fThread->start([this]() { Build(); });
			}
			ImGui::SetItemDefaultFocus();

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onCloseDialog);
			}

			if (fBuildResult)
			{
				// display result
				const char* title = "Solar2D Simulator";
				if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Dummy(ImVec2(100, 10));
					ImGui::TextUnformatted(fBuildResult);

					string s = "View";
					ImGui::Dummy(ImVec2(100, 30));
					int ok_width = 100;
					ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
					if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
					{
						OpenURL(fSaveToFolderInput);
						PushEvent(sdl::onCloseDialog);
					}

					s = "Done";
					ImGui::SameLine();
					if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
					{
						PushEvent(sdl::onCloseDialog);
					}
					ImGui::SetItemDefaultFocus();

					ImGui::EndPopup();
				}
				ImGui::OpenPopup(title);
			}
			ImGui::End();
		}

		// builder ended ?
		if (fThread)
		{
			DrawActivity();
			if (fThread->is_running() == false)
			{
				fThread = NULL;
			}
		}

		end();
	}

	// thread function
	void DlgHTML5Build::Build()
	{
		LinuxPlatform* platform = app->GetPlatform();
		MPlatformServices* service = new LinuxPlatformServices(platform);
		WebAppPackager packager(*service);
		Rtt::Runtime* runtimePointer = app->GetRuntime();
		string appName(fApplicationNameInput);
		string sourceDir(fProjectPathInput);
		string outputDir(fSaveToFolderInput);
		string appVersion(fVersionInput);
		std::string webtemplate(platform->getInstallDir());
		bool runAfterBuild = false; // runAfterBuildCheckbox->GetValue();
		const char* identity = "no-identity";
		const char* customBuildId = packager.GetCustomBuildId();
		const char* bundleId = "bundleId";
		int targetVersion = Rtt::TargetDevice::kWeb1_0;
		const TargetDevice::Platform targetPlatform(TargetDevice::Platform::kWebPlatform);
		bool isDistribution = true;
		const char kBuildSettings[] = "build.settings";
		Rtt::String buildSettingsPath;
		bool foundBuildSettings = packager.ReadBuildSettings(sourceDir.c_str());

		// setup paths
		webtemplate.append("/Resources/webtemplate.zip");

		// pre-build validation
		if (!foundBuildSettings)
		{
			fBuildResult = "build.settings file not found in project path.";
			return;
		}

		if (appName.empty())
		{
			fBuildResult = "App name cannot be empty.";
			return;
		}

		if (appVersion.empty())
		{
			fBuildResult = "App version cannot be empty.";
			return;
		}

		// ensure we have write access to the target output directory
		if (!Rtt_IsDirectory(fSaveToFolderInput) && !Rtt_MakeDirectory(fSaveToFolderInput))
		{
			fBuildResult = "Failed to create the selected output directory.";
			return;
		}

		// check if a custom build ID has been assigned
		if (!Rtt_StringIsEmpty(customBuildId))
		{
			Rtt_Log("\nUsing custom Build Id %s\n", customBuildId);
		}

		bool createFBInstantArchive = false;

		// Package build settings parameters.
		WebAppPackagerParams webBuilderParams(appName.c_str(), appVersion.c_str(), NULL, NULL,
			sourceDir.c_str(), outputDir.c_str(), NULL, targetPlatform, targetVersion,
			Rtt::TargetDevice::kWebGenericBrowser, customBuildId,
			NULL, bundleId, isDistribution, fIncludeStandardResources, NULL, createFBInstantArchive);


		// select build template
		app->GetPlatform()->PathForFile(kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath);
		webBuilderParams.SetBuildSettingsPath(buildSettingsPath.GetString());

		char tmpDirTemplate[] = "/tmp/webappXXXXXX";
		const char* tmpDirName = Rtt_MakeTempDirectory(tmpDirTemplate);
		if (tmpDirName == NULL)
		{
			fBuildResult = "MakeTempDirectory failed"; // ->SetMessage(strerror(errno));
			Rtt_LogException("%s\n", strerror(errno));
			return;
		}

		// build the app (warning! This is blocking call)
		int buildResult = packager.Build(&webBuilderParams, tmpDirName);
		fBuildResult = buildResult == 0 ? "Your application was built successfully." : "Failed to build the application.\nSee the console for more info.";

		if (buildResult == 0 && runAfterBuild)
		{
			string command("\"");
			command.append(outputDir);
			command.append("/").append(appName);
			command.append("/").append(appName);
			command.append("\"");
			system(command.c_str());
		}
	}

	//
	// DlgLinuxdBuild
	//

	DlgLinuxBuild::DlgLinuxBuild(const std::string& title, int w, int h)
		: DlgBuild(title, w, h)
		, fileDialog(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir)
		, fBuildResult(NULL)
		, fIncludeStandardResources(false)
	{
		strncpy(fApplicationNameInput, app->GetAppName(), sizeof(fApplicationNameInput));
		strncpy(fVersionInput, "1.0.0", sizeof(fVersionInput));
		strncpy(fSaveToFolderInput, app->GetContext()->GetSaveFolder().c_str(), sizeof(fSaveToFolderInput));
		strncpy(fProjectPathInput, app->GetContext()->GetAppPath(), sizeof(fProjectPathInput));

		fileDialog.SetTitle("Save To Folder");
		fileDialog.SetWindowSize(w, h);
	}

	DlgLinuxBuild::~DlgLinuxBuild()
	{
	}

	void DlgLinuxBuild::Draw()
	{
		begin();
		if (ImGui::Begin("##DlgLinuxBuild", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground))
		{
			const ImVec2& window_size = ImGui::GetWindowSize();
			LinuxPlatform* platform = app->GetPlatform();

			fileDialog.Display();
			if (fileDialog.HasSelected())
			{
				strncpy(fSaveToFolderInput, fileDialog.GetSelected().string().c_str(), sizeof(fSaveToFolderInput));
				fileDialog.ClearSelected();
			}

			string s;
			ImGui::Dummy(ImVec2(10, 10));
			ImGui::PushItemWidth(350);		// input field width

			s = "   Application Name :";
			float label_width = ImGui::CalcTextSize(s.c_str()).x;
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##ApplicationName", fApplicationNameInput, sizeof(fApplicationNameInput), ImGuiInputTextFlags_CharsNoBlank);

			s = "   Version Code :";
			ImGui::TextUnformatted(s.c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(label_width + 20);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);	// hack
			ImGui::InputText("##fVersionInput", fVersionInput, sizeof(fVersionInput), ImGuiInputTextFlags_CharsDecimal);

			s = "   Save To Folder: ";
			ImGui::TextUnformatted(s.c_str());
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
			ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				fThread = new mythread();
				fThread->start([this]() { Build(); });
			}
			ImGui::SetItemDefaultFocus();

			s = "Cancel";
			ImGui::SameLine();
			if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
			{
				PushEvent(sdl::onCloseDialog);
			}

			if (fBuildResult)
			{
				// display result
				const char* title = "Solar2D Simulator";
				if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Dummy(ImVec2(100, 10));
					ImGui::TextUnformatted(fBuildResult);

					string s = "View";
					ImGui::Dummy(ImVec2(100, 30));
					int ok_width = 100;
					ImGui::SetCursorPosX((window_size.x - ok_width) * 0.5f);
					if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
					{
						OpenURL(fSaveToFolderInput);
						PushEvent(sdl::onCloseDialog);
					}

					s = "Done";
					ImGui::SameLine();
					if (ImGui::Button(s.c_str(), ImVec2(ok_width, 0)))
					{
						PushEvent(sdl::onCloseDialog);
					}
					ImGui::SetItemDefaultFocus();

					ImGui::EndPopup();
				}
				ImGui::OpenPopup(title);
			}
			ImGui::End();
		}

		// builder ended ?
		if (fThread)
		{
			DrawActivity();
			if (fThread->is_running() == false)
			{
				fThread = NULL;
			}
		}

		end();
	}

	// thread function
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

		// setup paths
		const char* TEMPLATE_FILENAME = "linuxtemplate_x64.tgz";
		linuxtemplate.append("/Resources/");
		linuxtemplate.append(TEMPLATE_FILENAME);

		// pre-build validation
		if (!foundBuildSettings)
		{
			fBuildResult = "build.settings file not found in project path.";
			return;
		}

		if (appName.empty())
		{
			fBuildResult = "App name cannot be empty.";
			return;
		}

		if (appVersion.empty())
		{
			fBuildResult = "App version cannot be empty.";
			return;
		}

		// ensure we have write access to the target output directory
		if (!Rtt_IsDirectory(outputDir.c_str()) && !Rtt_MakeDirectory(outputDir.c_str()))
		{
			fBuildResult = "Failed to create the selected output directory.";
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
		fBuildResult = buildResult == 0 ? "Your application was built successfully." : "Failed to build the application.\nSee the console for more info.";
	}


}	// Rtt

