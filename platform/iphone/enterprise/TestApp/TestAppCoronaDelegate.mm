//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "TestAppCoronaDelegate.h"

#import "CoronaRuntime.h"
#import "CoronaLua.h"
#import "TestAppLibraryModule.h"

// Sample custom Lua error handler
// Register it via: Corona::Lua::SetErrorHandler()
static int
TestAppTraceback( lua_State* L )
{
	if (!lua_isstring(L, 1))  // 'message' not a string?
		return 1;  // keep it intact

	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  // pass error message
	lua_pushinteger(L, 1);  // skip this function and traceback
	lua_call(L, 2, 1);  // call debug.traceback

	if ( ! lua_gethook( L ) ) // Don't interfere with Lua debugger hook
	{
		// Log result of calling debug.traceback()
		NSLog( @"[LUA ERROR]: %s", lua_tostring( L, -1 ) );
	}

	return 1;
}

@implementation TestAppCoronaDelegate

- (void)willLoadMain:(id<CoronaRuntime>)runtime
{
	// Register modules before execution of main.lua
	const luaL_Reg moduleLoaders[] =
	{
		// Each module is a pair: (name, C-function loader)
		{ TestAppLibraryModule::Name(), TestAppLibraryModule::Open },
		
		// Termination
		{ NULL, NULL }
	};

	lua_State *L = runtime.L;

	// Make runtime available to each module
	lua_pushlightuserdata( L, runtime );
	Corona::Lua::RegisterModuleLoaders( L, moduleLoaders, 1 );

	// CUSTOM ERROR HANDLER
	// Uncomment the following line to set TestAppTraceback as a custom error handler:
	// Corona::Lua::SetErrorHandler( TestAppTraceback );
}

- (void)didLoadMain:(id<CoronaRuntime>)runtime
{
	lua_State *L = runtime.L;

	// DISPATCH CUSTOM EVENT
	// This does the equivalent of the following Lua code
	// where we have created a special 'delegate' event type
	// The main.lua file registers a listener for 'delegate' event:
	//		local event = { name = "delegate" }
	//		Runtime:dispatchEvent( event )

	// Create 'delegate' event
	const char kNameKey[] = "name";
	const char kValueKey[] = "delegate";
	lua_newtable( L );
	lua_pushstring( L, kValueKey );		// All events are Lua tables
	lua_setfield( L, -2, kNameKey );	// that have a 'name' property

	Corona::Lua::RuntimeDispatchEvent( L, -1 );

	lua_pop( L, 1 ); // pop event
}

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	[self printDictionary:launchOptions];
	return NO;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	[self printDictionary:launchOptions];
	return NO;
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	NSLog( @"sourceApplication %@.", sourceApplication );
	
	return NO;
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)applicationSignificantTimeChange:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application willChangeStatusBarOrientation:(UIInterfaceOrientation)newStatusBarOrientation duration:(NSTimeInterval)duration
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application didChangeStatusBarOrientation:(UIInterfaceOrientation)oldStatusBarOrientation
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application willChangeStatusBarFrame:(CGRect)newStatusBarFrame
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application didChangeStatusBarFrame:(CGRect)oldStatusBarFrame
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	[self printDictionary:userInfo];
}

- (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo fetchCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application performFetchWithCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application handleEventsForBackgroundURLSession:(NSString *)identifier completionHandler:(void (^)())completionHandler
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)applicationProtectedDataWillBecomeUnavailable:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)applicationProtectedDataDidBecomeAvailable:(UIApplication *)application
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window 
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	return UIInterfaceOrientationMaskAll;
}

- (UIViewController *) application:(UIApplication *)application viewControllerWithRestorationIdentifierPath:(NSArray *)identifierComponents coder:(NSCoder *)coder
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	// TODO
	return nil;
}

- (BOOL)application:(UIApplication *)application shouldSaveApplicationState:(NSCoder *)coder
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	return NO;
}

- (BOOL)application:(UIApplication *)application shouldRestoreApplicationState:(NSCoder *)coder
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
	return NO;
}

- (void)application:(UIApplication *)application willEncodeRestorableStateWithCoder:(NSCoder *)coder
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)application:(UIApplication *)application didDecodeRestorableStateWithCoder:(NSCoder *)coder
{
	NSLog( @"[CoronaDelegate] %@.", NSStringFromSelector( _cmd ) );
	[self printInfo:application];
}

- (void)printInfo:(NSObject*)object
{
	NSLog( @"Description : %@", [object description] );
}

- (void)printDictionary:(NSDictionary*)dictionary
{
	for ( NSString *key in [dictionary allKeys] )
	{
		NSLog( @"%@ : %@", key, [dictionary objectForKey:key] );
	}
}

- (void)application:(UIApplication *)application
	handleWatchKitExtensionRequest:(NSDictionary *)userInfo
	reply:(void (^)(NSDictionary *replyInfo))reply
{
	NSLog( @"handleWatchKitExtensionRequest: %@", userInfo );
	reply( @{ @"abc" : @"345" } );
}

@end
