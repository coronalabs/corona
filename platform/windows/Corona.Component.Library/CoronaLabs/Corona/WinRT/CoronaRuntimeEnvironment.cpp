//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Display\Rtt_Display.h"
#	include "Display\Rtt_Scene.h"
#	include "Display\Rtt_StageObject.h"
#	include "Display\Rtt_TextObject.h"
#	include "Renderer\Rtt_Renderer.h"
#	include "Rtt_DeviceOrientation.h"
#	include "Rtt_Event.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaResource.h"
#	include "Rtt_PlatformTimer.h"
#	include "Rtt_Runtime.h"
extern "C"
{
#	include "lua.h"
}
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END
#include "CoronaBoxedData.h"
#include "CoronaBoxedString.h"
#include "CoronaLuaEventArgs.h"
#include "CoronaLuaEventHandler.h"
#include "CoronaLuaEventProperties.h"
#include "CoronaRuntimeEnvironment.h"
#include "CoronaRuntimeLaunchSettings.h"
#include "CoronaRuntimeState.h"
#include "DispatchCoronaLuaEventResult.h"
#include "ICoronaBoxedData.h"
#include "ReadOnlyCoronaLuaEventProperties.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include "CoronaLabs\WinRT\Utf8String.h"
#include "Interop\Graphics\IRenderSurface.h"
#include "Interop\Input\IInputDeviceServices.h"
#include "Interop\Input\Key.h"
#include "Interop\Input\KeyEventArgs.h"
#include "Interop\Input\TapEventArgs.h"
#include "Interop\Input\TouchEventArgs.h"
#include "Interop\Input\TouchTrackerCollection.h"
#include "Interop\Input\TouchTracker.h"
#include "Interop\Logging\LoggingServices.h"
#include "Interop\Logging\NativeDebuggerLogger.h"
#include "Interop\UI\ICoronaControlAdapter.h"
#include "Interop\UI\IPage.h"
#include "Interop\UI\IUserInterfaceServices.h"
#include "Interop\UI\PageOrientationEventArgs.h"
#include "Interop\CoronaInteropSettings.h"
#include "Interop\InteropServices.h"
#include "Interop\IReadOnlyWeakReference.h"
#include "Phone\Interop\Graphics\Direct3DSurfaceAdapter.h"
#include "Rtt_WinRTCallback.h"
#include "Rtt_WinRTPlatform.h"
#include "Rtt_WinRTRuntimeDelegate.h"
#include <string>


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Constructors/Destructors
CoronaRuntimeEnvironment::CoronaRuntimeEnvironment(const CoronaRuntimeEnvironment::CreationSettings &settings)
:	fRuntimeState(CoronaRuntimeState::Starting),
	fNextRuntimeUpdateInElapsedMilliseconds(0),
	fWasSuspendRequestedExternally(false),
	fWasGraphicsResourcesLost(true),
	fLuaRuntimeEventReceivedClosureReferenceKey(LUA_NOREF)
{
	// Validate the given settings.
	if (nullptr == settings.LaunchSettings)
	{
		throw ref new Platform::NullReferenceException(L"settings.LaunchSettings");
	}
	if (nullptr == settings.InteropSettings)
	{
		throw ref new Platform::NullReferenceException(L"settings.InteropSettings");
	}

	// Set up the logging system, if not done already.
	if (nullptr == Interop::Logging::LoggingServices::Logger)
	{
		Interop::Logging::LoggingServices::Logger = Interop::Logging::NativeDebuggerLogger::Instance;
	}

	// Create a collection used to track touch events for each pointer/finger on the screen.
	fTouchTrackerCollection = ref new Interop::Input::TouchTrackerCollection();

	// Create the callback to be used by the Rtt::Runtime object's frame rate timer.
	fRuntimeTimerElapsedCallback = Rtt::WinRTMethodCallback(
			ref new Rtt::WinRTMethodCallbackHandler(this, &CoronaRuntimeEnvironment::OnRuntimeTimerElapsed));

	// Store copies of the given settings.
	fLaunchSettings = ref new CoronaRuntimeLaunchSettings();
	fLaunchSettings->CopyFrom(settings.LaunchSettings);
	fInteropServices = ref new Interop::InteropServices(settings.InteropSettings);

	// Store a reference to the Corona control hosting the rendering surface, if provided.
	// Note: This can be null, which means the Corona runtime will not support rendering.
	fCoronaControlAdapter = nullptr;
	if (settings.InteropSettings->WeakCoronaControlReference && settings.InteropSettings->UserInterfaceServices)
	{
		auto coronaControl = settings.InteropSettings->WeakCoronaControlReference->GetAsObject();
		auto controlAdapter = settings.InteropSettings->UserInterfaceServices->CreateAdapterFor(coronaControl);
		fCoronaControlAdapter = dynamic_cast<Interop::UI::ICoronaControlAdapter^>(controlAdapter);
	}

	// Set up the native Corona runtime objects.
	fPlatformPointer = new Rtt::WinRTPlatform(this);
	fRuntimePointer = Rtt_NEW(
			&(fPlatformPointer->GetAllocator()),
			Rtt::Runtime(*fPlatformPointer, &fRuntimeTimerElapsedCallback));
	fRuntimePointer->SetProperty(Rtt::Runtime::kDeferUpdate, true);
	fRuntimePointer->SetProperty(Rtt::Runtime::kRenderAsync, true);
#ifdef Rtt_ENTERPRISE
	fRuntimePointer->SetProperty(Rtt::Runtime::kIsEnterpriseFeature, true);
#endif
	bool hasMainLuaFile = (this->IsUsingResourceCar == false);
	fRuntimePointer->SetProperty(Rtt::Runtime::kIsApplicationNotArchived, hasMainLuaFile);
	fRuntimePointer->SetProperty(Rtt::Runtime::kIsLuaParserAvailable, hasMainLuaFile);
	if (hasMainLuaFile)
	{
		fRuntimePointer->SetProperty(Rtt::Runtime::kIsCoronaKit, true);
	}
	fRuntimeDelegatePointer = Rtt_NEW(&(fPlatformPointer->GetAllocator()), Rtt::WinRTRuntimeDelegate(this));
	fRuntimeDelegatePointer->SetLoadedCallback(
			Rtt::WinRTMethodCallback(ref new Rtt::WinRTMethodCallbackHandler(this, &CoronaRuntimeEnvironment::OnRuntimeLoaded)));
	fRuntimeDelegatePointer->SetStartedCallback(
			Rtt::WinRTMethodCallback(ref new Rtt::WinRTMethodCallbackHandler(this, &CoronaRuntimeEnvironment::OnRuntimeStarted)));
	fRuntimePointer->SetDelegate(fRuntimeDelegatePointer);

	// Fetch the orientation of the page that is hosting the Corona control and the orientation of the Direct3D surface.
	fLastPageOrientation = Rtt::DeviceOrientation::kUpright;
	fLastSurfaceOrientation = Rtt::DeviceOrientation::kUpright;
	if (fCoronaControlAdapter)
	{
		fLastPageOrientation = fCoronaControlAdapter->ParentPageProxy->Orientation->CoronaDeviceOrientation;
		fLastSurfaceOrientation = fCoronaControlAdapter->RenderSurface->Orientation->CoronaDeviceOrientation;
	}

	// Add event handlers.
	fNativeEventHandler = NativeEventHandler(this);
	if (settings.LoadedEventHandler)
	{
		this->Loaded += settings.LoadedEventHandler;
	}
	if (settings.StartedEventHandler)
	{
		this->Started += settings.StartedEventHandler;
	}
	if (settings.SuspendedEventHandler)
	{
		this->Suspended += settings.SuspendedEventHandler;
	}
	if (settings.ResumedEventHandler)
	{
		this->Resumed += settings.ResumedEventHandler;
	}
	if (settings.TerminatingEventHandler)
	{
		this->Terminating += settings.TerminatingEventHandler;
	}
	if (fCoronaControlAdapter)
	{
		fCoronaControlAdapter->RenderSurface->SetRenderFrameHandler(
				ref new Interop::Graphics::RenderFrameHandler(this, &CoronaRuntimeEnvironment::OnRenderFrame));
		fSurfaceAcquiringRenderContextEventToken = fCoronaControlAdapter->RenderSurface->AcquiringRenderContext += ref new Windows::Foundation::TypedEventHandler<Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntimeEnvironment::OnAcquiringRenderContext);
		fSurfaceReceivedRenderContextEventToken = fCoronaControlAdapter->RenderSurface->ReceivedRenderContext += ref new Windows::Foundation::TypedEventHandler<Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntimeEnvironment::OnReceivedRenderContext);
		fSurfaceLostRenderContextEventToken = fCoronaControlAdapter->RenderSurface->LostRenderContext += ref new Windows::Foundation::TypedEventHandler<Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntimeEnvironment::OnLostRenderContext);
		fSurfaceResizedEventToken = fCoronaControlAdapter->RenderSurface->Resized += ref new Windows::Foundation::TypedEventHandler<Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaRuntimeEnvironment::OnSurfaceResized);
		fPageOrientationChangedEventToken = fCoronaControlAdapter->ParentPageProxy->OrientationChanged += ref new Windows::Foundation::TypedEventHandler<Interop::UI::IPage^, Interop::UI::PageOrientationEventArgs^>(this, &CoronaRuntimeEnvironment::OnPageOrientationChanged);
	}
	fReceivedKeyDownEventToken = fInteropServices->InputDeviceServices->ReceivedKeyDown += ref new Windows::Foundation::EventHandler<Interop::Input::KeyEventArgs^>(this, &CoronaRuntimeEnvironment::OnReceivedKeyDown);
	fReceivedKeyUpEventToken = fInteropServices->InputDeviceServices->ReceivedKeyUp += ref new Windows::Foundation::EventHandler<Interop::Input::KeyEventArgs^>(this, &CoronaRuntimeEnvironment::OnReceivedKeyUp);
	fReceivedTapEventToken = fInteropServices->InputDeviceServices->ReceivedTap += ref new Windows::Foundation::EventHandler<Interop::Input::TapEventArgs^>(this, &CoronaRuntimeEnvironment::OnReceivedTap);
	fReceivedTouchEventToken = fInteropServices->InputDeviceServices->ReceivedTouch += ref new Windows::Foundation::EventHandler<Interop::Input::TouchEventArgs^>(this, &CoronaRuntimeEnvironment::OnReceivedTouch);

	// Request the surface to render now if it is ready.
	// Note: We can only call the runtime's LoadApplication() when synchronized with the Direct3D thread,
	//       which only happens when the OnSurfaceRequestingRenderFrame() method gets called.
	if (fCoronaControlAdapter && fCoronaControlAdapter->RenderSurface->IsReadyToRender)
	{
		fCoronaControlAdapter->RenderSurface->RequestRender();
	}
