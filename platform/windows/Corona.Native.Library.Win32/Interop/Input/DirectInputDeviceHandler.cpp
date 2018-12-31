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
#include "DirectInputDeviceHandler.h"
#include "InputAxisInfo.h"
#include "InputAxisSettings.h"
#include "InputDeviceSettings.h"
#include "InputDeviceDriverType.h"
#include "WinString.h"
#include <algorithm>
#include <map>
#include <vector>
#include <Windows.h>

// Include the "InitGuid.h" header before including the DirectInput header.
// This allows us to use its GUIDs without linking to its lib at compile time.
#include <InitGuid.h>
#include <dinput.h>


namespace Interop { namespace Input {

static const size_t kMaxDirectInputBufferSize = 256;

struct DirectInputDeviceObjects
{
	LPDIRECTINPUTDEVICE8W DevicePointer;
	std::vector<WORD> ButtonInstanceIdCollection;
	std::vector<WORD> HatInstanceIdCollection;
	std::vector<WORD> ActuatorInstanceIdCollection;
	std::map<WORD, InputAxisSettings> AxisInstanceIdTable;
};


#pragma region Constructors/Destructors
DirectInputDeviceHandler::DirectInputDeviceHandler()
:	fDeviceContext(InputDeviceDriverType::kDirectInput, fVibrationRequestManager.GetRequestHandler()),
	fDirectInputDevicePointer(nullptr),
	fDirectInputEffectPointer(nullptr),
	fIsDirectInputDataBuffered(false),
	fDirectInputDeviceInstanceGuid(GUID_NULL),
	fWindowHandle(nullptr)
{
}

DirectInputDeviceHandler::~DirectInputDeviceHandler()
{
	Detach();
}

#pragma endregion


#pragma region Public Methods
bool DirectInputDeviceHandler::IsAttached() const
{
	return (fDirectInputDevicePointer != nullptr);
}

void DirectInputDeviceHandler::AttachTo(LPDIRECTINPUTDEVICE8W devicePointer)
{
	HRESULT result;

	// If given null, then detach from the previous device.
	if (!devicePointer)
	{
		Detach();
		return;
	}

	// Do not continue if we are already attached to the given device.
	if (devicePointer == fDirectInputDevicePointer)
	{
		return;
	}

	// Detach from the previous device.
	Detach();

	// First, attempt to fetch information from the given DirectInput device.
	// If we can't, then there is something wrong with the given COM interface pointer and we need to give up.
	DIDEVICEINSTANCEW directInputDeviceInfo{};
	directInputDeviceInfo.dwSize = sizeof(directInputDeviceInfo);
	try
	{
		result = devicePointer->GetDeviceInfo(&directInputDeviceInfo);
		if (FAILED(result))
		{
			return;
		}
	}
	catch (...)
	{
		return;
	}

	// Store the given DirectInput COM interface pointer to the device and increment its reference count.
	// Note: COM interfaces are reference counted. They'll delete themselves when the reference count becomes zero.
	try
	{
		devicePointer->AddRef();
		fDirectInputDevicePointer = devicePointer;
	}
	catch (...) {}

	// Make sure the give DirectInput device has been "unaquired" before re-configuring down below.
	result = fDirectInputDevicePointer->Unacquire();

	// Create a device configuration object that we'll use to copy the DirectInput device's information to.
	// This device configuration will then be copied to this handler's device context.
	InputDeviceSettings deviceSettings;
	deviceSettings.SetDisplayName(directInputDeviceInfo.tszInstanceName);
	deviceSettings.SetProductName(directInputDeviceInfo.tszProductName);

	// Fetch the DirectInput device's unique identifier and use it as the permanent string ID.
	fDirectInputDeviceInstanceGuid = directInputDeviceInfo.guidInstance;
	{
		const int kMaxCharacters = 64;
		wchar_t utf16GuidString[kMaxCharacters];
		utf16GuidString[0] = L'\0';
		::StringFromGUID2(directInputDeviceInfo.guidInstance, utf16GuidString, kMaxCharacters);
		if (utf16GuidString[0] != L'\0')
		{
			WinString permanentStringId;
			permanentStringId.SetUTF16(L"DirectInput:");
			permanentStringId.Append(utf16GuidString);
			deviceSettings.SetPermanentStringId(permanentStringId.GetUTF8());
		}
	}

	// Fetch the device type.
	switch (directInputDeviceInfo.dwDevType & 0xFF)
	{
		case DI8DEVTYPE_FLIGHT:
			deviceSettings.SetType(Rtt::InputDeviceType::kFlightStick);
			break;
		case DI8DEVTYPE_JOYSTICK:
			deviceSettings.SetType(Rtt::InputDeviceType::kJoystick);
			break;
		case DI8DEVTYPE_GAMEPAD:
			deviceSettings.SetType(Rtt::InputDeviceType::kGamepad);
			break;
		case DI8DEVTYPE_SCREENPOINTER:
			if (((directInputDeviceInfo.dwDevType >> 8) & 0xFF) == DI8DEVTYPESCREENPTR_TOUCH)
			{
				deviceSettings.SetType(Rtt::InputDeviceType::kTouchscreen);
			}
			break;
		case DI8DEVTYPE_DRIVING:
			deviceSettings.SetType(Rtt::InputDeviceType::kSteeringWheel);
			break;
	}

	// Fetch the DirectInput device's axes, buttons, and hat switches.
	DirectInputDeviceObjects deviceObjects{};
	deviceObjects.DevicePointer = fDirectInputDevicePointer;
	fDirectInputDevicePointer->EnumObjects(&OnEnumDirectInputDeviceObject, &deviceObjects, DIDFT_ALL);

	// Sort the device's axes, buttons, and hat switches by their "Object Instance IDs".
	// Note: The STL map of axes automatically sorts axes instance IDs upon insert.
	std::sort(deviceObjects.ButtonInstanceIdCollection.begin(), deviceObjects.ButtonInstanceIdCollection.end());
	std::sort(deviceObjects.HatInstanceIdCollection.begin(), deviceObjects.HatInstanceIdCollection.end());
	std::sort(deviceObjects.ActuatorInstanceIdCollection.begin(), deviceObjects.ActuatorInstanceIdCollection.end());

	// Add all of the DirectInput device's buttons to the device configuration as keys.
	// Note: Corona only supports up to 16 generic buttons.
	auto keyCollection = deviceSettings.GetKeys();
	int buttonCount = (int)deviceObjects.ButtonInstanceIdCollection.size();
	if (buttonCount >= 1)	{ keyCollection.Add(Key::kButton1); }
	if (buttonCount >= 2)	{ keyCollection.Add(Key::kButton2); }
	if (buttonCount >= 3)	{ keyCollection.Add(Key::kButton3); }
	if (buttonCount >= 4)	{ keyCollection.Add(Key::kButton4); }
	if (buttonCount >= 5)	{ keyCollection.Add(Key::kButton5); }
	if (buttonCount >= 6)	{ keyCollection.Add(Key::kButton6); }
	if (buttonCount >= 7)	{ keyCollection.Add(Key::kButton7); }
	if (buttonCount >= 8)	{ keyCollection.Add(Key::kButton8); }
	if (buttonCount >= 9)	{ keyCollection.Add(Key::kButton9); }
	if (buttonCount >= 10)	{ keyCollection.Add(Key::kButton10); }
	if (buttonCount >= 11)	{ keyCollection.Add(Key::kButton11); }
	if (buttonCount >= 12)	{ keyCollection.Add(Key::kButton12); }
	if (buttonCount >= 13)	{ keyCollection.Add(Key::kButton13); }
	if (buttonCount >= 14)	{ keyCollection.Add(Key::kButton14); }
	if (buttonCount >= 15)	{ keyCollection.Add(Key::kButton15); }
	if (buttonCount >= 16)	{ keyCollection.Add(Key::kButton16); }

	// If at least 1 hat switch was found, then add virtual d-pad keys to the device configuration.
	// This matches Android's behavior. It also makes DirectInput device d-pads work like XInput devices.
	if (deviceObjects.HatInstanceIdCollection.size() > 0)
	{
		keyCollection.Add(Key::kUp);
		keyCollection.Add(Key::kDown);
		keyCollection.Add(Key::kLeft);
		keyCollection.Add(Key::kRight);
	}

	// Add all of the DirectInput device's axes to the device configuration.
	auto axisCollection = deviceSettings.GetAxes();
	for (auto axisPair : deviceObjects.AxisInstanceIdTable)
	{
		auto axisSettingsPointer = axisCollection.Add();
		if (axisSettingsPointer)
		{
			// Copy the axis configuration.
			*axisSettingsPointer = axisPair.second;

			// If the axis type is "unknown", then assign it an enumerated "generic#" axis type.
			if (axisSettingsPointer->GetType().Equals(Rtt::InputAxisType::kUnknown))
			{
				if (axisCollection.Contains(Rtt::InputAxisType::kGeneric1) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric1);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric2) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric2);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric3) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric3);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric4) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric4);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric5) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric5);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric6) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric6);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric7) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric7);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric8) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric8);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric9) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric9);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric10) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric10);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric11) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric11);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric12) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric12);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric13) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric13);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric14) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric14);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric15) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric15);
				}
				else if (axisCollection.Contains(Rtt::InputAxisType::kGeneric16) == false)
				{
					axisSettingsPointer->SetType(Rtt::InputAxisType::kGeneric16);
				}
			}
		}
	}

	// Add each hat switch input as a virtual "hatX" and "hatX" axis pair to the device configuration.
	// This matches Android's behavior and allows us to treat a hat switch kind of like a joystick.
	for (size_t hatIndex = 0; hatIndex < deviceObjects.HatInstanceIdCollection.size(); hatIndex++)
	{
		auto axisSettingsPointer = axisCollection.Add();
		if (axisSettingsPointer)
		{
			axisSettingsPointer->SetMinValue(INT16_MIN + 1);
			axisSettingsPointer->SetMaxValue(INT16_MAX);
			axisSettingsPointer->SetIsAbsolute(true);
			axisSettingsPointer->SetType(Rtt::InputAxisType::kHatX);
		}
		axisSettingsPointer = axisCollection.Add();
		if (axisSettingsPointer)
		{
			axisSettingsPointer->SetMinValue(INT16_MIN + 1);
			axisSettingsPointer->SetMaxValue(INT16_MAX);
			axisSettingsPointer->SetIsAbsolute(true);
			axisSettingsPointer->SetType(Rtt::InputAxisType::kHatY);
		}
	}

	// Determine if the device can vibrate/rumble.
	deviceSettings.SetCanVibrate(false);
	if (deviceObjects.ActuatorInstanceIdCollection.size() > 0)
	{
		DIDEVCAPS directInputDeviceCapabilities{};
		directInputDeviceCapabilities.dwSize = sizeof(directInputDeviceCapabilities);
		result = fDirectInputDevicePointer->GetCapabilities(&directInputDeviceCapabilities);
		if (directInputDeviceCapabilities.dwFlags & DIDC_FORCEFEEDBACK)
		{
			DIEFFECTINFO effectInfo{};
			effectInfo.dwSize = sizeof(effectInfo);
			result = fDirectInputDevicePointer->GetEffectInfo(&effectInfo, GUID_ConstantForce);
			if (SUCCEEDED(result))
			{
				deviceSettings.SetCanVibrate(true);
			}
		}
	}

	// Set up a data format for the DirectInput device to bind to.
	// Note: DirectInput requires this data format before acquiring the device.
	DirectInputDeviceStateData::FormatSettings dataFormatSettings{};
	dataFormatSettings.ButtonInstanceIdArraySize = deviceObjects.ButtonInstanceIdCollection.size();
	if (dataFormatSettings.ButtonInstanceIdArraySize > 0)
	{
		dataFormatSettings.ButtonInstanceIdArray = &deviceObjects.ButtonInstanceIdCollection.front();
	}
	dataFormatSettings.HatInstanceIdArraySize = deviceObjects.HatInstanceIdCollection.size();
	if (dataFormatSettings.HatInstanceIdArraySize > 0)
	{
		dataFormatSettings.HatInstanceIdArray = &deviceObjects.HatInstanceIdCollection.front();
	}
	std::vector<WORD> axisInstanceIds;
	for (auto&& axisPair : deviceObjects.AxisInstanceIdTable)
	{
		axisInstanceIds.push_back(axisPair.first);
	}
	dataFormatSettings.AxisInstanceIdArraySize = axisInstanceIds.size();
	if (dataFormatSettings.AxisInstanceIdArraySize > 0)
	{
		dataFormatSettings.AxisInstanceIdArray = &axisInstanceIds.front();
	}
	fDirectInputStateData.SetFormatUsing(dataFormatSettings);

	// Only do the following if the device has input data for us to collect.
	// ie: There is no point in acquiring/connecting to the device if we can't collect data from it.
	bool wasDeviceAcquired = false;
	if (fDirectInputStateData.GetFormat())
	{
		// Apply our data format to the DirectInput device.
		result = fDirectInputDevicePointer->SetDataFormat(fDirectInputStateData.GetFormat());

		// Attempt to set up the DirectInput device to buffer/queue input data.
		// If this succeeds, then we can fetch buffered data via the IDirectInputDeveice8::GetDeviceData() method.
		// If this fails, then the device does not support buffering and we must poll for the last recorded input data
		// via the IDirectInputDevice8::GetDeviceState() method.
		{
			DIPROPDWORD wordProperty{};
			wordProperty.diph.dwSize = sizeof(wordProperty);
			wordProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			wordProperty.diph.dwObj = 0;
			wordProperty.diph.dwHow = DIPH_DEVICE;
			wordProperty.dwData = kMaxDirectInputBufferSize;
			result = fDirectInputDevicePointer->SetProperty(DIPROP_BUFFERSIZE, &wordProperty.diph);
			fIsDirectInputDataBuffered = (DI_OK == result);
		}

		// Change some of the device's property to allow vibration to work properly, if supported.
		if (deviceSettings.CanVibrate())
		{
			DIPROPDWORD wordProperty{};
			wordProperty.diph.dwSize = sizeof(wordProperty);
			wordProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			wordProperty.diph.dwObj = 0;
			wordProperty.diph.dwHow = DIPH_DEVICE;
			wordProperty.dwData = DIPROPAUTOCENTER_OFF;
			result = fDirectInputDevicePointer->SetProperty(DIPROP_AUTOCENTER, &wordProperty.diph);
			wordProperty.dwData = 10000;
			result = fDirectInputDevicePointer->SetProperty(DIPROP_FFGAIN, &wordProperty.diph);
		}

		// Enable exclusive access to the DirectInput device. This does the following:
		// 1) Prevents other apps from stealing input data from the device.
		// 2) Allows vibration/rumble/force-feedback to work.
		if (fWindowHandle)
		{
			result = fDirectInputDevicePointer->SetCooperativeLevel(fWindowHandle, DISCL_EXCLUSIVE | DISCL_BACKGROUND);
		}

		// Start collecting input data from the device.
		wasDeviceAcquired = false;
		try
		{
			result = fDirectInputDevicePointer->Acquire();
			if (SUCCEEDED(result))
			{
				wasDeviceAcquired = true;
			}
		}
		catch (...) {}

		// Set up the device for vibration, if supported.
		if (wasDeviceAcquired && deviceSettings.CanVibrate())
		{
			// If the device is currently rumbling, then stop it.
			result = fDirectInputDevicePointer->SendForceFeedbackCommand(DISFFC_STOPALL);

			// Remove the currently loaded force-feedback effects from the device.
			// This is in preparation for the new effect that we're about to load below.
			result = fDirectInputDevicePointer->SendForceFeedbackCommand(DISFFC_RESET);

			// Fetch each actuator's offset within our data format's buffer.
			// Note: Actuators are tied to axes in DirectInput.
			auto actuatorBufferOffset1 = ValueResult<int>::FailedWith((const wchar_t*)nullptr);
			auto actuatorBufferOffset2 = actuatorBufferOffset1;
			if (deviceObjects.ActuatorInstanceIdCollection.size() >= 1)
			{
				auto instanceId = deviceObjects.ActuatorInstanceIdCollection.at(0);
				actuatorBufferOffset1 = fDirectInputStateData.GetAxisBufferOffsetByInstanceId(instanceId);
			}
			if (deviceObjects.ActuatorInstanceIdCollection.size() >= 2)
			{
				auto instanceId = deviceObjects.ActuatorInstanceIdCollection.at(1);
				actuatorBufferOffset2 = fDirectInputStateData.GetAxisBufferOffsetByInstanceId(instanceId);
			}

			// Create the DirectInput force-feedback effect needed to vibrate the device.
			// Note: We need at least 1 actuator to create this effect.
			if (actuatorBufferOffset1.HasSucceeded())
			{
				const size_t kAxisCount = actuatorBufferOffset2.HasSucceeded() ? 2 : 1;
				DWORD axes[] = { actuatorBufferOffset1.GetValue(), actuatorBufferOffset2.GetValue() };
				LONG directions[] = { 0, 0 };
				DICONSTANTFORCE constantForce;
				constantForce.lMagnitude = DI_FFNOMINALMAX;
				DIEFFECT effectSettings{};
				effectSettings.dwSize = sizeof(DIEFFECT);
				effectSettings.dwFlags = DIEFF_OBJECTOFFSETS | DIEFF_CARTESIAN;
				effectSettings.dwDuration = INFINITE;
				effectSettings.dwSamplePeriod = 0;
				effectSettings.dwGain = DI_FFNOMINALMAX;
				effectSettings.dwTriggerButton = DIEB_NOTRIGGER;
				effectSettings.dwTriggerRepeatInterval = 0;
				effectSettings.cAxes = kAxisCount;
				effectSettings.rgdwAxes = axes;
				effectSettings.rglDirection = directions;
				effectSettings.cbTypeSpecificParams = sizeof(constantForce);
				effectSettings.lpvTypeSpecificParams = &constantForce;
				fDirectInputDevicePointer->CreateEffect(
						GUID_ConstantForce, &effectSettings, &fDirectInputEffectPointer, 0);
			}
			if (fDirectInputEffectPointer)
			{
				// Enable the device's actuators so that we can vibrate it later.
				result = fDirectInputDevicePointer->SendForceFeedbackCommand(DISFFC_SETACTUATORSON);
			}
			else
			{
				// We've failed to create the force-feedback effect. This means we can't vibrate the device.
				result = fDirectInputDevicePointer->SendForceFeedbackCommand(DISFFC_SETACTUATORSOFF);
				deviceSettings.SetCanVibrate(false);
			}
		}
	}

	// Update the context's connection and device settings.
	InputDeviceContext::ScopedUpdater scopedUpdater(fDeviceContext);
	if (wasDeviceAcquired)
	{
		fDeviceContext.UpdateWith(Rtt::InputDeviceConnectionState::kConnected);
	}
	else
	{
		fDeviceContext.UpdateWith(Rtt::InputDeviceConnectionState::kDisconnected);
	}
	fDeviceContext.UpdateWith(deviceSettings);
}

