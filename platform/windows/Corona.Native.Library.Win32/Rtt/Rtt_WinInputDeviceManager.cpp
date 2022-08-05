//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinInputDeviceManager.h"
#include "Core\Rtt_Build.h"
#include "Display\Rtt_Display.h"
#include "Interop\Input\Key.h"
#include "Interop\Input\ModifierKeyStates.h"
#include "Interop\UI\Window.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "Interop\ScopedComInitializer.h"
#include "Rtt_Event.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_Runtime.h"
#include "Rtt_WinInputDevice.h"
#include "Rtt_WinPlatform.h"
#include <WindowsX.h>

namespace Rtt
{

#pragma region Static Members
/// <summary>
///  <para>Stores a handle to the "user32.dll" library loaded via the Win32 LoadLibrary() function.</para>
///  <para>Set to null if the library has not been loaded yet.</para>
/// </summary>
static HMODULE sUser32ModuleHandle;

typedef BOOL(WINAPI *RegisterTouchWindowCallback)(HWND, ULONG);
static RegisterTouchWindowCallback sRegisterTouchWindowCallback;

typedef BOOL(WINAPI *UnregisterTouchWindowCallback)(HWND);
static UnregisterTouchWindowCallback sUnregisterTouchWindowCallback;

typedef BOOL(WINAPI *CloseTouchInputHandleCallback)(HTOUCHINPUT);
static CloseTouchInputHandleCallback sCloseTouchInputHandleCallback;

typedef BOOL(WINAPI *GetTouchInputInfoCallback)(HTOUCHINPUT, UINT, PTOUCHINPUT, int);
static GetTouchInputInfoCallback sGetTouchInputInfoCallback;

#pragma endregion


#pragma region Constructors/Destructors

WinInputDeviceManager::WinInputDeviceManager(Interop::RuntimeEnvironment& environment)
:	PlatformInputDeviceManager(&environment.GetAllocator()),
	fEnvironment(environment),
	fRuntimeLoadedEventHandler(this, &WinInputDeviceManager::OnRuntimeLoaded),
	fRuntimeResumedEventHandler(this, &WinInputDeviceManager::OnRuntimeResumed),
	fRuntimeSuspendedEventHandler(this, &WinInputDeviceManager::OnRuntimeSuspended),
	fDiscoveredDeviceEventHandler(this, &WinInputDeviceManager::OnDiscoveredDevice),
	fReceivedMessageEventHandler(this, &WinInputDeviceManager::OnReceivedMessage),
	fIsMultitouchSupported(false),
	fIsCursorVisible(true),
	fIsWaitCursorEnabled(false),
	fCursorStyle(WinInputDeviceManager::CursorStyle::kDefaultArrow)
{
	// Initialize member variables.
	memset(&fTouchPointStates, 0, sizeof(fTouchPointStates));
	fIsLastMouseMovePointValid = false;

	// Add event handlers.
	fEnvironment.GetLoadedEventHandlers().Add(&fRuntimeLoadedEventHandler);
	fEnvironment.GetResumedEventHandlers().Add(&fRuntimeResumedEventHandler);
	fEnvironment.GetSuspendedEventHandlers().Add(&fRuntimeSuspendedEventHandler);
	fDeviceMonitor.GetDiscoveredDeviceEventHandlers().Add(&fDiscoveredDeviceEventHandler);
	auto surfacePointer = fEnvironment.GetRenderSurface();
	if (surfacePointer)
	{
		surfacePointer->GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
	}
}

WinInputDeviceManager::~WinInputDeviceManager()
{
	// Remove event handlers.
	fEnvironment.GetLoadedEventHandlers().Remove(&fRuntimeLoadedEventHandler);
	fEnvironment.GetResumedEventHandlers().Remove(&fRuntimeResumedEventHandler);
	fEnvironment.GetSuspendedEventHandlers().Remove(&fRuntimeSuspendedEventHandler);
	fDeviceMonitor.GetDiscoveredDeviceEventHandlers().Remove(&fDiscoveredDeviceEventHandler);
	auto surfacePointer = fEnvironment.GetRenderSurface();
	if (surfacePointer)
	{
		surfacePointer->GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);
	}

	// Remove the bindings to all Windows devices.
	// We must do this before the base class' destructor destroys all PlatformInputDevice objects or else they'll
	// crash attempting to use pointers to this derived class' InputDeviceMonitor objects that they're bound to.
	const ReadOnlyInputDeviceCollection deviceCollection = this->GetDevices();
	for (int index = deviceCollection.GetCount() - 1; index >= 0; index--)
	{
		auto windowsDevicePointer = dynamic_cast<WinInputDevice*>(deviceCollection.GetByIndex(index));
		if (windowsDevicePointer)
		{
			windowsDevicePointer->Unbind();
		}
	}

	// Unregister touchscreen input support.
	if (fEnvironment.GetRenderSurface() && sUnregisterTouchWindowCallback && fIsMultitouchSupported)
	{
		sUnregisterTouchWindowCallback(fEnvironment.GetRenderSurface()->GetWindowHandle());
	}
}

#pragma endregion


#pragma region Public Methods
bool WinInputDeviceManager::IsMultitouchSupported() const
{
	return fIsMultitouchSupported;
}

void WinInputDeviceManager::SetCursorVisible(bool value)
{
	// Do not continue if this setting isn't changing.
	if (value == fIsCursorVisible)
	{
		return;
	}

	// Store the given settings.
	fIsCursorVisible = value;

	// Update the mouse cursor.
	if (fIsCursorVisible)
	{
		// Show the currently assigned mouse cursor or "wait" mouse cursor.
		auto cursorName = fIsWaitCursorEnabled ? IDC_WAIT : MAKEINTRESOURCE(fCursorStyle);
		auto cursorHandle = ::LoadCursor(nullptr, cursorName);
		if (cursorHandle)
		{
			::SetCursor(cursorHandle);
		}
	}
	else
	{
		// Hide the mouse cursor.
		::SetCursor(nullptr);
	}
}

