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
	struct DlgAndroidBuild : public Dlg
	{
		DlgAndroidBuild();
		void Draw() override;

	private:

		bool ReadKeystore(std::string& keystorePath, std::string& password);
		void Build();
		void RunBuilder();

		ImGui::FileBrowser fileDialog;
		char fApplicationNameInput[32];
		char fVersionCodeInput[32];
		char fVersionNameInput[32];
		char fPackageInput[256];
		char fSaveToFolderInput[1024];
		char fProjectPathInput[1024];
		char fKeyStoreInput[1024];
		bool fCreateLiveBuild;
		smart_ptr<mythread> fThread;
		char fBuildResult[512];
		std::vector<std::string> fKeyAliases;
	};

	struct DlgLinuxBuild : public Dlg
	{
		DlgLinuxBuild();
		void Draw() override;

	private:

		void Build();
		void RunBuilder();

		ImGui::FileBrowser fileDialog;
		char fApplicationNameInput[32];
		char fVersionInput[32];
		char fSaveToFolderInput[1024];
		char fProjectPathInput[1024];
		bool fIncludeStandardResources;
		smart_ptr<mythread> fThread;
		char fBuildResult[512];
	};

	struct DlgHTML5Build : public Dlg
	{
	//	DlgHTML5Build();
		void Draw() override;
//		void Build();
	};

}