//TODO: Load the application *here* if a Corona control was not provided.
}

CoronaRuntimeEnvironment::~CoronaRuntimeEnvironment()
{
	// Destroy the runtime, if not done already.
	Terminate();
}

#pragma endregion


#pragma region Public Methods/Properties
Platform::String^ CoronaRuntimeEnvironment::ResourceDirectoryPath::get()
{
	return fLaunchSettings->ResourceDirectoryPath;
}

Platform::String^ CoronaRuntimeEnvironment::DocumentsDirectoryPath::get()
{
	return fLaunchSettings->DocumentsDirectoryPath;
}

Platform::String^ CoronaRuntimeEnvironment::TemporaryDirectoryPath::get()
{
	return fLaunchSettings->TemporaryDirectoryPath;
}

Platform::String^ CoronaRuntimeEnvironment::CachesDirectoryPath::get()
{
	return fLaunchSettings->CachesDirectoryPath;
}

Platform::String^ CoronaRuntimeEnvironment::InternalDirectoryPath::get()
{
	return fLaunchSettings->InternalDirectoryPath;
}

Platform::String^ CoronaRuntimeEnvironment::LaunchFilePath::get()
{
	return fLaunchSettings->LaunchFilePath;
}

Interop::InteropServices^ CoronaRuntimeEnvironment::InteropServices::get()
{
	return fInteropServices;
}

Interop::UI::ICoronaControlAdapter^ CoronaRuntimeEnvironment::CoronaControlAdapter::get()
{
	return fCoronaControlAdapter;
}

CoronaRuntimeState CoronaRuntimeEnvironment::RuntimeState::get()
{
	return fRuntimeState;
}