bool WinInputDeviceManager::IsCursorVisible() const
{
	return fIsCursorVisible;
}

void WinInputDeviceManager::SetCursor(WinInputDeviceManager::CursorStyle value)
{
	// Do not continue if this setting isn't changing.
	if (value == fCursorStyle)
	{
		return;
	}

	// Store the given setting.
	fCursorStyle = value;

	// Update the mouse cursor to the given style.
	if (fIsCursorVisible && !fIsWaitCursorEnabled)
	{
		auto cursorHandle = ::LoadCursor(nullptr, MAKEINTRESOURCE(fCursorStyle));
		if (cursorHandle)
		{
			::SetCursor(cursorHandle);
		}
	}
}

WinInputDeviceManager::CursorStyle WinInputDeviceManager::GetCursor() const
{
	return fCursorStyle;
}

void WinInputDeviceManager::SetWaitCursorEnabled(bool value)
{
	// Do not continue if this setting isn't changing.
	if (value == fIsWaitCursorEnabled)
	{
		return;
	}

	// Store the given setting.
	fIsWaitCursorEnabled = value;

	// Update the mouse cursor now.
	// Note: We also have to update the mouse cursor to use a "wait" icon everytime it moves when the
	//       WM_SETCURSOR windows message has been received. We'll do this in the OnReceivedMessage() method.
	auto cursorName = fIsWaitCursorEnabled ? IDC_WAIT : MAKEINTRESOURCE(fCursorStyle);
	auto cursorHandle = ::LoadCursor(nullptr, cursorName);
	if (cursorHandle)
	{
		::SetCursor(cursorHandle);
	}

	// Make sure that a child control does not have the focus when enabling the wait cursor.
	if (fIsWaitCursorEnabled)
	{
		auto renderSurfacePointer = fEnvironment.GetRenderSurface();
		if (renderSurfacePointer && ::IsChild(renderSurfacePointer->GetWindowHandle(), ::GetFocus()))
		{
			renderSurfacePointer->SetFocus();
		}
	}

	// If we're in the middle of handling a touch event, then cancel it.
	// We do this because mouse/touch events are supposed to be blocked while showing a wait cursor.
	if (fIsWaitCursorEnabled)
	{
		// Determine if we're in the middle of tracking at least 1 touch event.
		bool isTouching = false;
		for (uint32_t index = 0; index < kMaxTouchPoints; index++)
		{
			if (fTouchPointStates[index].HasStarted)
			{
				isTouching = true;
				break;
			}
		}

		// Dispatch a "touch" event set to canceled for each active touch point.
		if (isTouching)
		{
			TouchPointState touchPointStates[kMaxTouchPoints];
			memcpy(&touchPointStates, fTouchPointStates, sizeof(touchPointStates));
			memset(&fTouchPointStates, 0, sizeof(fTouchPointStates));
			for (uint32_t index = 0; index < kMaxTouchPoints; index++)
			{
				if (touchPointStates[index].HasStarted)
				{
					OnReceivedTouchEvent(
							index,
							touchPointStates[index].LastPoint,
							touchPointStates[index].StartPoint,
							Rtt::TouchEvent::kCancelled);
				}
			}
		}

		// Clear our tap detection handler.
		fTapTracker.Reset();
	}
}

bool WinInputDeviceManager::IsWaitCursorEnabled() const
{
	return fIsWaitCursorEnabled;
}

#pragma endregion


#pragma region Protected Methods
PlatformInputDevice* WinInputDeviceManager::CreateUsing(const InputDeviceDescriptor& descriptor)
{
	return Rtt_NEW(GetAllocator(), WinInputDevice(fEnvironment, descriptor));
}

void WinInputDeviceManager::Destroy(PlatformInputDevice* devicePointer)
{
	Rtt_DELETE(devicePointer);
}

#pragma endregion


