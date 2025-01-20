//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputDeviceMonitor.h"
#include "Interop\UI\MessageOnlyWindow.h"
#include "Interop\ScopedComInitializer.h"
#include "Interop\Ticks.h"
#include "DirectInputDeviceHandler.h"
#include "InputAxisSettings.h"
#include "InputDeviceInfo.h"
#include "InputDeviceInterface.h"
#include "InputDeviceSettings.h"
#include "MInputDeviceHandler.h"
#include "XInputDeviceHandler.h"
#include "XInputInterface.h"
#include <algorithm>
#include <Dbt.h>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <vector>
#include <Windows.h>
#include <Xinput.h>

// Include the "InitGuid.h" header before including the DirectInput header.
// This allows us to use its GUIDs without linking to its lib at compile time.
#include <InitGuid.h>
#include <dinput.h>


namespace Interop { namespace Input {

#pragma region Private Structures
/// <summary>
///  Private structure used to copy the worker thread's InputDeviceContext info to an InputDeviceMonitor
///  instance's InputDeviceContext.
/// </summary>
struct InputDeviceContextSettings
{
	int DriverTypeIntegerId;
	Rtt::InputDeviceConnectionState ConnectionState;
	std::shared_ptr<InputDeviceInfo> DeviceInfoPointer;
	const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* VibrationRequestHandlerPointer;
};

/// <summary>Structure intended to be passed as the context parameter to the IDirectInput8::EnumDevices() method.</summary>
struct DirectInputEnumDevicesContext
{
	/// <summary>
	///  <para>Set of vendor ID and product ID pairs to be excluded when fetching DirectInput devices.</para>
	///  <para>
	///   Intended to exclude XInput devices so that they're not controlled by DirectInput and XInput at the same time.
	///  </para>
	/// </summary>
	std::unordered_set<DWORD> VidPidExclusionSet;

	/// <summary>
	///  <para>Collection of instance GUIDs for all DirectInput devices currently connected to the system.</para>
	///  <para>This collection is expected to exclude devices matching the IDs in the "VidPidExclusionSet" set.</para>
	/// </summary>
	std::vector<GUID> DeviceInstanceGuids;
};

#pragma endregion


#pragma region Static Member Variables
/// <summary>Mutex used to synchronize access to this class' static member variables.</summary>
static std::recursive_mutex sMutex;

/// <summary>
///  Handler to be invoked when the "StatusChanged" event has been raised by a device handler
///  belonging to the worker thread.
/// </summary>
InputDeviceInterface::StatusChangedEvent::FunctionHandler InputDeviceMonitor::sDeviceStatusChangedEventHandler(&InputDeviceMonitor::OnAsyncDeviceStatusChanged);

/// <summary>
///  Handler to be invoked when the "ReceivedAxisInput" event has been raised by a device handler
///  belonging to the worker thread.
/// </summary>
InputDeviceInterface::ReceivedAxisInputEvent::FunctionHandler InputDeviceMonitor::sReceivedAxisInputEventHandler(&InputDeviceMonitor::OnAsyncReceivedAxisInput);

/// <summary>
///  Handler to be invoked when the "ReceivedKeyInput" event has been raised by a device handler
///  belonging to the worker thread.
/// </summary>
InputDeviceInterface::ReceivedKeyInputEvent::FunctionHandler InputDeviceMonitor::sReceivedKeyInputEventHandler(&InputDeviceMonitor::OnAsyncReceivedKeyInput);

/// <summary>
///  <para>Stores a collection of all "InputDeviceMonitor" instance pointers that currently exist.</para>
///  <para>Used by the worker thread to send input data and other events to each running InputDeviceMonitor.</para>
/// </summary>
static std::unordered_set<InputDeviceMonitor*> sDeviceMonitorSet;

/// <summary>
///  <para>Static collection of DirectInput and XInput handlers used to handle all devices discovered on the system.</para>
///  <para>They are responsible for managing the device connection and fetching axis/key input data.</para>
///  <para>
///   Every InputDeviceMonitor instance is expected to synchronize their private input device context collection
///   with this handler collection.
///  </para>
/// </summary>
static std::vector<MInputDeviceHandler*> sDeviceHandlerCollection;

/// <summary>
///  <para>
///   Map used to quickly fetch a zero based index to a handler in the "sDeviceHandlerCollection" matching
///   the given input device interface pointer associated with it.
///  </para>
///  <para>Used by the worker-thread when an event has been raised by a device handler's InputDeviceInterface.</para>
/// </summary>
static std::unordered_map<InputDeviceInterface*, int> sDeviceInterfaceToHandlerIndexMap;

/// <summary>
///  <para>
///   Set true if a thread is currently executing in function OnAsyncExecute(), meaning that we're actively
///   monitoring input devices for all InputDeviceMonitor instances.
///  </para>
///  <para>Set false if not, which should only happen when all InputDeviceMonitor instances are not currently running.</para>
/// </summary>
static bool sIsThreadAvailable = false;

/// <summary>
///  <para>Set true if an input device has been added to the system.</para>
///  <para>This notifies the worker thread to re-fetch all input devices and their information from the system.</para>
///  <para>Note: For best peformance, we should only fetch device info from the system when needed.</para>
/// </summary>
static bool sWasDeviceChangeDetected = false;

/// <summary>
///  <para>
///   Schedules a time in system ticks when the worker thread should re-fetch information from all device
///   connected to the system.
///  </para>
///  <para>Only applicable if static variable "sWasDeviceChangeDetected" is set true.</para>
/// </summary>
static Ticks sDeviceUpdateTimeInTicks;

#pragma endregion


#pragma region Constructors/Destructors
InputDeviceMonitor::InputDeviceMonitor()
:	fIsRunning(false),
	fMessageOnlyWindowPointer(Interop::UI::MessageOnlyWindow::GetSharedInstanceForCurrentThread()),
	fReceivedMessageEventHandler(this, &InputDeviceMonitor::OnReceivedMessage),
	fReservedMessageId(0),
	fVariantEventDataCollection(256)
{
	// Add event handlers.
	if (fMessageOnlyWindowPointer)
	{
		fMessageOnlyWindowPointer->GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
		fReservedMessageId = fMessageOnlyWindowPointer->ReserveMessageId();
	}

	// Add this input monitor to the static collection. This must be done last.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
	sDeviceMonitorSet.insert(this);
}

InputDeviceMonitor::~InputDeviceMonitor()
{
	// Stop input monitoring.
	Stop();

	// Remove this input monitor from the static collection.
	bool isLastDeviceMonitor = false;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		sDeviceMonitorSet.erase(this);
		isLastDeviceMonitor = sDeviceMonitorSet.empty();
	}

