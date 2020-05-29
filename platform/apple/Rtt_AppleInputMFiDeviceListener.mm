//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>
#import <GameController/GameController.h>

#include "Rtt_AppleInputMFiDeviceListener.h"
#include "Rtt_AppleInputDeviceManager.h"

#include "Rtt_Event.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Runtime.h"
#include "Rtt_KeyName.h"


using namespace Rtt;

@implementation AppleInputMFiDeviceListener

- (instancetype)initWithRuntime:(Rtt::Runtime*)runtime andDeviceManager:(Rtt::AppleInputDeviceManager*)devManager
{
	self = [super init];
	if (self) {
		self.runtime = runtime;
		self.dm = devManager;
	}
	return self;
}

#define MFi_KEY_CODE_MENU 1
#define MFi_KEY_CODE_A 2
#define MFi_KEY_CODE_B 3
#define MFi_KEY_CODE_X 4
#define MFi_KEY_CODE_Y 5
#define MFi_KEY_CODE_L1 6
#define MFi_KEY_CODE_L2 7
#define MFi_KEY_CODE_R1 8
#define MFi_KEY_CODE_R2 9
#define MFi_KEY_CODE_LEFT 10
#define MFi_KEY_CODE_UP 11
#define MFi_KEY_CODE_RIGHT 12
#define MFi_KEY_CODE_DOWN 13


