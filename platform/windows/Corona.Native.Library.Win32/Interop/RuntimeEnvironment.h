//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Graphics\CoronaFontServices.h"
#include "Storage\MStoredPreferences.h"
#include "UI\Control.h"
#include "UI\MessageOnlyWindow.h"
#include "UI\RenderSurfaceControl.h"
#include "Event.h"
#include "EventArgs.h"
#include "PointerResult.h"
#include "Rtt_Callback.h"
#include "Rtt_MPlatform.h"
#include "Rtt_ReadOnlyProjectSettings.h"
#include "Rtt_Runtime.h"
#include "Rtt_RuntimeDelegatePlayer.h"
#include "Rtt_WinProjectSettings.h"
#include "RuntimeState.h"
#include "ScopedOleInitializer.h"
#include "WinString.h"
#include <list>
#include <Windows.h>
extern "C"
{
#	include "lua.h"
}


#pragma region Forward Declarations
namespace Interop
{
	namespace UI
	{
		class Window;
	}
	class MDeviceSimulatorServices;
}
namespace Rtt
{
	class WinPlatform;
}
struct Rtt_Allocator;

#pragma endregion


namespace Interop {

/// <summary>Provides information and services for one active Corona runtime instance.</summary>
class RuntimeEnvironment
{
	Rtt_CLASS_NO_COPIES(RuntimeEnvironment)

	public:
		#pragma region Public Event Types
		/// <summary>
		///  <para>Defines the "Loaded" event type.</para>
		///  <para>Raised after loading the "config.lua" file and just but before executing the "shell.lua" file.</para>
		///  <para>
		///   This event is not to be exposed publicly to prevent Corona developers from overriding the
		///   CoronaCards authorization handling in the "shell.lua" file.
		///  </para>
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> LoadedEvent;

		/// <summary>
		///  <para>Defines the "Starting" event type.</para>
		///  <para>Raised just but before executing the "main.lua" file.</para>
		///  <para>This is the application's opportunity to register custom APIs into Lua.</para>
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> StartingEvent;

		/// <summary>
		///  Defines the "Started" event type which is raised after the "main.lua" has been executed.
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> StartedEvent;

		/// <summary>
		///  Defines the "Suspended" event type which is raised when the Corona runtime has been suspended.
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> SuspendedEvent;

		/// <summary>
		///  Defines the "Resumed" event type which is raised when the runtime has been resumed after a suspend.
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> ResumedEvent;

		/// <summary>
		///  Defines the "EnteringFrame" event type which is raised before the Corona runtime
		///  dispatches a Lua "enterFrame" event.
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> EnteringFrameEvent;

		/// <summary>
		///  Defines the "EnteredFrame" event type which is raised after the Corona runtime
		///  dispatches a Lua "enterFrame" event.
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> EnteredFrameEvent;

		/// <summary>
		///  Defines the "Terminating" event type which is raised just before the Corona runtime gets destroyed.
		/// </summary>
		typedef Event<RuntimeEnvironment&, const EventArgs&> TerminatingEvent;

		#pragma endregion


		#pragma region CreationSettings Structure
		/// <summary>Provides settings to be passed into the static CreateUsing() function.</summary>
		struct CreationSettings
		{
			/// <summary>
			///  <para>
			///   Set true (the default) to have the CreateUsing() function create a new Rtt::Runtime object
			///   and automatically run/load a the referenced Corona project.
			///  </para>
			///  <para>
			///   Set false to not create/run a runtime, in which case the runtime environment will be flagged as
			///   terminated upon creation. Only the Rtt::Platform derived object is created and is typically
			///   used for Corona Simulator authorization purposes.
			///  </para>
			/// </summary>
			bool IsRuntimeCreationEnabled;

			/// <summary>
			///  <para>The path that Corona's Lua "system.ResourceDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			/// </summary>
			const wchar_t* ResourceDirectoryPath;

#ifdef Rtt_AUTHORING_SIMULATOR
			/// <summary>
			///  <para>The path that Corona's internal Lua "system.SystemResourceDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			///  <para>
			///   This setting is only supported by the Corona Simulator and is expected to point to
			///   the "Corona\Resources" directory.
			///  </para>
			///  <para>
			///   For desktop apps, the Lua "system.SystemResourceDirectory" will be the same as "system.ResourceDirectory".
			///  </para>
			/// </summary>
			const wchar_t* SystemResourceDirectoryPath;
#endif

			/// <summary>
			///  <para>The path that Corona's Lua "system.DocumentsDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			/// </summary>
			const wchar_t* DocumentsDirectoryPath;

			/// <summary>
			///  <para>The path that Corona's Lua "system.ApplicationSupportDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			/// </summary>
			const wchar_t* ApplicationSupportDirectoryPath;

			/// <summary>
			///  <para>The path that Corona's Lua "system.TemporaryDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			/// </summary>
			const wchar_t* TemporaryDirectoryPath;

			/// <summary>
			///  <para>The path that Corona's Lua "system.CachesDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			/// </summary>
			const wchar_t* CachesDirectoryPath;

			/// <summary>
			///  <para>The path that Corona's internal Lua "system.SystemCachesDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			/// </summary>
			const wchar_t* SystemCachesDirectoryPath;

