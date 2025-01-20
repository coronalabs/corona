//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputDeviceInfo.h"
#include "InputAxisInfo.h"
#include "InputAxisSettings.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputDeviceInfo::InputDeviceInfo()
:	InputDeviceInfo(InputDeviceSettings())
{
}

InputDeviceInfo::InputDeviceInfo(const InputDeviceSettings& settings)
:	fSettings(settings)
{
	// Create immutable copies of the given input device's axis configurations and copy them to this object's collection.
	// Note: We currently need to hold on to the InputAxisSettings objects for easy Equals() checks.
	auto axisCollection = fSettings.GetAxes();
	for (int index = 0; index < axisCollection.GetCount(); index++)
	{
		InputAxisInfo* axisInfoPointer = nullptr;
		auto axisSettingsPointer = axisCollection.GetByIndex(index);
		if (axisSettingsPointer)
		{
			axisInfoPointer = new InputAxisInfo(*axisSettingsPointer);
		}
		fAxisCollection.push_back(axisInfoPointer);
	}
}

InputDeviceInfo::~InputDeviceInfo()
{
	// Delete the axis collection's objects.
	for (auto&& axisPointer : fAxisCollection)
	{
		if (axisPointer)
		{
			delete axisPointer;
		}
	}
	fAxisCollection.clear();
}

#pragma endregion


#pragma region Public Methods
void InputDeviceInfo::CopyTo(InputDeviceSettings& settings) const
{
	settings.CopyFrom(fSettings);
}

Rtt::InputDeviceType InputDeviceInfo::GetType() const
{
	return fSettings.GetType();
}

const char* InputDeviceInfo::GetPermanentStringId() const
{
	return fSettings.GetPermanentStringId();
}

const char* InputDeviceInfo::GetProductNameAsUtf8() const
{
	return fSettings.GetProductNameAsUtf8();
}

const wchar_t* InputDeviceInfo::GetProductNameAsUtf16() const
{
	return fSettings.GetProductNameAsUtf16();
}

const char* InputDeviceInfo::GetDisplayNameAsUtf8() const
{
	return fSettings.GetDisplayNameAsUtf8();
}

const wchar_t* InputDeviceInfo::GetDisplayNameAsUtf16() const
{
	return fSettings.GetDisplayNameAsUtf16();
}

unsigned int InputDeviceInfo::GetPlayerNumber() const
{
	return fSettings.GetPlayerNumber();
}

bool InputDeviceInfo::CanVibrate() const
{
	return fSettings.CanVibrate();
}

InputDeviceInfo::AxisCollectionAdapter InputDeviceInfo::GetAxes() const
{
	return InputDeviceInfo::AxisCollectionAdapter(*this);
}

InputDeviceInfo::KeyCollectionAdapter InputDeviceInfo::GetKeys() const
{
	return InputDeviceInfo::KeyCollectionAdapter(*this);
}

bool InputDeviceInfo::Equals(const InputDeviceInfo& value) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (&value == this)
	{
		return true;
	}

	// Compare configurations.
	return fSettings.Equals(value.fSettings);
}

bool InputDeviceInfo::Equals(const InputDeviceSettings& value) const
{
	return fSettings.Equals(value);
}

bool InputDeviceInfo::NotEquals(const InputDeviceInfo& value) const
{
	return !Equals(value);
}

bool InputDeviceInfo::NotEquals(const InputDeviceSettings& value) const
{
	return !Equals(value);
}

bool InputDeviceInfo::operator==(const InputDeviceInfo& value) const
{
	return Equals(value);
}

bool InputDeviceInfo::operator==(const InputDeviceSettings& value) const
{
	return Equals(value);
}

bool InputDeviceInfo::operator!=(const InputDeviceInfo& value) const
{
	return !Equals(value);
}

bool InputDeviceInfo::operator!=(const InputDeviceSettings& value) const
{
	return !Equals(value);
}

#pragma endregion


#pragma region AxisCollectionAdapter Class Members
InputDeviceInfo::AxisCollectionAdapter::AxisCollectionAdapter(const InputDeviceInfo& info)
:	fInputDevice(info)
{
}

InputDeviceInfo::AxisCollectionAdapter::~AxisCollectionAdapter()
{
}

