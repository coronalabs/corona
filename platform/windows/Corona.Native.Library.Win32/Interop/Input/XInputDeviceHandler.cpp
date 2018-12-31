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

#include "stdafx.h"
#include "XInputDeviceHandler.h"
#include "InputAxisDataEventArgs.h"
#include "InputAxisSettings.h"
#include "InputDeviceDriverType.h"
#include "Key.h"
#include "KeyEventArgs.h"
#include "XInputInterface.h"
#include <stdint.h>
#include <sstream>
#include <string>
#include <Xinput.h>


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
XInputDeviceHandler::XInputDeviceHandler()
:	fDeviceContext(InputDeviceDriverType::kXInput, fVibrationRequestManager.GetRequestHandler()),
	fLastAttachedUserIndex(-1),
	fHasReceivedData(false),
	fLastReceivedPacketNumber(0)
{
}

XInputDeviceHandler::~XInputDeviceHandler()
{
	Detach();
}

#pragma endregion


#pragma region Public Methods
bool XInputDeviceHandler::IsAttached() const
{
	const InputDeviceInterface& deviceInterface = fDeviceContext.GetDeviceInterface();
	return deviceInterface.IsConnected() && (deviceInterface.GetDeviceInfo()->GetPlayerNumber() > 0);
}

void XInputDeviceHandler::AttachTo(DWORD userIndex, const XINPUT_CAPABILITIES& deviceCapabilities)
{
	// Do not continue if the XInput library is unavailable.
	auto xinputPointer = XInputInterface::GetInstance();
	if (!xinputPointer)
	{
		return;
	}

	// Copy the given device capabilities for future reference.
	memcpy(&fLastAttachedXInputDeviceCapabilities, &deviceCapabilities, sizeof(XINPUT_CAPABILITIES));

	// Reset member variables used to track input packet numbers.
	// This is needed to prevent recording duplicate data.
	fHasReceivedData = false;
	fLastReceivedPacketNumber = 0;

	// Create a device configuration object that we'll use to copy the XInput device's information to.
	// This device configuration will then be copied to this handler's device context.
	InputDeviceSettings deviceSettings;

	// Store the given XInput user index.
	// Make it one based in our device configuration. (XInput's user index is zero based.)
	fLastAttachedUserIndex = (int)userIndex;
	deviceSettings.SetPlayerNumber(userIndex + 1);

	// Make up a nice controller name for this device based on the player number.
	// Note: Unlike DirectInput and RawInput, XInput does not provide names for devices.
	UpdateDeviceNameUsing(userIndex, deviceCapabilities, deviceSettings);

	// Determine if the device can vibrate/rumble.
	if ((deviceCapabilities.Vibration.wLeftMotorSpeed > 0) || (deviceCapabilities.Vibration.wRightMotorSpeed > 0))
	{
		deviceSettings.SetCanVibrate(true);
	}

	// If the device is currently rumbling, then stop it.
	if (deviceSettings.CanVibrate())
	{
		XINPUT_VIBRATION vibrationData{};
		xinputPointer->SetState(userIndex, &vibrationData);
	}

	// Determine the device type.
	switch (deviceCapabilities.SubType)
	{
		case XINPUT_DEVSUBTYPE_ARCADE_PAD:
		case XINPUT_DEVSUBTYPE_ARCADE_STICK:
		case XINPUT_DEVSUBTYPE_GAMEPAD:
			deviceSettings.SetType(Rtt::InputDeviceType::kGamepad);
			break;
		case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
			deviceSettings.SetType(Rtt::InputDeviceType::kFlightStick);
			break;
		case XINPUT_DEVSUBTYPE_WHEEL:
			deviceSettings.SetType(Rtt::InputDeviceType::kSteeringWheel);
			break;
		case XINPUT_DEVSUBTYPE_DANCE_PAD:
			deviceSettings.SetType(Rtt::InputDeviceType::kDancePad);
			break;
		case XINPUT_DEVSUBTYPE_GUITAR:
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
		case XINPUT_DEVSUBTYPE_GUITAR_BASS:
			deviceSettings.SetType(Rtt::InputDeviceType::kGuitar);
			break;
		case XINPUT_DEVSUBTYPE_DRUM_KIT:
			deviceSettings.SetType(Rtt::InputDeviceType::kDrumSet);
			break;
		default:
			deviceSettings.SetType(Rtt::InputDeviceType::kUnknown);
			break;
	}

	// Create a lambda expression for creating an axis of the given type and adding it to the given axis collection.
	// This is to be used down below because all XInput axes have mostly the same configuration.
	auto addAxisLambda = [](
		InputDeviceSettings::AxisCollectionAdapter& collection, int& axisIndexMapField, const Rtt::InputAxisType& type)
	{
		auto axisSettingsPointer = collection.Add();
		if (axisSettingsPointer)
		{
			axisSettingsPointer->SetType(type);
			if ((type == Rtt::InputAxisType::kLeftTrigger) || (type == Rtt::InputAxisType::kRightTrigger))
			{
				axisSettingsPointer->SetMinValue(0);
				axisSettingsPointer->SetMaxValue(255);
			}
			else
			{
				axisSettingsPointer->SetMinValue(INT16_MIN);
				axisSettingsPointer->SetMaxValue(INT16_MAX);
			}
			axisSettingsPointer->SetIsAbsolute(true);
			axisIndexMapField = collection.GetCount() - 1;
		}
		return axisSettingsPointer;
	};

	// Determine what buttons and axes the device has.
	fAxisIndexMap.Reset();
	auto keyCollection = deviceSettings.GetKeys();
	auto axisCollection = deviceSettings.GetAxes();
	{
		// First, add buttons that all XInput devices have.
		keyCollection.Add(Key::kUp);
		keyCollection.Add(Key::kDown);
		keyCollection.Add(Key::kLeft);
		keyCollection.Add(Key::kRight);
		keyCollection.Add(Key::kButtonA);
		keyCollection.Add(Key::kButtonB);
		keyCollection.Add(Key::kButtonX);
		keyCollection.Add(Key::kButtonY);
		keyCollection.Add(Key::kButtonStart);
		keyCollection.Add(Key::kButtonSelect);
	}
	switch (deviceCapabilities.SubType)
	{
		case XINPUT_DEVSUBTYPE_WHEEL:
		{
			// Set up for a steering wheel.
			keyCollection.Add(Key::kLeftJoystickButton);
			keyCollection.Add(Key::kRightJoystickButton);
			keyCollection.Add(Key::kLeftShoulderButton1);
			keyCollection.Add(Key::kRightShoulderButton1);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbLX, Rtt::InputAxisType::kWheel);
			addAxisLambda(axisCollection, fAxisIndexMap.RightTrigger, Rtt::InputAxisType::kGas);
			addAxisLambda(axisCollection, fAxisIndexMap.LeftTrigger, Rtt::InputAxisType::kBrake);
			break;
		}
		case XINPUT_DEVSUBTYPE_ARCADE_PAD:
		case XINPUT_DEVSUBTYPE_ARCADE_STICK:
		{
			// Set up for a fighting game controller.
			// Its analog triggers are really digital buttons.
			keyCollection.Add(Key::kLeftJoystickButton);
			keyCollection.Add(Key::kRightJoystickButton);
			keyCollection.Add(Key::kLeftShoulderButton1);
			keyCollection.Add(Key::kRightShoulderButton1);
			auto axisSettingsPointer = addAxisLambda(
					axisCollection, fAxisIndexMap.LeftTrigger, Rtt::InputAxisType::kLeftTrigger);
			if (axisSettingsPointer)
			{
				axisSettingsPointer->SetAccuracy((float)(axisSettingsPointer->GetMaxValue() / 2));
			}
			axisSettingsPointer = addAxisLambda(
					axisCollection, fAxisIndexMap.RightTrigger, Rtt::InputAxisType::kRightTrigger);
			if (axisSettingsPointer)
			{
				axisSettingsPointer->SetAccuracy((float)(axisSettingsPointer->GetMaxValue() / 2));
			}
			break;
		}
		case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
		{
			// Set up for a flight stick.
			keyCollection.Add(Key::kLeftJoystickButton);
			keyCollection.Add(Key::kRightJoystickButton);
			keyCollection.Add(Key::kLeftShoulderButton1);
			keyCollection.Add(Key::kRightShoulderButton1);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbLX, Rtt::InputAxisType::kX);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbLY, Rtt::InputAxisType::kY);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbRX, Rtt::InputAxisType::kHatX);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbRY, Rtt::InputAxisType::kHatY);
			addAxisLambda(axisCollection, fAxisIndexMap.LeftTrigger, Rtt::InputAxisType::kRudder);
			addAxisLambda(axisCollection, fAxisIndexMap.RightTrigger, Rtt::InputAxisType::kThrottle);
			break;
		}
		case XINPUT_DEVSUBTYPE_DANCE_PAD:
		{
			// A dance pad only has the basic buttons all controller have.
			break;
		}
		case XINPUT_DEVSUBTYPE_GUITAR:
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
		case XINPUT_DEVSUBTYPE_GUITAR_BASS:
		{
			// Set up for a "Guitar Hero" style controller.
			keyCollection.Add(Key::kLeftJoystickButton);
			keyCollection.Add(Key::kRightJoystickButton);
			keyCollection.Add(Key::kLeftShoulderButton1);
			keyCollection.Add(Key::kRightShoulderButton1);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbRY, Rtt::InputAxisType::kOrientation);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbRX, Rtt::InputAxisType::kWhammyBar);
			addAxisLambda(axisCollection, fAxisIndexMap.LeftTrigger, Rtt::InputAxisType::kLeftTrigger);
			addAxisLambda(axisCollection, fAxisIndexMap.RightTrigger, Rtt::InputAxisType::kRightTrigger);
			break;
		}
		case XINPUT_DEVSUBTYPE_DRUM_KIT:
		{
			// Set up for a "Rock Band" style drum.
			// This device does not have any analog inputs.
			keyCollection.Add(Key::kLeftJoystickButton);
			keyCollection.Add(Key::kRightJoystickButton);
			keyCollection.Add(Key::kLeftShoulderButton1);
			keyCollection.Add(Key::kRightShoulderButton1);
			break;
		}
		case XINPUT_DEVSUBTYPE_GAMEPAD:
		case XINPUT_DEVSUBTYPE_UNKNOWN:
		default:
		{
			// Set up as a normal Xbox controller.
			keyCollection.Add(Key::kLeftJoystickButton);
			keyCollection.Add(Key::kRightJoystickButton);
			keyCollection.Add(Key::kLeftShoulderButton1);
			keyCollection.Add(Key::kRightShoulderButton1);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbLX, Rtt::InputAxisType::kLeftX);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbLY, Rtt::InputAxisType::kLeftY);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbRX, Rtt::InputAxisType::kRightX);
			addAxisLambda(axisCollection, fAxisIndexMap.ThumbRY, Rtt::InputAxisType::kRightY);
			addAxisLambda(axisCollection, fAxisIndexMap.LeftTrigger, Rtt::InputAxisType::kLeftTrigger);
			addAxisLambda(axisCollection, fAxisIndexMap.RightTrigger, Rtt::InputAxisType::kRightTrigger);
			break;
		}
	}
	
	// Update the context's device configuration and connection state.
	// Note: Assume we're connected if the caller was able to fetch the XInput device's capabilites.
	InputDeviceContext::ScopedUpdater scopedUpdater(fDeviceContext);
	fDeviceContext.UpdateWith(Rtt::InputDeviceConnectionState::kConnected);
	fDeviceContext.UpdateWith(deviceSettings);
}