void DirectInputDeviceHandler::Detach()
{
	if (fDirectInputEffectPointer)
	{
		try { fDirectInputEffectPointer->Stop(); }
		catch (...) {}
		fDirectInputEffectPointer->Release();
		fDirectInputEffectPointer = nullptr;
	}
	if (fDirectInputDevicePointer)
	{
		fDirectInputDevicePointer->Unacquire();
		fDirectInputDevicePointer->Release();
		fDirectInputDevicePointer = nullptr;
	}
	fDirectInputStateData.ClearFormat();
	fIsDirectInputDataBuffered = false;
	fVibrationRequestManager.Reset();
}

InputDeviceContext& DirectInputDeviceHandler::GetContext()
{
	return fDeviceContext;
}

GUID DirectInputDeviceHandler::GetInstanceGuid() const
{
	if (!fDirectInputDevicePointer)
	{
		return GUID_NULL;
	}
	return fDirectInputDeviceInstanceGuid;
}

GUID DirectInputDeviceHandler::GetLastAttachedInstanceGuid() const
{
	return fDirectInputDeviceInstanceGuid;
}

HWND DirectInputDeviceHandler::GetWindowHandle() const
{
	return fWindowHandle;
}

void DirectInputDeviceHandler::SetWindowHandle(HWND windowHandle)
{
	HRESULT result;

	// Do not continue if the handle hasn't changed.
	if (windowHandle == fWindowHandle)
	{
		return;
	}

	// Unaquire the device if we're currently attached to it.
	// Note: We cannot change the cooperative level while acquired.
	if (fDirectInputDevicePointer)
	{
		result = fDirectInputDevicePointer->Unacquire();
	}

	// Store the given handle.
	fWindowHandle = windowHandle;

	// If currently attached to a device, then update its cooperative level now.
	if (fDirectInputDevicePointer)
	{
		DWORD flags = fWindowHandle ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE;
		flags |= DISCL_BACKGROUND;
		result = fDirectInputDevicePointer->SetCooperativeLevel(fWindowHandle, flags);
		result = fDirectInputDevicePointer->Acquire();
	}
}

