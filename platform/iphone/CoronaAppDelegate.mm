//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaAppDelegate.h"

#include "Core/Rtt_Build.h"

#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#if ! defined(Rtt_TVOS_ENV)
	#include "Rtt_IPhoneOrientation.h"
#endif
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

- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)windowArg
{
	NSUInteger result = UIInterfaceOrientationMaskAll;
	//Set default orientation to match build.settings
  NSBundle *bundle = [NSBundle mainBundle];
  id value = [bundle objectForInfoDictionaryKey:@"CoronaViewSupportedInterfaceOrientations"];
  if ( value && [value isKindOfClass:[NSArray class]])
  {
      if ([value count] > 0)
      {
          NSString* curOrientation = [value objectAtIndex:0];
          UIInterfaceOrientationMask supportedOrientations = Rtt::IPhoneOrientation::OrientationMaskForString( curOrientation );
          for(int i = 0; i < (int)[value count]; i++)
          {
              curOrientation = [value objectAtIndex:i];
              supportedOrientations |= Rtt::IPhoneOrientation::OrientationMaskForString( curOrientation );

          }

          result = supportedOrientations;
      }
  }
    //Allow Plugin Devs to adjust orientation(i.e Video Players)
    for ( id delegate in _fCoronaPluginDelegates ) {
            if ( [delegate respondsToSelector:_cmd] && delegate != _fEnterpriseDelegate )
            {
                result = [delegate application:application supportedInterfaceOrientationsForWindow:windowArg];
            }
        }


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
