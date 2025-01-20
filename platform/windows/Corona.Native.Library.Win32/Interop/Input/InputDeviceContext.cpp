//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputDeviceContext.h"
#include "InputAxisInfo.h"
#include "InputAxisSettings.h"
#include "InputDeviceInfo.h"
#include "InputDeviceSettings.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputDeviceContext::InputDeviceContext(const InputDeviceDriverType& driverType)
:	InputDeviceContext(driverType, nullptr)
{
}

InputDeviceContext::InputDeviceContext(
	const InputDeviceDriverType& driverType,
	const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* vibrationRequestHandlerPointer)
:	fDriverType(driverType),
	fDeviceInfoPointer(std::make_shared<InputDeviceInfo>()),
	fVibrationRequestEventHandlerPointer(vibrationRequestHandlerPointer),
	fConnectionState(Rtt::InputDeviceConnectionState::kDisconnected),
	fUpdateTransactionCount(0),
	fDeviceInterface(*this)
{
	// Initialize the pending data structure used by this object's UpdateWith() methods.
	fPendingUpdateData.ConnectionState = fConnectionState;
	fPendingUpdateData.DeviceInfoPointer = nullptr;
	fPendingUpdateData.AxisDataCollection.reserve(32);
	fPendingUpdateData.KeyDataCollection.reserve(32);

	// Set up the given vibration request handler to receive this context's "ReceivedVibrationRequest" event.
	// This handler is expected to come from a DirectInput or XInput device handling object and is used
	// to listen for vibration requests from the read-only InputDeviceInterface client side.
	fDeviceInterface.GetReceivedVibrationRequestEventHandlers().Add(fVibrationRequestEventHandlerPointer);
}

InputDeviceContext::~InputDeviceContext()
{
	// Remove event handlers.
	fDeviceInterface.GetReceivedVibrationRequestEventHandlers().Remove(fVibrationRequestEventHandlerPointer);
}

#pragma endregion


#pragma region Public Methods
InputDeviceInterface& InputDeviceContext::GetDeviceInterface()
{
	return fDeviceInterface;
}

const InputDeviceInterface& InputDeviceContext::GetDeviceInterface() const
{
	return fDeviceInterface;
}

const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* InputDeviceContext::GetVibrationRequestHandler() const
{
	return fVibrationRequestEventHandlerPointer;
}

bool InputDeviceContext::IsUpdating() const
{
	return (fUpdateTransactionCount > 0);
}

void InputDeviceContext::BeginUpdate()
{
	// Do not let the transaction count overflow.
	// This can only happen if we're doing something wrong and not calling EndUpdate() for every BeginUpdate().
	if (MAXINT == fUpdateTransactionCount)
	{
		Rtt_ASSERT(0);
		return;
	}

	// If this is the first call to BeginUpdate(), then reset member variables that track update changes.
	if (0 == fUpdateTransactionCount)
	{
		fPendingUpdateData.ConnectionState = fConnectionState;
		fPendingUpdateData.DeviceInfoPointer = nullptr;
	}

	// Increment the transaction count.
	// This also flags that we're in the middle of an update.
	fUpdateTransactionCount++;
}

void InputDeviceContext::UpdateWith(const Rtt::InputDeviceConnectionState& value)
{
	InputDeviceContext::ScopedUpdater scopedContextUpdater(*this);
	fPendingUpdateData.ConnectionState = value;
}

void InputDeviceContext::UpdateWith(const InputDeviceSettings& settings)
{
	// Do not continue if the device information isn't changing
	if (fDeviceInfoPointer->Equals(settings))
	{
		fPendingUpdateData.DeviceInfoPointer = nullptr;
		return;
	}
	if (fPendingUpdateData.DeviceInfoPointer.get() && fPendingUpdateData.DeviceInfoPointer->Equals(settings))
	{
		return;
	}

	// Update this context's device information.
	InputDeviceContext::ScopedUpdater scopedContextUpdater(*this);
	fPendingUpdateData.DeviceInfoPointer = std::make_shared<InputDeviceInfo>(settings);
}