DispatchCoronaLuaEventResult^ CoronaRuntimeEnvironment::DispatchEvent(CoronaLuaEventArgs^ eventArgs)
{
	// Validate argument.
	if (nullptr == eventArgs)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Fetch the runtime's lua state, if available.
	lua_State* luaStatePointer = nullptr;
	if (fRuntimePointer)
	{
		if (fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded) == false)
		{
			return DispatchCoronaLuaEventResult::FailedWith(
						L"Cannot dispatch an event to Lua yet. Corona runtime has not finished starting up.");
		}
		luaStatePointer = fRuntimePointer->VMContext().L();
	}
	if (nullptr == luaStatePointer)
	{
		return DispatchCoronaLuaEventResult::FailedWith(
					L"Cannot dispatch an event to Lua because the Corona runtime has been terminated.");
	}

	// Push the given Corona event's properties as a table to the top of the Lua stack.
	auto pushResult = eventArgs->Properties->PushTo(luaStatePointer);
	if (pushResult->HasFailed)
	{
		return DispatchCoronaLuaEventResult::FailedWith(pushResult->Message);
	}

	// Call Corona's Runtime:dispatchEvent() function in Lua using the above pushed in Lua event table as an argument.
	lua_getglobal(luaStatePointer, "Runtime");
	lua_getfield(luaStatePointer, -1, "dispatchEvent");
	lua_insert(luaStatePointer, -3);
	lua_insert(luaStatePointer, -2);
	int luaCallResult = fRuntimePointer->VMContext().DoCall(luaStatePointer, 2, 1);
	if (LUA_ERRRUN == luaCallResult)
	{
		return DispatchCoronaLuaEventResult::FailedWith(L"Lua runtime error occurred.");
	}
	else if (LUA_ERRMEM == luaCallResult)
	{
		return DispatchCoronaLuaEventResult::FailedWith(L"Lua memory allocation error occurred.");
	}
	else if (luaCallResult)
	{
		return DispatchCoronaLuaEventResult::FailedWith(L"An unknown error occurred in Lua. Please see the Visual Studio output log for more details.");
	}

	// Fetch the value returned by the Lua listener function, if any.
	ICoronaBoxedData^ returnedValue = CoronaBoxedData::FromLua((int64)luaStatePointer, -1);
	lua_pop(luaStatePointer, 1);

	// Return the result of this operation.
	return DispatchCoronaLuaEventResult::SucceededWith(returnedValue);
}

void CoronaRuntimeEnvironment::AddEventListener(Platform::String^ eventName, CoronaLuaEventHandler^ eventHandler)
{
	// Validate arguments.
	if (eventName->IsEmpty() || !eventHandler)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Fetch the event handler collection for the given event name.
	if (fEventHandlerMap.HasKey(eventName) == false)
	{
		// A collection has not been added to the dictionary yet. Add one.
		fEventHandlerMap.Insert(eventName, ref new Platform::Collections::Vector<CoronaLuaEventHandler^>());
	}
	auto eventHandlerCollection = fEventHandlerMap.Lookup(eventName);

	// Do not continue if the given event handler has already been added under the given event name.
	for (auto&& nextEventHandler : eventHandlerCollection)
	{
		if (nextEventHandler == eventHandler)
		{
			return;
		}
	}

	// Add the event handler to the collection.
	eventHandlerCollection->Append(eventHandler);

	// Add the event handler to Lua, if the Lua state is currently available.
	// Note: We only need to do this once per event name.
	if (eventHandlerCollection->Size == 1)
	{
		lua_State* luaStatePointer = nullptr;
		if (fRuntimePointer)
		{
			luaStatePointer = fRuntimePointer->VMContext().L();
		}
		if (luaStatePointer)
		{
			// Register this runtime's OnLuaRuntimeEventReceived() callback/closure to the Lua registry.
			// Note: We only need to this once per runtime instance. This closure used by all native event handlers.
			if (LUA_NOREF == fLuaRuntimeEventReceivedClosureReferenceKey)
			{
				lua_pushlightuserdata(luaStatePointer, (void*)&fNativeEventHandler);
				lua_pushcclosure(luaStatePointer, &NativeEventHandler::OnLuaRuntimeEventReceived, 1);
				fLuaRuntimeEventReceivedClosureReferenceKey = luaL_ref(luaStatePointer, LUA_REGISTRYINDEX);
			}

			// Add the event handler.
			auto utf8EventName = CoronaLabs::WinRT::NativeStringServices::Utf8From(eventName);
			lua_getglobal(luaStatePointer, "Runtime");
			lua_getfield(luaStatePointer, -1, "addEventListener");
			lua_insert(luaStatePointer, -2);
			lua_pushstring(luaStatePointer, utf8EventName->Data);
			lua_rawgeti(luaStatePointer, LUA_REGISTRYINDEX, fLuaRuntimeEventReceivedClosureReferenceKey);
			fRuntimePointer->VMContext().DoCall(luaStatePointer, 3, 0);
		}
	}
}

bool CoronaRuntimeEnvironment::RemoveEventListener(Platform::String^ eventName, CoronaLuaEventHandler^ eventHandler)
{
	// Validate arguments.
	if (eventName->IsEmpty() || !eventHandler)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Do not continue if we've never registered our native event listener.
	// This is a clear indicator that AddEventListener() was never called.
	if (LUA_NOREF == fLuaRuntimeEventReceivedClosureReferenceKey)
	{
		return false;
	}

	// Fetch the event handler collection for the given event name.
	if (fEventHandlerMap.HasKey(eventName) == false)
	{
		return false;
	}
	auto eventHandlerCollection = fEventHandlerMap.Lookup(eventName);

	// Find the given event handler in the collection.
	unsigned int index = 0;
	bool wasFound = eventHandlerCollection->IndexOf(eventHandler, &index);
	if (false == wasFound)
	{
		return false;
	}

	// Remove the given event handler from the collection.
	eventHandlerCollection->RemoveAt(index);

	// Remove the event handler from Lua if there are no more handlers for the given event name.
	if (eventHandlerCollection->Size == 0)
	{
		// First, remove the entry in this object's map.
		fEventHandlerMap.Remove(eventName);

		// Remove the handler from Lua, if available.
		lua_State* luaStatePointer = nullptr;
		if (fRuntimePointer)
		{
			luaStatePointer = fRuntimePointer->VMContext().L();
		}
		if (luaStatePointer)
		{
			auto utf8EventName = CoronaLabs::WinRT::NativeStringServices::Utf8From(eventName);
			lua_getglobal(luaStatePointer, "Runtime");
			lua_getfield(luaStatePointer, -1, "removeEventListener");
			lua_insert(luaStatePointer, -2);
			lua_pushstring(luaStatePointer, utf8EventName->Data);
			lua_rawgeti(luaStatePointer, LUA_REGISTRYINDEX, fLuaRuntimeEventReceivedClosureReferenceKey);
			fRuntimePointer->VMContext().DoCall(luaStatePointer, 3, 0);
		}
	}

	// Return true to indicate that removal was successful.
	return true;
}

