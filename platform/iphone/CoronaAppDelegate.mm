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

#include "CoronaAppDelegate.h"

#include "Core/Rtt_Build.h"

#include "Rtt_Event.h"
#include "Rtt_Runtime.h"

#import "CoronaLua.h"
#import "CoronaViewPrivate.h"

@interface CoronaAppDelegate()

@property(nonatomic, retain) NSArray *fCoronaPluginDelegates;
@property(nonatomic, retain) id< CoronaDelegate > fEnterpriseDelegate;
@property(nonatomic, retain) NSMutableDictionary *fRespondsToSelectorCache;


@end

// ----------------------------------------------------------------------------

@implementation CoronaAppDelegate

+ (BOOL)handlesUrl:(NSURL *)url
{
	BOOL result = NO;
	NSString *str = [url absoluteString];
	id value = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleURLTypes"];
	if ( [value isKindOfClass:[NSArray class]] )
	{
		for ( id item in value )
		{
			if ( [item isKindOfClass:[NSDictionary class]] )
			{
				NSArray *schemes = [item objectForKey:@"CFBundleURLSchemes"];
				if ( [schemes isKindOfClass:[NSArray class]] )
				{
					for ( id o in schemes )
					{
						if ( [o isKindOfClass:[NSString class]] )
						{
							NSString *prefix = (NSString*)o;
							if ( [str hasPrefix:prefix] )
							{
								result = YES;
								break;
							}
						}
					}
				}
			}
		}
	}
	
	return result;
}

- (id)initWithEnterpriseDelegate:(id<CoronaDelegate>)enterpriseDelegate
{
	self = [super init];
	if ( self )
	{
		// Create delegate from the plist.  Used for plugins which need access to runtime before its be dynamically loaded
		// These delegates don't have access to
		// - (BOOL)application:willFinishLaunchingWithOptions:
		NSMutableArray *dels = [[NSMutableArray alloc] init];
		
		_fEnterpriseDelegate = enterpriseDelegate;
		if ( _fEnterpriseDelegate )
		{
			[dels addObject:_fEnterpriseDelegate];
		}
		
		// Get the classes for plugins which should be instanciated before its actually loaded.
		// Needed for things like remote notifications so it can be registered ahead of time.
		NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
		NSArray* delegates = [infoDict objectForKey:@"CoronaDelegates"];
		
		for ( NSString *delegate in delegates )
		{
			Class c = NSClassFromString(delegate);
			if ( c )
			{
				[dels addObject:[[c alloc] init]];
			}
		}
		
		_fCoronaPluginDelegates = dels;
		
		_fRespondsToSelectorCache = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (void)dealloc
{
	[_fRespondsToSelectorCache release];
	[_fCoronaPluginDelegates release];
	[super dealloc];
}

// See NOTE below if you are tempted to implement a method in the CoronaDelegate protocol
// ----------------------------------------------------------------------------

- (void)performSelector:(SEL)aSelector withObject:(id)anObject withObject:(id)anotherObject skip:(id)value
{
	for ( id delegate in _fCoronaPluginDelegates )
	{
		if ( [delegate respondsToSelector:aSelector] && delegate != value )
		{
			[delegate performSelector:aSelector withObject:anObject withObject:anotherObject];
		}
	}
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url options:(NSDictionary *)launchOptions
{
	BOOL result = NO;

	for ( id delegate in _fCoronaPluginDelegates ) {
		if ( [delegate respondsToSelector:_cmd] && delegate != _fEnterpriseDelegate )
		{
			[delegate application:application openURL:url options:launchOptions];
		}
	}

	if ( [_fEnterpriseDelegate respondsToSelector:_cmd] )
	{
		result = [_fEnterpriseDelegate application:application openURL:url options:launchOptions];
	}

	return result;
}

#if ! defined( Rtt_TVOS_ENV )
- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
	BOOL result = NO;
	
	for ( id delegate in _fCoronaPluginDelegates ) {
		if ( [delegate respondsToSelector:_cmd] && delegate != _fEnterpriseDelegate )
		{
			[delegate application:application openURL:url sourceApplication:sourceApplication annotation:annotation];
		}
	}
	
	if ( [_fEnterpriseDelegate respondsToSelector:_cmd] )
	{
		result = [_fEnterpriseDelegate application:application openURL:url sourceApplication:sourceApplication annotation:annotation];
	}
	
	return result;
}
#endif // Rtt_TVOS_ENV

- (BOOL)application:(UIApplication*)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	Rtt_ASSERT_NOT_REACHED();
	
	return NO;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	BOOL result = [CoronaAppDelegate handlesUrl:[launchOptions valueForKey:UIApplicationLaunchOptionsURLKey]];
	
	[self performSelector:_cmd withObject:application withObject:launchOptions skip:_fEnterpriseDelegate];
	
	if ( [_fEnterpriseDelegate respondsToSelector:_cmd] )
	{
		result = [_fEnterpriseDelegate application:application didFinishLaunchingWithOptions:launchOptions];
	}
	
	return result;
}

#if ! defined( Rtt_TVOS_ENV )
// It doesn't make sense to ask all the plugins if this orientation is supported since the enterprise delegate should be the master
- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)windowArg
{
	NSUInteger result = UIInterfaceOrientationMaskAll;
	
	if ( [_fEnterpriseDelegate respondsToSelector:_cmd] )
	{
		result = [_fEnterpriseDelegate application:application supportedInterfaceOrientationsForWindow:windowArg];
	}
	
	return result;
}
#endif

