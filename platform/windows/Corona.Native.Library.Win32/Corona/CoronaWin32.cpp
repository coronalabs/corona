//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CoronaWin32.h"
#include "Interop\ApplicationServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_MPlatform.h"
#include "WinString.h"
#include <list>
#include <string>


#pragma region Private LaunchSettingsWrapper Struct
namespace
{
	struct CoronaLaunchSettings
	{
		/// <summary>Handle to the main window frame which contains the rendering surface.</summary>
		HWND MainWindowHandle;

		/// <summary>
		///  <para>Handle to a window/control's client area to render OpenGL content to.</para>
		///  <para>Can be null, which prevents Corona from rendering anything.</para>
		/// </summary>
		HWND RenderSurfaceHandle;

		/// <summary>
		///  <para>Handle to an invisible dummy window (just a duplicate of RenderSurfaceHandle).</para>
		///  <para>
		///   It is used only in non-simulator runs, when doing the first OpenGL capabilities test,
		///   and then immediately discarded. The test seems to leave a certain "GL-ness" on the
		///   window that other backends might not be able to use. We therefore do the test with
		///   this window and leave the original intact for the not-yet-decided backend.
		///  </para>
		/// </summary>
		HWND TestSurfaceHandle;

		/// <summary>
		///  <para>
		///   Set true to set up Corona to connect to a debugger upon launch, such as with the
		///   "Corona Debugger" or the Sublime "Corona Editor".
		///  </para>
		///  <para>Set false (the default) to not connect to a debugger.</para>
		///  <para>
		///   Debugging can only be done if the project contains Lua files. You cannot debug an app with a "resource.car".
		///  </para>
		/// </summary>
		bool IsDebuggerEnabled;

		/// <summary>
		///  Array storing paths of all sandboxed dirctories supported by Corona such as
		///  kResourceDir, kDcoumentsDir, etc.
		/// </summary>
		WinString DirectoryPaths[Rtt::MPlatform::kNumDirs];

		/// <summary>
		///  <para>
		///   Collection of strings to be delivered to the "main.lua" as "..." launch arguments via the "args" field.
		///  </para>
		///  <para>These are typically the command line arguments used to launch the application.</para>
		/// </summary>
		std::list<std::wstring> LaunchArguments;

		CoronaLaunchSettings()
		:	MainWindowHandle(nullptr),
			RenderSurfaceHandle(nullptr),
			TestSurfaceHandle(nullptr),
			IsDebuggerEnabled(false)
		{}

		void CopyTo(Interop::RuntimeEnvironment::CreationSettings& settings) const
		{
			settings.MainWindowHandle = this->MainWindowHandle;
			settings.RenderSurfaceHandle = this->RenderSurfaceHandle;
			settings.ResourceDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16();
			settings.DocumentsDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kDocumentsDir].GetUTF16();
			settings.ApplicationSupportDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kApplicationSupportDir].GetUTF16();
			settings.TemporaryDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kTmpDir].GetUTF16();
			settings.CachesDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kCachesDir].GetUTF16();
			settings.SystemCachesDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kSystemCachesDir].GetUTF16();
			settings.PluginsDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kPluginsDir].GetUTF16();
			settings.SkinResourceDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kSkinResourceDir].GetUTF16();
			settings.UserSkinsDirectoryPath = this->DirectoryPaths[Rtt::MPlatform::kUserSkinsDir].GetUTF16();
			settings.LaunchArgumentsPointer = &(this->LaunchArguments);

			int launchOptions = Rtt::Runtime::kDefaultLaunchOption;
			if (this->IsDebuggerEnabled)
			{
				launchOptions |= Rtt::Runtime::kConnectToDebugger;
			}
			settings.LaunchOptions = (Rtt::Runtime::LaunchOptions)launchOptions;
		}
	};
}

#pragma endregion


