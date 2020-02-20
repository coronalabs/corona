//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>
#import <ForceFeedback/ForceFeedback.h>

#include "Rtt_AppleInputHIDDeviceListener.h"
#include "Rtt_AppleInputDeviceManager.h"
#include "Rtt_AppleInputDevice.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Rtt_KeyName.h"

namespace Rtt {
	
	
	AppleInputHIDDeviceListener::AppleInputHIDDeviceListener()
	: fHidManager(0)
	{
		
	}
	
	
	void AppleInputHIDDeviceListener::StartListening(Runtime* runtime, AppleInputDeviceManager* deviceManager)
    {
		if ( fHidManager )
		{
			StopListening();
		}
		
		Rtt_ASSERT(runtime && deviceManager);
		fRuntime = runtime;
		fDeviceManager = deviceManager;
		
        fHidManager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		
		//We are interested only in Joysticks and Gamepads. We would treat everything as Joystick. See: RegisterMatchingDevice
        NSArray *matchingDevices = @[
                                     @{
                                         @kIOHIDDeviceUsageKey : [NSNumber numberWithInteger:kHIDUsage_GD_Joystick],
                                         @kIOHIDDeviceUsagePageKey : [NSNumber numberWithInteger:kHIDPage_GenericDesktop]
                                     },
                                     @{
                                         @kIOHIDDeviceUsageKey : [NSNumber numberWithInteger:kHIDUsage_GD_GamePad],
                                         @kIOHIDDeviceUsagePageKey : [NSNumber numberWithInteger:kHIDPage_GenericDesktop]
                                     },
                                  ];
        // Pass NULL to get all devices
        IOHIDManagerSetDeviceMatchingMultiple(fHidManager, (CFArrayRef)matchingDevices);
        
        IOHIDManagerRegisterDeviceMatchingCallback(fHidManager, RegisterMatchingDevice, this);
        IOHIDManagerRegisterDeviceRemovalCallback(fHidManager, RegisterDeviceRemoval, this);
        
        IOHIDManagerScheduleWithRunLoop(fHidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
        IOHIDManagerOpen(fHidManager, kIOHIDOptionsTypeNone);

        IOHIDManagerRegisterInputValueCallback( fHidManager, Handle_IOHIDInputValueCallback, this );
		
		//Fill initial devices list. This is hack unfortunatelly caused by Current Run Loop no ticking between callig this code and main.lua
		NSSet* devices = (NSSet*) IOHIDManagerCopyDevices(fHidManager);
		for (id device in devices)
		{
			RegisterDevice((IOHIDDeviceRef) device, false);
		}
		[devices release];
    }
	
	
	void AppleInputHIDDeviceListener::StopListening()
    {
		if(fHidManager)
		{
			IOHIDManagerUnscheduleFromRunLoop(fHidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
			
			IOHIDManagerRegisterDeviceMatchingCallback(fHidManager, NULL, 0);
			IOHIDManagerRegisterDeviceRemovalCallback(fHidManager, NULL, 0);
			
			IOHIDManagerClose(fHidManager, kIOHIDOptionsTypeNone);
			CFRelease( fHidManager );
			fHidManager = 0;
		}		
    }
	
	void AddHatSwitchAxis(AppleInputDevice * devicePointer, IOHIDElementRef elementRef, S64 deviceDescriptorId )
	{
		PlatformInputAxis * axisX = devicePointer->AddAxis();
		axisX->SetType( InputAxisType::kHatX );
		PlatformInputAxis * axisY = devicePointer->AddAxis();
		axisY->SetType( InputAxisType::kHatY );

		NSArray *val = @[ [NSNumber numberWithLong:deviceDescriptorId]
						 ,[NSNumber numberWithInt:axisX->GetDescriptor().GetAxisNumber()]
						 ,[NSNumber numberWithInt:axisY->GetDescriptor().GetAxisNumber()]];
		IOHIDElementSetProperty(elementRef, CFSTR(kIOHIDElementCookieKey), (CFTypeRef)val);
	}

	
	bool AppleInputHIDDeviceListener::AddAxis(AppleInputDevice * devicePointer, IOHIDElementRef elementRef, S64 deviceDescriptorId, InputAxisType axisType )
	{
		PlatformInputAxis * axis = NULL;
		
		axis = devicePointer->AddAxis();
		axis->SetType(axisType);
		
		float physicalMax = IOHIDElementGetPhysicalMax( elementRef );
		float physicalMin = IOHIDElementGetPhysicalMin( elementRef );
		bool absolute = ! IOHIDElementIsRelative( elementRef );
		
		axis->SetMinValue(physicalMin);
		axis->SetMaxValue(physicalMax);
		axis->SetIsAbsolute(absolute);
		
		// TODO: find a (better) way to do this
		// axis->SetAccuracy( 0.01 );
		
		//bind axis ID and device ID to device element
		NSArray *val = @[[NSNumber numberWithLong:deviceDescriptorId] ,[NSNumber numberWithInt:axis->GetDescriptor().GetAxisNumber()] ];
		IOHIDElementSetProperty(elementRef, CFSTR(kIOHIDElementCookieKey), (CFTypeRef)val);

		return physicalMax<0 || physicalMin<0;
	}
	
	void AppleInputHIDDeviceListener::RegisterDevice(IOHIDDeviceRef inIOHIDDeviceRef, bool dispatchConnectedEvent)
	{
		InputDeviceType kInputDeviceType = InputDeviceType::kJoystick;
		// All our devices are joysticks for weird compatibility with android issues.
		// May be in a future uncomment this lines to distinguish between Joysticks and Gamepads
		// if( IOHIDDeviceConformsTo( inIOHIDDeviceRef, kHIDPage_GenericDesktop, kHIDUsage_GD_Gamepad ) )
		// kInputDeviceType = InputDeviceType::kGamepad;
		
		NSString* serialNumber = AppleInputDevice::GetSerialNumber(inIOHIDDeviceRef);
		Rtt_ASSERT(serialNumber);
		
		AppleInputDevice *devicePointer = fDeviceManager->GetBySerialNumber(serialNumber);
		S64 deviceDescriptorId = 0;
		
		
		// Attempt to see if there is MFi device registered with same Vendor name already.
		NSString* product = (NSString*)IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR(kIOHIDProductKey));
		NSString* manufacturer = (NSString*) IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR( kIOHIDManufacturerKey ) );
		NSString* productName;
		if (product == nil || [product isEqualToString:@"Unknown"])
		{
			productName = [NSString stringWithFormat:@"%@", manufacturer];
		}
		else if (manufacturer == nil || [manufacturer isEqualToString:@"Unknown"])
		{
			productName = [NSString stringWithFormat:@"%@", product];
		}
		else
		{
			productName = [NSString stringWithFormat:@"%@ %@", manufacturer, product];
		}
		
		if(devicePointer == NULL)
		{
			devicePointer = fDeviceManager->GetByVendorName(productName, AppleInputDevice::kDriverMFi);
		}
		
		
		//if device was not yet registered in device manager, create a new one
		if(devicePointer == NULL)
		{
			fDeviceManager->fCurrentDriverType = AppleInputDevice::kDriverHID;
			devicePointer = static_cast<AppleInputDevice*>( fDeviceManager->Add(kInputDeviceType) );
			devicePointer->fSerialNumber = [[NSString alloc] initWithString:serialNumber];
			
			Rtt_ASSERT( devicePointer );
		}
		else if( AppleInputDevice::kDriverHID != devicePointer->fDriverType )
		{
			//This device is already handled by MFi driver.
			return;
		}
		else if( devicePointer->GetConnectionState().IsConnected() )
		{
			//avoid duplicating messages, for consistancy. Existing devices are not reported on android
			dispatchConnectedEvent = false;
		}
		
		devicePointer->fCanVibrate = AppleInputDevice::CanVibrate(inIOHIDDeviceRef);
		devicePointer->hidDeviceRef = inIOHIDDeviceRef;
		
		//Set device properties
		devicePointer->fConnected = InputDeviceConnectionState::kConnected;
		if(product && devicePointer->fProductName == NULL)
		{
			devicePointer->fProductName = [productName retain];
		}
		
		
		//Setting up axis
		devicePointer->RemoveAllAxes();
		deviceDescriptorId = devicePointer->GetDescriptor().GetIntegerId();
		bool axesNeutral0 = false; //commented below
		
		NSArray *elements = (NSArray*)IOHIDDeviceCopyMatchingElements( inIOHIDDeviceRef, NULL, kIOHIDOptionsTypeNone );
		for (id element in elements)
		{
			IOHIDElementRef elementRef = (IOHIDElementRef)element;
			switch(IOHIDElementGetType( elementRef ))
			{
				case kIOHIDElementTypeInput_Misc:
				{
					uint usage = IOHIDElementGetUsage( elementRef );
					switch (usage) {
						case kHIDUsage_GD_X:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kX ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Y:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kY ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Z:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kZ ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Rx:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kRotationX ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Ry:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kRotationY ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Rz:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kRotationZ ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Wheel:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kWheel ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Vx:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kGeneric1 ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Vy:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kGeneric2 ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Vz:
							axesNeutral0 = AddAxis( devicePointer, elementRef, deviceDescriptorId, InputAxisType::kGeneric3 ) || axesNeutral0;
							break;
						case kHIDUsage_GD_Hatswitch:
							AddHatSwitchAxis( devicePointer, elementRef, deviceDescriptorId );
							break;
						default:
							break;
					}
				}
					break;
				case kIOHIDElementTypeInput_Button:
				{
					NSNumber *val = [NSNumber numberWithLong:deviceDescriptorId];
					IOHIDElementSetProperty(elementRef, CFSTR(kIOHIDElementCookieKey), (CFTypeRef)val);
				}
					break;
				default:
					break;
			}
			
			
		}
		[elements release];
		
		// HACK: most of devices have bad habbit of reporting all axes from 0 to 255. It means that
		// neutral position is 0.5, which is not nice.
		// So, if any of the axes reports it's minimal position as negative, we assume that device knows where it's neutral position is
		// and reports it as 0. So we would set old way of scaling for all axis on device
		if (!axesNeutral0)
		{
			const ReadOnlyInputAxisCollection& axes = devicePointer->GetAxes();
			for (S32 index = axes.GetCount() - 1; index >= 0; index--)
			{
				AppleInputAxis* axis = (AppleInputAxis*)axes.GetByIndex(index);
				if(axis)
				{
					axis->centerPoint0 = false;
				}
			}
		}
		
		if(dispatchConnectedEvent)
		{
			InputDeviceStatusEvent event(devicePointer, true, false);
			fRuntime->DispatchEvent(event);
		}
	}
	