			/// <summary>
			///  <para>The path that Corona's internal Lua "system.PluginsDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			/// </summary>
			const wchar_t* PluginsDirectoryPath;

			/// <summary>
			///  <para>The path that Corona's internal Lua "system.SkinResourceDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			///  <para>This path only applies to the Corona Simulator.</para>
			/// </summary>
			const wchar_t* SkinResourceDirectoryPath;

			/// <summary>
			///  <para>The path that Corona's internal Lua "system.UserSkinsDirectory" property will map to.</para>
			///  <para>Set to null to use the runtime environment's default path.</para>
			///  <para>This path only applies to the Corona Simulator.</para>
			/// </summary>
			const wchar_t* UserSkinsDirectoryPath;

			/// <summary>
			///  <para>Handle to the main window frame which contains the rendering surface.</para>
			///  <para>This is optional. Providing this handle allows the runtime to control the main window.</para>
			/// </summary>
			HWND MainWindowHandle;

			/// <summary>
			///  <para>Handle to a window/control's client area to render OpenGL content to.</para>
			///  <para>Can be null, which prevents Corona from rendering anything.</para>
			/// </summary>
			HWND RenderSurfaceHandle;

			/// <summary>Launch options to be used such as kConnectToDebugger, kLaunchDeviceShell, etc.</summary>
			Rtt::Runtime::LaunchOptions LaunchOptions;

			/// <summary>
			///  <para>
			///   Pointer to a collection of strings to be passed into the "main.lua" file via its launch arguments
			///   "..." table under the "args" field.
			///  </para>
			///  <para>Set this to null or to an empty collection to not pass any arguments.</para>
			/// </summary>
			const std::list<std::wstring>* LaunchArgumentsPointer;

			/// <summary>
			///  Pointer to a "Loaded" event handler which will get invoked after the "config.lua" file was loaded,
			///  but before the "main.lua" gets executed.
			/// </summary>
			RuntimeEnvironment::LoadedEvent::Handler* LoadedEventHandlerPointer;

			/// <summary>Creates a new settings object initialized to its defaults.</summary>
			CreationSettings()
			:	IsRuntimeCreationEnabled(true),
				ResourceDirectoryPath(nullptr),
#ifdef Rtt_AUTHORING_SIMULATOR
				SystemResourceDirectoryPath(nullptr),
#endif
				DocumentsDirectoryPath(nullptr),
				ApplicationSupportDirectoryPath(nullptr),
				TemporaryDirectoryPath(nullptr),
				CachesDirectoryPath(nullptr),
				SystemCachesDirectoryPath(nullptr),
				PluginsDirectoryPath(nullptr),
				SkinResourceDirectoryPath(nullptr),
				UserSkinsDirectoryPath(nullptr),
				MainWindowHandle(nullptr),
				RenderSurfaceHandle(nullptr),
				LaunchOptions(Rtt::Runtime::kDefaultLaunchOption),
				LaunchArgumentsPointer(nullptr),
				LoadedEventHandlerPointer(nullptr)
			{
			}
		};

		#pragma endregion


		#pragma region CreationResult Class
		/// <summary>
		///  <para>Type returned by the RuntimeEnvironment class' CreateUsing() static function.</para>
		///  <para>Determines if the CreateUsing() function successfully created a new RuntimeEnvironment object.</para>
		///  <para>If successful, the GetPointer() method will provide the newly created RuntimeEnvironment object.</para>
		/// </summary>
		typedef PointerResult<RuntimeEnvironment> CreationResult;

		#pragma endregion


		#pragma region CreationResult Class
		/// <summary>
		///  <para>Provides CreationResult constants used to identify particular failures.</para>
		///  <para>You cannot create instances of this class. You can only access its static members.</para>
		/// </summary>
		class CreationResults
		{
			private:
				CreationResults() = delete;
			public:
				/// <summary>
				///  Indicates that the RuntimeEnvironment::CreateUsing() function failed to create a runtime because
				///  it is set up to be single instance app and another instance already exists on the desktop.
				/// </summary>
				static const CreationResult kSingleAppInstanceAlreadyExists;
		};

		#pragma endregion


		#pragma region ValidateRenderSurfaceResult Structure
		/// <summary>Result object returned by the static RuntimeEnvironment::ValiateRenderSurface() function.</summary>
		struct ValidateRenderSurfaceResult
		{
			/// <summary>
			///  Provides the minimum rendering driver version required by Corona in order to
			///  successfully render to a control.
			/// </summary>
			UI::RenderSurfaceControl::Version MinVersionSupported;

			/// <summary>The rendering driver's version info. Version numbers be zero if CanRender is false.</summary>
			UI::RenderSurfaceControl::Version RendererVersion;

			/// <summary>
			///  <para>Set to true if Corona is able to render to the given control successfully.</para>
			///  <para>
			///   Set to false if given an invalid window handle or if the rendering driver version
			///   is older than what is required by Corona.
			///  </para>
			/// </summary>
			bool CanRender;