void XInputDeviceHandler::Detach()
{
	// Clear the current vibration state.
	fVibrationRequestManager.Reset();

	// Fetch a mutable copy of the context's current device info.
	InputDeviceSettings deviceSettings;
	fDeviceContext.GetDeviceInterface().GetDeviceInfo()->CopyTo(deviceSettings);

	// Change the device's assigned player number to zero.
	// This flags the device as disconnected/detached.
	deviceSettings.SetPlayerNumber(0);

	// Update the device's display name and product name now that the player number has been cleared.
	deviceSettings.SetDisplayName(deviceSettings.GetProductNameAsUtf16());
	UpdateDeviceNameUsing(-1, fLastAttachedXInputDeviceCapabilities, deviceSettings);

	// Update the context's device settings with the above changes.
	InputDeviceContext::ScopedUpdater scopedUpdater(fDeviceContext);
	fDeviceContext.UpdateWith(deviceSettings);
}

InputDeviceContext& XInputDeviceHandler::GetContext()
{
	return fDeviceContext;
}

ValueResult<DWORD> XInputDeviceHandler::GetLastAttachedUserIndex() const
{
	if (fLastAttachedUserIndex < 0)
	{
		return ValueResult<DWORD>::FailedWith(
				L"This device handler has never been successfully attached to a user index before.");
	}
	return ValueResult<DWORD>::SucceededWith((DWORD)fLastAttachedUserIndex);
}

