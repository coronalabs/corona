//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include "Interop\Ticks.h"
#include "InputDeviceInterface.h"


namespace Interop { namespace Input {

/// <summary>
///  <para>Listens for vibration requests and manages the current vibration state for one input device.</para>
///  <para>
///   Instances of this class are expected to be held by MInputDeviceHandler derived class to process to
///   manage a device's current vibration state.
///  </para>
///  <para>
///   This manager's vibration request handler is expected to passed into the constructor of all InputDeviceContext
///   objects associated with an MInputDeviceHandler's device. These contexts will raise the vibration requests.
///  </para>
/// </summary>
class VibrationRequestManager
{
	public:
		#pragma region RequestType Enum
		/// <summary>Indicates what vibration operation should be performed on a device.</summary>
		enum class RequestType
		{
			/// <summary>
			///  <para>Requests the device handler to do nothing with the device.</para>
			///  <para>This means keep the device in its current vibration state (ie: vibrating or not vibrating).</para>
			/// </summary>
			kDoNothing,

			/// <summary>Requests the device handler to "start" vibrating the device.</summary>
			kStart,

			/// <summary>Requests the device handler to "stop" vibrating the device.</summary>
			kStop
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new vibration request manager.</summary>
		VibrationRequestManager();

		/// <summary>Destroys this object.</summary>
		virtual ~VibrationRequestManager();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Gets an event handler used to receive vibration requests.</para>
		///  <para>
		///   Invoking this handler will flag the device to be vibrated the next time this manager's
		///   ProcessRequests() method gets called.
		///  </para>
		///  <para>
		///   This handler is expected to be passed into an InputDeviceContext's constructor associated with
		///   the device to be vibrated.
		///  </para>
		/// </summary>
		/// <returns>Returns an event handler used to received/handle vibration requests.</returns>
		const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* GetRequestHandler() const;

		/// <summary>
		///  <para>
		///   Processes all vibration requests received by the handler returned by the GetRequestHandler() method.
		///  </para>
		///  <para>
		///   Returns what vibration action should be performed on a device and updates the value returned
		///   by the WasVibrationRequested() method.
		///  </para>
		///  <para>
		///   This method is expected to be called at regular intervals, such as by an MInputDeviceHandler's Poll() method.
		///  </para>
		/// </summary>
		/// <returns>
		///  Returns what vibration action should be performed on the input device such as kStart, kStop, or kDoNothing.
		///  The kDoNothing action indicates that the device should be left in its current state.
		/// </returns>
		VibrationRequestManager::RequestType ProcessRequests();

		/// <summary>
		///  <para>Clears all vibration requests and resets the current vibration state of the device to stopped.</para>
		///  <para>This is expected to be called by an MInputDeviceHandler when detaching/disconnecting from a device.</para>
		/// </summary>
		void Reset();

		/// <summary>
		///  <para>Determines if the device should be currently vibrating/rumbling or not.</para>
		///  <para>The value returned by this method only changes after calling the ProcessRequests() method.</para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns true if a device should be currently vibrating. This happens when vibration requests
		///   have been received by this manager's event handler from an InputDeviceInterface.
		///  </para>
		///  <para>Returns false if the device should no longer be vibrating.</para>
		/// </returns>
		bool WasVibrationRequested() const;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Called when a vibration request has been received.</summary>
		/// <param name="sender">The object that is raising this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnReceivedVibrationRequest(InputDeviceInterface& sender, const EventArgs& arguments);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Handler to be invoked when a "ReceivedVibrationRequest" event has been raised.</summary>
		InputDeviceInterface::ReceivedVibrationRequestEvent::MethodHandler<VibrationRequestManager> fReceivedVibrationRequestEventHandler;

		/// <summary>Set true if the device should be currently vibrating. Set false if not.</summary>
		bool fIsDeviceVibrating;

		/// <summary>
		///  <para>Set true if a vibration request has just been received.</para>
		///  <para>This flag is expected to be cleared when the ProcessRequests() method gets called.</para>
		/// </summary>
		bool fHasReceivedVibrationRequest;

		/// <summary>
		///  <para>The time in system ticks that the device should stop vibrating.</para>
		///  <para>Only applies if member variable "fIsDeviceVibrating" is set true.</para>
		/// </summary>
		Ticks fEndVibrationTimeInTicks;

		#pragma endregion
};

} }	// namespace Interop::Input