	// Remove event handlers.
	if (fMessageOnlyWindowPointer)
	{
		fMessageOnlyWindowPointer->GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);
		fMessageOnlyWindowPointer->UnreserveMessageId(fReservedMessageId);
		fReservedMessageId = 0;
	}

	// Delete device contexts belonging to this input monitor.
	for (auto&& deviceContextPointer : fDeviceContextCollection)
	{
		delete deviceContextPointer;
	}
	fDeviceContextCollection.clear();

	// Do final cleanup if this is the last input monitor instance.
	if (isLastDeviceMonitor)
	{
		// Wait for the worker thread to terminate gracefully.
		// This prevents the worker thread from being forcibly aborted during an app exit, causing memory leaks.
		const Ticks kTimeoutInTicks = Ticks::FromCurrentTime().AddSeconds(2);
		do
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			if (false == sIsThreadAvailable)
			{
				// Worker thread has terminated gracefully.
				break;
			}
			if (false == sDeviceMonitorSet.empty())
			{
				// This is no longer the last input monitor. This means that the worker thread is still needed.
				break;
			}
		} while (Ticks::FromCurrentTime() < kTimeoutInTicks);
	}
}

#pragma endregion


#pragma region Public Methods
InputDeviceMonitor::DiscoveredDeviceEvent::HandlerManager& InputDeviceMonitor::GetDiscoveredDeviceEventHandlers()
{
	return fDiscoveredDeviceEvent.GetHandlerManager();
}

InputDeviceMonitor::DeviceCollectionAdapter InputDeviceMonitor::GetDeviceCollection()
{
	return InputDeviceMonitor::DeviceCollectionAdapter(*this);
}

bool InputDeviceMonitor::IsRunning() const
{
	return fIsRunning;
}

bool InputDeviceMonitor::IsNotRunning() const
{
	return !fIsRunning;
}

void InputDeviceMonitor::Start()
{
	// Do not continue if already running.
	if (IsRunning())
	{
		return;
	}

	// Do not continue if we don't have a message-only window for the worker thread to post notifications to.
	// Note: This should never happen, but we should check just in case.
	if (!fMessageOnlyWindowPointer)
	{
		return;
	}

	// Synchronize this input monitor's device collection with the main static collection managed by the worker thread.
	// This is needed when a monitor has been stopped (not listening to devices) and then restarted later.
	if (sDeviceHandlerCollection.size() > 0)
	{
		// Fetch a copy of all device context configurations.
		const int kTotalDeviceCount = (int)sDeviceHandlerCollection.size();
		auto deviceContextSettingsArray = new InputDeviceContextSettings[kTotalDeviceCount];
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			for (int index = 0; index < kTotalDeviceCount; index++)
			{
				InputDeviceContext& deviceContext = sDeviceHandlerCollection.at(index)->GetContext();
				InputDeviceInterface& deviceInterface = deviceContext.GetDeviceInterface();
				auto deviceContextSettingsPointer = &deviceContextSettingsArray[index];
				deviceContextSettingsPointer->DriverTypeIntegerId = deviceInterface.GetDriverType().GetIntegerId();
				deviceContextSettingsPointer->ConnectionState = deviceInterface.GetConnectionState();
				deviceContextSettingsPointer->DeviceInfoPointer = deviceInterface.GetDeviceInfo();
				deviceContextSettingsPointer->VibrationRequestHandlerPointer = deviceContext.GetVibrationRequestHandler();
			}
		}

		// Update this input monitor's device collection with the copied device information.
		// Note: Never update a context or raise an event while a mutex is locked. That will cause the
		//       worker thread to be blocked, which is why we fetch a copy of device info up above.
		const int kNewDeviceCount = kTotalDeviceCount - (int)fDeviceContextCollection.size();
		for (int index = 0; index < kTotalDeviceCount; index++)
		{
			auto deviceContextSettingsPointer = &deviceContextSettingsArray[index];
			InputDeviceContext* deviceContextPointer = nullptr;
			if (index < (int)fDeviceContextCollection.size())
			{
				deviceContextPointer = fDeviceContextCollection.at(index);
			}
			else
			{
				int driverTypeIntegerId = deviceContextSettingsPointer->DriverTypeIntegerId;
				auto driverTypePointer = InputDeviceDriverType::FromIntegerId(driverTypeIntegerId);
				if (!driverTypePointer)
				{
					driverTypePointer = &InputDeviceDriverType::kDirectInput;
				}
				deviceContextPointer = new InputDeviceContext(
						*driverTypePointer, deviceContextSettingsPointer->VibrationRequestHandlerPointer);
				fDeviceContextCollection.push_back(deviceContextPointer);
			}
			InputDeviceContext::ScopedUpdater scopedUpdater(*deviceContextPointer);
			deviceContextPointer->UpdateWith(deviceContextSettingsPointer->ConnectionState);
			deviceContextPointer->UpdateWith(deviceContextSettingsPointer->DeviceInfoPointer);
		}

		// Delete the temporary array used to store copied device info.
		delete[] deviceContextSettingsArray;

		// Raise a "DiscoveredDevice" event for any newly added devices.
		for (int index = kTotalDeviceCount - kNewDeviceCount; index < kTotalDeviceCount; index++)
		{
			InputDeviceInterfaceEventArgs eventArgs(fDeviceContextCollection.at(index)->GetDeviceInterface());
			fDiscoveredDeviceEvent.Raise(*this, eventArgs);
		}
	}

	// Flag that this monitor is now running.
	fIsRunning = true;

	// Start the input monitoring thread, if not done already.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		if (false == sIsThreadAvailable)
		{
			sIsThreadAvailable = true;
			std::thread newThread(&InputDeviceMonitor::OnAsyncExecute);
			newThread.detach();
		}
	}
}

