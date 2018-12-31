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
#include "Input\Rtt_InputDeviceConnectionState.h"
#include "Interop\Event.h"
#include "InputAxisDataEventArgs.h"
#include "InputDeviceDriverType.h"
#include "InputDeviceInfo.h"
#include "InputDeviceInterface.h"
#include "InputDeviceStatusChangedEventArgs.h"
#include "Key.h"
#include "KeyEventArgs.h"
#include <memory>
#include <unordered_map>


namespace Interop { namespace Input {

class InputDeviceSettings;

/// <summary>
///  <para>Stores the configuration, connection status, and data of one input device.</para>
///  <para>Raises events when the device status has changed and when new data has been received.</para>
///  <para>
///   Instances of this class are intended to be used by MInputDeviceHandler derived classes to manage
///   device info/data consistently between DirectInput, RawInput, and XInput.
///  </para>
/// </summary>
class InputDeviceContext final
{
	Rtt_CLASS_NO_COPIES(InputDeviceContext)

	private:
		friend class InputDeviceInterface;

	public:
		#pragma region ScopedUpdater Class
		/// <summary>
		///  <para>
		///   Object whose constructor calls an InputDeviceContext's BeginUpdate() and destructor calls EndUpdate().
		///  </para>
		///  <para>
		///   Intended to be used as a local variable to ensure that EndUpdate() gets called when the variable
		///   falls out of scope.
		///  </para>
		/// </summary>
		class ScopedUpdater
		{
			Rtt_CLASS_NO_COPIES(ScopedUpdater)

			public:
				/// <summary>Calls the given input device context's BeginUpdate() method.</summary>
				/// <param name="context">
				///  The input device context whose BeginUpdate() and EndUpdate() methods will be invoked.
				/// </param>
				ScopedUpdater(InputDeviceContext& context);

				/// <summary>Calls the input device context's EndUpdate() method.</summary>
				virtual ~ScopedUpdater();

			private:
				/// <summary>The input device context to call BeginUpdate() and EndUpdate() on.</summary>
				InputDeviceContext& fContext;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new input device context initialized to its defaults and without vibration support.</summary>
		/// <param name="driverType">Indicates if the context is controller by DirectInput, RawInput, or XInput.</param>
		InputDeviceContext(const InputDeviceDriverType& driverType);

		/// <summary>Creates a new input device context initialized to its defaults.</summary>
		/// <param name="driverType">Indicates if the context is controller by DirectInput, RawInput, or XInput.</param>
		/// <param name="vibrationRequestHandlerPointer">
		///  <para>
		///   Pointer to an event handler which will vibrate/rumble the device if this context's
		///   InputDeviceInterface RequestVibrate() method gets called.
		///  </para>
		///  <para>Can be null, in which case, the device cannot be vibrated/rumbled.</para>
		/// </para>
		InputDeviceContext(
				const InputDeviceDriverType& driverType,
				const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* vibrationRequestHandlerPointer);

		/// <summary>Destroys this object.</summary>
		virtual ~InputDeviceContext();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  Gets client access to the input device's read-only configuration, input/status events,
		///  and the ability to request device vibration/rumble.
		/// </summary>
		/// <returns>Returns client access to this context's device information, events, and other features.</returns>
		InputDeviceInterface& GetDeviceInterface();

		/// <summary>
		///  Gets client access to the input device's read-only configuration, input/status events,
		///  and the ability to request device vibration/rumble.
		/// </summary>
		/// <returns>Returns client access to this context's device information, events, and other features.</returns>
		const InputDeviceInterface& GetDeviceInterface() const;

		/// <summary>
		///  <para>Gets a pointer to a handler used to vibrate/rumble the device when invoked.</para>
		///  <para>This handler will automatically be invoked when the "ReceivedVibrationRequest" event gets raised.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to a handler used to vibrate this context's associated device.</para>
		///  <para>
		///   Returns null if a vibration request handler was not provided to this context upon construction.
		///   This means its associated device cannot be vibrated/rumbled.
		///  </para>
		/// </returns>
		const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* GetVibrationRequestHandler() const;

		/// <summary>Determines if this context's device information and data is currently being updated.</summary>
		/// <returns>
		///  <para>
		///   Returns true if the BeginUpdate() method has been called, which means its UpdateWith() method(s)
		///   are being called.
		///  </para>
		///  <para>Returns false if the device context is not currently being updated.</para>
		/// </returns>
		bool IsUpdating() const;

		/// <summary>
		///  <para>Informs this context that its BeginUpdate() methods are about to be called.</para>
		///  <para>
		///   This prevents an UpdateWith() method from raising events until the EndUpdate() method gets called.
		///  </para>
		///  <para>This allows the caller to make several updates to the context as a single atomic transaction.</para>
		///  <para>
		///   This should be used when updating the device's configuration and connection state because a
		///   single "StatusChanged" event is raised for both.
		///  </para>
		///  <para>
		///   The changes made during a BeginUpdate() will not be committed to this object until an EndUpdate() gets called.
		///  </para>
		///  <para>You must call EndUpdate() for every call to the BeginUpdate() method.</para>
		///  <para>
		///   Note: It is recommended that you use the InputDeviceContext::ScopedUpdater class which guarrantees
		///   that the BeginUpdate() and EndUpdate() method calls are always paired.
		///  </para>
		/// </summary>
		void BeginUpdate();