			/// <summary>
			///  <para>
			///   Set true if all of Corona's shaders and graphic effects are supported by the rendering surface.
			///  </para>
			///  <para>
			///   Set false if not all of Corona's shaders will work, which can happen if the rendering driver is too old.
			///  </para>
			/// </summary>
			bool SupportsAllShaders;

			/// <summary>Creates a new result object initialized to failed.</summary>
			ValidateRenderSurfaceResult()
			:	CanRender(false),
				SupportsAllShaders(false)
			{
			}
		};

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets an object used to add or remove an event handler for the "Loaded" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		LoadedEvent::HandlerManager& GetLoadedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Starting" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		StartingEvent::HandlerManager& GetStartingEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Started" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		StartedEvent::HandlerManager& GetStartedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Suspended" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		SuspendedEvent::HandlerManager& GetSuspendedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Resumed" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ResumedEvent::HandlerManager& GetResumedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "EnteringFrame" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		EnteringFrameEvent::HandlerManager& GetEnteringFrameEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "EnteredFrame" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		EnteredFrameEvent::HandlerManager& GetEnteredFrameEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Terminating" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		TerminatingEvent::HandlerManager& GetTerminatingEventHandlers();

		/// <summary>Gets Corona's memory allocator used by the Corona runtime and platform.</summary>
		/// <returns>Returns a reference to Corona's memory allocator for this runtime environment.</returns>
		Rtt_Allocator& GetAllocator() const;

		/// <summary>Gets a pointer to Corona's Win32 implementation of the Rtt::MPlatform class.</summary>
		/// <returns>Returns a pointer to Corona's Win32 implementation of the Rtt::MPlatform class.</returns>
		Rtt::WinPlatform* GetPlatform() const;

		/// <summary>
		///  Gets a pointer to Corona's internal Rtt::Runtime object used to drive the Corona application.
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to Corona's runtime object.</para>
		///  <para>Returns null if the Corona runtime has been terminated.</para>
		/// </returns>
		Rtt::Runtime* GetRuntime() const;

		/// <summary>Gets the current state of the Corona runtime such as Running, Suspended, Terminated, etc.</summary>
		/// <returns>Returns the current state of the Corona runtime.</returns>
		RuntimeState GetRuntimeState() const;

		/// <summary>
		///  <para>
		///   Gets a "message-only window" that the caller can use to send/post private Windows messages for this
		///   one Corona runtime environment.
		///  </para>
		///  <para>This is available even if Corona is not rendering to a control.</para>
		///  <para>
		///   Messages posted to this window will never be delivered to the app window, preventing message ID collision.
		///  </para>
		/// </summary>
		/// <returns>Returns a Win32 "message-only window" used to post and receive private messages.</returns>
		UI::MessageOnlyWindow& GetMessageOnlyWindow();

		/// <summary>Gets a pointer to the main window that is hosting the rendering surface.</summary>
		/// <returns>
		///  <para>Returns a pointer to the main window.</para>
		///  <para>Returns null if the runtime was not given access to the main window.</para>
		/// </returns>
		Interop::UI::Window* GetMainWindow() const;

		/// <summary>Gets a pointer to the control that the runtime is rendering to.</summary>
		/// <returns>
		///  <para>Returns a pointer to the control that the runtime is rendering to.</para>
		///  <para>Returns null if the runtime was not assigned a rendering surface.</para>
		/// </returns>
		Interop::UI::RenderSurfaceControl* GetRenderSurface() const;

		/// <summary>Gets the specified directory's path, without a trailing slash, in UTF-8 form.</summary>
		/// <param name="value">The directory type to fetch a path for such as kResourceDir, kDocumentsDir, etc.</param>
		/// <returns>
		///  <para>Returns the specified directory's path, without a trailing slash, in UTF-8 form.</para>
		///  <para>Returns null if given an invalid directory enum type such as kUnknownDir or kNumDirs.</para>
		/// </returns>
		const char* GetUtf8PathFor(Rtt::MPlatform::Directory value) const;

		/// <summary>Gets the specified directory's path, without a trailing slash, in UTF-16 form.</summary>
		/// <param name="value">The directory type to fetch a path for such as kResourceDir, kDocumentsDir, etc.</param>
		/// <returns>
		///  <para>Returns the specified directory's path, without a trailing slash, in UTF-16 form.</para>
		///  <para>Returns null if given an invalid directory enum type such as kUnknownDir or kNumDirs.</para>
		/// </returns>
		const wchar_t* GetUtf16PathFor(Rtt::MPlatform::Directory value) const;

		/// <summary>
		///  <para>Gets a UTF-16 registry path used by the Corona project.</para>
		///  <para>The returned path will not be prefixed with a registry hive name, such as HKCU or HKLM.</para>
		///  <para>The returned path will not end with a trailing slash.</para>
		/// </summary>
		/// <returns>Returns a UTF-16 registry path used by the Corona project.</returns>
		const wchar_t* GetRegistryPathWithoutHive() const;

