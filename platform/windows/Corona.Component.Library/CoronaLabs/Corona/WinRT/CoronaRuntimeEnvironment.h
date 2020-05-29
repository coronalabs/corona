//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "CoronaLuaEventHandler.h"
#include "CoronaRuntimeState.h"
#include "Rtt_WinRTCallback.h"
#include <collection.h>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Rtt_DeviceOrientation.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	namespace Interop
	{
		namespace Graphics
		{
			interface class IRenderSurface;
		}
		namespace Input
		{
			ref class KeyEventArgs;
			ref class TapEventArgs;
			ref class TouchEventArgs;
			ref class TouchTrackerCollection;
		}
		namespace UI
		{
			interface class ICoronaControlAdapter;
			interface class IPage;
			ref class PageOrientationEventArgs;
		}
		ref class CoronaInteropSettings;
		ref class InteropServices;
	}
	ref class CoronaLuaEventArgs;
	ref class CoronaRuntimeLaunchSettings;
	ref class DispatchCoronaLuaEventResult;
} } }
namespace Rtt
{
	class Runtime;
	class WinRTPlatform;
	class WinRTRuntimeDelegate;
}

#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>Provides information and services for one active Corona runtime instance.</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class CoronaRuntimeEnvironment sealed
{
	private:
		#pragma region Private Destructors
		/// <summary>Deletes/releases the resources consumed by the runtime environment.</summary>
		~CoronaRuntimeEnvironment();

		#pragma endregion

	internal:
		#pragma region Internal CreationSettings Structure
		/// <summary>Provides settings to be passed into the CoronaRuntimeEnvironment class' constructor.</summary>
		struct CreationSettings
		{
			CoronaRuntimeLaunchSettings^ LaunchSettings;
			Interop::CoronaInteropSettings^ InteropSettings;
			Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ LoadedEventHandler;
			Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ StartedEventHandler;
			Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ SuspendedEventHandler;
			Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ ResumedEventHandler;
			Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ TerminatingEventHandler;
		};

		#pragma endregion


		#pragma region Internal Constructors
		/// <summary>Creates a new Corona runtime environment with the given settings.</summary>
		/// <remarks>This constructor is internal and is only intended to be invoked by the CoronaRuntime class.</remarks>
		/// <param name="settings">
		///  <para>Provides launch settings and interop layer settings to be copied to the new environment object.</para>
		///  <para>Will throw an exception if any of the settings are invalid.</para>
		/// </param>
		CoronaRuntimeEnvironment(const CoronaRuntimeEnvironment::CreationSettings &settings);

		#pragma endregion


		#pragma region Internal Methods/Properties
		/// <summary>
		///  Gets a native C++ pointer to Corona's WinRTPlatform object, which provides some WinRT
		///  feature implementations to Lua.
		/// </summary>
		/// <remarks>This is an internal property that is only accessible to Corona Labs.</remarks>
		/// <value>
		///  <para>Native C++ pointer to Corona's internal WinRTPlatform object.</para>
		///  <para>Will only be null when this property is accessed in the WinRTPlatform class' constructor.</para>
		/// </value>
		property Rtt::WinRTPlatform* NativePlatformPointer { Rtt::WinRTPlatform* get(); }

		/// <summary>
		///  Gets a native C++ pointer to Corona's internal Rtt::Runtime object used to drive the Corona application.
		/// </summary>
		/// <remarks>This is an internal property that is only accessible to Corona Labs.</remarks>
		/// <value>
		///  <para>Native C++ pointer to Corona's internal WinRTPlatform object.</para>
		///  <para>Returns null when the Corona runtime has been terminated.</para>
		/// </value>
		property Rtt::Runtime* NativeRuntimePointer { Rtt::Runtime* get(); }

		/// <summary>Suspends the runtime, if currently running. Does nothing if terminated.</summary>
		void RequestSuspend();

		/// <summary>Resumes the runtime, if currently suspended. Does nothing if terminated.</summary>
		void RequestResume();

		/// <summary>
		///  <para>Exits and destroys the runtime, if not done already.</para>
		///  <para>The CoronaRuntimeEnvironment.NativeRuntimePointer property will return null after termination.</para>
		/// </summary>
		void Terminate();

		#pragma endregion


		#pragma region Internal Events
		/// <summary>
		///  <para>Raised after loading the "config.lua" file and just but before executing the "main.lua" file.</para>
		///  <para>This is the application's opportunity to register custom APIs into Lua.</para>
		/// </summary>
		event Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ Loaded;

		/// <summary>Raised just after the "main.lua" file has been executed by the Corona runtime.</summary>
		event Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ Started;

		#pragma endregion

	public:
		#pragma region Public Events
		/// <summary>
		///  <para>
		///   Raised when the Corona runtime has been suspended which pauses all rendering, audio, timers
		///   and other Corona related operations.
		///  </para>
		///  <para>
		///   This is typically raised when the end-user navigates to another app or when the power button has been pressed.
		///  </para>
		/// </summary>
		event Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ Suspended;

		/// <summary>Raised when the Corona runtime has been resumed after a suspend.</summary>
		event Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ Resumed;

		/// <summary>
		///  <para>Raised just before the Corona runtime environment is about to be terminated.</para>
		///  <para>
		///   This typically happens when the end-user backs out of the app, the Corona XAML control has been unloaded
		///   from the page, or when the runtime's Terminate() method has been called.
		///  </para>
		/// </summary>
		event Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ Terminating;

		#pragma endregion


		#pragma region Public Methods/Properties
		/// <summary>Gets the path that Corona's "system.ResourceDirectory" property maps to in Lua.</summary>
		/// <value>The path to Corona's resource directory.</value>
		property Platform::String^ ResourceDirectoryPath { Platform::String^ get(); }

		/// <summary>Gets the path that Corona's "system.DocumentsDirectory" property maps to in Lua.</summary>
		/// <value>The path to Corona's documents directory.</value>
		property Platform::String^ DocumentsDirectoryPath { Platform::String^ get(); }

		/// <summary>
		///  <para>Gets the path that Corona's "system.TemporaryDirectory" property maps to in Lua.</para>
		///  <para>This directory's files will be automatically deleted if the system is running low on storage space.</para>
		/// </summary>
		/// <value>The path to Corona's temporary directory.</value>
		property Platform::String^ TemporaryDirectoryPath { Platform::String^ get(); }

		/// <summary>
		///  <para>Gets the path that Corona's "system.CachesDirectory" property maps to in Lua.</para>
		///  <para>This directory's files will be automatically deleted if the system is running low on storage space.</para>
		/// </summary>
		/// <value>The path to Corona's caches directory.</value>
		property Platform::String^ CachesDirectoryPath { Platform::String^ get(); }

		/// <summary>
		///  <para>Gets the directory path that Corona uses to store files for its internal features, such as analytics.</para>
		///  <para>This directory is not made available in Lua.</para>
		/// </summary>
		/// <value>Directory path used by Corona to store files for its own internal usage, such as analytics.</value>
		property Platform::String^ InternalDirectoryPath { Platform::String^ get(); }

		/// <summary>
		///  <para>Gets the path to the file that the Corona runtime began execution on.</para>
		///  <para>Typically set to a "resource.car" "main.lua" or "resource.car" file.</para>
		/// </summary>
		/// <value>
		///  <para>Path to the file that the Corona runtime began execution on.</para>
		///  <para>
		///   This is typically set to a "resource.car" or "main.lua" file unless launch settings were provided
		///   to the Corona runtime to launch a different file.
		///  </para>
		/// </value>
		property Platform::String^ LaunchFilePath { Platform::String^ get(); }

		/// <summary>
		///  <para>Provides cross-platform and cross-language access to features to the C/C++ side of Corona.</para>
		///  <para>This property is not intended for public use.</para>
		/// </summary>
		/// <value>
		///  Interop layer providing access to features between languages such as C++/CX and .NET.
		///  Also provides access to different feature implementation between the Windows Phone 8.0
		///  and Windows Store Universal app platforms.
		/// </value>
		property Interop::InteropServices^ InteropServices { Interop::InteropServices^ get(); }

		/// <summary>
		///  Gets the control that hosts the rendering surface and child controls created via Lua's native APIs.
		/// </summary>
		/// <value>
		///  <para>Gets a reference to the Corona control.</para>
		///  <para>Set to null if the Corona runtime is not set up to render to a control.</para>
		/// </value>
		property Interop::UI::ICoronaControlAdapter^ CoronaControlAdapter { Interop::UI::ICoronaControlAdapter^ get(); }

		/// <summary>Gets the current state of the Corona runtime such as Running, Suspended, Terminated, etc.</summary>
		/// <value>The current state of the Corona runtime.</value>
		property CoronaRuntimeState RuntimeState{ CoronaRuntimeState get(); }

		/// <summary>
		///  <para>Dispatches the given Corona event to Lua to be received by a Lua function.</para>
		///  <para>Lua functions can subscribe to the given event via the Lua Runtime:addEventListener() function.</para>
		/// </summary>
		/// <param name="eventArgs">
		///  <para>The Corona event to be dispatched to Lua. Its properties will be passed into Lua as an "event" table.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>Returns the result of this Corona event dispatch operation.</para>
		///  <para>
		///   Will return a success result if the given Corona event was dispatched to Lua. The result object will also
		///   provide the value returned by a Lua event listener function (if any) via its "ReturnedValue" property.
		///   Note that a success result does not mean that there were any Lua event listeners subscribed to the given
		///   event via the Lua Runtime:addEventListener() function.
		///  </para>
		///  <para>
		///   Will return a failure result if the Corona runtime has not been started up yet or if the runtime has
		///   been terminated. This means that there is no Lua state object available to push the event into.
		///  </para>
		/// </returns>
		DispatchCoronaLuaEventResult^ DispatchEvent(CoronaLuaEventArgs^ eventArgs);

		/// <summary>
		///  <para>Subscribes an event handler to be invoked when the given Corona event name gets dispatched.</para>
		///  <para>
		///   It is okay to call this method before the runtime has started. Once started, the runtime will
		///   automatically subscribe the given handler to the event.
		///  </para>
		/// </summary>
		/// <param name="eventName">
		///  <para>The name of the event to subscribe to such as "system", "enterFrame", etc.</para>
		///  <para>
		///   You can also specify your own custom event name that you can dispatch in Lua via Runtime:dispatchEvent()
		///   or via this object's <see cref="DispatchEvent()"/> method.
		///  </para>
		///  <para>The given event name cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <param name="eventHandler">
		///  <para>The handler to be invoked when the Corona event has been dispatched/raised.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		void AddEventListener(Platform::String^ eventName, CoronaLuaEventHandler^ eventHandler);

		/// <summary>
		///  <para>Unsubscribes the given event handler from the specified Corona event.</para>
		///  <para>
		///   This method is expected to be called with the same event name and handler arguments passed into the
		///   <see cref="AddEventListener()"/> method.
		///  </para>
		/// </summary>
		/// <param name="eventName">
		///  <para>The name of the Corona event to unsubscribe from such as "system", "enterFrame", etc.</para>
		///  <para>The given event name cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <param name="eventHandler">
		///  <para>Reference to the handler that was previously given to the <see cref="AddEventListener()"/> method.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>Returns true if the event handler was successfully unsubscribed from the Corona event.</para>
		///  <para>
		///   Returns false if the given event handler reference was not previously subscribed to the given event name.
		///  </para>
		/// </returns>
		bool RemoveEventListener(Platform::String^ eventName, CoronaLuaEventHandler^ eventHandler);

		#pragma endregion


		#pragma region Public Static Functions/Properties
		/// <summary>Gets the default path that Corona will use for the "system.ResourceDirectory" property in Lua.</summary>
		/// <remarks>
		///  Note that if you are a Corona Cards developer, then you can change this directory via launch settings
		///  provided to the CoronaRuntime.Run() method.
		/// </remarks>
		/// <value>
		///  On Windows Phone, this path defaults to ".\Assets\Corona" under the package's "InstalledLocation" directory.
		/// </value>
		static property Platform::String^ DefaultResourceDirectoryPath { Platform::String^ get(); }

		/// <summary>Gets the default path that Corona will use for the "system.DocumentsDirectory" property in Lua.</summary>
		/// <remarks>
		///  Note that if you are a Corona Cards developer, then you can change this directory via launch settings
		///  provided to the CoronaRuntime.Run() method.
		/// </remarks>
		/// <value>
		///  On Windows Phone, this path defaults to ".\Corona\Documents" under the application's "LocalFolder" directory.
		/// </value>
		static property Platform::String^ DefaultDocumentsDirectoryPath { Platform::String^ get(); }

		/// <summary>
		///  <para>Gets the default path that Corona will use for the "system.TemporaryDirectory" property in Lua.</para>
		///  <para>This directory's files will be automatically deleted if the system is running low on storage space.</para>
		/// </summary>
		/// <remarks>
		///  Note that if you are a Corona Cards developer, then you can change this directory via launch settings
		///  provided to the CoronaRuntime.Run() method.
		/// </remarks>
		/// <value>
		///  On Windows Phone, this path defaults to ".\Corona\TemporaryFiles" under the application's "LocalFolder" directory.
		/// </value>
		static property Platform::String^ DefaultTemporaryDirectoryPath { Platform::String^ get(); }

		/// <summary>
		///  <para>Gets the default path that Corona will use for the "system.CachesDirectory" property in Lua.</para>
		///  <para>This directory's files will be automatically deleted if the system is running low on storage space.</para>
		/// </summary>
		/// <remarks>
		///  Note that if you are a Corona Cards developer, then you can change this directory via launch settings
		///  provided to the CoronaRuntime.Run() method.
		/// </remarks>
		/// <value>
		///  On Windows Phone, this path defaults to ".\Corona\CachedFiles" under the application's "LocalFolder" directory.
		/// </value>
		static property Platform::String^ DefaultCachesDirectoryPath { Platform::String^ get(); }

		/// <summary>
		///  <para>Gets the default path that Corona will use to store files for its internal features, such as analytics.</para>
		///  <para>This directory is not made available in Lua.</para>
		/// </summary>
		/// <remarks>
		///  Note that if you are a Corona Cards developer, then you can change this directory via launch settings
		///  provided to the CoronaRuntime.Run() method.
		/// </remarks>
		/// <value>
		///  On Windows Phone, this path defaults to ".\Corona\.system" under the application's "LocalFolder" directory.
		/// </value>
		static property Platform::String^ DefaultInternalDirectoryPath { Platform::String^ get(); }

		#pragma endregion

	private:
		#pragma region Private NativeEventHandler Class
		/// <summary>Class used by the CoronaRuntimeEnvironment to receive native C/C++ event.</summary>
		class NativeEventHandler
		{
			public:
				/// <summary>Creates a native event handler that does nothing.</summary>
				NativeEventHandler();

				/// <summary>Creates a native event handler owned by the given CoronaRuntimeEnvironment object.</summary>
				/// <param name="environment">
				///  <para>The Corona runtime environment that the native event handler will pass events to.</para>
				///  <para>Cannot be null or else an exception will be thrown.</para>
				///  <para>This class will only hold on to a weak reference to this object.</para>
				/// </param>
				NativeEventHandler(CoronaRuntimeEnvironment^ environment);

				/// <summary>
				///  <para>Callback to be given to Lua to be called when Corona's Runtime:dispatchEvent() gets called.</para>
				///  <para>Passes the Corona event to the handlers that subscibed to it via the AddEventLister() method.</para>
				/// </summary>
				/// <param name="luaStatePointer">The Lua state that the event was dispatched from.</param>
				/// <returns>
				///  <para>Returns the number of return values that have been pushed into Lua.</para>
				///  <para>Returns zero if no return values have been pushed into Lua.</para>
				/// </returns>
				static int OnLuaRuntimeEventReceived(lua_State *luaStatePointer);

			private:
				/// <summary>Weak reference to the CoronaRuntimeEnvironment object that owns this native event handler.</summary>
				Platform::WeakReference fEnvironmentWeakReference;
		};

		#pragma endregion


		#pragma region Private Methods/Properties
		/// <summary>Suspends the runtime, if currently running. Does nothing if terminated.</summary>
		void Suspend();

		/// <summary>Resumes the runtime, if currently suspended. Does nothing if terminated.</summary>
		void Resume();

		/// <summary>Determines if the runtime was launched using a "resource.car" file instead of a Lua file.</summary>
		/// <value>
		///  <para>
		///   Set true if the runtime was launched using a "resource.car" file.
		///   This is typically the case for Corona Simulator or Corona Enterprise built applications.
		///  </para>
		///  <para>
		///   Set false if the runtime was launched using a Lua file, such as "main.lua".
		///   This is typically the case for Corona Cards built applications.
		///  </para>
		/// </value>
		property bool IsUsingResourceCar { bool get(); }

		/// <summary>
		///  Called when the native Corona runtime has loaded the "config.lua" and is about to execute the "main.lua" file.
		/// </summary>
		void OnRuntimeLoaded();

		/// <summary>Called when the native Corona runtime has finished executing the "main.lua" file.</summary>
		void OnRuntimeStarted();

		/// <summary>
		///  <para>Called when the native Corona runtime's timer has elapsed.</para>
		///  <para>Updates the Corona runtime for the next "enterFrame" and requests the surface to render a frame.</para>
		/// </summary>
		void OnRuntimeTimerElapsed();

		/// <summary>Called just before the rendering surface attempts to synchronize with the rendering thread.</summary>
		/// <param name="sender">The rendering surface that is about to acquire the rendering context.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnAcquiringRenderContext(Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args);

		/// <summary>Called when the a new Direct3D context has been received.</summary>
		/// <param name="sender">The rendering surface that has received a new context.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnReceivedRenderContext(Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args);
		
		/// <summary>
		///  <para>Called when the rendering surface has lost its Direct3D context.</para>
		///  <para>This can happen for the following reasons:</para>
		///  <para>- When the application has been suspended.</para>
		///  <para>- The Xaml control hosting the Direct3D surface has been removed from the application page.</para>
		///  <para>- The Corona content provider has been removed from the Direct3D drawing surface.</para>
		/// </summary>
		/// <param name="sender">The rendering surface that has lost its context.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnLostRenderContext(Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args);
		
		/// <summary>Called when the rendering surface has been resized.</summary>
		/// <param name="sender">The rendering surface that has been resized.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnSurfaceResized(Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args);
		
		/// <summary>Called when the surface is requesting the runtime to render the next frame.</summary>
		/// <param name="sender">The rendering surface requesting a frame.</param>
		void OnRenderFrame(Interop::Graphics::IRenderSurface ^sender);
		
		/// <summary>Called when the application page that is hosting the Corona control has changed orientation.</summary>
		/// <param name="sender">The application page that has raised this event.</param>
		/// <param name="args">Provides the orientation that the page has changed to.</param>
		void OnPageOrientationChanged(Interop::UI::IPage^ sender, Interop::UI::PageOrientationEventArgs^ args);
		
		/// <summary>Called when a key has been pressed down.</summary>
		/// <param name="sender">The UI control that raised this event.</param>
		/// <param name="args">Provides information about the key that was pressed.</param>
		void OnReceivedKeyDown(Platform::Object^ sender, Interop::Input::KeyEventArgs^ args);
		
		/// <summary>Called when a key has been released.</summary>
		/// <param name="sender">The UI control that raised this event.</param>
		/// <param name="args">Provides information about the key that was released.</param>
		void OnReceivedKeyUp(Platform::Object^ sender, Interop::Input::KeyEventArgs^ args);
		
		/// <summary>Raises a "key" event in Lua.</summary>
		/// <param name="args">The key event that was received.</param>
		/// <param name="isDown">Set true if the key was pressed down. Set false if the key was released.</param>
		void RaiseKeyEventFor(Interop::Input::KeyEventArgs^ args, bool isDown);
		
		/// <summary>Called when the surface has received a "tap" event.</summary>
		/// <param name="sender">The surface that is raising this event.</param>
		/// <param name="args">Provides the tap location, tap count, and timestamp.</param>
		void OnReceivedTap(Platform::Object ^sender, Interop::Input::TapEventArgs ^args);
		
		/// <summary>Called when the surface has received a "touch" event.</summary>
		/// <param name="sender">The surface that is raising this event.</param>
		/// <param name="args">Provides the touch location, timestamp, and other details.</param>
		void OnReceivedTouch(Platform::Object ^sender, Interop::Input::TouchEventArgs ^args);
		
		/// <summary>Raises all queued "touch" events, to be received by Lua's listeners.</summary>
		void RaiseTouchEvents();

		/// <summary>
		///  <para>Detects if a resize or orientation change has occurred</para>
		///  <para>If occurred, then this method updates the Corona display and raises events in Lua.</para>
		/// </summary>
		void UpdateOrientationAndSurfaceSize();
		
		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Stores a copy of the launch settings used to create the Corona runtime environment.</summary>
		CoronaRuntimeLaunchSettings^ fLaunchSettings;

		/// <summary>Provides read-only access to Corona's interop layer between languages and platforms.</summary>
		Interop::InteropServices^ fInteropServices;

		/// <summary>
		///  <para>Reference to the Corona control that is hosting the Direct3D rendering surface.</para>
		///  <para>Set to null if the Corona runtime is not set up to render to a surface.</para>
		/// </summary>
		Interop::UI::ICoronaControlAdapter^ fCoronaControlAdapter;

		/// <summary>Indicates the current state of the runtime such as Starting, Running, Suspended, etc.</summary>
		CoronaRuntimeState fRuntimeState;

		/// <summary>
		///  <para>
		///   Set to true if the CoronaRuntimeEnvironment::Controller object Suspend() method was called
		///   and false if its Resume() method was called.
		///  </para>
		///  <para>
		///   This is needed to respect the developer's Suspend() call so that the environment will not automatically
		///   resume the runtime when the app has been suspended/resumed.
		///  </para>
		/// </summary>
		bool fWasSuspendRequestedExternally;

		/// <summary>
		///  Set to true if the Direct3D context was lost during runtime, which means that GPU resources
		///  such as textures and shaders need to be reloaded.
		/// </summary>
		bool fWasGraphicsResourcesLost;

		/// <summary>Pointer to Corona's native C++ WinRT platform implementation.</summary>
		Rtt::WinRTPlatform* fPlatformPointer;

		/// <summary>Pointer to Corona's native C++ runtime which drives the Corona application.</summary>
		Rtt::Runtime* fRuntimePointer;

		/// <summary>Delegate given to the native "fRuntimePointer" object handle its events.</summary>
		Rtt::WinRTRuntimeDelegate* fRuntimeDelegatePointer;

		/// <summary>
		///  <para>Callback given to the native Rtt::Runtime which gets invoked every time the runtime's main timer elapses.</para>
		///  <para>Used to "update" the Corona runtime for the next frame and to request rendering.</para>
		/// </summary>
		Rtt::WinRTMethodCallback fRuntimeTimerElapsedCallback;

		/// <summary>Provides native C/C++ callbacks used to receive events from native C/C++ code such as Lua.</summary>
		NativeEventHandler fNativeEventHandler;

		/// <summary>
		///  <para>Schedules the next time the Corona runtime should be updated and render to the surface.</para>
		///  <para>
		///   This time is based on the value return from Rtt::Runtime::ElapsedMS(), which is the number of milliseconds
		///   the runtime has been running.
		///  </para>
		/// </summary>
		double fNextRuntimeUpdateInElapsedMilliseconds;

		/// <summary>
		///  <para>Collection of Corona event handlers stored using the Corona event names as the key.</para>
		///  <para>Handlers are added and removed via the AddEventListener() and RemoveEventListener methods().</para>
		/// </summary>
		Platform::Collections::Map<Platform::String^, Windows::Foundation::Collections::IVector<CoronaLuaEventHandler^>^> fEventHandlerMap;

		/// <summary>
		///  <para>Lua registry reference key to the OnLuaRuntimeEventReceived() native callback closure.</para>
		///  <para>
		///   The referenced closure is passed to the Lua Runtime:addEventListener() and Runtime:removeEventListener()
		///   functions.
		///  </para>
		///  <para>Set to LUA_NOREF if this closure has not been registered into the Lua registry yet.</para>
		/// </summary>
		int fLuaRuntimeEventReceivedClosureReferenceKey;

		/// <summary>Stores the last application page orientation that the Corona control is being hosted in.</summary>
		Rtt::DeviceOrientation::Type fLastPageOrientation;
		
		/// <summary>
		///  <para>Stores the last relative orientation that the Direct3D surface was in.</para>
		///  <para>For a Windows Phone Xaml DrawingSurface, this is always upright and renders relative to the app page's orientation.</para>
		///  <para>For a Windows Phone Xaml DrawingSurfaceBackgroundGrid, the surface orientation can change.</para>
		/// </summary>
		Rtt::DeviceOrientation::Type fLastSurfaceOrientation;
		
		/// <summary>Tracks touche events received from the drawing surface.</summary>
		Interop::Input::TouchTrackerCollection^ fTouchTrackerCollection;
		
		/// <summary>Token received when adding a handler to the surface's "AcquiringRenderContext" event.</summary>
		Windows::Foundation::EventRegistrationToken fSurfaceAcquiringRenderContextEventToken;

		/// <summary>Token received when adding a handler to the surface's "ReceivedRenderContext" event.</summary>
		Windows::Foundation::EventRegistrationToken fSurfaceReceivedRenderContextEventToken;
		
		/// <summary>Token received when adding a handler to the surface's "LostRenderContext" event.</summary>
		Windows::Foundation::EventRegistrationToken fSurfaceLostRenderContextEventToken;
		
		/// <summary>Token received when adding a handler to the surface's "Resized" event.</summary>
		Windows::Foundation::EventRegistrationToken fSurfaceResizedEventToken;
		
		/// <summary>Token received when adding a handler to the page's "OrientationChanged" event.</summary>
		Windows::Foundation::EventRegistrationToken fPageOrientationChangedEventToken;
		
		/// <summary>Token received when adding a handler to the surface's "ReceivedKeyDown" event.</summary>
		Windows::Foundation::EventRegistrationToken fReceivedKeyDownEventToken;
		
		/// <summary>Token received when adding a handler to the surface's "ReceivedKeyUp" event.</summary>
		Windows::Foundation::EventRegistrationToken fReceivedKeyUpEventToken;
		
		/// <summary>Token received when adding a handler to the surface's "ReceivedTap" event.</summary>
		Windows::Foundation::EventRegistrationToken fReceivedTapEventToken;
		
		/// <summary>Token received when adding a handler to the surface's "ReceivedTouch" event.</summary>
		Windows::Foundation::EventRegistrationToken fReceivedTouchEventToken;

		#pragma endregion
};

} } }	// namespace CoronaLabs::Corona::WinRT
