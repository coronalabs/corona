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
#include "Interop\ValueResult.h"
#include "InputDeviceContext.h"
#include "MInputDeviceHandler.h"
#include "VibrationRequestManager.h"
#include <Xinput.h>


namespace Interop { namespace Input {

/// <summary>
///  <para>Handler used to access, control, and read data from one XInput device.</para>
///  <para>Instances of this class are only expected to be created and used by the "InputDeviceMonitor" class.</para>
/// </summary>
class XInputDeviceHandler : public MInputDeviceHandler
{
	Rtt_CLASS_NO_COPIES(XInputDeviceHandler)

	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new XInput device handler.</summary>
		XInputDeviceHandler();

		/// <summary>Detaches this handler from its device (if attached) and then destroys this handler.</summary>
		virtual ~XInputDeviceHandler();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Determines if this handler is currently attached/bound to a connected input device.</para>
		///  <para>This object's Poll() method will only collect input data if attached to a device.</para>
		/// </summary>
		/// <returns>Returns true if currently connected to a device. Returns false if not.</returns>
		virtual bool IsAttached() const override;

		/// <summary>
		///  <para>
		///   Attaches this handler to the device associated with the given XInput user index and device capabilities.
		///  </para>
		///  <para>The XInput device is assumed to be connected if this method is called.</para>
		/// </summary>
		/// <param name="userIndex">Zero based XInput user index this handler will poll for device info/data.</para>
		/// <param name="deviceCapabilities">
		///  Reference to the XInput device capabilites retrieved via Microsoft's XInputGetCapabilities() function.
		/// </param>
		void AttachTo(DWORD userIndex, const XINPUT_CAPABILITIES& deviceCapabilities);

		/// <summary>
		///  <para>Detaches this handler from its input device.</para>
		///  <para>This prevents the Poll() method from working since it is no longer attached.</para>
		/// </summary>
		virtual void Detach() override;

		/// <summary>
		///  <para>Gets a device context that this handler uses store the device config and its connection status.</para>
		///  <para>
		///   This context will raise events when the device status has changed or when new input data has been received.
		///  </para>
		/// </summary>
		/// <returns>
		///  Returns a reference to this handler's device context which stores the device config, connection status,
		///  and provides device related events.
		/// </returns>
		virtual InputDeviceContext& GetContext() override;

		/// <summary>Gets the XInput user index this handler is currently or last attached to.</summary>
		/// <returns>
		///  <para>
		///   Returns a success result whose GetValue() method returns the XInput user index this handler
		///   was last attached to.
		///  </para>
		///  <para>Returns a failure result if this handler has never been attached an XInput device before.</para>
		/// </returns>
		ValueResult<DWORD> GetLastAttachedUserIndex() const;

		/// <summary>
		///  Determines if the given XInput device capabilities match this handler current or
		///  last attached device's capabilities.
		/// </summary>
		/// <param name="deviceCapabilities">The device capabilities/configuration to be compared with.</param>
		/// <returns>
		///  <para>Returns true if the device capabilties/configuration match.</para>
		///  <para>Returns false if they don't match or if this handler has never been attached to a device before.</para>
		/// </returns>
		bool IsLastAttachedEqualTo(const XINPUT_CAPABILITIES& deviceCapabilities) const;

		/// <summary>
		///  <para>
		///   Checks the device's connection status, fetches new input data (if available), and controls the device's
		///   vibration/rumble motors.
		///  </para>
		///  <para>This method is expected to be called at regular intervals.</para>
		/// </summary>
		virtual void Poll() override;

		#pragma endregion

	private:
		#pragma region Private XInputAxisIndexMap Struct
		/// <summary>Stores a mapping of XInput axes to device context axis indexes.</summary>
		struct XInputAxisIndexMap
		{
			/// <summary>
			///  <para>Provides the InputDeviceContext's axis index for the XInput left thumbstick x-axis.</para>
			///  <para>Set to -1 if it doesn't index an axis in the InputDeviceContext.</para>
			/// </summary>
			int ThumbLX;

			/// <summary>
			///  <para>Provides the InputDeviceContext's axis index for the XInput left thumbstick y-axis.</para>
			///  <para>Set to -1 if it doesn't index an axis in the InputDeviceContext.</para>
			/// </summary>
			int ThumbLY;

