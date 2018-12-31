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
#include "InputDeviceSettings.h"
#include "InputAxisSettings.h"
#include <algorithm>


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputDeviceSettings::InputDeviceSettings()
:	fType(Rtt::InputDeviceType::kUnknown),
	fPermanentStringId(""),
	fProductName(L""),
	fDisplayName(L""),
	fPlayerNumber(0),
	fCanVibrate(false)
{
}

InputDeviceSettings::InputDeviceSettings(const InputDeviceSettings& settings)
:	InputDeviceSettings()
{
	CopyFrom(settings);
}

InputDeviceSettings::~InputDeviceSettings()
{
	// Delete the axis collection's objects.
	GetAxes().Clear();
}

#pragma endregion


#pragma region Public Methods
Rtt::InputDeviceType InputDeviceSettings::GetType() const
{
	return fType;
}

void InputDeviceSettings::SetType(const Rtt::InputDeviceType& value)
{
	fType = value;
}

const char* InputDeviceSettings::GetPermanentStringId() const
{
	return fPermanentStringId.c_str();
}

void InputDeviceSettings::SetPermanentStringId(const char* value)
{
	fPermanentStringId = value ? value : "";
}

const char* InputDeviceSettings::GetProductNameAsUtf8() const
{
	return fProductName.GetUTF8();
}

const wchar_t* InputDeviceSettings::GetProductNameAsUtf16() const
{
	return fProductName.GetUTF16();
}

void InputDeviceSettings::SetProductName(const char* value)
{
	fProductName.SetUTF8(value ? value : "");
}

void InputDeviceSettings::SetProductName(const wchar_t* value)
{
	fProductName.SetUTF16(value ? value : L"");
}

const char* InputDeviceSettings::GetDisplayNameAsUtf8() const
{
	return fDisplayName.GetUTF8();
}

const wchar_t* InputDeviceSettings::GetDisplayNameAsUtf16() const
{
	return fDisplayName.GetUTF16();
}

void InputDeviceSettings::SetDisplayName(const char* value)
{
	fDisplayName.SetUTF8(value ? value : "");
}

void InputDeviceSettings::SetDisplayName(const wchar_t* value)
{
	fDisplayName.SetUTF16(value ? value : L"");
}

unsigned int InputDeviceSettings::GetPlayerNumber() const
{
	return fPlayerNumber;
}

void InputDeviceSettings::SetPlayerNumber(unsigned int value)
{
	fPlayerNumber = value;
}

bool InputDeviceSettings::CanVibrate() const
{
	return fCanVibrate;
}

void InputDeviceSettings::SetCanVibrate(bool value)
{
	fCanVibrate = value;
}

InputDeviceSettings::AxisCollectionAdapter InputDeviceSettings::GetAxes()
{
	return InputDeviceSettings::AxisCollectionAdapter(*this);
}

const InputDeviceSettings::AxisCollectionAdapter InputDeviceSettings::GetAxes() const
{
	return InputDeviceSettings::AxisCollectionAdapter(*this);
}

InputDeviceSettings::KeyCollectionAdapter InputDeviceSettings::GetKeys()
{
	return InputDeviceSettings::KeyCollectionAdapter(*this);
}

const InputDeviceSettings::KeyCollectionAdapter InputDeviceSettings::GetKeys() const
{
	return InputDeviceSettings::KeyCollectionAdapter(*this);
}

void InputDeviceSettings::CopyFrom(const InputDeviceSettings& settings)
{
	// Copy the given settings main member variables.
	fType = settings.fType;
	fPermanentStringId = settings.fPermanentStringId;
	fProductName = settings.fProductName;
	fDisplayName = settings.fDisplayName;
	fPlayerNumber = settings.fPlayerNumber;
	fCanVibrate = settings.fCanVibrate;
	fKeyCollection = settings.fKeyCollection;

	// Do a deep copy of the axis settings collection.
	GetAxes().Clear();
	for (auto&& sourceAxisSettingsPointer : settings.fAxisCollection)
	{
		InputAxisSettings* newAxisSettingsPointer = nullptr;
		if (sourceAxisSettingsPointer)
		{
			newAxisSettingsPointer = new InputAxisSettings(*sourceAxisSettingsPointer);
		}
		fAxisCollection.push_back(newAxisSettingsPointer);
	}
}

