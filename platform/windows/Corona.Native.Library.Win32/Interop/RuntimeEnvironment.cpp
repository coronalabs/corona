//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// Enables the Crypto++ library's MD4 and MD5 support without compiler warnings.
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "RuntimeEnvironment.h"
#include "Core\Rtt_Build.h"
#include "Core\Rtt_Allocator.h"
#include "Core\Rtt_String.h"
#include "CryptoPP\md5.h"
#include "Display\Rtt_Display.h"
#include "Display\Rtt_Scene.h"
#include "Display\Rtt_StageObject.h"
#include "Interop\Storage\RegistryStoredPreferences.h"
#include "Interop\Storage\SQLiteStoredPreferences.h"
#include "Interop\UI\RenderSurfaceControl.h"
#include "Interop\UI\Window.h"
#include "ApplicationServices.h"
#include "MDeviceSimulatorServices.h"
#include "Rtt_CKWorkflow.h"
#include "Rtt_DependencyLoader.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_NativeWindowMode.h"
#include "Rtt_Preference.h"
#include "Rtt_Runtime.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_WinTimer.h"
#include <algorithm>
#include <exception>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <sstream>
#include <WinInet.h>


namespace Interop {

#pragma region Public Constants
const RuntimeEnvironment::CreationResult RuntimeEnvironment::CreationResults::kSingleAppInstanceAlreadyExists
(
	RuntimeEnvironment::CreationResult::FailedWith(L"Another instance of this application is already running.")
);

#pragma endregion


#pragma region Internal Members
/// <summary>
///  <para>The semaphore name used by Corona to flag that a runtime is currently connected to a debugger.</para>
///  <para>Expected to be passed into the Win32 CreateSemaphoreW() function.</para>
/// </summary>
/// <remarks>
///  Corona debugging uses a fixed TCP port number which only allows one Corona app on a system to be debugged
///  at a time. So, a named semaphore is used to detect if another Corona app is currently connected to the
///  debugger. If the named semaphore is not found on the system, then TCP debugging is available.
/// </remarks>
static const wchar_t kDebuggerSemaphoreName[] = L"Global\\CoronaLabs.Corona.Runtime.ConnectedToDebugger";

#pragma endregion


#pragma region Constructors/Destructors
RuntimeEnvironment::RuntimeEnvironment(const RuntimeEnvironment::CreationSettings& settings)
:	fAllocatorPointer(Rtt_AllocatorCreate()),
	fPlatformPointer(nullptr),
	fRuntimePointer(nullptr),
	fRuntimeState(RuntimeState::kNotStarted),
	fRuntimeDelegate(this),
	fRuntimeTimerElapsedCallback(this, &RuntimeEnvironment::OnRuntimeTimerElapsed),
	fMainWindowReceivedMessageEventHandler(this, &RuntimeEnvironment::OnMainWindowReceivedMessage),
	fIpcWindowReceivedMessageEventHandler(this, &RuntimeEnvironment::OnIpcWindowReceivedMessage),
	fRenderFrameEventHandler(this, &RuntimeEnvironment::OnRenderFrame),
	fDestroyingSurfaceEventHandler(this, &RuntimeEnvironment::OnDestroyingSurface),
	fSurfaceResizedEventHandler(this, &RuntimeEnvironment::OnSurfaceResized),
	fWasSuspendRequestedExternally(false),
	fProjectSettings(),
	fReadOnlyProjectSettings(fProjectSettings),
	fMainMessageOnlyWindowPointer(nullptr),
	fIpcMessageOnlyWindowPointer(nullptr),
	fMainWindowPointer(nullptr),
	fRenderSurfacePointer(nullptr),
	fLastOrientation(Rtt::DeviceOrientation::kUnknown),
	fGdiPlusToken(0),
	fFontServices(*this),
	fDebuggerSemaphoreHandle(nullptr),
	fLastActivatedSent(true),
	fSingleWindowInstanceSemaphoreHandle(nullptr)
{
	// Do not continue if the given render surface handle (if provided) is already being used by another runtime.
	if (settings.IsRuntimeCreationEnabled && settings.RenderSurfaceHandle)
	{
		auto componentCollection = UI::UIComponent::FetchExistingBy(settings.RenderSurfaceHandle);
		for (int index = componentCollection.GetCount() - 1; index >= 0; index--)
		{
			auto componentPointer = componentCollection.GetByIndex(index);
			if (componentPointer)
			{
				if (dynamic_cast<UI::RenderSurfaceControl*>(componentPointer) != nullptr)
				{
					throw std::runtime_error(
								"There is another Corona runtime already referencing the given window handle.");
				}
			}
		}
	}

	// Copy the given directory paths to this object.
	// Note: The kSystemResourceDirectory and kResourceDirectory must both point to the same directory for desktop apps.
	//       This is because the Rtt::Runtime loads the "resource.car" from kSystemResourceDirectory.
	fDirectoryPaths[Rtt::MPlatform::kResourceDir].SetUTF16(settings.ResourceDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kProjectResourceDir].SetUTF16(settings.ResourceDirectoryPath);
#ifdef Rtt_AUTHORING_SIMULATOR
	fDirectoryPaths[Rtt::MPlatform::kSystemResourceDir].SetUTF16(settings.SystemResourceDirectoryPath);
#else
	fDirectoryPaths[Rtt::MPlatform::kSystemResourceDir].SetUTF16(settings.ResourceDirectoryPath);
#endif
	fDirectoryPaths[Rtt::MPlatform::kDocumentsDir].SetUTF16(settings.DocumentsDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kApplicationSupportDir].SetUTF16(settings.ApplicationSupportDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kTmpDir].SetUTF16(settings.TemporaryDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kCachesDir].SetUTF16(settings.CachesDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kSystemCachesDir].SetUTF16(settings.SystemCachesDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kPluginsDir].SetUTF16(settings.PluginsDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kSkinResourceDir].SetUTF16(settings.SkinResourceDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kUserSkinsDir].SetUTF16(settings.UserSkinsDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kApplicationSupportDir].SetUTF16(settings.DocumentsDirectoryPath);
	for (int index = 0; index < Rtt::MPlatform::kNumDirs; index++)
	{
		// Make sure the directory path string is never null. (Only applicable to virtual directories.)
		if (fDirectoryPaths[index].GetBuffer() == nullptr)
		{
			fDirectoryPaths[index].SetUTF16(L"");
		}

		// Remove any trailing slashes or spaces from the end of the path.
		fDirectoryPaths[index].TrimEnd(L"\\/ ");

		// Create the directory if it doesn't already exist.
		// Note: We don't need to do this with virtual directories.
		if (Rtt::MPlatform::IsVirtualDirectory((Rtt::MPlatform::Directory)index) == false)
		{
			int resultCode = ::SHCreateDirectoryExW(nullptr, fDirectoryPaths[index].GetUTF16(), nullptr);
			if ((resultCode != ERROR_SUCCESS) && (resultCode != ERROR_FILE_EXISTS) && (resultCode != ERROR_ALREADY_EXISTS))
			{
				// Failed to create/access the given directory path. Throw an exception explaining why.
				LPWSTR utf16Buffer;
				::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						nullptr, resultCode,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&utf16Buffer, 0, nullptr);
				WinString stringConverter;
				stringConverter.SetUTF16(utf16Buffer);
				std::string utf8Message = stringConverter.GetUTF8();
				::LocalFree(utf16Buffer);
				throw std::runtime_error(utf8Message);
			}
		}
	}

	// Load the project's "build.settings" and "config.lua" file.
	// Used to fetch supported orientations, window size, content size, preference storage type, etc.
	if (settings.IsRuntimeCreationEnabled)
	{
		fProjectSettings.LoadFromDirectory(fDirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF8());
	}

	// Set up a registry path for this project.
	{
		std::wstringstream stringStream;
		stringStream << L"Software";
		if (ApplicationServices::IsCoronaSdkApp())
		{
			// Simulated apps are sandboxed under the "Corona Simulator" registry tree.
			stringStream << L'\\';
			stringStream << Storage::RegistryStoredPreferences::kAnscaCoronaKeyName;
			stringStream << L'\\';
			stringStream << Storage::RegistryStoredPreferences::kCoronaSimulatorKeyName;
			stringStream << L'\\';
			{
				WinString escapedDirectoryPath(fDirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16());
				escapedDirectoryPath.MakeLowerCase();
				escapedDirectoryPath.Replace("/", "\\");
				escapedDirectoryPath.Replace("\\\\", "");
				stringStream << escapedDirectoryPath.GetUTF16();
			}
			stringStream << L'\\';
			stringStream << Storage::RegistryStoredPreferences::kSimulatorPreferencesKeyName;
		}
		else
		{
			// This is a Win32 app template. Registry path defaults to:  <CompanyName>\<AppName>
			auto companyName = ApplicationServices::GetCompanyName();
			if (companyName && (companyName[0] != L'\0'))
			{
				std::wstring escapedString(companyName);
				std::replace(escapedString.begin(), escapedString.end(), L'\\', L' ');
				std::replace(escapedString.begin(), escapedString.end(), L'/', L' ');
				stringStream << L'\\';
				stringStream << escapedString;
			}
			auto productName = ApplicationServices::GetProductName();
			if (productName && (productName[0] != L'\0'))
			{
				std::wstring escapedString(productName);
				std::replace(escapedString.begin(), escapedString.end(), L'\\', L' ');
				std::replace(escapedString.begin(), escapedString.end(), L'/', L' ');
				stringStream << L'\\';
				stringStream << escapedString;
			}
			else
			{
				auto exeFileNameWithoutExtension = ApplicationServices::GetExeFileNameWithoutExtension();
				if (exeFileNameWithoutExtension && (exeFileNameWithoutExtension[0] != L'\0'))
				{
					stringStream << L'\\';
					stringStream << exeFileNameWithoutExtension;
				}
				else
				{
					stringStream << L"\\Corona App";
				}
			}
		}
		fRegistryPathWithoutHive = stringStream.str();
	}

	// Create an Apple-style preferences manager used to easily read/write key-value pairs to file.
	switch (fProjectSettings.GetWin32PreferenceStorageType())
	{
		case Rtt::WinProjectSettings::PreferenceStorageType::kRegistry:
		{
			// Preferences are stored to the Windows registry.
			std::wstring baseRegistryPath = L"HKCU\\" + fRegistryPathWithoutHive;
			Interop::Storage::RegistryStoredPreferences::CreationSettings creationSettings{};
			creationSettings.BaseRegistryPath = baseRegistryPath.c_str();
			creationSettings.IsUsingForwardSlashAsPathSeparator = true;
			creationSettings.Wow64ViewType = Interop::Storage::RegistryStoredPreferences::Wow64ViewType::kDefault;
			fStoredPreferencesPointer = std::make_shared<Interop::Storage::RegistryStoredPreferences>(creationSettings);
			break;
		}
		case Rtt::WinProjectSettings::PreferenceStorageType::kSQLite:
		default:
		{
			// Preferences are stored to a SQLite database file under Corona's undocumented system caches directory.
			std::wstring filePath = fDirectoryPaths[Rtt::MPlatform::kSystemCachesDir].GetUTF16();
			filePath += L"\\CoronaPreferences.sqlite";
			fStoredPreferencesPointer = std::make_shared<Interop::Storage::SQLiteStoredPreferences>(filePath.c_str());
			break;
		}
	}

	// Copy the array of launch arguments strings, if any.
	// These are to be delivered to the "main.lua" file via its "..." launch arguments table under the "args" field.
	if (settings.LaunchArgumentsPointer)
	{
		for (auto&& nextArgument : *settings.LaunchArgumentsPointer)
		{
			fLaunchArguments.push_back(WinString(nextArgument.c_str()));
		}
	}

	// Fetch a global message-only window for this thread.
	// Used for private communications within the Corona runtime environment(s).
	// Note: We need a global instance that will outlive the main app window in case we need to display modal
	//       windows with it, which are blocking calls. If the main window hosting this Corona runtime gets
	//       destroyed via this message-only window, it won't cause a crash because it'll outlive the main window.
	fMainMessageOnlyWindowPointer = Interop::UI::MessageOnlyWindow::GetSharedInstanceForCurrentThread();
	if (!fMainMessageOnlyWindowPointer)
	{
		throw std::runtime_error("Failed to create message-only window.");
	}

	// If given a control to render to, wrap it with our own render surface object.
	// This allows us to receive all Windows messages delivered to this control and tightly control rendering.
	if (settings.RenderSurfaceHandle)
	{
		// Set up the control to clip its child controls, if not done already.
		// This prevents the rendered content from being painted on top of all of the child controls.
		auto windowStyles = ::GetWindowLongW(settings.RenderSurfaceHandle, GWL_STYLE);
		if ((windowStyles & WS_CLIPCHILDREN) == 0)
		{
			::SetWindowLongW(settings.RenderSurfaceHandle, GWL_STYLE, windowStyles | WS_CLIPCHILDREN);
		}

		// Wrap the given control with our rendering surface adapter and set up event handlers.
		fRenderSurfacePointer = new Interop::UI::RenderSurfaceControl(settings.RenderSurfaceHandle);
		fRenderSurfacePointer->SetRenderFrameHandler(&fRenderFrameEventHandler);
		fRenderSurfacePointer->GetDestroyingEventHandlers().Add(&fDestroyingSurfaceEventHandler);
		fRenderSurfacePointer->GetResizedEventHandlers().Add(&fSurfaceResizedEventHandler);
	}

	// If given access to the window hosting the render surface, wrap it with our UI object class.
	// This gives us access to its Windows messages, control its size, position, etc.
	if (settings.MainWindowHandle)
	{
		// Wrap the window.
		fMainWindowPointer = new Interop::UI::Window(settings.MainWindowHandle);
		fMainWindowPointer->GetReceivedMessageEventHandlers().Add(&fMainWindowReceivedMessageEventHandler);
	}