void InputDeviceMonitor::Stop()
{
	// Do not continue if already stopped.
	if (IsNotRunning())
	{
		return;
	}

	// Flag that this input monitor is no longer running.
	fIsRunning = false;

	// Clear this input monitor's event queue.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
	fVariantEventDataCollection.clear();
}

#pragma endregion


#pragma region Private Methods
void InputDeviceMonitor::OnReceivedMessage(
	Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments)
{
	// Do not continue if this message is not aimed toward this input monitor.
	if (arguments.GetMessageId() != fReservedMessageId)
	{
		return;
	}

	// Do not continue if the received message was already handled.
	if (arguments.WasHandled())
	{
		return;
	}

	// Handle all events queued/posted to the device monitor by the worker thread.
	// Note: We do this in batches for best performance.
	const size_t kVariantEventDataArraySize = 32;
	VariantEventData variantEventDataArray[kVariantEventDataArraySize];
	while (fVariantEventDataCollection.size() > 0)
	{
		// Pop off a batch of events from "fVariantEventDataCollection" and copy them to our temporary array.
		int variantEventDataItemsCopied = 0;
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
			const int kVariantEventDataCount = (int)fVariantEventDataCollection.size();
			int index;
			for (index = 0; (index < kVariantEventDataCount) && (index < kVariantEventDataArraySize); index++)
			{
				variantEventDataArray[index] = fVariantEventDataCollection.at(index);
			}
			variantEventDataItemsCopied = index;
			if (variantEventDataItemsCopied > 0)
			{
				auto iterator = fVariantEventDataCollection.begin();
				if (variantEventDataItemsCopied >= 2)
				{
					fVariantEventDataCollection.erase(iterator, iterator + variantEventDataItemsCopied);
				}
				else
				{
					fVariantEventDataCollection.erase(iterator);
				}
			}
		}

		// Update this input monitor's device contexts with the event data fetched up above.
		for (int index = 0; index < variantEventDataItemsCopied; index++)
		{
			// Fetch the next event data item.
			VariantEventData& variantEventData = variantEventDataArray[index];
			int deviceContextIndex = variantEventData.DeviceIndex;

			// If the event references a device that is not in this input monitor's collection, then add *all*
			// new devices from the worker's global collection to this monitor's collection in the same order.
			if (deviceContextIndex >= (int)fDeviceContextCollection.size())
			{
				// Fetch a copy of all the new device configurations from the global collection.
				const int kTotalDeviceCount = (int)sDeviceHandlerCollection.size();
				const int kNewDeviceCount = kTotalDeviceCount - (int)fDeviceContextCollection.size();
				if (kNewDeviceCount <= 0)
				{
					continue;
				}
				auto deviceContextSettingsArray = new InputDeviceContextSettings[kNewDeviceCount];
				{
					std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
					for (int arrayIndex = 0; arrayIndex < kNewDeviceCount; arrayIndex++)
					{
						int sourceIndex = arrayIndex + (int)fDeviceContextCollection.size();
						InputDeviceContext& deviceContext = sDeviceHandlerCollection.at(sourceIndex)->GetContext();
						InputDeviceInterface& deviceInterface = deviceContext.GetDeviceInterface();
						auto deviceContextSettingsPointer = &deviceContextSettingsArray[arrayIndex];
						deviceContextSettingsPointer->DriverTypeIntegerId = deviceInterface.GetDriverType().GetIntegerId();
						deviceContextSettingsPointer->ConnectionState = deviceInterface.GetConnectionState();
						deviceContextSettingsPointer->DeviceInfoPointer = deviceInterface.GetDeviceInfo();
						deviceContextSettingsPointer->VibrationRequestHandlerPointer =
								deviceContext.GetVibrationRequestHandler();
					}
				}

				// Add the new device configuration's to this input monitor's collection.
				// Note: Never update a context or raise an event while a mutex is locked. That will cause the
				//       worker thread to be blocked, which is why we fetch a copy of device info up above.
				for (int arrayIndex = 0; arrayIndex < kNewDeviceCount; arrayIndex++)
				{
					auto deviceContextSettingsPointer = &deviceContextSettingsArray[arrayIndex];
					int driverTypeIntegerId = deviceContextSettingsPointer->DriverTypeIntegerId;
					auto driverTypePointer = InputDeviceDriverType::FromIntegerId(driverTypeIntegerId);
					if (!driverTypePointer)
					{
						driverTypePointer = &InputDeviceDriverType::kDirectInput;
					}
					auto deviceContextPointer = new InputDeviceContext(
							*driverTypePointer, deviceContextSettingsPointer->VibrationRequestHandlerPointer);
					fDeviceContextCollection.push_back(deviceContextPointer);
					InputDeviceContext::ScopedUpdater scopedUpdater(*deviceContextPointer);
					deviceContextPointer->UpdateWith(deviceContextSettingsPointer->ConnectionState);
					deviceContextPointer->UpdateWith(deviceContextSettingsPointer->DeviceInfoPointer);
				}

				// Delete the temporary array used to store copied device info.
				delete[] deviceContextSettingsArray;

				// Raise a "DiscoveredDevice" event for each newly added device.
				for (int index = kTotalDeviceCount - kNewDeviceCount; index < kTotalDeviceCount; index++)
				{
					InputDeviceInterfaceEventArgs eventArgs(fDeviceContextCollection.at(index)->GetDeviceInterface());
					fDiscoveredDeviceEvent.Raise(*this, eventArgs);
				}
			}

			// Fetch the event's indexed device context, if valid.
			if ((deviceContextIndex < 0) || (deviceContextIndex >= (int)fDeviceContextCollection.size()))
			{
				Rtt_ASSERT(0);
				continue;
			}
			auto deviceContextPointer = fDeviceContextCollection.at(deviceContextIndex);
			if (!deviceContextPointer)
			{
				continue;
			}

			// Handle the input device's event.
			switch (variantEventData.Type)
			{
				case VariantEventData::Type::kDeviceStatusChanged:
				{
					// Update the device context's configuration and connection state.
					InputDeviceContextSettings deviceContextSettings;
					{
						std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
						if (deviceContextIndex >= (int)sDeviceHandlerCollection.size())
						{
							continue;
						}
						InputDeviceInterface& sourceDeviceInterface =
								sDeviceHandlerCollection.at(deviceContextIndex)->GetContext().GetDeviceInterface();
						deviceContextSettings.ConnectionState = sourceDeviceInterface.GetConnectionState();
						deviceContextSettings.DeviceInfoPointer = sourceDeviceInterface.GetDeviceInfo();
					}
					InputDeviceContext::ScopedUpdater scopedUpdater(*deviceContextPointer);
					deviceContextPointer->UpdateWith(deviceContextSettings.ConnectionState);
					deviceContextPointer->UpdateWith(deviceContextSettings.DeviceInfoPointer);
					break;
				}
				case VariantEventData::Type::kReceivedAxisInput:
				{
					// Update the device context with the received axis data.
					deviceContextPointer->UpdateWith(variantEventData.Data.Axis);
					break;
				}
				case VariantEventData::Type::kReceivedKeyInput:
				{
					// Update the device context with the received key data.
					deviceContextPointer->UpdateWith(variantEventData.Data.Key);
					break;
				}
			}
		}
	}

	// Flag the message as handled.
	arguments.SetHandled();
	arguments.SetReturnResult(0);
}

