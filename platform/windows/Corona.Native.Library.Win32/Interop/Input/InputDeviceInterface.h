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
#include "Input\Rtt_InputDeviceConnectionState.h"
#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include "InputAxisDataEventArgs.h"
#include "InputDeviceDriverType.h"
#include "InputDeviceInfo.h"
#include "InputDeviceStatusChangedEventArgs.h"
#include "KeyEventArgs.h"
#include <memory>


namespace Interop { namespace Input {

class InputDeviceContext;

/// <summary>
///  <para>
///   Provides client access to an input device's read-only configuration, input/status events,
///   and the ability to request device vibration/rumble.
///  </para>
///  <para>
///   Instances of this class would normally be acquired from an "InputDeviceMonitor" or "InputDeviceContext" object.
///  </para>
/// </summary>
class InputDeviceInterface final
{
	Rtt_CLASS_NO_COPIES(InputDeviceInterface)

	public:
		#pragma region Public Event Types
		/// <summary>
		///  Defines the "StatusChanged" event type which is raised when the input device's
		///  connection state or configuration has changed.
		/// </summary>
		typedef Event<InputDeviceInterface&, const InputDeviceStatusChangedEventArgs&> StatusChangedEvent;

		/// <summary>
		///  Defines the "ReceivedAxisInput" event which is raised when one of the input device's
		///  axes has received new data.
		/// </summary>
		typedef Event<InputDeviceInterface&, const InputAxisDataEventArgs&> ReceivedAxisInputEvent;

		/// <summary>
		///  Defines the "ReceivedKeyInput" event which is raised when one of the input device's
		///  keys/buttons has been pressed or released.
		/// </summary>
		typedef Event<InputDeviceInterface&, const KeyEventArgs&> ReceivedKeyInputEvent;

		/// <summary>
		///  <para>
		///   Defines the "ReceivedVibrationRequest" event which is raised when the InputDeviceInterface
		///   object's RequestVibrate() method has been called.
		///  </para>
		///  <para>Intended to be handled by the InputDeviceMonitor to vibrate/rumble the physical input device.</para>
		/// </summary>
		typedef Event<InputDeviceInterface&, const EventArgs&> ReceivedVibrationRequestEvent;

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new client interface to the given input device context.</summary>
		/// <param name="context">Reference to the input device context the new interface object will wrap.</param>
		InputDeviceInterface(InputDeviceContext& context);

		/// <summary>Destroys this wrapper interface.</summary>
		virtual ~InputDeviceInterface();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets an object used to add or remove an event handler for the "StatusChanged" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		StatusChangedEvent::HandlerManager& GetStatusChangedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "ReceivedAxisInput" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ReceivedAxisInputEvent::HandlerManager& GetReceivedAxisInputEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "ReceivedKeyInput" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ReceivedKeyInputEvent::HandlerManager& GetReceivedKeyInputEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "ReceivedVibrationRequest" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ReceivedVibrationRequestEvent::HandlerManager& GetReceivedVibrationRequestEventHandlers();

		/// <summary>Gets the type of driver that controls the device such as DirectInput, RawInput, or XInput.</summary>
		/// <returns>Returns the driver that controls the device such as DirectInput, RawInput, or XInput.</returns>
		InputDeviceDriverType GetDriverType() const;

		/// <summary>
		///  <para>Gets a shared pointer to the input device's current configuration.</para>
		///  <para>If the device configuration changes, then a new shared pointer will be returned.</para>
		/// </summary>
		/// <returns>Returns a shared pointer to the input device's current configuration.</returns>
		std::shared_ptr<InputDeviceInfo> GetDeviceInfo() const;

		/// <summary>Gets the input device's current connection state such as kConnected or kDisconnected.</summary>
		/// <returns>Returns the device's current connection state such as kConnected or kDisconnected.</returns>
		Rtt::InputDeviceConnectionState GetConnectionState() const;

		/// <summary>Determines if the device is currently connected to the system and can provide input.</summary>
		/// <returns>Returns true if the input device is currently connected. Returns false if not.</returns>
		bool IsConnected() const;

		/// <summary>
		///  <para>Requests the device to be vibrated/rumbled for a short amount of time.</para>
		///  <para>Calling this method will cause the "ReceivedVibrationRequest" event to be raised.</para>
		/// </summary>
		void RequestVibrate();

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>Reference to the mutable input device context this interface wraps.</summary>
		InputDeviceContext& fDeviceContext;

		#pragma endregion
};

} }	// namespace Interop::Input