bool XInputDeviceHandler::IsLastAttachedEqualTo(const XINPUT_CAPABILITIES& deviceCapabilities) const
{
	// Return false if this handler has never attached to a device successfully.
	if (fLastAttachedUserIndex < 0)
	{
		return false;
	}

	// Compare device capabilities.
	// Note: We cannot use memcmp() for this because the byte padding between struct fields might contain garbage.
	if (fLastAttachedXInputDeviceCapabilities.Flags != deviceCapabilities.Flags)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Type != deviceCapabilities.Type)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.SubType != deviceCapabilities.SubType)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Vibration.wLeftMotorSpeed != deviceCapabilities.Vibration.wLeftMotorSpeed)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Vibration.wRightMotorSpeed != deviceCapabilities.Vibration.wRightMotorSpeed)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Gamepad.wButtons != deviceCapabilities.Gamepad.wButtons)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Gamepad.bLeftTrigger != deviceCapabilities.Gamepad.bLeftTrigger)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Gamepad.bRightTrigger != deviceCapabilities.Gamepad.bRightTrigger)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Gamepad.sThumbLX != deviceCapabilities.Gamepad.sThumbLX)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Gamepad.sThumbLY != deviceCapabilities.Gamepad.sThumbLY)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Gamepad.sThumbRX != deviceCapabilities.Gamepad.sThumbRX)
	{
		return false;
	}
	if (fLastAttachedXInputDeviceCapabilities.Gamepad.sThumbRY != deviceCapabilities.Gamepad.sThumbRY)
	{
		return false;
	}

	// The XInput device capabilities match.
	return true;
}