	// Create the platform.
	fPlatformPointer = new Rtt::WinPlatform(*this);

	// Add the caller's "Loaded" event handler, if provided.
	if (settings.IsRuntimeCreationEnabled)
	{
		fLoadedEvent.GetHandlerManager().Add(settings.LoadedEventHandlerPointer);
	}
}

RuntimeEnvironment::~RuntimeEnvironment()
{
	// Terminate the Corona runtime first.
	Terminate();

	// Destroy the platform.
	delete fPlatformPointer;
	fPlatformPointer = nullptr;

	// Destroy the allocator.
	Rtt_AllocatorDestroy(fAllocatorPointer);
	fAllocatorPointer = nullptr;
}

#pragma endregion


#pragma region Public Methods
RuntimeEnvironment::LoadedEvent::HandlerManager& RuntimeEnvironment::GetLoadedEventHandlers()
{
	return fLoadedEvent.GetHandlerManager();
}

RuntimeEnvironment::StartingEvent::HandlerManager& RuntimeEnvironment::GetStartingEventHandlers()
{
	return fStartingEvent.GetHandlerManager();
}

RuntimeEnvironment::StartedEvent::HandlerManager& RuntimeEnvironment::GetStartedEventHandlers()
{
	return fStartedEvent.GetHandlerManager();
}

RuntimeEnvironment::SuspendedEvent::HandlerManager& RuntimeEnvironment::GetSuspendedEventHandlers()
{
	return fSuspendedEvent.GetHandlerManager();
}

RuntimeEnvironment::ResumedEvent::HandlerManager& RuntimeEnvironment::GetResumedEventHandlers()
{
	return fResumedEvent.GetHandlerManager();
}

RuntimeEnvironment::EnteringFrameEvent::HandlerManager& RuntimeEnvironment::GetEnteringFrameEventHandlers()
{
	return fEnteringFrameEvent.GetHandlerManager();
}

RuntimeEnvironment::EnteredFrameEvent::HandlerManager& RuntimeEnvironment::GetEnteredFrameEventHandlers()
{
	return fEnteredFrameEvent.GetHandlerManager();
}

RuntimeEnvironment::TerminatingEvent::HandlerManager& RuntimeEnvironment::GetTerminatingEventHandlers()
{
	return fTerminatingEvent.GetHandlerManager();
}

Rtt_Allocator& RuntimeEnvironment::GetAllocator() const
{
	return *fAllocatorPointer;
}

Rtt::WinPlatform* RuntimeEnvironment::GetPlatform() const
{
	return fPlatformPointer;
}

Rtt::Runtime* RuntimeEnvironment::GetRuntime() const
{
	return fRuntimePointer;
}

RuntimeState RuntimeEnvironment::GetRuntimeState() const
{
	return fRuntimeState;
}

UI::MessageOnlyWindow& RuntimeEnvironment::GetMessageOnlyWindow()
{
	return *fMainMessageOnlyWindowPointer;
}

UI::Window* RuntimeEnvironment::GetMainWindow() const
{
	return fMainWindowPointer;
}

Interop::UI::RenderSurfaceControl* RuntimeEnvironment::GetRenderSurface() const
{
	return fRenderSurfacePointer;
}

const char* RuntimeEnvironment::GetUtf8PathFor(Rtt::MPlatform::Directory value) const
{
	// Validate argument.
	if ((value < 0) || (value >= Rtt::MPlatform::kNumDirs))
	{
		return nullptr;
	}

	// Fetch the given directory's path.
	return fDirectoryPaths[value].GetUTF8();
}

const wchar_t* RuntimeEnvironment::GetUtf16PathFor(Rtt::MPlatform::Directory value) const
{
	// Validate argument.
	if ((value < 0) || (value >= Rtt::MPlatform::kNumDirs))
	{
		return nullptr;
	}

	// Fetch the given directory's path.
	return fDirectoryPaths[value].GetUTF16();
}

const wchar_t* RuntimeEnvironment::GetRegistryPathWithoutHive() const
{
	if (fRegistryPathWithoutHive.empty())
	{
		Rtt_ASSERT(0);
		return L"";
	}
	return fRegistryPathWithoutHive.c_str();
}

void RuntimeEnvironment::SetPathForProjectResourceDirectory(const wchar_t* path)
{
	// Copy the given path and remove any trailing slashes or spaces from the end of the path.
	WinString newPath(path);
	newPath.TrimEnd(L"\\/ ");

	// Update the Corona runtime's project resource directory path.
	WinString& destinationString = fDirectoryPaths[Rtt::MPlatform::kProjectResourceDir];
	if (newPath.IsEmpty())
	{
		// We were given a null/empty string. Reset the path to the resource directory.
		destinationString.SetUTF16(fDirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16());
	}
	else
	{
		// Use the given directory path.
		destinationString.SetUTF16(path);
	}
}

std::shared_ptr<Interop::Storage::MStoredPreferences> RuntimeEnvironment::GetStoredPreferences() const
{
	return fStoredPreferencesPointer;
}

MDeviceSimulatorServices* RuntimeEnvironment::GetDeviceSimulatorServices() const
{
	return nullptr;
}

Graphics::CoronaFontServices& RuntimeEnvironment::GetFontServices()
{
	return fFontServices;
}

const Rtt::ReadOnlyProjectSettings& RuntimeEnvironment::GetProjectSettings() const
{
	return fReadOnlyProjectSettings;
}

lua_CFunction RuntimeEnvironment::GetLuaPackageLoaderCallback() const
{
	return &OnLuaLoadPackage;
}

void RuntimeEnvironment::RequestSuspend()
{
	fWasSuspendRequestedExternally = true;
	Suspend();
}

void RuntimeEnvironment::RequestResume()
{
	fWasSuspendRequestedExternally = false;
	Resume();
}

void RuntimeEnvironment::Terminate()
{
	// Do not continue if we're in the middle of terminating the runtime.
	if (fRuntimePointer)
	{
		if ((RuntimeState::kTerminating == fRuntimeState) || (RuntimeState::kTerminated == fRuntimeState))
		{
			return;
		}
	}

	// Clear the suspend request state.
	fWasSuspendRequestedExternally = false;

	// Destroy the Corona runtime.
	if (fRuntimePointer)
	{
		fRuntimeState = RuntimeState::kTerminating;
		Rtt_DELETE(fRuntimePointer);
		fRuntimePointer = nullptr;
		fRuntimeState = RuntimeState::kTerminated;
	}

	// Destroy the main window wrapper.
	if (fMainWindowPointer)
	{
		delete fMainWindowPointer;
		fMainWindowPointer = nullptr;
	}

	// Destroy the render surface wrapper.
	if (fRenderSurfacePointer)
	{
		delete fRenderSurfacePointer;
		fRenderSurfacePointer = nullptr;
	}

	// Destroy the IPC message-only window.
	if (fIpcMessageOnlyWindowPointer)
	{
		delete fIpcMessageOnlyWindowPointer;
		fIpcMessageOnlyWindowPointer = nullptr;
	}

	// Shutdown Microsoft GDI+.
	if (fGdiPlusToken)
	{
		Gdiplus::GdiplusShutdown(fGdiPlusToken);
		fGdiPlusToken = 0;
	}

	// If this runtime was connected to a debugger, then destroy its named semaphore.
	// This makes the debugger available to other Corona apps on the system.
	if (fDebuggerSemaphoreHandle)
	{
		::CloseHandle(fDebuggerSemaphoreHandle);
		fDebuggerSemaphoreHandle = nullptr;
	}

	// If this runtime was set up to only support 1 application window at a time,
	// then release its semaphore so that other another window can be made in the future.
	if (fSingleWindowInstanceSemaphoreHandle)
	{
		::CloseHandle(fSingleWindowInstanceSemaphoreHandle);
		fSingleWindowInstanceSemaphoreHandle = nullptr;
	}
}

#pragma endregion