void InputDeviceContext::UpdateWith(const std::shared_ptr<InputDeviceInfo>& infoPointer)
{
	// Validate.
	if (!infoPointer)
	{
		return;
	}

	// Do not continue if the device information isn't changing
	if (fDeviceInfoPointer->Equals(*infoPointer))
	{
		fPendingUpdateData.DeviceInfoPointer = nullptr;
		return;
	}
	if (fPendingUpdateData.DeviceInfoPointer.get() && fPendingUpdateData.DeviceInfoPointer->Equals(*infoPointer))
	{
		return;
	}

	// Update this context's device information.
	InputDeviceContext::ScopedUpdater scopedContextUpdater(*this);
	fPendingUpdateData.DeviceInfoPointer = infoPointer;
}

void InputDeviceContext::UpdateWith(const InputAxisDataEventArgs::Data& data)
{
	// Update this context with the given axis data.
	InputDeviceContext::ScopedUpdater scopedContextUpdater(*this);

	// First, check if we've stored data for this axis index before. If so, then replace the data.
	// Note: An STL "map" would make this easier, but we use an STL "vector" for best performance and memory usage.
	//       A vector does not delete memory when cleared, allowing us to re-use that memory for future data updates.
	bool wasCollectionUpdated = false;
	auto count = fPendingUpdateData.AxisDataCollection.size();
	if (count > 0)
	{
		auto pendingDataPointer = &(fPendingUpdateData.AxisDataCollection.front());
		for (unsigned int index = 0; index < count; ++index, ++pendingDataPointer)
		{
			if (data.AxisIndex == pendingDataPointer->EventData.AxisIndex)
			{
				pendingDataPointer->EventData = data;
				wasCollectionUpdated = true;
				break;
			}
		}
	}

	// If the given data wasn't added up above, then insert it into the pending data collection.
	if (false == wasCollectionUpdated)
	{
		PendingAxisData pendingData;
		pendingData.EventData = data;
		fPendingUpdateData.AxisDataCollection.push_back(pendingData);
	}
}

void InputDeviceContext::UpdateWith(const KeyEventArgs::Data& data)
{
	// Update this context with the given key data.
	InputDeviceContext::ScopedUpdater scopedContextUpdater(*this);

	// First, check if we've stored data for this key before. If so, then replace the data.
	// Note: An STL "map" would make this easier, but we use an STL "vector" for best performance and memory usage.
	//       A vector does not delete memory when cleared, allowing us to re-use that memory for future data updates.
	bool wasCollectionUpdated = false;
	auto count = fPendingUpdateData.KeyDataCollection.size();
	if (count > 0)
	{
		auto pendingDataPointer = &(fPendingUpdateData.KeyDataCollection.front());
		for (unsigned int index = 0; index < count; ++index, ++pendingDataPointer)
		{
			if (data.NativeCode == pendingDataPointer->EventData.NativeCode)
			{
				pendingDataPointer->EventData = data;
				wasCollectionUpdated = true;
				break;
			}
		}
	}

	// If the given data wasn't added up above, then insert it into the pending data collection.
	if (false == wasCollectionUpdated)
	{
		PendingKeyData pendingData;
		pendingData.EventData = data;
		fPendingUpdateData.KeyDataCollection.push_back(pendingData);
	}
}

