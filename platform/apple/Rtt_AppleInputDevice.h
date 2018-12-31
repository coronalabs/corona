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

#ifndef __ratatouille__Rtt_AppleInputDevice__
#define __ratatouille__Rtt_AppleInputDevice__

#include "Input/Rtt_PlatformInputDeviceManager.h"

#ifdef Rtt_APPLE_HID
	#import <IOKit/hid/IOHIDManager.h>
#endif

@class GCController;

namespace Rtt
{
	
	class AppleInputDevice : public PlatformInputDevice
	{
	public:
		typedef PlatformInputDevice Super;

		enum DriverType {
			kDriverNone,
			kDriverHID,
			kDriverMFi
		} fDriverType;

		InputDeviceConnectionState fConnected;
		int fPlayerIndex;
		NSString *fSerialNumber;
		NSString *fProductName;
		const char* fMFiProfile;
		
		bool fCanVibrate;
		
		GCController* fController;
		unsigned int fMFIConfiguration; // For guessing reconnecting MFi devices
		
	public:
		AppleInputDevice(const InputDeviceDescriptor &descriptor);
				
		virtual const char* GetProductName() override;
		virtual const char* GetDisplayName() override;
		virtual const char* GetPermanentStringId() override;
		virtual InputDeviceConnectionState GetConnectionState() override;
		virtual void Vibrate() override;
		virtual bool CanVibrate() override;
		virtual const char* GetDriverName() override;
		PlatformInputAxis* OnCreateAxisUsing(const InputAxisDescriptor &descriptor) override;
		virtual int GetPlayerNumber() override;

		virtual ~AppleInputDevice();
		
		static NSString* GetSerialNumber(GCController* controller);
		static unsigned GetMFiConfiguration(GCController *controller);
		
		static const char* sMFiProfileExtendedGamepad;
		static const char* sMFiProfileGamepad;
		static const char* sMFiProfileMicroGamepad;
		
		bool MatchConfiguration(GCController* other);
		bool MatchVendorName(NSString* vendor);
	protected:
		virtual int OnAccessingField(lua_State *L, const char fieldName[]) override;
		virtual int OnAssigningField(lua_State *L, const char fieldName[], int valueIndex) override;

		
#ifdef Rtt_APPLE_HID

	public:
		IOHIDDeviceRef hidDeviceRef; //for force feedback
	private:
		static bool FreeHID_IO_Object(io_service_t inHIDObject);
		static io_service_t AllocateHIDObjectFromIOHIDDeviceRef(IOHIDDeviceRef inIOHIDDeviceRef);
	public:
		static bool CanVibrate(IOHIDDeviceRef hidDevice);
		static NSString* GetSerialNumber(IOHIDDeviceRef inIOHIDDeviceRef);
		
#endif
	};
	
	class AppleInputAxis : public PlatformInputAxis
	{
	public:
		bool centerPoint0;

		AppleInputAxis(const InputAxisDescriptor &descriptor);
		virtual ~AppleInputAxis(){};
		
		virtual Rtt_Real GetNormalizedValue(Rtt_Real rawValue) override;
	};
	
} // namespace Rtt


#endif /* defined(__ratatouille__Rtt_AppleInputDevice__) */