#pragma region Public Static Functions
RuntimeEnvironment::CreationResult RuntimeEnvironment::CreateUsing(const RuntimeEnvironment::CreationSettings& settings)
{
	/// <summary>Returns the given string if not null. Returns an empty string if given null.</summary>
	/// <param name="text">The string to be returned if not null.</param>
#	define ReturnEmptyWStringIfNull(text) text ? text : L""

	/// <summary>Replaces the invalid file system characters in the given filename with valid characters.</summary>
	/// <param name="filename">The UTF-16 filename to be escaped. Cannot be null.</param>
#	define EscapeFileNameW(filename) \
	{ \
		std::replace(##filename.begin(), ##filename.end(), L'\\', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L'/', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L':', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L'*', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L'?', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L'\"', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L'<', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L'>', L'_'); \
		std::replace(##filename.begin(), ##filename.end(), L'|', L'_'); \
	}

	// Fetch the Lua "system.ResourceDirectory" path equivalent.
	std::wstring resourceDirectoryPath(ReturnEmptyWStringIfNull(settings.ResourceDirectoryPath));
	auto index = resourceDirectoryPath.find_last_not_of(L"\\/");
	if (index == std::wstring::npos)
	{
		resourceDirectoryPath.clear();
	}
	else if ((index + 1) < resourceDirectoryPath.length())
	{
		resourceDirectoryPath.erase(index + 1);
	}
	if (resourceDirectoryPath.empty())
	{
		// A resource directory path was not provided.
		// Attempt to assign a default path if a "resource.car" or "main.lua" was found in the following directories:
		// 1) The application's root directory. (ie: Where the *.exe is located.)
		// 2) The application's "Resources" subdirectory, which is the default for Corona Simulator builds.
		resourceDirectoryPath = ApplicationServices::GetDirectoryPath();
		std::wstring filePath(resourceDirectoryPath);
		filePath.append(L"\\resource.car");
		if (::PathFileExistsW(filePath.c_str()) == false)
		{
			filePath = resourceDirectoryPath;
			filePath.append(L"\\main.lua");
			if (::PathFileExistsW(filePath.c_str()) == false)
			{
				// Main Corona project file not found at root level. Check under "Resources" subdirectory next.
				resourceDirectoryPath.append(L"\\Resources");
				filePath = resourceDirectoryPath;
				filePath.append(L"\\resource.car");
				if (::PathFileExistsW(filePath.c_str()) == false)
				{
					filePath = resourceDirectoryPath;
					filePath.append(L"\\main.lua");
					if (::PathFileExistsW(filePath.c_str()) == false)
					{
						// Failed to find "resource.car" or "main.lua".
						// Default to the root app directory and move on.
						resourceDirectoryPath = ApplicationServices::GetDirectoryPath();
					}
				}
			}
		}
	}

#ifdef Rtt_AUTHORING_SIMULATOR
	// Fetch the Lua "system.SystemResourceDirectory" path equivalent.
	std::wstring systemResourceDirectoryPath(ReturnEmptyWStringIfNull(settings.SystemResourceDirectoryPath));
	if (systemResourceDirectoryPath.empty())
	{
		systemResourceDirectoryPath = ApplicationServices::GetDirectoryPath();
		systemResourceDirectoryPath += L"\\Resources";
	}
#endif

	// Fetch a path to Corona's plugins directory.
	std::wstring pluginsDirectoryPath(ReturnEmptyWStringIfNull(settings.PluginsDirectoryPath));
	if (pluginsDirectoryPath.empty())
	{
		if (ApplicationServices::IsCoronaSdkApp())
		{
			CopySimulatorPluginDirectoryPathTo(pluginsDirectoryPath);
		}
		if (pluginsDirectoryPath.empty())
		{
			pluginsDirectoryPath = ApplicationServices::GetDirectoryPath();
		}
	}

	// Fetch a path to theskins directory.
	// Note: These are really only relevant to the Corona Simulator, but we must default them to something for now.
	std::wstring skinResourceDirectoryPath(ReturnEmptyWStringIfNull(settings.SkinResourceDirectoryPath));
	if (skinResourceDirectoryPath.empty())
	{
		skinResourceDirectoryPath = resourceDirectoryPath;
	}
	std::wstring userSkinsDirectoryPath(ReturnEmptyWStringIfNull(settings.UserSkinsDirectoryPath));
	if (userSkinsDirectoryPath.empty())
	{
		userSkinsDirectoryPath = resourceDirectoryPath;
	}

	// Generate the root sandboxed directory paths.
	std::wstring rootSandboxDocumentsDirectoryPath;
	std::wstring rootSandboxTempDirectoryPath;
	if (ApplicationServices::IsCoronaSdkApp())
	{
		// This is a Corona Labs made app, such as the Corona Shell.
		// Generate a hashed subdirectory like how the Corona Simulator does it so that each project gets its own sandbox.
		RuntimeEnvironment::GenerateSimulatorSandboxPath(resourceDirectoryPath.c_str(), rootSandboxDocumentsDirectoryPath);
		rootSandboxTempDirectoryPath = rootSandboxDocumentsDirectoryPath;
	}
	else
	{
		// Attempt to generate a path like this:  <Company Name>\<App Name>
		std::wstring partialPath;
		auto companyName = ApplicationServices::GetCompanyName();
		if (companyName && (companyName[0] != L'\0'))
		{
			std::wstring escapedString(companyName);
			EscapeFileNameW(escapedString);
			partialPath = escapedString;
			partialPath.append(L"\\");
		}
		auto productName = ApplicationServices::GetProductName();
		if (productName && (productName[0] != L'\0'))
		{
			std::wstring escapedString(productName);
			EscapeFileNameW(escapedString);
			partialPath.append(escapedString);
		}
		else
		{
			auto exeFileNameWithoutExtension = ApplicationServices::GetExeFileNameWithoutExtension();
			if (exeFileNameWithoutExtension && (exeFileNameWithoutExtension[0] != L'\0'))
			{
				partialPath.append(exeFileNameWithoutExtension);
			}
			else
			{
				partialPath.append(L"Corona App");
			}
		}

		// Set up the documents directory to be under the hidden app data folder.
		// Note: This folder's files can be synchronized with other machines on the same domain.
		wchar_t utf16Buffer[1024];
		utf16Buffer[0] = L'\0';
		::SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, utf16Buffer);
		rootSandboxDocumentsDirectoryPath = utf16Buffer;
		rootSandboxDocumentsDirectoryPath.append(L"\\");
		rootSandboxDocumentsDirectoryPath.append(partialPath);

		// Set up the temp and cached files directories to be under the hidden "local" app data folder.
		// These files are not synchronized between machines on a domain.
		utf16Buffer[0] = L'\0';
		::SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, utf16Buffer);
		rootSandboxTempDirectoryPath = utf16Buffer;
		rootSandboxTempDirectoryPath.append(L"\\");
		rootSandboxTempDirectoryPath.append(partialPath);
	}

	// Fetch the Lua "system.DocumentsDirectory" path equivalent.
	std::wstring documentsDirectoryPath(ReturnEmptyWStringIfNull(settings.DocumentsDirectoryPath));
	if (documentsDirectoryPath.empty())
	{
		documentsDirectoryPath = rootSandboxDocumentsDirectoryPath;
		documentsDirectoryPath.append(L"\\Documents");
	}

	// Fetch the Lua "system.ApplicationSupportDirectory" path equivalent.
	std::wstring applicationSupportDirectoryPath(ReturnEmptyWStringIfNull(settings.ApplicationSupportDirectoryPath));
	if (applicationSupportDirectoryPath.empty())
	{
		applicationSupportDirectoryPath = rootSandboxDocumentsDirectoryPath;
		applicationSupportDirectoryPath.append(L"\\ApplicationSupport");
	}

	// Fetch the Lua "system.TemporaryDirectory" path equivalent.
	std::wstring temporaryDirectoryPath(ReturnEmptyWStringIfNull(settings.TemporaryDirectoryPath));
	if (temporaryDirectoryPath.empty())
	{
		temporaryDirectoryPath = rootSandboxTempDirectoryPath;
		temporaryDirectoryPath.append(L"\\TemporaryFiles");
	}

	// Fetch the Lua "system.CachesDirectory" path equivalent.
	std::wstring cachesDirectoryPath(ReturnEmptyWStringIfNull(settings.CachesDirectoryPath));
	if (cachesDirectoryPath.empty())
	{
		cachesDirectoryPath = rootSandboxTempDirectoryPath;
		cachesDirectoryPath.append(L"\\CachedFiles");
	}

	// Fetch the Lua "system.SystemCachesDirectory" path equivalent.
	std::wstring systemCachesDirectoryPath(ReturnEmptyWStringIfNull(settings.SystemCachesDirectoryPath));
	if (systemCachesDirectoryPath.empty())
	{
		systemCachesDirectoryPath = rootSandboxTempDirectoryPath;
		systemCachesDirectoryPath.append(L"\\.system");
	}

	// Create a copy of the given settings and update it with the default settings above.
	CreationSettings updatedSettings(settings);
	updatedSettings.ResourceDirectoryPath = resourceDirectoryPath.c_str();
	updatedSettings.DocumentsDirectoryPath = documentsDirectoryPath.c_str();
	updatedSettings.ApplicationSupportDirectoryPath = applicationSupportDirectoryPath.c_str();
	updatedSettings.TemporaryDirectoryPath = temporaryDirectoryPath.c_str();
	updatedSettings.CachesDirectoryPath = cachesDirectoryPath.c_str();
	updatedSettings.SystemCachesDirectoryPath = systemCachesDirectoryPath.c_str();
#ifdef Rtt_AUTHORING_SIMULATOR
	updatedSettings.SystemResourceDirectoryPath = systemResourceDirectoryPath.c_str();
#endif
	updatedSettings.PluginsDirectoryPath = pluginsDirectoryPath.c_str();
	updatedSettings.SkinResourceDirectoryPath = skinResourceDirectoryPath.c_str();
	updatedSettings.UserSkinsDirectoryPath = userSkinsDirectoryPath.c_str();

	// Attempt to create the Corona runtime environment.
	RuntimeEnvironment* environmentPointer = nullptr;
	try
	{
		environmentPointer = new RuntimeEnvironment(updatedSettings);
	}
	catch (const std::exception& ex)
	{
		return CreationResult::FailedWith(ex.what());
	}
	catch (...) {}
	if (!environmentPointer)
	{
		// The constructor threw an exception.
		return CreationResult::FailedWith(L"Failed to create the Corona runtime environment.");
	}

	// Do the following if the Corona project is set up to be a single instance application.
	// Note: This can only happen for Win32 desktop apps and project ran via the Corona Shell.
	if (settings.IsRuntimeCreationEnabled &&
	    (environmentPointer->GetDeviceSimulatorServices() == nullptr) &&
	    environmentPointer->fMainWindowPointer &&
	    environmentPointer->fProjectSettings.IsWin32SingleInstanceWindowEnabled())
	{
		// Attempt to create a name semaphore on the system for this Corona runtime.
		// If the semaphore alread exists, then we know another instance of this app already exists on the desktop.
		bool doesInstanceAlreadyExist = false;
		{
			std::wstring semaphoreName;
			GenerateSingleWindowInstanceSemaphoreName(resourceDirectoryPath.c_str(), semaphoreName);
			if (!semaphoreName.empty())
			{
				// Attempt to claim a named semaphore for this Corona runtime project.
				auto semaphoreHandle = ::CreateSemaphoreW(nullptr, 0, 1, semaphoreName.c_str());
				const auto errorCode = ::GetLastError();
				if (errorCode == ERROR_ALREADY_EXISTS)
				{
					// An instance already exists.
					if (semaphoreHandle)
					{
						::CloseHandle(semaphoreHandle);
					}
					doesInstanceAlreadyExist = true;
				}
				else if (semaphoreHandle)
				{
					// This is the first instance.
					environmentPointer->fSingleWindowInstanceSemaphoreHandle = semaphoreHandle;
				}
				else
				{
					// We've failed to create semaphore.
					Rtt_LogException("Failed to create named semaphore for single instance app.");
				}
			}
		}

		// If an existing app instance was found, then do the following and exit out with a failure result.
		if (doesInstanceAlreadyExist)
		{
			// First, communicate with the single instance application via IPC (Inter-Process Communications).
			std::wstring className;
			GenerateIpcMessageOnlyWindowClassName(resourceDirectoryPath.c_str(), className);
			if (!className.empty())
			{
				// Attempt to find the single instance's message-only window by its registered class name.
				HWND windowHandle = nullptr;
				while (true)
				{
					// Fetch the next message-only window by its unique class name.
					// Note: Typically, only one will be found. More than one will only be found if two versions
					//       of this app is running where one is setup to be single instance and the other is not.
					windowHandle = ::FindWindowExW(HWND_MESSAGE, windowHandle, className.c_str(), nullptr);
					if (!windowHandle)
					{
						break;
					}

					// Determine if the other instance is the single instance app.
					COPYDATASTRUCT copyData{};
					copyData.dwData = 1;
					copyData.lpData = (LPVOID)L"corona:isSingleInstance";
					copyData.cbData = wcslen((wchar_t*)copyData.lpData) * sizeof(wchar_t);
					auto isSingleInstance = ::SendMessageW(windowHandle, WM_COPYDATA, 0, (LPARAM)&copyData);
					if (isSingleInstance)
					{
						break;
					}
				}

				// Dispatch an "applicationOpen" event to the single instance app's message-only window, if found above.
				if (windowHandle)
				{
					// Create the "applicationOpen" message to be sent to the other app via the WM_COPYDATA.
					std::wstringstream stringStream;
					stringStream << L"corona:applicationOpen\n";

					// Append this instance's working directory to the message.
					// If we were given any relative paths at the command line, then they're relative to this directory.
					{
						DWORD characterCount = ::GetCurrentDirectoryW(0, nullptr);
						if (characterCount > 0)
						{
							auto workingDirectoryPath = new wchar_t[characterCount];
							if (workingDirectoryPath)
							{
								workingDirectoryPath[0] = L'\0';
								::GetCurrentDirectoryW(characterCount, workingDirectoryPath);
								if (workingDirectoryPath[0] != L'\0')
								{
									stringStream << L"/commandLineDir\n";
									stringStream << workingDirectoryPath;
									stringStream << L'\n';
								}
								delete[] workingDirectoryPath;
							}
						}
					}

					// Append any command line arguments given to this app instance to the message.
					if (environmentPointer->fLaunchArguments.size() > 0)
					{
						stringStream << L"/commandLineArgs\n";
						for (auto&& nextArgument : environmentPointer->fLaunchArguments)
						{
							if (!nextArgument.IsEmpty())
							{
								stringStream << nextArgument.GetUTF16();
							}
							stringStream << L'\n';
						}
					}
					std::wstring message = stringStream.str();

					// Send the "applicationOpen" command to the single instance app.
					COPYDATASTRUCT copyData{};
					copyData.dwData = 1;
					copyData.lpData = (LPVOID)message.c_str();
					copyData.cbData = message.length() * sizeof(wchar_t);
					::SendMessageW(windowHandle, WM_COPYDATA, 0, (LPARAM)&copyData);
				}
			}

			// Delete the RuntimeEnvironment instance created by this function.
			delete environmentPointer;

			// Return a failure result indicating that an instance already exists.
			// The caller can do an equality check with this result constant to identify this particular failure.
			return CreationResults::kSingleAppInstanceAlreadyExists;
		}
	}

	// Load and run a Corona project, if enabled.
	if (settings.IsRuntimeCreationEnabled)
	{
		auto result = OperationResult::FailedWith((wchar_t*)nullptr);
		try
		{
			result = environmentPointer->RunUsing(settings);
		}
		catch (const std::exception& ex)
		{
			result = OperationResult::FailedWith(ex.what());
		}
		catch (...) {}
		if (result.HasFailed())
		{
			delete environmentPointer;
			return CreationResult::FailedWith(result.GetMessage());
		}
	}

	// Return the newly created Corona runtime environment.
	return CreationResult::SucceededWith(environmentPointer);
}

void RuntimeEnvironment::Destroy(RuntimeEnvironment* environmentPointer)
{
	if (environmentPointer)
	{
		delete environmentPointer;
	}
}

RuntimeEnvironment::ValidateRenderSurfaceResult RuntimeEnvironment::ValidateRenderSurface(HWND windowHandle)
{
	// Initialize the result object with Corona's minimum OpenGL version requirements.
	ValidateRenderSurfaceResult result;
	result.MinVersionSupported.SetString("2.1");
	result.MinVersionSupported.SetMajorNumber(2);
	result.MinVersionSupported.SetMinorNumber(1);

	// Do not continue if given a null handle.
	if (!windowHandle)
	{
		return result;
	}

	// Fetch the version of the OpenGL driver to be used on the given window handle.
	// For best performance, first check if the given window handle is already referenced by Corona.
	UI::RenderSurfaceControl* surfaceControlPointer = nullptr;
	auto componentCollection = UI::UIComponent::FetchExistingBy(windowHandle);
	for (int index = componentCollection.GetCount() - 1; index >= 0; index--)
	{
		auto componentPointer = componentCollection.GetByIndex(index);
		if (componentPointer)
		{
			surfaceControlPointer = dynamic_cast<UI::RenderSurfaceControl*>(componentPointer);
			if (surfaceControlPointer)
			{
				break;
			}
		}
	}
	if (surfaceControlPointer)
	{
		// Corona currently has the given window handle wrapped by a UIComponent derived object.
		// Fetch the version of the rendering driver that's currently being used by Corona's surface.
		result.RendererVersion = surfaceControlPointer->GetRendererVersion();
	}
	else
	{
		// A Corona runtime has not been given a reference to the given window handle yet.
		// Create a temporary RenderSurfaceControl wrapper to obtain renderer information from it.
		surfaceControlPointer = new UI::RenderSurfaceControl(windowHandle);
		result.RendererVersion = surfaceControlPointer->GetRendererVersion();
		delete surfaceControlPointer;
		surfaceControlPointer = nullptr;
	}

	// Check if the rendering driver meets Corona's minimum requirements.
	if (result.RendererVersion.CompareTo(result.MinVersionSupported) >= 0)
	{
		// The rendering driver meets Corona's minimum requirements.
		result.CanRender = true;
		result.SupportsAllShaders = true;
	}
	else if (result.RendererVersion.CompareTo(UI::RenderSurfaceControl::Version(2, 0)) >= 0)
	{
		// The rendering driver is good enough to render Corona basic features, but premium graphics
		// features such as some shader effects will fail to render.
		result.CanRender = true;
		result.SupportsAllShaders = false;
	}
	else
	{
		// The rendering driver is too old. (Result object is already initialized as failed.)
	}

	// Return the final validation results.
	return result;
}

bool RuntimeEnvironment::IsDebuggerConnectionAvailable()
{
	bool isAvailable = true;
	{
		auto handle = ::CreateSemaphoreW(nullptr, 0, 1, kDebuggerSemaphoreName);
		auto errorCode = ::GetLastError();
		if (ERROR_ALREADY_EXISTS == errorCode)
		{
			isAvailable = false;
		}
		if (handle)
		{
			::CloseHandle(handle);
		}
	}
	return isAvailable;
}

#pragma endregion


