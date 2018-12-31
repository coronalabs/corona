//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Interop\UI\UIComponent.h"
#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include "InputAxisDataEventArgs.h"
#include "InputDeviceContext.h"
#include "InputDeviceInterfaceEventArgs.h"
#include "InputDeviceStatusChangedEventArgs.h"
#include "KeyEventArgs.h"
#include <mutex>
#include <Windows.h>


#pragma region Forward Declarations
namespace Interop {
	namespace Input {
		class InputDeviceInterface;
	}
	namespace UI {
		class HandleMessageEventArgs;
		class MessageOnlyWindow;
	}
}
extern "C" {
	struct DIDEVICEINSTANCEW;
	struct IDirectInput8W;
	typedef const DIDEVICEINSTANCEW *LPCDIDEVICEINSTANCEW;
	typedef struct IDirectInput8W *LPDIRECTINPUT8W;
}

#pragma endregion


namespace Interop { namespace Input {

/// <summary>
///  <para>Monitors all input devices connected to the system.</para>
///  <para>
///   Detects when devices have been connected/disconnected, when they have been reconfigured,
///   and provides axis and key data received from them.
///  </para>
///  <para>You must call this object's Start() method to start monitor devices and to received data from them.</para>
///  <para>
///   Calling the Stop() method will disabling disable monitoring, preventing device status and axis/key events
///   from being raised.
///  </para>
/// </summary>
class InputDeviceMonitor final
{
	Rtt_CLASS_NO_COPIES(InputDeviceMonitor)

	public:
		#pragma region Public Event Types
		/// <summary>
		///  <para>
		///   Defines the "DiscoveredDevice" event which is raised when a new input device has been discovered
		///   on the system and is now being monitored.
		///  </para>
		///  <para>
		///   This event does not get raised when a past discovered device has been disconnected and then reconnected,
		///   because the input monitor is already actively monitoring it and waiting for it to reconnect.
		///  </para>
		/// </summary>
		typedef Event<InputDeviceMonitor&, InputDeviceInterfaceEventArgs&> DiscoveredDeviceEvent;

		#pragma endregion


		#pragma region Public DeviceCollectionAdapter Class
		/// <summary>
		///  <para>Provides read-only access to an InputDeviceMonitor object's device collection.</para>
		///  <para>Instances of this class was designed to be passed by value efficiently.</para>
		/// </summary>
		class DeviceCollectionAdapter
		{
			public:
				/// <summary>Creates a new object providing access to the given input monitor's device collection.</summary>
				/// <param name="deviceMonitor">
				///  Reference to the device monitor whose device collection you want to access.
				/// </param>
				DeviceCollectionAdapter(InputDeviceMonitor& deviceMonitor);

				/// <summary>Destroys this object.</summary>
				virtual ~DeviceCollectionAdapter();

				/// <summary>
				///  Gets the number of connected and disconnected devices that have ever been connected to the system
				///  while the input monitor has been running.
				/// </summary>
				/// <returns>
				///  <para>Returns the number of input devices in the collection.</para>
				///  <para>Returns zero if no input devices have ever been discovered.</para>
				/// </returns>
				int GetCount() const;

				/// <summary>Fetches an input device from the collection by its zero based index.</summary>
				/// <param name="index">Zero based index to the device.</param>
				/// <returns>
				///  <para>Returns a pointer to the indexed input device.</para>
				///  <para>Returns null if given an invalid index.</para>
				/// </returns>
				InputDeviceInterface* GetByIndex(int index) const;

			private:
				/// <summary>
				///  <para>Reference to the input monitor that this object wraps.</para>
				///  <para>Used to access the input monitor's "fDeviceContextCollection" member variable.</para>
				/// </summary>
				InputDeviceMonitor& fDeviceMonitor;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>
		///  <para>Creates a new input device monitor initialized as "stopped".</para>
		///  <para>
		///   You must call the Start() method afterward to start monitoring devices and to receive their input data.
		///  </para>
		/// </summary>
		InputDeviceMonitor();