#pragma endregion


#pragma region Private Static Functions
void InputDeviceMonitor::OnAsyncExecute()
{
	// Initialize COM for this thread.
	// We must do this before using our DirectInput COM interface below.
	ScopedComInitializer scopedComInitializer;

	// Fetch a handle to the module this source code is running in.
	HMODULE moduleHandle = nullptr;
	{
		DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
		::GetModuleHandleExW(flags, (LPCWSTR)&OnAsyncExecute, &moduleHandle);
	}

	// Create an interface to DirectInput.
	IDirectInput8W* directInputPointer = nullptr;
	::CoCreateInstance(
			CLSID_DirectInput8, nullptr, CLSCTX_INPROC_SERVER, IID_IDirectInput8W, (LPVOID*)&directInputPointer);
	if (directInputPointer)
	{
		try
		{
			directInputPointer->Initialize(moduleHandle, DIRECTINPUT_VERSION);
		}
		catch (...)
		{
			try { directInputPointer->Release(); }
			catch (...) {}
			directInputPointer = nullptr;
		}
	}

	// Enable XInput data reporting for all devices.
	auto xinputPointer = XInputInterface::GetInstance();
	if (xinputPointer && xinputPointer->CanEnable())
	{
		xinputPointer->Enable(true);
	}

	// Set up an invisible window on this thread for receiving broadcasted notifications from the OS
	// when an input device has been added/removed from the system.
	// Note: A Win32 "message-only window" does not officially support broadcasted messages. (Definitely not on WinXP.)
	HWND invisibleWindowHandle = nullptr;
	{
		static const wchar_t *sWindowClassName = nullptr;
		if (!sWindowClassName)
		{
			WNDCLASSEXW settings{};
			settings.cbSize = sizeof(settings);
			settings.lpszClassName = L"CoronaLabs.Corona.InputDeviceMonitor.AsyncWindow";
			settings.hInstance = moduleHandle;
			settings.lpfnWndProc = InputDeviceMonitor::OnAsyncReceivedMessage;
			auto atom = ::RegisterClassExW(&settings);
			if (atom)
			{
				sWindowClassName = settings.lpszClassName;
			}
		}
		invisibleWindowHandle = ::CreateWindowExW(
				0, sWindowClassName, L"", WS_ICONIC, 0, 0, CW_USEDEFAULT, 0, nullptr, nullptr, moduleHandle, nullptr);
	}
	HDEVNOTIFY deviceNotificationHandle = nullptr;
	{
		DEV_BROADCAST_DEVICEINTERFACE_W deviceFilterData{};
		deviceFilterData.dbcc_size = sizeof(deviceFilterData);
		deviceFilterData.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		deviceNotificationHandle = ::RegisterDeviceNotificationW(
				(HANDLE)invisibleWindowHandle, &deviceFilterData,
				DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
	}

	// Fetch all devices currently connected to the system.
	AsyncUpdateAllDevicesUsing(directInputPointer, invisibleWindowHandle);
	sWasDeviceChangeDetected = false;

	// Start our infinite loop which monitors all devices, polls for input, and handles vibration/rumble.
	while (true)
	{
		// Exit out of this thread if all InputDeviceMonitor objects have been destroyed.
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			if (sDeviceMonitorSet.empty())
			{
				break;
			}
		}

		// Pump all Windows messages sent to this thread's invisible window.
		// This is mostly needed to receive broadcasted messages indicating that a device has been connected to the system.
		MSG message{};
		while (::PeekMessage(&message, invisibleWindowHandle, 0, 0, PM_NOREMOVE))
		{
			if (::GetMessage(&message, invisibleWindowHandle, 0, 0))
			{
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		}

		// If a device has been recently connected to the system, then fetch all device info from the system
		// and update this worker thread's current device handler collection.
		// Note: This is an expensive operation. We schedule this to be done via "sDeviceUpdateTimeInTicks" since Windows
		//       tends to send 3 notifications for every 1 device connected and we only want to do this update once.
		if (sWasDeviceChangeDetected && (Ticks::FromCurrentTime() >= sDeviceUpdateTimeInTicks))
		{
			sWasDeviceChangeDetected = false;
			AsyncUpdateAllDevicesUsing(directInputPointer, invisibleWindowHandle);
		}

		// Poll all input device handlers.
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			for (int index = 0; index < (int)sDeviceHandlerCollection.size(); index++)
			{
				// Fetch the next input device handler.
				auto deviceHandlerPointer = sDeviceHandlerCollection.at(index);
				if (!deviceHandlerPointer)
				{
					continue;
				}

				// Ignore the handler if it's no longer attached/connected to the device.
				if (deviceHandlerPointer->IsAttached() == false)
				{
					continue;
				}

				// Poll the input device. This does the following:
				// - Fetches input data.
				// - Handles vibration/rumble feedback.
				// - Updates the device connection status.
				deviceHandlerPointer->Poll();

				// If we've lost our device connection during the above poll, then detach from the device.
				// Note: This improves performance because polling a disconnected device will block the thread
				//       for several milliseconds. We'll reconnect when a WM_DEVICECHANGE message has been received.
				if (deviceHandlerPointer->GetContext().GetDeviceInterface().IsConnected() == false)
				{
					deviceHandlerPointer->Detach();
				}
			}
		}

		// Sleep for 10 milliseconds.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Destroy all device handlers.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (int index = (int)sDeviceHandlerCollection.size() - 1; index >= 0; index--)
		{
			auto deviceHandlerPointer = sDeviceHandlerCollection.at(index);
			if (deviceHandlerPointer)
			{
				delete deviceHandlerPointer;
			}
		}
		sDeviceHandlerCollection.clear();
		sDeviceInterfaceToHandlerIndexMap.clear();
	}

	// Unregister this thread's invisible window from receiving system notifications for input devices.
	::UnregisterDeviceNotification(deviceNotificationHandle);

	// Destroy this thread's invisible window.
	// Note: This must be done after unregistering for device notifications and deleting DirectInput device handlers.
	if (invisibleWindowHandle)
	{
		::DestroyWindow(invisibleWindowHandle);
		invisibleWindowHandle = nullptr;
	}

	// Release our reference to DirectInput.
	// Note: This must be done after deleting our DirectInput device handlers.
	if (directInputPointer)
	{
		directInputPointer->Release();
		directInputPointer = nullptr;
	}

	// This thread is about to terminate. Flag that it is no longer available.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		sIsThreadAvailable = false;
	}
}