void InputDeviceContext::EndUpdate()
{
	// Do not continue if BeginUpdate() was not called.
	// Note: The BeginUpdate() and EndUpdate() methods *must* be called in pairs.
	if (fUpdateTransactionCount <= 0)
	{
		Rtt_ASSERT(0);
		return;
	}

	// Decrement the transaction count for the BeginUpdate/EndUpdate pairing.
	fUpdateTransactionCount--;

	// Do not continue if we're still in the middle of an update.
	// That is, do not finish the transaction until EndUpdate() has been called as many times as BeginUdpate().
	if (fUpdateTransactionCount > 0)
	{
		return;
	}

	// Update this device's connection state and configuration, if changed.
	InputDeviceStatusChangedEventArgs::Settings statusChangedSettings{};
	if (fConnectionState.NotEquals(fPendingUpdateData.ConnectionState))
	{
		// The connection state has changed.
		fConnectionState = fPendingUpdateData.ConnectionState;
		statusChangedSettings.HasConnectionStateChanged = true;
	}
	if (fPendingUpdateData.DeviceInfoPointer.get())
	{
		if (fDeviceInfoPointer->NotEquals(*fPendingUpdateData.DeviceInfoPointer))
		{
			// *** The device information has changed. ***

			// Remove the last recorded data belonging to axes that no longer exist in the new device configuration.
			auto oldAxisCollection = fDeviceInfoPointer->GetAxes();
			auto newAxisCollection = fPendingUpdateData.DeviceInfoPointer->GetAxes();
			int oldAxisCount = oldAxisCollection.GetCount();
			int newAxisCount = newAxisCollection.GetCount();
			int maxAxisCount = (oldAxisCount >= newAxisCount) ? oldAxisCount : newAxisCount;
			for (int index = maxAxisCount - 1; index >= 0; index--)
			{
				auto oldAxisPointer = oldAxisCollection.GetByIndex(index);
				auto newAxisPointer = newAxisCollection.GetByIndex(index);
				if (!oldAxisPointer || !newAxisPointer || oldAxisPointer->NotEquals(*newAxisPointer))
				{
					fLastReceivedAxisData.erase((unsigned int)index);
				}
			}

			// Remove the last recorded data belonging to keys that no longer exist in the new device configuration.
			auto oldKeyCollection = fDeviceInfoPointer->GetKeys();
			auto newKeyCollection = fPendingUpdateData.DeviceInfoPointer->GetKeys();
			for (int index = oldKeyCollection.GetCount() - 1; index >= 0; index--)
			{
				auto keyPointer = oldKeyCollection.GetByIndex(index);
				if (keyPointer)
				{
					if (newKeyCollection.Contains(*keyPointer) == false)
					{
						fLastReceivedKeyStates.erase(*keyPointer);
					}
				}
			}

			// Copy the new device information and flag that it has been changed.
			fDeviceInfoPointer = fPendingUpdateData.DeviceInfoPointer;
			fPendingUpdateData.DeviceInfoPointer = nullptr;
			statusChangedSettings.WasReconfigured = true;
		}
	}

	// Update this device's last recorded axis/key value collections.
	if (fConnectionState.IsConnected())
	{
		// Process all axis data received via the UpdateWith() method.
		for (auto&& pendingAxisData : fPendingUpdateData.AxisDataCollection)
		{
			// First, intialize the pending axis data to not be raised as a "ReceivedAxisInput" event.
			pendingAxisData.IsNew = false;

			// Fetch the data's axis information.
			auto axisInfoPointer = fDeviceInfoPointer->GetAxes().GetByIndex((int)pendingAxisData.EventData.AxisIndex);
			if (!axisInfoPointer)
			{
				continue;
			}

			// Make sure that the axis data does not exceed the axis' min/max bounds.
			if (pendingAxisData.EventData.RawValue > axisInfoPointer->GetMaxValue())
			{
				pendingAxisData.EventData.RawValue = axisInfoPointer->GetMaxValue();
			}
			else if (pendingAxisData.EventData.RawValue < axisInfoPointer->GetMinValue())
			{
				pendingAxisData.EventData.RawValue = axisInfoPointer->GetMinValue();
			}

			// Do not continue if the axis data matches the last recorded data.
			// That is, ignore duplicate values. We only want to raise events when the data changes.
			auto iterator = fLastReceivedAxisData.find(pendingAxisData.EventData.AxisIndex);
			if (iterator != fLastReceivedAxisData.end())
			{
				if (pendingAxisData.EventData.RawValue == (*iterator).second)
				{
					continue;
				}
			}

			// This is *new* axis data. Flag it to be raised via a "ReceivedAxisInput" event down below.
			pendingAxisData.IsNew = true;

			// Store this data to the last recorded values collection.
			fLastReceivedAxisData[pendingAxisData.EventData.AxisIndex] = pendingAxisData.EventData.RawValue;
		}

		// Process all key event data received via the UpdateWith() method.
		for (auto&& pendingKeyData : fPendingUpdateData.KeyDataCollection)
		{
			// First, initialize the pending key event data to not be raised as a "ReceivedKeyInput" event.
			pendingKeyData.IsNew = false;

			// Fetch a key object for the given native integer key code.
			auto key = Key::FromNativeCode(pendingKeyData.EventData.NativeCode);

			// Do not continue if the key for the given data does not exist on this device.
			if (fDeviceInfoPointer->GetKeys().Contains(key) == false)
			{
				continue;
			}

			// Do not continue if the key's up/down state hasn't changed.
			auto iterator = fLastReceivedKeyStates.find(key);
			if (iterator != fLastReceivedKeyStates.end())
			{
				if (pendingKeyData.EventData.IsDown == (*iterator).second)
				{
					continue;
				}
			}
			else if (false == pendingKeyData.EventData.IsDown)
			{
				continue;
			}

			// This is *new* key event data. Flag it to be raised via a "ReceivedKeyInput" event down below.
			pendingKeyData.IsNew = true;

			// Store this data to the last recorded values collection.
			fLastReceivedKeyStates[key] = pendingKeyData.EventData.IsDown;
		}
	}
	else
	{
		// We're not connected to the device. Clear all axis and key data.
		fLastReceivedAxisData.clear();
		fLastReceivedKeyStates.clear();
		fPendingUpdateData.AxisDataCollection.clear();
		fPendingUpdateData.KeyDataCollection.clear();
	}

	// If the device's status has changed, then notify the system.
	if (statusChangedSettings.HasConnectionStateChanged || statusChangedSettings.WasReconfigured)
	{
		// Raise a "StatusChanged" event.
		fStatusChangedEvent.Raise(fDeviceInterface, InputDeviceStatusChangedEventArgs(statusChangedSettings));

		// If a handler for the above event has called BeginUpdate(), then exit out of this EndUpdate().
		// We now must wait for EndUpdate() to get called again before commiting the event handler's changes.
		if (IsUpdating())
		{
			return;
		}
	}

	// Notify the system about all key up/down state changes received.
	// Note: The below will raise all released-up key events before pressed-down key events.
	//       This helps minimize support issues for apps that mishandle sudden key changes. Especially with a d-pad.
	auto& keyDataCollection = fPendingUpdateData.KeyDataCollection;
	for (int keyPhaseEventPass = 0; keyPhaseEventPass < 2; keyPhaseEventPass++)
	{
		bool isDown = (keyPhaseEventPass != 0);
		for (auto iterator = keyDataCollection.begin(); iterator != keyDataCollection.end(); iterator++)
		{
			// Ignore data if not flagged as new up above. This way:
			// - We only raise events when input data has changed. (No duplicate values.)
			// - We ignore keys that do not exist on the device. (In case invalid data was received.)
			if (false == iterator->IsNew)
			{
				continue;
			}

			// Skip this key event for now if the up/down phase doesn't match what the outer loop is requesting.
			// The intent is to raise all "up" phase events first and then all "down" phase events last.
			if (iterator->EventData.IsDown != isDown)
			{
				continue;
			}

			// Raise a "ReceivedKeyInput" event.
			fReceivedKeyInputEvent.Raise(fDeviceInterface, KeyEventArgs(iterator->EventData));

			// If a handler for the above event has called BeginUpdate(), then exit out of this EndUpdate().
			// We now must wait for EndUpdate() to get called again before commiting the event handler's changes.
			if (IsUpdating())
			{
				return;
			}

			// Check if a handler for the above event has cleared the collection that we're currently traversing.
			// This can happen if the handler calls BeginUpdate() and EndUpdate().
			if (keyDataCollection.empty())
			{
				break;
			}
		}
	}
	keyDataCollection.clear();

	// Notify the system about all axis data changes received.
	auto& axisDataCollection = fPendingUpdateData.AxisDataCollection;
	for (auto iterator = axisDataCollection.begin(); iterator != axisDataCollection.end(); iterator++)
	{
		// Ignore data if not flagged as new up above. This way:
		// - We only raise events when input data has changed. (No duplicate values.)
		// - We ignore axis indexes that do not exist on the device. (In case invalid data was received.)
		if (false == iterator->IsNew)
		{
			continue;
		}

		// Raise a "ReceivedAxisInput" event.
		fReceivedAxisInputEvent.Raise(fDeviceInterface, InputAxisDataEventArgs(iterator->EventData));

		// If a handler for the above event has called BeginUpdate(), then exit out of this EndUpdate().
		// We now must wait for EndUpdate() to get called again before commiting the event handler's changes.
		if (IsUpdating())
		{
			return;
		}

		// Check if a handler for the above event has cleared the collection that we're currently traversing.
		// This can happen if the handler calls BeginUpdate() and EndUpdate().
		if (axisDataCollection.empty())
		{
			break;
		}
	}
	axisDataCollection.clear();
}

#pragma endregion


#pragma region ScopedUpdater Class Methods
InputDeviceContext::ScopedUpdater::ScopedUpdater(InputDeviceContext& context)
:	fContext(context)
{
	fContext.BeginUpdate();
}

InputDeviceContext::ScopedUpdater::~ScopedUpdater()
{
	fContext.EndUpdate();
}

#pragma endregion

} }	// namespace Interop::Input
