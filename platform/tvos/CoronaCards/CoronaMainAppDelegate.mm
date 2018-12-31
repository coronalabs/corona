//
//  CoronaMainAppDelegate.mm
//  ratatouille
//
//  Copyright © 2015 Corona Labs. All rights reserved.
//

#include "Core/Rtt_Build.h"

#import "CoronaCards/CoronaMainAppDelegate.h"
#import "CoronaAppDelegate.h"
#import "CoronaDelegate.h"
#import "CoronaRuntime.h"

#import "CoronaCards/CoronaMainViewController.h"

#import "CoronaViewPrivate.h"
#import "CoronaViewControllerPrivate.h"

#import "Rtt_TVOSPlatform.h"

#include "Rtt_Display.h"
#include "Rtt_IPhoneRuntimeDelegate.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformDisplayObject.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

static Class sCoronaDelegateClass = nil;

static Class CoronaGetDelegateClass()
{
	return sCoronaDelegateClass;
}

// Declaration
FOUNDATION_EXPORT void CoronaSetDelegateClass( Class c );

FOUNDATION_EXPORT void CoronaSetDelegateClass( Class c )
{
	sCoronaDelegateClass = c;
}

// ----------------------------------------------------------------------------

@interface CoronaMainAppDelegate () <CoronaRuntime, CoronaViewLaunchDelegate>

@property (retain, nonatomic) CoronaMainViewController *viewController;
@property (retain, nonatomic) id<CoronaDelegate> customAppDelegate;
@property (nonatomic, retain) NSDictionary *launchOptions;
@property (nonatomic, assign) int suspendCount;
@property (nonatomic, assign) BOOL appEnteredBackground;
@property (nonatomic, assign) BOOL appStarted;

- (void)run;
- (void)setLaunchArgs:(id<CoronaRuntime>)runtime with:(NSDictionary*)launchOptions;

@end

// ----------------------------------------------------------------------------

@implementation CoronaMainAppDelegate

-(id)init
{
	self = [super init];
	
	if ( self )
	{
		[self initSelf];

//		[[NSBundle mainBundle] classNamed:@"CoronaView"];
//		[[NSBundle mainBundle] classNamed:@"CoronaViewController"];
	}

	return self;
}

- (void)dealloc
{
	[self deallocSelf];
	[super dealloc];
}


- (void)initSelf
{
/*
	window = nil;
	view = nil;
	viewController = nil;
	fCoronaDelegate = nil;
	fSuspendCount = 0;
	for ( size_t i = 0; i < sizeof( fGravityAccel ) / sizeof( fGravityAccel[0] ); i++ )
	{
		Rtt_STATIC_ASSERT( sizeof( fGravityAccel ) == sizeof( fInstantAccel ) );
		Rtt_STATIC_ASSERT( sizeof( fGravityAccel[0] ) == sizeof( fInstantAccel[0] ) );
		fGravityAccel[i] = 0.;
		fInstantAccel[i] = 0.;
	}
	fPreviousShakeTime = 0.;
	fActiveText = nil;
	fIsAppStarted = NO;
	fKeyboardShown = NO;
	fKeyboardOffset = 0.f;
	fWindowOffset = CGPointZero;
	fWindowCenterStart = CGPointZero;

	NSNumber *shouldWindowMove = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CoronaWindowMovesWhenKeyboardAppears"];
	fCoronaWindowMovesWhenKeyboardAppears = [shouldWindowMove boolValue];

	// Register for keyboard notifications
	NSNotificationCenter *notifier = [NSNotificationCenter defaultCenter];

	[notifier addObserver:self
		selector:@selector(keyboardWillShow:)
		name:UIKeyboardWillShowNotification object:nil];

	[notifier addObserver:self
		selector:@selector(keyboardWillHide:)
		name:UIKeyboardWillHideNotification object:nil];
*/
}

- (void)deallocSelf
{
/*
	// Unregister for keyboard notifications
	NSNotificationCenter *notifier = [NSNotificationCenter defaultCenter];

	[notifier removeObserver:self
		name:UIKeyboardWillShowNotification object:nil];

	[notifier removeObserver:self
		name:UIKeyboardWillHideNotification object:nil];

	[view terminate];
	Rtt_ASSERT( NULL == [view runtime] );

	[viewController release];
	[view release];
	[window release];
*/
}

- (CoronaView *)coronaView
{
	return (CoronaView *)self.viewController.coronaViewController.view;
}

