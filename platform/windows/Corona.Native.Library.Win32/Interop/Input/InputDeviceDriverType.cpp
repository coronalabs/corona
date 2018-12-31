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
#include "InputDeviceDriverType.h"
#include <exception>
#include <list>


namespace Interop { namespace Input {

#pragma region Static Variables
/// <summary>
///  <para>Stores a collection of pointers to this class' driver type constants.</para>
///  <para>Used by this class' static From*() methods to fetch driver types by their integer or string IDs.</para>
/// </summary>
static std::list<InputDeviceDriverType*> sDriveTypeCollection;

#pragma endregion


#pragma region Static Constants
const InputDeviceDriverType InputDeviceDriverType::kDirectInput("directInput", 0);

const InputDeviceDriverType InputDeviceDriverType::kRawInput("rawInput", 1);

const InputDeviceDriverType InputDeviceDriverType::kXInput("xInput", 2);

#pragma endregion


#pragma region Constructors/Destructors
InputDeviceDriverType::InputDeviceDriverType(const char* stringId, int integerId)
:	fStringId(stringId),
	fIntegerId(integerId)
{
	// Validate.
	if (!fStringId || ('\0' == fStringId[0]))
	{
		throw std::exception();
	}

	// Add the driver type to the global collection.
	sDriveTypeCollection.push_back(this);
}

InputDeviceDriverType::~InputDeviceDriverType()
{
}

#pragma endregion


#pragma region Public Methods
int InputDeviceDriverType::GetIntegerId() const
{
	return fIntegerId;
}

const char* InputDeviceDriverType::GetStringId() const
{
	return fStringId;
}

bool InputDeviceDriverType::Equals(const InputDeviceDriverType& value) const
{
	return (fStringId == value.fStringId);
}

bool InputDeviceDriverType::NotEquals(const InputDeviceDriverType& value) const
{
	return !Equals(value);
}

bool InputDeviceDriverType::operator==(const InputDeviceDriverType& value) const
{
	return Equals(value);
}

bool InputDeviceDriverType::operator!=(const InputDeviceDriverType& value) const
{
	return !Equals(value);
}

#pragma endregion


#pragma region Public Static Functions
const InputDeviceDriverType* InputDeviceDriverType::FromIntegerId(int integerId)
{
	for (auto driverTypePointer : sDriveTypeCollection)
	{
		if (integerId == driverTypePointer->fIntegerId)
		{
			return driverTypePointer;
		}
	}
	return nullptr;
}

const InputDeviceDriverType* InputDeviceDriverType::FromStringId(const char* stringId)
{
	if (stringId && (stringId[0] != '\0'))
	{
		for (auto driverTypePointer : sDriveTypeCollection)
		{
			if ((stringId == driverTypePointer->fStringId) || !strcmp(stringId, driverTypePointer->fStringId))
			{
				return driverTypePointer;
			}
		}
	}
	return nullptr;
}

#pragma endregion

} }	// namespace Interop::Input