		/// <summary>Destroys this input device monitor.</summary>
		virtual ~InputDeviceMonitor();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets an object used to add or remove an event handler for the "DiscoveredDevice" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		DiscoveredDeviceEvent::HandlerManager& GetDiscoveredDeviceEventHandlers();

		/// <summary>Gets a read-only collection of all input devices discovered by this device monitor.</summary>
		/// <returns>Returns a read-only collection of all input devices discovered by this device monitor.</returns>
		InputDeviceMonitor::DeviceCollectionAdapter GetDeviceCollection();

		/// <summary>
		///  <para>Determines if this object is currently monitoring input devices connected to the system.</para>
		///  <para>This device monitor will be put into the running state when the Start() method gets called.</para>
		/// </summary>
		/// <returns>
		///  Returns true if this object is currently monitoring devices and their input. Returns false if not.
		/// </returns>
		bool IsRunning() const;

		/// <summary>
		///  <para>Determines if this object is not currently monitoring input devices connected to the system.</para>
		///  <para>
		///   This device monitor will not be in the running state when first created or if the Stop() method gets called.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if this object is not currently monitoring devices and their input.</para>
		///  <para>Returns false if it is actively monitoring input devices.</para>
		/// </returns>
		bool IsNotRunning() const;

		/// <summary>
		///  <para>Starts monitoring all devices connected to the system and provides their input events.</para>
		///  <para>The IsRunning() method will return true after calling this method.</para>
		///  <para>It's okay to call this method if the monitor has already been started.</para>
		/// </summary>
		void Start();

		/// <summary>
		///  <para>Stops monitoring input devices and their key/axis input.</para>
		///  <para>The IsRunning() method will return false after calling this method.</para>
		///  <para>It's okay to call this method if the monitor has already been stopped.</para>
		/// </summary>
		void Stop();

		#pragma endregion

	private:
		#pragma region Private VariantEventData Structure
		/// <summary>
		///  <para>Stores a device status change event, key event, or axis event via a union.</para>
		///  <para>Optimized to be copied cheaply by value to an array or queue, avoiding heap allocations.</para>
		///  <para>
		///   Intended to be used by this monitor's worker thread to cheaply post/queue events to all
		///   InputDeviceMonitor instances' "fVariantEventDataCollection" member variable.
		///  </para>
		/// </summary>
		struct VariantEventData
		{
			/// <summary>Zero based index to the worker thread's device handler that raised this event.</summary>
			size_t DeviceIndex;

			enum class Type : int8_t
			{
				/// <summary>Indicates that you should access the "DeviceStatus" part of the union "Data" field.</summary>
				kDeviceStatusChanged,

				/// <summary>Indicates that you should access the "Axis" part of the union "Data" field.</summary>
				kReceivedAxisInput,

				/// <summary>Indicates that you should access the "Key" part of the union "Data" field.</summary>
				kReceivedKeyInput
			}
			/// <summary>
			///  Indicates the type of event stored by this structure and how to interpret the union "Data" field.
			/// </summary>
			Type;

			union
			{
				/// <summary>
				///  <para>Provides the device status change event data.</para>
				///  <para>Only applicable if the VariantEventData's "Type" field is set to "kDeviceStatusChanged".</para>
				/// </summary>
				InputDeviceStatusChangedEventArgs::Settings DeviceStatus;

				/// <summary>
				///  <para>Provides the axis event data received from an input device.</para>
				///  <para>Only applicable if the VariantEventData's "Type" field is set to "kReceivedAxisInput".</para>
				/// </summary>
				InputAxisDataEventArgs::Data Axis;