void XInputDeviceHandler::Poll()
{
	// Do not continue if this handler is not currently attached to an XInput user index.
	if (IsAttached() == false)
	{
		return;
	}

	// Fetch an interface to the XInput library.
	auto xinputPointer = XInputInterface::GetInstance();
	if (!xinputPointer)
	{
		fDeviceContext.UpdateWith(Rtt::InputDeviceConnectionState::kDisconnected);
		return;
	}

	// Fetch input data from the device.
	XINPUT_STATE inputState{};
	auto result = xinputPointer->GetState((DWORD)fLastAttachedUserIndex, &inputState);

	// Do not continue if the device connection was lost.
	if (result != ERROR_SUCCESS)
	{
		fDeviceContext.UpdateWith(Rtt::InputDeviceConnectionState::kDisconnected);
		return;
	}

	// Determine if new input data has been received from the device. (Indicated by an updated packet number.)
	bool hasReceivedNewData = false;
	if (!fHasReceivedData || (fLastReceivedPacketNumber != inputState.dwPacketNumber))
	{
		hasReceivedNewData = true;
		fHasReceivedData = true;
		fLastReceivedPacketNumber = inputState.dwPacketNumber;
	}

	// Update the context with input data, if a new packet has been received.
	// Note: This is an optimization. We don't want to update the context with duplicate data from the last poll.
	if (hasReceivedNewData)
	{
		InputDeviceContext::ScopedUpdater scopedUpdater(fDeviceContext);

		KeyEventArgs::Data keyEventData;
		keyEventData.ModifierFlags = ModifierKeyStates::FromKeyboard().GetFlags();
		{
			keyEventData.NativeCode = Key::kUp.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kDown.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kLeft.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kRight.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kButtonA.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kButtonB.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kButtonX.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kButtonY.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kButtonStart.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kButtonSelect.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kLeftShoulderButton1.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kRightShoulderButton1.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kLeftJoystickButton.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kRightJoystickButton.GetNativeCodeValue();
			keyEventData.IsDown = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0);
			fDeviceContext.UpdateWith(keyEventData);
		}

		InputAxisDataEventArgs::Data axisEventData;
		if (fAxisIndexMap.ThumbLX >= 0)
		{
			axisEventData.AxisIndex = (unsigned int)fAxisIndexMap.ThumbLX;
			axisEventData.RawValue = inputState.Gamepad.sThumbLX;
			fDeviceContext.UpdateWith(axisEventData);
		}
		if (fAxisIndexMap.ThumbLY >= 0)
		{
			axisEventData.AxisIndex = (unsigned int)fAxisIndexMap.ThumbLY;
			axisEventData.RawValue = inputState.Gamepad.sThumbLY;
			{
				// Y-axis data is inverted compared to HID/DirectInput joystick axis data.
				// Flip it so that up is negative and down is positive for consistency.
				if (INT32_MIN == axisEventData.RawValue)
				{
					axisEventData.RawValue = INT32_MAX;
				}
				else if (INT32_MAX == axisEventData.RawValue)
				{
					axisEventData.RawValue = INT32_MIN;
				}
				else
				{
					axisEventData.RawValue *= -1;
				}
			}
			fDeviceContext.UpdateWith(axisEventData);
		}
		if (fAxisIndexMap.ThumbRX >= 0)
		{
			axisEventData.AxisIndex = (unsigned int)fAxisIndexMap.ThumbRX;
			axisEventData.RawValue = inputState.Gamepad.sThumbRX;
			fDeviceContext.UpdateWith(axisEventData);
		}
		if (fAxisIndexMap.ThumbRY >= 0)
		{
			axisEventData.AxisIndex = (unsigned int)fAxisIndexMap.ThumbRY;
			axisEventData.RawValue = inputState.Gamepad.sThumbRY;
			{
				// Y-axis data is inverted compared to HID/DirectInput joystick axis data.
				// Flip it so that up is negative and down is positive for consistency.
				if (INT32_MIN == axisEventData.RawValue)
				{
					axisEventData.RawValue = INT32_MAX;
				}
				else if (INT32_MAX == axisEventData.RawValue)
				{
					axisEventData.RawValue = INT32_MIN;
				}
				else
				{
					axisEventData.RawValue *= -1;
				}
			}
			fDeviceContext.UpdateWith(axisEventData);
		}
		if (fAxisIndexMap.LeftTrigger >= 0)
		{
			axisEventData.AxisIndex = (unsigned int)fAxisIndexMap.LeftTrigger;
			axisEventData.RawValue = inputState.Gamepad.bLeftTrigger;
			fDeviceContext.UpdateWith(axisEventData);
		}
		if (fAxisIndexMap.RightTrigger >= 0)
		{
			axisEventData.AxisIndex = (unsigned int)fAxisIndexMap.RightTrigger;
			axisEventData.RawValue = inputState.Gamepad.bRightTrigger;
			fDeviceContext.UpdateWith(axisEventData);
		}
	}

	// Update the device's vibration state, if supported.
	if (fDeviceContext.GetDeviceInterface().GetDeviceInfo()->CanVibrate())
	{
		auto vibrationRequestResult = fVibrationRequestManager.ProcessRequests();
		if (fVibrationRequestManager.WasVibrationRequested())
		{
			// Vibrate the device.
			// Note: We need to spam this request to the device because it doesn't always work after initially
			//       connecting the device and it won't return an error if it failed to set the vibration state.
			XINPUT_VIBRATION vibrationData;
			vibrationData.wLeftMotorSpeed = MAXWORD;
			vibrationData.wRightMotorSpeed = MAXWORD;
			xinputPointer->SetState(fLastAttachedUserIndex, &vibrationData);
		}
		else if (VibrationRequestManager::RequestType::kStop == vibrationRequestResult)
		{
			// Stop vibrating the device.
			XINPUT_VIBRATION vibrationData;
			vibrationData.wLeftMotorSpeed = 0;
			vibrationData.wRightMotorSpeed = 0;
			xinputPointer->SetState(fLastAttachedUserIndex, &vibrationData);
		}
	}
}