void DirectInputDeviceHandler::Poll()
{
	HRESULT result;

	// Do not continue if not attached to a device.
	if (!fDirectInputDevicePointer)
	{
		return;
	}

	// Make sure we're still connected to the device.
	result = fDirectInputDevicePointer->Poll();
	if ((result != DI_OK) && (result != DI_NOEFFECT))
	{
		// We're not connected. Try to re-connect.
		fDirectInputDevicePointer->Acquire();
		result = fDirectInputDevicePointer->Poll();
		if ((result != DI_OK) && (result != DI_NOEFFECT))
		{
			// Device connection lost.
			fDeviceContext.UpdateWith(Rtt::InputDeviceConnectionState::kDisconnected);
			return;
		}
	}

	// At this point, we know the device is connected. Make sure we've flagged it so in the context.
	if (fDeviceContext.GetDeviceInterface().IsConnected() == false)
	{
		// Update the context's device connection state. This also notifies the system via an event.
		fDeviceContext.UpdateWith(Rtt::InputDeviceConnectionState::kConnected);

		// Make sure that an event handler for the above update hasn't detached us from the device.
		if (!fDirectInputDevicePointer)
		{
			return;
		}
	}

	// Fetch input data from the device.
	if (fIsDirectInputDataBuffered)
	{
		PollForBufferedData();
	}
	else
	{
		PollForCurrentData();
	}

	// Update the device's vibration state, if supported.
	if (fDirectInputEffectPointer)
	{
		auto vibrationRequestResult = fVibrationRequestManager.ProcessRequests();
		switch (vibrationRequestResult)
		{
			case VibrationRequestManager::RequestType::kStart:
				result = fDirectInputEffectPointer->Start(INFINITE, DIES_SOLO);
				break;
			case VibrationRequestManager::RequestType::kStop:
				result = fDirectInputEffectPointer->Stop();
				break;
		}
	}
}