void InputDeviceSettings::operator=(const InputDeviceSettings& settings)
{
	CopyFrom(settings);
}

bool InputDeviceSettings::Equals(const InputDeviceSettings& value) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (&value == this)
	{
		return true;
	}

	// Compare configurations.
	if (fType != value.fType)
	{
		return false;
	}
	if (fPermanentStringId != value.fPermanentStringId)
	{
		return false;
	}
	if (fProductName != value.fProductName)
	{
		return false;
	}
	if (fDisplayName != value.fDisplayName)
	{
		return false;
	}
	if (fPlayerNumber != value.fPlayerNumber)
	{
		return false;
	}
	if (fCanVibrate != value.fCanVibrate)
	{
		return false;
	}
	if (GetAxes().NotEquals(value.GetAxes()))
	{
		return false;
	}
	if (GetKeys().NotEquals(value.GetKeys()))
	{
		return false;
	}

	// The given settings matches this object's settings.
	return true;
}

bool InputDeviceSettings::NotEquals(const InputDeviceSettings& value) const
{
	return !Equals(value);
}

bool InputDeviceSettings::operator==(const InputDeviceSettings& value) const
{
	return Equals(value);
}

bool InputDeviceSettings::operator!=(const InputDeviceSettings& value) const
{
	return !Equals(value);
}

#pragma endregion


#pragma region AxisCollectionAdapter Class Members
InputDeviceSettings::AxisCollectionAdapter::AxisCollectionAdapter(InputDeviceSettings& settings)
:	fSettings(settings)
{
}

InputDeviceSettings::AxisCollectionAdapter::AxisCollectionAdapter(const InputDeviceSettings& settings)
:	fSettings(const_cast<InputDeviceSettings&>(settings))
{
}

InputDeviceSettings::AxisCollectionAdapter::~AxisCollectionAdapter()
{
}

InputAxisSettings* InputDeviceSettings::AxisCollectionAdapter::Add()
{
	auto axisSettingsPointer = new InputAxisSettings();
	fSettings.fAxisCollection.push_back(axisSettingsPointer);
	return axisSettingsPointer;
}

void InputDeviceSettings::AxisCollectionAdapter::Clear()
{
	for (auto&& itemPointer : fSettings.fAxisCollection)
	{
		if (itemPointer)
		{
			delete itemPointer;
		}
	}
	fSettings.fAxisCollection.clear();
}

int InputDeviceSettings::AxisCollectionAdapter::GetCount() const
{
	return (int)(fSettings.fAxisCollection.size());
}

InputAxisSettings* InputDeviceSettings::AxisCollectionAdapter::GetByIndex(int index) const
{
	if ((index >= 0) && (index < GetCount()))
	{
		return fSettings.fAxisCollection.at(index);
	}
	return nullptr;
}

bool InputDeviceSettings::AxisCollectionAdapter::Contains(const Rtt::InputAxisType& value) const
{
	for (auto&& itemPointer : fSettings.fAxisCollection)
	{
		if (itemPointer && itemPointer->GetType().Equals(value))
		{
			return true;
		}
	}
	return false;
}

