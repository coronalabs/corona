//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
