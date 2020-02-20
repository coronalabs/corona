//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleCallback_H__
#define _Rtt_AppleCallback_H__

#import <Foundation/NSObject.h>

// ----------------------------------------------------------------------------

@class NSNotification;

namespace Rtt
{
	class MCallback;
}

// ----------------------------------------------------------------------------

// Obj-C wrapper for Rtt::MCallback

@interface AppleCallback : NSObject
{
	Rtt::MCallback* callback;
}

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
@property (nonatomic) Rtt::MCallback *callback;
#else
- (void)setCallback:(Rtt::MCallback*)newValue;
#endif

- (void)invoke:(id)sender;

@end

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleCallback_H__