#pragma region Protected Methods
OperationResult RuntimeEnvironment::RunUsing(const RuntimeEnvironment::CreationSettings& settings)
{
	// Do not continue if the launch settings flagged that a Corona runtime should not be created.
	if (false == settings.IsRuntimeCreationEnabled)
	{
		throw std::exception();
	}

	// Do not continue if a Corona runtime has already been created.
	// Only one runtime is allowed per RuntimeEnvironment instance.
	if (fRuntimePointer)
	{
		throw std::exception();
	}

	// Determine if the given resource directory has a "main.lua" or "resource.car" file in it.
	// Note: The Corona Simulator does not support loading a "resource.car" file.
	//       This is a limitation in the Rtt::Runtime::LoadApplication() method.
	bool hasMainLuaFile = false;
	std::wstring utf16MainFilePath;
#ifndef Rtt_AUTHORING_SIMULATOR
	utf16MainFilePath = fDirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16();
	utf16MainFilePath += L"\\resource.car";
	if (::PathFileExistsW(utf16MainFilePath.c_str()))
	{
		// A "resource.car" file was found.
		hasMainLuaFile = false;
	}
	else
#endif
	{
		utf16MainFilePath = fDirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16();
		utf16MainFilePath += L"\\main.lua";
		if (::PathFileExistsW(utf16MainFilePath.c_str()))
		{
			// A "main.lua" file was found.
			hasMainLuaFile = true;
		}
		else
		{
			// Neither files were found. Return an error.
			std::wstring utf16Message =
					L"Failed to start Corona project.\r\n"
					L"Could not find a \"main.lua\" or \"resource.car\" file in directory:\r\n\r\n";
			utf16Message += fDirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16();
			return OperationResult::FailedWith(utf16Message.c_str());
		}
	}

	// Resize and reconfigure the window that will be hosting the Corona runtime, if given access.
	// Note: This must be done before the Corona runtime loads the "config.lua" file because some Corona developers
	//       like to dynamically set the content width/height based on the window's pixel width/height on startup.
	if (fProjectSettings.HasBuildSettings() || fProjectSettings.HasConfigLua())
	{
		UpdateMainWindowUsing(fReadOnlyProjectSettings);
	}
	
	// If not simulating a device, then choose a default orientation that best fits the given surface's bounds.
	// Note: This respects the "build.settings" supported orientations. If this file was not found or if it
	//       does not provide any supported orientations, then a default orientation will not be selected.
	auto deviceSimulatorServicesPointer = GetDeviceSimulatorServices();
	if (!deviceSimulatorServicesPointer && fRenderSurfacePointer)
	{
		auto surfaceBounds = fRenderSurfacePointer->GetClientBounds();
		auto surfaceWidth = surfaceBounds.right - surfaceBounds.left;
		auto surfaceHeight = surfaceBounds.bottom - surfaceBounds.top;
		if ((surfaceWidth > surfaceHeight) && fProjectSettings.IsLandscapeSupported())
		{
			if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kSidewaysRight))
			{
				fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kSidewaysRight);
			}
			else
			{
				fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kSidewaysLeft);
			}
		}
		else if ((surfaceWidth <= surfaceHeight) && fProjectSettings.IsPortraitSupported())
		{
			if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kUpright))
			{
				fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kUpright);
			}
			else
			{
				fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kUpsideDown);
			}
		}
	}

	// If the "build.settings" file does not have a default orientation set or if the file was not found,
	// then assign a default orientation for this Corona project to use.
	// Note: If a "build.settings" was supplied, then the content width/height are relative to a portrait orientation.
	//       If a "build.settings" was not found, then the content width/height are considered upright and used as-is.
	if (Rtt::DeviceOrientation::IsInterfaceOrientation(fProjectSettings.GetDefaultOrientation()) == false)
	{
		if (fProjectSettings.GetSupportedOrientationsCount() > 0)
		{
			// The "build.settings" file definitely exists and contains a "supported" orientations list.
			// Use an orientation flagged as supported within the "build.settings" file.
			if (GetDeviceSimulatorServices() == nullptr)
			{
				// Favor a landscape orientation for Windows desktop apps, if supported.
				if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kSidewaysRight))
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kSidewaysRight);
				}
				else if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kSidewaysLeft))
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kSidewaysLeft);
				}
				else
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kUpright);
				}
			}
			else
			{
				// Favor a portrait orientation for Corona Simulator device simulation, if supported.
				if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kUpright))
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kUpright);
				}
				else if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kUpsideDown))
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kUpsideDown);
				}
				else if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kSidewaysRight))
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kSidewaysRight);
				}
				else if (fProjectSettings.IsSupported(Rtt::DeviceOrientation::kSidewaysLeft))
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kSidewaysLeft);
				}
				else
				{
					fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kUpright);
				}
			}
		}
		else
		{
			// No "build.settings" file was found or no supported orientations was configured.
			// We have no choice but to hard code it to an upright orientation.
			fProjectSettings.SetDefaultOrientation(Rtt::DeviceOrientation::kUpright);
		}
	}

	// If running under the Corona Simulator, make sure its orientation matches the runtime's launch orientation.
	if (deviceSimulatorServicesPointer)
	{
		deviceSimulatorServicesPointer->SetOrientation(fProjectSettings.GetDefaultOrientation());
	}

	// Do the following if the Corona runtime was requested to connect to a debugger.
	// Note: We cannot debug a Corona project using a "resource.car" file. We can only debug via Lua files.
	if (hasMainLuaFile && (settings.LaunchOptions & Rtt::Runtime::kConnectToDebugger))
	{
		while (true)
		{
			// Attempt to create a system-wide named semaphore which flags that a Corona runtime is using the debugger.
			auto semaphoreHandle = ::CreateSemaphoreW(nullptr, 0, 1, kDebuggerSemaphoreName);
			auto errorCode = ::GetLastError();

			// If we've failed to create a named semaphore, then we cannot debug reliably on the system.
			if (!semaphoreHandle)
			{
				// Ask if the user wants to continue running the Corona project without debugging.
				WinString message(L"Failed to set up Corona with the debugger.\r\n");
				if (errorCode)
				{
					LPWSTR utf16ErrorMessage = nullptr;
					::FormatMessageW(
							FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							nullptr, errorCode,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPWSTR)&utf16ErrorMessage, 0, nullptr);
					if (utf16ErrorMessage)
					{
						message.Append(L"Reason:\r\n\t");
						message.Append(utf16ErrorMessage);
						message.Append(L"\r\n");
						::LocalFree(utf16ErrorMessage);
					}
				}
				message.Append(
						L"\r\n"
						L"Would you like to run your Corona project without debug mode?\r\n"
						L"(You will still be able to receive log output.)");
				HWND windowHandle = settings.MainWindowHandle ? settings.MainWindowHandle : settings.RenderSurfaceHandle;
				auto dialogResult = ::MessageBoxW(windowHandle, message.GetUTF16(), L"Warning", MB_ICONWARNING | MB_YESNO);
				if (IDYES == dialogResult)
				{
					// The user want to run the Corona project without debugging.
					break;
				}
				else
				{
					// The user is aborting. Do not create/launch a Corona runtime and exit out.
					return OperationResult::FailedWith(L"The user has canceled out of debug mode.");
				}
			}

			// If the named semaphore already exists, then another Corona app is already connected to the debugger.
			// The Corona debugger uses a fixed TCP port number, meaning only 1 app can be debugged on the system at a time.
			if (ERROR_ALREADY_EXISTS == errorCode)
			{
				// Destroy this app's reference to the semaphore created up above.
				if (semaphoreHandle)
				{
					::CloseHandle(semaphoreHandle);
				}

				// Ask the user to close the other Corona app that is currently connected to the debugger.
				const wchar_t utf16Message[] =
						L"There is another Corona application currently connected to a debugger.\r\n"
						L"Please close that application and then click the \"Retry\" button to try again.";
				HWND windowHandle = settings.MainWindowHandle ? settings.MainWindowHandle : settings.RenderSurfaceHandle;
				auto dialogResult = ::MessageBoxW(windowHandle, utf16Message, L"Warning", MB_ICONWARNING | MB_RETRYCANCEL);
				if (IDRETRY == dialogResult)
				{
					// The user clicked the Retry button, which hopefully means the user closed the other
					// Corona app that was in debug mode. Attempt to set this app for the debugger again.
					continue;
				}
				else
				{
					// The user is aborting. Do not create/launch a Corona runtime and exit out.
					return OperationResult::FailedWith(L"The user has canceled out of debug mode.");
				}
			}

			// We have exclusive access to the Corona debugger and are ready to start debugging.
			// Store the semaphore handle used to flag that this app is using the debugger.
			// We'll later destroy this semaphore upon termination, letting the system know the debugger is available.
			fDebuggerSemaphoreHandle = semaphoreHandle;
			break;
		}
	}

	// Copy the given Corona runtime launch settings, modifying it if needed.
	auto updatedLaunchOptions = settings.LaunchOptions;
	if (!fDebuggerSemaphoreHandle)
	{
		// This runtime environment is not set up to connect to a Corona debugger.
		// Remove the debugger launch option flag in case it is set.
		int value = (int)updatedLaunchOptions & ~(int)Rtt::Runtime::kConnectToDebugger;
		updatedLaunchOptions = (Rtt::Runtime::LaunchOptions)value;
	}

	// Create a message-only window to be used for IPC (Inter-Process Communications).
	// It must be created when the runtime is created and destroyed when the runtime is terminated.
	// Note: This is mostly intended for single instance app support where a 2nd app instance would send its
	//       command line arguments to the 1st instance's IPC message-only window via a WM_COPYDATA message.
	if (settings.IsRuntimeCreationEnabled)
	{
		// Create a unique class name for the message-only window.
		// Other app instances will find this window by this registered class name.
		std::wstring className;
		GenerateIpcMessageOnlyWindowClassName(fDirectoryPaths[Rtt::MPlatform::kResourceDir].GetUTF16(), className);

		// Create the message-only window and add an event handler to it to receive its IPC messages.
		fIpcMessageOnlyWindowPointer = new UI::MessageOnlyWindow(className.c_str());
		fIpcMessageOnlyWindowPointer->GetReceivedMessageEventHandlers().Add(&fIpcWindowReceivedMessageEventHandler);

		// Set up this process to receive WM_COPYDATA messages from other application processes.
		// Note: The ChangeWindowMessageFilter() call is only needed on Windows Vista and newer OS versions.
		HMODULE moduleHandle = ::LoadLibraryW(L"user32");
		if (moduleHandle)
		{
			typedef BOOL(WINAPI *ChangeWindowMessageFilterCallback)(UINT, DWORD);
			auto callback = (ChangeWindowMessageFilterCallback)::GetProcAddress(moduleHandle, "ChangeWindowMessageFilter");
			if (callback)
			{
				callback(WM_COPYDATA, MSGFLT_ADD);
			}
			::FreeLibrary(moduleHandle);
		}
	}

	// Create the Corona runtime.
	fRuntimePointer = Rtt_NEW(fAllocatorPointer, Rtt::Runtime(*fPlatformPointer, &fRuntimeTimerElapsedCallback));
	fRuntimePointer->SetDelegate(&fRuntimeDelegate);
	fRuntimePointer->SetProperty(Rtt::Runtime::kDeferUpdate, true);
	fRuntimePointer->SetProperty(Rtt::Runtime::kIsApplicationNotArchived, hasMainLuaFile);
	fRuntimePointer->SetProperty(Rtt::Runtime::kIsCustomEffectsAvailable, true);
	fRuntimePointer->SetProperty(Rtt::Runtime::kIsLuaParserAvailable, true);
	fRuntimePointer->SetProperty(Rtt::Runtime::kRenderAsync, true);
	fRuntimePointer->SetProperty(Rtt::Runtime::kShouldVerifyLicense, true);

	// Load and run the Corona project.
	fRuntimeState = RuntimeState::kStarting;
	fLastOrientation = fProjectSettings.GetDefaultOrientation();
	auto result = fRuntimePointer->LoadApplication(updatedLaunchOptions, fLastOrientation);
	if (Rtt::Runtime::kSuccess == result)
	{
		// Load was successful. Start running the Corona application.
		fRuntimePointer->BeginRunLoop();
		OnRuntimeTimerElapsed();

		// Force Corona to render immediately instead of waiting for the next Windows paint message.
		if (fRenderSurfacePointer && fRenderSurfacePointer->GetWindowHandle())
		{
			::UpdateWindow(fRenderSurfacePointer->GetWindowHandle());
		}
	}
	else if (Rtt::Runtime::kSecurityIssue == result)
	{
		// A CoronaCards licensing error occurred.
		// Leave the runtime in the "Starting" state because it'll run the "loader_callback.lua" script
		// in this case which displays a native alert on how to correct the licensing issue.
		return OperationResult::FailedWith(L"Licensing error.");
	}
	else
	{
		// Failed to load the Corona "resource.car" or "main.lua" file.
		Terminate();
		return OperationResult::FailedWith(L"Failed to load the Corona project.");
	}
	return OperationResult::kSucceeded;
}