LRESULT InputDeviceMonitor::OnAsyncReceivedMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam)
{
	// Handle the received message.
	switch (messageId)
	{
		case WM_DEVICECHANGE:
		{
			switch (wParam)
			{
				case DBT_DEVICEARRIVAL:
				{
					auto deviceHeaderPointer = (DEV_BROADCAST_HDR*)lParam;
					if (deviceHeaderPointer && (DBT_DEVTYP_DEVICEINTERFACE == deviceHeaderPointer->dbch_devicetype))
					{
						// A device has just been connected to the system.
						// Schedule the worker thread to fetch all device info and update its device handlers.
						// Note: We typically get at least 3 of these Windows messages for every device connected.
						//       Since fetching device info can be a huge performance hit, wait for a short amount
						//       time (after all 3 messages are received) so that we'll only take this hit once.
						sDeviceUpdateTimeInTicks = Ticks::FromCurrentTime().AddMilliseconds(30);
						sWasDeviceChangeDetected = true;
					}
					break;
				}
			}
			break;
		}
	}

	// Let Windows do its default handling on the received message.
	return ::DefWindowProc(windowHandle, messageId, wParam, lParam);
}

void InputDeviceMonitor::AsyncUpdateAllDevicesUsing(LPDIRECTINPUT8W directInputPointer, HWND windowHandle)
{
	// Fetch an interface to the system's XInput library.
	// Will return null if XInput is not available on the system. (Can only happen if user deletes DLL by hand.)
	auto xinputPointer = XInputInterface::GetInstance();

	// If we're using DirectInput and XInput at the same time, then identify all XInput devices connected to the system.
	// We don't want to connect to XInput devices via DirectInput because it'll have reduced functionality.
	// We'll identify these devices via their USB Vendor ID and Product ID (ie: VID and PID).
	DirectInputEnumDevicesContext directInputEnumDevicesContext;
	if (directInputPointer && xinputPointer)
	{
		RAWINPUTDEVICELIST* rawDeviceListPointer = nullptr;
		UINT rawDeviceCount = 0;
		::GetRawInputDeviceList(nullptr, &rawDeviceCount, sizeof(RAWINPUTDEVICELIST));
		if (rawDeviceCount > 0)
		{
			rawDeviceListPointer = new RAWINPUTDEVICELIST[rawDeviceCount];
			UINT rawDevicesFetched =
					::GetRawInputDeviceList(rawDeviceListPointer, &rawDeviceCount, sizeof(RAWINPUTDEVICELIST));
			for (UINT rawDeviceIndex = 0; rawDeviceIndex < rawDevicesFetched; rawDeviceIndex++)
			{
				UINT rawDeviceNameLength = 0;
				HANDLE rawDeviceHandle = rawDeviceListPointer[rawDeviceIndex].hDevice;
				::GetRawInputDeviceInfoW(rawDeviceHandle, RIDI_DEVICENAME, nullptr, &rawDeviceNameLength);
				if (rawDeviceNameLength > 0)
				{
					rawDeviceNameLength++;
					auto rawDeviceName = new wchar_t[rawDeviceNameLength];
					rawDeviceName[0] = L'\0';
					::GetRawInputDeviceInfoW(rawDeviceHandle, RIDI_DEVICENAME, rawDeviceName, &rawDeviceNameLength);
					if (wcsstr(rawDeviceName, L"IG_"))
					{
						// This is an XInput device.
						DWORD vendorId = 0;
						DWORD productId = 0;
						wchar_t* substringPointer = wcsstr(rawDeviceName, L"VID_");
						if (substringPointer && (wcslen(substringPointer) >= 8))
						{
							swscanf_s(substringPointer, L"VID_%4X", &vendorId);
						}
						substringPointer = wcsstr(rawDeviceName, L"PID_");
						if (substringPointer && (wcslen(substringPointer) >= 8))
						{
							swscanf_s(substringPointer, L"PID_%4X", &productId);
						}
						DWORD combinedId = MAKELONG(vendorId, productId);
						directInputEnumDevicesContext.VidPidExclusionSet.insert(combinedId);
					}
					delete[] rawDeviceName;
				}
			}
		}
		if (rawDeviceListPointer)
		{
			delete[] rawDeviceListPointer;
			rawDeviceListPointer = nullptr;
		}
	}

	// Update our handlers for all DirectInput devices currently connected/disconnected on the system.
	if (directInputPointer)
	{
		// Fetch the GUIDs of all DirectInput devices that are currently connected to the system.
		directInputPointer->EnumDevices(
				DI8DEVCLASS_ALL, &OnAsyncEnumDirectInputDevice, &directInputEnumDevicesContext, DIEDFL_ATTACHEDONLY);

		// Add/update handlers for each device that is currently connected to the system.
		auto& directInputDeviceGuids = directInputEnumDevicesContext.DeviceInstanceGuids;
		for (auto iterator = directInputDeviceGuids.begin(); iterator != directInputDeviceGuids.end(); iterator++)
		{
			// Fetch the next DirectInput device GUID.
			GUID deviceInstanceGuid = *iterator;
			if (::IsEqualGUID(deviceInstanceGuid, GUID_NULL))
			{
				continue;
			}

			// If a handler has been created for the device before, then fetch it.
			DirectInputDeviceHandler* deviceHandlerPointer = nullptr;
			{
				std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
				for (int index = (int)sDeviceHandlerCollection.size() - 1; index >= 0; index--)
				{
					auto nextDeviceHandlerPointer = sDeviceHandlerCollection.at(index);
					if (nextDeviceHandlerPointer)
					{
						auto driverType = nextDeviceHandlerPointer->GetContext().GetDeviceInterface().GetDriverType();
						if (driverType.Equals(InputDeviceDriverType::kDirectInput))
						{
							auto nextDirectInputDeviceHandlerPointer = (DirectInputDeviceHandler*)nextDeviceHandlerPointer;
							GUID nextDeviceInstanceGuid = nextDirectInputDeviceHandlerPointer->GetLastAttachedInstanceGuid();
							if (::IsEqualGUID(deviceInstanceGuid, nextDeviceInstanceGuid))
							{
								deviceHandlerPointer = nextDirectInputDeviceHandlerPointer;
								break;
							}
						}
					}
				}
			}

			// Skip this device if we're already handling it.
			if (deviceHandlerPointer && deviceHandlerPointer->IsAttached())
			{
				continue;
			}

			// Create a new DirectInput COM interface to the device.
			LPDIRECTINPUTDEVICE8W directInputDevicePointer = nullptr;
			directInputPointer->CreateDevice(deviceInstanceGuid, &directInputDevicePointer, nullptr);
			if (!directInputDevicePointer)
			{
				continue;
			}

			// Set up a handler to collect data from the device, but only if it has at least 1 input channel.
			try
			{
				DIDEVCAPS directInputDeviceCapabilities{};
				directInputDeviceCapabilities.dwSize = sizeof(directInputDeviceCapabilities);
				directInputDevicePointer->GetCapabilities(&directInputDeviceCapabilities);
				if ((directInputDeviceCapabilities.dwAxes > 0) ||
				    (directInputDeviceCapabilities.dwButtons > 0) ||
				    (directInputDeviceCapabilities.dwPOVs > 0))
				{
					// If this is a newly discovered device, then create a new handler for it.
					if (!deviceHandlerPointer)
					{
						std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
						deviceHandlerPointer = new DirectInputDeviceHandler();
						InputDeviceInterface& deviceInterface = deviceHandlerPointer->GetContext().GetDeviceInterface();
						deviceInterface.GetStatusChangedEventHandlers().Add(&sDeviceStatusChangedEventHandler);
						deviceInterface.GetReceivedAxisInputEventHandlers().Add(&sReceivedAxisInputEventHandler);
						deviceInterface.GetReceivedKeyInputEventHandlers().Add(&sReceivedKeyInputEventHandler);
						sDeviceHandlerCollection.push_back(deviceHandlerPointer);
						sDeviceInterfaceToHandlerIndexMap[&deviceInterface] = (int)sDeviceHandlerCollection.size() - 1;
					}

					// Attach a handler to the DirectInput device's COM interface.
					// This sets it up to collect input data from the device.
					deviceHandlerPointer->SetWindowHandle(windowHandle);
					deviceHandlerPointer->AttachTo(directInputDevicePointer);
				}
			}
			catch (...) {}

			// Release our COM interface to the DirectInput device.
			// Note: This is reference counted. If we've attached a handler to it above, then it will keep it alive.
			directInputDevicePointer->Release();
		}

		// Detach handlers from devices that were not found by the DirectInput EnumDevices() method.
		// These are assumed to be disconnected from the system.
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			for (auto deviceHandlerPointer : sDeviceHandlerCollection)
			{
				if (!deviceHandlerPointer)
				{
					continue;
				}
				auto driverType = deviceHandlerPointer->GetContext().GetDeviceInterface().GetDriverType();
				if (driverType.NotEquals(InputDeviceDriverType::kDirectInput))
				{
					continue;
				}
				auto directInputDeviceHandlerPointer = (DirectInputDeviceHandler*)deviceHandlerPointer;
				auto deviceGuid = directInputDeviceHandlerPointer->GetInstanceGuid();
				bool isConnected = false;
				for (auto&& nextGuid : directInputDeviceGuids)
				{
					if (::IsEqualGUID(deviceGuid, nextGuid))
					{
						isConnected = true;
						break;
					}
				}
				if (!isConnected && deviceHandlerPointer->IsAttached())
				{
					deviceHandlerPointer->GetContext().UpdateWith(Rtt::InputDeviceConnectionState::kDisconnected);
					directInputDeviceHandlerPointer->Detach();
				}
			}
		}
	}

	// Update our handlers for all XInput devices currently connected/disconnected on the system.
	if (xinputPointer)
	{
		// Temporary struct used to store an XInput device's connection status and its capabilities.
		struct XInputDeviceSettings
		{
			bool IsConnected;
			XINPUT_CAPABILITIES Capabilities;
		};

		// Fetch information about all XInput devices currently connected to the system.
		XInputDeviceSettings xinputDeviceSettingsArray[XUSER_MAX_COUNT];
		for (DWORD xinputUserIndex = 0; xinputUserIndex < XUSER_MAX_COUNT; xinputUserIndex++)
		{
			auto deviceCapabilitiesPointer = &(xinputDeviceSettingsArray[xinputUserIndex].Capabilities);
			memset(deviceCapabilitiesPointer, 0, sizeof(XINPUT_CAPABILITIES));
			auto xinputResult = xinputPointer->GetCapabilities(xinputUserIndex, 0, deviceCapabilitiesPointer);
			xinputDeviceSettingsArray[xinputUserIndex].IsConnected = (ERROR_SUCCESS == xinputResult);
		}

		// Detach/disconnect handlers that were not found by XInput up above.
		// Note: We must do this first in case a device has been shifted down to a lower user index (aka: player number).
		//       This can happen when connecting/disconnecting a mix of Xbox One and 360 controllers.
		for (DWORD xinputUserIndex = 0; xinputUserIndex < XUSER_MAX_COUNT; xinputUserIndex++)
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			auto deviceCapabilitiesPointer = &(xinputDeviceSettingsArray[xinputUserIndex].Capabilities);
			for (auto nextDeviceHandlerPointer : sDeviceHandlerCollection)
			{
				// Do not continue if the next device handler is not attached to the given XInput user index.
				if (!nextDeviceHandlerPointer || !nextDeviceHandlerPointer->IsAttached())
				{
					continue;
				}
				auto driverType = nextDeviceHandlerPointer->GetContext().GetDeviceInterface().GetDriverType();
				if (driverType.NotEquals(InputDeviceDriverType::kXInput))
				{
					continue;
				}
				auto nextXInputDeviceHandlerPointer = (XInputDeviceHandler*)nextDeviceHandlerPointer;
				if (nextXInputDeviceHandlerPointer->GetLastAttachedUserIndex().GetValue() != xinputUserIndex)
				{
					continue;
				}

				// Do not continue if the XInput device capabilties exactly matches what our handler is referencing.
				// ie: A different XInput device has *not* been swapped into this user index.
				if (xinputDeviceSettingsArray[xinputUserIndex].IsConnected)
				{
					if (nextXInputDeviceHandlerPointer->IsLastAttachedEqualTo(*deviceCapabilitiesPointer))
					{
						continue;
					}
				}

				// Detach this handler and flag its device as disconnected. We do this when:
				// - The device has been disconnected at this user index.
				// - A different device has been swapped into this user index. (Needs a new handler for new device config.)
				InputDeviceContext::ScopedUpdater scopedUpdater(nextDeviceHandlerPointer->GetContext());
				nextDeviceHandlerPointer->Detach();
				nextDeviceHandlerPointer->GetContext().UpdateWith(Rtt::InputDeviceConnectionState::kDisconnected);
			}
		}

		// Update handlers for all currently connected XInput devices.
		for (DWORD xinputUserIndex = 0; xinputUserIndex < XUSER_MAX_COUNT; xinputUserIndex++)
		{
			// Do not continue if the device is disconnected at this user index.
			if (false == xinputDeviceSettingsArray[xinputUserIndex].IsConnected)
			{
				continue;
			}

			// Do not continue if we already have a handler attached to the user index's associated XInput device.
			auto deviceCapabilitiesPointer = &(xinputDeviceSettingsArray[xinputUserIndex].Capabilities);
			{
				std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
				bool isDeviceHandled = false;
				for (auto nextDeviceHandlerPointer : sDeviceHandlerCollection)
				{
					if (!nextDeviceHandlerPointer || !nextDeviceHandlerPointer->IsAttached())
					{
						continue;
					}
					auto driverType = nextDeviceHandlerPointer->GetContext().GetDeviceInterface().GetDriverType();
					if (driverType.NotEquals(InputDeviceDriverType::kXInput))
					{
						continue;
					}
					auto nextXInputDeviceHandlerPointer = (XInputDeviceHandler*)nextDeviceHandlerPointer;
					if (!nextXInputDeviceHandlerPointer->IsLastAttachedEqualTo(*deviceCapabilitiesPointer))
					{
						continue;
					}
					auto userIndexResult = nextXInputDeviceHandlerPointer->GetLastAttachedUserIndex();
					if (userIndexResult.HasFailed() || (userIndexResult.GetValue() != xinputUserIndex))
					{
						continue;
					}
					isDeviceHandled = true;
					break;
				}
				if (isDeviceHandled)
				{
					continue;
				}
			}

			// Fetch all detached handlers matching the user index's device capabilities.
			std::vector<XInputDeviceHandler*> xinputDeviceHandlerPointers;
			{
				std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
				for (auto nextDeviceHandlerPointer : sDeviceHandlerCollection)
				{
					if (nextDeviceHandlerPointer && (nextDeviceHandlerPointer->IsAttached() == false))
					{
						auto driverType = nextDeviceHandlerPointer->GetContext().GetDeviceInterface().GetDriverType();
						if (driverType.Equals(InputDeviceDriverType::kXInput))
						{
							auto nextXInputDeviceHandlerPointer = (XInputDeviceHandler*)nextDeviceHandlerPointer;
							if (nextXInputDeviceHandlerPointer->IsLastAttachedEqualTo(*deviceCapabilitiesPointer))
							{
								xinputDeviceHandlerPointers.push_back(nextXInputDeviceHandlerPointer);
							}
						}
					}
				}
			}

			// Attempt to find a detached/disconnected handler that:
			// - Was once assigned this user index.
			// - Has the same device capabilites.
			XInputDeviceHandler* xinputDeviceHandlerPointer = nullptr;
			for (auto nextXInputDeviceHandlerPointer : xinputDeviceHandlerPointers)
			{
				auto userIndexResult = nextXInputDeviceHandlerPointer->GetLastAttachedUserIndex();
				if (userIndexResult.HasSucceeded() && (userIndexResult.GetValue() == xinputUserIndex))
				{
					xinputDeviceHandlerPointer = nextXInputDeviceHandlerPointer;
					break;
				}
			}

			// If we've failed to find a handler up above, then use the first detached handler matching
			// the XInput device's capabilities/configuration.
			if (!xinputDeviceHandlerPointer && !xinputDeviceHandlerPointers.empty())
			{
				xinputDeviceHandlerPointer = xinputDeviceHandlerPointers.at(0);
			}

			// If no detached handler matches the XInput device's capabilities, then create a new handler.
			if (!xinputDeviceHandlerPointer)
			{
				std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
				xinputDeviceHandlerPointer = new XInputDeviceHandler();
				InputDeviceInterface& deviceInterface = xinputDeviceHandlerPointer->GetContext().GetDeviceInterface();
				deviceInterface.GetStatusChangedEventHandlers().Add(&sDeviceStatusChangedEventHandler);
				deviceInterface.GetReceivedAxisInputEventHandlers().Add(&sReceivedAxisInputEventHandler);
				deviceInterface.GetReceivedKeyInputEventHandlers().Add(&sReceivedKeyInputEventHandler);
				sDeviceHandlerCollection.push_back(xinputDeviceHandlerPointer);
				sDeviceInterfaceToHandlerIndexMap[&deviceInterface] = (int)sDeviceHandlerCollection.size() - 1;
			}

			// Attach a handler to the XInput device.
			xinputDeviceHandlerPointer->AttachTo(xinputUserIndex, *deviceCapabilitiesPointer);
		}
	}
}