    void AppleInputHIDDeviceListener::RegisterMatchingDevice(void *inContext,
                                                       IOReturn inResult,
                                                       void *inSender,
                                                       IOHIDDeviceRef inIOHIDDeviceRef)
    {
        Self* manager = (Self*)inContext;
        Rtt_ASSERT(manager->fHidManager == inSender);
		manager->RegisterDevice(inIOHIDDeviceRef, true);
	}
    
    
    
    ////////////////////////////////////////////////////////////
    void AppleInputHIDDeviceListener::RegisterDeviceRemoval(void *inContext,
                                                      IOReturn inResult,
                                                      void *inSender,
                                                      IOHIDDeviceRef inIOHIDDeviceRef)
    {
        Self* manager = (Self*)inContext;
        Rtt_ASSERT(manager->fHidManager == inSender);

		NSString* serialNumber = AppleInputDevice::GetSerialNumber(inIOHIDDeviceRef);
		
		Rtt_ASSERT(serialNumber);
		
		AppleInputDevice *devicePointer = NULL;
		devicePointer = manager->fDeviceManager->GetBySerialNumber(serialNumber);
		
		//Set device properties
		if( devicePointer && devicePointer->fDriverType == AppleInputDevice::kDriverHID )
		{
			devicePointer->fConnected = InputDeviceConnectionState::kDisconnected;
			
			InputDeviceStatusEvent event(devicePointer, true, false);
			manager->fRuntime->DispatchEvent(event);
		}
		
	}
	
	
    