		/// <summary>
		///  <para>Sets the path to the "Project Resource" directory.</para>
		///  <para>This is only used by the Corona Simulator's welcome screen and Corona Composer.</para>
		///  <para>
		///   This path can be fetched via the GetUtf*PathFor() method using the
		///   "Rtt::MPlatform::kProjectResourceDir" constant.
		///  </para>
		/// </summary>
		/// <param name="path">
		///  <para>Directory path in UTF-16 form.</para>
		///  <para>If set to null or empty string, then this directory will default to "kResourceDirectory".</para>
		/// </para>
		void SetPathForProjectResourceDirectory(const wchar_t* path);

		/// <summary>
		///  <para>Gets an object used to read, write, and delete application preferences to storage.</para>
		///  <para>Preferences are key-value pairs used to easily store simple data/settings for the current user.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to an object used to read, write, and delete preferences.</para>
		///  <para>Returns null if this runtime/application does not support stored preferences.</para>
		/// </returns>
		std::shared_ptr<Interop::Storage::MStoredPreferences> GetStoredPreferences() const;

		/// <summary>
		///  Gets a pointer to an optional interface providing device simulation services for the Corona Simulator.
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to a device simulation interface if running under the Corona Simulator.</para>
		///  <para>Returns null if the Corona runtime is not simulating a device.</para>
		/// </returns>
		virtual MDeviceSimulatorServices* GetDeviceSimulatorServices() const;

		/// <summary>Gets an object used to load font files and access font features.</summary>
		/// <returns>Returns an object which provides font features.</returns>
		Graphics::CoronaFontServices& GetFontServices();

		/// <summary>Gets the Corona project's loaded "build.settings" and "config.lua" information.</summary>
		/// <returns>Returns a read-only reference to the runtime's current project information.</returns>
		const Rtt::ReadOnlyProjectSettings& GetProjectSettings() const;

		/// <summary>
		///  <para>Fetches a function pointer to be called by Lua when loading a package via a require() function.</para>
		///  <para>Used to load packages/plugins that are in special directories or compiled into the Corona library.</para>
		///  <para>
		///   This function point is intended to be pushed into Lua via the Rtt::Lua::InsertPackageLoader() function
		///   on application startup.
		///  </para>
		/// </summary>
		/// <returns>Returns a function pointer to be called by a Lua require() function.</returns>
		lua_CFunction GetLuaPackageLoaderCallback() const;

		/// <summary>Suspends the runtime, if currently running. Does nothing if terminated.</summary>
		void RequestSuspend();

		/// <summary>Resumes the runtime, if currently suspended. Does nothing if terminated.</summary>
		void RequestResume();

		/// <summary>
		///  <para>Exits and destroys the runtime, if not done already.</para>
		///  <para>The GetRuntime() method will return null after termination.</para>
		/// </summary>
		void Terminate();

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>
		///  <para>Creates a new Corona runtime environment using the given creation/project settings.</para>
		///  <para>
		///   The Corona runtime will start running immediately upon return, provided that the
		///   "IsRuntimeCreationEnabled" setting is set to true.
		///  </para>
		///  <para>
		///   The environment's "Loaded" event will be raised before this function returns,
		///   provided that the project was successfully loaded.
		///  </para>
		///  <para>
		///   To delete the returned runtime environment object, you must pass it into this class' static Destroy() function.
		///  </para>
		/// </summary>
		/// <param name="settings">
		///  <para>Provides Corona project settings such as directory paths, the window to render to, etc.</para>
		///  <para>
		///   If the "IsRuntimeCreationEnabled" field is set false, then only an Rtt::Platform object will be
		///   created and a Corona project will never be loaded/started.
		///  </para>
		/// </param>
		/// <returns>
		///  <para>
		///   Returns a success result and a pointer to a new runtime environment object if creation was
		///   successful and was able to load the given Corona project if applicable.
		///  </para>
		///  <para>
		///   Returns a failure result if creation or project loading failed. The result's GetMessage() method
		///   will provide details as to what went wrong.
		///  </para>
		/// </returns>
		static RuntimeEnvironment::CreationResult CreateUsing(const RuntimeEnvironment::CreationSettings& settings);

		/// <summary>
		///  <para>
		///   Destroys the runtime environment object that was returned by this class' static CreateUsing() function.
		///  </para>
		///  <para>This will automatically terminate the runtime if it is currently running.</para>
		/// </summary>
		/// <param name="environmentPointer">
		///  <para>Pointer to the runtime enivornment to be deleted/destroyed.</para>
		///  <para>A null pointer will be safely ignored.</para>
		/// </para>
		static void Destroy(RuntimeEnvironment* environmentPointer);

		/// <summary>
		///  <para>Determines if Corona is able to render to the given window/control successfully.</para>
		///  <para>Checks if the video hardware and its OpenGL driver meets Corona's minimum requirements.</para>
		/// </summary>
		/// <param name="windowHandle">Handle to the window or control to be validated.</param>
		/// <returns>Returns a result object determining if Corona can render to the given window successfully.</returns>
		static ValidateRenderSurfaceResult ValidateRenderSurface(HWND windowHandle);

		/// <summary>
		///  <para>
		///   Determines if there are no Corona apps currently connected to a debugger on the system, such as to
		///   the "Corona Debugger" or Sublime "Corona Editor.
		///  </para>
		///  <para>
		///   Note that Corona debugging uses a fixed TCP port number, which means that only one Corona application
		///   on the system can connect to a debugger at a time.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if no Corona application on the system is currently connected to a debugger.</para>
		///  <para>Returns false if there is a Corona application connected to a debugger.</para>
		/// </returns>
		static bool IsDebuggerConnectionAvailable();