void InputDeviceMonitor::OnAsyncDeviceStatusChanged(
	InputDeviceInterface& sender, const InputDeviceStatusChangedEventArgs& arguments)
{
	// Copy the received event's info to a VariantEventData object to be posted to all active input monitors.
	// Note: This object is a union that can store different event data types and be copied by value efficiently.
	InputDeviceMonitor::VariantEventData variantEventData;
	variantEventData.Type = InputDeviceMonitor::VariantEventData::Type::kDeviceStatusChanged;
	variantEventData.Data.DeviceStatus = arguments.ToSettings();
	AsyncPostEvent(sender, variantEventData);
}

void InputDeviceMonitor::OnAsyncReceivedAxisInput(InputDeviceInterface& sender, const InputAxisDataEventArgs& arguments)
{
	// Copy the received event's info to a VariantEventData object to be posted to all active input monitors.
	// Note: This object is a union that can store different event data types and be copied by value efficiently.
	InputDeviceMonitor::VariantEventData variantEventData;
	variantEventData.Type = InputDeviceMonitor::VariantEventData::Type::kReceivedAxisInput;
	variantEventData.Data.Axis = arguments.ToData();
	AsyncPostEvent(sender, variantEventData);
}

void InputDeviceMonitor::OnAsyncReceivedKeyInput(InputDeviceInterface& sender, const KeyEventArgs& arguments)
{
	// Copy the received event's info to a VariantEventData object to be posted to all active input monitors.
	// Note: This object is a union that can store different event data types and be copied by value efficiently.
	InputDeviceMonitor::VariantEventData variantEventData;
	variantEventData.Type = InputDeviceMonitor::VariantEventData::Type::kReceivedKeyInput;
	variantEventData.Data.Key = arguments.ToData();
	AsyncPostEvent(sender, variantEventData);
}