		/// <summary>
		///  <para>Updates the connection state of the input device.</para>
		///  <para>If the state has changed, then the context will raise a "StatusChanged" event.</para>
		///  <para>
		///   If BeginUpdate() was called, then this method will not raise an event until EndUpdate() gets called.
		///  </para>
		/// </summary>
		/// <param name="value">The new connection state such as kConnected or kDisconnected.</param>
		void UpdateWith(const Rtt::InputDeviceConnectionState& value);

		/// <summary>
		///  <para>Updates the stored device configuration.</para>
		///  <para>
		///   If the given configuration is different than the old configuration, then this context will
		///   raise a "StatusChanged" event.
		///  </para>
		///  <para>
		///   If BeginUpdate() was called, then this method will not raise an event until EndUpdate() gets called.
		///  </para>
		/// </summary>
		/// <param name="settings">The new device configuration to update the context with.</param>
		void UpdateWith(const InputDeviceSettings& settings);

		/// <summary>
		///  <para>Updates the stored device configuration.</para>
		///  <para>
		///   If the given configuration is different than the old configuration, then this context will
		///   raise a "StatusChanged" event.
		///  </para>
		///  <para>
		///   If BeginUpdate() was called, then this method will not raise an event until EndUpdate() gets called.
		///  </para>
		/// </summary>
		/// <param name="infoPointer">
		///  <para>Shared pointer to an immutable device configuration to update the context with.</para>
		///  <para>The context's configuration will not be updated if given a null pointer.</para>
		/// </param>
		void UpdateWith(const std::shared_ptr<InputDeviceInfo>& infoPointer);

		/// <summary>
		///  <para>Updates the current key state stored by this context.</para>
		///  <para>Will raise a "ReceivedKeyInput" event if the key state has changed.</para>
		///  <para>The given event data will be ignored if the key does not exist in the device configuration.</para>
		///  <para>
		///   If BeginUpdate() was called, then this method will not raise an event until EndUpdate() gets called.
		///  </para>
		/// </summary>
		/// <param name="data">The key data that was received from the input device.</param>
		void UpdateWith(const InputAxisDataEventArgs::Data& data);

		/// <summary>
		///  <para>Updates the axis data stored by this context.</para>
		///  <para>Will raise a "ReceivedAxisInput" event if the axis data has changed.</para>
		///  <para>The given event data will be ignored if the axis index does not exist in the device configuration.</para>
		///  <para>
		///   If BeginUpdate() was called, then this method will not raise an event until EndUpdate() gets called.
		///  </para>
		/// </summary>
		/// <param name="data">The axis data that was received from the input device.</param>
		void UpdateWith(const KeyEventArgs::Data& data);

		/// <summary>
		///  <para>This method must be called after this context's BeginUpdate() has been called.</para>
		///  <para>
		///   Calling this method commits any changes made via the UpdateWith() methods, making the changes available
		///   via this context's Get*() methods.
		///  </para>
		///  <para>
		///   If any changes were detected, then this context will raise events such as "StatusChanged",
		///   "AxisInputReceived", and/or "KeyInputReceived".
		///  </para>
		/// </summary>
		void EndUpdate();

		#pragma endregion

	private:
		#pragma region Private PendingAxisData Struct
		/// <summary>
		///  <para>Stores the data for one axis received by the context's UpdateWith() method.</para>
		///  <para>
		///   Provides an additional "IsNew" flag used by EndUpdate() to determine if the data value has changed
		///   and if an event should be raised.
		///  </para>
		///  <para>This struct is only expected to be used by the PendingUpdateData struct's axis collection.</para>
		/// </summary>
		struct PendingAxisData
		{
			/// <summary>
			///  <para>Set true if the axis data value is different compared to the previously received value.</para>
			///  <para>Set false if the axis data value is the same as before.</para>
			///  <para>This flag is assigned by EndUpdate() and used to determine if an axis event should be raised.</para>
			/// </summary>
			bool IsNew;

			/// <summary>Stores the axis data received via the context's UpdateWith() method.</summary>
			InputAxisDataEventArgs::Data EventData;
		};

		#pragma endregion


		#pragma region Private PendingKeyData Struct
		/// <summary>
		///  <para>Stores the data for one key event received by the context's UpdateWith() method.</para>
		///  <para>
		///   Provides an additional "IsNew" flag used by EndUpdate() to determine if the data value has changed
		///   and if an event should be raised.
		///  </para>
		///  <para>This struct is only expected to be used by the PendingUpdateData struct's key collection.</para>
		/// </summary>
		struct PendingKeyData
		{
			/// <summary>
			///  <para>Set true if the key state is different compared to the previously received key state.</para>
			///  <para>Set false if the key state is the same as before.</para>
			///  <para>This flag is assigned by EndUpdate() and used to determine if a key event should be raised.</para>
			/// </summary>
			bool IsNew;