bool RuntimeEnvironment::GenerateSingleWindowInstanceSemaphoreName(
	const wchar_t* resourceDirectoryPath, std::wstring& destinationString)
{
	// Create a unique semaphore name used to enforce single app instance support.
	std::wstringstream stringStream;
	if (ApplicationServices::IsCoronaSdkApp())
	{
		// Generate a name based on the simulated project's hashed sandbox directory.
		wchar_t* sandboxDirectoryName = nullptr;
		std::wstring sandboxDirectoryPath;
		GenerateSimulatorSandboxPath(resourceDirectoryPath, sandboxDirectoryPath);
		if (!sandboxDirectoryPath.empty())
		{
			sandboxDirectoryName = ::PathFindFileNameW(sandboxDirectoryPath.c_str());
		}
		if (!sandboxDirectoryName || (L'\0' == sandboxDirectoryName[0]))
		{
			sandboxDirectoryName = L"Corona App";
		}
		stringStream << sandboxDirectoryName;
	}
	else
	{
		// Generate a name for the Win32 desktop app based on the company name and product name assigned to the EXE.
		auto companyName = ApplicationServices::GetCompanyName();
		if (companyName && (companyName[0] != L'\0'))
		{
			stringStream << companyName;
			stringStream << L':';
		}
		auto productName = ApplicationServices::GetProductName();
		if (productName && (productName[0] != L'\0'))
		{
			stringStream << productName;
		}
		else
		{
			auto exeFileNameWithoutExtension = ApplicationServices::GetExeFileNameWithoutExtension();
			if (exeFileNameWithoutExtension && (exeFileNameWithoutExtension[0] != L'\0'))
			{
				stringStream << exeFileNameWithoutExtension;
			}
			else
			{
				stringStream << L"Corona App";
			}
		}
	}
	stringStream << L":Corona.SingleInstance";
	destinationString = stringStream.str();

	// Semaphore names are not allowed to contain backslashes past the "Global\" or "Local\" prefix.
	std::replace(destinationString.begin(), destinationString.end(), L'\\', L'/');

	// Prefix the semaphore name generated above to indicate that it belongs to the global namespace on Windows.
	destinationString.insert(0, L"Global\\");

	// Truncate the semaphore name if it exceeds the max characters allowed by the system.
	if (destinationString.length() > MAX_PATH)
	{
		destinationString.erase(MAX_PATH);
	}
	return true;
}

bool RuntimeEnvironment::GenerateIpcMessageOnlyWindowClassName(
	const wchar_t* resourceDirectoryPath, std::wstring& destinationString)
{
	// Create a unique class name for IPS message-only window.
	std::wstringstream stringStream;
	if (ApplicationServices::IsCoronaSdkApp())
	{
		// Generate a name based on the simulated project's hashed sandbox directory.
		wchar_t* sandboxDirectoryName = nullptr;
		std::wstring sandboxDirectoryPath;
		GenerateSimulatorSandboxPath(resourceDirectoryPath, sandboxDirectoryPath);
		if (!sandboxDirectoryPath.empty())
		{
			sandboxDirectoryName = ::PathFindFileNameW(sandboxDirectoryPath.c_str());
		}
		if (!sandboxDirectoryName || (L'\0' == sandboxDirectoryName[0]))
		{
			sandboxDirectoryName = L"Corona App";
		}
		stringStream << sandboxDirectoryName;
	}
	else
	{
		// Generate a name for the Win32 desktop app based on the company name and product name assigned to the EXE.
		auto companyName = ApplicationServices::GetCompanyName();
		if (companyName && (companyName[0] != L'\0'))
		{
			stringStream << companyName;
			stringStream << L'\\';
		}
		auto productName = ApplicationServices::GetProductName();
		if (productName && (productName[0] != L'\0'))
		{
			stringStream << productName;
		}
		else
		{
			auto exeFileNameWithoutExtension = ApplicationServices::GetExeFileNameWithoutExtension();
			if (exeFileNameWithoutExtension && (exeFileNameWithoutExtension[0] != L'\0'))
			{
				stringStream << exeFileNameWithoutExtension;
			}
			else
			{
				stringStream << L"Corona App";
			}
		}
	}

	// Always append the following so that we can identify it as a Corona registered class.
	stringStream << L"\\Corona.Ipc.MessageOnlyWindow";

	// Return the generated class name.
	destinationString = stringStream.str();
	return true;
}

bool RuntimeEnvironment::GenerateSimulatorSandboxPath(
	const wchar_t* resourceDirectoryPath, std::wstring& destinationString)
{
	// Validate.
	if (!resourceDirectoryPath || (L'\0' == resourceDirectoryPath[0]))
	{
		return false;
	}

	// Remove the trailing slash from the given resource directory path.
	std::wstring resourceDirectoryPathWithoutSlash(resourceDirectoryPath);
	auto index = resourceDirectoryPathWithoutSlash.find_last_not_of(L"\\/");
	if (index == std::wstring::npos)
	{
		resourceDirectoryPathWithoutSlash.clear();
	}
	else if ((index + 1) < resourceDirectoryPathWithoutSlash.length())
	{
		resourceDirectoryPathWithoutSlash.erase(index + 1);
	}
	if (resourceDirectoryPathWithoutSlash.empty())
	{
		return false;
	}

	// Fetch the Corona project folder's name from the given resource directory path.
	std::wstring projectFolderName;
	index = resourceDirectoryPathWithoutSlash.find_last_of(L"\\/");
	if ((index != std::wstring::npos) && ((index + 1) < resourceDirectoryPathWithoutSlash.length()))
	{
		projectFolderName = (const wchar_t*)(resourceDirectoryPathWithoutSlash.c_str() + index + 1);
	}
	if (projectFolderName.empty())
	{
		projectFolderName = resourceDirectoryPathWithoutSlash;
	}
	std::transform(projectFolderName.begin(), projectFolderName.end(), projectFolderName.begin(), ::towlower);

	// Create a sandbox directory path for the given Corona project folder.
	std::wstring sandboxDirectoryPath;
	{
		// Fetch a path to the user's local app data directory.
		// The files put into this directory are not synchronized between machines on a domain.
		wchar_t utf16Buffer[1024];
		utf16Buffer[0] = L'\0';
		::SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, utf16Buffer);
		if (L'\0' == utf16Buffer[0])
		{
			return false;
		}
		sandboxDirectoryPath = utf16Buffer;

		// Create an MD5 hash of the resource directory path.
		WinString stringConverter;
		stringConverter.SetUTF16(resourceDirectoryPathWithoutSlash.c_str());
		stringConverter.MakeLowerCase();
		CryptoPP::Weak1::MD5 md5;
		const int binaryHashLength = CryptoPP::Weak1::MD5::DIGESTSIZE;
		byte binaryHash[binaryHashLength];
		md5.CalculateDigest(binaryHash, (const byte*)stringConverter.GetUTF8(), strlen(stringConverter.GetUTF8()));

		// Convert the above MD5 hash into a UTF-16 encoded hexadecimal string.
		const int utf16HexadecimalHashLength = (sizeof(binaryHash) * 2) + 1;
		wchar_t utf16HexadecimalHash[utf16HexadecimalHashLength];
		for (index = 0; index < binaryHashLength; index++)
		{
			wnsprintf(utf16HexadecimalHash + (index * 2), 3, L"%02X", binaryHash[index]);
		}
		utf16HexadecimalHash[utf16HexadecimalHashLength - 1] = L'\0';

		// Now generate the project's sandbox path under the user's hidden "Application Data" directory.
		// Format:  <LocalAppDataPath>\Corona Labs\Corona Simulator\Sandbox\<ProjectFolderName>-<Hash>
		sandboxDirectoryPath.append(L"\\Corona Labs\\Corona Simulator\\Sandbox\\");
		if (projectFolderName.length() > 0)
		{
			sandboxDirectoryPath.append(projectFolderName);
			sandboxDirectoryPath.append(L"-");
		}
		sandboxDirectoryPath.append(utf16HexadecimalHash);
	}

	// Copy the sandbox directory path to the destination string buffer.
	destinationString = sandboxDirectoryPath;
	return true;
}

bool RuntimeEnvironment::CopySimulatorRootAppDataDirectoryPathTo(std::wstring& path)
{
	// Fetch a path to the user's roaming app data directory.
	wchar_t appDataDirectoryPath[1024];
	appDataDirectoryPath[0] = L'\0';
	HRESULT result = ::SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appDataDirectoryPath);
	if (FAILED(result) || (L'\0' == appDataDirectoryPath[0]))
	{
		return false;
	}

	// Append the Corona Simulator's subdirectory path and copy it to the given argument.
	path = appDataDirectoryPath;
	path += L"\\Corona Labs\\Corona Simulator";
	return true;
}

bool RuntimeEnvironment::CopySimulatorPluginDirectoryPathTo(std::wstring& path)
{
	// Fetch the Corona Simulator root path under the user's AppData directory.
	std::wstring pluginPath;
	auto result = CopySimulatorRootAppDataDirectoryPathTo(pluginPath);
	if (!result || pluginPath.empty())
	{
		return false;
	}

	// Append the simulator's plugin subdirectory to the path and copy it to the given argument.
	path = pluginPath;
	path += L"\\Plugins";
	return true;
}

#pragma endregion


#pragma region Private Methods
void RuntimeEnvironment::Suspend()
{
	if (fRuntimePointer && !fRuntimePointer->IsSuspended())
	{
		fRuntimePointer->Suspend();
	}
}

void RuntimeEnvironment::Resume()
{
	// Do not continue if the runtime has not been started yet or has already been terminated.
	if (!fRuntimePointer || (fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded) == false))
	{
		return;
	}

	// Resume the runtime if currently suspended.
	if (fRuntimePointer->IsSuspended())
	{
		fRuntimePointer->Resume();
		UpdateOrientationAndSurfaceSize();
	}
}

void RuntimeEnvironment::OnRuntimeTimerElapsed()
{
	// Do not continue if the runtime has not been loaded.
	if (!fRuntimePointer || (fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded) == false))
	{
		return;
	}

	// Do not continue if the window hosting the runtime was minimized.
	if (fProjectSettings.SuspendWhenMinimized() && fMainWindowPointer)
	{
		auto windowHandle = fMainWindowPointer->GetWindowHandle();
		if (windowHandle && ::IsIconic(windowHandle))
		{
			Suspend();
			return;
		}
	}

	// Update the runtime's scene such as physics, sprites, etc.
	// Note: This does not render the scene since the "kRenderAsync" property is set.
	fEnteringFrameEvent.Raise(*this, EventArgs::kEmpty);
	(*fRuntimePointer)();
	fEnteredFrameEvent.Raise(*this, EventArgs::kEmpty);

	// Request the surface (if we have one) to render another frame, but only if the scene has changed.
	if (fRenderSurfacePointer)
	{
		if (fRuntimePointer->GetDisplay().GetStage()->GetScene().IsValid() == false)
		{
			fRenderSurfacePointer->RequestRender();
		}
	}
}

void RuntimeEnvironment::OnMainWindowReceivedMessage(UI::UIComponent &sender, UI::HandleMessageEventArgs &arguments)
{
	switch (arguments.GetMessageId())
	{
		case WM_ACTIVATE:
		{
			if (fRuntimePointer)
			{
				bool foreground = LOWORD(arguments.GetWParam()) > 0;
				if (fLastActivatedSent != foreground)
				{
					fLastActivatedSent = foreground;
					Rtt::WindowStateEvent event(foreground);
					fRuntimePointer->DispatchEvent(event);
				}
			}
			break;
		}
		case WM_SIZING:
		{
			// Make sure the window is not made smaller than the configured min width/height, if provided.
			LONG minClientWidth = (LONG)fProjectSettings.GetMinWindowViewWidth();
			LONG minClientHeight = (LONG)fProjectSettings.GetMinWindowViewHeight();
			LPRECT boundsPointer = (LPRECT)arguments.GetLParam();
			if (boundsPointer && (minClientWidth > 0) && (minClientHeight > 0))
			{
				// Fetch the border/margin distance between the edge of the window and its client area.
				UI::Control& control = dynamic_cast<UI::Control&>(sender);
				RECT windowBounds = control.GetBounds();
				RECT clientBounds = control.GetClientBounds();
				LONG totalMarginX = (windowBounds.right - windowBounds.left) - (clientBounds.right - clientBounds.left);
				LONG totalMarginY = (windowBounds.bottom - windowBounds.top) - (clientBounds.bottom - clientBounds.top);

				// Update the window's bounds provided by the message if it's less than the minimum.
				LONG length = (boundsPointer->right - boundsPointer->left);
				LONG minLength = minClientWidth + totalMarginX;
				if (length < minLength)
				{
					boundsPointer->right += minLength - length;
					arguments.SetHandled();
				}
				length = (boundsPointer->bottom - boundsPointer->top);
				minLength = minClientHeight + totalMarginY;
				if (length < minLength)
				{
					boundsPointer->bottom += minLength - length;
					arguments.SetHandled();
				}
				if (arguments.WasHandled())
				{
					arguments.SetReturnResult(1);
				}
			}
			break;
		}
		case WM_SYSCOMMAND:
		{
			// Suspend/resume the runtime if the window has been minimized/unminized respectively.
			if (fProjectSettings.SuspendWhenMinimized())
			{
				auto commandId = arguments.GetWParam() & 0xFFF0;
				if (SC_MINIMIZE == commandId)
				{
					Suspend();
				}
				else if ((SC_RESTORE == commandId) || (SC_MAXIMIZE == commandId))
				{
					if (!fWasSuspendRequestedExternally)
					{
						Resume();
					}
				}
			}
			break;
		}
		case WM_SHOWWINDOW:
		case WM_WINDOWPOSCHANGED:
		{
			// Suspend the runtime if the window was minimized.
			if (fProjectSettings.SuspendWhenMinimized())
			{
				if (::IsIconic(arguments.GetWindowHandle()))
				{
					Suspend();
				}
				else if (!fWasSuspendRequestedExternally)
				{
					Resume();
				}
			}
			break;
		}
		case WM_ACTIVATEAPP:
		{
			// Force the rendering surface to re-paint itself.
			// Note: This works-around an issue with some video cards where the surface will have
			//       a lot of ugly artifacts when switching to/from a fullscreen Corona app.
			if (fRenderSurfacePointer)
			{
				::InvalidateRect(fRenderSurfacePointer->GetWindowHandle(), nullptr, FALSE);
			}
			break;
		}
		case WM_CLOSE:
		{
			// Do not allow the window to be closed if its "Close" [x] button is currently disabled.
			// This prevents the user from closing the window via Alt+F4 or via the Windows taskbar.
			HMENU menuHandle = ::GetSystemMenu(arguments.GetWindowHandle(), FALSE);
			if (menuHandle)
			{
				auto menuItemState = ::GetMenuState(menuHandle, SC_CLOSE, MF_BYCOMMAND);
				if (menuItemState & MF_DISABLED)
				{
					arguments.SetReturnResult(0);
					arguments.SetHandled();
					break;
				}
			}

//TODO: In the future, we should dispatch an event to Lua to determine if the window should be closed or not.

			// Store the window's current position, size, and mode to the application's preferences.
			if (fStoredPreferencesPointer && fMainWindowPointer)
			{
				// Fetch the window's current positional info.
				const Rtt::NativeWindowMode& windowMode = fMainWindowPointer->GetWindowMode();
				auto windowBounds = fMainWindowPointer->GetNormalModeBounds();
				auto clientSize = fMainWindowPointer->GetNormalModeClientSize();

				// Write the above window info to this app's stored preferences.
				Rtt::PreferenceCollection preferenceCollection;
				preferenceCollection.Add(
						"corona/lastWindowPosition/x", Rtt::PreferenceValue(windowBounds.left));
				preferenceCollection.Add(
						"corona/lastWindowPosition/y", Rtt::PreferenceValue(windowBounds.top));
				preferenceCollection.Add(
						"corona/lastWindowPosition/viewWidth", Rtt::PreferenceValue(clientSize.cx));
				preferenceCollection.Add(
						"corona/lastWindowPosition/viewHeight", Rtt::PreferenceValue(clientSize.cy));
				preferenceCollection.Add(
						"corona/lastWindowPosition/mode", Rtt::PreferenceValue(windowMode.GetStringId()));
				fStoredPreferencesPointer->UpdateWith(preferenceCollection);
			}

			// Let the system's default handler decide if the window should be closed.
			break;
		}
	}
}

