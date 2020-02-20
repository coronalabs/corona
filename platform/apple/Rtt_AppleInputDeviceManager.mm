//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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