void InputDeviceMonitor::AsyncPostEvent(
	InputDeviceInterface& sender, InputDeviceMonitor::VariantEventData& variantEventData)
{
	// Fetch the given device's index within static member variable "sDeviceHandlerCollection".
	auto iterator = sDeviceInterfaceToHandlerIndexMap.find(&sender);
	if (iterator == sDeviceInterfaceToHandlerIndexMap.end())
	{
		return;
	}
	int deviceHandlerIndex = (*iterator).second;
	if (deviceHandlerIndex < 0)
	{
		return;
	}

	// Apple the device index to the given event data.
	variantEventData.DeviceIndex = deviceHandlerIndex;

	// Notify all InputDeviceMonitor objects about the received event.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
	for (auto&& deviceMonitorPointer : sDeviceMonitorSet)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(deviceMonitorPointer->fMutex);

		// Do not notify the device monitor if it's not running.
		if (false == deviceMonitorPointer->fIsRunning)
		{
			continue;
		}

		// Skip this device monitor if it doesn't have a messsage-only window.
		// We need this to send a notification message on the thread the device monitor runs on.
		if (nullptr == deviceMonitorPointer->fMessageOnlyWindowPointer)
		{
			continue;
		}

		// Add the event to the device monitor's queue.
		deviceMonitorPointer->fVariantEventDataCollection.push_back(variantEventData);

		// Notify the device monitor via its message-only window.
		HWND windowHandle = deviceMonitorPointer->fMessageOnlyWindowPointer->GetWindowHandle();
		::PostMessage(windowHandle, deviceMonitorPointer->fReservedMessageId, 0, 0);
	}
}

