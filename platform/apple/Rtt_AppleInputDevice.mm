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

#include "Core/Rtt_Config.h"
#import <Foundation/Foundation.h>
#import <GameController/GameController.h>

#ifdef Rtt_APPLE_HID
	#import <ForceFeedback/ForceFeedback.h>
#endif

#include "Rtt_AppleInputDeviceManager.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Rtt_KeyName.h"
#include "Rtt_Lua.h"


namespace Rtt {
	
	const char* AppleInputDevice::sMFiProfileExtendedGamepad = "extendedGamepad";
	const char* AppleInputDevice::sMFiProfileGamepad = "gamepad";
	const char* AppleInputDevice::sMFiProfileMicroGamepad = "microGamepad";

	
	AppleInputDevice::AppleInputDevice(const InputDeviceDescriptor &descriptor)
	: Super(descriptor)
	, fConnected(InputDeviceConnectionState::kConnected)
	, fSerialNumber(nil)
	, fProductName(nil)
	, fDriverType(kDriverNone)
	, fPlayerIndex(-1)
	, fCanVibrate(false)
	, fController(0)
	, fMFIConfiguration(0)
	, fMFiProfile(NULL)
	{
		
	}
	
	
	const char* AppleInputDevice::GetProductName()
	{
		return [fProductName UTF8String];
	}
	
	const char* AppleInputDevice::GetDisplayName()
	{
		return [fProductName UTF8String];
	}
	
	const char* AppleInputDevice::GetPermanentStringId()
	{
		return [fSerialNumber UTF8String];
	}
	
	InputDeviceConnectionState AppleInputDevice::GetConnectionState()
	{
		return fConnected;
	}
	
	AppleInputDevice::~AppleInputDevice()
	{
		[fSerialNumber release];
		[fProductName release];
	}
	
	bool AppleInputDevice::CanVibrate()
	{
		return fCanVibrate;
	}

	PlatformInputAxis* AppleInputDevice::OnCreateAxisUsing(const InputAxisDescriptor &descriptor)
	{
		return Rtt_NEW(GetAllocator(), AppleInputAxis(descriptor));
	}
	
	const char* AppleInputDevice::GetDriverName()
	{
		switch(fDriverType)
		{
			case kDriverMFi:
				return "MFi";
			case kDriverHID:
				return "HID";
			case kDriverNone:
			default:
				return NULL;
		}
	}
	
	int AppleInputDevice::OnAccessingField(lua_State *L, const char fieldName[])
	{
		int result = 0;
		
		// Handle the accessed field.
		if (strcmp(fieldName, "MFiProfile") == 0)
		{
			if (fMFiProfile)
			{
				lua_pushstring(L, fMFiProfile);
			}
			else
			{
				lua_pushnil(L);
			}
			result = 1;
		}
#ifdef Rtt_TVOS_ENV
		else if (strcmp(fieldName, "reportsAbsoluteDpadValues") == 0)
		{
			if ([fController respondsToSelector:@selector(microGamepad)])
			{
				lua_pushboolean(L, fController.microGamepad.reportsAbsoluteDpadValues);
				result = 1;
			}
		}
		else if (strcmp(fieldName, "allowsRotation") == 0)
		{
			if ([fController respondsToSelector:@selector(microGamepad)])
			{
				lua_pushboolean(L, fController.microGamepad.allowsRotation);
				result = 1;
			}
		}
#endif
		else
		{
			result = Super::OnAccessingField(L, fieldName);
		}
		
		// Return the number of Lua values returned by this field.
		return result;

	}
	
	int AppleInputDevice::OnAssigningField(lua_State *L, const char fieldName[], int valueIndex)
	{
		int result = 0;
#ifdef Rtt_TVOS_ENV
		if (strcmp(fieldName, "reportsAbsoluteDpadValues") == 0)
		{
			bool val = lua_toboolean( L, valueIndex );
			fController.microGamepad.reportsAbsoluteDpadValues = val;
		}
		else if (strcmp(fieldName, "allowsRotation") == 0)
		{
			bool val = lua_toboolean( L, valueIndex );
			fController.microGamepad.allowsRotation = val;
		}
		else
#endif
		{
			result = Super::OnAssigningField(L, fieldName, valueIndex);
		}
		return result;
	}

	
	int AppleInputDevice::GetPlayerNumber()
	{
		if (fDriverType == kDriverMFi && fPlayerIndex >=0)
		{
			return fPlayerIndex+1;
		}
		return Super::GetPlayerNumber();
	}
	
