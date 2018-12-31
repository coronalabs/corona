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
#include "Rtt_WinInputDevice.h"
#include "Input\Rtt_InputDeviceDescriptor.h"
#include "Input\Rtt_PlatformInputAxis.h"
#include "Interop\Input\InputAxisInfo.h"
#include "Interop\Input\InputDeviceInfo.h"
#include "Interop\Input\InputDeviceInterface.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"


namespace Rtt {

#pragma region Constructors/Destructors
WinInputDevice::WinInputDevice(Interop::RuntimeEnvironment& environment, const InputDeviceDescriptor& descriptor)
:	PlatformInputDevice(descriptor),
	fEnvironment(environment),
	fDeviceInterfacePointer(nullptr),
	fStatusChangedEventHandler(this, &WinInputDevice::OnDeviceStatusChanged),
	fReceivedAxisInputEventHandler(this, &WinInputDevice::OnReceivedAxisInput),
	fReceivedKeyInputEventHandler(this, &WinInputDevice::OnReceivedKeyInput)
{
}

WinInputDevice::~WinInputDevice()
{
	// Remove this object's binding to its currently assigned device interface.
	Unbind();
}

#pragma endregion


#pragma region Public Methods
void WinInputDevice::BindTo(Interop::Input::InputDeviceInterface* deviceInterfacePointer)
{
	// Do not continue if the binding isn't changing.
	if (deviceInterfacePointer == fDeviceInterfacePointer)
	{
		return;
	}

	// Unsubscribe from the last device interface's events.
	if (fDeviceInterfacePointer)
	{
		fDeviceInterfacePointer->GetStatusChangedEventHandlers().Remove(&fStatusChangedEventHandler);
		fDeviceInterfacePointer->GetReceivedAxisInputEventHandlers().Remove(&fReceivedAxisInputEventHandler);
		fDeviceInterfacePointer->GetReceivedKeyInputEventHandlers().Remove(&fReceivedKeyInputEventHandler);
	}

	// Store the given device interface pointer.
	fDeviceInterfacePointer = deviceInterfacePointer;

	// Subscribe to the given device interface's events.
	if (fDeviceInterfacePointer)
	{
		fDeviceInterfacePointer->GetStatusChangedEventHandlers().Add(&fStatusChangedEventHandler);
		fDeviceInterfacePointer->GetReceivedAxisInputEventHandlers().Add(&fReceivedAxisInputEventHandler);
		fDeviceInterfacePointer->GetReceivedKeyInputEventHandlers().Add(&fReceivedKeyInputEventHandler);
	}

	// Update this object's device configuration with the given interface's information.
	UpdateDeviceSettings();
}

void WinInputDevice::Unbind()
{
	BindTo(nullptr);
}

Interop::Input::InputDeviceInterface* WinInputDevice::GetDeviceInterface() const
{
	return fDeviceInterfacePointer;
}

const char* WinInputDevice::GetProductName()
{
	if (!fDeviceInterfacePointer)
	{
		return PlatformInputDevice::GetProductName();
	}
	return fDeviceInterfacePointer->GetDeviceInfo()->GetProductNameAsUtf8();
}

const char* WinInputDevice::GetDisplayName()
{
	if (!fDeviceInterfacePointer)
	{
		return PlatformInputDevice::GetDisplayName();
	}
	return fDeviceInterfacePointer->GetDeviceInfo()->GetDisplayNameAsUtf8();
}

const char* WinInputDevice::GetPermanentStringId()
{
	if (!fDeviceInterfacePointer)
	{
		return PlatformInputDevice::GetPermanentStringId();
	}
	return fDeviceInterfacePointer->GetDeviceInfo()->GetPermanentStringId();
}

InputDeviceConnectionState WinInputDevice::GetConnectionState()
{
	if (!fDeviceInterfacePointer)
	{
		return PlatformInputDevice::GetConnectionState();
	}
	return fDeviceInterfacePointer->GetConnectionState();
}

bool WinInputDevice::CanVibrate()
{
	if (!fDeviceInterfacePointer)
	{
		return PlatformInputDevice::CanVibrate();
	}
	return fDeviceInterfacePointer->GetDeviceInfo()->CanVibrate();
}

void WinInputDevice::Vibrate()
{
	if (fDeviceInterfacePointer)
	{
		fDeviceInterfacePointer->RequestVibrate();
	}
	else
	{
		PlatformInputDevice::Vibrate();
	}
}

#pragma endregion


#pragma region Private Methods
void WinInputDevice::UpdateDeviceSettings()
{
	// Remove this object's old axis configurations.
	this->RemoveAllAxes();

	// Copy the Win32 device interface's axis configurations to this object.
	if (fDeviceInterfacePointer)
	{
		auto axisCollection = fDeviceInterfacePointer->GetDeviceInfo()->GetAxes();
		const int kAxisCount = axisCollection.GetCount();
		for (int index = 0; index < kAxisCount; index++)
		{
			// Add a new axis configuration to this object's collection.
			auto coronaAxisPointer = this->AddAxis();
			if (!coronaAxisPointer)
			{
				continue;
			}

			// Copy the device interface's axis config to this object axis config.
			auto interfaceAxisPointer = axisCollection.GetByIndex(index);
			if (!interfaceAxisPointer)
			{
				continue;
			}
			coronaAxisPointer->SetType(interfaceAxisPointer->GetType());
			coronaAxisPointer->SetIsAbsolute(interfaceAxisPointer->IsAbsolute());
			coronaAxisPointer->SetMinValue(Rtt_IntToReal(interfaceAxisPointer->GetMinValue()));
			coronaAxisPointer->SetMaxValue(Rtt_IntToReal(interfaceAxisPointer->GetMaxValue()));
			coronaAxisPointer->SetAccuracy(Rtt_FloatToReal(interfaceAxisPointer->GetAccuracy()));
		}
	}
}

void WinInputDevice::OnDeviceStatusChanged(
	Interop::Input::InputDeviceInterface& sender, const Interop::Input::InputDeviceStatusChangedEventArgs& arguments)
{
	// If the device this object is bound to was reconfigured, then update this object's configuration.
	if (arguments.WasReconfigured())
	{
		UpdateDeviceSettings();
	}

	// Dispatch an "inputDeviceStatus" event to Lua.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (runtimePointer)
	{
		Rtt::InputDeviceStatusEvent event(this, arguments.HasConnectionStateChanged(), arguments.WasReconfigured());
		runtimePointer->DispatchEvent(event);
	}
}

void WinInputDevice::OnReceivedAxisInput(
	Interop::Input::InputDeviceInterface& sender, const Interop::Input::InputAxisDataEventArgs& arguments)
{
	// Fetch the Corona runtime that owns this object.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (!runtimePointer)
	{
		return;
	}

	// Fetch the axis configuration that the received data belongs to.
	auto axisPointer = GetAxes().GetByIndex(arguments.GetAxisIndex());
	if (!axisPointer)
	{
		return;
	}

	// Dispatch an "axis" event to Lua.
	Rtt::AxisEvent event(this, axisPointer, Rtt_IntToReal(arguments.GetRawValue()));
	runtimePointer->DispatchEvent(event);
}

void WinInputDevice::OnReceivedKeyInput(
	Interop::Input::InputDeviceInterface& sender, const Interop::Input::KeyEventArgs& arguments)
{
	// Dispatch a "key" event to Lua.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (runtimePointer)
	{
		auto keyInfo = arguments.GetKey();
		auto modifierKeyStates = arguments.GetModifierKeyStates();
		Rtt::KeyEvent event(
				this, arguments.IsDown() ? Rtt::KeyEvent::kDown : Rtt::KeyEvent::kUp,
				keyInfo.GetCoronaName(), keyInfo.GetNativeCodeValue(),
				modifierKeyStates.IsShiftDown(), modifierKeyStates.IsAltDown(),
				modifierKeyStates.IsControlDown(), modifierKeyStates.IsCommandDown());
		runtimePointer->DispatchEvent(event);
	}
}

#pragma endregion

}	// namespace Rtt