		#pragma endregion

	protected:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new Corona runtime environment with the given settings.</summary>
		/// <param name="settings">
		///  <para>Provides launch settings and interop layer settings to be copied to the new environment object.</para>
		///  <para>Will throw an exception if any of the settings are invalid.</para>
		/// </param>
		RuntimeEnvironment(const RuntimeEnvironment::CreationSettings& settings);

		/// <summary>Terminates the runtime, if running, and deletes resources consumed by the Corona environment.</summary>
		virtual ~RuntimeEnvironment();

		#pragma endregion


		#pragma region Protected Methods
		/// <summary>
		///  <para>Creates an Rtt::Runtime and then loads/starts the Corona project.</para>
		///  <para>This is a protected method that is only expected to be called by the CreateUsing() functions.</para>
		///  <para>
		///   Will throw an exception if an Rtt::Runtime was already created or if the given settings flag
		///   that a runtime should not be created.
		///  </para>
		/// </summary>
		/// <param name="settings">The creation settings used to create the runtime environment object.</param>
		/// <returns>
		///  <para>Returns a success result if successfully created the Rtt::Runtime and loaded/started the project.</para>
		///  <para>Returns a failure result if unable to load/start the project with a message detailing why.</para>
		/// </returns>
		OperationResult RunUsing(const RuntimeEnvironment::CreationSettings& settings);

		/// <summary>
		///  <para>
		///   Generates a unique semaphore name to be used by single instance Win32 desktop applications
		///   and copies the result to the "destinationString" argument.
		///  </para>
		///  <para>This name is expected to be passed to the Win32 CreateSemaphore() function.</para>
		///  <para>For simulated apps, this name is based on the given Corona project directory.</para>
		///  <para>
		///   For Win32 desktop apps, this name is based on the company name and product named assigned to the EXE file.
		///  </para>
		/// </summary>
		/// <param name="resourceDirectoryPath">Path to the Corona project directory.</param>
		/// <param name="destinationString">The string that the semaphore name will be copied to.</param>
		/// <returns>
		///  <para>Returns true if this function copied the semaphore name to argument "destinationString".</para>
		///  <para>Returns false if failed to generate a semaphore name.</para>
		/// </returns>
		static bool GenerateSingleWindowInstanceSemaphoreName(
				const wchar_t* resourceDirectoryPath, std::wstring& destinationString);

		/// <summary>
		///  <para>
		///   Generates a class name to be registered for a Win32 message-only window to be used for IPC
		///   (Inter-Process Communications) and copies the results to the given "className" argument.
		///  </para>
		///  <para>For simulated apps, this class name is based on the given Corona project/resource directory.</para>
		///  <para>
		///   For Win32 desktop apps, this is based on the company name and product named assigned to the EXE file.
		///  </para>
		/// </summary>
		/// <param name="resourceDirectoryPath">Path to the Corona project directory.</param>
		/// <param name="destinationString">The string that the class name will be copied to.</param>
		/// <returns>
		///  <para>Returns true if this function copied the class name to argument "destinationString".</para>
		///  <para>Returns false if failed to generate a class name.</para>
		/// </returns>
		static bool GenerateIpcMessageOnlyWindowClassName(
				const wchar_t* resourceDirectoryPath, std::wstring& destinationString);

		/// <summary>
		///  Generates a Corona Simulator sandbox directory path based on the given Corona project resource directory
		///  and copies the result to the given destination string.
		/// </summary>
		/// <remarks>
		///  <para>This function is expected to be called by the Corona Simulator and the Corona Shell.</para>
		///  <para>This allows these 2 applications to use consistent sandbox directories for the same project.</para>
		/// </remarks>
		/// <param name="resourceDirectoryPath">Path to the Corona project directory. Cannot be null or empty.</param>
		/// <param name="destinationString">The string that the sandbox directory path will be copied to.</param>
		/// <returns>
		///  <para>Returns true if this function copied the sandbox path to argument "destinationString".</para>
		///  <para>Returns false if given an invalid "resourceDirectoryPath" argument.</para>
		/// </returns>
		static bool GenerateSimulatorSandboxPath(const wchar_t* resourceDirectoryPath, std::wstring& destinationString);

		/// <summary>
		///  <para>Fetches the Corona Simulator's root directoy path under the hidden "AppData" directory folder.</para>
		///  <para>This is where the simulator stores sandboxed project files, plugins, and user defined skins.</para>
		/// </summary>
		/// <param name="path">The string that the simulator's directoy path will be copied to.</param>
		/// <returns>
		///  <para>Returns true if the directory path was successfully fetched and copied to the given argument.</para>
		///  <para>Returns false if the directory path was not copied to the given argument.</para>
		/// </returns>
		static bool CopySimulatorRootAppDataDirectoryPathTo(std::wstring& path);