#pragma region Private CoronaRuntimeController Class
namespace
{
	class CoronaRuntimeController
	{
		public:
			CoronaRuntimeController();
			virtual ~CoronaRuntimeController();

			Interop::RuntimeEnvironment::CreationResult RunUsing(const CoronaLaunchSettings& settings);
			void Suspend();
			void Resume();
			void Terminate();

		private:
			void OnLoaded(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);
			void OnStarting(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);
			void OnStarted(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);
			void OnSuspended(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);
			void OnResumed(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);
			void OnTerminating(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments);

			Interop::RuntimeEnvironment* fEnvironmentPointer;
			Interop::RuntimeEnvironment::LoadedEvent::MethodHandler<CoronaRuntimeController> fLoadedEventHandler;
			Interop::RuntimeEnvironment::StartingEvent::MethodHandler<CoronaRuntimeController> fStartingEventHandler;
			Interop::RuntimeEnvironment::StartedEvent::MethodHandler<CoronaRuntimeController> fStartedEventHandler;
			Interop::RuntimeEnvironment::SuspendedEvent::MethodHandler<CoronaRuntimeController> fSuspendedEventHandler;
			Interop::RuntimeEnvironment::ResumedEvent::MethodHandler<CoronaRuntimeController> fResumedEventHandler;
			Interop::RuntimeEnvironment::TerminatingEvent::MethodHandler<CoronaRuntimeController> fTerminatingEventHandler;
	};

	CoronaRuntimeController::CoronaRuntimeController()
	:	fEnvironmentPointer(nullptr),
		fLoadedEventHandler(this, &CoronaRuntimeController::OnLoaded),
		fStartingEventHandler(this, &CoronaRuntimeController::OnStarting),
		fStartedEventHandler(this, &CoronaRuntimeController::OnStarted),
		fSuspendedEventHandler(this, &CoronaRuntimeController::OnSuspended),
		fResumedEventHandler(this, &CoronaRuntimeController::OnResumed),
		fTerminatingEventHandler(this, &CoronaRuntimeController::OnTerminating)
	{
	}

	CoronaRuntimeController::~CoronaRuntimeController()
	{
		Terminate();
	}

	Interop::RuntimeEnvironment::CreationResult CoronaRuntimeController::RunUsing(const CoronaLaunchSettings& settings)
	{
		// Terminate the previous Corona runtime, if running.
		Terminate();

		// Verify that the system video hardware and OpenGL driver meets Corona's min requirements.
		HWND renderSurfaceHandle = settings.RenderSurfaceHandle;

		if (settings.TestSurfaceHandle)
		{
			renderSurfaceHandle = settings.TestSurfaceHandle;
		}

		auto validationResult = Interop::RuntimeEnvironment::ValidateRenderSurface(renderSurfaceHandle);
		if (false == validationResult.CanRender)
		{
			const char kMessageFormat[] =
					"Your video hardware or driver does not meet this application's minimum requirements.\r\n"
					"A video driver supporting OpenGL %s or higher is required.\r\n"
					"\r\n"
					"Please upgrade your video driver and try again.";
			WinString message;
			message.Format(kMessageFormat, validationResult.MinVersionSupported.GetString());
			return Interop::RuntimeEnvironment::CreationResult::FailedWith(message.GetUTF16());
		}

		// Finish setting up the launch settings.
		Interop::RuntimeEnvironment::CreationSettings runtimeSettings;
		settings.CopyTo(runtimeSettings);
		runtimeSettings.LoadedEventHandlerPointer = &fLoadedEventHandler;

		// Create and run a new Corona environment.
		auto creationResult = Interop::RuntimeEnvironment::CreateUsing(runtimeSettings);
		return creationResult;
	}

	void CoronaRuntimeController::Suspend()
	{
		if (fEnvironmentPointer)
		{
			fEnvironmentPointer->RequestSuspend();
		}
	}

