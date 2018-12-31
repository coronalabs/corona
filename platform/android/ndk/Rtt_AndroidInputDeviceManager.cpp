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


#include "Rtt_AndroidInputDeviceManager.h"
#include "Core/Rtt_Build.h"
#include "Input/Rtt_InputDeviceDescriptor.h"
#include "Input/Rtt_InputDeviceType.h"
#include "Rtt_AndroidInputDevice.h"
#include "NativeToJavaBridge.h"

namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new input device manager.
/// @param allocatorPointer Allocator needed to create input device objects.
AndroidInputDeviceManager::AndroidInputDeviceManager(Rtt_Allocator *allocatorPointer, NativeToJavaBridge *ntjb)
:	PlatformInputDeviceManager(allocatorPointer), fNativeToJavaBridge(ntjb)
{
}

/// Destroys this device manager and the input devices it manages.
AndroidInputDeviceManager::~AndroidInputDeviceManager()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Fetches an input device by its unique integer ID assigned by the Java side of Corona.
/// @param id Unique integer ID assigned by Corona's InputDeviceServices Java class.
/// @return Returns a pointer to the input device having the given ID.
///         <br>
///         Returns NULL if the given ID was not found.
AndroidInputDevice* AndroidInputDeviceManager::GetByCoronaDeviceId(int id) const
{
	AndroidInputDevice *devicePointer;

	const ReadOnlyInputDeviceCollection &collection = GetDevices();
	for (int index = 0; index < collection.GetCount(); index++)
	{
		devicePointer = (AndroidInputDevice*)collection.GetByIndex(index);
		if ((devicePointer != NULL) && (devicePointer->GetCoronaDeviceId() == id))
		{
			return devicePointer;
		}
	}
	return NULL;
}


// ----------------------------------------------------------------------------
// Protected Member Functions
// ----------------------------------------------------------------------------

/// Called when this device manager needs a new input device object to be created.
/// @param descriptor Unique descriptor used to identify the new input device.
/// @return Returns a pointer to the newly created input device object.
PlatformInputDevice* AndroidInputDeviceManager::CreateUsing(const InputDeviceDescriptor &descriptor)
{
	return Rtt_NEW(GetAllocator(), AndroidInputDevice(descriptor, fNativeToJavaBridge));
}

/// Called when this device manager needs to destroy the given input device object.
/// @param devicePointer Pointer to the input device object to be deleted.
void AndroidInputDeviceManager::Destroy(PlatformInputDevice* devicePointer)
{
	Rtt_DELETE(devicePointer);
}

// ----------------------------------------------------------------------------

} // namespace Rtt
