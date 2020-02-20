//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_AppleInputMFiDeviceListener__
#define __ratatouille__Rtt_AppleInputMFiDeviceListener__

namespace Rtt
{
	class Runtime;
	class AppleInputDeviceManager;
}

@interface AppleInputMFiDeviceListener: NSObject

- (instancetype)initWithRuntime:(Rtt::Runtime*)runtime andDeviceManager:(Rtt::AppleInputDeviceManager*)devManager;

-(void)start;
-(void)stop;

@property (assign, nonatomic) Rtt::Runtime* runtime;
@property (assign, nonatomic) Rtt::AppleInputDeviceManager* dm;

@end

#endif /* defined(__ratatouille__Rtt_AppleInputMFiDeviceListener__) */
