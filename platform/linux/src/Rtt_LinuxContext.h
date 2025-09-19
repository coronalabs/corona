//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_LINUX_CONTEXT_H
#define Rtt_LINUX_CONTEXT_H

#include "Rtt_Event.h"
#include "Core/Rtt_Types.h"
#include "Rtt_Runtime.h"
#include "Core/Rtt_Math.h"
#include "Core/Rtt_Array.h"
#include "shared/Rtt_ProjectSettings.h"
#include "shared/Rtt_NativeWindowMode.h"
#include "Rtt_LinuxInputDeviceManager.h"
#include "Rtt_LinuxSimulatorServices.h"
#include "Rtt_LinuxRuntime.h"
#include "Rtt_LinuxRuntimeDelegate.h"
#include "Rtt_LinuxKeyListener.h"
#include "Rtt_LinuxMouseListener.h"
#include "Rtt_LinuxContainer.h"
#include "Rtt_LinuxUtils.h"
#include <string>
#include <SDL2/SDL.h>

#define HOMESCREEN_ID "homescreen"

namespace Rtt
{
	class SolarApp;
	class LinuxPlatform;

	struct Config
	{
		Config();
		Config(const std::string& path, bool crypted = false);
		~Config();

		as_value& operator[](const std::string& name);
		const as_value& operator[](const std::string& name) const;
		bool HasItem(const std::string& name) const
		{
			const auto& it = fConfig.find(name);
			return it != fConfig.end();
		}

		void Load(const std::string& path, bool crypted = false);
		void Save();

	private:

		std::map<std::string, as_value> fConfig;
		std::string fPath;
		bool fIsCrypted;
	};

	struct SolarAppContext : public ref_counted
	{
		SolarAppContext(SDL_Window* window);
		virtual ~SolarAppContext();

		bool IsInitialized() const { return NULL != fRuntime; }

		Runtime* GetRuntime() { return fRuntime; }
		LinuxRuntimeDelegate* GetDelegate() { return fRuntimeDelegate; }
		const Runtime* GetRuntime() const { return fRuntime; }
		void Pause();
		void Resume();
		void RestartRenderer();
		int GetFPS() const { return fRuntime ? fRuntime->GetFPS() : 30; }
		int GetWidth() const;
		void SetWidth(int val);
		int GetHeight() const;
		void SetHeight(int val);
		DeviceOrientation::Type GetOrientation() const { return fRuntimeDelegate->fOrientation; }
		void SetOrientation(DeviceOrientation::Type t) { fRuntimeDelegate->fOrientation = t; }
		void Flush();
		bool LoadApp(const std::string& appPath);
		const std::string& GetAppPath() const { return fPathToApp; }
		LinuxPlatform* GetPlatform() const { return fPlatform; }
		const std::string& GetAppName() const { return fAppName; }
		const std::string& GetSaveFolder() const { return fSaveFolder; }
		const LinuxRuntimeDelegate* GetRuntimeDelegate() const { return fRuntimeDelegate; }
		void advance();
		static int Print(lua_State* L);		// re-defined global.print
		void DispatchEvent(const MEvent& e)
		{
			if (fRuntime)
			{
				fRuntime->DispatchEvent(e);
			}
		}

		void SetSize(int w, int h);

		std::string GetTitle() const;
		void SetTitle(const std::string& title);

	private:

		void Init();

		LinuxRuntime* fRuntime;
		LinuxRuntimeDelegate* fRuntimeDelegate;
		std::string fPathToApp;
		std::string fAppName;
		LinuxPlatform* fPlatform;
		bool fTouchDeviceExist;
		const char* fMode;
		bool fIsDebApp;
		LinuxSimulatorServices* fLinuxSimulatorServices;
		std::string fSaveFolder;
		ProjectSettings* fProjectSettings;

		SDL_Window* fWindow;
		Config fConfig;
		bool fBeginRunLoop;
	};
}; // namespace Rtt

#endif // Rtt_LINUX_CONTEXT_H