// UIApplicationDelegate
// ----------------------------------------------------------------------------

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url options:(NSDictionary *)launchOptions
{
	using namespace Rtt;

	BOOL result = NO;

	if ( [[self appDelegateProxy] respondsToSelector:_cmd] )
	{
		result = [[self appDelegateProxy] application:application openURL:url options:launchOptions];
	}

	if ( ! result )
	{
		SystemOpenEvent e( [[url absoluteString] UTF8String] );
		Runtime *runtime = [[self coronaView] runtime];
		if ( Rtt_VERIFY( runtime ) )
		{
			runtime->DispatchEvent( e );
		}
	}

	return result;
}

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
	using namespace Rtt;

	BOOL result = [CoronaAppDelegate handlesUrl:[launchOptions valueForKey:UIApplicationLaunchOptionsURLKey]];

		Class c = CoronaGetDelegateClass();
		if ( c )
		{
			self.customAppDelegate = [[c alloc] init];
		}
		
	// This is before the IPhoneRuntimeDelegate is created so the this should be the enterprise delegate
	// which is why we can directly return the result
	if ( [[self appDelegateProxy] respondsToSelector:_cmd] )
		{
		result = [[self appDelegateProxy] application:application willFinishLaunchingWithOptions:launchOptions];
			}
		
	return result;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	// Override point for customization after application launch.

	self.launchOptions = [[launchOptions copy] autorelease];

	return [[self appDelegateProxy] application:application didFinishLaunchingWithOptions:launchOptions];
}

- (void)applicationWillResignActive:(UIApplication *)application {
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
	[[self appDelegateProxy] performSelector:_cmd withObject:application];

	[[self coronaView] suspend];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
	[[self appDelegateProxy] performSelector:_cmd withObject:application];
	self.appEnteredBackground = YES;
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
	self.appEnteredBackground = NO;

	[[self appDelegateProxy] performSelector:_cmd withObject:application];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
	// We need this guard b/c on app launch, this method is invoked and we need
	// to distinguish between the startup case and a potential resume case.
	if ( self.appStarted )
	{
		[[self coronaView] resume];
	}
	else
	{
		self.appStarted = YES;
	}

	[[self appDelegateProxy] performSelector:_cmd withObject:application];

}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.

	[[self appDelegateProxy] performSelector:_cmd withObject:application];
	
	if ( [[self coronaView] runtime] )
	{
		[self.customAppDelegate release];
		self.customAppDelegate = nil;
		
		[[self coronaView] terminate];
	}
}

// CoronaRuntime
// ----------------------------------------------------------------------------

- (UIWindow *)appWindow
{
	return self.window;
}

- (UIViewController *)appViewController
{
	return self.viewController;
}

- (lua_State *)L
{
	return [self coronaView].runtime->VMContext().L();
}

- (CGPoint)coronaPointToUIKitPoint:(CGPoint)coronaPoint
{
	using namespace Rtt;

	Rtt_Real x = Rtt_FloatToReal( coronaPoint.x );
	Rtt_Real y = Rtt_FloatToReal( coronaPoint.y );

	Rtt::Rect bounds;
	bounds.xMin = x;
	bounds.yMin = y;
	bounds.xMax = x;
	bounds.yMax = y;

	Rtt_Real sx, sy;
	const Display& display = [self coronaView].runtime->GetDisplay();
	display.CalculateContentToScreenScale( sx, sy );
	bool shouldScale = ! Rtt_RealEqual( Rtt_REAL_1, sx ) || ! Rtt_RealEqual( Rtt_REAL_1, sy );
	if ( shouldScale )
	{
		sx = Rtt_RealDiv( Rtt_REAL_1, sx );
		sy = Rtt_RealDiv( Rtt_REAL_1, sy );
		PlatformDisplayObject::CalculateScreenBounds( display, sx, sy, bounds );
	}

	CGPoint result = { Rtt_RealToFloat( bounds.xMin ), Rtt_RealToFloat( bounds.yMin ) };

	return result;
}

- (void)suspend
{
	Rtt_ASSERT( self.suspendCount >= 0 );

	if ( 0 == self.suspendCount++ )
	{
		[self coronaView].runtime->Suspend();
	}
}

- (void)resume
{
	Rtt_ASSERT( self.suspendCount > 0 );

	if ( 0 == --self.suspendCount )
	{
		[self coronaView].runtime->Resume();
	}
}