#pragma endregion


#pragma region Private Methods
void DirectInputDeviceHandler::PollForBufferedData()
{
	// Validate.
	if (!fDirectInputDevicePointer)
	{
		return;
	}
	if (fDirectInputStateData.GetFormat() == nullptr)
	{
		return;
	}

	// Attempt to fetch all buffered/queued data from the device.
	DIDEVICEOBJECTDATA dataObjectArray[kMaxDirectInputBufferSize];
	DWORD dataObjectCount = (DWORD)kMaxDirectInputBufferSize;
	auto result = fDirectInputDevicePointer->GetDeviceData(
			sizeof(DIDEVICEOBJECTDATA), dataObjectArray, &dataObjectCount, 0);
	if (FAILED(result) || (dataObjectCount <= 0))
	{
		return;
	}

	// Make sure that the returned object count is smaller than our array.
	// Note: This should never happen, but better safe than sorry.
	if (dataObjectCount > kMaxDirectInputBufferSize)
	{
		dataObjectCount = kMaxDirectInputBufferSize;
	}

	// Update the context with all data fetched from the device up above.
	bool isUpdatingContext = false;
	try
	{
		DWORD lastSequenceNumber = 0;
		auto currentModifierKeyStates = ModifierKeyStates::FromKeyboard();
		auto keyCollection = fDeviceContext.GetDeviceInterface().GetDeviceInfo()->GetKeys();
		auto axisCollection = fDeviceContext.GetDeviceInterface().GetDeviceInfo()->GetAxes();
		for (int dataObjectIndex = 0; dataObjectIndex < (int)dataObjectCount; dataObjectIndex++)
		{
			// Get a pointer to the next data object in the buffer.
			auto dataObjectPointer = dataObjectArray + dataObjectIndex;

			// Record all data belonging to the same DirectInput sequence number as a single transaction/snapshot.
			// We can do this with the device context by calling BeginUpdate/EndUpdate, where EndUpdate commits the data.
			if (!isUpdatingContext)
			{
				// Start a new transaction.
				isUpdatingContext = true;
				fDeviceContext.BeginUpdate();
				lastSequenceNumber = dataObjectPointer->dwSequence;
			}
			else if (dataObjectPointer->dwSequence != lastSequenceNumber)
			{
				// End the last transaction.
				// This will raise events for all data given to the context via the UpdateWith() method.
				if (isUpdatingContext)
				{
					isUpdatingContext = false;
					fDeviceContext.EndUpdate();
				}

				// Start a new transaction.
				isUpdatingContext = true;
				fDeviceContext.BeginUpdate();
				lastSequenceNumber = dataObjectPointer->dwSequence;
			}

			// Fetch information about this input object from our data format.
			// This tells us what kind of input it is (axis, button, or hat) and provides its index.
			auto objectInfoResult = fDirectInputStateData.GetInfoFromBufferOffset(dataObjectPointer->dwOfs);
			if (objectInfoResult.HasFailed())
			{
				continue;
			}

			// Record the data.
			if (objectInfoResult.IsButton())
			{
				auto keyPointer = keyCollection.GetByIndex(objectInfoResult.GetIndex());
				if (keyPointer)
				{
					KeyEventArgs::Data keyEventData{};
					keyEventData.NativeCode = keyPointer->GetNativeCodeValue();
					keyEventData.IsDown = dataObjectPointer->dwData ? true : false;
					keyEventData.ModifierFlags = currentModifierKeyStates.GetFlags();
					fDeviceContext.UpdateWith(keyEventData);
				}
			}
			else if (objectInfoResult.IsAxis())
			{
				InputAxisDataEventArgs::Data axisEventData{};
				axisEventData.AxisIndex = objectInfoResult.GetIndex();
				axisEventData.RawValue = (int32_t)dataObjectPointer->dwData;
				fDeviceContext.UpdateWith(axisEventData);
			}
			else if (objectInfoResult.IsHat())
			{
				RecordHatData(objectInfoResult.GetIndex(), dataObjectPointer->dwData, currentModifierKeyStates);
			}
		}
	}
	catch (...) {}

	// End the last transaction.
	// This will raise events for all data given to the context via the UpdateWith() method.
	if (isUpdatingContext)
	{
		fDeviceContext.EndUpdate();
	}
}