bool InputDeviceSettings::AxisCollectionAdapter::Equals(
	const InputDeviceSettings::AxisCollectionAdapter& collection) const
{
	// First, check if both collections reference the same input device object. (ie: Compare memory addresses.)
	if (&collection.fSettings == &fSettings)
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

bool InputDeviceSettings::AxisCollectionAdapter::NotEquals(
	const InputDeviceSettings::AxisCollectionAdapter& collection) const
{
	return !Equals(collection);
}

bool InputDeviceSettings::AxisCollectionAdapter::operator==(
	const InputDeviceSettings::AxisCollectionAdapter& collection) const
{
	return Equals(collection);
}

bool InputDeviceSettings::AxisCollectionAdapter::operator!=(
	const InputDeviceSettings::AxisCollectionAdapter& collection) const
{
	return !Equals(collection);
}

#pragma endregion


#pragma region KeyCollectionAdapter Class Members
InputDeviceSettings::KeyCollectionAdapter::KeyCollectionAdapter(InputDeviceSettings& settings)
:	fSettings(settings)
{
}

InputDeviceSettings::KeyCollectionAdapter::KeyCollectionAdapter(const InputDeviceSettings& settings)
:	fSettings(const_cast<InputDeviceSettings&>(settings))
{
}

InputDeviceSettings::KeyCollectionAdapter::~KeyCollectionAdapter()
{
}

bool InputDeviceSettings::KeyCollectionAdapter::Add(const Key& value)
{
	// Do not continue if the key already exists in the collection.
	if (Contains(value))
	{
		return false;
	}

	// Add the key to the end of the collection.
	fSettings.fKeyCollection.push_back(value);
	return true;
}

void InputDeviceSettings::KeyCollectionAdapter::Clear()
{
	fSettings.fKeyCollection.clear();
}

int InputDeviceSettings::KeyCollectionAdapter::GetCount() const
{
	return (int)(fSettings.fKeyCollection.size());
}

const Key* InputDeviceSettings::KeyCollectionAdapter::GetByIndex(int index) const
{
	if ((index >= 0) && (index < GetCount()))
	{
		return &(fSettings.fKeyCollection.at(index));
	}
	return nullptr;
}

bool InputDeviceSettings::KeyCollectionAdapter::Contains(const Key& value) const
{
	auto iterator = std::find(fSettings.fKeyCollection.begin(), fSettings.fKeyCollection.end(), value);
	return (iterator != fSettings.fKeyCollection.end());
}

bool InputDeviceSettings::KeyCollectionAdapter::Equals(
	const InputDeviceSettings::KeyCollectionAdapter& collection) const
{
	// First, check if both collections reference the same input device object. (ie: Compare memory addresses.)
	if (&collection.fSettings == &fSettings)
	{
		return true;
	}

	// Next, check if the number of keys differ between the two collections.
	if (GetCount() != collection.GetCount())
	{
		return false;
	}

	// Finally, do the more intensive check.
	// Compare each key between the two collections.
	for (int index = GetCount() - 1; index >= 0; index--)
	{
		auto keyPointer1 = GetByIndex(index);
		auto keyPointer2 = collection.GetByIndex(index);
		if (keyPointer1 == keyPointer2)
		{
			// Memory addresses match or they are both null. Both cases count as a match.
			continue;
		}
		else if (!keyPointer1 || !keyPointer2)
		{
			// One pointer is null and the other is not. Not a match.
			return false;
		}
		else if (keyPointer1->NotEquals(*keyPointer2))
		{
			// Configurations do not match.
			return false;
		}
	}

	// The given collection matches this collection.
	return true;
}

bool InputDeviceSettings::KeyCollectionAdapter::NotEquals(
	const InputDeviceSettings::KeyCollectionAdapter& collection) const
{
	return !Equals(collection);
}

bool InputDeviceSettings::KeyCollectionAdapter::operator==(
	const InputDeviceSettings::KeyCollectionAdapter& collection) const
{
	return Equals(collection);
}

bool InputDeviceSettings::KeyCollectionAdapter::operator!=(
	const InputDeviceSettings::KeyCollectionAdapter& collection) const
{
	return !Equals(collection);
}

#pragma endregion

} }	// namespace Interop::Input