#pragma region Private Methods
void WinInputDeviceManager::OnRuntimeLoaded(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
{
	// Enable input device (ie: game controller) support if:
	// - Running in Win32 desktop app mode.
	// - We're simulating a device that supports it, such as Android.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (!deviceSimulatorServicesPointer || deviceSimulatorServicesPointer->AreInputDevicesSupported())
	{
		// First, fetch all input devices currently connected to the system before starting the input device monitor.
		// This makes each devices' info and input events available to Lua.
		// This also prevents us from dispatching Lua "inputDeviceStatus" events upon starting the device monitor.
		// Note: This collection is typically empty unless multiple Corona runtimes have existed for the lifetime
		//       of the application, such as with the Corona Simulator.
		auto deviceCollection = fDeviceMonitor.GetDeviceCollection();
		const int kDeviceCount = deviceCollection.GetCount();
		for (int index = 0; index < kDeviceCount; index++)
		{
			auto deviceInterfacePointer = deviceCollection.GetByIndex(index);
			if (deviceInterfacePointer)
			{
				auto deviceType = deviceInterfacePointer->GetDeviceInfo()->GetType();
				auto coronaDevicePointer = dynamic_cast<WinInputDevice*>(this->Add(deviceType));
				if (coronaDevicePointer)
				{
					coronaDevicePointer->BindTo(deviceInterfacePointer);
				}
			}
		}

		// Start monitoring input devices and their key/axis input events.
		fDeviceMonitor.Start();
	}

	// Register the window for touchscreen input, if available.
	if (sender.GetRenderSurface())
	{
		// Fetch callbacks to the Win32 touch APIs. (Only available on Windows 7 and newer OS versions.)
		if (!sUser32ModuleHandle)
		{
			sUser32ModuleHandle = ::LoadLibraryW(L"User32");
			if (sUser32ModuleHandle)
			{
				sRegisterTouchWindowCallback = (RegisterTouchWindowCallback)::GetProcAddress(
						sUser32ModuleHandle, "RegisterTouchWindow");
				sUnregisterTouchWindowCallback = (UnregisterTouchWindowCallback)::GetProcAddress(
						sUser32ModuleHandle, "UnregisterTouchWindow");
				sCloseTouchInputHandleCallback = (CloseTouchInputHandleCallback)::GetProcAddress(
						sUser32ModuleHandle, "CloseTouchInputHandle");
				sGetTouchInputInfoCallback = (GetTouchInputInfoCallback)::GetProcAddress(
						sUser32ModuleHandle, "GetTouchInputInfo");
			}
		}

		// Register the window for touch input. Allows us to receive WM_TOUCH messages.
		if (sRegisterTouchWindowCallback)
		{
			auto wasRegistered = sRegisterTouchWindowCallback(sender.GetRenderSurface()->GetWindowHandle(), 0);
			if (wasRegistered)
			{
				fIsMultitouchSupported = true;
			}
		}
	}

	// Set up this app to show a virtual keyboard when a native text input control has been tapped on via a trouchscreen.
	// Notes:
	// - This is a Windows 8 exclusive feature which doesn't do this by default for Win32 apps. (Only WinRT apps.)
	// - This COM interface is not available on Windows 10, but that OS version already does this by default.
	// - We only need to enable this when the Corona runtime has a window surface to render to.
	if (sender.GetRenderSurface())
	{
		try
		{
			// Define prototype and GUIDs to Microsoft's IInputPanelConfiguration COM interface.
			// Note: These are defined in Microsoft's "inputpanelconfiguration.h" header file,
			//       but this header is not available in Visual Studio 2013. So, define them ourselves.
			class IInputPanelConfiguration : public IUnknown
			{
				public:
					virtual HRESULT STDMETHODCALLTYPE EnableFocusTracking() = 0;
			};
			struct __declspec(uuid("2853ADD3-F096-4C63-A78F-7FA3EA837FB7")) Corona_CLSID_IInputPanelConfiguration;
			struct __declspec(uuid("41C81592-514C-48BD-A22E-E6AF638521A6")) Corona_IID_IInputPanelConfiguration;

			// Attempt to aquire the Windows 8 IInputPanelConfiguration COM object and enable focus tracking.
			// Will return null on any other Windows OS version, which is okay.
			auto comApartmentType = Interop::ScopedComInitializer::ApartmentType::kSingleThreaded;
			Interop::ScopedComInitializer scopedComInitializer(comApartmentType);
			IInputPanelConfiguration* inputPanelConfigPointer = nullptr;
			::CoCreateInstance(
					_uuidof(Corona_CLSID_IInputPanelConfiguration), nullptr, CLSCTX_INPROC_SERVER,
					_uuidof(Corona_IID_IInputPanelConfiguration), (LPVOID*)&inputPanelConfigPointer);
			if (inputPanelConfigPointer)
			{
				inputPanelConfigPointer->EnableFocusTracking();
				inputPanelConfigPointer->Release();
			}
		}
		catch (...) {}
	}
}

void WinInputDeviceManager::OnRuntimeResumed(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
{
	// Start monitoring input devices and their key/axis input events.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (!deviceSimulatorServicesPointer || deviceSimulatorServicesPointer->AreInputDevicesSupported())
	{
		fDeviceMonitor.Start();
	}
}

void WinInputDeviceManager::OnRuntimeSuspended(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
{
	// Stop monitoring input devices while the Corona runtime is suspended.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (!deviceSimulatorServicesPointer || deviceSimulatorServicesPointer->AreInputDevicesSupported())
	{
		fDeviceMonitor.Stop();
	}
}

void WinInputDeviceManager::OnDiscoveredDevice(
	Interop::Input::InputDeviceMonitor& sender, Interop::Input::InputDeviceInterfaceEventArgs& arguments)
{
	WinInputDevice* coronaDevicePointer;

	// Fetch a pointer to the newly discovered input device.
	auto deviceIterfacePointer = &arguments.GetDeviceInterface();

	// Do not continue if the discovered device has already been added to Corona's collection.
	const ReadOnlyInputDeviceCollection& deviceCollection = GetDevices();
	for (int index = deviceCollection.GetCount(); index >= 0; index--)
	{
		coronaDevicePointer = dynamic_cast<WinInputDevice*>(deviceCollection.GetByIndex(index));
		if (coronaDevicePointer && (coronaDevicePointer->GetDeviceInterface() == deviceIterfacePointer))
		{
			return;
		}
	}

	// Set up a Corona binding to the newly discovered input device.
	// This makes the device's info and input events available to Lua.
	auto deviceType = arguments.GetDeviceInterface().GetDeviceInfo()->GetType();
	coronaDevicePointer = dynamic_cast<WinInputDevice*>(this->Add(deviceType));
	if (!coronaDevicePointer)
	{
		return;
	}
	coronaDevicePointer->BindTo(&arguments.GetDeviceInterface());

	// Dispatch an "inputDeviceStatus" event to Lua.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (runtimePointer)
	{
		bool hasConnectionStateChanged = true;
		bool wasReconfigured = false;
		Rtt::InputDeviceStatusEvent event(coronaDevicePointer, hasConnectionStateChanged, wasReconfigured);
		runtimePointer->DispatchEvent(event);
	}
}

void WinInputDeviceManager::OnReceivedMessage(
	Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments)
{
	static const int kTapTolerance = 1;

	// Do not continue if the message was already handled.
	if (arguments.WasHandled())
	{
		return;
	}

	// Do not continue if Corona is not currently running.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (!runtimePointer || runtimePointer->IsSuspended())
	{
		return;
	}

	// Handle the received message.
	switch (arguments.GetMessageId())
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			// Ignore mouse events while the wait cursor is displayed.
			if (fIsWaitCursorEnabled)
			{
				break;
			}

			// Dispatch a "mouse" event to Corona.
			POINT point = GetMousePointFrom(arguments.GetLParam());
			OnReceivedMouseEvent(Rtt::MouseEvent::kDown, point, 0, 0, arguments.GetWParam());

			// This function call allows us to receive the WM_LBUTTONUP message.
			::SetCapture(sender.GetWindowHandle());

			// Simulate a touch event if the message came from a mouse.
			auto touchInputStatePointer = &fTouchPointStates[0];
			if (!touchInputStatePointer->HasStarted && !WasMouseMessageGeneratedFromTouchInput())
			{
				// Store the simulated touch state.
				touchInputStatePointer->HasStarted = true;
				touchInputStatePointer->StartPoint = point;
				touchInputStatePointer->LastPoint = point;

				// Update our simulated touch "tap" tracker for a single or double mouse click.
				// Note: The Lua "tap" event will be dispatched later, when the mouse button has been released.
				fTapTracker.Reset();
				fTapTracker.SetInputDeviceType(Interop::Input::TapTracker::InputDeviceType::kMouse);
				fTapTracker.UpdateWith(point, Rtt::TouchEvent::kBegan);
				if (arguments.GetMessageId() == WM_LBUTTONDBLCLK)
				{
					fTapTracker.UpdateWith(point, Rtt::TouchEvent::kEnded);
					fTapTracker.UpdateWith(point, Rtt::TouchEvent::kBegan);
				}

				// Dispatch a "touch" event to Corona.
				OnReceivedTouchEvent(0, point, point, Rtt::TouchEvent::kBegan);
			}

			// Flag the message as handled.
			arguments.SetReturnResult(0);
			arguments.SetHandled();
			break;
		}
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		{
			// Ignore mouse events while the wait cursor is displayed.
			if (fIsWaitCursorEnabled)
			{
				break;
			}

			// Dispatch a "mouse" event to Corona.
			// Note: We do not treat middle and right mouse button drags as touch events.
			POINT point = GetMousePointFrom(arguments.GetLParam());
			OnReceivedMouseEvent(Rtt::MouseEvent::kDown, point, 0, 0, arguments.GetWParam());

			// Flag the message as handled.
			arguments.SetReturnResult(0);
			arguments.SetHandled();
			break;
		}
		case WM_MOUSEMOVE:
		{
			// Ignore mouse events while the wait cursor is displayed.
			if (fIsWaitCursorEnabled)
			{
				break;
			}

			// Fetch current mouse position.
			POINT point = GetMousePointFrom(arguments.GetLParam());

			// Do not continue if the mouse position hasn't changed. This prevents duplicate events.
			// Note: Windows sometimes sends mouse move messages when the mouse hasn't actually moved.
			//       For example, displaying the Windows "Task Manager" can trigger a mouse move message once a second.
			//       Microsoft's WM_SETREDRAW, MFC's SetRedraw(TRUE), and .NET's EndUpdate() triggers this issue.
			if (fIsLastMouseMovePointValid && (fLastMouseMovePoint.x == point.x) && (fLastMouseMovePoint.y == point.y))
			{
				break;
			}
			fLastMouseMovePoint = point;
			fIsLastMouseMovePointValid = true;

			// Dispatch a "mouse" event to Corona.
			OnReceivedMouseEvent(Rtt::MouseEvent::kMove, point, 0, 0, arguments.GetWParam());

			// Dispatch a "touch" event to Corona, but only if the left mouse button is down.
			auto touchInputStatePointer = &fTouchPointStates[0];
			if (touchInputStatePointer->HasStarted && !WasMouseMessageGeneratedFromTouchInput() &&
			    (::GetCapture() == sender.GetWindowHandle()) && (arguments.GetWParam() & MK_LBUTTON))
			{
				touchInputStatePointer->LastPoint = point;
				OnReceivedTouchEvent(0, point, touchInputStatePointer->StartPoint, Rtt::TouchEvent::kMoved);
			}

			// Flag the message as handled.
			arguments.SetReturnResult(0);
			arguments.SetHandled();
			break;
		}
		case WM_LBUTTONUP:
		{
			// Fetch current mouse position.
			POINT point = GetMousePointFrom(arguments.GetLParam());

			// Must be called after calling SetCapture() in OnLButtonDown().
			::ReleaseCapture();

			// Ignore mouse events while the wait cursor is displayed.
			// Note: This must be done after a call to ReleaseCapture() to stop monitoring mouse drags.
			if (fIsWaitCursorEnabled)
			{
				break;
			}

			// Dispatch a "mouse" event to Corona.
			OnReceivedMouseEvent(Rtt::MouseEvent::kUp, point, 0, 0, arguments.GetWParam());

			// Only dispatch a "touch" and "tap" event if:
			// - Corona has dispatched a "began" touch event phase.
			// - The touch events were not canceled, such as by showing a wait cursor.
			auto touchInputStatePointer = &fTouchPointStates[0];
			if (touchInputStatePointer->HasStarted && !WasMouseMessageGeneratedFromTouchInput())
			{
				// Clear the touch tracking flag.
				touchInputStatePointer->HasStarted = false;
				touchInputStatePointer->LastPoint = point;

				// Dispatch a "touch" event to Corona.
				OnReceivedTouchEvent(0, point, touchInputStatePointer->StartPoint, Rtt::TouchEvent::kEnded);

				// Dispatch a "tap" event if mouse button was released in the same position it was pressed in.
				fTapTracker.SetInputDeviceType(Interop::Input::TapTracker::InputDeviceType::kMouse);
				fTapTracker.UpdateWith(point, Rtt::TouchEvent::kEnded);
				if (fTapTracker.HasTapOccurred())
				{
					Rtt::TapEvent event(Rtt_IntToReal(point.x), Rtt_IntToReal(point.y), fTapTracker.GetTapCount());
					runtimePointer->DispatchEvent(event);
				}
			}

			// Flag the message as handled.
			arguments.SetReturnResult(0);
			arguments.SetHandled();
			break;
		}
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			// Ignore mouse events while the wait cursor is displayed.
			if (fIsWaitCursorEnabled)
			{
				break;
			}

			// Dispatch a "mouse" event to Corona.
			// Note: We do not treat middle and right mouse button drags as touch events.
			POINT point = GetMousePointFrom(arguments.GetLParam());
			OnReceivedMouseEvent(Rtt::MouseEvent::kUp, point, 0, 0, arguments.GetWParam());

			// Flag the message as handled.
			arguments.SetReturnResult(0);
			arguments.SetHandled();
			break;
		}
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		{
			// Ignore mouse events while the wait cursor is displayed.
			if (fIsWaitCursorEnabled)
			{
				break;
			}

			// Fetch current mouse position.
			POINT point;
			point.x = GET_X_LPARAM(arguments.GetLParam());
			point.y = GET_Y_LPARAM(arguments.GetLParam());
			::ScreenToClient(sender.GetWindowHandle(), &point);
			point = GetMousePointFrom(MAKELPARAM(point.x, point.y));

			// Fetch the distance traveled by the vertical or horizontal scroll wheel.
			// This is measured in Win32 "WHEEL_DELTA" units, where a value of 120 is considered 1 unit.
			float scrollWheelDeltaX = 0;
			float scrollWheelDeltaY = 0;
			if (arguments.GetMessageId() == WM_MOUSEWHEEL)
			{
				scrollWheelDeltaY = (float)GET_WHEEL_DELTA_WPARAM(arguments.GetWParam());
			}
			else
			{
				scrollWheelDeltaX = (float)GET_WHEEL_DELTA_WPARAM(arguments.GetWParam());
			}

			// Convert the scroll wheel Win32 deltas to Corona content coordinate deltas.
			// This way they can be easily used to translate/offset display objects in Lua.
			// Note: The Win32 scroll values are not in pixels. It is typically used as a scale (value / WHEEL_DELTA) 
			//       which is multiplied against the system's default number of text lines to scroll vertically
			//       and default characters to scroll horizontally.
			//       These line/character scrolling defaults are settable in Windows' "Mouse Properties" dialog.
			auto runtimePointer = fEnvironment.GetRuntime();
			if (runtimePointer && (WHEEL_DELTA > 0))
			{
				float fontSize = Rtt_RealToFloat(runtimePointer->Platform().GetStandardFontSize());
				if (scrollWheelDeltaX != 0)
				{
					// Scroll horizontally in characters based on the app's default font size.
					UINT charactersToScroll = 3;
					::SystemParametersInfoW(SPI_GETWHEELSCROLLCHARS, 0, &charactersToScroll, 0);
					if (charactersToScroll < 1)
					{
						charactersToScroll = 1;
					}
					scrollWheelDeltaX *= (fontSize * (float)charactersToScroll) / (float)WHEEL_DELTA;
					scrollWheelDeltaX *= runtimePointer->GetDisplay().GetSxUpright();
				}
				if (scrollWheelDeltaY != 0)
				{
					UINT linesToScroll = 3;
					::SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &linesToScroll, 0);
					if (linesToScroll < 1)
					{
						linesToScroll = 1;
					}
					if (linesToScroll != UINT_MAX)
					{
						// Scroll vertically in lines of text based on the app's default font size.
						// Also, invert the scroll so that down heads towards the positive direction.
						scrollWheelDeltaY *= (fontSize * (float)linesToScroll) / (float)WHEEL_DELTA;
						scrollWheelDeltaY *= -1.0f;
						scrollWheelDeltaY *= runtimePointer->GetDisplay().GetSyUpright();
					}
					else
					{
						// The system is set up to scroll by 1 page/screen at a time.
						// So, scroll by the content height.
						bool isScrollingUp = (scrollWheelDeltaY >= 0);
						scrollWheelDeltaY = (float)runtimePointer->GetDisplay().ContentHeight();
						if (isScrollingUp)
						{
							scrollWheelDeltaY *= -1.0f;
						}
					}
				}
			}

			// Dispatch a "mouse" event to Corona.
			OnReceivedMouseEvent(
					Rtt::MouseEvent::kScroll, point, scrollWheelDeltaX, scrollWheelDeltaY, arguments.GetWParam());

			// Flag the message as handled.
			arguments.SetReturnResult(0);
			arguments.SetHandled();
			break;
		}
		case WM_TOUCH:
		{
			// We've received new touchscreen input information.
			auto touchInputCount = LOWORD(arguments.GetWParam());
			auto touchInputHandle = (HTOUCHINPUT)arguments.GetLParam();
			if (sGetTouchInputInfoCallback && touchInputHandle && (touchInputCount > 0) && !fIsWaitCursorEnabled)
			{
				// Fetch all of the touch input information received, up to "kMaxTouchPoints".
				// Note: Our "kMaxTouchPoints" maximum is a Win32 Corona limitation, not a Microsoft limitation.
				TOUCHINPUT touchInputs[kMaxTouchPoints];
				if (touchInputCount > kMaxTouchPoints)
				{
					touchInputCount = kMaxTouchPoints;
				}
				auto hasReceivedTouchInput = sGetTouchInputInfoCallback(
						touchInputHandle, touchInputCount, touchInputs, sizeof(TOUCHINPUT));
				if (!hasReceivedTouchInput)
				{
					touchInputCount = 0;
				}

				// Handle the received touch inputs.
				for (int touchInputIndex = 0; touchInputIndex < touchInputCount; touchInputIndex++)
				{
					// Fetch the next touch input object.
					auto touchInputPointer = &touchInputs[touchInputIndex];

					// Fetch the touch phase.
					Rtt::TouchEvent::Phase phase;
					switch (touchInputPointer->dwFlags & 0x7)
					{
						case TOUCHEVENTF_DOWN:
							phase = Rtt::TouchEvent::kBegan;
							break;
						case TOUCHEVENTF_MOVE:
							phase = Rtt::TouchEvent::kMoved;
							break;
						case TOUCHEVENTF_UP:
							phase = Rtt::TouchEvent::kEnded;
							break;
						default:
							continue;
					}

					// Fetch the touch input's current position in Corona content coordinates.
					POINT point;
					point.x = TOUCH_COORD_TO_PIXEL(touchInputPointer->x);
					point.y = TOUCH_COORD_TO_PIXEL(touchInputPointer->y);
					auto windowHandle = arguments.GetWindowHandle();
					::ScreenToClient(windowHandle, &point);
					auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
					if (deviceSimulatorServicesPointer)
					{
						point = deviceSimulatorServicesPointer->GetSimulatedPointFromClient(point);
					}

					// Fetch a touch state element in our "fTouchPointStates" array matching the received touch point.
					// We use this to track the finger's start touch, last touch, and other stateful information.
					int touchPointStateIndex = -1;
					if (touchInputPointer->dwFlags & TOUCHEVENTF_PRIMARY)
					{
						// *** This is the 1st finger to be pressed on the screen. ***
						// Always select the first element in our "fTouchPointStates" in this case.
						touchPointStateIndex = 0;
					}
					else
					{
						// *** This is the 2nd, 3rd, etc. finger on the screen. ***

						// First, check if we're already tracking the received touch point.
						int availableTouchPointStateIndex = -1;
						for (int index = 1; index < kMaxTouchPoints; index++)
						{
							if (fTouchPointStates[index].HasStarted)
							{
								if (fTouchPointStates[index].TouchInputId == touchInputPointer->dwID)
								{
									touchPointStateIndex = index;
									break;
								}
							}
							else if (availableTouchPointStateIndex < 0)
							{
								availableTouchPointStateIndex = index;
							}
						}

						// If we're not tracking the recieved touch point, then select an available slot in the array.
						if ((touchPointStateIndex < 0) && (availableTouchPointStateIndex >= 0))
						{
							touchPointStateIndex = availableTouchPointStateIndex;
						}
					}
					if (touchPointStateIndex < 0)
					{
						continue;
					}

					// Store the touch input's current state.
					auto touchPointStatePointer = &fTouchPointStates[touchPointStateIndex];
					touchPointStatePointer->TouchInputId = touchInputPointer->dwID;
					touchPointStatePointer->LastPoint = point;
					if (phase == Rtt::TouchEvent::kBegan)
					{
						if (touchPointStatePointer->HasStarted)
						{
							phase = Rtt::TouchEvent::kMoved;
						}
						else
						{
							touchPointStatePointer->HasStarted = true;
							touchPointStatePointer->StartPoint = point;
						}
					}
					else if (phase == Rtt::TouchEvent::kMoved)
					{
						if (!touchPointStatePointer->HasStarted)
						{
							continue;
						}
					}
					else if (phase == Rtt::TouchEvent::kEnded)
					{
						if (!touchPointStatePointer->HasStarted)
						{
							continue;
						}
						touchPointStatePointer->HasStarted = false;
					}

					// Dispatch a Lua "touch" event.
					OnReceivedTouchEvent(
							(uint32_t)touchPointStateIndex, touchPointStatePointer->LastPoint,
							touchPointStatePointer->StartPoint, phase);

					// If this is the first/primary finger on the screen, then determine if a "tap" has occurred.
					if (touchInputPointer->dwFlags & TOUCHEVENTF_PRIMARY)
					{
						fTapTracker.SetInputDeviceType(Interop::Input::TapTracker::InputDeviceType::kTouchscreen);
						fTapTracker.UpdateWith(point, phase);
						if (fTapTracker.HasTapOccurred())
						{
							Rtt::TapEvent event(Rtt_IntToReal(point.x), Rtt_IntToReal(point.y), fTapTracker.GetTapCount());
							runtimePointer->DispatchEvent(event);
						}
					}
				}
			}

			// Finalize the above touch handling.
			if (arguments.WasHandled() && touchInputHandle && sCloseTouchInputHandleCallback)
			{
				// We've successful loaded and handled the touch input above.
				// We must call the Win32 CloseTouchInputHandle() function after calling GetTouchInputInfo().
				sCloseTouchInputHandleCallback(touchInputHandle);
			}
			else
			{
				// We were not able to acquire touch input above.
				// So, we must pass the touch input to Microsoft's default handler or else an assert will occur.
				arguments.SetReturnResult(::DefWindowProc(
						arguments.GetWindowHandle(), arguments.GetMessageId(),
						arguments.GetWParam(), arguments.GetLParam()));
				arguments.SetHandled();
			}
			break;
		}
		case WM_SETCURSOR:
		{
			// Do not continue if the mouse cursor is not within the client area of the window.
			if (LOWORD(arguments.GetLParam()) != HTCLIENT)
			{
				break;
			}

			// Display a "wait" mouse cursor if enabled.
			// This setting overrides the assigned cursor style and is not allowed to be hidden.
			if (fIsWaitCursorEnabled)
			{
				auto cursorHandle = ::LoadCursor(nullptr, IDC_WAIT);
				if (cursorHandle)
				{
					::SetCursor(cursorHandle);
					arguments.SetReturnResult(0);
					arguments.SetHandled();
					break;
				}
			}

			// Do not change the mouse cursor style while it's hovering over a child control.
			if (arguments.GetWindowHandle() != (HWND)arguments.GetWParam())
			{
				break;
			}

			// Update the current mouse cursor style.
			if (fIsCursorVisible)
			{
				// Show the assigned mouse cursor.
				// Optimization: If we do not handle this event, then the OS will display an arrow cursor for us.
				if ((WORD)fCursorStyle != (WORD)IDC_ARROW)
				{
					auto cursorHandle = ::LoadCursor(nullptr, MAKEINTRESOURCE(fCursorStyle));
					if (cursorHandle)
					{
						::SetCursor(cursorHandle);
						arguments.SetReturnResult(0);
						arguments.SetHandled();
					}
				}
			}
			else
			{
				// Hide the mouse cursor.
				::SetCursor(nullptr);
				arguments.SetReturnResult(0);
				arguments.SetHandled();
			}
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			// Ignore key events while the wait cursor is displayed.
			if (fIsWaitCursorEnabled)
			{
				break;
			}

			// Do not continue if simulating a device that does not support key events.
			auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
			if (deviceSimulatorServicesPointer && (deviceSimulatorServicesPointer->AreKeyEventsSupported() == false))
			{
				break;
			}

			// Determine if the key state is down or up.
			bool isKeyDown = (arguments.GetMessageId() == WM_KEYDOWN);

			// Ignore the key event if it is being repeated.
			// Note: If bit 30 (zero based) is set to 1, then the key was previously down.
			bool wasKeyPreviouslyDown = (arguments.GetLParam() & 0x40000000) ? true : false;
			if ((isKeyDown && wasKeyPreviouslyDown) || (!isKeyDown && !wasKeyPreviouslyDown))
			{
				arguments.SetReturnResult(0);
				arguments.SetHandled();
				break;
			}

			// Fetch the key code that was pressed/released.
			S32 keyCode = (S32)arguments.GetWParam();

			// Do not continue if simulating a device that does not support key events from the keyboard.
			// Special Case: If the device supports a back button (like Android and WP8), then let it through.
			if (deviceSimulatorServicesPointer &&
			    (deviceSimulatorServicesPointer->AreKeyEventsFromKeyboardSupported() == false))
			{
				if (keyCode != VK_BROWSER_BACK)
				{
					break;
				}
				else if (deviceSimulatorServicesPointer->IsBackKeySupported() == false)
				{
					break;
				}
			}

			// If the key code for "shift", "alt", or "ctrl" has been received, then determine if
			// the left/right version of that key was pressed/released by its scan code.
			if ((VK_SHIFT == keyCode) || (VK_MENU == keyCode) || (VK_CONTROL == keyCode))
			{
				UINT scanCode = (arguments.GetLParam() >> 16) & 0xFF;
				S32 result = (S32)MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
				if (result != 0)
				{
					keyCode = result;
				}
			}

			// Fetch the current state of the "shift", "alt", and "ctrl" keys.
			auto modifierKeyStates = Interop::Input::ModifierKeyStates::FromKeyboard();

			// Dispatch a "key" event to Lua.
			auto keyInfo = Interop::Input::Key::FromNativeCode(keyCode);
			Rtt::KeyEvent keyEvent(
					nullptr, isKeyDown ? Rtt::KeyEvent::kDown : Rtt::KeyEvent::kUp,
					keyInfo.GetCoronaName(), keyCode,
					modifierKeyStates.IsShiftDown(), modifierKeyStates.IsAltDown(),
					modifierKeyStates.IsControlDown(), modifierKeyStates.IsCommandDown());
			fEnvironment.GetRuntime()->DispatchEvent(keyEvent);
			if (keyEvent.GetResult())
			{
				// The Lua key listener returned true. Flag the Windows message as handled.
				arguments.SetReturnResult(0);
				arguments.SetHandled();
			}
			else
			{
				// The Lua key listener returned false or nothing, which means Lua did not handle it.

				// If we're simulating a device that supports a "back" key, then terminate the runtime when
				// the back key has been released. This mimics the behavior on Android and WP8.
				if (deviceSimulatorServicesPointer && deviceSimulatorServicesPointer->IsBackKeySupported() &&
				    (VK_BROWSER_BACK == keyCode) && (false == isKeyDown))
				{
					deviceSimulatorServicesPointer->RequestTerminate();
					arguments.SetReturnResult(0);
					arguments.SetHandled();
				}
			}
			break;
		}
		case WM_CHAR:
		{
			wchar_t wParam = (wchar_t)arguments.GetWParam();
			wchar_t wParamArray[2] = { wParam, 0 };

			WinString stringConverter;
			stringConverter.SetUTF16(wParamArray);
			int utf8Length = strlen(stringConverter.GetUTF8()) + 1;
			char * utf8Character = new char[utf8Length];
			strcpy_s(utf8Character, utf8Length, stringConverter.GetUTF8());
			if (strlen(utf8Character) > 1 || isprint(utf8Character[0]))
			{
				Rtt::CharacterEvent characterEvent(nullptr, utf8Character);
				runtimePointer->DispatchEvent(characterEvent);
			}
			break;
		}
		case WM_APPCOMMAND:
		{
//TODO: WM_APPCOMMAND messages need to be handled by the main window too because these messages
//      will not be received here if the mouse is hovering outside of the render surface control.

			// We've received an "Application Command" message from a media keyboard or mouse.
			auto appCommandId = GET_APPCOMMAND_LPARAM(arguments.GetLParam());
			switch (appCommandId)
			{
				case APPCOMMAND_BROWSER_BACKWARD:
				case APPCOMMAND_BROWSER_FORWARD:
				{
					// A browse back/forward command has been received.
					// Simulate its equivalent Windows key messages to be delivered to Corona as Lua key events.
					Interop::UI::MessageSettings messageSettings;
					messageSettings.WindowHandle = arguments.GetWindowHandle();
					if (APPCOMMAND_BROWSER_BACKWARD == appCommandId)
					{
						messageSettings.WParam = VK_BROWSER_BACK;
					}
					else
					{
						messageSettings.WParam = VK_BROWSER_FORWARD;
					}

					// Simulate a Windows key down message.
					messageSettings.MessageId = WM_KEYDOWN;
					messageSettings.LParam = 0;
					Interop::UI::HandleMessageEventArgs keyDownMessageEventArgs(messageSettings);
					this->OnReceivedMessage(sender, keyDownMessageEventArgs);

					// Simulate a Windows key up message.
					messageSettings.MessageId = WM_KEYUP;
					messageSettings.LParam = 0x40000000L;
					Interop::UI::HandleMessageEventArgs keyUpMessageEventArgs(messageSettings);
					this->OnReceivedMessage(sender, keyUpMessageEventArgs);

					// If the above key messages were handled, then set the return result to true.
					// Returning true prevents this application command from be passed to the parent window or desktop.
					if (keyDownMessageEventArgs.WasHandled() || keyUpMessageEventArgs.WasHandled())
					{
						arguments.SetReturnResult((LRESULT)TRUE);
						arguments.SetHandled();
					}
					break;
				}
			}
			break;
		}
	}
}