	unsigned AppleInputDevice::GetMFiConfiguration(GCController *controller)
	{
		return    ((controller.extendedGamepad == nil)?1<<0:0)
				| ((controller.gamepad         == nil)?1<<1:0)
				| ((controller.motion          == nil)?1<<2:0)
#ifdef Rtt_TVOS_ENV
				| ((controller.microGamepad    == nil)?1<<3:0)
#endif
		
		;
	}
	
	bool AppleInputDevice::MatchConfiguration(GCController *other)
	{
		if (0 == fMFIConfiguration)
		{
			return false;
		}
		unsigned config = AppleInputDevice::GetMFiConfiguration(other);
		if (config == fMFIConfiguration)
		{
			if (other.vendorName == nil || [other.vendorName isEqualToString:fProductName])
			{
				return true;
			}
		}
		return false;
	}
	
	bool AppleInputDevice::MatchVendorName(NSString* vendor)
	{
		if (   ![fProductName length]
			|| ![vendor length]
			|| [fProductName isEqualToString:@"Unknown"]
			|| [vendor isEqualToString:@"Unknown"])
		{
			return false;
		}
		if ( [fProductName rangeOfString:vendor].location != NSNotFound || [vendor rangeOfString:fProductName].location != NSNotFound)
		{
			return true;
		}
		return false;
	}
	
#ifdef Rtt_APPLE_HID
	
	
	bool AppleInputDevice::CanVibrate(IOHIDDeviceRef hidDevice)
	{
		bool ret = false;
		io_service_t io_device = AllocateHIDObjectFromIOHIDDeviceRef(hidDevice);
		if (io_device)
		{
			ret = (FFIsForceFeedback( io_device ) == FF_OK);
			FreeHID_IO_Object(io_device);
		}
		return ret;
	}
	
	void AppleInputDevice::Vibrate()
	{
		if(fCanVibrate && fConnected.IsConnected())
		{
			io_service_t io_device = AllocateHIDObjectFromIOHIDDeviceRef(hidDeviceRef);
			if(!io_device)
				return;
			
			const int kVibroDuration = 180;
			
			FFDeviceObjectReference ff_device;
			FFCreateDevice(io_device, &ff_device);
			
			FFEffectObjectReference effectRef;
			
			FFEFFECT effect = {0};
			effect.dwSize = sizeof(FFEFFECT);
			
			FFCONSTANTFORCE force;
			force.lMagnitude = 10000;
			
			FFCAPABILITIES capabs = {0};
			FFDeviceGetForceFeedbackCapabilities(ff_device, &capabs);
			
			DWORD *axes = new DWORD[capabs.numFfAxes];
			LONG *directions = new LONG[capabs.numFfAxes];
			
			for(UInt32 i=0; i<capabs.numFfAxes; i++)
			{
				axes[i] = capabs.ffAxes[i];
			}
			
			effect.cAxes = capabs.numFfAxes;
			effect.rglDirection = directions;
			effect.rgdwAxes = axes;
			effect.dwSamplePeriod = 0;
			effect.dwGain = 10000;
			effect.dwFlags = FFEFF_OBJECTOFFSETS | FFEFF_SPHERICAL;
			effect.dwDuration = kVibroDuration*1000;
			effect.lpEnvelope = NULL;
			
			
			effect.cbTypeSpecificParams = sizeof(FFCONSTANTFORCE);
			effect.lpvTypeSpecificParams = &force;
			
			if(FFDeviceCreateEffect(ff_device, kFFEffectType_ConstantForce_ID, &effect, &effectRef) == FF_OK)
			{
				FFEffectStart(effectRef, 1, 0);
				
				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, kVibroDuration * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
					FFDeviceReleaseEffect(ff_device, effectRef);
					FFReleaseDevice(ff_device);
				});
			}
			else
			{
				FFReleaseDevice(ff_device);
			}
			