#pragma endregion


#pragma region Public Static Functions/Properties
Platform::String^ CoronaRuntimeEnvironment::DefaultResourceDirectoryPath::get()
{
	static Platform::String^ kPath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + L"\\Assets\\Corona";
	return kPath;
}

Platform::String^ CoronaRuntimeEnvironment::DefaultDocumentsDirectoryPath::get()
{
	static Platform::String^ kPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path + L"\\Corona\\Documents";
	return kPath;
}

Platform::String^ CoronaRuntimeEnvironment::DefaultTemporaryDirectoryPath::get()
{
	static Platform::String^ kPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path + L"\\Corona\\TemporaryFiles";
	return kPath;
}

Platform::String^ CoronaRuntimeEnvironment::DefaultCachesDirectoryPath::get()
{
	static Platform::String^ kPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path + L"\\Corona\\CachedFiles";
	return kPath;
}

Platform::String^ CoronaRuntimeEnvironment::DefaultInternalDirectoryPath::get()
{
	static Platform::String^ kPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path + L"\\Corona\\.system";
	return kPath;
}

#pragma endregion


#pragma region Internal Methods/Properties
Rtt::WinRTPlatform* CoronaRuntimeEnvironment::NativePlatformPointer::get()
{
	return fPlatformPointer;
}

Rtt::Runtime* CoronaRuntimeEnvironment::NativeRuntimePointer::get()
{
	return fRuntimePointer;
}

void CoronaRuntimeEnvironment::RequestSuspend()
{
	fWasSuspendRequestedExternally = true;
	Suspend();
}

void CoronaRuntimeEnvironment::RequestResume()
{
	fWasSuspendRequestedExternally = false;
	Resume();
}

void CoronaRuntimeEnvironment::Terminate()
{
	// Do not continue if currently terminating or already terminated.
	if (nullptr == fRuntimePointer)
	{
		return;
	}
	if ((CoronaRuntimeState::Terminating == fRuntimeState) || (CoronaRuntimeState::Terminated == fRuntimeState))
	{
		return;
	}

	// Indicate that we're currently terminating the runtime.
	fRuntimeState = CoronaRuntimeState::Terminating;
	fWasSuspendRequestedExternally = false;

	// Notify the system that we're about to terminate.
	this->Terminating(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);

	// Reset all touch trackers. This clears all of the touch data that they have received.
	fTouchTrackerCollection->ResetAll();

	// Stop listening for events.
	if (fCoronaControlAdapter)
	{
		if (fCoronaControlAdapter->RenderSurface)
		{
			fCoronaControlAdapter->RenderSurface->SetRenderFrameHandler(nullptr);
			fCoronaControlAdapter->RenderSurface->AcquiringRenderContext -= fSurfaceAcquiringRenderContextEventToken;
			fCoronaControlAdapter->RenderSurface->ReceivedRenderContext -= fSurfaceReceivedRenderContextEventToken;
			fCoronaControlAdapter->RenderSurface->LostRenderContext -= fSurfaceLostRenderContextEventToken;
			fCoronaControlAdapter->RenderSurface->Resized -= fSurfaceResizedEventToken;
		}
		fCoronaControlAdapter->ParentPageProxy->OrientationChanged -= fPageOrientationChangedEventToken;
	}
	fInteropServices->InputDeviceServices->ReceivedKeyDown -= fReceivedKeyDownEventToken;
	fInteropServices->InputDeviceServices->ReceivedKeyUp -= fReceivedKeyUpEventToken;
	fInteropServices->InputDeviceServices->ReceivedTap -= fReceivedTapEventToken;
	fInteropServices->InputDeviceServices->ReceivedTouch -= fReceivedTouchEventToken;

	// Release our strong reference to the Corona control, if available.
	// This allows the control to be garbage collected if no longer referenced.
	if (fCoronaControlAdapter)
	{
		fCoronaControlAdapter->ReleaseReferencedControl();
	}

	// Delete the native Corona runtime.
	Rtt_DELETE(fRuntimePointer);
	fRuntimePointer = nullptr;
	Rtt_DELETE(fPlatformPointer);
	fPlatformPointer = nullptr;
	Rtt_DELETE(fRuntimeDelegatePointer);
	fRuntimeDelegatePointer = nullptr;

	// Indicate that the runtime has been terminated.
	fRuntimeState = CoronaRuntimeState::Terminated;
}

#pragma endregion