// ----------------------------------------------------------------------------

- (int)execute:(id<CoronaRuntime>)runtime command:(NSString *)key param:(id)param
{
	if ( [key isEqualToString:@"pushLaunchArgs"] )
	{
		using namespace Rtt;
		
		CoronaView *view = [param objectForKey:@"CoronaView"];
		NSDictionary *launchOptions = [param objectForKey:@"launchOptions"];
		Runtime *rttRuntime = view.runtime;
//TODO clear CoronaView key
		lua_State *L = runtime.L;
		
		for ( id delegate in _fCoronaPluginDelegates )
		{
			if ( [delegate respondsToSelector:@selector(execute:command:param:)] )
			{
				int top = lua_gettop( L );
				int itemsPushed = [delegate execute:runtime command:@"pushLaunchArgKey" param:nil];
				if ( lua_type( L, -1 ) == LUA_TSTRING && itemsPushed > 0 )
				{
					const char* property = lua_tostring( L, -1 );
					itemsPushed = [delegate execute:runtime command:@"pushLaunchArgValue" param:launchOptions];
					if ( Rtt_VERIFY( rttRuntime->PushLaunchArgs( true ) > 0 ) && itemsPushed > 0 )
					{
						lua_insert( L, -2 );
						lua_setfield( L, -2, property );  // will pop the value from the stack
					}
				}
				lua_settop( L, top );
			}
		}
	}
	return 0;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
	BOOL result = [super respondsToSelector:aSelector];

	if ( ! result )
	{
		NSString *key = NSStringFromSelector( aSelector );
		NSNumber *value = self.fRespondsToSelectorCache[key];
		
		if ( value )
		{
			result = [value boolValue];
		}
		else
		{
			// Cache miss
			for ( id delegate in _fCoronaPluginDelegates )
			{
				if ( [delegate respondsToSelector:aSelector] )
				{
					// Only one child needs to respond for a YES
					result = YES;
					break;
				}
			}
			
			// Add value to cache
			value = [NSNumber numberWithBool:result];
			self.fRespondsToSelectorCache[key] = value;
		}
	}

	return result;
}

// NOTE: This forwards delegate calls to each element in _fCoronaPluginDelegates
// If you implement one of the methods in the CoronaDelegate protocol, you must call:
// 
//    - (void)performSelector:(SEL)aSelector withObject:(id)anObject withObject:(id)anotherObject skip:(id)value
// 
- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	SEL aSelector = [anInvocation selector];
	for ( id delegate in _fCoronaPluginDelegates )
	{
		if ( [delegate respondsToSelector:aSelector] )
		{
			[anInvocation invokeWithTarget:delegate];
		}
	}
}

@end