-(void)connectedController:(GCController*)controller andSendEvent:(BOOL)sendEvent
{	
	AppleInputDevice* devicePointer = self.dm->GetByMFiDevice(controller);
	bool reconfigured = false;
	
	bool reassignPlayerNumber = false;
	
	if ( devicePointer == NULL )
	{
		self.dm->fCurrentDriverType = AppleInputDevice::kDriverMFi;
		devicePointer = static_cast<AppleInputDevice*>( self.dm->Add( InputDeviceType::kGamepad ) );
		reassignPlayerNumber = true;
	}
	else if (devicePointer->fDriverType == AppleInputDevice::kDriverHID)
	{
		devicePointer->fDriverType = AppleInputDevice::kDriverMFi;
		reconfigured = true;
		reassignPlayerNumber = true;
	}
	
	if (reassignPlayerNumber)
	{
		if (self.dm->fLastSeenPlayerIndex < GCControllerPlayerIndex4)
		{
			self.dm->fLastSeenPlayerIndex += 1;
			devicePointer->fPlayerIndex = (int)self.dm->fLastSeenPlayerIndex;
		}
		else
		{
			devicePointer->fPlayerIndex = (int)GCControllerPlayerIndexUnset;
		}
	}
	controller.playerIndex = (GCControllerPlayerIndex)devicePointer->fPlayerIndex;
	
	devicePointer->fController = controller;
	devicePointer->fMFIConfiguration = AppleInputDevice::GetMFiConfiguration(controller);
	
	if (devicePointer->fSerialNumber)
	{
		[devicePointer->fSerialNumber release];
	}
	devicePointer->fSerialNumber = [AppleInputDevice::GetSerialNumber(controller) copy];
	
	devicePointer->fConnected = InputDeviceConnectionState::kConnected;
	
	if (devicePointer->fProductName)
	{
		[devicePointer->fProductName release];
	}
	
	devicePointer->fProductName = [controller.vendorName copy];
	if (!devicePointer->fProductName)
	{
		devicePointer->fProductName = @"MFi Controller";
	}
	
	devicePointer->RemoveAllAxes();
	__block AppleInputMFiDeviceListener *weakSelf = self;
	
	controller.controllerPausedHandler = ^(GCController*) {
		KeyEvent event = KeyEvent(devicePointer, KeyEvent::kDown, KeyName::kMenu, MFi_KEY_CODE_MENU, false, false, false, false);
		weakSelf.runtime->DispatchEvent(event);
		[[NSOperationQueue mainQueue] addOperationWithBlock:^{
			KeyEvent event = KeyEvent(devicePointer, KeyEvent::kUp, KeyName::kMenu, MFi_KEY_CODE_MENU, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		}];
	};
	
	
	if ( controller.extendedGamepad )
	{
		devicePointer->fMFiProfile = AppleInputDevice::sMFiProfileExtendedGamepad;
		GCExtendedGamepad* gp = controller.extendedGamepad;
		
		// gamepad buttons A B X Y
		gp.buttonA.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonA, MFi_KEY_CODE_A, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.buttonB.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonB, MFi_KEY_CODE_B, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.buttonX.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonX, MFi_KEY_CODE_X, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.buttonY.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonY, MFi_KEY_CODE_Y, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		
		// Left right shoulders & triggers as buttons L1 L2 R1 R2
		gp.leftShoulder.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kLeftShoulderButton1, MFi_KEY_CODE_L1, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.leftTrigger.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kLeftShoulderButton2, MFi_KEY_CODE_L2, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.rightShoulder.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kRightShoulderButton1, MFi_KEY_CODE_R1, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.rightTrigger.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kRightShoulderButton2, MFi_KEY_CODE_R2, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		
		// D-Pad buttons
		gp.dpad.up.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kUp, MFi_KEY_CODE_UP, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.dpad.down.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kDown, MFi_KEY_CODE_DOWN, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.dpad.left.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kLeft, MFi_KEY_CODE_LEFT, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.dpad.right.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kRight, MFi_KEY_CODE_RIGHT, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		
		// Axes
		
		{ // Left X
			PlatformInputAxis * leftX = devicePointer->AddAxis();
			leftX->SetType(InputAxisType::kLeftX);
			gp.leftThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
				AxisEvent event = AxisEvent(devicePointer, leftX, value);
				weakSelf.runtime->DispatchEvent(event);
			};
		}
		
		{ // Left Y
			PlatformInputAxis * leftY = devicePointer->AddAxis();
			leftY->SetType(InputAxisType::kLeftY);
			gp.leftThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
				AxisEvent event = AxisEvent(devicePointer, leftY, -value); // Note '-'. Hardcoded for consistancy
				weakSelf.runtime->DispatchEvent(event);
			};
		}
		
		{ // Right X
			PlatformInputAxis * rightX = devicePointer->AddAxis();
			rightX->SetType(InputAxisType::kRightX);
			gp.rightThumbstick.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
				AxisEvent event = AxisEvent(devicePointer, rightX, value);
				weakSelf.runtime->DispatchEvent(event);
			};
		}
		
		{ // Right Y
			PlatformInputAxis * rightY = devicePointer->AddAxis();
			rightY->SetType(InputAxisType::kRightY);
			gp.rightThumbstick.yAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
				AxisEvent event = AxisEvent(devicePointer, rightY, -value); // Note '-'. Hardcoded for consistancy
				weakSelf.runtime->DispatchEvent(event);
			};
		}
		
		{ // Left Shoulder 2 Axis
			PlatformInputAxis * l2 = devicePointer->AddAxis();
			l2->SetType(InputAxisType::kLeftTrigger);
			gp.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput *btn, float value, BOOL pressed) {
				AxisEvent event = AxisEvent(devicePointer, l2, value);
				weakSelf.runtime->DispatchEvent(event);
			};
		}
		
		{ // Right Shoulder 2 Axis
			PlatformInputAxis * r2 = devicePointer->AddAxis();
			r2->SetType(InputAxisType::kRightTrigger);
			gp.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput *btn, float value, BOOL pressed) {
				AxisEvent event = AxisEvent(devicePointer, r2, value);
				weakSelf.runtime->DispatchEvent(event);
			};
		}
	}
	else if ( controller.gamepad )
	{
		devicePointer->fMFiProfile = AppleInputDevice::sMFiProfileGamepad;
		GCGamepad* gp = controller.gamepad;
		
		// gamepad buttons A B X Y
		gp.buttonA.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonA, MFi_KEY_CODE_A, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.buttonB.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonB, MFi_KEY_CODE_B, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.buttonX.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonX, MFi_KEY_CODE_X, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.buttonY.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonY, MFi_KEY_CODE_Y, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		
		// Left right shoulders & triggers as buttons L1 L2 R1 R2
		gp.leftShoulder.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kLeftShoulderButton1, MFi_KEY_CODE_L1, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};

		gp.rightShoulder.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kRightShoulderButton1, MFi_KEY_CODE_R1, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		
		// D-Pad buttons
		gp.dpad.up.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kUp, MFi_KEY_CODE_UP, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.dpad.down.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kDown, MFi_KEY_CODE_DOWN, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.dpad.left.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kLeft, MFi_KEY_CODE_LEFT, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		gp.dpad.right.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kRight, MFi_KEY_CODE_RIGHT, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
	}
	
#ifdef Rtt_TVOS_ENV
	else if ( controller.microGamepad )
	{
		devicePointer->fMFiProfile = AppleInputDevice::sMFiProfileMicroGamepad;
		GCMicroGamepad* gp = controller.microGamepad;

		gp.buttonA.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonA, MFi_KEY_CODE_A, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};

		gp.buttonX.pressedChangedHandler = ^(GCControllerButtonInput *button, float value, BOOL pressed) {
			KeyEvent event = KeyEvent(devicePointer, pressed?KeyEvent::kDown:KeyEvent::kUp , KeyName::kButtonX, MFi_KEY_CODE_X, false, false, false, false);
			weakSelf.runtime->DispatchEvent(event);
		};
		
		{ // Left X
			PlatformInputAxis * xAxis = devicePointer->AddAxis();
			xAxis->SetType(InputAxisType::kX);
			gp.dpad.xAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
				AxisEvent event = AxisEvent(devicePointer, xAxis, value);
				weakSelf.runtime->DispatchEvent(event);
			};
		}
		
		{ // Left Y
			PlatformInputAxis * yAxis = devicePointer->AddAxis();
			yAxis->SetType(InputAxisType::kY);
			gp.dpad.yAxis.valueChangedHandler = ^(GCControllerAxisInput *axis, float value) {
				AxisEvent event = AxisEvent(devicePointer, yAxis, -value);  // Note '-'. Hardcoded for consistancy
				weakSelf.runtime->DispatchEvent(event);
			};
		}

	}
