//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_AppleInputHIDDeviceListener__
#define __ratatouille__Rtt_AppleInputHIDDeviceListener__

#include "Rtt_Config.h"
#include "Rtt_Types.h"

#ifndef Rtt_APPLE_HID
	#error "Rtt_APPLE_HID must be defined to use this class"
#endif

#import <IOKit/hid/IOHIDManager.h>


namespace Rtt
{
	class AppleInputDeviceManager;
	class Runtime;
	class AppleInputDevice;
	class InputAxisType;
	
	class AppleInputHIDDeviceListener
	{
	public:
		typedef AppleInputHIDDeviceListener Self;
	public:
		AppleInputHIDDeviceListener();
		void StartListening(Runtime*, AppleInputDeviceManager*);
		void StopListening();
		
	private:
		Runtime *fRuntime;
		AppleInputDeviceManager *fDeviceManager;
		IOHIDManagerRef fHidManager;
		
		//Helpers
		void RegisterDevice(IOHIDDeviceRef inIOHIDDeviceRef, bool dispatchConnectedEvent);
		
		//Callbacks
		static void RegisterMatchingDevice(void* context, IOReturn, void*, IOHIDDeviceRef);
		static void RegisterDeviceRemoval(void* context, IOReturn, void*, IOHIDDeviceRef);
		static void Handle_IOHIDInputValueCallback(void *, IOReturn, void *, IOHIDValueRef);
		
		static bool AddAxis(AppleInputDevice * devicePointer, IOHIDElementRef elementRef, S64 deviceDescriptorId, InputAxisType axisType );
	};
	
} // namespace Rtt


#endif /* defined(__ratatouille__Rtt_AppleInputHIDDeviceListener__) */