void RuntimeEnvironment::OnIpcWindowReceivedMessage(UI::UIComponent &sender, UI::HandleMessageEventArgs &arguments)
{
	// Do not continue if the received message was already handled.
	if (arguments.WasHandled())
	{
		return;
	}

	// Handle the received message.
	switch (arguments.GetMessageId())
	{
		case WM_COPYDATA:
		{
			// Validate.
			auto dataPointer = (COPYDATASTRUCT*)arguments.GetLParam();
			size_t characterLength = dataPointer->cbData / sizeof(wchar_t);
			if (!dataPointer || !dataPointer->lpData || (dataPointer->dwData != 1) || (characterLength <= 0))
			{
				break;
			}

			// Do not continue if the received message does not start with a "corona:" prefix.
			// This blocks rogue messages that might be broadcasted by naughty 3rd party apps.
			const wchar_t kCoronaHeaderPrefix[] = L"corona:";
			const size_t kCoronaHeaderPrefixLength = wcslen(kCoronaHeaderPrefix);
			if (characterLength < kCoronaHeaderPrefixLength)
			{
				break;
			}
			if (wcsncmp(kCoronaHeaderPrefix, (wchar_t*)dataPointer->lpData, kCoronaHeaderPrefixLength) != 0)
			{
				break;
			}

			// Parse the first line from the given WM_COPYDATA buffer.
			// This first line is the command that is being sent to this application.
			std::wstring receivedString((wchar_t*)dataPointer->lpData, characterLength);
			std::wistringstream stringStream(receivedString);
			std::wstring commandString(L"");
			std::getline(stringStream, commandString);

			// Determine which operation should be performed.
			if (!wcscmp(commandString.c_str(), L"corona:hasAppWindow"))
			{
				// We're being asked if this Corona runtime has been assigned an application window.
				// Return non-zero for true. Return zero for false.
				arguments.SetReturnResult(fMainWindowPointer ? 1 : 0);
				arguments.SetHandled();
			}
			else if (!wcscmp(commandString.c_str(), L"corona:isSingleInstance"))
			{
				// We're being asked if this Corona runtime is a single instance application.
				// Return non-zero for true. Return zero for false.
				arguments.SetReturnResult(fSingleWindowInstanceSemaphoreHandle ? 1 : 0);
				arguments.SetHandled();
			}
			else if (!wcscmp(commandString.c_str(), L"corona:applicationOpen"))
			{
				// We're being asked to display the app window and dispatch an "applicationOpen" system event.
				std::string utf8Url("");
				std::vector<std::shared_ptr<std::string>> utf8Arguments;
				std::vector<const char*> utf8ArgumentPointers;
				std::string utf8WorkingDirectory;

				// Fetch additional newline separated parameters from the WM_COPYDATA buffer, if any.
				WinString stringTranscoder;
				for (std::wstring utf16String; stringStream && std::getline(stringStream, utf16String);)
				{
					// Determine which parameter we were given.
					if (!wcscmp(utf16String.c_str(), L"/commandLineDir"))
					{
						// We were given a new working directory.
						if (stringStream)
						{
							utf16String.clear();
							std::getline(stringStream, utf16String);
							if (::PathIsDirectoryW(utf16String.c_str()))
							{
								stringTranscoder.SetUTF16(utf16String.c_str());
								utf8WorkingDirectory = stringTranscoder.GetUTF8();
							}
						}
					}
					else if (!wcscmp(utf16String.c_str(), L"/commandLineArgs"))
					{
						// All proceeding lines in WM_COPYDATA's buffer are command line arguments.
						// Copy them to the UTF-8 argument collections.
						while (stringStream && std::getline(stringStream, utf16String))
						{
							// Add the argument string to our temporary collections.
							stringTranscoder.SetUTF16(utf16String.c_str());
							utf8Arguments.push_back(std::make_shared<std::string>(stringTranscoder.GetUTF8()));
							utf8ArgumentPointers.push_back(utf8Arguments.back()->c_str());

							// Check if this argument is a valid URL or DOS path, if we haven't found one already.
							// This will be passed to Lua via the system event's "url" property.
							if (utf8Url.empty())
							{
								if (::PathIsURLW(stringTranscoder.GetUTF16()))
								{
									// The argument is a valid URL. Keep the string as-is.
									utf8Url = stringTranscoder.GetUTF8();
								}
								else if (::PathIsUNCW(stringTranscoder.GetUTF16()))
								{
									// The argument is a Windows UNC path. (Ex: "\\MachineName\SharedFolder")
									// Note: For best performance, we want to avoid a path existence check with
									//       UNC paths because they are blocking and can take several seconds.
									utf8Url = stringTranscoder.GetUTF8();
								}
								else if (::PathFileExistsW(stringTranscoder.GetUTF16()) ||
										 ::PathIsDirectoryW(stringTranscoder.GetUTF16()))
								{
									// The argument references an existing file or directory.
									// Only accept it if it's an absolute path.
									// Note: Relative paths are too ambiguous and might be string literals.
									if (::PathIsRelativeW(stringTranscoder.GetUTF16()) == FALSE)
									{
										utf8Url = stringTranscoder.GetUTF8();
									}
								}
							}
						}
					}
				}

				// If we haven't received a working directory from WM_COPYDATA, then use this app's working directory.
				if (utf8WorkingDirectory.empty())
				{
					DWORD characterCount = ::GetCurrentDirectoryW(0, nullptr);
					if (characterCount > 0)
					{
						auto utf16WorkingDirectory = new wchar_t[characterCount];
						if (utf16WorkingDirectory)
						{
							utf16WorkingDirectory[0] = L'\0';
							::GetCurrentDirectoryW(characterCount, utf16WorkingDirectory);
							if (utf16WorkingDirectory[0] != L'\0')
							{
								stringTranscoder.SetUTF16(utf16WorkingDirectory);
								utf8WorkingDirectory = stringTranscoder.GetUTF8();
							}
							delete[] utf16WorkingDirectory;
						}
					}
				}

				// Bring the window to the front of the desktop.
				if (fMainWindowPointer)
				{
					HWND windowHandle = fMainWindowPointer->GetWindowHandle();
					if (fMainWindowPointer->GetWindowMode() == Rtt::NativeWindowMode::kMinimized)
					{
						::ShowWindow(windowHandle, SW_RESTORE);
					}
					::SetForegroundWindow(windowHandle);
				}

				// Resume the Corona runtime if currently suspended.
				if (fRuntimePointer && fRuntimePointer->IsSuspended())
				{
					fRuntimePointer->Resume();
				}
				if (!fRuntimePointer)
				{
					break;
				}

				// Dispatch an "applicationOpen" system event to Lua, but only if we were given arguments.
				if (!utf8Url.empty() || !utf8ArgumentPointers.empty())
				{
					Rtt::SystemOpenEvent event(utf8Url.empty() ? "" : utf8Url.c_str());
					if (utf8ArgumentPointers.size() > 0)
					{
						event.SetCommandLineArgs((int)utf8ArgumentPointers.size(), &utf8ArgumentPointers.front());
					}
					if (!utf8WorkingDirectory.empty())
					{
						event.SetCommandLineDir(utf8WorkingDirectory.c_str());
					}
					fRuntimePointer->DispatchEvent(event);
				}

				// Flag that this event was handled.
				arguments.SetReturnResult(1);
				arguments.SetHandled();
			}
			break;
		}
	}
}

void RuntimeEnvironment::OnRenderFrame(UI::RenderSurfaceControl &sender, HandledEventArgs &arguments)
{
	// Do not continue if the runtime is not currently running.
	if (!fRuntimePointer || (fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded) == false))
	{
		return;
	}
	switch (fRuntimeState)
	{
		case RuntimeState::kNotStarted:
		case RuntimeState::kTerminating:
		case RuntimeState::kTerminated:
			return;
	}

	// Have the runtime render a frame.
	fRuntimePointer->GetDisplay().Invalidate();
	fRuntimePointer->Render();
	arguments.SetHandled();
}

void RuntimeEnvironment::OnDestroyingSurface(UI::UIComponent &sender, const EventArgs &arguments)
{
	// Terminate the runtime now that we no longer have a window/control to render to anymore.
	// Note: Cannot delete the render surface yet since this method is being called by the surface. (Would cause a crash.)
	if (fRuntimePointer)
	{
		// We can't reliably select the OpenGL context of a window/control that is being destroyed.
		// So, let Corona's OpenGL functions no-op and let the surface destroy the context itself.
		::wglMakeCurrent(nullptr, nullptr);

		// Delete the Corona runtime.
		// Note: This will dispatch a Lua "applicationEvent" and terminate the runtime before deletion.
		Rtt_DELETE(fRuntimePointer);
		fRuntimePointer = nullptr;
	}
}

void RuntimeEnvironment::OnSurfaceResized(UI::Control &sender, const EventArgs &arguments)
{
	// Update the runtime's cached orientation and surface size.
	UpdateOrientationAndSurfaceSize();

	// Check if the Corona runtime's main timer is ready to elapse, and if so, then invoke it now.
	// This will trigger an "enterFrame" in the runtime, which provides a smoother resize animation.
	// Note: Microsoft blocks the window's main message pump while that same window is being resized.
	//       This prevents timers from elapsing since their queued WM_TIMER message will be blocked as well.
	if (fRuntimePointer)
	{
		auto timerPointer = dynamic_cast<Rtt::WinTimer*>(fRuntimePointer->GetTimer());
		if (timerPointer)
		{
			timerPointer->Evaluate();
		}
	}
}

void RuntimeEnvironment::UpdateOrientationAndSurfaceSize()
{
	// Do not continue unless the Corona runtime is currently running.
	if (!fRuntimePointer)
	{
		return;
	}

	// Do not continue if a rendering surface was not provided to this runtime.
	if (!fRenderSurfacePointer)
	{
		return;
	}

	// Fetch the current orientation. Only applies if simulating a device.
	auto previousOrientation = fLastOrientation;
	auto currentOrientation = Rtt::DeviceOrientation::kUpright;
	auto deviceSimulatorServicesPointer = GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		currentOrientation = deviceSimulatorServicesPointer->GetOrientation();
	}
	else if (Rtt::DeviceOrientation::IsInterfaceOrientation(fProjectSettings.GetDefaultOrientation()))
	{
		currentOrientation = fProjectSettings.GetDefaultOrientation();
	}

	// Determine if there was an orientation change.
	bool hasOrientationChanged = (currentOrientation != fLastOrientation);

	// Determine if the content width/height was resized.
	// Note: An orientation change from portrait->landscape or vice-versa counts as a resize.
	bool hasSurfaceSizeChanged = false;
	if ((Rtt::DeviceOrientation::IsUpright(currentOrientation) != Rtt::DeviceOrientation::IsUpright(fLastOrientation)) ||
	    fRuntimePointer->GetDisplay().HasWindowSizeChanged())
	{
		hasSurfaceSizeChanged = true;
	}

	// Store the current orientation.
	fLastOrientation = currentOrientation;

	// Update the renderer if a change was detected.
	if (hasOrientationChanged || hasSurfaceSizeChanged)
	{
#ifdef Rtt_AUTHORING_SIMULATOR
		if (deviceSimulatorServicesPointer)
		{
			fRuntimePointer->GetDisplay().WindowDidRotate(
					currentOrientation, fProjectSettings.IsSupported(currentOrientation));
		}
		else
#endif
		{
			if (hasOrientationChanged)
			{
				fRuntimePointer->GetDisplay().SetContentOrientation(currentOrientation);
			}
			if (fRuntimePointer->GetDisplay().HasWindowSizeChanged())
			{
				fRuntimePointer->GetDisplay().WindowSizeChanged();
			}
			fRuntimePointer->RestartRenderer(currentOrientation);
		}
		fRuntimePointer->GetDisplay().Invalidate();
	}

	// Dispatch an 'orientation" event to Lua if it has changed.
	if (hasOrientationChanged)
	{
		Rtt::OrientationEvent event(currentOrientation, previousOrientation);
		fRuntimePointer->DispatchEvent(event);
	}

	// Dispatch a "resize" event to Lua if it has changed.
	if (hasSurfaceSizeChanged && fProjectSettings.IsSupported(currentOrientation))
	{
		fRuntimePointer->DispatchEvent(Rtt::ResizeEvent());
	}
}

