//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoronaMacros.h"
//#include "CoronaLua.h"		//TODO: Figure out a good way to include this.
#include <Windows.h>


CORONA_EXTERN_C struct lua_State;
CORONA_EXTERN_C typedef void* CoronaWin32RuntimeRef;
CORONA_EXTERN_C typedef void* CoronaWin32LaunchSettingsRef;


//CORONA_EXTERN_C typedef void((*CoronaWin32RuntimeEventCallback)(CoronaWin32RuntimeRef, void*));


CORONA_API CoronaWin32LaunchSettingsRef CoronaWin32LaunchSettingsNewRef(void);
CORONA_API void CoronaWin32LaunchSettingsDeleteRef(CoronaWin32LaunchSettingsRef settingsReference);
CORONA_API void CoronaWin32LaunchSettingsGetMainWindowHandle(CoronaWin32LaunchSettingsRef settingsReference, HWND *valuePointer);
CORONA_API void CoronaWin32LaunchSettingsSetMainWindowHandle(CoronaWin32LaunchSettingsRef settingsReference, HWND value);
CORONA_API void CoronaWin32LaunchSettingsGetRenderSurfaceHandle(CoronaWin32LaunchSettingsRef settingsReference, HWND *valuePointer);
CORONA_API void CoronaWin32LaunchSettingsSetRenderSurfaceHandle(CoronaWin32LaunchSettingsRef settingsReference, HWND value);
CORONA_API void CoronaWin32LaunchSettingsGetTestSurfaceHandle(CoronaWin32LaunchSettingsRef settingsReference, HWND *valuePointer);
CORONA_API void CoronaWin32LaunchSettingsSetTestSurfaceHandle(CoronaWin32LaunchSettingsRef settingsReference, HWND value);
CORONA_API void CoronaWin32LaunchSettingsGetResourceDirectory(CoronaWin32LaunchSettingsRef settingsReference, const wchar_t **pathPointer);
CORONA_API void CoronaWin32LaunchSettingsSetResourceDirectory(CoronaWin32LaunchSettingsRef settingsReference, const wchar_t *path);
CORONA_API void CoronaWin32LaunchSettingsAddLaunchArgument(CoronaWin32LaunchSettingsRef settingsReference, const wchar_t *argument);
CORONA_API void CoronaWin32LaunchSettingsGetLaunchArgumentByIndex(CoronaWin32LaunchSettingsRef settingsReference, int index, const wchar_t **stringPointer);
CORONA_API void CoronaWin32LaunchSettingsGetLaunchArgumentCount(CoronaWin32LaunchSettingsRef settingsReference, int *valuePointer);
CORONA_API void CoronaWin32LaunchSettingsClearLaunchArguments(CoronaWin32LaunchSettingsRef settingsReference);
CORONA_API void CoronaWin32LaunchSettingsIsDebuggerEnabled(CoronaWin32LaunchSettingsRef settingsReference, int *valuePointer);
CORONA_API void CoronaWin32LaunchSettingsSetDebuggerEnabled(CoronaWin32LaunchSettingsRef settingsReference, int value);

CORONA_API CoronaWin32RuntimeRef CoronaWin32RuntimeNewRef(void);
CORONA_API void CoronaWin32RuntimeDeleteRef(CoronaWin32RuntimeRef runtimeReference);
CORONA_API int CoronaWin32RuntimeRun(CoronaWin32RuntimeRef runtimeReference, CoronaWin32LaunchSettingsRef settingsReference);
CORONA_API void CoronaWin32RuntimeSuspend(CoronaWin32RuntimeRef runtimeReference);
CORONA_API void CoronaWin32RuntimeResume(CoronaWin32RuntimeRef runtimeReference);
CORONA_API void CoronaWin32RuntimeTerminate(CoronaWin32RuntimeRef runtimeReference);
//CORONA_API void CoronaWin32RuntimeAddLoadedEventCallback(CoronaWin32RuntimeRef runtimeReference, CoronaWin32RuntimeEventCallback callback, void *contextPointer);
//CORONA_API void CoronaWin32RuntimeRemoveLoadedEventCallback(CoronaWin32RuntimeRef runtimeReference, CoronaWin32RuntimeEventCallback callback, void *contextPointer);
CORONA_API lua_State* CoronaWin32RuntimeGetLuaState(CoronaWin32RuntimeRef runtimeReference);
//CORONA_API CoronaWin32RuntimeRef CoronaWin32RuntimeFromLuaState(lua_State* luaStatePointer);


#ifdef __cplusplus

namespace Corona { namespace Win32 {

class LaunchSettings
{
	public:
		LaunchSettings()
		{
			fSettingsReference = ::CoronaWin32LaunchSettingsNewRef();
		}

