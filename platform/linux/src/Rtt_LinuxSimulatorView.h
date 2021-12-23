//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Array.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_LinuxSimulatorServices.h"
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxPreferencesDialog.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_MPlatform.h"
#include "wx/config.h"

namespace Rtt
{
	class LinuxPlatformServices : public MPlatformServices
	{
	public:
		LinuxPlatformServices(MPlatform *platform);

	protected:
		bool RequestAdminAuthorization(const char *name) const;

	public:
		// MPlatformServices
		virtual const MPlatform &Platform() const override { return *fPlatform; }

		virtual void GetPreference(const char *key, String *value) const override
		{
			Preference::ReadValueResult val = fPlatform->GetPreference(key, "");
			const char *str = val.GetValue().ToString().GetValue()->c_str();
			value->Set(str);
		}

		// Pass NULL for value to remove pref
		virtual void SetPreference(const char *key, const char *value) const override
		{
			//fPlatform->SetPreference(key, value);
			int k = 1;
		}

		virtual void GetSecurePreference(const char *key, String *value) const override { return GetPreference(key, value); }
		virtual bool SetSecurePreference(const char *key, const char *value) const
		{
			SetPreference(key, value);
			return true;
		}

		virtual void GetLibraryPreference(const char *key, String *value) const override { return GetPreference(key, value); }
		virtual void SetLibraryPreference(const char *key, const char *value) const override { SetPreference(key, value); }
		virtual bool IsInternetAvailable() const override { return false; };
		virtual bool IsLocalWifiAvailable() const override { return false; };
		virtual void Terminate() const override {}
		virtual void Sleep(int milliseconds) const override {};

	private:
		MPlatform *fPlatform;
	};

	class LinuxSimulatorView
	{
	public:
		LinuxSimulatorView();
		virtual ~LinuxSimulatorView();

		class Config
		{
		public:
			static void Load();
			static void Save();
			static void Cleanup();

		public:
			static wxString settingsFilePath;
			static wxString lastProjectDirectory;
			static bool showRuntimeErrors;
			static bool openLastProject;
			static LinuxPreferencesDialog::RelaunchType relaunchOnFileChange;
			static int windowXPos;
			static int windowYPos;
			static int skinID;
			static int skinWidth;
			static int skinHeight;
			static int zoomedWidth;
			static int zoomedHeight;
			static int welcomeScreenZoomedWidth;
			static int welcomeScreenZoomedHeight;
			static wxConfig *configFile;
		};

		struct SkinProperties
		{
			wxString device;
			int screenOriginX;
			int screenOriginY;
			int screenWidth;
			int screenHeight;
			int androidDisplayApproximateDpi;
			wxString displayManufacturer;
			wxString displayName;
			bool isUprightOrientationPortrait;
			bool supportsScreenRotation;
			wxString windowTitleBarName;
			wxString skinTitle;
			int id;
			bool selected;
		};

		static bool LoadSkin(lua_State *L, int skinID, std::string filePath);
		static LinuxSimulatorView::SkinProperties GetSkinProperties(int skinID);
		static LinuxSimulatorView::SkinProperties GetSkinProperties(wxString skinTitle);
		static void DeselectSkins();
		static void SelectSkin(int skinID);
		static bool IsRunningOnSimulator();
		static void OnBuildForWeb(SolarAppContext *ctx);
		static void OnWebBuild(wxCommandEvent &);
		static void OnLinuxPluginGet(const char *appPath, const char *appName, LinuxPlatform *platform);
		static void OnCancel(wxCommandEvent &);
		static void AppWebBuild(SolarAppContext *ctx);
		static void GetRecentDocs(LightPtrArray<RecentProjectInfo>* listPointer);

	public:
		static std::map<int, LinuxSimulatorView::SkinProperties> fSkins;
		static const float skinScaleFactor;
		static const int skinMinWidth;

	public:
		struct webBuildParams : public wxObject
		{
			webBuildParams(wxDialog *dlg, SolarAppContext *ctx, wxCheckBox *useStandardResources, wxCheckBox *runAfterBuild, wxCheckBox *createFBInstance)
				: fDlg(dlg), fCtx(ctx), fUseStandardResources(useStandardResources), fRunAfterBuild(runAfterBuild), fCreateFBInstance(createFBInstance) {};

			wxDialog *fDlg;
			SolarAppContext *fCtx;
			wxCheckBox *fUseStandardResources;
			wxCheckBox *fRunAfterBuild;
			wxCheckBox *fCreateFBInstance;
			std::string fErrMsg;
		};

		struct cancelBuild : public wxObject
		{
			cancelBuild(wxDialog *dlg) : fDlg(dlg) {};
			wxDialog *fDlg;
		};
	};
}; // namespace Rtt
