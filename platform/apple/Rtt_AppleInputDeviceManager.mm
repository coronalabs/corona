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

#import <Foundation/Foundation.h>
#include "Rtt_AppleInputDevice.h"
#include "Rtt_AppleInputDeviceManager.h"
#import <GameController/GameController.h>


namespace Rtt {
	
	
	AppleInputDeviceManager::AppleInputDeviceManager(Rtt_Allocator *allocatorPointer)
	: PlatformInputDeviceManager(allocatorPointer)
	, fCurrentDriverType(AppleInputDevice::kDriverNone)
	, fLastSeenPlayerIndex(-1)
	{
		
	}

	AppleInputDeviceManager::~AppleInputDeviceManager()
	{
		
	}

	
	PlatformInputDevice* AppleInputDeviceManager::CreateUsing(const InputDeviceDescriptor &descriptor)
	{
		PlatformInputDevice* ret = NULL;
		if (Rtt_VERIFY(AppleInputDevice::kDriverNone != fCurrentDriverType))
		{
			ret =  Rtt_NEW(GetAllocator(), AppleInputDevice(descriptor));
		}
		if (ret)
		{
			((AppleInputDevice*)ret)->fDriverType = fCurrentDriverType;
		}
		fCurrentDriverType = AppleInputDevice::kDriverNone;
		Rtt_ASSERT(ret);
		return ret;
	}
	
	AppleInputDevice* AppleInputDeviceManager::GetBySerialNumber(NSString* sn)
	{
		if (nil == sn)
		{
			return NULL;
		}
		
		AppleInputDevice *devicePointer;
		
		const ReadOnlyInputDeviceCollection &collection = GetDevices();
		for (int index = 0; index < collection.GetCount(); index++)
		{
			devicePointer = (AppleInputDevice*)collection.GetByIndex(index);
			if ((devicePointer != NULL) && ([devicePointer->fSerialNumber isEqualToString:sn]))
			{
				return devicePointer;
			}
		}
		return NULL;
	}

	void AppleInputDeviceManager::Destroy(PlatformInputDevice* devicePointer)
	{
		Rtt_DELETE(devicePointer);
	}
	
	AppleInputDevice* AppleInputDeviceManager::GetByMFiDevice(GCController* controller)
	{
		if ( nil == controller )
		{
			return NULL;
		}
		AppleInputDevice* ret = NULL;
		NSString* sn = AppleInputDevice::GetSerialNumber(controller);
		if (nil != sn)
		{
			ret = GetBySerialNumber(sn);
		}
		if (nil == ret)
		{
			const ReadOnlyInputDeviceCollection &collection = GetDevices();
			for (int index = 0; index < collection.GetCount(); index++)
			{
				AppleInputDevice* devicePointer = (AppleInputDevice*)collection.GetByIndex(index);
				if (devicePointer != NULL && devicePointer->fController == controller)
				{
					ret = devicePointer;
					break;
				}
			}
		}
		if (nil == ret)
		{
			// try our best to guess by configuration
			const ReadOnlyInputDeviceCollection &collection = GetDevices();
			for (int index = 0; index < collection.GetCount(); index++)
			{
				AppleInputDevice* devicePointer = (AppleInputDevice*)collection.GetByIndex(index);
				if (devicePointer != NULL
					&& devicePointer->fDriverType == AppleInputDevice::kDriverMFi
					&& devicePointer->fConnected == InputDeviceConnectionState::kDisconnected
					&& devicePointer->MatchConfiguration(controller) )
				{
					ret = devicePointer;
					break;
				}
			}
		}
		// Since we don't have to reliably match HID and MFi controller, lets try to do it with some "heuristics", like assume that vendor name would be same
		// it can be nil or empty. Both cases would not pass
		if (nil == ret)
		{
			ret = GetByVendorName(controller.vendorName, AppleInputDevice::kDriverHID);
		}
		
		return ret;
	}
	
	AppleInputDevice* AppleInputDeviceManager::GetByVendorName(NSString *vendor, AppleInputDevice::DriverType driver)
	{
		if (![vendor length])
		{
			return NULL;
		}
		const ReadOnlyInputDeviceCollection &collection = GetDevices();
		for (int index = 0; index < collection.GetCount(); index++)
		{
			AppleInputDevice* devicePointer = (AppleInputDevice*)collection.GetByIndex(index);
			if (devicePointer != NULL
				&& devicePointer->fDriverType == driver
				&& devicePointer->MatchVendorName(vendor) )
			{
				return devicePointer;
			}
		}
		return NULL;
	}
}