void DirectInputDeviceHandler::PollForCurrentData()
{
	// Validate.
	if (!fDirectInputDevicePointer)
	{
		return;
	}

	// Fetch a data buffer needed by the DirectInput device to copy its current data to.
	auto bufferPointer = fDirectInputStateData.GetBufferPointer();
	if (!bufferPointer)
	{
		return;
	}

	// Fetch the DirectInput device's current data.
	auto result = fDirectInputDevicePointer->GetDeviceState(fDirectInputStateData.GetBufferSize(), bufferPointer);
	if (result != DI_OK)
	{
		return;
	}

	// Update the context with the data fetched from the device up above.
	// Note: The ScopedUpdater's destructor will commit the data changes/updates when we exit this method.
	InputDeviceContext::ScopedUpdater scopedUpdater(fDeviceContext);

	// Write the current buttons states to the context.
	auto keyCollection = fDeviceContext.GetDeviceInterface().GetDeviceInfo()->GetKeys();
	auto currentModifierKeyStates = ModifierKeyStates::FromKeyboard();
	const int kDirectInputButtonCount = fDirectInputStateData.GetButtonCount();
	for (int index = 0; index < kDirectInputButtonCount; index++)
	{
		const Key* keyPointer = keyCollection.GetByIndex(index);
		auto fetchDataResult = fDirectInputStateData.GetButtonValueByIndex(index);
		if (fetchDataResult.HasSucceeded() && keyPointer)
		{
			KeyEventArgs::Data keyEventData{};
			keyEventData.NativeCode = keyPointer->GetNativeCodeValue();
			keyEventData.IsDown = fetchDataResult.GetValue() ? true : false;
			keyEventData.ModifierFlags = currentModifierKeyStates.GetFlags();
			fDeviceContext.UpdateWith(keyEventData);
		}
	}

	// Write axis data to the context.
	auto axisCollection = fDeviceContext.GetDeviceInterface().GetDeviceInfo()->GetAxes();
	const int kDirectInputAxisCount = fDirectInputStateData.GetAxisCount();
	for (int index = 0; index < kDirectInputAxisCount; index++)
	{
		auto fetchDataResult = fDirectInputStateData.GetAxisValueByIndex(index);
		if (fetchDataResult.HasSucceeded())
		{
			InputAxisDataEventArgs::Data axisEventData{};
			axisEventData.AxisIndex = index;
			axisEventData.RawValue = fetchDataResult.GetValue();
			fDeviceContext.UpdateWith(axisEventData);
		}
	}

	// Record hat switch data to our virtual keys and virtual axes in the context.
	// This must be done last because all of our virtual key/axis inputs are at the end of their respective collections.
	const int kDirectInputHatCount = fDirectInputStateData.GetHatCount();
	for (int index = 0; index < kDirectInputHatCount; index++)
	{
		auto fetchDataResult = fDirectInputStateData.GetHatValueByIndex(index);
		if (fetchDataResult.HasSucceeded())
		{
			RecordHatData(index, fetchDataResult.GetValue(), currentModifierKeyStates);
		}
	}
}

