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
#include <string>
#include <SDL2/SDL.h>

#define HOMESCREEN_ID "homescreen"

namespace Rtt
{
	class SolarApp;
	class LinuxPlatform;

	struct SolarAppContext : public ref_counted
	{
		SolarAppContext(SDL_Window* window, const std::string& path);
		~SolarAppContext();

		bool IsInitialized() const { return NULL != fRuntime; }

		Runtime* GetRuntime() { return fRuntime; }
		LinuxRuntimeDelegate* GetDelegate() { return fRuntimeDelegate; }
		const Runtime* GetRuntime() const { return fRuntime; }
		LinuxMouseListener* GetMouseListener() { return fMouseListener; }
		const LinuxMouseListener* GetMouseListener() const { return fMouseListener; }
		LinuxKeyListener* GetKeyListener() { return fKeyListener; }
		const LinuxKeyListener* GetKeyListener() const { return fKeyListener; }
		void Pause();
		void Resume();
		void RestartRenderer();
		int GetFPS() const { return fRuntime ? fRuntime->GetFPS() : 30; }
		int GetWidth() const;
		void SetWidth(int val);
		int GetHeight() const;
		void SetHeight(int val);
		DeviceOrientation::Type GetOrientation() const { return fRuntimeDelegate->fOrientation; }
		const std::string& GetTitle() const { return fTitle; }
		void Flush();
		bool LoadApp();
		const char* GetAppPath() const { return fPathToApp.c_str(); }
		LinuxPlatform* GetPlatform() const { return fPlatform; }
		const char* GetAppName() const { return fAppName.c_str(); }
		const std::string& GetSaveFolder() const { return fSaveFolder; }
		const LinuxRuntimeDelegate* GetRuntimeDelegate() const { return fRuntimeDelegate; }
		void advance();
		void ResetWindowSize();
		static int Print(lua_State* L);		// re-defined global.print
		void DispatchEvent(const MEvent& e)
		{
			if (fRuntime)
			{
				fRuntime->DispatchEvent(e);
			}
		}
	private:

		void Init();

		std::string fTitle;
		LinuxRuntime* fRuntime;
		LinuxRuntimeDelegate* fRuntimeDelegate;
		LinuxMouseListener* fMouseListener;
		LinuxKeyListener* fKeyListener;
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
	};
}; // namespace Rtt

#endif // Rtt_LINUX_CONTEXT_H