			/// <summary>Stores the key event data received via the context's UpdateWith() method.</summary>
			KeyEventArgs::Data EventData;
		};

		#pragma endregion


		#pragma region Private PendingUpdateData Struct
		/// <summary>
		///  <para>
		///   Stores device configuration changes, connection changes, and axis/key event data received
		///   by this context's UpdateWith() methods.
		///  </para>
		///  <para>
		///   This data is then expected to be applied to the context when EndUpdate() gets called,
		///   which will update member variables and raise events for any changes detected.
		///  </para>
		/// </summary>
		struct PendingUpdateData
		{
			/// <summary>Indicates if we're currently connected or disconnected with the device.</summary>
			Rtt::InputDeviceConnectionState ConnectionState;

			/// <summary>
			///  <para>Shared pointer to the device's new configuration.</para>
			///  <para>Set to null if the device configuration has not changed during an update.</para>
			/// </summary>
			std::shared_ptr<InputDeviceInfo> DeviceInfoPointer;

			/// <summary>
			///  <para>Collection of axis data received from a device.</para>
			///  <para>
			///   This collection is an STL vector instead of a map for best performance and to avoid
			///   heap allocations/de-allocations when adding/removing data frequently.
			///  </para>
			///  <para>The owner of this struct is expected to not add duplicate axis indexes to this collection.</para>
			/// </summary>
			std::vector<PendingAxisData> AxisDataCollection;

			/// <summary>
			///  <para>Collection of key up/down event data received from a device.</para>
			///  <para>
			///   This collection is an STL "vector" instead of a "map" for best performance and to avoid
			///   heap allocations/de-allocations when adding/removing data frequently.
			///  </para>
			///  <para>The owner of this collection is expected to not add duplicate key codes to this collection.</para>
			/// </summary>
			std::vector<PendingKeyData> KeyDataCollection;
		};

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Manages the "StatusChanged" event.</summary>
		InputDeviceInterface::StatusChangedEvent fStatusChangedEvent;

		/// <summary>Manages the "ReceivedAxisInput" event.</summary>
		InputDeviceInterface::ReceivedAxisInputEvent fReceivedAxisInputEvent;

		/// <summary>Manages the "ReceivedKeyInput" event.</summary>
		InputDeviceInterface::ReceivedKeyInputEvent fReceivedKeyInputEvent;

		/// <summary>Manages the "ReceivedVibrationRequest" event.</summary>
		InputDeviceInterface::ReceivedVibrationRequestEvent fReceivedVibrationRequestEvent;

		/// <summary>
		///  Indicates the driver/library that is accessing this input device such as kXInput, kDirectInput, or kRawInput.
		/// </summary>
		const InputDeviceDriverType fDriverType;

		/// <summary>Stores the input device's current configuration.</summary>
		std::shared_ptr<InputDeviceInfo> fDeviceInfoPointer;

		/// <summary>Indicates if the input device is currently connected or disconnected.</summary>
		Rtt::InputDeviceConnectionState fConnectionState;

		/// <summary>
		///  <para>Stores the last recorded values from all axis inputs.</para>
		///  <para>This is a hash table which uses an axis index as the key.</para>
		///  <para>If an axis index entry is not in the collection, then it hasn't recorded any data yet.</para>
		/// </summary>
		std::unordered_map<unsigned int, int32_t> fLastReceivedAxisData;

		/// <summary>
		///  <para>Stores the last recorded up/down state from all keys/buttons.</para>
		///  <para>This is a hash table which uses a "Key" object as the key.</para>
		///  <para>If a key entry is not in the collection, then it hasn't recorded any data yet.</para>
		/// </summary>
		std::unordered_map<Key, bool> fLastReceivedKeyStates;

		/// <summary>
		///  <para>Incremented when BeginUpdate() gets called and decremented when EndUpdate() gets called.</para>
		///  <para>Used to keep count of the number of update transactions that is currently occurring.</para>
		///  <para>
		///   If this value is greater than zero, then this context is in middle of being updated
		///   and all update events will be suppressed.
		///  </para>
		///  <para>Update events will only be raised when this value gets decremented to zero in EndUpdate().</para>
		/// </summary>
		int fUpdateTransactionCount;

		/// <summary>
		///  <para>Stores device, key, and axis data given to this context via the UpdateWith() methods.</para>
		///  <para>This data is only applied to this context's member variables when EndUpdate() gets called.</para>
		///  <para>Configuration and data values changes are also detected by EndUpdate() and raised as events.</para>
		/// </summary>
		PendingUpdateData fPendingUpdateData;

		/// <summary>
		///  Provides read-only client access to this context's device configuration, input/status events,
		///  and the ability to request device vibration/rumble.
		/// </summary>
		InputDeviceInterface fDeviceInterface;

		/// <summary>
		///  <para>
		///   Pointer to a vibration request handler used to vibrate this context's associated device when invoked.
		///  </para>
		///  <para>Can be null, in which case, the device cannot be vibrated by the InputDeviceInterface.</para>
		/// </summary>
		const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* fVibrationRequestEventHandlerPointer;

		#pragma endregion
};

} }	// namespace Interop::Input