// ----------------------------------------------------------------------------

- (void)initialize
{
	using namespace Rtt;

	UIWindow *window = self.window;

	CoronaMainViewController *viewController = (CoronaMainViewController *)window.rootViewController;
	Rtt_ASSERT( [viewController isKindOfClass:[CoronaMainViewController class]] );

	self.viewController = viewController;

//	[view setAutoresizingMask:UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];

//	[window setRootViewController:viewController];
//	[window makeKeyAndVisible];

//	Rtt_ASSERT( view );
//	Rtt_ASSERT( ! view.runtime );
//	Rtt_ASSERT( viewController );
//	Rtt_ASSERT( view == viewController.view );

	CoronaView *view = [self coronaView];
	TVOSPlatform *platform = new TVOSPlatform( view );

	view.launchDelegate = self;

	// TODO: Custom UIApplicationDelegate for Enterprise is not implemented
	id <CoronaDelegate> enterpriseDelegate = nil;

	IPhoneRuntimeDelegate *runtimeDelegate = new IPhoneRuntimeDelegate( view, self, enterpriseDelegate );
	[view initializeRuntimeWithPlatform:platform runtimeDelegate:runtimeDelegate];

	// The CoronaMainAppDelegate will observe suspend/resume notification, so don't use default observers
	view.observeSuspendResume = NO;
}


- (NSInteger)runView:(CoronaView *)sender withPath:(NSString *)path parameters:(NSDictionary *) params
{
	[self initialize];
	[self run];
	CoronaView *view = [self coronaView];
	[view beginRunLoop];
	return 0;
}

- (void)run
{
	[self setLaunchArgs:self with:self.launchOptions];

	CoronaView *view = [self coronaView];
	view.beginRunLoopManually = YES;
	[view runWithPath:nil parameters:nil];
}

static void
SetLaunchArgs( UIApplication *application, NSDictionary *launchOptions, Rtt::Runtime *runtime )
{
	using namespace Rtt;

	lua_State *L = runtime->VMContext().L();

	if ( Rtt_VERIFY( runtime->PushLaunchArgs( true ) > 0 ) )
	{
		NSURL *url = [launchOptions objectForKey:UIApplicationLaunchOptionsURLKey];
		if ( url )
		{
			lua_pushstring( L, [[url absoluteString] UTF8String] );
			lua_setfield( L, -2, "url" );
		}

		lua_pop( L, 1 );
	}
}

- (void)setLaunchArgs:(id<CoronaRuntime>)runtime with:(NSDictionary*)launchOptions
{
	using namespace Rtt;

	CoronaView *view = [self coronaView];
	Runtime *rttRuntime = view.runtime;
	SetLaunchArgs( [UIApplication sharedApplication], launchOptions, rttRuntime );
	
	NSMutableDictionary *params = [NSMutableDictionary dictionaryWithObject:view forKey:@"CoronaView"];
	if ( launchOptions )
	{
		[params setObject:launchOptions forKey:@"launchOptions"];
	}

	// This should return a CoronaAppDelegate which has the actual logic of looping through the plugin delegates
	// and asking each one to should on the correct values onto the lua stack
	id<CoronaDelegate> delegate = [self appDelegateProxy];
	
	if ( [delegate respondsToSelector:@selector(execute:command:param:)] )
	{
		[delegate execute:runtime command:@"pushLaunchArgs" param:params];
	}
}

// ----------------------------------------------------------------------------

- (BOOL)respondsToSelector:(SEL)aSelector
{
	BOOL result = [super respondsToSelector:aSelector];

	if ( ! result )
	{
		result = [[self appDelegateProxy] respondsToSelector:aSelector];
	}

	return result;
}

- (id)forwardingTargetForSelector:(SEL)aSelector
{
	return [self appDelegateProxy];
}

- (id<CoronaDelegate>)appDelegateProxy
{
	CoronaView *view = [self coronaView];
	if ( view && [view runtimeDelegate] )
	{
		Rtt::CoronaViewRuntimeDelegate* coronaViewDelegate = [view runtimeDelegate];
		Rtt::IPhoneRuntimeDelegate* del = static_cast<Rtt::IPhoneRuntimeDelegate*>((coronaViewDelegate));
		return del->GetCoronaDelegate();
	}

	return self.customAppDelegate;
}

// ----------------------------------------------------------------------------

@end