				/// <summary>
				///  <para>Provides the key event data received from an input device.</para>
				///  <para>Only applicable if the VariantEventData's "Type" field is set to "kReceivedKeyInput".</para>
				/// </summary>
				KeyEventArgs::Data Key;
			}
			/// <summary>
			///  <para>Union storing the data for the event. Only one of these union's fields is applicable.</para>
			///  <para>You must query the VariantEventData's "Type" field to know which field in the union to access.</para>
			/// </summary>
			Data;
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>
		///  <para>
		///   Called when a Windows message has been received by the InputDeviceMonitor instance's message-only window.
		///  </para>
		///  <para>Used by the worker thread to post input events to the input monitor's thread.</para>
		/// </summary>
		/// <param name="sender">Reference to the message-only window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments);

		#pragma endregion


		#pragma region Private Static Functions
		/// <summary>Called by this class' worker thread to monitor input devices.</summary>
		static void OnAsyncExecute();

		/// <summary>
		///  <para>
		///  To be called by the worker thread to add newly discovered devices and to update all existing
		///  devices statuses and configuration stored by the worker thread.
		///  </para>
		///  <para>Note: This function call is a large performance hit and should only be called when needed.</para>
		/// </summary>
		/// <param name="directInputPointer">
		///  <para>The COM interface pointer to DirectInput. This is needed to query for all DirectInput devices.</para>
		///  <para>Can be null, in which case, this function will not query for DirectInput devices.</para>
		/// </param>
		/// <param name="windowHandle">
		///  Handle to a Win32 window. Needed to put DirectInput devices into exclusive mode for vibration support.
		///  Can be null.
		/// </param>
		static void AsyncUpdateAllDevicesUsing(LPDIRECTINPUT8W directInputPointer, HWND windowHandle);

		/// <summary>
		///  <para>
		///   A WndProc C callback that gets invoked when the worker thread's invisible window has received a message.
		///  </para>
		///  <para>Used to receive notifications when an input device has been added/removed from the system.</para>
		/// </summary>
		/// <param name="windowHandle">Handle to the window or control that is receiving the message.</param>
		/// <param name="messageId">Unique integer ID of the message such as WM_DEVICECHANGE.</param>
		/// <param name="wParam">Additional information assigned to the message.</para>
		/// <param name="lParam">Additional information assigned to the message.</para>
		/// <returns>Returns a value to the source of the Windows message.</returns>
		static LRESULT CALLBACK OnAsyncReceivedMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam);

		/// <summary>
		///  <para>
		///   Called by the worker thread's device handlers when a connection state has change or the device
		///   has been reconfigured.
		///  </para>
		///  <para>Posts the event to all InputDeviceMonitor instances that are actively running.</para>
		/// </summary>
		/// <param name="sender">Reference to the input device that raised this event.</param>
		/// <param name="arguments">Indicates what aspects of the device's status has changed.</param>
		static void OnAsyncDeviceStatusChanged(
				InputDeviceInterface& sender, const InputDeviceStatusChangedEventArgs& arguments);

		/// <summary>
		///  <para>Called by the worker thread's device handlers when new axis data has been received.</para>
		///  <para>Posts the event to all InputDeviceMonitor instances that are actively running.</para>
		/// </summary>
		/// <param name="sender">Reference to the input device that raised this event.</param>
		/// <param name="arguments">Provides the axis data that was received.</param>
		static void OnAsyncReceivedAxisInput(InputDeviceInterface& sender, const InputAxisDataEventArgs& arguments);

		/// <summary>
		///  <para>Called by the worker thread's device handlers when a key has been pressed or released.</para>
		///  <para>Posts the event to all InputDeviceMonitor instances that are actively running.</para>
		/// </summary>
		/// <param name="sender">Reference to the input device that raised this event.</param>
		/// <param name="arguments">Provides the key input that was received.</param>
		static void OnAsyncReceivedKeyInput(InputDeviceInterface& sender, const KeyEventArgs& arguments);