	void CoronaRuntimeController::Resume()
	{
		if (fEnvironmentPointer)
		{
			fEnvironmentPointer->RequestResume();
		}
	}

	void CoronaRuntimeController::Terminate()
	{
		// Do not continue if there is no runtime to terminate.
		if (!fEnvironmentPointer)
		{
			return;
		}

		// Destroy the runtime.
		Interop::RuntimeEnvironment::Destroy(fEnvironmentPointer);
		fEnvironmentPointer = nullptr;
	}

	void CoronaRuntimeController::OnLoaded(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
	{
		// Store a pointer to the newly created/loaded runtime environment.
		fEnvironmentPointer = (Interop::RuntimeEnvironment*)&sender;

		// Add event handlers to the new runtime.
		sender.GetStartingEventHandlers().Add(&fStartingEventHandler);
		sender.GetStartedEventHandlers().Add(&fStartedEventHandler);
		sender.GetSuspendedEventHandlers().Add(&fSuspendedEventHandler);
		sender.GetResumedEventHandlers().Add(&fResumedEventHandler);
		sender.GetTerminatingEventHandlers().Add(&fTerminatingEventHandler);
	}

	void CoronaRuntimeController::OnStarting(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
	{
	}

	void CoronaRuntimeController::OnStarted(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
	{
	}

	void CoronaRuntimeController::OnSuspended(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
	{
	}

	void CoronaRuntimeController::OnResumed(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
	{
	}

	void CoronaRuntimeController::OnTerminating(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
	{
	}
}

#pragma endregion


#pragma region Public Functions
CORONA_API CoronaWin32LaunchSettingsRef CoronaWin32LaunchSettingsNewRef(void)
{
	auto settings = new CoronaLaunchSettings();
	return (CoronaWin32LaunchSettingsRef)settings;
}

CORONA_API void CoronaWin32LaunchSettingsDeleteRef(CoronaWin32LaunchSettingsRef settingsReference)
{
	if (settingsReference)
	{
		delete (CoronaLaunchSettings*)settingsReference;
	}
}

CORONA_API void CoronaWin32LaunchSettingsGetMainWindowHandle(
	CoronaWin32LaunchSettingsRef settingsReference, HWND *valuePointer)
{
	// Validate arguments.
	if (!settingsReference || !valuePointer)
	{
		return;
	}

	// Fetch the requested setting.
	*valuePointer = ((CoronaLaunchSettings*)settingsReference)->MainWindowHandle;
}

CORONA_API void CoronaWin32LaunchSettingsSetMainWindowHandle(
	CoronaWin32LaunchSettingsRef settingsReference, HWND value)
{
	if (settingsReference)
	{
		((CoronaLaunchSettings*)settingsReference)->MainWindowHandle = value;
	}
}

CORONA_API void CoronaWin32LaunchSettingsGetRenderSurfaceHandle(
	CoronaWin32LaunchSettingsRef settingsReference, HWND *valuePointer)
{
	// Validate arguments.
	if (!settingsReference || !valuePointer)
	{
		return;
	}

	// Fetch the requested setting.
	*valuePointer = ((CoronaLaunchSettings*)settingsReference)->RenderSurfaceHandle;
}

CORONA_API void CoronaWin32LaunchSettingsSetRenderSurfaceHandle(
	CoronaWin32LaunchSettingsRef settingsReference, HWND value)
{
	if (settingsReference)
	{
		((CoronaLaunchSettings*)settingsReference)->RenderSurfaceHandle = value;
	}
}

CORONA_API void CoronaWin32LaunchSettingsGetTestSurfaceHandle(
	CoronaWin32LaunchSettingsRef settingsReference, HWND *valuePointer)
{
	// Validate arguments.
	if (!settingsReference || !valuePointer)
	{
		return;
	}

	// Fetch the requested setting.
	*valuePointer = ((CoronaLaunchSettings*)settingsReference)->TestSurfaceHandle;
}

CORONA_API void CoronaWin32LaunchSettingsSetTestSurfaceHandle(
	CoronaWin32LaunchSettingsRef settingsReference, HWND value)
{
	if (settingsReference)
	{
		((CoronaLaunchSettings*)settingsReference)->TestSurfaceHandle = value;
	}
}

CORONA_API void CoronaWin32LaunchSettingsGetResourceDirectory(
	CoronaWin32LaunchSettingsRef settingsReference, const wchar_t **pathPointer)
{
	// Validate arguments.
	if (!settingsReference || !pathPointer)
	{
		return;
	}

	// Fetch the requested setting.
	*pathPointer = ((CoronaLaunchSettings*)settingsReference)->DirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16();
}

CORONA_API void CoronaWin32LaunchSettingsSetResourceDirectory(
	CoronaWin32LaunchSettingsRef settingsReference, const wchar_t *path)
{
	if (settingsReference)
	{
		((CoronaLaunchSettings*)settingsReference)->DirectoryPaths[Rtt::MPlatform::kResourceDir].SetUTF16(path);
	}
}

CORONA_API void CoronaWin32LaunchSettingsAddLaunchArgument(
	CoronaWin32LaunchSettingsRef settingsReference, const wchar_t *argument)
{
	if (settingsReference)
	{
		((CoronaLaunchSettings*)settingsReference)->LaunchArguments.push_back(std::wstring(argument ? argument : L""));
	}
}

CORONA_API void CoronaWin32LaunchSettingsGetLaunchArgumentByIndex(
	CoronaWin32LaunchSettingsRef settingsReference, int index, const wchar_t **stringPointer)
{
	// Validate arguments.
	if (!settingsReference || !stringPointer || (index < 0))
	{
		return;
	}
	auto settingsPointer = (CoronaLaunchSettings*)settingsReference;
	if (index >= (int)settingsPointer->LaunchArguments.size())
	{
		return;
	}

	// Fetch the indexed argument.
	auto iterator = settingsPointer->LaunchArguments.begin();
	if (index > 0)
	{
		std::advance(iterator, index);
	}
	*stringPointer = (*iterator).c_str();
}

CORONA_API void CoronaWin32LaunchSettingsGetLaunchArgumentCount(
	CoronaWin32LaunchSettingsRef settingsReference, int *valuePointer)
{
	// Validate arguments.
	if (!settingsReference || !valuePointer)
	{
		return;
	}

	// Fetch the number of launch arguments currently stored.
	*valuePointer = (int)((CoronaLaunchSettings*)settingsReference)->LaunchArguments.size();
}

CORONA_API void CoronaWin32LaunchSettingsClearLaunchArguments(CoronaWin32LaunchSettingsRef settingsReference)
{
	if (settingsReference)
	{
		((CoronaLaunchSettings*)settingsReference)->LaunchArguments.clear();
	}
}

CORONA_API void CoronaWin32LaunchSettingsIsDebuggerEnabled(
	CoronaWin32LaunchSettingsRef settingsReference, int *valuePointer)
{
	// Validate arguments.
	if (!settingsReference || !valuePointer)
	{
		return;
	}

	// Fetch the requested setting.
	*valuePointer = ((CoronaLaunchSettings*)settingsReference)->IsDebuggerEnabled ? 1 : 0;
}

CORONA_API void CoronaWin32LaunchSettingsSetDebuggerEnabled(CoronaWin32LaunchSettingsRef settingsReference, int value)
{
	if (settingsReference)
	{
		((CoronaLaunchSettings*)settingsReference)->IsDebuggerEnabled = value ? true : false;
	}
}

CORONA_API CoronaWin32RuntimeRef CoronaWin32RuntimeNewRef(void)
{
	auto runtimeControllerPointer = new CoronaRuntimeController();
	return (CoronaWin32RuntimeRef)runtimeControllerPointer;
}

CORONA_API void CoronaWin32RuntimeDeleteRef(CoronaWin32RuntimeRef runtimeReference)
{
	if (runtimeReference)
	{
		delete (CoronaRuntimeController*)runtimeReference;
	}
}

CORONA_API int CoronaWin32RuntimeRun(
	CoronaWin32RuntimeRef runtimeReference, CoronaWin32LaunchSettingsRef settingsReference)
{
	// Validate arguments.
	if (!runtimeReference || !settingsReference)
	{
		return false;
	}

	// Run the Corona project referenced by the given launch settings.
	auto runtimeControllerPointer = (CoronaRuntimeController*)runtimeReference;
	auto launchSettingsPointer = (CoronaLaunchSettings*)settingsReference;
	auto result = runtimeControllerPointer->RunUsing(*launchSettingsPointer);

	if (launchSettingsPointer->TestSurfaceHandle)
	{
		DestroyWindow(launchSettingsPointer->TestSurfaceHandle);
	}

	// Check if we've failed to start the Corona runtime.
	if (result.HasFailed())
	{
		// Determine how the failure should be handled.
		bool shouldShowMessage = true;
		bool isError = true;
		if (result == Interop::RuntimeEnvironment::CreationResults::kSingleAppInstanceAlreadyExists)
		{
			// We attempted to create a 2nd instance for an app configured to be single instance only.
			// This type of failure is okay and can be ignored. It's not an error and shouldn't show an alert.
			// Note: The exception is the Corona Shell, which should display all failures to the developer.
			HWND windowHandle = launchSettingsPointer->MainWindowHandle;
			if (Interop::ApplicationServices::IsCoronaSdkApp() && windowHandle && ::IsWindowVisible(windowHandle))
			{
				shouldShowMessage = true;
			}
			else
			{
				shouldShowMessage = false;
			}
			isError = false;
		}

		// Fetch the failure message.
		WinString message(result.GetMessage());
		if (message.IsEmpty())
		{
			message.SetUTF16(L"An unknown error occurred while starting up the Corona runtime.");
			isError = true;
		}

		// Log the failure message.
		if (isError)
		{
			Rtt_LogException("ERROR: %s", message.GetUTF8());
		}
		else
		{
			Rtt_LogException(message.GetUTF8());
		}

		// Display the failure message to the user, if needed.
		if (shouldShowMessage)
		{
			HWND windowHandle = launchSettingsPointer->RenderSurfaceHandle;
			auto titleText = isError ? L"Error" : L"Warning";
			::MessageBoxW(windowHandle, message.GetUTF16(), titleText, MB_OK | MB_ICONEXCLAMATION);
		}
	}

	// Return non-zero if we've succeeded in running the Corona project.
	return result.HasSucceeded() ? 1 : 0;
}

CORONA_API void CoronaWin32RuntimeSuspend(CoronaWin32RuntimeRef runtimeReference)
{
	if (runtimeReference)
	{
		((CoronaRuntimeController*)runtimeReference)->Suspend();
	}
}

CORONA_API void CoronaWin32RuntimeResume(CoronaWin32RuntimeRef runtimeReference)
{
	if (runtimeReference)
	{
		((CoronaRuntimeController*)runtimeReference)->Resume();
	}
}

CORONA_API void CoronaWin32RuntimeTerminate(CoronaWin32RuntimeRef runtimeReference)
{
	if (runtimeReference)
	{
		((CoronaRuntimeController*)runtimeReference)->Terminate();
	}
}

CORONA_API lua_State* CoronaWin32RuntimeGetLuaState(CoronaWin32RuntimeRef runtimeReference)
{
	lua_State* luaStatePointer = nullptr;
	if (runtimeReference)
	{
//TODO: Finish this...
//		luaStatePointer = ((CoronaRuntimeController*)runtimeReference)->GetLuaState();
	}
	return luaStatePointer;
}

#pragma endregion