		/// <summary>Fetches the Corona Simulator's plugin directory path to the given argument.</summary>
		/// <param name="path">The string that the simulator's plugin directoy path will be copied to.</param>
		/// <returns>
		///  <para>Returns true if the directory path was successfully fetched and copied to the given argument.</para>
		///  <para>Returns false if the directory path was not copied to the given argument.</para>
		/// </returns>
		static bool CopySimulatorPluginDirectoryPathTo(std::wstring& path);

		#pragma endregion

	private:
		#pragma region RuntimeDelegate Class
		/// <summary>
		///  <para>Delegate used to receive events from the Corona Runtime.</para>
		///  <para>This is done by assigning an instance of this class to the Runtime.SetDelegate() function.</para>
		/// </summary>
		class RuntimeDelegate : public Rtt::RuntimeDelegatePlayer
		{
			public:
				/// <summary>Creates a new delegate used to receive events from the Corona runtime.</summary>
				/// <param name="environment">
				///  <para>Reference to the Corona runtime environment.</para>
				///  <para>Cannot be null or else an exception will be thrown.</para>
				/// </param>
				RuntimeDelegate(RuntimeEnvironment* environmentPointer);

				/// <summary>Called just after all core Lua libraries have been loaded into the runtime's Lua state.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void DidInitLuaLibraries(const Rtt::Runtime& sender) const;

				/// <summary>
				///  <para>Called when the "config.lua" file's table has been pushed to the top of the Lua stack.</para>
				///  <para>This is the application's opportunity to modify these setting before Corona processes them.</para>
				/// </summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				/// <param name="L">The Lua state that the "config.lua" settings is currently loaded into.</param>
				virtual void InitializeConfig(const Rtt::Runtime& sender, lua_State *L) const;

				/// <summary>
				///  Called just after the "config.lua" has been loaded but before the "shell.lua" has been executed.
				/// </summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				/// <param name="L">The Lua state that the "config.lua" settings is currently loaded into.</param>
				virtual void DidLoadConfig(const Rtt::Runtime& sender, lua_State *L) const;

				/// <summary>Determines if the Corona library is licensed and authorized to continue.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				/// <returns>Returns true if the Corona library is licensed/authorized. Returns false if not.</returns>
				virtual bool HasDependencies(const Rtt::Runtime& sender) const;

				/// <summary>
				///  <para>Called just before the "main.lua" get executed.</para>
				///  <para>This is the application's opportunity to register custom APIs into Lua.</para>
				/// </summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void WillLoadMain(const Rtt::Runtime& sender) const;

				/// <summary>Called after the "main.lua" file has been loaded and executed.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void DidLoadMain(const Rtt::Runtime& sender) const;

				/// <summary>Called just before the runtime is about to suspend itself.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void WillSuspend(const Rtt::Runtime& sender) const;

				/// <summary>Called just after the runtime has been suspended.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void DidSuspend(const Rtt::Runtime& sender) const;

				/// <summary>Called just before the runtime resumes itself.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void WillResume(const Rtt::Runtime& sender) const;

				/// <summary>Called just after the runtime has been resumed.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void DidResume(const Rtt::Runtime& sender) const;

				/// <summary>Called just before the runtime is about to be terminated via its destructor.</summary>
				/// <param name="sender">The Corona runtime that is raising this event.</param>
				virtual void WillDestroy(const Rtt::Runtime& sender) const;

			private:
				/// <summary>The Corona runtime instance this delegate is assigned to.</summary>
				RuntimeEnvironment* fEnvironmentPointer;
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>Suspends the runtime, if currently running. Does nothing if terminated.</summary>
		void Suspend();

		/// <summary>Resumes the runtime, if currently suspended. Does nothing if terminated.</summary>
		void Resume();

		/// <summary>
		///  <para>Called when the native Corona runtime's timer has elapsed.</para>
		///  <para>Updates the Corona runtime for the next "enterFrame" and requests the surface to render a frame.</para>
		/// </summary>
		void OnRuntimeTimerElapsed();

		/// <summary>
		///  Called when the main window that is hosting the runtime's rendering surface has received a Windows message.
		/// </summary>
		/// <param name="sender">Reference to the window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnMainWindowReceivedMessage(UI::UIComponent &sender, UI::HandleMessageEventArgs &arguments);

		/// <summary>
		///  Called when the IPC (Inter-Process Communications) message-only window has received a message.
		/// </summary>
		/// <param name="sender">Reference to the message-only window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnIpcWindowReceivedMessage(UI::UIComponent &sender, UI::HandleMessageEventArgs &arguments);

		/// <summary>Called when the surface is requesting the runtime to render the a frame.</summary>
		/// <param name="sender">The rendering surface requesting a frame.</param>
		/// <param name="arguments">
		///  <para>Provides a SetHandled() method that must be called if the runtime successfully rendered the scene.</para>
		///  <para>If SetHandled() is not called, then the surface will draw a black screen until the runtime is ready.</para>
		/// </param>
		void OnRenderFrame(UI::RenderSurfaceControl &sender, HandledEventArgs &arguments);

		/// <summary>Called when the rendering surface control/window is about to be destroyed.</summary>
		/// <param name="sender">The rendering surface's control/window that is about to be destroyed.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnDestroyingSurface(UI::UIComponent &sender, const EventArgs &arguments);