void RuntimeEnvironment::UpdateMainWindowUsing(const Rtt::ReadOnlyProjectSettings& projectSettings)
{
	// Validate.
	if (!fMainWindowPointer)
	{
		return;
	}

	// Fetch the main window's Win32 handle.
	HWND windowHandle = fMainWindowPointer->GetWindowHandle();
	if (!windowHandle)
	{
		return;
	}

	// Enable/disable the window's icon menu, minimize button, maximize button, and ability to resize.
	LONG oldWindowStyles = ::GetWindowLongW(windowHandle, GWL_STYLE);
	LONG newWindowStyles = oldWindowStyles;
	if (projectSettings.IsWindowResizable())
	{
		newWindowStyles |= WS_SIZEBOX;
	}
	else
	{
		newWindowStyles &= ~WS_SIZEBOX;
	}
	if (projectSettings.IsWindowMinimizeButtonEnabled())
	{
		newWindowStyles |= WS_MINIMIZEBOX;
	}
	else
	{
		newWindowStyles &= ~WS_MINIMIZEBOX;
	}
	if (projectSettings.IsWindowMaximizeButtonEnabled())
	{
		newWindowStyles |= WS_MAXIMIZEBOX;
	}
	else
	{
		newWindowStyles &= ~WS_MAXIMIZEBOX;
	}
	if (newWindowStyles != oldWindowStyles)
	{
		::SetWindowLongW(windowHandle, GWL_STYLE, newWindowStyles);
		::SetWindowPos(
				windowHandle, nullptr, 0, 0, 0, 0,
				SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
	}

	// Enable/disable the window's close button.
	HMENU menuHandle = ::GetSystemMenu(windowHandle, FALSE);
	if (menuHandle)
	{
		UINT enableFlag = projectSettings.IsWindowCloseButtonEnabled() ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
		::EnableMenuItem(menuHandle, SC_CLOSE, MF_BYCOMMAND | enableFlag);
	}

	// Update the window's title bar text.
	{
		// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
		// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
		//       For example, this can happen with the Hawaiian language, which will return "haw".
		const size_t kLanguageStringCodeLength = 128;
		char languageStringCode[kLanguageStringCodeLength];
		languageStringCode[0] = '\0';
		::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SPARENT, languageStringCode, kLanguageStringCodeLength);
		if (languageStringCode[0] != '\0')
		{
			// Special handling for older OS versions.
			// Replace non-standard Chinese "CHS" and "CHT" script names with respective ISO 15924 codes.
			if (_stricmp(languageStringCode, "zh-chs") == 0)
			{
				strncpy_s(languageStringCode, kLanguageStringCodeLength, "zh-Hans", kLanguageStringCodeLength);
			}
			else if (_stricmp(languageStringCode, "zh-cht") == 0)
			{
				strncpy_s(languageStringCode, kLanguageStringCodeLength, "zh-Hant", kLanguageStringCodeLength);
			}
		}
		else
		{
			// Use an older API that only fetches the ISO 639 language code if the above API call fails.
			::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, languageStringCode, kLanguageStringCodeLength);
		}

		// Fetch the 2 character ISO country code the system is currently using. (Ex: "us" for USA.)
		char countryStringCode[16];
		countryStringCode[0] = '\0';
		::GetLocaleInfoA(
				LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, countryStringCode, sizeof(countryStringCode));

		// Attempt to update the title bar with the "build.settings" file's localized text for the current locale.
		auto utf8TitleText = projectSettings.GetWindowTitleTextForLocale(languageStringCode, countryStringCode);
		if (utf8TitleText)
		{
			auto utf16TitleText = lua_create_utf16_string_from(utf8TitleText);
			if (utf16TitleText)
			{
				::SetWindowTextW(windowHandle, utf16TitleText);
				lua_destroy_utf16_string(utf16TitleText);
			}
		}
	}

	// Move the window's "normal" mode position to its last remembered location.
	// We need to do this before setting the width/height below which sometimes sizes based on monitor resolution.
	if (fStoredPreferencesPointer)
	{
		bool hasWindowPosition = false;
		POINT windowPosition{};
		{
			// Fetch the last stored position from preferences, if it exists.
			auto lastXResult = fStoredPreferencesPointer->Fetch("corona/lastWindowPosition/x");
			auto lastYResult = fStoredPreferencesPointer->Fetch("corona/lastWindowPosition/y");
			if (lastXResult.HasSucceeded() && lastYResult.HasSucceeded())
			{
				hasWindowPosition = true;
				auto intConversionResult = lastXResult.GetValue().ToSignedInt32();
				if (intConversionResult.HasSucceeded())
				{
					windowPosition.x = intConversionResult.GetValue();
				}
				else
				{
					hasWindowPosition = false;
				}
				intConversionResult = lastYResult.GetValue().ToSignedInt32();
				if (intConversionResult.HasSucceeded())
				{
					windowPosition.y = intConversionResult.GetValue();
				}
				else
				{
					hasWindowPosition = false;
				}
			}
		}
		if (hasWindowPosition)
		{
			// Move the window.
			auto windowBounds = fMainWindowPointer->GetBounds();
			auto deltaX = windowPosition.x - windowBounds.left;
			auto deltaY = windowPosition.y - windowBounds.top;
			windowBounds.left = windowPosition.x;
			windowBounds.top = windowPosition.y;
			windowBounds.right += deltaX;
			windowBounds.bottom += deltaY;
			fMainWindowPointer->SetBounds(windowBounds);
		}
	}

	// Fetch the last window width/height this application was using before.
	// Note: We only fetch the previous width/height if the window is set up to be resizable.
	//       If not resizable, then we leverage the configured width/height and image suffix scaling to fit the monitor.
	bool isClientSizeFromPreferences = false;
	SIZE clientSize{};
	if (fStoredPreferencesPointer)
	{
		if (projectSettings.IsWindowResizable())
		{
			auto lastWidthResult = fStoredPreferencesPointer->Fetch("corona/lastWindowPosition/viewWidth");
			auto lastHeightResult = fStoredPreferencesPointer->Fetch("corona/lastWindowPosition/viewHeight");
			if (lastWidthResult.HasSucceeded() && lastHeightResult.HasSucceeded())
			{
				clientSize.cx = lastWidthResult.GetValue().ToSignedInt32().GetValue();
				clientSize.cy = lastHeightResult.GetValue().ToSignedInt32().GetValue();
				isClientSizeFromPreferences = (clientSize.cx > 0) && (clientSize.cy > 0);
			}
		}
	}

	// If we haven't restored the previous window client width/height above, then do the following:
	// - First, attempt to fetch the default window client width/height from the "build.settings" file.
	// - If not provided, then fetch the content width/height from the "config.lua" file for backward compatibility.
	if ((clientSize.cx <= 0) || (clientSize.cy <= 0))
	{
		clientSize.cx = projectSettings.GetDefaultWindowViewWidth();
		clientSize.cy = projectSettings.GetDefaultWindowViewHeight();
		if ((clientSize.cx <= 0) || (clientSize.cy <= 0))
		{
			clientSize.cx = projectSettings.GetMinWindowViewWidth();
			clientSize.cy = projectSettings.GetMinWindowViewHeight();
			if ((clientSize.cx <= 0) || (clientSize.cy <= 0))
			{
				clientSize.cx = projectSettings.GetContentWidth();
				clientSize.cy = projectSettings.GetContentHeight();
				if (Rtt::DeviceOrientation::IsSideways(projectSettings.GetDefaultOrientation()))
				{
					Rtt::Swap<LONG>(clientSize.cx, clientSize.cy);
				}
			}
		}
	}

	// If we've failed to fetch a window content width/height above, then use a hardcoded default.
	if ((clientSize.cx <= 0) || (clientSize.cy <= 0))
	{
		if (projectSettings.HasBuildSettings())
		{
			clientSize.cx = 320;
			clientSize.cy = 480;
			if (projectSettings.IsLandscapeSupported())
			{
				Rtt::Swap<LONG>(clientSize.cx, clientSize.cy);
			}
		}
	}

	// If the project supports image suffix scales (ie: @2x, @3x, etc.) then attempt to increase
	// the size of the window using these scales to best-fit the desktop monitor it is in.
	// Note: Never apply the image suffix scale if restoring the previous app instance's window width/height.
	if (!isClientSizeFromPreferences &&
	    (clientSize.cx > 0) && (clientSize.cy > 0) &&
	    (projectSettings.GetImageSuffixScaleCount() > 0))
	{
		auto monitorHandle = ::MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
		if (monitorHandle)
		{
			// Fetch the workspace size of the monitor the app is currently positioned in.
			// The workspace area excludes the Windows taskbar and sidebars.
			MONITORINFOEXW monitorInfo{};
			monitorInfo.cbSize = sizeof(monitorInfo);
			::GetMonitorInfoW(monitorHandle, &monitorInfo);
			auto workspaceWidth = std::abs(monitorInfo.rcWork.right - monitorInfo.rcWork.left);
			auto workspaceHeight = std::abs(monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);

			// Fetch the size of the window's borders, excluding the client area.
			auto currentWindowSize = fMainWindowPointer->GetNormalModeSize();
			auto currentClientSize = fMainWindowPointer->GetNormalModeClientSize();
			auto totalBorderWidth = currentWindowSize.cx - currentClientSize.cx;
			auto totalBorderHeight = currentWindowSize.cy - currentClientSize.cy;

			// Use an image suffix scale that'll best-fit the workspace/desktop area.
			auto maxWidth = workspaceWidth - totalBorderWidth;
			auto maxHeight = workspaceHeight - totalBorderHeight;
			if ((maxWidth > 0) && (maxHeight > 0))
			{
				for (int index = projectSettings.GetImageSuffixScaleCount() - 1; index >= 0; index--)
				{
					double scale = projectSettings.GetImageSuffixScaleByIndex(index);
					if (((clientSize.cx * scale) <= maxWidth) && ((clientSize.cy * scale) <= maxHeight))
					{
						clientSize.cx = (LONG)((double)clientSize.cx * scale);
						clientSize.cy = (LONG)((double)clientSize.cy * scale);
						break;
					}
				}
			}
		}
	}

	// Make sure the window width/height does not exceed the minimum in the "build.settings" file, if provided.
	LONG minClientWidth = projectSettings.GetMinWindowViewWidth();
	LONG minClientHeight = projectSettings.GetMinWindowViewHeight();
	if (clientSize.cx < minClientWidth)
	{
		clientSize.cx = minClientWidth;
	}
	if (clientSize.cy < minClientHeight)
	{
		clientSize.cy = minClientHeight;
	}

	// Resize the window.
	// Note: This must be done after changing the window's styles.
	//       Particulary any styles that effect the border size, such as WS_SIZEBOX.
	if ((clientSize.cx > 0) && (clientSize.cy > 0))
	{
		fMainWindowPointer->SetNormalModeClientSize(clientSize);
	}

	// Fetch the window mode (ie: "normal", "maximized", "fullscreen", etc.) the last time this app was running with.
	const Rtt::NativeWindowMode* windowModePointer = nullptr;
	if (fStoredPreferencesPointer)
	{
		auto fetchResult = fStoredPreferencesPointer->Fetch("corona/lastWindowPosition/mode");
		if (fetchResult.HasSucceeded())
		{
			auto modeNamePointer = fetchResult.GetValue().ToString().GetValue();
			if (modeNamePointer.NotNull())
			{
				windowModePointer = Rtt::NativeWindowMode::FromStringId(modeNamePointer->c_str());
			}
		}
	}
	if (windowModePointer)
	{
		// Make sure that the fetched window mode is okay to use.
		if (windowModePointer->Equals(Rtt::NativeWindowMode::kMinimized))
		{
			// The window used to be minimized. Do not restore this window mode state. This is what Microsoft does too.
			// Otherwise, users will get confused and think the app isn't running because the window didn't appear.
			windowModePointer = nullptr;
		}
		else if (windowModePointer->Equals(Rtt::NativeWindowMode::kMaximized))
		{
			// The last mode was maximized.
			// Do not use this mode if the "build.settings" file no longer supports it.
			if (!projectSettings.IsWindowMaximizeButtonEnabled())
			{
				windowModePointer = nullptr;
			}
		}
	}
	if (!windowModePointer)
	{
		// The last window mode was not found or is no longer valid.
		// Attempt to use the default window mode from the "build.settings" file, if specified.
		windowModePointer = projectSettings.GetDefaultWindowMode();
	}

	// Apply the selected window mode up above, if provided.
	// Note: Do not apply the window mode if the given window is maximimzed or minimized, because this assumes
	//       that the end-user applied this mode to the application's shortcut, which we should always respect.
	if (windowModePointer)
	{
		const Rtt::NativeWindowMode& currentWindowMode = fMainWindowPointer->GetWindowMode();
		if (currentWindowMode.Equals(Rtt::NativeWindowMode::kNormal))
		{
			fMainWindowPointer->SetWindowMode(*windowModePointer);
		}
	}
}