#endif
	
	if ( [controller respondsToSelector:@selector(motion)] && [controller motion] )
	{
		// Each closure gets its own copy of these variables due to __block
		__block double lastAccelerometerTimeStamp = -1.f;
//		__block double lastGyroscopeTimeStamp = -1.f;
		__block double previousShakeTime = 0.f;
		controller.motion.valueChangedHandler = ^(GCMotion *motion)
		{
			const MPlatformDevice& device = weakSelf.runtime->Platform().GetDevice();
			if ( device.DoesNotify( MPlatformDevice::kAccelerometerEvent ) )
			{
				double gravityAccel[3] = { motion.gravity.x, motion.gravity.y, motion.gravity.z };
				double instantAccel[3] = { motion.userAcceleration.x, motion.userAcceleration.y, motion.userAcceleration.z };
				double totalAccel[3] = { gravityAccel[0] + instantAccel[0], gravityAccel[1] + instantAccel[1], gravityAccel[2] + instantAccel[2] };


				double x = instantAccel[0];
				double y = instantAccel[1];
				double z = instantAccel[2];

				// Compute the magnitude of the current acceleration
				// and if above a given threshold, it's a shake
				bool isShake = false;
				const double kShakeAccelSq = 4.0;
				double accelSq = x*x + y*y + z*z;
				CFTimeInterval t = CFAbsoluteTimeGetCurrent();
				if ( accelSq >= kShakeAccelSq )
				{
					const CFTimeInterval kMinShakeInterval = 0.5;

					isShake = ( t > previousShakeTime + kMinShakeInterval );
					if ( isShake )
					{
						previousShakeTime = t;
					}
				}

				// throw away first value since we don't have a delta
				if ( lastAccelerometerTimeStamp < 0 )
				{
					lastAccelerometerTimeStamp = t;
				}
				else
				{
					AccelerometerEvent e( gravityAccel, instantAccel, totalAccel, isShake, t - lastAccelerometerTimeStamp, devicePointer );
					weakSelf.runtime->DispatchEvent( e );
					lastAccelerometerTimeStamp = t;
				}
			}
			
//			if( device.DoesNotify( MPlatformDevice::kGyroscopeEvent) )
//			{
//				CFTimeInterval t = CFAbsoluteTimeGetCurrent();
//				
//				if ( lastGyroscopeTimeStamp < 0 )
//				{
//					lastGyroscopeTimeStamp = t;
//				}
//				else
//				{
//					// According to Documentation in GCMotion.h
//					// > Remotes can not determine a stable rotation rate so the values will be (0,0,0) at all times
//					// motion.attitude (what do we do here???)
//					Rtt::GyroscopeEvent gyroEvent( motion.rotationRate.x, motion.rotationRate.y, motion.rotationRate.z, t - lastGyroscopeTimeStamp, devicePointer );
//					weakSelf.runtime->DispatchEvent( gyroEvent );
//					lastGyroscopeTimeStamp = t;
//				}
//			}
		};
	}
	
	if(sendEvent)
	{
		InputDeviceStatusEvent connectedEvent(devicePointer, !reconfigured, reconfigured);
		self.runtime->DispatchEvent(connectedEvent);
	}
}

-(void)controllerConnectedWithNote:(NSNotification*)note
{
	GCController* controller = (GCController*)note.object;
	[self connectedController:controller andSendEvent:YES];
}

-(void)controllerDisconnectedWithNote:(NSNotification*)note
{
	GCController* controller = note.object;
	AppleInputDevice* dp = self.dm->GetByMFiDevice(controller);
	
	if ( dp != NULL )
	{
		dp->fConnected = InputDeviceConnectionState::kDisconnected;
		dp->fController = nil; // after disconnecting this pointer is irrelevant

		InputDeviceStatusEvent connectedEvent(dp, true, false);
		self.runtime->DispatchEvent(connectedEvent);
	}
}


-(void)start
{
	if (![GCController class])
	{
		return;
	}
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controllerConnectedWithNote:) name:GCControllerDidConnectNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(controllerDisconnectedWithNote:) name:GCControllerDidDisconnectNotification object:nil];
	for (GCController* c in [GCController controllers]) {
		[self connectedController:c andSendEvent:NO];
	}
}

-(void)stop
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end