	void AppleInputHIDDeviceListener::Handle_IOHIDInputValueCallback(
																   void *          inContext,      // context from IOHIDManagerRegisterInputValueCallback
																   IOReturn        inResult,       // completion result for the input value operation
																   void *          inSender,       // the IOHIDManagerRef
																   IOHIDValueRef   inIOHIDValueRef // the new element value
	)
	{
		Self* manager = (Self*)inContext;
		if( !manager->fRuntime )
			return;
		
		IOHIDElementRef element = IOHIDValueGetElement( inIOHIDValueRef );
		
		switch (IOHIDElementGetType(element))
		{
			case kIOHIDElementTypeInput_Misc:
			{
				NSArray *data = (NSArray *)IOHIDElementGetProperty( element, CFSTR(kIOHIDElementCookieKey) );
				if( data && [data isKindOfClass:[NSArray class]])
				{
					NSNumber *deviceValue = [data objectAtIndex:0];
					NSNumber *axisValue = [data objectAtIndex:1];
					AppleInputDevice *device = (AppleInputDevice*)manager->fDeviceManager->GetDevices().GetByDescriptorId(deviceValue.longValue);
					if ( device && device->fDriverType == AppleInputDevice::kDriverHID )
					{
						PlatformInputAxis *axis = device->GetAxes().GetByAxisNumber(axisValue.intValue);
						if ( axis )
						{
							if( axis->GetType() == InputAxisType::kHatX )
							{
								double_t angle = IOHIDValueGetScaledValue( inIOHIDValueRef, kIOHIDValueScaleTypePhysical );
								double valueX = 0, valueY = 0;
								if ( angle>=0 && angle<360 )
								{
									valueX =  sin( 2*M_PI*angle/360 );
									valueY = -cos( 2*M_PI*angle/360 );
								}
								
								
								PlatformInputAxis *axisY = device->GetAxes().GetByAxisNumber( [[data objectAtIndex:2] intValue] );
								if( axisY && axisY->GetType() == InputAxisType::kHatY )
								{
									AxisEvent eventX(device, axis, valueX);
									manager->fRuntime->DispatchEvent(eventX);
									
									AxisEvent eventY(device, axisY, valueY);
									manager->fRuntime->DispatchEvent(eventY);
								}
							}
							else
							{
								double_t value = IOHIDValueGetScaledValue( inIOHIDValueRef, kIOHIDValueScaleTypePhysical );
								AxisEvent event(device, axis, value);
								manager->fRuntime->DispatchEvent(event);
							}

						}
					}
				}
			}
			break;
			case kIOHIDElementTypeInput_Button:
			{
				unsigned int key = IOHIDElementGetUsage( element ) - 1;
				KeyEvent::Phase phase = (double)IOHIDValueGetIntegerValue(inIOHIDValueRef)>0 ? KeyEvent::kDown : KeyEvent::kUp;
				
				//this part is sketcy. I'm not really sure how to tell which button is it...
				const char * keyNames[] = { KeyName::kButton1, KeyName::kButton2, KeyName::kButton3, KeyName::kButton4, KeyName::kButton5, KeyName::kButton6, KeyName::kButton7, KeyName::kButton8, KeyName::kButton9, KeyName::kButton10, KeyName::kButton11, KeyName::kButton12, KeyName::kButton13, KeyName::kButton14, KeyName::kButton15, KeyName::kButton16 };

				char deviceName[25];
				if ( key < (sizeof(keyNames)/sizeof(keyNames[0])) )
				{
					strncpy(deviceName, keyNames[key], 25);
				}
				else
				{
					snprintf(deviceName, 25, "button%d", key+1);
				}
				
				NSNumber *deviceValue = (NSNumber *)IOHIDElementGetProperty( element, CFSTR(kIOHIDElementCookieKey) );
				if( deviceValue && [deviceValue isKindOfClass:[NSNumber class]] )
				{
					AppleInputDevice *device = (AppleInputDevice*)manager->fDeviceManager->GetDevices().GetByDescriptorId(deviceValue.longValue);

					if (device->fDriverType == AppleInputDevice::kDriverHID)
					{
						// 188 - copied from android. Joystick buttons should not use KeyCodes, but it is required. So joystick keycodes would
						// start from 188 as they do on Android.
						KeyEvent event(device, phase, deviceName, 188+key%100, false, false, false, false);
						manager->fRuntime->DispatchEvent(event);
					}
				}
			}
				break;
				
			default:
				break;
		}
		
	}
	

}