#pragma region Runtime Event Handlers
void CoronaRuntimeEnvironment::OnRuntimeLoaded()
{
	// Notify the system that we've loaded the "config.lua" and we are about to execute the "main.lua" file.
	// This is the system's opportunity to register custom APIs into Lua.
	this->Loaded(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
}

void CoronaRuntimeEnvironment::OnRuntimeStarted()
{
	// Notify the system that we've finished executing the "main.lua" and we're now running the project and starting to render.
	fRuntimeState = CoronaRuntimeState::Running;
	this->Started(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);

	// If a suspend was requested while loading/starting, then suspend it now.
	if (fWasSuspendRequestedExternally)
	{
		Suspend();
	}
}

void CoronaRuntimeEnvironment::OnRuntimeTimerElapsed()
{
	bool isTimeToUpdateRuntime = false;
	bool isTimeToRender = false;

	// Do not continue if the Corona runtime is not currently starting/running.
	if ((nullptr == fRuntimePointer) || (false == fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded)))
	{
		return;
	}
	if ((fRuntimeState != CoronaRuntimeState::Starting) && (fRuntimeState != CoronaRuntimeState::Running))
	{
		return;
	}

	// Determine if we're scheduled to update the runtime.
	// Note: The runtime's timer is set up to elapse faster than the configured framerate.
	//       We do this because XAML timers are inaccurate at short intervals. (Min interval is typically around 8 ms.)
	//       This means we need to update earlier than scheduled by below tolerance because next interval will be late.
	static const double kTimerToleranceInMilliseconds = 5.0;
	double elapsedTimeInMilliseconds = fRuntimePointer->GetElapsedMS();
	if (elapsedTimeInMilliseconds >= (fNextRuntimeUpdateInElapsedMilliseconds - kTimerToleranceInMilliseconds))
	{
		isTimeToUpdateRuntime = true;
	}

	// Reload resources and update the runtime below if the rendering context was lost.
	if (fWasGraphicsResourcesLost)
	{
		fWasGraphicsResourcesLost = false;
		fRuntimePointer->GetDisplay().ReloadResources();
		fRuntimePointer->GetDisplay().Invalidate();
		isTimeToUpdateRuntime = true;
	}

	// Update the Corona runtime if scheduled to do so up above.
	if (isTimeToUpdateRuntime)
	{
		// Update the runtime's scene such as sprites and physics.
		// Note: This does not render the scene since the "kRenderAsync" property is set.
		(*fRuntimePointer)();
		if (fCoronaControlAdapter)
		{
			isTimeToRender = (fRuntimePointer->GetDisplay().GetStage()->GetScene().IsValid() == false);
		}

		// Schedule the next time the runtime should be updated.
		double framerateIntervalInMilliseconds = 1000.0 / (double)fRuntimePointer->GetFPS();
		do
		{
			fNextRuntimeUpdateInElapsedMilliseconds += framerateIntervalInMilliseconds;
		} while (fNextRuntimeUpdateInElapsedMilliseconds <= elapsedTimeInMilliseconds);
	}

	// Request the surface to render a frame.
	if (isTimeToRender && fCoronaControlAdapter)
	{
		if (fRuntimePointer->GetFPS() > 30)
		{
			// For 60 FPS apps, we want to mostly block the rendering thread and spend little time on the main UI thread.
			// This way we can update the Corona runtime on-time 60x a second and catch the render thread's every pass.
			// But this causes bad framerates with native UI which will block while waiting for the rendering thread.
			fCoronaControlAdapter->RenderSurface->RequestRender();
		}
		else
		{
			// For 30 FPS apps, we want to mostly block the main UI thread and spend little time on the rendering thread.
			// This makes Corona more native UI friendly because we won't be competing for the rendering thread with XAML.
			// But it's impossible to force render at 60 FPS since main UI timer will sometimes be called late.
			fCoronaControlAdapter->RenderSurface->ForceRender();
		}
	}
}

#pragma endregion


#pragma region RenderSurfaceController Event Handlers
void CoronaRuntimeEnvironment::OnAcquiringRenderContext(
	Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	// Do not continue if the runtime has not been started yet or has already been terminated.
	if (!fRuntimePointer || (fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded) == false))
	{
		return;
	}

	// Generate all needed text bitmaps before we render.
	// Note: We must do this here because XAML requires access to the rendering thread to generate text bitmaps,
	//       which it can't do while Corona has the rendering thread blocked in OnRenderFrame() below.
	Rtt::TextObject::UpdateAllBelongingTo(fRuntimePointer->GetDisplay());
}

void CoronaRuntimeEnvironment::OnReceivedRenderContext(
	Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	// Resume the runtime and reload all GPU resources if the rendering context was lost and later received.
	if (fRuntimeState != CoronaRuntimeState::Starting)
	{
		fWasGraphicsResourcesLost = true;
		if (false == fWasSuspendRequestedExternally)
		{
			Resume();
		}
	}
}

void CoronaRuntimeEnvironment::OnLostRenderContext(
	Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	// Flag that GPU resources such as textures and shaders have been lost.
	// This tells the system to reloaded resources when a new context has been received.
	fWasGraphicsResourcesLost = true;

	// Delete all references to GPU resources in application memory.
	if (fRuntimePointer && fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded))
	{
		fRuntimePointer->GetDisplay().GetScene().ForceCollect();
		fRuntimePointer->GetDisplay().UnloadResources();
	}

	// Suspend the Corona runtime since we no longer have a surface to render to.
	Suspend();
}

void CoronaRuntimeEnvironment::OnSurfaceResized(
	Interop::Graphics::IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	UpdateOrientationAndSurfaceSize();
}

void CoronaRuntimeEnvironment::OnRenderFrame(Interop::Graphics::IRenderSurface ^sender)
{
	// Do not continue if the runtime was terminated.
	if (nullptr == fRuntimePointer)
	{
		return;
	}

	// Load the application, if not done already.
	// Note: We must do this here because we have to wait until the surface has synchronized
	//       with the Direct3D thread and has acquired its Direct3D/OpenGL context.
	if (CoronaRuntimeState::Starting == fRuntimeState)
	{
		// Load the Corona application, starting with the "config.lua" and "shell.lua".
		auto result = fRuntimePointer->LoadApplication(Rtt::Runtime::kLaunchDeviceShell, fLastPageOrientation);
		if (Rtt::Runtime::kSuccess == result)
		{
			// Load was successful. Start running the Corona application.
			fRuntimePointer->BeginRunLoop();

			// Change the runtime's timer interval to 1 millisecond so that we can schedule updates/rendering ourselves.
			// Note: XAML timers are inaccurate and elapse erratically at low intervals. (Won't elapse less than 8 ms.)
			//       So, we have to schedule runtime updates at the closest time possible ourselves in OnRuntimeTimerElapsed().
			auto timerPointer = fRuntimePointer->GetTimer();
			if (timerPointer)
			{
				timerPointer->SetInterval(1);
			}
		}
		else if (Rtt::Runtime::kSecurityIssue == result)
		{
			// A CoronaCards licensing error occurred.
			// Leave the runtime in the "Starting" state because it'll run the "loader_callback.lua" script
			// in this case which displays a native alert on how to correct the licensing issue.
		}
		else
		{
			// Failed to load the Corona "resource.car" or "main.lua" file.
			Terminate();
		}

		// We must defer rendering until the next pass, even if we did successfully loaded the application.
		// This is due to text bitmap generation via Xaml which will fail while Corona is synchronized with the Direct3D thread.
		return;
	}

	// Do not continue if the Corona runtime is not currently running.
	if ((fRuntimeState != CoronaRuntimeState::Running) ||
	    (fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded) == false))
	{
		return;
	}

	// Determine if we're ready to render to the surface.
	bool canRender = true;
	if (Rtt::TextObject::IsUpdateNeededFor(fRuntimePointer->GetDisplay()))
	{
		// Do not render if we have not generated text bitmaps yet, which should happen in OnAcquiringRenderContent().
		// Note: XAML cannot create text bitmaps while we're in this method, causing a blank text bitmap to be rendered.
		canRender = false;
	}

	// Have the runtime render a frame.
	if (canRender)
	{
		fRuntimePointer->Render();
	}

	// If configured for 60 FPS, then always request another frame to be rendered.
	// This drastically improves the framerate, but only if no other XAML controls are onscreen.
	// Otherwise, Corona and XAML UI will be competing for time over the rendering thread.
	// Note: For 30 FPS Corona apps, do not dominate the rendering thread, which avoids framerate issues with XAML UI.
	if (fRuntimePointer->GetFPS() > 30)
	{
		fCoronaControlAdapter->RenderSurface->RequestRender();
	}
}

