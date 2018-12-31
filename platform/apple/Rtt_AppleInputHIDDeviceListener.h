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
