//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "AppleWeakProxy.h"

// ----------------------------------------------------------------------------

@interface AppleWeakProxy()

@property (nonatomic, assign) id target; // Weak reference

@end

// ----------------------------------------------------------------------------

@implementation AppleWeakProxy

- (instancetype)initWithTarget:(id)aTarget
{
	self = [super init];
	if ( self )
	{
		_target = aTarget;
	}
	return self;
}

- (void)invalidate
{
	self.target = nil;
}

- (BOOL)conformsToProtocol:(Protocol *)aProtocol
{
	return [self.target conformsToProtocol:aProtocol];
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
	return [self.target respondsToSelector:aSelector];
}

// According to Apple docs, this is much less expensive
// than overriding forwardInvocation:
- (id)forwardingTargetForSelector:(SEL)aSelector
{
	return self.target;
}

//- (void)forwardInvocation:(NSInvocation *)invocation
//{
//	if ( [self.owner respondsToSelector:[invocation selector]] )
//	{
//		[invocation invokeWithTarget:self.owner];
//	}
//	else
//	{
//		[super forwardInvocation:invocation];
//	}
//}

@end

// ----------------------------------------------------------------------------