void DirectInputDeviceHandler::RecordHatData(
	int hatIndex, DWORD hundrethsOfDegrees, const ModifierKeyStates& modifierKeyStates)
{
	// Validate.
	if (hatIndex < 0)
	{
		return;
	}

	// If this is the device's first hat switch, then provide virtual up/down/left/right key events.
	if (0 == hatIndex)
	{
		// Determine which virtual up/down/left/right key is being held down based on the hat switch's data.
		// Note: Hat switch data is reported as hundreths of degrees clockwise where zero is up.
		bool isUpKeyPressed = false;
		bool isDownKeyPressed = false;
		bool isLeftKeyPressed = false;
		bool isRightKeyPressed = false;
		if (MAXDWORD == hundrethsOfDegrees)
		{
		}
		else if (hundrethsOfDegrees < (4500 - 2250))
		{
			isUpKeyPressed = true;
		}
		else if (hundrethsOfDegrees < (9000 - 2250))
		{
			isUpKeyPressed = true;
			isRightKeyPressed = true;
		}
		else if (hundrethsOfDegrees < (13500 - 2250))
		{
			isRightKeyPressed = true;
		}
		else if (hundrethsOfDegrees < (18000 - 2250))
		{
			isRightKeyPressed = true;
			isDownKeyPressed = true;
		}
		else if (hundrethsOfDegrees < (22500 - 2250))
		{
			isDownKeyPressed = true;
		}
		else if (hundrethsOfDegrees < (27000 - 2250))
		{
			isLeftKeyPressed = true;
			isDownKeyPressed = true;
		}
		else if (hundrethsOfDegrees < (31500 - 2250))
		{
			isLeftKeyPressed = true;
		}
		else
		{
			isUpKeyPressed = true;
			isLeftKeyPressed = true;
		}

		// Update the context's pressed/released directional key.
		// The context will only raise a key event if the key states have changed since the last update.
		KeyEventArgs::Data keyEventData{};
		keyEventData.ModifierFlags = modifierKeyStates.GetFlags();
		{
			keyEventData.NativeCode = Key::kUp.GetNativeCodeValue();
			keyEventData.IsDown = isUpKeyPressed;
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kDown.GetNativeCodeValue();
			keyEventData.IsDown = isDownKeyPressed;
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kLeft.GetNativeCodeValue();
			keyEventData.IsDown = isLeftKeyPressed;
			fDeviceContext.UpdateWith(keyEventData);
		}
		{
			keyEventData.NativeCode = Key::kRight.GetNativeCodeValue();
			keyEventData.IsDown = isRightKeyPressed;
			fDeviceContext.UpdateWith(keyEventData);
		}
	}

	// Fetch indexes to the device context's virtual "hatX" and "hatY" axes matching the given DirectInput hat/pov index.
	int hatXIndex = -1;
	int hatYIndex = -1;
	double hatHypotenuse = 0;
	auto axisCollection = fDeviceContext.GetDeviceInterface().GetDeviceInfo()->GetAxes();
	const int kAxisCount = axisCollection.GetCount();
	for (int axisIndex = 0, hatSwitchCount = 0; axisIndex < kAxisCount; axisIndex++)
	{
		auto axisInfoPointer = axisCollection.GetByIndex(axisIndex);
		if (axisInfoPointer && axisInfoPointer->GetType().Equals(Rtt::InputAxisType::kHatX))
		{
			if (hatSwitchCount >= hatIndex)
			{
				axisInfoPointer = axisCollection.GetByIndex(axisIndex + 1);
				if (axisInfoPointer && axisInfoPointer->GetType().Equals(Rtt::InputAxisType::kHatY))
				{
					// Both a hatX and hatY axis pair were found. Store their indexes.
					hatXIndex = axisIndex;
					hatYIndex = axisIndex + 1;

					// For best performance, this assumes that the max and min values are equal lengths.
					hatHypotenuse = (double)axisInfoPointer->GetMaxValue();
				}
				break;
			}
			hatSwitchCount++;
		}
	}
	if ((hatXIndex < 0) || (hatYIndex < 0) || (hatHypotenuse <= 0))
	{
		return;
	}

	// Convert the hat switch angle to joystick x/y like data with a circular restrictor gate.
	// Notes:
	// - Hat switch data is an angle which point north and rotates clockwise.
	// - A hat switch value of MAXDWORD means that hat switch is in the neutral position.
	// - This does not match Android's behavior which treats hatX/hatY like a square restrictor gate joystick.
	int32_t hatXValue = 0;
	int32_t hatYValue = 0;
	if (hundrethsOfDegrees != MAXDWORD)
	{
		// The hat switch is not in the neutral position. Calculate the x/y position.
		double fractionalDegrees = (double)hundrethsOfDegrees / 100.0;
		if (fractionalDegrees > 359.0)
		{
			fractionalDegrees = 359.0;
		}
		double radians = (fractionalDegrees * (double)M_PI) / 180.0;
		hatXValue = (int32_t)std::lround(std::sin(radians) * hatHypotenuse);
		hatYValue = (int32_t)std::lround((std::cos(radians) * hatHypotenuse) * -1.0);
	}

	// Update the context with the new virtual hatX and hatY axis positions calculated above.
	InputAxisDataEventArgs::Data axisEventData;
	{
		axisEventData.AxisIndex = hatXIndex;
		axisEventData.RawValue = hatXValue;
		fDeviceContext.UpdateWith(axisEventData);
	}
	{
		axisEventData.AxisIndex = hatYIndex;
		axisEventData.RawValue = hatYValue;
		fDeviceContext.UpdateWith(axisEventData);
	}
}