#pragma endregion


#pragma region Page Event Handlers
void CoronaRuntimeEnvironment::OnPageOrientationChanged(Interop::UI::IPage^ sender, Interop::UI::PageOrientationEventArgs^ args)
{
	UpdateOrientationAndSurfaceSize();
}

#pragma endregion


#pragma region InputDeviceServices Event Handlers
void CoronaRuntimeEnvironment::OnReceivedKeyDown(Platform::Object^ sender, Interop::Input::KeyEventArgs^ args)
{
	bool isDown = true;
	RaiseKeyEventFor(args, isDown);
}

void CoronaRuntimeEnvironment::OnReceivedKeyUp(Platform::Object^ sender, Interop::Input::KeyEventArgs^ args)
{
	bool isDown = false;
	RaiseKeyEventFor(args, isDown);
}

void CoronaRuntimeEnvironment::RaiseKeyEventFor(Interop::Input::KeyEventArgs^ args, bool isDown)
{
	// Do not continue if the Corona runtime is not currently running.
	if (!fRuntimePointer || (fRuntimeState != CoronaRuntimeState::Running))
	{
		return;
	}

	// Ignore the received key if it was already handled.
	if (args->Handled)
	{
		return;
	}

	// Raise a key event in Lua.
	Rtt::KeyEvent::Phase phase = isDown ? Rtt::KeyEvent::kDown : Rtt::KeyEvent::kUp;
	Rtt::KeyEvent keyEvent(
					nullptr, phase, args->Key->CoronaNameAsStringPointer,
					args->Key->NativeKeyCode, args->IsShiftDown, args->IsAltDown,
					args->IsControlDown, args->IsCommandDown);
	fRuntimePointer->DispatchEvent(keyEvent);

	//Raise a character event in Lua.
	if (isDown)
	{
		std::string character = keyInfo.GetCharacter();
		if (character.length() || isprint(character[0]))
		{
			Rtt::CharacterEvent characterEvent(nullptr, character.data());
			runtimePointer->DispatchEvent(characterEvent);
		}
	}

	// Consume the key event if the Lua listener returned true.
	args->Handled = keyEvent.GetResult();
}

void CoronaRuntimeEnvironment::OnReceivedTap(Platform::Object ^sender, Interop::Input::TapEventArgs ^args)
{
	// Do not continue if the Corona runtime is not currently running.
	if (!fRuntimePointer || (fRuntimeState != CoronaRuntimeState::Running))
	{
		return;
	}

	// Raise a tap event in Lua.
	auto tapPoint = args->TapPoint;
	Rtt::TapEvent event(Rtt_FloatToReal((float)tapPoint.X), Rtt_FloatToReal((float)tapPoint.Y), args->TapCount);
	fRuntimePointer->DispatchEvent(event);

	// Consume the tap event so that it won't get passed over to the Corona view's parent.
	args->Handled = true;
}

void CoronaRuntimeEnvironment::OnReceivedTouch(Platform::Object ^sender, Interop::Input::TouchEventArgs ^args)
{
	// Do not continue if the Corona runtime is not currently running.
	if (!fRuntimePointer || (fRuntimeState != CoronaRuntimeState::Running))
	{
		return;
	}

	// Fetch a touch tracker having the given event's pointer ID.
	auto touchTracker = fTouchTrackerCollection->GetByPointerId(args->PointerId);
	if (!touchTracker)
	{
		touchTracker = ref new Interop::Input::TouchTracker(args->PointerId);
		fTouchTrackerCollection->Add(touchTracker);
	}

	// Ignore the touch event if its phase is out of sync with what we've last recorded.
	if (args->Phase == Interop::Input::TouchPhase::Began)
	{
		if ((touchTracker->Phase == Interop::Input::TouchPhase::Began) ||
		    (touchTracker->Phase == Interop::Input::TouchPhase::Moved))
		{
			return;
		}
	}
	else if (args->Phase == Interop::Input::TouchPhase::Moved)
	{
		if (touchTracker->IsNotTracking ||
			(touchTracker->Phase == Interop::Input::TouchPhase::Ended) ||
			(touchTracker->Phase == Interop::Input::TouchPhase::Canceled))
		{
			return;
		}
	}
	else if (touchTracker->IsNotTracking)
	{
		return;
	}

//TODO: I need to raise a Canceled touch event if we're out of sync.

	// Ignore the touch event if it is flagged "moved", but it hasn't actually moved.
	if (Interop::Input::TouchPhase::Moved == args->Phase)
	{
		double deltaX = abs(args->Point.X - touchTracker->LastPoint.X);
		double deltaY = abs(args->Point.Y - touchTracker->LastPoint.Y);
		if ((deltaX < 1.0) && (deltaY < 1.0))
		{
			return;
		}
	}

	// Update the touch tracker with the newly received point.
	touchTracker->UpdateWith(args->Phase, args->Point);

	// Convert the touch event's timestamp from Win32 FileTime to milliseconds since the Corona runtime started.
	double touchTimeInMilliseconds;
	{
		Rtt_AbsoluteTime elapsedAbsoluteTime = fRuntimePointer->GetElapsedTime();
		SYSTEMTIME currentSystemTime;
		FILETIME currentFileTime;
		ULARGE_INTEGER int64Converter;
		GetSystemTime(&currentSystemTime);
		SystemTimeToFileTime(&currentSystemTime, &currentFileTime);
		int64Converter.LowPart = currentFileTime.dwLowDateTime;
		int64Converter.HighPart = currentFileTime.dwHighDateTime;
		Rtt_AbsoluteTime currentFileTimeInMicroseconds = (int64Converter.QuadPart / 10ULL);
		Rtt_AbsoluteTime touchFileTimeInMicroseconds = args->Point.Timestamp.UniversalTime / 10LL;
		Rtt_AbsoluteTime deltaAbsoluteTime = currentFileTimeInMicroseconds - touchFileTimeInMicroseconds;
		touchTimeInMilliseconds = (double)(elapsedAbsoluteTime - deltaAbsoluteTime) / 1000.0;
	}

	// Corona's touch event ID must be 1 based. So, add 1 since Microsoft's pointer IDs are zero based.
	int coronaTouchId = touchTracker->PointerId + 1;

	// Create a native Corona touch event object for the received event.
	auto startPoint = touchTracker->StartPoint;
	auto lastPoint = touchTracker->LastPoint;
	Rtt::TouchEvent touchEvent(
			(float)lastPoint.X, (float)lastPoint.Y,
			(float)startPoint.X, (float)startPoint.Y,
			touchTracker->Phase->CoronaTouchEventPhase);
	touchEvent.SetId((void*)coronaTouchId);
	touchEvent.SetTime(touchTimeInMilliseconds);

	// Raise a touch event in Lua.
	if (fInteropServices->InputDeviceServices->MultitouchEnabled)
	{
		Rtt::MultitouchEvent multitouchEvent(&touchEvent, 1);
		fRuntimePointer->DispatchEvent(multitouchEvent);
	}
	else
	{
		fRuntimePointer->DispatchEvent(touchEvent);
	}
}

