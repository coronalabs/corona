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
#include "DirectInputDeviceStateData.h"
#include "InputDeviceContext.h"
#include "MInputDeviceHandler.h"
#include "VibrationRequestManager.h"
#include <Windows.h>


#pragma region Forward Declarations
namespace Interop { namespace Input {
	struct ModifiedKeyStates;
} }
extern "C" {
	struct DIDEVICEOBJECTINSTANCEW;
	struct IDirectInputDevice8W;
	struct IDirectInputEffect;
	typedef const DIDEVICEOBJECTINSTANCEW *LPCDIDEVICEOBJECTINSTANCEW;
	typedef struct IDirectInputDevice8W *LPDIRECTINPUTDEVICE8W;
	typedef struct IDirectInputEffect *LPDIRECTINPUTEFFECT;
}

#pragma endregion


namespace Interop { namespace Input {

/// <summary>
///  <para>Handler used to access, control, and read data from one DirectInput device.</para>
///  <para>Instances of this class are only expected to be created and used by the "InputDeviceMonitor" class.</para>
/// </summary>
class DirectInputDeviceHandler : public MInputDeviceHandler
{
	Rtt_CLASS_NO_COPIES(DirectInputDeviceHandler)

	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new DirectInput device handler.</summary>
		DirectInputDeviceHandler();

		/// <summary>Detaches this handler from its device (if attached) and then destroys this handler.</summary>
		virtual ~DirectInputDeviceHandler();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Determines if this handler is currently attached/bound to a connected input device.</para>
		///  <para>This object's Poll() method will only collect input data if attached to a device.</para>
		/// </summary>
		/// <returns>Returns true if currently connected to a device. Returns false if not.</returns>
		virtual bool IsAttached() const override;

		/// <summary>
		///  <para>Attaches this handler to the give DirectInput device</para>
		///  <para>This allows this handler to control and collect input data from the device.</para>
		/// </summary>
		/// <param name="devicePointer">COM interface pointer to the DirectInput device.</param>
		void AttachTo(LPDIRECTINPUTDEVICE8W devicePointer);

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

		/// <summary>Gets the currently attached DirectInput device's instance GUID.</summary>
		/// <returns>
		///  <para>Returns the instance GUID of the DirectInput device this handler is currently attached to.</para>
		///  <para>Returns GUID_NULL if this handler is not currently attached to a device.</para>
		/// </returns>
		GUID GetInstanceGuid() const;

		/// <summary>
		///  Gets the instance GUID of the DirectInput device this handler is currently or was last attached to.
		/// </summary>
		/// <returns>
		///  <para>Returns the instance GUID of the DirectInput device this handler is/was attached to.</para>
		///  <para>Returns GUID_NULL if this handler has never been attached to a device.</para>
		/// </returns>
		GUID GetLastAttachedInstanceGuid() const;

		/// <summary>
		///  <para>Gets the Win32 window handle given to this object via the SetWindowHandle() method.</para>
		///  <para>
		///   This handle is used to put the attached DirectInput device into "exclusive" mode, which is
		///   required for vibration/rumble support.
		///  </para>
		/// </summary>
		/// <returns>
		///  <praa>Returns the window handle assigned to this handler.</para>
		///  <para>Returns null if a window handle is not assigned.</para>
		/// </returns>
		HWND GetWindowHandle() const;

		/// <summary>
		///  <para>
		///   Sets a Win32 handle to a window for this handler to use to put its DirectInput device into "exclusive" mode.
		///  </para>
		///  <para>This is required in order to use a device's vibration/rumble feature.</para>
		///  <para>This handle should be assigned before attaching the device to this handler.</para>
		/// </summary>
		/// <param name="windowHandle">Handle to a Win32 window. Can be null.</param>
		void SetWindowHandle(HWND windowHandle);

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
		#pragma region Private Methods
		/// <summary>Polls the attached DirectInput device for buffered/queued input.</summary>
		void PollForBufferedData();

		/// <summary>Polls the attached DirectInput device for its last recorded input data.</summary>
		void PollForCurrentData();

		/// <summary>
		///  <para>Records the indexed DirectInput hat switch data to this handler's device context.</para>
		///  <para>
		///   Will convert the given hat data to virtual up/down/left/right key events and virtual hatX/hatY axis events.
		///  </para>
		/// </summary>
		/// <param name="hatIndex">
		///  Zero based index to the hat switch input object in member variable's "fDirectInputStateData" format.
		/// </param>
		/// <param name="hundrethsOfDegrees">
		///  The DirectInput hat switch (aka: POV) value in 100ths of degrees, where zero is north/up
		///  and the values rotate clockwise in the positive direction.
		/// </param>
		/// <param name="modifierKeyStates">
		///  Provides the keyboard's current modifier (shift/alt/control) key states to be delivered with
		///  the virtual up/down/left/right key events.
		/// </param>
		void RecordHatData(int hatIndex, DWORD hundrethsOfDegrees, const ModifierKeyStates& modifierKeyStates);

		#pragma endregion


		#pragma region Private Static Functions
		/// <summary>Called by DirectInput for every input object (axis, button, hat switch) found on a device.</summary>
		/// <param name="objectInstancePointer">Provides information about one input object on the device.</param>
		/// <param name="contextPointer">
		///  Pointer to optional user data defined by the caller of the IDirectInputDevice8::EnumObjects() method.
		/// </param>
		static BOOL FAR PASCAL OnEnumDirectInputDeviceObject(
				LPCDIDEVICEOBJECTINSTANCEW objectInstancePointer, LPVOID contextPointer);

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
		///  <para>Stores the data format info and byte buffer required by an attached DirectInput device.</para>
		///  <para>This format must be defined and given to a DirectInput device in order to "acquire" the device.</para>
		/// </summary>
		DirectInputDeviceStateData fDirectInputStateData;

		/// <summary>COM interface pointer to the DirectInput device this handler is attached to.</summary>
		LPDIRECTINPUTDEVICE8W fDirectInputDevicePointer;

		/// <summary>
		///  <para>Pointer to a DirectInput force feedback effect needed to vibrate/rumble a DirectInput device.</para>
		///  <para>Set null if the device does not support vibration or if not attached to a device.</para>
		/// </summary>
		LPDIRECTINPUTEFFECT fDirectInputEffectPointer;

		/// <summary>
		///  <para>Set true if the device supports buffered/queued input via the PollForBufferedData() method.</para>
		///  <para>Set false if only the last recorded data can be fetched via the PollForCurrentData() method.</para>
		/// </summary>
		bool fIsDirectInputDataBuffered;

		/// <summary>
		///  <para>The instance GUID of the DirectInput device this handler is currently or last attached to.</para>
		///  <para>Set to GUID_NULL if this handler has never been attached to a DirectInput device.</para>
		/// </summary>
		GUID fDirectInputDeviceInstanceGuid;

		/// <summary>Handle to a Win32 window used to put a DirectInput device into "exclusive" mode. Can be null.</summary>
		HWND fWindowHandle;

		#pragma endregion
};

} }	// namespace Interop::Input