int RuntimeEnvironment::OnLuaLoadPackage(lua_State* luaStatePointer)
{
	// Fetch the package name that the Lua require() function is trying to load.
	const char* packageName = lua_tostring(luaStatePointer, 1);
	if (Rtt_StringIsEmpty(packageName))
	{
		lua_pushnil(luaStatePointer);
		return 1;
	}

	// Convert the given Lua package name into a valid C function name.
	// We do this by replacing all period characters with underscores.
	std::string partialFunctionName(packageName);
	std::replace(partialFunctionName.begin(), partialFunctionName.end(), '.', '_');

	// Fetch a handle to this library.
	HMODULE moduleHandle = nullptr;
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	::GetModuleHandleExW(flags, (LPCWSTR)OnLuaLoadPackage, &moduleHandle);

	// Check if the request Lua package/plugin is compiled into this library.
	std::string fullFunctionName = "luaload_" + partialFunctionName;
	lua_CFunction functionPointer = (lua_CFunction)GetProcAddress(moduleHandle, fullFunctionName.c_str());
	if (!functionPointer)
	{
		fullFunctionName = "luaopen_" + partialFunctionName;
		functionPointer = (lua_CFunction)GetProcAddress(moduleHandle, fullFunctionName.c_str());
		if (!functionPointer)
		{
			fullFunctionName = "CoronaPluginLuaLoad_" + partialFunctionName;
			functionPointer = (lua_CFunction)GetProcAddress(moduleHandle, fullFunctionName.c_str());
		}
	}

	// If the plugin is compiled into this library, then push its C function into Lua.
	if (functionPointer)
	{
		lua_pushcfunction(luaStatePointer, functionPointer);
	}
	else
	{
		lua_pushnil(luaStatePointer);
	}
	return 1;
}

#pragma endregion


#pragma region RuntimeDelegate Class Methods
RuntimeEnvironment::RuntimeDelegate::RuntimeDelegate(RuntimeEnvironment* environmentPointer)
:	fEnvironmentPointer(environmentPointer),
	Rtt::RuntimeDelegatePlayer()
{
	if (!environmentPointer)
	{
		throw std::invalid_argument(nullptr);
	}
}

void RuntimeEnvironment::RuntimeDelegate::DidInitLuaLibraries(const Rtt::Runtime& sender) const
{
	// If the runtime is using the CoronaCards feature, then add Corona's resource directory path to Lua's package path.
	lua_State *L = sender.VMContext().L();
	if (sender.IsProperty(Rtt::Runtime::kIsApplicationNotArchived))
	{
		Rtt::LuaContext::InitializeLuaPath(L, sender.Platform());
	}

	// Push in Corona's other Lua libraries such as "socket", "mime", and "ltn12".
	// Note: These libraries are not typically provided by CoronaCards, but we offer them on Windows Phone.
	RuntimeDelegatePlayer::DidInitLuaLibraries(sender);

	// Add the Lua library's exported package loader which supports loading DLLs with embedded '.' in their file names.
	// For example, this allows plugins named "plugin.fuse.dll" to be loaded.
	typedef int(*LuaLoaderCFlatCallback)(lua_State*);
	static LuaLoaderCFlatCallback sLuaLoaderCFlatCallback = nullptr;
	if (!sLuaLoaderCFlatCallback)
	{
		HMODULE moduleHandle = nullptr;
		DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
		::GetModuleHandleExW(flags, (LPCWSTR)lua_newstate, &moduleHandle);
		if (moduleHandle)
		{
			sLuaLoaderCFlatCallback = (LuaLoaderCFlatCallback)::GetProcAddress(moduleHandle, "loader_Cflat");
			Rtt_ASSERT(sLuaLoaderCFlatCallback);
		}
	}
	if (sLuaLoaderCFlatCallback)
	{
		Rtt::Lua::InsertPackageLoader(sender.VMContext().L(), sLuaLoaderCFlatCallback, -1);
	}

	// Add a custom package/library loader to the runtime's Lua state.
	// This allows us to load plugins that are compiled into this Corona library, such as the "network" plugin.
	lua_State *luaStatePointer = sender.VMContext().L();
	auto luaPackageLoaderCallback = fEnvironmentPointer->GetLuaPackageLoaderCallback();
	if (luaPackageLoaderCallback)
	{
		Rtt::Lua::InsertPackageLoader(luaStatePointer, luaPackageLoaderCallback, -1);
	}
}

void RuntimeEnvironment::RuntimeDelegate::InitializeConfig(const Rtt::Runtime& sender, lua_State *L) const
{
	static const char kApplication[] = "application";
	static const char kMetadata[] = "metadata";

	Rtt_LUA_STACK_GUARD(L);

	// Query for an "application" table in Lua. (Normally created via "config.lua", if provided.)
	lua_getglobal(L, kApplication);
	{
		// If an "application" table does not exist, then create one.
		if (!lua_istable(L, -1))
		{
			// Pop nil off of the stack.
			lua_pop(L, 1);

			// Push a new "application" table to the top of the stack and to the global table.
			lua_createtable(L, 0, 1);
			lua_setglobal(L, kApplication);
		}
		int applicationLuaTableIndex = lua_gettop(L);

		// Disable launchpad analytics until we have a working replacement.
		lua_pushboolean(L, 0);
		lua_setfield(L, applicationLuaTableIndex, "launchPad");

		// Query for the "metadata" field, which is used by CoronaCards.
		lua_getfield(L, applicationLuaTableIndex, kMetadata);
		{
			// If a "metadata" table was not found, then create one.
			// Note: The CoronaCards licensing system requires this to exist.
			if (!lua_istable(L, -1))
			{
				// Pop nil off of the stack.
				lua_pop(L, 1);

				// Create a mock "metadata" table and add it to the global "application" table.
				lua_createtable(L, 0, 1);
				lua_pushvalue(L, -1);	// Leave "metadata" on stack. So, push extra.
				lua_setfield(L, applicationLuaTableIndex, kMetadata);
			}
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

void RuntimeEnvironment::RuntimeDelegate::DidLoadConfig(const Rtt::Runtime& sender, lua_State *L) const
{
	// Startup Microsoft GDI+. Needed by Corona to load images and generate text bitmaps.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&fEnvironmentPointer->fGdiPlusToken, &gdiplusStartupInput, nullptr);

	// If we were not able to load the "config.lua" file before Runtime::LoadApplication(), then try one more time here.
	// Note: This can happen if the "config.lua" contains Corona Lua APIs, which requires a runtime to execute.
	if (fEnvironmentPointer->fProjectSettings.HasConfigLua() == false)
	{
		// Load the "config.lua" file.
		fEnvironmentPointer->fProjectSettings.LoadFrom(sender);

		// If successful, update the window hosting the Corona runtime according to the loaded settings above.
		auto windowPointer = fEnvironmentPointer->fMainWindowPointer;
		if (windowPointer && fEnvironmentPointer->fProjectSettings.HasConfigLua())
		{
			fEnvironmentPointer->UpdateMainWindowUsing(fEnvironmentPointer->fReadOnlyProjectSettings);
		}
	}

	// Notify the system that we have loaded the "config.lua" file and we are about to execute the "shell.lua" file.
	fEnvironmentPointer->fLoadedEvent.Raise(*fEnvironmentPointer, EventArgs::kEmpty);
}

bool RuntimeEnvironment::RuntimeDelegate::HasDependencies(const Rtt::Runtime& sender) const
{
#if 1
	return true;
#else
	// Always return true if not using CoronaCards/Kit.
	// Note: Authorization is typically done via the signature attached to the "resource.car" in this case.
	bool isNotUsingCoronaKit = !sender.IsProperty(Rtt::Runtime::kShouldVerifyLicense);
	if (isNotUsingCoronaKit)
	{
		return true;
	}

	// *** The runtime is set up for CoronaCards. Handle its licensing below. ***

	// Check if the CoronaCards license file exists.
	bool wasLicenseFileFound = false;
	{
		Rtt::String utf8LicenseFilePath(&fEnvironmentPointer->fPlatformPointer->GetAllocator());
		fEnvironmentPointer->fPlatformPointer->PathForFile(
			"license.ccdata", Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, utf8LicenseFilePath);
		if (utf8LicenseFilePath.IsEmpty() == false)
		{
			wasLicenseFileFound = true;
		}
	}

	// Display a trial watermark if the license was not found.
	// Note: Android and iOS display a native alert message instead, which exits the app when a button has been pressed.
	//       We're lowering the barrier on this platform so that the Corona project created by our Visual Studio extension
	//       will "just work", which would hopefully provide a better developer experience when trying out our SDK.
	if (false == wasLicenseFileFound)
	{
		// Flag the Corona runtime to display a trial watermark.
		sender.SetShowingTrialMessage(true);

		// Log a message indicating where the developer can obtain a valid license file.
		Rtt::CKWorkflow workflow;
		auto alertSettings = workflow.CreateAlertSettingsFor(Rtt::CKWorkflow::kMissingLicense, "coronacards");
		Rtt_LogException(
			"[CoronaCards Trial Notice]\r\n"
			"   Corona is currently in trial mode. To obtain a valid license, please go here:\r\n"
			"   %s\r\n",
			alertSettings.ActionButtonUrl.c_str());
		return true;
	}

	// Load and validate the CoronaCards license.
	return Corona::DependencyLoader::CCDependencyCheck(sender);
#endif
}

void RuntimeEnvironment::RuntimeDelegate::WillLoadMain(const Rtt::Runtime& sender) const
{
	// Notify the system that we're about to load the "main.lua" file.
	// This is the system's opportunity to register custom APIs into Lua.
	fEnvironmentPointer->fStartingEvent.Raise(*fEnvironmentPointer, EventArgs::kEmpty);

	// Push launch arguments to Lua.
	lua_State *luaStatePointer = sender.VMContext().L();
	const size_t argumentCount = fEnvironmentPointer->fLaunchArguments.size();
	if (luaStatePointer && (argumentCount > 0))
	{
		if (Rtt_VERIFY(const_cast<Rtt::Runtime&>(sender).PushLaunchArgs(true) > 0))
		{
			int luaLaunchTableIndex = lua_gettop(luaStatePointer);
			if (lua_istable(luaStatePointer, luaLaunchTableIndex))
			{
				WinString* urlStringPointer = nullptr;

				// Add an "args" field providing a string array of command line arguments.
				lua_createtable(luaStatePointer, argumentCount, 0);
				int luaArgumentArrayIndex = lua_gettop(luaStatePointer);
				int argumentIndex = 0;
				for (auto&& nextArgument : fEnvironmentPointer->fLaunchArguments)
				{
					// Check if this argument is a valid URL or absolute DOS path, if we haven't found one already.
					// This will be passed to Lua via a "url" property.
					if (!urlStringPointer && !nextArgument.IsEmpty())
					{
						if (::PathIsURLW(nextArgument.GetUTF16()))
						{
							urlStringPointer = &nextArgument;
						}
						else if (::PathIsUNCW(nextArgument.GetUTF16()))
						{
							urlStringPointer = &nextArgument;
						}
						else if (::PathFileExistsW(nextArgument.GetUTF16()) ||
						         ::PathIsDirectoryW(nextArgument.GetUTF16()))
						{
							if (::PathIsRelativeW(nextArgument.GetUTF16()) == FALSE)
							{
								urlStringPointer = &nextArgument;
							}
						}
					}

					// Add the command line argument to the Lua "args" array.
					lua_pushstring(luaStatePointer, nextArgument.IsEmpty() ? "" : nextArgument.GetUTF8());
					lua_rawseti(luaStatePointer, luaArgumentArrayIndex, argumentIndex + 1);
					argumentIndex++;
				}
				lua_setfield(luaStatePointer, luaLaunchTableIndex, "args");

				// Add a "url" field if a URL or a file/directory path was found at the command line.
				if (urlStringPointer && !urlStringPointer->IsEmpty())
				{
					lua_pushstring(luaStatePointer, urlStringPointer->GetUTF8());
					lua_setfield(luaStatePointer, luaLaunchTableIndex, "url");
				}
			}
			lua_pop(luaStatePointer, 1);
		}
	}
}

void RuntimeEnvironment::RuntimeDelegate::DidLoadMain(const Rtt::Runtime& sender) const
{
	// Notify the system the we've finished executing the "main.lua" and the app is now running.
	fEnvironmentPointer->fRuntimeState = RuntimeState::kRunning;
	fEnvironmentPointer->fStartedEvent.Raise(*fEnvironmentPointer, EventArgs::kEmpty);

	// If a suspend was requested while loading/starting, then suspend it now.
	if (fEnvironmentPointer->fWasSuspendRequestedExternally)
	{
		fEnvironmentPointer->Suspend();
	}
}

void RuntimeEnvironment::RuntimeDelegate::WillSuspend(const Rtt::Runtime& sender) const
{
	fEnvironmentPointer->fRuntimeState = RuntimeState::kSuspending;
}

void RuntimeEnvironment::RuntimeDelegate::DidSuspend(const Rtt::Runtime& sender) const
{
	// Notify the system that the runtime has just been suspended.
	fEnvironmentPointer->fRuntimeState = RuntimeState::kSuspended;
	fEnvironmentPointer->fSuspendedEvent.Raise(*fEnvironmentPointer, EventArgs::kEmpty);
}

void RuntimeEnvironment::RuntimeDelegate::WillResume(const Rtt::Runtime& sender) const
{
	fEnvironmentPointer->fRuntimeState = RuntimeState::kResuming;
}

void RuntimeEnvironment::RuntimeDelegate::DidResume(const Rtt::Runtime& sender) const
{
	// Notify the system that the runtime has just been resumed.
	fEnvironmentPointer->fRuntimeState = RuntimeState::kRunning;
	fEnvironmentPointer->fResumedEvent.Raise(*fEnvironmentPointer, EventArgs::kEmpty);
}

void RuntimeEnvironment::RuntimeDelegate::WillDestroy(const Rtt::Runtime& sender) const
{
	// Notify the system that the runtime is about to be terminated.
	fEnvironmentPointer->fRuntimeState = RuntimeState::kTerminating;
	fEnvironmentPointer->fTerminatingEvent.Raise(*fEnvironmentPointer, EventArgs::kEmpty);
}

#pragma endregion

}	// namespace Interop