int InputDeviceInfo::AxisCollectionAdapter::GetCount() const
{
	return (int)(fInputDevice.fAxisCollection.size());
}

InputAxisInfo* InputDeviceInfo::AxisCollectionAdapter::GetByIndex(int index) const
{
	if ((index >= 0) && (index < GetCount()))
	{
		return fInputDevice.fAxisCollection.at(index);
	}
	return nullptr;
}

bool InputDeviceInfo::AxisCollectionAdapter::Contains(const Rtt::InputAxisType& value) const
{
	for (auto&& itemPointer : fInputDevice.fAxisCollection)
	{
		if (itemPointer && itemPointer->GetType().Equals(value))
		{
			return true;
		}
	}
	return false;
}

bool InputDeviceInfo::AxisCollectionAdapter::Equals(const InputDeviceInfo::AxisCollectionAdapter& collection) const
{
	// First, check if both collections reference the same input device object. (ie: Compare memory addresses.)
	if (&collection.fInputDevice == &fInputDevice)
	{
		return true;
	}

	// Next, check if the number of axis configurations differ between the two collections.
	if (GetCount() != collection.GetCount())
	{
		return false;
	}

	// Finally, do the more intensive check.
	// Compare each axis configuration between the two collections.
	for (int index = GetCount() - 1; index >= 0; index--)
	{
		auto axisPointer1 = GetByIndex(index);
		auto axisPointer2 = collection.GetByIndex(index);
		if (axisPointer1 == axisPointer2)
		{
			// Memory addresses match or they are both null. Both cases count as a match.
			continue;
		}
		else if (!axisPointer1 || !axisPointer2)
		{
			// One pointer is null and the other is not. Not a match.
			return false;
		}
		else if (axisPointer1->NotEquals(*axisPointer2))
		{
			// Configurations do not match.
			return false;
		}
	}

	// The given collection matches this collection.
	return true;
}

bool InputDeviceInfo::AxisCollectionAdapter::NotEquals(const InputDeviceInfo::AxisCollectionAdapter& collection) const
{
	return !Equals(collection);
}

bool InputDeviceInfo::AxisCollectionAdapter::operator==(const InputDeviceInfo::AxisCollectionAdapter& collection) const
{
	return Equals(collection);
}

bool InputDeviceInfo::AxisCollectionAdapter::operator!=(const InputDeviceInfo::AxisCollectionAdapter& collection) const
{
	return !Equals(collection);
}

#pragma endregion


#pragma region KeyCollectionAdapter Class Members
InputDeviceInfo::KeyCollectionAdapter::KeyCollectionAdapter(const InputDeviceInfo& info)
:	fInputDevice(info)
{
}

InputDeviceInfo::KeyCollectionAdapter::~KeyCollectionAdapter()
{
}

int InputDeviceInfo::KeyCollectionAdapter::GetCount() const
{
	return fInputDevice.fSettings.GetKeys().GetCount();
}

const Key* InputDeviceInfo::KeyCollectionAdapter::GetByIndex(int index) const
{
	return fInputDevice.fSettings.GetKeys().GetByIndex(index);
}

bool InputDeviceInfo::KeyCollectionAdapter::Contains(const Key& value) const
{
	return fInputDevice.fSettings.GetKeys().Contains(value);
}

bool InputDeviceInfo::KeyCollectionAdapter::Equals(const InputDeviceInfo::KeyCollectionAdapter& collection) const
{
	// First, check if both collections reference the same input device object. (ie: Compare memory addresses.)
	if (&collection.fInputDevice == &fInputDevice)
	{
		return true;
	}

	// Compare keys between the collections.
	return fInputDevice.fSettings.GetKeys().Equals(collection.fInputDevice.fSettings.GetKeys());
}

bool InputDeviceInfo::KeyCollectionAdapter::NotEquals(const InputDeviceInfo::KeyCollectionAdapter& collection) const
{
	return !Equals(collection);
}

bool InputDeviceInfo::KeyCollectionAdapter::operator==(const InputDeviceInfo::KeyCollectionAdapter& collection) const
{
	return Equals(collection);
}

bool InputDeviceInfo::KeyCollectionAdapter::operator!=(const InputDeviceInfo::KeyCollectionAdapter& collection) const
{
	return !Equals(collection);
}

#pragma endregion

} }	// namespace Interop::Input