void WinInputDeviceManager::OnReceivedMouseEvent(
	Rtt::MouseEvent::MouseEventType eventType, POINT& point,
	float scrollWheelDeltaX, float scrollWheelDeltaY, WPARAM mouseButtonFlags)
{
	// Do not continue if simulating a device that does not support mouse events.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer && (deviceSimulatorServicesPointer->IsMouseSupported() == false))
	{
		return;
	}

	// Fetch the Corona runtime.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (!runtimePointer)
	{
		return;
	}

	// Determine which mouse buttons are primary and secondary.
	// Typically, the left mouse button is the primary, unless it has been swapped in the Control Panel.
	bool areMouseButtonsNotSwapped = ::GetSystemMetrics(SM_SWAPBUTTON) ? false : true;
	UINT primaryButtonMask = areMouseButtonsNotSwapped ? MK_LBUTTON : MK_RBUTTON;
	UINT secondaryButtonMask = areMouseButtonsNotSwapped ? MK_RBUTTON : MK_LBUTTON;

	// Fetch the mouse's current up/down buttons states.
	bool isPrimaryDown = (mouseButtonFlags & primaryButtonMask) ? true : false;
	bool isSecondaryDown = (mouseButtonFlags & secondaryButtonMask) ? true : false;
	bool isMiddleDown = (mouseButtonFlags & MK_MBUTTON) ? true : false;

	// Determine if this is a "drag" event.
	if ((Rtt::MouseEvent::kMove == eventType) && (isPrimaryDown || isSecondaryDown || isMiddleDown))
	{
		eventType = Rtt::MouseEvent::kDrag;
	}

	// Fetch the current state of the "shift", "alt", and "ctrl" keys.
	auto modifierKeyStates = Interop::Input::ModifierKeyStates::FromKeyboard();

	// Dispatch the mouse event to Lua.
	Rtt::MouseEvent mouseEvent(
			eventType,
			Rtt_IntToReal(point.x), Rtt_IntToReal(point.y),
			Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0,
			isPrimaryDown, isSecondaryDown, isMiddleDown,
			modifierKeyStates.IsShiftDown(), modifierKeyStates.IsAltDown(),
			modifierKeyStates.IsControlDown(), modifierKeyStates.IsCommandDown());
	runtimePointer->DispatchEvent(mouseEvent);
}