#pragma endregion


#pragma region Native Event Handlers
CoronaRuntimeEnvironment::NativeEventHandler::NativeEventHandler()
{
}

CoronaRuntimeEnvironment::NativeEventHandler::NativeEventHandler(CoronaRuntimeEnvironment^ environment)
:	fEnvironmentWeakReference(environment)
{
	if (nullptr == environment)
	{
		throw ref new Platform::NullReferenceException();
	}
}

int CoronaRuntimeEnvironment::NativeEventHandler::OnLuaRuntimeEventReceived(lua_State *luaStatePointer)
{
	// Validate.
	if (nullptr == luaStatePointer)
	{
		return 0;
	}

	// Fetch the Corona runtime environment the lua state is tied to.
	auto nativeEventHandlerPointer = (NativeEventHandler*)lua_touserdata(luaStatePointer, lua_upvalueindex(1));
	if (nullptr == nativeEventHandlerPointer)
	{
		return 0;
	}
	auto environment = nativeEventHandlerPointer->fEnvironmentWeakReference.Resolve<CoronaRuntimeEnvironment>();
	if (nullptr == environment)
	{
		return 0;
	}

	// Fetch the Lua event table's properties, converted to WinRT objects.
	auto properties = CoronaLuaEventProperties::From(luaStatePointer, 1);
	if (nullptr == properties)
	{
		return 0;
	}

	// Fetch the Corona event's name.
	auto boxedEventName = dynamic_cast<CoronaBoxedString^>(properties->Get(L"name"));
	if (nullptr == boxedEventName)
	{
		return 0;
	}
	auto utf16EventName = boxedEventName->ToUtf16String();

	// Fetch this event's handlers, if any.
	if (environment->fEventHandlerMap.HasKey(utf16EventName) == false)
	{
		return 0;
	}
	auto eventHandlerCollection = environment->fEventHandlerMap.Lookup(utf16EventName);
	if (!eventHandlerCollection || (eventHandlerCollection->Size < 1))
	{
		return 0;
	}

	// Clone the event handler collection.
	// This prevents the iterator from being invalidated when an invoked handler calls Add/RemoveEventLister().
	auto eventHandlerArray = ref new Platform::Array<CoronaLuaEventHandler^>(eventHandlerCollection->Size);
	for (unsigned int index = 0; index < eventHandlerCollection->Size; index++)
	{
		eventHandlerArray[index] = eventHandlerCollection->GetAt(index);
	}

	// Invoke all of the event handlers in the cloned collection.
	auto eventArgs = ref new CoronaLuaEventArgs(properties);
	ICoronaBoxedData^ finalReturnedValue = nullptr;
	for (unsigned int index = 0; index < eventHandlerArray->Length; index++)
	{
		// Invoke the next handler and acquire its returned result.
		auto returnedResult = eventHandlerArray[index]->Invoke(environment, eventArgs);

		// Determine if we should use the returned result or not.
		// Note: This duplicates the behavior in "shell.lua" file's dispatchEvent() function.
		if (nullptr == finalReturnedValue)
		{
			finalReturnedValue = returnedResult;
		}
		else if (returnedResult && !Platform::Object::ReferenceEquals(returnedResult, CoronaBoxedBoolean::False))
		{
			finalReturnedValue = returnedResult;
		}
	}

	// Stop here if we have nothing to return back to Lua.
	if (nullptr == finalReturnedValue)
	{
		return 0;
	}

	// Push the returned value to Lua.
	finalReturnedValue->PushToLua((int64)luaStatePointer);
	return 1;
}

#pragma endregion