		virtual ~LaunchSettings()
		{
			::CoronaWin32LaunchSettingsDeleteRef(fSettingsReference);
		}

		::CoronaWin32LaunchSettingsRef GetRef() const
		{
			return fSettingsReference;
		}

		HWND GetMainWindowHandle() const
		{
			HWND value = nullptr;
			::CoronaWin32LaunchSettingsGetMainWindowHandle(fSettingsReference, &value);
			return value;
		}

		void SetMainWindowHandle(HWND value)
		{
			::CoronaWin32LaunchSettingsSetMainWindowHandle(fSettingsReference, value);
		}

		HWND GetRenderSurfaceHandle() const
		{
			HWND value = nullptr;
			::CoronaWin32LaunchSettingsGetRenderSurfaceHandle(fSettingsReference, &value);
			return value;
		}

		void SetRenderSurfaceHandle(HWND value)
		{
			::CoronaWin32LaunchSettingsSetRenderSurfaceHandle(fSettingsReference, value);
		}

		HWND GetTestSurfaceHandle() const
		{
			HWND value = nullptr;
			::CoronaWin32LaunchSettingsGetTestSurfaceHandle(fSettingsReference, &value);
			return value;
		}

		void SetTestSurfaceHandle(HWND value)
		{
			::CoronaWin32LaunchSettingsSetTestSurfaceHandle(fSettingsReference, value);
		}

		const wchar_t* GetResourceDirectory() const
		{
			const wchar_t* pathPointer = nullptr;
			::CoronaWin32LaunchSettingsGetResourceDirectory(fSettingsReference, &pathPointer);
			return pathPointer;
		}

		void SetResourceDirectory(const wchar_t *path)
		{
			::CoronaWin32LaunchSettingsSetResourceDirectory(fSettingsReference, path);
		}

		void AddLaunchArgument(const wchar_t *argument)
		{
			::CoronaWin32LaunchSettingsAddLaunchArgument(fSettingsReference, argument);
		}

		const wchar_t* GetLaunchArgumentByIndex(int index) const
		{
			const wchar_t* stringPointer = nullptr;
			::CoronaWin32LaunchSettingsGetLaunchArgumentByIndex(fSettingsReference, index, &stringPointer);
			return stringPointer;
		}

		int GetLaunchArgumentCount() const
		{
			int value = 0;
			::CoronaWin32LaunchSettingsGetLaunchArgumentCount(fSettingsReference, &value);
			return value;
		}

		void ClearLaunchArguments()
		{
			::CoronaWin32LaunchSettingsClearLaunchArguments(fSettingsReference);
		}

		bool IsDebuggerEnabled() const
		{
			int value = 0;
			::CoronaWin32LaunchSettingsIsDebuggerEnabled(fSettingsReference, &value);
			return value ? true : false;
		}

		void SetDebuggerEnabled(bool value)
		{
			::CoronaWin32LaunchSettingsSetDebuggerEnabled(fSettingsReference, value ? 1 : 0);
		}

	private:
		LaunchSettings(const LaunchSettings &settings) {}
		void operator=(const LaunchSettings &descriptor) {}

		::CoronaWin32LaunchSettingsRef fSettingsReference;
};

class Runtime
{
	public:
		Runtime()
		{
			fRuntimeReference = ::CoronaWin32RuntimeNewRef();
		}

		virtual ~Runtime()
		{
			::CoronaWin32RuntimeDeleteRef(fRuntimeReference);
		}

		::CoronaWin32RuntimeRef GetRef() const
		{
			return fRuntimeReference;
		}

		LaunchSettings& GetLaunchSettings()
		{
			return fLaunchSettings;
		}

		lua_State* GetLuaState() const
		{
			return ::CoronaWin32RuntimeGetLuaState(fRuntimeReference);
		}

		bool Run()
		{
			return ::CoronaWin32RuntimeRun(fRuntimeReference, fLaunchSettings.GetRef()) ? true : false;
		}

		void Suspend()
		{
			::CoronaWin32RuntimeSuspend(fRuntimeReference);
		}

		void Resume()
		{
			::CoronaWin32RuntimeResume(fRuntimeReference);
		}

		void Terminate()
		{
			::CoronaWin32RuntimeTerminate(fRuntimeReference);
		}

	private:
		Runtime(const Runtime &settings) {}
		void operator=(const Runtime &descriptor) {}

		::CoronaWin32RuntimeRef fRuntimeReference;
		LaunchSettings fLaunchSettings;
};

class Environment
{
	private:
		Environment() {}

	public:
		static const wchar_t* GetDefaultResourceDirectoryPath()
		{
			return nullptr;
		}
};

} }	// namespace Corona::Win32

#endif