void WinInputDeviceManager::OnReceivedTouchEvent(
	uint32_t touchIndex, POINT currentPosition, POINT startPosition, Rtt::TouchEvent::Phase phase)
{
	// Fetch the Corona runtime.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (!runtimePointer)
	{
		return;
	}

	// Dispatch a "touch" event to Lua.
	Rtt::TouchEvent event(
			Rtt_IntToReal(currentPosition.x),
			Rtt_IntToReal(currentPosition.y),
			Rtt_IntToReal(startPosition.x),
			Rtt_IntToReal(startPosition.y),
			phase);
	event.SetId((const void*)(touchIndex + 1));
	if (fEnvironment.GetPlatform()->GetDevice().DoesNotify(Rtt::MPlatformDevice::kMultitouchEvent))
	{
		runtimePointer->DispatchEvent(Rtt::MultitouchEvent(&event, 1));
	}
	else if (0 == touchIndex)
	{
		runtimePointer->DispatchEvent(event);
	}
}

POINT WinInputDeviceManager::GetMousePointFrom(LPARAM LParam)
{
	// Fetch the mouse coordinate from the Windows message's LPARAM.
	POINT point;
	point.x = GET_X_LPARAM(LParam);
	point.y = GET_Y_LPARAM(LParam);

	// If simulating a device, then adjust the point for the simulator window's current rotation and zoom level.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		point = deviceSimulatorServicesPointer->GetSimulatedPointFromClient(point);
	}

	// Return the mouse coordinate.
	return point;
}

bool WinInputDeviceManager::WasMouseMessageGeneratedFromTouchInput()
{
	// Microsoft documents that this is how you detect if a Win32 mouse message was generated by a touch event.
	// Unfortunately, Microsoft does not provide any constants for this.
	return ((::GetMessageExtraInfo() & 0xFFFFFF00) == 0xFF515700);
}

#pragma endregion

}	// namespace Rtt