#pragma region Private Methods
void CoronaRuntimeEnvironment::Suspend()
{
	// Do not continue if the runtime is not currently running.
	if (!fRuntimePointer || (fRuntimeState != CoronaRuntimeState::Running))
	{
		return;
	}

	// Perform this suspend operation later if we're currently rendering to a Xaml control.
	// Note: We can only suspend the runtime if we're not synchronized with the rendering thread,
	//       because deadlock can occur if Corona's suspend listeners invoke anything in Xaml.
	if (fCoronaControlAdapter && Phone::Interop::Graphics::Direct3DSurfaceAdapter::IsSynchronizedWithRenderingThread)
	{
		auto handler = ref new Windows::UI::Core::DispatchedHandler(this, &CoronaRuntimeEnvironment::Suspend);
		fCoronaControlAdapter->Dispatcher->InvokeAsync(handler);
		return;
	}

	// Suspend the runtime.
	fRuntimeState = CoronaRuntimeState::Suspending;
	fRuntimePointer->Suspend();
	if (CoronaRuntimeState::Suspending == fRuntimeState)
	{
		fRuntimeState = CoronaRuntimeState::Suspended;
	}

	// Notify the system that the runtime is suspended.
	if (CoronaRuntimeState::Suspended == fRuntimeState)
	{
		this->Suspended(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
	}
}

void CoronaRuntimeEnvironment::Resume()
{
	// Do not continue if the runtime has not been started yet or has already been terminated.
	if (!fRuntimePointer || (fRuntimePointer->IsProperty(Rtt::Runtime::kIsApplicationLoaded) == false))
	{
		return;
	}

	// Perform this resume operation later if we're currently rendering to a Xaml control.
	// Note: We can only resume the runtime if we're not synchronized with the rendering thread,
	//       because deadlock can occur if Corona's resume listeners invoke anything in Xaml.
	if (fCoronaControlAdapter && Phone::Interop::Graphics::Direct3DSurfaceAdapter::IsSynchronizedWithRenderingThread)
	{
		auto handler = ref new Windows::UI::Core::DispatchedHandler(this, &CoronaRuntimeEnvironment::Resume);
		fCoronaControlAdapter->Dispatcher->InvokeAsync(handler);
		return;
	}

	// Resume the runtime if currently suspended.
	if ((CoronaRuntimeState::Suspended == fRuntimeState) ||
	    (CoronaRuntimeState::Suspending == fRuntimeState))
	{
		fRuntimeState = CoronaRuntimeState::Resuming;
	}
	if (CoronaRuntimeState::Resuming == fRuntimeState)
	{
		if (!fCoronaControlAdapter || fCoronaControlAdapter->RenderSurface->IsReadyToRender)
		{
			fRuntimePointer->Resume();
			if (CoronaRuntimeState::Resuming == fRuntimeState)
			{
				fRuntimeState = CoronaRuntimeState::Running;
				UpdateOrientationAndSurfaceSize();
				this->Resumed(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
			}
		}
	}
}

bool CoronaRuntimeEnvironment::IsUsingResourceCar::get()
{
	// Do not continue if launch settings or a launch file was never provided to this runtime. (This should never happen.)
	if (!fLaunchSettings || fLaunchSettings->LaunchFilePath->IsEmpty())
	{
		return false;
	}

	// Define the "resource.car" extension name and length.
	static const wchar_t RESOURCE_CAR_EXTENSION_NAME[] = L".car";
	int extensionNameLength = (int)::wcslen(RESOURCE_CAR_EXTENSION_NAME);

	// Fetch the launch file's path and determine where the extension starts in the string.
	auto launchFilePath = fLaunchSettings->LaunchFilePath->Data();
	int launchFilePathLength = (int)(fLaunchSettings->LaunchFilePath->Length());
	int launchFileExtensionStartIndex = launchFilePathLength - extensionNameLength;
	if (launchFileExtensionStartIndex <= 0)
	{
		return false;
	}

	// Determine if this runtime's launch file ends with a ".car" extension.
	bool isResourceCar = true;
	for (int index = 0; index < extensionNameLength; index++)
	{
		if (::towlower(launchFilePath[index + launchFileExtensionStartIndex]) != RESOURCE_CAR_EXTENSION_NAME[index])
		{
			isResourceCar = false;
			break;
		}
	}
	return isResourceCar;
}

void CoronaRuntimeEnvironment::UpdateOrientationAndSurfaceSize()
{
	// Do not continue if the Corona runtime is not currently running.
	if (!fRuntimePointer || (fRuntimeState != CoronaRuntimeState::Running))
	{
		return;
	}

	// Do not continue if a rendering surface was not provided to this runtime.
	if (nullptr == fCoronaControlAdapter)
	{
		return;
	}

	// Fetch the current and previous orientations.
	Rtt::DeviceOrientation::Type previousPageOrientation = fLastPageOrientation;
	auto currentPageOrientation = fCoronaControlAdapter->ParentPageProxy->Orientation->CoronaDeviceOrientation;
	auto currentSurfaceOrientation = fCoronaControlAdapter->RenderSurface->Orientation->CoronaDeviceOrientation;

	// Determine if there was an orientation change.
	// Note: On Windows Phone, a surface orientation change can happen when hot swapping between a
	//       Xaml DrawingSurface and DrawingSurfaceBackgroundGrid control.
	bool hasPageOrientationChanged = (currentPageOrientation != fLastPageOrientation);
	bool hasSurfaceOrientationChanged = (currentSurfaceOrientation != fLastSurfaceOrientation);

	// Determine if the content width/height was resized.
	// Note: An orientation change from portrait->landscape or vice-versa counts as a resize.
	bool hasSurfaceSizeChanged = false;
	if ((Rtt::DeviceOrientation::IsUpright(currentPageOrientation) != Rtt::DeviceOrientation::IsUpright(fLastPageOrientation)) ||
	    fRuntimePointer->GetDisplay().HasWindowSizeChanged())
	{
		hasSurfaceSizeChanged = true;
	}

	// Store the current orientation.
	fLastPageOrientation = currentPageOrientation;
	fLastSurfaceOrientation = currentSurfaceOrientation;

	// Update the renderer if a change was detected.
	if (hasPageOrientationChanged || hasSurfaceOrientationChanged || hasSurfaceSizeChanged)
	{
		if (hasPageOrientationChanged)
		{
			fRuntimePointer->GetDisplay().SetContentOrientation(currentPageOrientation);
		}
		if (fRuntimePointer->GetDisplay().HasWindowSizeChanged())
		{
			fRuntimePointer->GetDisplay().WindowSizeChanged();
		}
		fRuntimePointer->RestartRenderer(currentPageOrientation);
		fRuntimePointer->GetDisplay().Invalidate();
	}

	// Raise an orientation event if it has changed.
	if (hasPageOrientationChanged)
	{
		Rtt::OrientationEvent event(currentPageOrientation, previousPageOrientation);
		fRuntimePointer->DispatchEvent(event);
	}

	// Raise a resize event if it has changed.
	if (hasSurfaceSizeChanged)
	{
		fRuntimePointer->DispatchEvent(Rtt::ResizeEvent());
	}
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