			/// <summary>
			///  <para>Provides the InputDeviceContext's axis index for the XInput rogjt thumbstick x-axis.</para>
			///  <para>Set to -1 if it doesn't index an axis in the InputDeviceContext.</para>
			/// </summary>
			int ThumbRX;

			/// <summary>
			///  <para>Provides the InputDeviceContext's axis index for the XInput left thumbstick y-axis.</para>
			///  <para>Set to -1 if it doesn't index an axis in the InputDeviceContext.</para>
			/// </summary>
			int ThumbRY;

			/// <summary>
			///  <para>Provides the InputDeviceContext's axis index for the XInput left analog trigger.</para>
			///  <para>Set to -1 if it doesn't index an axis in the InputDeviceContext.</para>
			/// </summary>
			int LeftTrigger;

			/// <summary>
			///  <para>Provides the InputDeviceContext's axis index for the XInput right analog trigger.</para>
			///  <para>Set to -1 if it doesn't index an axis in the InputDeviceContext.</para>
			/// </summary>
			int RightTrigger;

			/// <summary>Creates a new XInput axis index mapping object.</summary>
			XInputAxisIndexMap();

			/// <summary>Resets all of the axis indexes to -1.</summary>
			void Reset();
		};

		#pragma endregion


		#pragma region Private Static Functions
		/// <summary>
		///  <para>
		///   Generates a new device model name and display name using the given XInput user index and device capabilities
		///   and then applies these names to the given device settings object.
		///  </para>
		///  <para>Intended to be called when the user index or device capabilities has been changed.</para>
		/// </summary>
		/// <param name="userIndex">
		///  <para>Zero based XInput user index the device capabilites was retrieved from.</para>
		///  <para>Set to a negative number if the handler is no longer attached to an XInput user index.</para>
		/// </param>
		/// <param name="deviceCapabilities">The XInput device's capabilities/configuration.</param>
		/// <param name="deviceSettings">
		///  The settings this function will copy the newly generated device model name and display name to.
		/// </param>
		static void UpdateDeviceNameUsing(
				int userIndex, const XINPUT_CAPABILITIES& deviceCapabilities, InputDeviceSettings& deviceSettings);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>
		///  <para>Manages what the current vibration state should be for the device.</para>
		///  <para>Receives external vibration requests via InputDeviceInterface objects to start vibration.</para>
		/// </summary>
		/// <remarks>
		///  WARNING! This member variable must be defined before the "fDeviceContext" member variable.
		///  This is because this object's event handler is passed into "fDeviceConext" object's constructor.
		/// </remarks>
		VibrationRequestManager fVibrationRequestManager;

		/// <summary>
		///  The context this handler uses to update with retrieved device settings, input data, and connection status.
		/// </summary>
		InputDeviceContext fDeviceContext;

		/// <summary>
		///  <para>Stores the XInput user index this handler is currently or last attached to.</para>
		///  <para>Set to -1 if this handler has never been attached to an XInput user index.</para>
		/// </summary>
		int fLastAttachedUserIndex;

		/// <summary>
		///  Stores the XInput device capabilities/config that this handler is currently or last attached to.
		/// </summary>
		XINPUT_CAPABILITIES fLastAttachedXInputDeviceCapabilities;

		/// <summary>
		///  <para>Set true if this handler has received at least 1 packet from the device since it's been attached.</para>
		///  <para>Set false if this device has not received any input data yet.</para>
		///  <para>This flag is used in conjunction with the "fLastReceivedPacketNumber" member variable.</para>
		/// </summary>
		bool fHasReceivedData;

		/// <summary>
		///  <para>Stores the last packet number received from the XInput device.</para>
		///  <para>Only applicable if member variable "fHasReceivedData" is currently set true.</para>
		///  <para>Used to prevent recording duplicate data to the device context. (Improves performance.)</para>
		/// </summary>
		DWORD fLastReceivedPacketNumber;

		/// <summary>Stores a mapping between XInput axes and this handler device context axis indexes.</summary>
		XInputAxisIndexMap fAxisIndexMap;

		#pragma endregion
};

} }	// namespace Interop::Input
