//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_LinuxDialog.h"
#include "Rtt_LinuxUtils.h"

namespace Rtt
{
	struct DlgBuild : public Window
	{
		DlgBuild(const std::string& title, int w, int h);
		virtual ~DlgBuild();

	protected:

		void DrawResult();
		const char* fBuildResult;
		bool fBuildSuccessed;
		char fApplicationNameInput[32];
		char fSaveToFolderInput[1024];
		char fProjectPathInput[1024];
		char fVersionNameInput[32];
		bool fIncludeStandardResources;
	};

	struct DlgAndroidBuild : public DlgBuild
	{
		DlgAndroidBuild(const std::string& title, int w, int h);
		virtual ~DlgAndroidBuild();

		void Draw() override;

	private:

		bool ReadKeystore(const std::string& keystorePath, const std::string& password);
		void ReadVersion();
		void Build();
		void RunBuilder();
		void ClearKeyAliases();

		ImGui::FileBrowser fileDialogKeyStore;
		ImGui::FileBrowser fileDialogSaveTo;
		char fVersionCodeInput[32];
		char fPackageInput[256];
		char fKeyStoreInput[1024];
		bool fCreateLiveBuild;
		smart_ptr<mythread> fThread;
		char** fKeyAliases;
		int fKeyAliasesSize;
		int fAppStoreIndex;
		int fKeyAliasIndex;
		std::string fStorePassword;
		std::string fAliasPassword;
	};

	struct DlgLinuxBuild : public DlgBuild
	{
		DlgLinuxBuild(const std::string& title, int w, int h);
		virtual ~DlgLinuxBuild();

		void Draw() override;

	private:

		void Build();

		ImGui::FileBrowser fileDialog;
		smart_ptr<mythread> fThread;
	};

	struct DlgHTML5Build : public DlgBuild
	{
		DlgHTML5Build(const std::string& title, int w, int h);
		virtual ~DlgHTML5Build();

		void Draw() override;

	private:

		void Build();
		void RunBuilder();

		ImGui::FileBrowser fileDialog;
		smart_ptr<mythread> fThread;
	};

}
