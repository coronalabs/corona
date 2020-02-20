//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/NSObject.h>

// 
// AppleWeakProxy
// 
// AppleWeakProxy allows you to forward invocations to a target object.
// Also overrides 'conformsToProtocol:' and 'respondsToSelector:'
// so the proxy behaves like it's the target.
// 
@interface AppleWeakProxy : NSObject

- (instancetype)initWithTarget:(id)aTarget;

// Invalidates this proxy, so future messages are sent to nil.
// Useful when you know it's no longer safe to message the target.
- (void)invalidate;

@end


// ----------------------------------------------------------------------------
