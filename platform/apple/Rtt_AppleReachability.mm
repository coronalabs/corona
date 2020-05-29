//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleReachability.h"

#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "Reachability.h"

#import <Foundation/Foundation.h>
#include <arpa/inet.h> // inet_pton and other Unix IPv4/6 functions

// ----------------------------------------------------------------------------

namespace Rtt
{
	class AppleReachability;
}

//Called by Reachability whenever status changes.
// ----------------------------------------------------------------------------
@interface ReachabilityCallbackDelegate : NSObject
{
	Rtt::AppleReachability* networkReachability;
	BOOL hasInvokedFirstCallback;
}

@property(nonatomic, assign) Rtt::AppleReachability* networkReachability;
- (void) reachabilityChanged:(NSNotification*)the_notification;
- (void) invokeChangeCallback;
- (void) invokeFirstTimeChangeCallbackIfNecessary;
@end

@implementation ReachabilityCallbackDelegate
@synthesize networkReachability;


- (void) invokeChangeCallback
{
	using namespace Rtt;
	Rtt::AppleReachability* reachability = [self networkReachability];

	Rtt_ASSERT( reachability != NULL );
	reachability->InvokeCallback();
	hasInvokedFirstCallback = YES;	
}

- (void) reachabilityChanged:(NSNotification*)the_notification
{
#ifdef Rtt_DEBUG
	Rtt_Reachability* reach = [the_notification object];
	NSParameterAssert([reach isKindOfClass:[Rtt_Reachability class]]);
#endif
	
	[self invokeChangeCallback];
}

- (void) invokeFirstTimeChangeCallbackIfNecessary
{
	if ( NO == hasInvokedFirstCallback )
	{
		[self invokeChangeCallback];
	}
}

/*
*/
@end

// ----------------------------------------------------------------------------

namespace Rtt
{
	
	// ----------------------------------------------------------------------------
	
	AppleReachability::AppleReachability( const ResourceHandle<lua_State> & handle, PlatformReachabilityType type, const char* address  )
	:	PlatformReachability( handle, type, address ),
		networkReachability( nil ),
		reachabilityCallbackDelegate( nil ),
		fAllocator( NULL )
	{
		fAllocator = LuaContext::GetAllocator( handle.Dereference() );

		
		if ( ( NULL == address ) || ( address[0] == '\0' ) )
		{
			return;
		}
		
		// Could be a human readable name, or an IP address. If an IP address, it can be IPv4 or IPv6
		struct sockaddr_in6 sa_for_ipv6;
		struct sockaddr_in sa_for_ipv4;
		int error_val;
		memset(&sa_for_ipv6, 0, sizeof(sa_for_ipv6)); 
		memset(&sa_for_ipv4, 0, sizeof(sa_for_ipv4)); 
		sa_for_ipv6.sin6_len = sizeof(sa_for_ipv6);
		sa_for_ipv4.sin_len = sizeof(sa_for_ipv4);
		sa_for_ipv6.sin6_family=AF_INET6;
		sa_for_ipv4.sin_family=AF_INET;

		// Test for IPv6 address
		error_val = inet_pton(AF_INET6, address, &(sa_for_ipv6.sin6_addr));
		if ( error_val > 0 )
		{
			// FIXME: Apple doesn't say how to pass a IPv6 structure to their API or if it is even supported.
			// DDGReachability also has obvious bugs that make it clear it doesn't support IPv6.
			// I tried some quick workarounds, but I always get back isReachable=false, isConnectionRequired=true.
			networkReachability = [Rtt_Reachability reachabilityWithAddress:(const struct sockaddr_in *)(&(sa_for_ipv6))];
		}
		else
		{
			// Test for IPv4
			error_val = inet_pton(AF_INET, address, &(sa_for_ipv4.sin_addr));
			if ( error_val > 0 )
			{
				networkReachability = [Rtt_Reachability reachabilityWithAddress:(const struct sockaddr_in *)(&(sa_for_ipv4))];
			}
			else
			{
				// Assuming human readable host name
				networkReachability = [Rtt_Reachability reachabilityWithHostName:[NSString stringWithUTF8String:address]];
			}

		}
		
		// Bug 5986: Reachability allocation can fail and return nil. Since we are in a constructor and turned off exception handling, we need a way to elegantly handle failure conditions. This was caused by passing in a string of "" (instead of nil), but in theory other things can break this API.
		if ( nil == networkReachability )
		{
			return;
		}
		networkReachability.originalKey = [NSString stringWithUTF8String:address];
		
		CFRetain( networkReachability );
		
		
		reachabilityCallbackDelegate = [[[ReachabilityCallbackDelegate alloc] init] autorelease];
		CFRetain( reachabilityCallbackDelegate );
		[reachabilityCallbackDelegate setNetworkReachability:this];
		
		[[NSNotificationCenter defaultCenter] addObserver:reachabilityCallbackDelegate selector:@selector(reachabilityChanged:) name:[Rtt_Reachability reachabilityChangedNotificationKey] object:nil];
		

		[networkReachability startNotifier];
		
		// Sometimes, particularly with IP addresses, we don't get an initial callback so the users don't know the status until a hard network transition occurs.
		// This will force a callback to fire if a natural one doesn't occur first.
		// (Note: Invoking the callback now actually won't work because in init.lua, we don't save the listener until after this function returns.)
		[reachabilityCallbackDelegate performSelector:@selector(invokeFirstTimeChangeCallbackIfNecessary) withObject:nil afterDelay:5.0];
	}
	
	AppleReachability::~AppleReachability()
	{
		[networkReachability stopNotifier];
		if( nil != networkReachability )
		{
			CFRelease( networkReachability );
		}
		if( nil != reachabilityCallbackDelegate )
		{
			[[NSNotificationCenter defaultCenter] removeObserver:reachabilityCallbackDelegate];
			CFRelease( reachabilityCallbackDelegate );
		}
	}
	
	
	bool
	AppleReachability::IsValid() const
	{
		return (networkReachability != nil);
	}

	
	bool
	AppleReachability::IsReachable() const
	{
		return (bool)[networkReachability isReachable];
	}
	
	bool
	AppleReachability::IsConnectionRequired() const
	{
		return (bool)[networkReachability isConnectionRequired];
	}
	
	bool
	AppleReachability::IsConnectionOnDemand() const
	{
		return (bool)[networkReachability isConnectionOnDemand];
	}
	
	bool
	AppleReachability::IsInteractionRequired() const
	{
		return (bool)[networkReachability isInterventionRequired];
	}
	
	bool
	AppleReachability::IsReachableViaCellular() const
	{
		return (bool)[networkReachability isReachableViaWWAN];
	}
	
	bool
	AppleReachability::IsReachableViaWiFi() const
	{
		return (bool)[networkReachability isReachableViaWiFi];
	}

	
	void
	AppleReachability::InvokeCallback()
	{
		const char* address = [[networkReachability originalKey] UTF8String];
		ReachabilityChangeEvent event( * this, address );
		
		lua_State * L = fLuaState.Dereference();
		Rtt_ASSERT( L );
		
		if ( L )
		{
			Runtime* runtime = LuaContext::GetRuntime( L );
			runtime->DispatchEvent( event );
		}
	}
	
	// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------