BOOL FAR PASCAL InputDeviceMonitor::OnAsyncEnumDirectInputDevice(
	LPCDIDEVICEINSTANCEW deviceInstancePointer, LPVOID contextPointer)
{
	// Validate.
	if (!deviceInstancePointer || !contextPointer)
	{
		return DIENUM_STOP;
	}
	auto enumDevicesContextPointer = (DirectInputEnumDevicesContext*)contextPointer;

	// Do not continue if this is a keyboard or mouse.
	// We do not want to handle these device types via DirectInput.
	switch (deviceInstancePointer->dwDevType & 0xFF)
	{
		case DI8DEVTYPE_KEYBOARD:
		case DI8DEVTYPE_MOUSE:
			return DIENUM_CONTINUE;
	}

	// Do not continue if this device's Vendor ID and Product ID is in our exclusion set.
	// This is used to exclude XInput devices from being handled by DirectInput if XInput is supported by the system.
	auto& vidPidExclusionSet = enumDevicesContextPointer->VidPidExclusionSet;
	if (vidPidExclusionSet.find(deviceInstancePointer->guidProduct.Data1) != vidPidExclusionSet.end())
	{
		return DIENUM_CONTINUE;
	}

	// Add the DirectInput device's GUID to the context's collection.
	enumDevicesContextPointer->DeviceInstanceGuids.push_back(deviceInstancePointer->guidInstance);
	return DIENUM_CONTINUE;
}

#pragma endregion


#pragma region DeviceCollectionAdapter Methods
InputDeviceMonitor::DeviceCollectionAdapter::DeviceCollectionAdapter(InputDeviceMonitor& deviceMonitor)
:	fDeviceMonitor(deviceMonitor)
{
}

InputDeviceMonitor::DeviceCollectionAdapter::~DeviceCollectionAdapter()
{
}

int InputDeviceMonitor::DeviceCollectionAdapter::GetCount() const
{
	return (int)fDeviceMonitor.fDeviceContextCollection.size();
}

InputDeviceInterface* InputDeviceMonitor::DeviceCollectionAdapter::GetByIndex(int index) const
{
	if ((index >= 0) && (index < (int)fDeviceMonitor.fDeviceContextCollection.size()))
	{
		auto deviceContextPointer = fDeviceMonitor.fDeviceContextCollection.at(index);
		if (deviceContextPointer)
		{
			return &(deviceContextPointer->GetDeviceInterface());
		}
	}
	return nullptr;
}

#pragma endregion

} }	// namespace Interop::Input