			delete [] axes;
			delete [] directions;
		}
	}
	
	
	
	io_service_t AppleInputDevice::AllocateHIDObjectFromIOHIDDeviceRef(IOHIDDeviceRef inIOHIDDeviceRef)
	{
		io_service_t result = 0L;
		if (inIOHIDDeviceRef) {
			// Set up the matching criteria for the devices we're interested in.
			// We are interested in instances of class IOHIDDevice.
			// matchingDict is consumed below(in IOServiceGetMatchingService)
			// so we have no leak here.
			CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOHIDDeviceKey);
			if (matchingDict) {
				// Add a key for locationID to our matching dictionary.  This works for matching to
				// IOHIDDevices, so we will only look for a device attached to that particular port
				// on the machine.
				CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR(kIOHIDLocationIDKey));
				if (tCFTypeRef) {
					CFDictionaryAddValue(matchingDict, CFSTR(kIOHIDLocationIDKey), tCFTypeRef);
					
					// IOServiceGetMatchingService assumes that we already know that there is only one device
					// that matches.  This way we don't have to do the whole iteration dance to look at each
					// device that matches.  This is a new API in 10.2
					result = IOServiceGetMatchingService(kIOMasterPortDefault, matchingDict);
					// (matchingDict is released by IOServiceGetMatchingServices)
				} else {
					CFRelease(matchingDict);
				}
			}
		}
		
		return (result);
	} // AllocateHIDObjectFromIOHIDDeviceRef
	
	bool AppleInputDevice::FreeHID_IO_Object(io_service_t inHIDObject)
	{
		kern_return_t kr;
		
		kr = IOObjectRelease(inHIDObject);
		
		return (kIOReturnSuccess == kr);
	} // FreeHIDObject
	
	NSString* AppleInputDevice::GetSerialNumber(IOHIDDeviceRef inIOHIDDeviceRef)
	{
		NSString* ret = (NSString*)IOHIDDeviceGetProperty( inIOHIDDeviceRef, CFSTR(kIOHIDSerialNumberKey) );
		if ( nil == ret )
		{
			//Sony seems not to care about reporting Serial Numbers.
			NSNumber* location = (NSNumber*)IOHIDDeviceGetProperty( inIOHIDDeviceRef, CFSTR(kIOHIDLocationIDKey) );
			if( location )
			{
				ret = [NSString stringWithFormat:@"%ld", [location longValue]];
			}
		}
		if ( nil == ret )
		{
			//location should be always provided by system
			ret = @"Joystick";
		}
		return ret;
	}

	NSString* AppleInputDevice::GetSerialNumber(GCController* controller)
	{
//		TODO: Uncomment/replace it with another logic to make connections between HID and MFi controllers
//		SEL s = NSSelectorFromString(@"deviceRef");
//		if ([controller respondsToSelector:s]) {
//			IOHIDDeviceRef r = (IOHIDDeviceRef)[controller performSelector:s];
//			if (r)
//			{
//				return GetSerialNumber((IOHIDDeviceRef)[controller performSelector:s]);
//			}
//		}
		return nil;
	}


#else // Rtt_APPLE_HID not defined
	
	NSString* AppleInputDevice::GetSerialNumber(GCController* controller)
	{
		return nil;
	}
	
	void AppleInputDevice::Vibrate()
	{
	}
	
#endif // Rtt_APPLE_HID end

	
	AppleInputAxis::AppleInputAxis(const InputAxisDescriptor &descriptor)
	: PlatformInputAxis(descriptor)
	, centerPoint0(true)
	{
	}
	
	
	Rtt_Real AppleInputAxis::GetNormalizedValue(Rtt_Real rawValue)
	{
		if (centerPoint0)
		{
			return PlatformInputAxis::GetNormalizedValue(rawValue);
		}
		Rtt_Real physicalMax   = GetMaxValue();
		Rtt_Real physicalMin   = GetMinValue();
		Rtt_Real scaledMin     = Rtt_REAL_NEG_1;
		Rtt_Real scaledMax     = Rtt_REAL_1;
		Rtt_Real ret   = Rtt_RealDiv(Rtt_RealMul(rawValue - physicalMin, scaledMax - scaledMin) , physicalMax - physicalMin) + scaledMin;
		ret = Clamp( ret, Rtt_REAL_NEG_1, Rtt_REAL_1 );
		return ret;
	}
	

	
}