		/// <summary>
		///  To be called by the worker thread to post/queue a device event to all InputDeviceMonitor instances
		///  that are currently running.
		/// </summary>
		/// <param name="sender">Reference to the input device that raised the event.</param>
		/// <param name="variantEventData">
		///  A union storing a device status change event, axis input event, or key input event.
		/// </param>
		static void AsyncPostEvent(InputDeviceInterface& sender, InputDeviceMonitor::VariantEventData& variantEventData);

		/// <summary>Called by Microsoft's DirectInput for every device it has discovered.</summary>
		/// <param name="deviceInstancePointer">COM interface pointer to a device discovered by DirectInput.</param>
		/// <param name="contextPointer">
		///  <para>Optional application defined pointer.</para>
		///  <para>Used by this monitor's worker thread to store DirectInput instance IDs of all dveices found.</para>
		/// </param>
		static BOOL FAR PASCAL OnAsyncEnumDirectInputDevice(
				LPCDIDEVICEINSTANCEW deviceInstancePointer, LPVOID contextPointer);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Mutex used to synchronize access to an input monitor's member variables.</summary>
		std::recursive_mutex fMutex;

		/// <summary>Manages the "DiscoveredDevice" event.</summary>
		DiscoveredDeviceEvent fDiscoveredDeviceEvent;

		/// <summary>Set true if the input monitor is currently running. Set false if not.</summary>
		bool fIsRunning;

		/// <summary>
		///  Pointer to a Win32 message-only window used by this monitor to send notifications to itself
		///  from a background thread to the thread that created this input monitor.
		/// </summary>
		Interop::UI::MessageOnlyWindow* fMessageOnlyWindowPointer;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		Interop::UI::UIComponent::ReceivedMessageEvent::MethodHandler<InputDeviceMonitor> fReceivedMessageEventHandler;

		/// <summary>
		///  <para>Unique Windows message ID reserved by the Win32 message-only window for this device monitor.</para>
		///  <para>
		///   Used by the worker thread to notify a device monitor that events have been queued to its
		///   "fVariantEventDataCollection" member variable.
		///  </para>
		/// </summary>
		UINT fReservedMessageId;

		/// <summary>
		///  <para>Stores a queue of newly received input device event data received by the worker thread.</para>
		///  <para>Event data is stored as a union to cheaply copy them by value and avoid memory allocations.</para>
		///  <para>
		///   This queue is really a C++ STL vector to avoid memory allocations as well.
		///   (ie: Erased elements does not cause a memory re-allocation.)
		///  </para>
		/// </summary>
		std::vector<InputDeviceMonitor::VariantEventData> fVariantEventDataCollection;

		/// <summary>
		///  <para>Collection of input device contexts discovered on the system.</para>
		///  <para>Stores the device's connection status, configuration, and current axis/key data values.</para>
		///  <para>
		///   Each context will raise an event when the device status has changed or new input data has been received.
		///  </para>
		///  <para>
		///   The device monitor is expected to keep this collection in sync with the worker thread's collection.
		///   The order of the devices between threads is expected to match as well.
		///  </para>
		/// </summary>
		std::vector<InputDeviceContext*> fDeviceContextCollection;

		/// <summary>
		///  Handler to be invoked when the "StatusChanged" event has been raised by a device handler
		///  belonging to the worker thread.
		/// </summary>
		static InputDeviceInterface::StatusChangedEvent::FunctionHandler sDeviceStatusChangedEventHandler;

		/// <summary>
		///  Handler to be invoked when the "ReceivedAxisInput" event has been raised by a device handler
		///  belonging to the worker thread.
		/// </summary>
		static InputDeviceInterface::ReceivedAxisInputEvent::FunctionHandler sReceivedAxisInputEventHandler;

		/// <summary>
		///  Handler to be invoked when the "ReceivedKeyInput" event has been raised by a device handler
		///  belonging to the worker thread.
		/// </summary>
		static InputDeviceInterface::ReceivedKeyInputEvent::FunctionHandler sReceivedKeyInputEventHandler;

		#pragma endregion
};

} }	// namespace Interop::Input