		/// <summary>Called when the rendering surface control/window has been resized.</summary>
		/// <param name="sender">The rendering surface's control/window that has been resized.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnSurfaceResized(UI::Control &sender, const EventArgs &arguments);

		/// <summary>
		///  <para>Detects if a resize or orientation change has occurred</para>
		///  <para>If occurred, then this method updates the Corona display and raises events in Lua.</para>
		/// </summary>
		void UpdateOrientationAndSurfaceSize();

		/// <summary>
		///  <para>
		///   Updates the styles, window mode, and size of the main window that is hosting the rendering surface
		///   according to the given project settings.
		///  </para>
		///  <para>
		///   Uses member variable "fMainWindowPointer" to access the window. If set null, then this method will no-op.
		///  </para>
		/// </summary>
		/// <param name="projectSettings">Provides the "build.settings" configuration used to modify the window.</param>
		void UpdateMainWindowUsing(const Rtt::ReadOnlyProjectSettings& projectSettings);

		/// <summary>
		///  <para>Called by a Lua require() function.</para>
		///  <para>Loads a plugin that may be in a special directory or compiled into this Corona library.</para>
		/// </summary>
		/// <param name="luaStatePointer">
		///  <para>Pointer to the Lua state that is attempting to load a required-in package.</para>
		///  <para>The first object on the Lua stack is expected to be the name of the package to be loaded.</para>
		/// </param>
		/// <returns>Returns a value of 1 indicating that this function is returning 1 object back to Lua.</returns>
		static int OnLuaLoadPackage(lua_State* luaStatePointer);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Manages the "Loaded" event.</summary>
		LoadedEvent fLoadedEvent;

		/// <summary>Manages the "Starting" event.</summary>
		StartingEvent fStartingEvent;

		/// <summary>Manages the "Started" event.</summary>
		StartedEvent fStartedEvent;

		/// <summary>Manages the "Suspended" event.</summary>
		SuspendedEvent fSuspendedEvent;

		/// <summary>Manages the "Resumed" event.</summary>
		ResumedEvent fResumedEvent;

		/// <summary>Manages the "EnteringFrame" event.</summary>
		EnteringFrameEvent fEnteringFrameEvent;

		/// <summary>Manages the "EnteredFrame" event.</summary>
		EnteredFrameEvent fEnteredFrameEvent;

		/// <summary>Manages the "Terminating" event.</summary>
		TerminatingEvent fTerminatingEvent;

		/// <summary>Corona's memory allocator used by the Corona runtime and MPlaform objects.</summary>
		Rtt_Allocator* fAllocatorPointer;

		/// <summary>Pointer to Corona's native C++ Win32 platform implementation.</summary>
		Rtt::WinPlatform* fPlatformPointer;

		/// <summary>Pointer to Corona's native C++ runtime which drives the Corona application.</summary>
		Rtt::Runtime* fRuntimePointer;

		/// <summary>Indicates the current state of the runtime such as Starting, Running, Suspended, etc.</summary>
		RuntimeState fRuntimeState;

		/// <summary>Delegate given to the native "fRuntimePointer" object handle its events.</summary>
		RuntimeEnvironment::RuntimeDelegate fRuntimeDelegate;

		/// <summary>
		///  <para>
		///   Callback given to the native "fRuntimePointer" which gets invoked every time the runtime's main timer elapses.
		///  </para>
		///  <para>Used to "update" the Corona runtime for the next frame and to request rendering.</para>
		/// </summary>
		Rtt::MethodCallback<RuntimeEnvironment> fRuntimeTimerElapsedCallback;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised by the main window.</summary>
		UI::UIComponent::ReceivedMessageEvent::MethodHandler<RuntimeEnvironment> fMainWindowReceivedMessageEventHandler;

		/// <summary>
		///  Handler to be invoked when the "ReceivedMessage" event has been raised by the IPC message-only window.
		/// </summary>
		UI::UIComponent::ReceivedMessageEvent::MethodHandler<RuntimeEnvironment> fIpcWindowReceivedMessageEventHandler;

		/// <summary>Handler to be invoked when the "RenderFrame" event has been raised by the rendering surface.</summary>
		UI::RenderSurfaceControl::RenderFrameEvent::MethodHandler<RuntimeEnvironment> fRenderFrameEventHandler;

		/// <summary>Handler to be invoked when the "Destroying" event has been raised by the rendering surface.</summary>
		UI::UIComponent::DestroyingEvent::MethodHandler<RuntimeEnvironment> fDestroyingSurfaceEventHandler;

		/// <summary>Handler to be invoked when the "Resized" event has been raised by the rendering surface.</summary>
		UI::RenderSurfaceControl::ResizedEvent::MethodHandler<RuntimeEnvironment> fSurfaceResizedEventHandler;

		/// <summary>
		///  <para>Set true if RuntimeEnvironment::RequestSuspend() was called. Set false if not.</para>
		///  <para>
		///   This is needed so to respect the Corona developer's call to the RequestSuspend() method so that
		///   the runtime environment won't automatically resume when returning from a minimized window,
		///   if the window visibility changed from hidden to shown, etc.
		///  </para>
		/// </summary>
		bool fWasSuspendRequestedExternally;

