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
#include <string>

#define HOMESCREEN_ID "homescreen"

class SolarApp;
class SolarGLCanvas;

namespace Rtt
{
	class LinuxPlatform;

	struct SolarAppContext
	{
		SolarAppContext(const char *path);
		~SolarAppContext();

		bool Init();
		bool IsInitialized() const { return NULL != fRuntime; }

		Runtime *GetRuntime() { return fRuntime; }
		LinuxRuntimeDelegate *GetDelegate() { return fRuntimeDelegate; }
		const Runtime *GetRuntime() const { return fRuntime; }
		LinuxMouseListener *GetMouseListener() { return fMouseListener; }
		const LinuxMouseListener *GetMouseListener() const { return fMouseListener; }
		LinuxKeyListener *GetKeyListener() { return fKeyListener; }
		const LinuxKeyListener *GetKeyListener() const { return fKeyListener; }
		void Pause();
		void Resume();
		void RestartRenderer();
		int GetFPS() const { return fRuntime ? fRuntime->GetFPS() : 30; }
		int GetWidth() const;
		void SetWidth(int val);
		int GetHeight() const;
		void SetHeight(int val);
		DeviceOrientation::Type GetOrientation() const { return fRuntimeDelegate->fOrientation; }
		const std::string &GetTitle() const { return fTitle; }
		void Flush();
		bool LoadApp(SolarGLCanvas *canvas);
		SolarGLCanvas *GetCanvas() const { return fCanvas; }
		void SetCanvas(SolarGLCanvas *val) { fCanvas = val; }
		const char *GetAppPath() const { return fPathToApp.c_str(); }
		LinuxPlatform *GetPlatform() const { return fPlatform; }
		const std::string &GetAppName() const { return fAppName; }
		const std::string &GetSaveFolder() const { return fSaveFolder; }
		const LinuxRuntimeDelegate *GetRuntimeDelegate() const { return fRuntimeDelegate; }
		static int Print(lua_State* L);		// re-defined global.print

	private:
		std::string fTitle;
		LinuxRuntime *fRuntime;
		LinuxRuntimeDelegate *fRuntimeDelegate;
		LinuxMouseListener *fMouseListener;
		LinuxKeyListener *fKeyListener;
		std::string fPathToApp;
		std::string fAppName;
		LinuxPlatform *fPlatform;
		bool fTouchDeviceExist;
		const char *fMode;
		SolarGLCanvas *fCanvas;
		bool fIsDebApp;
		LinuxSimulatorServices *fSimulator;
		std::string fSaveFolder;
		ProjectSettings* fProjectSettings;

	};
}; // namespace Rtt

#endif // Rtt_LINUX_CONTEXT_H
