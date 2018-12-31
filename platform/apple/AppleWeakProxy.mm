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