#pragma endregion


#pragma region Private Static Functions
BOOL FAR PASCAL DirectInputDeviceHandler::OnEnumDirectInputDeviceObject(
	LPCDIDEVICEOBJECTINSTANCEW objectInstancePointer, LPVOID contextPointer)
{
	// Fetch the context's struct that we'll store this input device object's info to.
	auto deviceObjectsPointer = (DirectInputDeviceObjects*)contextPointer;
	if (!deviceObjectsPointer || !deviceObjectsPointer->DevicePointer)
	{
		return DIENUM_STOP;
	}

	// Identify the device's input item.
	auto instanceId = DIDFT_GETINSTANCE(objectInstancePointer->dwType);
	if (objectInstancePointer->dwType & DIDFT_BUTTON)
	{
		// This is a button.
		// Add its ID to the caller's collection so that we can collect data from it.
		deviceObjectsPointer->ButtonInstanceIdCollection.push_back(instanceId);
	}
	else if (objectInstancePointer->dwType & DIDFT_AXIS)
	{
		// This is an analog axis.
		// We'll need to fetch more information about it before deciding to collect data from it.
		InputAxisSettings axisSettings;
		bool isValidAxis = true;

		// Fetch the axis type.
		if (::IsEqualGUID(objectInstancePointer->guidType, GUID_XAxis))
		{
			axisSettings.SetType(Rtt::InputAxisType::kX);
		}
		else if (::IsEqualGUID(objectInstancePointer->guidType, GUID_YAxis))
		{
			axisSettings.SetType(Rtt::InputAxisType::kY);
		}
		else if (::IsEqualGUID(objectInstancePointer->guidType, GUID_ZAxis))
		{
			axisSettings.SetType(Rtt::InputAxisType::kZ);
		}
		else if (::IsEqualGUID(objectInstancePointer->guidType, GUID_RxAxis))
		{
			axisSettings.SetType(Rtt::InputAxisType::kRotationX);
		}
		else if (::IsEqualGUID(objectInstancePointer->guidType, GUID_RyAxis))
		{
			axisSettings.SetType(Rtt::InputAxisType::kRotationY);
		}
		else if (::IsEqualGUID(objectInstancePointer->guidType, GUID_RzAxis))
		{
			axisSettings.SetType(Rtt::InputAxisType::kRotationZ);
		}
		else if (::IsEqualGUID(objectInstancePointer->guidType, GUID_Slider))
		{
			axisSettings.SetType(Rtt::InputAxisType::kThrottle);
		}
		else
		{
			axisSettings.SetType(Rtt::InputAxisType::kUnknown);
		}

		// Determine if the axis provide absolute or relative values.
		axisSettings.SetIsAbsolute((objectInstancePointer->dwType & DIDFT_ABSAXIS) != 0);

		// Set up the axis' min/max range.
		{
			DIPROPRANGE rangeProperty{};
			rangeProperty.diph.dwSize = sizeof(rangeProperty);
			rangeProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			rangeProperty.diph.dwObj = objectInstancePointer->dwType;
			rangeProperty.diph.dwHow = DIPH_BYID;
			rangeProperty.lMin = INT16_MIN;
			rangeProperty.lMax = INT16_MAX;
			auto result = deviceObjectsPointer->DevicePointer->SetProperty(DIPROP_RANGE, &rangeProperty.diph);
			if (FAILED(result))
			{
				// The device doesn't allow us to change the min/max range. So, use its default range instead.
				result = deviceObjectsPointer->DevicePointer->GetProperty(DIPROP_RANGE, &rangeProperty.diph);
			}
			if (SUCCEEDED(result))
			{
				// Store the axis' min/max range.
				axisSettings.SetMinValue(rangeProperty.lMin);
				axisSettings.SetMaxValue(rangeProperty.lMax);
			}
			else
			{
				// This axis' data is meaningless if we don't know what its range is.
				// So, we have not choice but to ignore this axis.
				isValidAxis = false;
			}
		}

		// Fetch the axis' accuracy/granularity.
		{
			DIPROPDWORD dwordProperty{};
			dwordProperty.diph.dwSize = sizeof(dwordProperty);
			dwordProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			dwordProperty.diph.dwObj = objectInstancePointer->dwType;
			dwordProperty.diph.dwHow = DIPH_BYID;
			auto result = deviceObjectsPointer->DevicePointer->GetProperty(DIPROP_GRANULARITY, &dwordProperty.diph);
			if (SUCCEEDED(result) && (dwordProperty.dwData >= 1))
			{
				// Convert granularity to +/- accuracy, which is half the granularity range.
				axisSettings.SetAccuracy((float)dwordProperty.dwData / 2.0f);
			}
			else
			{
				// Use a +/- accuracy of zero if granularity is unknown or invalid.
				axisSettings.SetAccuracy(0);
			}
		}

		// Do the following if we can collect data from the axis.
		if (isValidAxis)
		{
			// Add the axis' ID to the caller's collection so that we can collect data from it.
			deviceObjectsPointer->AxisInstanceIdTable.insert(
					std::pair<WORD, InputAxisSettings>(instanceId, axisSettings));

			// If this axis supports force feedback, then add it to the caller's actuator collection.
			// We'll need it for vibration/rumble support.
			if (objectInstancePointer->dwType & DIDFT_FFACTUATOR)
			{
				deviceObjectsPointer->ActuatorInstanceIdCollection.push_back(instanceId);
			}
		}
	}
	else if (objectInstancePointer->dwType & DIDFT_POV)
	{
		// This is a hat switch or d-pad.
		// Add its ID to the caller's collection so that we can collect data from it.
		deviceObjectsPointer->HatInstanceIdCollection.push_back(instanceId);
	}

	// This return value requests the caller to send us the next object belonging to a device.
	return DIENUM_CONTINUE;
}

#pragma endregion

} }	// namespace Interop::Input