		/// <summary>Stores "build.settings" and "config.lua" information.</summary>
		Rtt::WinProjectSettings fProjectSettings;

		/// <summary>Read-only container which wraps the "fProjectSettings" member variable.</summary>
		Rtt::ReadOnlyProjectSettings fReadOnlyProjectSettings;

		/// <summary>
		///  <para>A Win32 "message-only window" used to post private messages within the Corona runtime environment.</para>
		///  <para>This allows Corona to use a Windows message pump even when it is not rendering to a control/window.</para>
		///  <para>Also prevents Corona's custom messages from reaching the app window, avoiding message ID collision.</para>
		/// </summary>
		UI::MessageOnlyWindow* fMainMessageOnlyWindowPointer;

		/// <summary>
		///  <para>A Win32 "message-only window" used for IPC (Inter-Process Communications).</para>
		///  <para>
		///   Intended for other apps to post messages to this app's message-only window via WM_COPYDATA messages.
		///  </para>
		/// </summary>
		UI::MessageOnlyWindow* fIpcMessageOnlyWindowPointer;

		/// <summary>
		///  <para>Pointer to the window that is hosting the Corona runtime.</para>
		///  <para>Note that this can point to the same Windows handle as "fRenderSurfacePointer".</para>
		///  <para>Set null if the runtime was not given access to the window, meaning Corona cannot control it.</para>
		/// </summary>
		Interop::UI::Window* fMainWindowPointer;

		/// <summary>
		///  <para>Pointer to a Windows control that this runtime is rendering to.</para>
		///  <para>Set null if the runtime was not assigned a rendering surface.</para>
		/// </summar>
		Interop::UI::RenderSurfaceControl* fRenderSurfacePointer;

		/// <summary>
		///  Array storing paths of all sandboxed dirctories supported by Corona such as
		///  kResourceDir, kDcoumentsDir, etc.
		/// </summary>
		WinString fDirectoryPaths[Rtt::MPlatform::kNumDirs];

		/// <summary>Registry path used by the Corona project without the hive name prefix.</summary>
		std::wstring fRegistryPathWithoutHive;

		/// <summary>Pointer to an object used to read, write, and delete stored preferences for this runtime.</summary>
		std::shared_ptr<Interop::Storage::MStoredPreferences> fStoredPreferencesPointer;

		/// <summary>
		///  <para>
		///   Collection of strings that are passed into "main.lua" via the launch arguments "..." table,
		///   under the "args" field.
		///  </para>
		///  <para>These strings are assigned via the RuntimeEnvironment::CreationSettings::LaunchArguments field.</para>
		/// </summary>
		std::list<WinString> fLaunchArguments;

		/// <summary>The last orientation reported to the Corona runtime while running.</summary>
		Rtt::DeviceOrientation::Type fLastOrientation;

		/// <summary>
		///  <para>Initializes Microsoft OLE/COM upon construction and unitializes OLE/COM upon destruction.</para>
		///  <para>OLE is needed to support ActiveX controls, drag & drop, and clipboard copy/paste.</para>
		///  <para>Note: The Internet Explorer WebBrowser control is an ActiveX control which needs OLE support.</para>
		/// </summary>
		ScopedOleInitializer fScopedOleInitializer;

		/// <summary>
		///  <para>Microsoft GDI+ token received from the GdiplusStartup() function.</para>
		///  <para>Must be passed into the GdiplusShutdown() function when the runtime is terminated.</para>
		///  <para>Will be zero if GDI+ has not be started yet or it has already been shutdown.</para>
		///  <para>Note: GDI+ is needed by Corona to load images and render text to bitmaps.</para>
		/// </summary>
		ULONG_PTR fGdiPlusToken;

		/// <summary>Provides font loading and accessing features.</summary>
		Graphics::CoronaFontServices fFontServices;

		/// <summary>
		///  <para>
		///   Handle to a "named semaphore" used to indicate that this runtime is connected to a debugger such
		///   as the "Corona Debugger" or Sublime "Corona Editor".
		///  </para>
		///  <para>Set to null if the Corona runtime is not set up to connect to a debugger.</para>
		/// </summary>
		/// <remarks>
		///  Corona debugging uses a fixed TCP port number which only allows one Corona app on a system to be debugged
		///  at a time. This named semaphore is used to detect if another Corona app is currently connected to the
		///  debugger. If this named semaphore is not found on the system, then TCP debugging is available.
		/// </remarks>
		HANDLE fDebuggerSemaphoreHandle;

		/// <summary>
		///  <para>
		///   Handle to a "named semaphore" used to indicate that this runtime is set up to to be single instance.
		///   Once this handle has been claimed, no other instance with the same name can claim it on the desktop.
		///  </para>
		///  <para>
		///   Set to null if the Corona runtime is not set up for single instance support or is unable to
		///   if another instance already exists.
		///  </para>
		/// </summary>
		HANDLE fSingleWindowInstanceSemaphoreHandle;

		bool fLastActivatedSent;

		#pragma endregion
};

}	// namespace Interop
