//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Input\Rtt_InputDeviceConnectionState.h"
#include "Input\Rtt_PlatformInputDevice.h"
#include "Interop\Input\InputDeviceInterface.h"


#pragma region Forward Declarations
namespace Interop {
	class RuntimeEnvironment;
}
namespace Rtt {
	class InputDeviceDescriptor;
}

#pragma endregion


namespace Rtt
{

/// <summary>
///  <para>Provides access to one Win32 input device's information and functionality.</para>
///  <para>Also provides a Lua interface that can be pushed into the Lua state.</para>
/// </summary>
class WinInputDevice : public PlatformInputDevice
{
	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new interface to an input device.</summary>
		/// <param name="environment">
		///  <para>Reference to the Corona runtime environment this object will be tied to.</para>
		///  <para>Windows input device events will be dispatched as Corona events to this runtime's Lua state.</para>
		/// </param>
		/// <param name="descriptor">Unique descriptor used to identify this input device.</param>
		WinInputDevice(Interop::RuntimeEnvironment& environment, const InputDeviceDescriptor& descriptor);

		/// <summary>Destroys this input device and its resources.</summary>
		virtual ~WinInputDevice();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Binds this Corona device object to the given Win32 device interface.</para>
		///  <para>Used to fetch device information, listen for input events, and apply vibration feedback to.</para>
		///  <para>This method is expected to be called after this object was created.</para>
		/// </summary>
		/// <param name="deviceInterfacePointer">
		///  <para>Pointer to a Windows device interface to bind this object to.</para>
		///  <para>
		///   Set to null to remove the device binding. This will make it stop receiving input data
		///   and provide null device information.
		///  </para>
		/// </param>
		void BindTo(Interop::Input::InputDeviceInterface* deviceInterfacePointer);

		/// <summary>
		///  <para>Remove the binding to the device interface that was given to this object via the BindTo() method.</para>
		///  <para>This prevents this object from fetch device info and stops it from receiving input events.</para>
		/// </summary>
		void Unbind();

		/// <summary>Gets a pointer to the Win32 device interface this object was bound to via the BindTo() method.</summary>
		/// <returns>
		///  <para>Returns a pointer to the device interface this object is bound to.</para>
		///  <para>Returns null if the BindTo() method hasn't been called yet.</para>
		/// </returns>
		Interop::Input::InputDeviceInterface* GetDeviceInterface() const;

		/// <summary>Gets the UTF-8 encoded name of the device as assigned by the manufacturer.</summary>
		/// <remarks>
		///  If the device is a gamepad or joystick, then this name is typically used to determined
		///  which axis inputs and key/button inputs are what on the device.  Especially when determining
		///  which axis inputs make up the right thumbstick on a gamepad since there is no standard.
		/// </remarks>
		/// <returns>
		///  <para>Returns the device's name as assigned to it by the manufacturer.</para>
		///  <para>Returns null if the product name could not be obtained.</para>
		/// </returns>
		virtual const char* GetProductName() override;

		/// <summary>
		///  <para>Gets a UTF-8 encoded descriptive name assigned to the device by the end-user or by the system.</para>
		///  <para>This is typically the product name if the display/alias name could not be obtained.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the device's display name.</para>
		///  <para>Returns null if the display name could not be obtained.</para>
		/// </returns>
		virtual const char* GetDisplayName() override;

		/// <summary>
		///  <para>Gets a unique string ID assigned to the device that can be saved to file.</para>
		///  <para>This string ID will persist after restarting the app or after rebooting the system.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the device's unique string ID.</para>
		///  <para>Returns null if the device does not have a permanent string ID assigned to it.</para>
		/// </returns>
		virtual const char* GetPermanentStringId() override;

		/// <summary>Gets the input device's current connection state such as kConnected, kDisconnected, etc.</summary>
		/// <returns>Returns the input device's current connection state.</returns>
		virtual InputDeviceConnectionState GetConnectionState() override;

		/// <summary>Determines if the input device supports vibrate/rumble functionality.</summary>
		/// <returns>Returns true if the device support vibration feedback. Returns false if not.</returns>
		virtual bool CanVibrate() override;

		/// <summary>
		///  <para>Causes the controller to vibrate/rumble, if supported.</para>
		///  <para>You can determine if the device supports vibration feedback if function CanVibrate() returns true.</para>
		/// </summary>
		virtual void Vibrate() override;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>
		///  <para>Updates this object's configuration with the Win32 "InputDeviceInterface" it is bound to.</para>
		///  <para>
		///   Expected to be called when BindTo() was invoked or when a device interface config change has been detected.
		///  </para>
		/// </summary>
		void UpdateDeviceSettings();

		/// <summary>Called when the input device's configuration or connection state has changed.</summary>
		/// <param name="sender">The input device that has raised this event.</param>
		/// <param name="arguments">Object indicating which aspects of the device has changed.</param>
		void OnDeviceStatusChanged(
				Interop::Input::InputDeviceInterface& sender,
				const Interop::Input::InputDeviceStatusChangedEventArgs& arguments);

		/// <summary>Called when new axis data has been received from the input device.</summary>
		/// <param name="sender">The input device that has raised this event.</param>
		/// <param name="arguments">Provides the new axis data that was received.</param>
		void OnReceivedAxisInput(
				Interop::Input::InputDeviceInterface& sender,
				const Interop::Input::InputAxisDataEventArgs& arguments);

		/// <summary>Called when new key data has been received from the input device.</summary>
		/// <param name="sender">The input device that has raised this event.</param>
		/// <param name="arguments">Provides the new key data that was received.</param>
		void OnReceivedKeyInput(
				Interop::Input::InputDeviceInterface& sender, const Interop::Input::KeyEventArgs& arguments);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Reference to the Corona runtime environment that owns this object.</summary>
		Interop::RuntimeEnvironment& fEnvironment;

		/// <summary>
		///  <para>Pointer to a Win32 device interface this object is bound to.</para>
		///  <para>Used to fetch device information, listen for input events, and apply vibration feedback to.</para>
		/// </summary>
		Interop::Input::InputDeviceInterface* fDeviceInterfacePointer;

		/// <summary>Handler to be invoked when the device's "StatusChanged" event has been raised.</summary>
		Interop::Input::InputDeviceInterface::StatusChangedEvent::MethodHandler<WinInputDevice> fStatusChangedEventHandler;

		/// <summary>Handler to be invoked when the device's "ReceivedAxisInput" event has been raised.</summary>
		Interop::Input::InputDeviceInterface::ReceivedAxisInputEvent::MethodHandler<WinInputDevice> fReceivedAxisInputEventHandler;

		/// <summary>Handler to be invoked when the device's "ReceivedKeyInput" event has been raised.</summary>
		Interop::Input::InputDeviceInterface::ReceivedKeyInputEvent::MethodHandler<WinInputDevice> fReceivedKeyInputEventHandler;

		#pragma endregion
};

} // namespace Rtt