#pragma endregion


#pragma region Private Static Functions
void XInputDeviceHandler::UpdateDeviceNameUsing(
	int userIndex, const XINPUT_CAPABILITIES& deviceCapabilities, InputDeviceSettings& deviceSettings)
{
	// Generate a nice product name for the given XInput device info.
	std::wstringstream utf16Stream;
	utf16Stream << L"XInput ";
	switch (deviceCapabilities.SubType)
	{
		case XINPUT_DEVSUBTYPE_GAMEPAD:
			utf16Stream << L"Gamepad";
			break;
		case XINPUT_DEVSUBTYPE_WHEEL:
			utf16Stream << L"Steering Wheel";
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_STICK:
			utf16Stream << L"Arcade Stick";
			break;
		case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
			utf16Stream << L"Flight Stick";
			break;
		case XINPUT_DEVSUBTYPE_DANCE_PAD:
			utf16Stream << L"Dance Pad";
			break;
		case XINPUT_DEVSUBTYPE_GUITAR:
			utf16Stream << L"Guitar";
			break;
		case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
			utf16Stream << L"Alternate Guitar";
			break;
		case XINPUT_DEVSUBTYPE_GUITAR_BASS:
			utf16Stream << L"Bass Guitar";
			break;
		case XINPUT_DEVSUBTYPE_DRUM_KIT:
			utf16Stream << L"Drum Set";
			break;
		case XINPUT_DEVSUBTYPE_ARCADE_PAD:
			utf16Stream << L"Arcade Pad";
			break;
		default:
			utf16Stream << L"Controller";
			break;
	}
	auto utf16ProductName = utf16Stream.str();

	// Generate a display name for the given XInput device info.
	if (userIndex >= 0)
	{
		utf16Stream << L" (Player " << (userIndex + 1) << L")";
	}
	auto utf16DisplayName = utf16Stream.str();

	// Update the given device settings with the above generated names.
	deviceSettings.SetProductName(utf16ProductName.c_str());
	deviceSettings.SetDisplayName(utf16DisplayName.c_str());
}

#pragma endregion


#pragma region XInputAxisIndexMap Struct Methods
XInputDeviceHandler::XInputAxisIndexMap::XInputAxisIndexMap()
{
	Reset();
}

void XInputDeviceHandler::XInputAxisIndexMap::Reset()
{
	this->ThumbLX = -1;
	this->ThumbLY = -1;
	this->ThumbRX = -1;
	this->ThumbRY = -1;
	this->LeftTrigger = -1;
	this->RightTrigger = -1;
}

#pragma endregion

} }	// namespace Interop::Input
