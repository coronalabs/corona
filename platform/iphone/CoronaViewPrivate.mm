//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import <QuartzCore/QuartzCore.h>
#import <CoreLocation/CoreLocation.h>

#import "CoronaViewPrivate.h"
#import "CoronaViewRuntimeDelegate.h"
#import "CoronaGyroscopeObserver.h"
#ifdef Rtt_ORIENTATION
	#import "CoronaOrientationObserver.h"
#endif
#import "CoronaOrientationProvider.h"
#import "CoronaSystemResourceManager.h"
#import "AppleWeakProxy.h"

#ifdef Rtt_CORE_MOTION
	#import <CoreMotion/CoreMotion.h>
#endif

#include "Core/Rtt_Build.h"
#include "Core/Rtt_New.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_StageObject.h"
#include "Rtt_AppleBitmap.h"
#include "Rtt_AppleInputDeviceManager.h"
#include "Rtt_AppleInputMFiDeviceListener.h"
#include "Rtt_IPhonePlatformBase.h"
#include "Rtt_DeviceOrientation.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_Runtime.h"
#import <objc/runtime.h>

#if defined( Rtt_IPHONE_ENV )
	#include "Rtt_IPhonePlatformCore.h"
	#include "Rtt_IPhoneOrientation.h"
	#include "Rtt_IPhoneTemplate.h"
#endif

#include "Rtt_Event.h"
#include "Rtt_GPU.h"
#include "Rtt_MCallback.h"

#include "CoronaEvent.h"
#include "CoronaLua.h"
#include "Rtt_LuaContext.h"

#import "Rtt_AppleTextDelegateWrapperObjectHelper.h"
#import "Rtt_IPhoneTextBoxObject.h"
#import "Rtt_IPhoneTextFieldObject.h"

#include "Rtt_KeyName.h"
#include "Rtt_MetalAngleTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	
class MCallback;

// ----------------------------------------------------------------------------
	
} // Rtt

// ----------------------------------------------------------------------------

// Creates the default platform for CoronaView
// NOTE: Default implies iOS and CoronaCards.
// NOTE: We deliberately do not have a default for non-iOS platforms
// to make the maintenance of this file (which is shared across iOS/tvOS)
// somewhat sane.
static Rtt::IPhonePlatformBase *
CreatePlatform( CoronaView *view )
{
#if defined( Rtt_IPHONE_ENV )
	return new Rtt::IPhonePlatformCore( view );
#else
	// On non-iOS platforms, you should pass in your own instance of iPhonePlatformBase
	// We are trying to keep all non-iOS related classes/etc (e.g. TVOS) out of this file.
	Rtt_ASSERT_NOT_REACHED();
	return nullptr;
#endif
}

// UITouch (CoronaViewExtensions)
// ----------------------------------------------------------------------------
#pragma mark # UITouch (CoronaViewExtensions)

@interface UITouch ( CoronaViewExtensions )

- (CGPoint)locationInCoronaView:(CoronaView*)view;

@end


@implementation UITouch ( CoronaViewExtensions )

- (CGPoint)locationInCoronaView:(CoronaView*)view
{
	using namespace Rtt;

	// This should be the only place in this file where we call locationInView:
	CGPoint result = [self locationInView:view];

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 40000
	// Compiler macro is insufficient if you are building against the 4.0 SDK for new features, but
	// also want to support older OS's. You need a runtime check as well.
	if([view respondsToSelector:@selector(contentScaleFactor)])
	{
		CGFloat scale = view.contentScaleFactor;
		
		result.x *= scale;
		result.y *= scale;
	}
#endif

	return result;
}

@end

// TapEventWrapper
// ----------------------------------------------------------------------------
#pragma mark # TapEventWrapper

@interface TapEventWrapper : NSObject
{
	Rtt::TapEvent *event;
}

- (id)initWithTouch:(UITouch*)touch inView:(CoronaView*)view;

@property (nonatomic, readonly) Rtt::TapEvent *event;

@end


@implementation TapEventWrapper

@synthesize event;

- (id)initWithTouch:(UITouch*)touch inView:(CoronaView*)view
{
	self = [super init];
	if ( self )
	{
		CGPoint p = [touch locationInCoronaView:view];
		event = new Rtt::TapEvent( Rtt_FloatToReal( p.x ), Rtt_FloatToReal( p.y ), (S32)touch.tapCount );
//		Rtt_TRACE( ( "TapEventWrapper create(%p)\n", event ) );
	}

	return self;
}

- (void)dealloc
{
	delete event;

	[super dealloc];
}

@end

// CoronaViewListenerAdapter
// ----------------------------------------------------------------------------
#pragma mark # CoronaViewListenerAdapter

static int
CoronaViewListenerAdapter( lua_State *L )
{
	using namespace Rtt;

	int result = 0;

	int eventIndex = 1;
	if ( lua_istable( L, eventIndex ) )
	{
		CoronaView *view = (CoronaView *)lua_touserdata( L, lua_upvalueindex( 1 ) );
		id <CoronaViewDelegate> delegate = view.coronaViewDelegate;
		if ( [delegate respondsToSelector:@selector(coronaView:receiveEvent:)] )
		{
			NSDictionary *event = ApplePlatform::CreateDictionary( L, eventIndex );
			id value = [delegate coronaView:view receiveEvent:event];

			result = (int)ApplePlatform::Push( L, value );
		}
	}

	return result;
}


// CoronaView()
// ----------------------------------------------------------------------------
#pragma mark # CoronaView()

@interface CoronaView() <
#ifdef Rtt_ORIENTATION
							CoronaOrientationObserver,
#endif
							CLLocationManagerDelegate,
							CoronaGyroscopeObserver,
							CoronaOrientationProvider>
{
@private
	NSString *fResourcePath;
	CFMutableDictionaryRef fTouchesData;
	CGPoint fStartTouchPosition;
	NSTimeInterval fTapDelay;
	int fInhibitCount; // used by TouchInhibitor
	int fSuspendCount;
	int fLastContentHeight;
	bool fShouldInvalidate;
	bool fBeganRunLoop;

	// Gyroscope
	BOOL gyroscopeEnabled;
	U64 gyroscopePreviousTimestampCorona;
	NSTimeInterval gyroscopePreviousTimestamp;

	// Orientation
	Rtt::DeviceOrientation::Type fLoadOrientation; // orientation when loading Runtime

	// Load Parameters
	NSDictionary *fParams;

	// TextField/TextBox
	UIView *fActiveText;
	CGPoint fWindowOffset;
	CGPoint fWindowCenterStart;
	CGFloat fKeyboardOffset;
	BOOL fKeyboardShown;
	BOOL fCoronaWindowMovesWhenKeyboardAppears;

	AppleInputMFiDeviceListener *fMFiInputListener;
}

@property (nonatomic, readonly) Rtt::IPhonePlatformBase *platform;
@property (nonatomic, retain) AppleWeakProxy *observerProxy;

- (void)addApplicationObserver;
- (void)removeApplicationObserver;
- (void)applicationWillResignActive:(NSNotification *)notification;
- (void)applicationDidBecomeActive:(NSNotification *)notification;

- (void)dispatchTapEvent:(TapEventWrapper*)event;
- (void)dispatchEvent:(Rtt::MEvent*)event;

- (void)pollAndDispatchMotionEvents;

- (void)dispatchEditingEventForTextView:(Rtt_AppleTextDelegateWrapperObjectHelper*)eventData;
- (void)dispatchEditingEventForTextField:(Rtt_AppleTextDelegateWrapperObjectHelper*)eventData;

@end

// CoronaView
// ----------------------------------------------------------------------------
#pragma mark # CoronaView

@implementation CoronaView

// ----------------------------------------------------------------------------

+ (Rtt::DeviceOrientation::Type)deviceOrientationForString:(NSString *)value
{
	using namespace Rtt;

	DeviceOrientation::Type result = DeviceOrientation::kUpright;
	
	if ( [value isEqualToString:@"UIInterfaceOrientationPortraitUpsideDown"] )
	{
		result = DeviceOrientation::kUpsideDown;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeLeft"] )
	{
		result = DeviceOrientation::kSidewaysLeft;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeRight"] )
	{
		result = DeviceOrientation::kSidewaysRight;
	}

	return result;
}

+ (Rtt::DeviceOrientation::Type)defaultOrientation
{
	using namespace Rtt;
	
#ifdef Rtt_IPHONE_ENV
	return IPhoneOrientation::ConvertOrientation( UIInterfaceOrientationPortrait );
#elif defined( Rtt_TVOS_ENV )
	// We are always in landscapeRight on tvOS.
	// This has the side effect of not allowing CoronaCards portrait apps for tvOS.
	return DeviceOrientation::kSidewaysRight;
#else
	#error Default orientation not defined for this target platform
	return DeviceOrientation::kUnknown;
#endif
}

// ----------------------------------------------------------------------------

@synthesize fInhibitCount;
@synthesize fTapDelay;

// Bottleneck for startup
- (void)initCommon
{
	fResourcePath = nil;
	fTouchesData = nil;
	fTapDelay = 0.;
	fInhibitCount = 0;
	fSuspendCount = 0;
	_observeSuspendResume = YES;
	fLastContentHeight = -1;
	fShouldInvalidate = false;
	fLoadOrientation = Rtt::DeviceOrientation::kUpright;
	fParams = nil;

	// TextField/TextBox
	{
		fActiveText = nil;
		fWindowOffset = CGPointZero;
		fWindowCenterStart = CGPointZero;
		fKeyboardOffset = 0.f;
		fKeyboardShown = NO;

		NSNumber *shouldWindowMove = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CoronaWindowMovesWhenKeyboardAppears"];
		fCoronaWindowMovesWhenKeyboardAppears = [shouldWindowMove boolValue];

#ifndef Rtt_TVOS_ENV
		// Register for keyboard notifications
		NSNotificationCenter *notifier = [NSNotificationCenter defaultCenter];

		[notifier addObserver:self
			selector:@selector(keyboardWillShow:)
			name:UIKeyboardWillShowNotification object:nil];

		[notifier addObserver:self
			selector:@selector(keyboardWillHide:)
			name:UIKeyboardWillHideNotification object:nil];
#endif
	}

	_observerProxy = [[AppleWeakProxy alloc] initWithTarget:self];
	_orientationObserver = (id< CoronaOrientationObserver >)_observerProxy;
	_locationObserver = (id< CLLocationManagerDelegate >)_observerProxy;
	_gyroscopeObserver = (id< CoronaGyroscopeObserver >)_observerProxy;
}

- (id)initWithFrame:(CGRect)rect context:(Rtt_EAGLContext *)context
{
	if ( (self = [super initWithFrame:rect context:context]) )
	{
		[self initCommon];
	}

	return self;
}

#ifndef Rtt_MetalANGLE
- (id)initWithFrame:(CGRect)rect
{
	if ( self || (self = [self initWithFrame:rect context:nil]) )
	{
		[self initCommon];
	}

	return self;
}
#endif

- (id)initWithCoder:(NSCoder *)aDecoder
{
	if ( (self = [super initWithCoder:aDecoder]) )
	{		
		[self initCommon];
	}

	return self;
}

// Bottleneck for teardown
- (void)deallocCommon
{
	// TextField/TextBox
	{
#ifndef Rtt_TVOS_ENV
		// Unregister for keyboard notifications
		NSNotificationCenter *notifier = [NSNotificationCenter defaultCenter];

		[notifier removeObserver:self
			name:UIKeyboardWillShowNotification object:nil];

		[notifier removeObserver:self
			name:UIKeyboardWillHideNotification object:nil];
#endif
	}

	[self terminate];

	if ( fTouchesData )
	{
		[fParams release];

		CFRelease( fTouchesData );
		fTouchesData = nil;

		[fResourcePath release];
	}
}

- (void)dealloc
{
	[self deallocCommon];

	[super dealloc];
}

- (void)initializeRuntime
{
	[self initializeRuntimeWithPlatform:NULL runtimeDelegate:NULL];
}

- (void)initializeRuntimeWithPlatform:(Rtt::IPhonePlatformBase *)platform runtimeDelegate:(Rtt::CoronaViewRuntimeDelegate *)runtimeDelegate
{
	using namespace Rtt;

	if ( ! _runtime )
	{
		_platform = ( platform ? platform : CreatePlatform( self ) );
		_runtime = new Runtime( * _platform );

		_runtimeDelegate = ( runtimeDelegate ? runtimeDelegate : new CoronaViewRuntimeDelegate( self ) );
		
		_runtime->SetDelegate( _runtimeDelegate );

#ifdef Rtt_IPHONE_ENV
		bool isCoronaKit = Rtt::IPhoneTemplate::IsProperty( IPhoneTemplate::kIsCoronaKit );
#else
		bool isCoronaKit = false;
#endif
		_runtime->SetProperty( Rtt::Runtime::kIsCoronaKit, isCoronaKit );
		if ( isCoronaKit )
		{
			_runtime->SetProperty( Rtt::Runtime::kIsApplicationNotArchived, true );
		 	_runtime->SetProperty( Rtt::Runtime::kIsLuaParserAvailable, true );
		}
	}
}

- (Rtt_GLKViewController *)viewController
{
	Rtt_ASSERT( [self.delegate isKindOfClass:[UIViewController class]] );
	return (Rtt_GLKViewController *)self.delegate;
}

- (void)willBeginRunLoop:(NSDictionary *)params
{
	using namespace Rtt;

	lua_State *L = _runtime->VMContext().L();

	// Pass launch params to main.lua
	if ( params )
	{
		Rtt_LUA_STACK_GUARD( L );

		if ( Rtt_VERIFY( _runtime->PushLaunchArgs( true ) > 0 ) )
		{
			lua_State *L = _runtime->VMContext().L();
			ApplePlatform::CopyDictionary( L, -1, params );
			lua_pop( L, 1 );
		}
	}

	// Attach listener for "coronaView" events
	{
		Lua::AddCoronaViewListener( L, CoronaViewListenerAdapter, self );
//		Rtt_LUA_STACK_GUARD( L );
//
//		// Push Runtime.addEventListener
//		// Push Runtime
//		Lua::PushRuntime( L );
//		lua_getfield( L, -1, "addEventListener" );
//		lua_insert( L, -2 ); // swap table and function
//
//		// Push 'coronaView'
//		lua_pushstring( L, "coronaView" );
//
//		// Push 'CoronaViewListenerAdapter'
//		lua_pushlightuserdata( L, self );
//		lua_pushcclosure( L, CoronaViewListenerAdapter, 1 );
//
//		// Runtime.addEventListener( Runtime, "coronaView", CoronaViewListenerAdapter )
//		int status = Lua::DoCall( L, 3, 0 ); Rtt_UNUSED( status );
//		Rtt_ASSERT( 0 == status );
	}
}

- (NSInteger)run
{
	return [self runWithPath:nil parameters:nil];
}

- (NSInteger)runWithPath:(NSString*)path parameters:(NSDictionary *)params
{
	Rtt::DeviceOrientation::Type orientation = [[self class] defaultOrientation];
	NSString *orientationParam = [params valueForKey:@"orientation"];

	// Allow params to override default orientation
	if (orientationParam != nil)
	{
		orientation = Rtt::DeviceOrientation::TypeForString([orientationParam UTF8String]);
	}

	return [self runWithPath:path parameters:params orientation:orientation];
}

- (NSInteger)runWithPath:(NSString*)path parameters:(NSDictionary *)params orientation:(Rtt::DeviceOrientation::Type)orientation
{
	using namespace Rtt;

	NSInteger result = (NSInteger)Runtime::kGeneralFail;

	[self initializeRuntime];

	if(!fMFiInputListener) {
		AppleInputDeviceManager& inputDeviceManager = (AppleInputDeviceManager&)_runtime->Platform().GetDevice().GetInputDeviceManager();
		fMFiInputListener = [[AppleInputMFiDeviceListener alloc] initWithRuntime:_runtime andDeviceManager:&inputDeviceManager];
	}

	// Check if already launched
	if ( ! _runtime->IsProperty( Runtime::kIsApplicationExecuting ) )
	{
		[self addApplicationObserver];

		fResourcePath = [path copy];
		_platform->SetResourceDirectory( path );

		Rtt_ASSERT( self.context );

		// We need to bind to correct framebuffer so that GL commands triggered by
		// * LoadApplication()
		// * BeginRunLoop()
		// go to to the right place
		[Rtt_EAGLContext setCurrentContext:self.context];
		[self bindDrawable];

		fLoadOrientation = orientation;
		fParams = [params retain];


		if ( ! self.beginRunLoopManually )
		{
			result = [self beginRunLoop];
		}
		else
		{
			result = Runtime::kSuccess;
		}
	}


	return result;
}

- (NSInteger)beginRunLoop
{
	using namespace Rtt;

	if ( fBeganRunLoop ) { return (NSInteger)Runtime::kGeneralFail; }

	fBeganRunLoop = true;

	Rtt::Runtime::LoadParameters loadParameters;
	U32 launchOptions = Runtime::kDeviceLaunchOption;
	loadParameters.orientation = fLoadOrientation;

	loadParameters.launchOptions = launchOptions;

	id width = [fParams objectForKey:@"contentWidth"];
	S32 contentWidth = ( [width isKindOfClass:[NSNumber class]] ? [width intValue] : -1 );
	id height = [fParams objectForKey:@"contentHeight"];
	S32 contentHeight = ( [height isKindOfClass:[NSNumber class]] ? [height intValue] : -1 );
	if ( contentWidth > 0 && contentHeight > 0 )
	{
		loadParameters.contentWidth = contentWidth;
		loadParameters.contentHeight = contentHeight;
	}


	NSInteger result =_runtime->LoadApplication( loadParameters );
	if ( result == (NSInteger)Runtime::kSuccess )
	{
		[fMFiInputListener start];
		[self willBeginRunLoop:fParams];

		_runtime->BeginRunLoop();
		
		//Forcing immediate blit (outside MGLKViewController which normally drives the display update)
		[self display];
	}

	return result;
}

#ifdef Rtt_ORIENTATION
- (void)notifyRuntimeAboutOrientationChange:(UIInterfaceOrientation)toInterfaceOrientation
{
	Rtt::Runtime *runtime = self.runtime;
	if ( runtime )
	{
		runtime->SetContentOrientation( Rtt::IPhoneOrientation::ConvertOrientation( toInterfaceOrientation ) );
	}
}
#endif

+(Rtt::Real)getTouchForce:(UITouch*)touch {
	static bool initialized = false;
	static bool supports3DTouch = false;
	if(!initialized) {
		initialized = true;
		Method m = class_getInstanceMethod([touch class], @selector(force));
		if(m) {
			char type[10]={0};
			method_getReturnType(m, type, sizeof(type));
			supports3DTouch = (strncmp(type, "d", sizeof(type)) == 0);
		}
	}
	
	Rtt::Real force = Rtt::TouchEvent::kPressureInvalid;
	if (supports3DTouch) {
		force = [touch force];
		if(force == 0 && [touch type] != UITouchTypePencil) {
			force = Rtt::TouchEvent::kPressureInvalid;
		}
	}
	return force;
}

- (BOOL)simulateCommand:(NSDictionary *)args
{
	BOOL result = NO;

	NSString *command = [args valueForKey:@"command"];
	NSDictionary *commandOptions = [args valueForKey:@"options"];

	if ( [command isEqualToString:@"launch"] )
	{
		NSString *resourcePath = [commandOptions valueForKey:@"resourcePath"];
		NSString *mainPath = [resourcePath stringByAppendingPathComponent:@"main.lua"];
		NSFileManager *fileMgr = [NSFileManager defaultManager];
		if ( mainPath && [fileMgr fileExistsAtPath:mainPath isDirectory:nil] )
		{
			_platform->SetResourceDirectory( resourcePath );

			lua_State *L = _runtime->VMContext().L();

			// package.path = resourcePath .. "/?.lua"
			lua_getglobal( L, "package" );
			{
				lua_pushfstring( L, "%s" LUA_DIRSEP LUA_PATH_MARK ".lua", [resourcePath UTF8String] );
				lua_setfield( L, -2, "path" );
			}
			lua_pop( L, 1 );

			result = ( 0 == CoronaLuaDoFile( L, [mainPath UTF8String], 0, true ) );
		}
	}
	else if ( [command isEqualToString:@"relaunch"] )
	{
		// NOTE: Relaunch has to happen asynchronously, so we queue the relaunch.
	
		// The balancing release happens in the block. This guarantees it's valid
		// by the time the block executes
		NSString *path = [fResourcePath retain];
		CoronaView *view = self;

		// Define the op
		NSBlockOperation *op = [NSBlockOperation blockOperationWithBlock:^{
			[view deallocCommon];

			[view initCommon];

			if ( view.launchDelegate )
			{
				[view.launchDelegate runView:view withPath:path parameters:nil];
			}
			else
			{
				[view runWithPath:path parameters:nil];
			}

			[path release];
		}];

		// Queue the op
		NSOperationQueue *queue = [NSOperationQueue mainQueue];
		[queue addOperation:op];
		result = YES;
	}

	return result;
}

- (void)suspend
{
	if ( _runtime )
	{
		Rtt_ASSERT( fSuspendCount >= 0 );

		if ( 0 == fSuspendCount++ )
		{
			if ( [_coronaViewDelegate respondsToSelector:@selector(coronaViewWillSuspend:)] )
			{
				[_coronaViewDelegate coronaViewWillSuspend:self];
			}

			_runtime->Suspend();

			if ( [_coronaViewDelegate respondsToSelector:@selector(coronaViewDidSuspend:)] )
			{
				[_coronaViewDelegate coronaViewDidSuspend:self];
			}
		}
	}
}

- (void)resume
{
	if ( _runtime )
	{
		if ( fSuspendCount > 0 )
		{
			if ( 0 == --fSuspendCount )
			{
				if ( [_coronaViewDelegate respondsToSelector:@selector(coronaViewWillResume:)] )
				{
					[_coronaViewDelegate coronaViewWillResume:self];
				}

				_runtime->Resume();

				_runtime->GetDisplay().Invalidate();

				if ( [_coronaViewDelegate respondsToSelector:@selector(coronaViewDidResume:)] )
				{
					[_coronaViewDelegate coronaViewDidResume:self];
				}
			}
		}
	}
}

- (void)terminate
{
	//Grab the view's context
	Rtt_EAGLContext *context = self.context;
	
	//Grab the active openGL context
	Rtt_EAGLContext *openGlContext = [Rtt_EAGLContext currentContext];
	
	if ( openGlContext != context)
	{
		[Rtt_EAGLContext setCurrentContext:context];
	}
	
	[self removeApplicationObserver];

	// Destroy runtime first
	fBeganRunLoop = false;
	delete _runtime;
	_runtime = NULL;

	delete _runtimeDelegate;
	_runtimeDelegate = NULL;

	delete _platform; // We assume _platform is instantiated with plain new
	_platform = NULL;

	[fMFiInputListener stop];
	[fMFiInputListener release];
	fMFiInputListener = nil;

	// Cleanup observers used by CoronaSystemResourceManager
	// Do this after _platform is deleted b/c IPhoneDevice needs to remove these observers
	[_observerProxy invalidate];
	[_observerProxy release];
	_observerProxy = nil;
	_orientationObserver = nil;
	_locationObserver = nil;
	_gyroscopeObserver = nil;
	
	//Restore the context
	[Rtt_EAGLContext setCurrentContext:openGlContext];
}

- (id)sendEvent:(NSDictionary *)event
{
	using namespace Rtt;

	id result = nil;

	lua_State *L = _runtime->VMContext().L();
	Rtt_LUA_STACK_GUARD( L );
	
	id name = [event valueForKey:@"name"];
	if ( [name isKindOfClass:[NSString class]]
		 && [name length] > 0 )
	{
		ApplePlatform::CreateAndPushTable( L, event ); // push event
		Corona::Lua::DispatchRuntimeEvent( L, 1 );

		// NOTE: In the EventListener:dispatchEvent code, the default result
		// is 'false'. For Obj-C, we'd prefer the default to be 'nil'.
		// Thus, if we get 'false', we skip the conversion. In Lua, these are
		// morally equivalent and we probably should have set the default to be
		// nil to begin with.
		if ( 0 != lua_toboolean( L, -1 ) )
		{
			result = ApplePlatform::ToValue( L, -1 );
		}
		lua_pop( L, 1 ); // pop result
	}

	return result;
}

- (void)addApplicationObserver
{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
#ifdef Rtt_ORIENTATION
	[[NSNotificationCenter defaultCenter] addObserver:self.orientationObserver selector:@selector(didOrientationChange:) name:UIDeviceOrientationDidChangeNotification object:nil];
#endif
}

- (void)removeApplicationObserver
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationWillResignActiveNotification object:nil];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidBecomeActiveNotification object:nil];
#ifdef Rtt_ORIENTATION
	[[NSNotificationCenter defaultCenter] removeObserver:self.orientationObserver name:UIDeviceOrientationDidChangeNotification object:nil];
#endif
}

- (void)applicationWillResignActive:(NSNotification *)notification
{
	if ( _observeSuspendResume )
	{
		[self suspend];
	}
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
	if ( _observeSuspendResume )
	{
		[self resume];
	}
}


- (void)dispatchTapEvent:(TapEventWrapper*)e;
{
//	Rtt_TRACE( ( "dispatchTapEventWrapper(%p) numTaps(%d)\n", e, e.event->NumTaps() ) );
	[self dispatchEvent:e.event];
}

- (void)dispatchEvent:(Rtt::MEvent*)e;
{
	using namespace Rtt;

	Rtt_ASSERT( _runtime );

	if ( Rtt_VERIFY( e ) )
	{
		_runtime->DispatchEvent( * e );
	}
}

static void
TouchesDataValueRelease( CFAllocatorRef allocator, const void *p )
{
	Rtt_ASSERT( p );
	free( (CGPoint*)p );
}

static Rtt::TouchEvent::Phase
UITouchPhaseToTouchEventPhase( UITouchPhase phase )
{
	Rtt::TouchEvent::Phase result;

	switch( phase )
	{
		case UITouchPhaseBegan:
			result = Rtt::TouchEvent::kBegan;
			break;
		case UITouchPhaseMoved:
			result = Rtt::TouchEvent::kMoved;
			break;
		case UITouchPhaseStationary:
			result = Rtt::TouchEvent::kStationary;
			break;
		case UITouchPhaseEnded:
			result = Rtt::TouchEvent::kEnded;
			break;
		case UITouchPhaseCancelled:
			result = Rtt::TouchEvent::kCancelled;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			result = Rtt::TouchEvent::kBegan;
			break;
	}

	return result;
}

static void
InitializeEvents( CoronaView *view, Rtt::TouchEvent *touchEvents, NSSet *touches, CFMutableDictionaryRef touchesData )
{
	using namespace Rtt;

	UITouch *touch = touches.anyObject;
	TouchEvent::Phase phase = UITouchPhaseToTouchEventPhase( touch.phase );

	int i = 0;
	for ( UITouch *t in touches )
	{
		CGPoint touchPoint = [t locationInCoronaView:view];
		
		CGPoint *startPoint = (CGPoint *)CFDictionaryGetValue( touchesData, t );
		if ( TouchEvent::kBegan == phase )
		{
			if ( Rtt_VERIFY( ! startPoint ) )
			{
				startPoint = (CGPoint *)malloc(sizeof(CGPoint));
				CFDictionarySetValue( touchesData, t, startPoint );
			}
			
			* startPoint = touchPoint;
		}
		
		// Just in case we somehow didn't store the startPoint
		if ( ! Rtt_VERIFY( startPoint ) )
		{
			startPoint = & touchPoint;
		}

		TouchEvent::Phase eventPhase = phase;
		if ( eventPhase >= TouchEvent::kNumPhases )
		{
			eventPhase = UITouchPhaseToTouchEventPhase( t.phase );
		}
        Rtt::Real pressure = [CoronaView getTouchForce:touch];

		// Initialize TouchEvent (use placement new to call c-tor)
		TouchEvent *event = & ( touchEvents[i] );
		new( event ) TouchEvent( touchPoint.x, touchPoint.y, startPoint->x, startPoint->y, eventPhase, pressure );
		event->SetId( t );

		i++;
		
		// On iOS 8.1 on iPhone 6+, once CFDictionaryRemoveValue is called, the value for the key
		// is also deleted.  This is because of the TouchesDataValueRelease function we pass in
		// when creating the dictionary
		if ( TouchEvent::kEnded == phase || TouchEvent::kCancelled == phase )
		{
			CFDictionaryRemoveValue( touchesData, t );
		}
	}
}

- (void)dispatchTouches:(NSSet *)touches withEvent:(UIEvent *)event
{
	using namespace Rtt;

	Rtt_ASSERT( 0 == fInhibitCount );

	if ( ! fTouchesData )
	{
		CFDictionaryValueCallBacks valueCallbacks = { 0, NULL, & TouchesDataValueRelease, NULL, NULL };
		fTouchesData = CFDictionaryCreateMutable( NULL, 4, NULL, & valueCallbacks );
	}

	int numEvents = (int)touches.count;
	if ( numEvents > 0 )
	{
		TouchEvent *touchEvents = (TouchEvent*)malloc( sizeof( TouchEvent ) * numEvents );
		Rtt_ASSERT( touchEvents );

		InitializeEvents( self, touchEvents, touches, fTouchesData );

		MultitouchEvent t( touchEvents, numEvents );
		[self dispatchEvent: (&t)];

		free( touchEvents );
	}
}

// #define Rtt_DEBUG_TOUCH 1

#ifdef Rtt_DEBUG_TOUCH
static void
PrintTouches( NSSet *touches, const char *header )
{
	const char *prefix = "";
	if ( header )
	{
		prefix = "\t";
		Rtt_TRACE( ( "%s\n", header ) );		
	}

	for ( UITouch *t in touches )
	{
		Rtt_TRACE( ( "%sTouch(%x) phase(%d)\n", prefix, t, t.phase ) );
	}

	Rtt_TRACE( ( "\n" ) );
}
#endif

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
#ifdef Rtt_DEBUG_TOUCH
	PrintTouches( event.allTouches, "BEGAN" );
#endif

	if ( fInhibitCount > 0 ) { return; }

	UITouch *touch = touches.anyObject;
	fStartTouchPosition = [touch locationInCoronaView:self];

#ifdef Rtt_MULTITOUCH
	if ( self.multipleTouchEnabled )
	{
		[self dispatchTouches:touches withEvent:event];
	}
	else
#endif
	{
#ifdef Rtt_TVOS_ENV
		CGPoint currentTouchPosition = { [touch locationInView:nil].x - [self center].x, [touch locationInView:nil].y - [self center].y };
		Rtt::RelativeTouchEvent t( currentTouchPosition.x, currentTouchPosition.y, Rtt::TouchEvent::kBegan );
#else
		Rtt::Real pressure = [CoronaView getTouchForce:touch];
		Rtt::TouchEvent t( fStartTouchPosition.x, fStartTouchPosition.y, fStartTouchPosition.x, fStartTouchPosition.y, Rtt::TouchEvent::kBegan, pressure );
#endif
		t.SetId( touch );
		[self dispatchEvent: (&t)];
	}

	// Rtt_TRACE(  ( "touch(%p)\n\tphase(%d)\n", touch, touch.phase ) );
	// Rtt_TRACE(  ( "touch(%p)\n\tbegin(%d)\n", touch, touch.tapCount ) );

	if ( fTapDelay > 0. && touch.tapCount > 1 )
	{
		[NSObject cancelPreviousPerformRequestsWithTarget:self]; // selector:@selector(dispatchTapEvent:) object:@"tapSingle"];
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
#ifdef Rtt_DEBUG_TOUCH
	PrintTouches( event.allTouches, "MOVED" );
#endif

	if ( fInhibitCount > 0 ) { return; }

	UITouch *touch = touches.anyObject;
	CGPoint currentTouchPosition;

#ifdef Rtt_MULTITOUCH
	if ( self.multipleTouchEnabled )
	{
		[self dispatchTouches:touches withEvent:event];
	}
	else
#endif
	{
#ifdef Rtt_TVOS_ENV
		currentTouchPosition = { [touch locationInView:self].x - [self center].x, [touch locationInView:self].y - [self center].y };
		Rtt::RelativeTouchEvent t( currentTouchPosition.x, currentTouchPosition.y, Rtt::TouchEvent::kMoved );
#else
		currentTouchPosition = [touch locationInCoronaView:self];
		Rtt::Real pressure = [CoronaView getTouchForce:touch];
		Rtt::TouchEvent t( currentTouchPosition.x, currentTouchPosition.y, fStartTouchPosition.x, fStartTouchPosition.y, Rtt::TouchEvent::kMoved, pressure );
#endif
		t.SetId( touch );
		[self dispatchEvent: (&t)];
	}

	// Rtt_TRACE(  ( "touch(%p)\n\tphase(%d)\n", touch, touch.phase ) );
	// Rtt_TRACE(  ( "touch(%p)\n\tmove(%d)\n", touch, touch.tapCount ) );

#ifndef Rtt_TVOS_ENV
	Rtt::DragEvent e( fStartTouchPosition.x, fStartTouchPosition.y, currentTouchPosition.x, currentTouchPosition.y );
	[self dispatchEvent: (&e)];
#endif
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
#ifdef Rtt_DEBUG_TOUCH
	PrintTouches( event.allTouches, "ENDED" );
#endif

	if ( fInhibitCount > 0 ) { return; }

	UITouch *touch = touches.anyObject;

#ifdef Rtt_MULTITOUCH
	if ( self.multipleTouchEnabled )
	{
		[self dispatchTouches:touches withEvent:event];
	}
	else
#endif
	{
#ifdef Rtt_TVOS_ENV
		CGPoint currentTouchPosition = { [touch locationInView:self].x - [self center].x, [touch locationInView:self].y - [self center].y };
		Rtt::RelativeTouchEvent t( currentTouchPosition.x, currentTouchPosition.y, Rtt::TouchEvent::kEnded, [touch tapCount] );
#else
		CGPoint currentTouchPosition = [touch locationInCoronaView:self];
		Rtt::Real pressure = [CoronaView getTouchForce:touch];
		Rtt::TouchEvent t( currentTouchPosition.x, currentTouchPosition.y, fStartTouchPosition.x, fStartTouchPosition.y, Rtt::TouchEvent::kEnded, pressure );
#endif
		t.SetId( touch );
		[self dispatchEvent: (&t)];
	}

	// Rtt_TRACE(  ( "touch(%p)\n\tphase(%d)\n", touch, touch.phase ) );
	
	// If the touch moved, the tapCount is zero
	if ( touch.tapCount > 0 )
	{
#ifdef Rtt_TVOS_ENV
		// On tvOS, we provide a key event for the tap rather than a true tap event.
		// The tap count can still be obtained from ended-phase "RelativeTouchEvent" events.
		Rtt::KeyEvent e( NULL, Rtt::KeyEvent::kDown, Rtt::KeyName::kButtonZ, 0, false, false, false, false );
		[self dispatchEvent:(&e)];
#else
		TapEventWrapper* e = [[TapEventWrapper alloc] initWithTouch:touch inView:self];

		NSTimeInterval delayInSeconds = fTapDelay;
		if ( delayInSeconds > 0. )
		{
			[self performSelector:@selector(dispatchTapEvent:) withObject:e afterDelay:delayInSeconds];
		}
		else
		{
			[self dispatchTapEvent:e];
		}

		[e release];
#endif
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
#ifdef Rtt_DEBUG_TOUCH
	PrintTouches( event.allTouches, "CANCELLED" );
#endif

	if ( fInhibitCount > 0 ) { return; }

#ifdef Rtt_MULTITOUCH
	// Multitouch
	if ( self.multipleTouchEnabled )
	{
		[self dispatchTouches:touches withEvent:event];
	}
	else
#endif
	{
		UITouch *touch = touches.anyObject;
		
#ifdef Rtt_TVOS_ENV
		CGPoint currentTouchPosition = { [touch locationInView:self].x - [self center].x, [touch locationInView:self].y - [self center].y };
		Rtt::RelativeTouchEvent t( currentTouchPosition.x, currentTouchPosition.y, Rtt::TouchEvent::kCancelled );
#else
		CGPoint currentTouchPosition = [touch locationInCoronaView:self];
		Rtt::Real pressure = [CoronaView getTouchForce:touch];
		Rtt::TouchEvent t( currentTouchPosition.x, currentTouchPosition.y, fStartTouchPosition.x, fStartTouchPosition.y, Rtt::TouchEvent::kCancelled, pressure );
#endif
		t.SetId( touch );
		[self dispatchEvent: (&t)];
	}
}

- (void)pollAndDispatchMotionEvents
{
#ifdef Rtt_CORE_MOTION
	// Gyroscope
	if ( gyroscopeEnabled )
	{
		U64 currentTimeCorona = Rtt_AbsoluteToMilliseconds( Rtt_GetAbsoluteTime() );
		CMMotionManager *motion = [CoronaSystemResourceManager sharedInstance].motionManager;

		// Don't callback more frequently than the user's gyroUpdateInterval
		NSTimeInterval updateInterval = motion.gyroUpdateInterval;
		U64 updateIntervalMS = (U64)(updateInterval * 1000);
		if ( (currentTimeCorona - gyroscopePreviousTimestampCorona) >= updateIntervalMS )
		{
			gyroscopePreviousTimestampCorona = currentTimeCorona;

			CMGyroData *gyroData = motion.gyroData;
			
			// Initial timestamp is 0, so only pay attention to subsequent data
			if ( 0.0 != gyroscopePreviousTimestamp )
			{
				Rtt::GyroscopeEvent e(
					gyroData.rotationRate.x, gyroData.rotationRate.y, gyroData.rotationRate.z,
					gyroData.timestamp - gyroscopePreviousTimestamp );
				self.runtime->DispatchEvent( e );
			}

			gyroscopePreviousTimestamp = gyroData.timestamp;
		}
	}
#endif // Rtt_CORE_MOTION
}

- (void)drawView
{
	[self pollAndDispatchMotionEvents];

	if ( _runtime )
	{
		(*_runtime)();
	}
}

- (void)didAddSubview:(UIView *)subview
{
	//It appears that adding subViews can cause the GL window to be flushed
	//This will detect adding subviews and force a re-render.
	//This seems to happen late enough in the process that we can lazilly invalidate
	//after the next DrawRect call
	[super didAddSubview:subview];
	fShouldInvalidate = true;
}

- (void)didMoveToWindow
{
	if ( [self.window.screen respondsToSelector:@selector(nativeScale)] )
	{
		self.contentScaleFactor = self.window.screen.nativeScale;
	}
}

- (void)drawRect:(CGRect)rect
{
	//This check is required due to radar://20416615
	//OpenGL commands are not permitted in background, but in some edge cases, drawRect is called on lock screen.
	if ( _runtime && ! _runtime->IsSuspended() )
	{
		[self drawView];
	}
	
	if ( fShouldInvalidate )
	{
		if ( _runtime )
		{
			_runtime->GetDisplay().Invalidate();
			fShouldInvalidate = false;
		}
	}
}

/*
- (void)flush
{

	// Flush
	Rtt_ASSERT( context == [Rtt_EAGLContext currentContext] );

	// This is a check to make sure the correct render buffer is bound.
	// Normally, this wouldn't ever happen, but there's a check here in
	// case we need it
	#if 0
	{
		// glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
		GLint currentBuffer = 0;
		glGetIntegerv( GL_RENDERBUFFER_BINDING, & currentBuffer );
		Rtt_ASSERT( (GLuint)currentBuffer == viewRenderbuffer );
	}
	#endif

	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}
*/

- (void)setFrame:(CGRect)frame
{
	[super setFrame:frame];
	fShouldInvalidate = true;
}

- (void)setBounds:(CGRect)bounds
{
	[super setBounds:bounds];
	fShouldInvalidate = true;
}

// CoronaOrientationObserver
// ----------------------------------------------------------------------------
#ifdef Rtt_ORIENTATION
- (void)didOrientationChange:(id)sender
{
	using namespace Rtt;

	const UIDeviceOrientation orientation = [UIDevice currentDevice].orientation;

	if ( orientation != UIDeviceOrientationUnknown && _runtime)
	{
		Runtime *runtime = self.runtime;
		const MPlatform& platform = runtime->Platform();

		// Store the given orientation and retrieve the previous orientation.
		DeviceOrientation::Type currentType = IPhoneDevice::ToOrientationTypeFromUIDeviceOrientation( orientation );
		DeviceOrientation::Type previousType =
			static_cast< IPhoneDevice& >( platform.GetDevice() ).GetPreviousOrientationAndUpdate( orientation );
		
		// Raise an orientation event if the orientation has changed.
		if ((previousType != DeviceOrientation::kUnknown) && (currentType != previousType))
		{
			OrientationEvent e( currentType, previousType );
			runtime->DispatchEvent( e );
		}
		
		// Raise a resize event if the screen has been resized.
		// We raise this event here because we assume resizes only occur when changing orientations.
		int currentContentHeight = (int)runtime->GetDisplay().ContentHeight();
		if ((fLastContentHeight >= 0) && (fLastContentHeight != currentContentHeight))
		{
			runtime->DispatchEvent( ResizeEvent() );
		}
		fLastContentHeight = currentContentHeight;
	}
}
#endif

#pragma mark # UITraitEnvironment

// Called when a horizontal or vertical size class, display scale, or user interface idiom changes on an iOS 8.0+ device.
// This is where we should check for new accessibility settings as well, such as if the user disabled 3D Touch. Values
// updated here should be considered unreliable prior to the first change (performed when the app has finished launching
// but before any lua code is executed).
- (void)traitCollectionDidChange:(UITraitCollection *)previousTraitCollection {
	[super traitCollectionDidChange:previousTraitCollection];
}

#ifdef Rtt_CORE_LOCATION

// CLLocationManagerDelegate
// ----------------------------------------------------------------------------
#pragma mark # CLLocationManagerDelegate

- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{
	using namespace Rtt;

	double latitude = newLocation.coordinate.latitude;
	double longitude = newLocation.coordinate.longitude;
	double altitude = newLocation.altitude;
	double accuracy = newLocation.horizontalAccuracy;
	double speed = newLocation.speed;
	double direction = newLocation.course;
	double time = [newLocation.timestamp timeIntervalSince1970];

	LocationEvent e( latitude, longitude, altitude, accuracy, speed, direction, time );
	self.runtime->DispatchEvent( e );
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
	using namespace Rtt;

	if ( kCLErrorLocationUnknown == error.code )
	{
		// Ignore this "error" which seems to occur spuriously
		// Apple says: kCLErrorLocationUnknown: location is currently unknown, but CL will keep trying
		return;
	}

	if ( kCLErrorDenied == error.code ) 
	{
		self.runtime->Platform().GetDevice().EndNotifications( MPlatformDevice::kLocationEvent );
		self.runtime->Platform().GetDevice().EndNotifications( MPlatformDevice::kHeadingEvent );
	}

	LocationEvent e( [[error localizedDescription] UTF8String], (S32)[error code] );
	self.runtime->DispatchEvent( e );

/*
	NSString *msg;
	if (error.code == kCLErrorLocationUnknown) 
	{
		msg = @"Unable to obtain current location";
	} 
	else if (error.code == kCLErrorDenied) 
	{
		msg = @"Location access denied";
		runtime->Platform().GetDevice().EndNotifications( MPlatformDevice::kLocationEvent );
		runtime->Platform().GetDevice().EndNotifications( MPlatformDevice::kHeadingEvent );
	} 
	else if (error.code == kCLErrorNetwork)
	{
		msg = @"Network unavailable";
	}

	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Location Error"
													message:msg
												   delegate:nil
										  cancelButtonTitle:@"OK"
										  otherButtonTitles:nil];
	[alert show];
	[alert release];
*/
}

- (void)locationManager:(CLLocationManager *)manager didUpdateHeading:(CLHeading *)newHeading
{	
	using namespace Rtt;
	
	double headingTrue = (double)[newHeading trueHeading];
	double headingMagnetic = (double)[newHeading magneticHeading];

	HeadingEvent e( headingTrue, headingMagnetic );
	self.runtime->DispatchEvent( e );
}

#endif // Rtt_CORE_LOCATION

// CoronaGyroscopeObserver
// ----------------------------------------------------------------------------
#pragma mark # CoronaGyroscopeObserver

- (void)startPolling
{
	if ( ! gyroscopeEnabled )
	{
		gyroscopeEnabled = YES;
		gyroscopePreviousTimestampCorona = Rtt_AbsoluteToMilliseconds( Rtt_GetAbsoluteTime() );
		gyroscopePreviousTimestamp = 0.0;
	}
}

- (void)stopPolling
{
	gyroscopeEnabled = NO;
}


// CoronaViewOrientation
// ----------------------------------------------------------------------------
#pragma mark # CoronaViewOrientation

- (BOOL)isUpright
{
	BOOL result = NO;

#ifdef Rtt_ORIENTATION
	UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
	if (orientation == UIDeviceOrientationPortrait || orientation == UIDeviceOrientationPortraitUpsideDown)
	{
		result = YES;
	}
#else
	result = Rtt::DeviceOrientation::IsUpright( fLoadOrientation );
#endif

	return result;
}


// UITextFieldDelegate
// ----------------------------------------------------------------------------
#pragma mark # UITextFieldDelegate

static void
DispatchEvent( Rtt_UITextField *textField, Rtt::UserInputEvent::Phase phase )
{
	using namespace Rtt;

	IPhoneTextFieldObject *o = (textField).owner;
	if ( o )
	{
		UserInputEvent e ( phase );
		o->DispatchEventWithTarget( e );
	}
}

static void
DispatchEvent( Rtt_UITextField *textField, int startpos, int numdeleted, const char* replacementstring, const char* oldstring, const char* newstring )
{
	using namespace Rtt;

	IPhoneTextFieldObject *o = (textField).owner;
	if ( o )
	{
		UserInputEvent e( startpos, numdeleted, replacementstring, oldstring, newstring );
		o->DispatchEventWithTarget( e );
	}
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	DispatchEvent( (Rtt_UITextField*)textField, Rtt::UserInputEvent::kSubmitted );

	return NO;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
	Rtt_ASSERT( ! fActiveText );

	fActiveText = textField;

	DispatchEvent( (Rtt_UITextField*)textField, Rtt::UserInputEvent::kBegan );
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
	Rtt_ASSERT( fActiveText == textField );

	fActiveText = nil;

	DispatchEvent( (Rtt_UITextField*)textField, Rtt::UserInputEvent::kEnded );
}

- (BOOL)textField:(UITextField*)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString*)string
{
	// location (first number) is the start position of the string
	// length (second number) is the length or number of the characters replaced/deleted
//	NSLog(@"%@, %@, %@", NSStringFromSelector(_cmd), NSStringFromRange(range), string );

	if (((Rtt_UITextField*)textField).owner->rejectEmoji([string UTF8String]))
	{
		return NO;
	}

	// We're worried about dispatching an event to Lua here before we return from this delegate callback.
	// While Apple doesn't explicitly say anything about this, it seems a little dangerous.
	// To workaround, we can do a performSelector:withDelay to queue up the event for later handling.
	// One thing to test though is that the "ended" event doesn't appear out of order.
	
	// Must release object on other side of performSelector
	Rtt_AppleTextDelegateWrapperObjectHelper* eventData = [[Rtt_AppleTextDelegateWrapperObjectHelper alloc] init];
	eventData.textWidget = textField;
	eventData.theRange = range;
	eventData.replacementString = string;
	eventData.originalString = textField.text;

	// HACK
	// The delay is there because there seems to have been a internal iOS7 change where the textField.text property was changed at a different timing.
	// Its there as a hack to get the code working with minimal changes.  The correct way would be to run the method after a textDidChange event.
	[self performSelector:@selector(dispatchEditingEventForTextField:) withObject:eventData afterDelay:0.05];
	
	return YES;
}

- (void) dispatchEditingEventForTextField:(Rtt_AppleTextDelegateWrapperObjectHelper*)eventData
{
	Rtt_UITextField* textField = (Rtt_UITextField*)eventData.textWidget;
	NSRange range = eventData.theRange;
	NSString* string = eventData.replacementString;
	NSString* oldstring = eventData.originalString;
	NSString* newstring = textField.text;

	// Don't forget to add 1 for Lua index conventions
	DispatchEvent( textField, (int)(range.location + 1), (int)range.length, [string UTF8String], [oldstring UTF8String], [textField.text UTF8String] );
	
	// Data must be released or it will leak because it created so it could be passed through this callback.
	[eventData release];
}

// UITextViewDelegate
// ----------------------------------------------------------------------------
#pragma mark # UITextViewDelegate

static void
DispatchEvent( Rtt_UITextView *textView, Rtt::UserInputEvent::Phase phase )
{
	using namespace Rtt;

	IPhoneTextBoxObject *o = (textView).owner;
	if ( o )
	{
		UserInputEvent e( phase );
		o->DispatchEventWithTarget( e );
	}
}

static void
DispatchEvent( Rtt_UITextView *textView, int startpos, int numdeleted, const char* replacementstring, const char* oldstring, const char* newstring )
{
	using namespace Rtt;
    
	IPhoneTextBoxObject *o = (textView).owner;
	if ( o )
	{
		UserInputEvent e( startpos, numdeleted, replacementstring, oldstring, newstring );
		o->DispatchEventWithTarget( e );
	}
}

- (void)textViewDidBeginEditing:(UITextView *)textView
{
	Rtt_ASSERT( ! fActiveText );

	fActiveText = textView;

	DispatchEvent( (Rtt_UITextView*)textView, Rtt::UserInputEvent::kBegan );
}

- (void)textViewDidEndEditing:(UITextView *)textView
{
	if(kCFCoreFoundationVersionNumber < kCFCoreFoundationVersionNumber_iPhoneOS_3_1)
	{
		// IPhone BUG pre-OS 3.1: Spurious add'l invocation when forceable dismissing
		// the keyboard --- may be related to the bug where setting the "editable"
		// property of textview automatically brings up keyboard.
		if ( ! fActiveText )
		{
			return;
		}
	}

	Rtt_VERIFY( fActiveText == textView );

	fActiveText = nil;

	Rtt_UITextView* tView = (Rtt_UITextView*)textView;
	[tView updatePlaceholder];
	
	DispatchEvent( tView, Rtt::UserInputEvent::kEnded );
}

- (BOOL)textView:(UITextView*)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString*)string
{
	// location (first number) is the start position of the string
	// length (second number) is the length or number of the characters replaced/deleted
//	NSLog(@"%@, %@, %@", NSStringFromSelector(_cmd), NSStringFromRange(range), string );
	
	// We're worried about dispatching an event to Lua here before we return from this delegate callback.
	// While Apple doesn't explicitly say anything about this, it seems a little dangerous.
	// To workaround, we can do a performSelector:withDelay to queue up the event for later handling.
	// One thing to test though is that the "ended" event doesn't appear out of order.

	// Must release object on other side of performSelector
	Rtt_AppleTextDelegateWrapperObjectHelper* eventData = [[Rtt_AppleTextDelegateWrapperObjectHelper alloc] init];
	eventData.textWidget = textView;
	eventData.theRange = range;
	eventData.replacementString = string;
	eventData.originalString = textView.text;
	
	// HACK
	// The delay is there because there seems to have been a internal iOS7 change where the textField.text property was changed at a different timing.
	// Its there as a hack to get the code working with minimal changes.  The correct way would be to run the method after a textDidChange event.
	[self performSelector:@selector(dispatchEditingEventForTextView:) withObject:eventData afterDelay:0.05];

	return YES;
}

- (void) dispatchEditingEventForTextView:(Rtt_AppleTextDelegateWrapperObjectHelper*)eventData
{
	Rtt_UITextView* textView = (Rtt_UITextView*)eventData.textWidget;
	NSRange range = eventData.theRange;
	NSString* string = eventData.replacementString;
	NSString* oldstring = eventData.originalString;
	
	// Don't forget to add 1 for Lua index conventions
	DispatchEvent( textView, (int)(range.location + 1), (int)range.length, [string UTF8String], [oldstring UTF8String], [textView.text UTF8String] );

	// Data must be released or it will leak because it created so it could be passed through this callback.
	[eventData release];
}

// Keyboard
// ----------------------------------------------------------------------------
#pragma mark # Keyboard

- (void)dismissKeyboard
{
	[fActiveText resignFirstResponder];
}

#define SYSTEM_VERSION_LESS_THAN(v)                 ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedAscending)


#ifndef Rtt_TVOS_ENV

// Called when the UIKeyboardWillShowNotification is sent
- (void)keyboardWillShow:(NSNotification*)aNotification
{
	if ( ! fKeyboardShown && fActiveText )
	{
		if ( fCoronaWindowMovesWhenKeyboardAppears
			 && CGPointEqualToPoint( fWindowCenterStart, CGPointZero ) )
		{
			NSDictionary* info = [aNotification userInfo];

			// Get the frame of the keyboard at the *end* of the transition
			NSValue* aValue = [info objectForKey:UIKeyboardFrameEndUserInfoKey];
			CGRect keyboardRect = [aValue CGRectValue];

			CGRect activeTextRect = fActiveText.frame;

			// When the top edge of keyboard is above bottom edge of active text,
			// offset the window
			CGFloat xOffset = 0.f;
			CGFloat yOffset = 0.f;
			
			bool useDefaultOffset = true;
			if ( SYSTEM_VERSION_LESS_THAN( @"8.0" ) )
			{
#ifdef Rtt_ORIENTATION
				UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];

				switch ( orientation )
				{
					case UIInterfaceOrientationLandscapeRight:
					{
						xOffset = CGRectGetMinY( activeTextRect ) - CGRectGetMaxX( keyboardRect );
						useDefaultOffset = false;
						break;
					}

					case UIInterfaceOrientationLandscapeLeft:
					{
						xOffset = CGRectGetMinX( keyboardRect ) - CGRectGetMaxY( activeTextRect );
						useDefaultOffset = false;
						break;
					}

					case UIInterfaceOrientationPortraitUpsideDown:
					{
						yOffset = CGRectGetMaxY( keyboardRect ) - (self.bounds.size.height - CGRectGetMaxY( activeTextRect ));
						useDefaultOffset = false;
						break;
					}
					
					default:
						break;
				}
#endif
			}
			
			if ( useDefaultOffset )
			{
				yOffset = CGRectGetMinY( keyboardRect ) - CGRectGetMaxY( activeTextRect );
			}

			if ( yOffset < 0.f || ( ! useDefaultOffset ) )
			{
				NSTimeInterval animationDuration;
				[[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&animationDuration];

				UIView *v = self.window;

				fWindowCenterStart = v.center;
				fWindowOffset = CGPointMake( xOffset, yOffset );
//				fKeyboardOffset = yOffset;

				[UIView beginAnimations:nil context:NULL];
				[UIView animateWithDuration:animationDuration
					delay:0.f
					options:UIViewAnimationOptionBeginFromCurrentState
					animations:^(void)
					{
						CGPoint newCenter = fWindowCenterStart;
						newCenter.x += fWindowOffset.x;
						newCenter.y += fWindowOffset.y;
						v.center = newCenter;
					}
					completion:nil];
				[UIView commitAnimations];

			}
			else
			{
				fWindowCenterStart = CGPointZero;
				fWindowOffset = CGPointZero;
//				fKeyboardOffset = 0.f;
			}
		}

		fKeyboardShown = YES;
	}
}

// Called when the UIKeyboardDidHideNotification is sent
- (void)keyboardWillHide:(NSNotification*)aNotification
{
	if ( fKeyboardShown )
	{
		if ( fCoronaWindowMovesWhenKeyboardAppears )
		{
			NSDictionary* info = [aNotification userInfo];

			NSTimeInterval animationDuration;
			[[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&animationDuration];

			UIView *v = self.window;

//			CGFloat yOffset = fKeyboardOffset;
			
			if ( fWindowOffset.y < 0.f
				 || ( SYSTEM_VERSION_LESS_THAN( @"8.0" ) && ! CGPointEqualToPoint( fWindowOffset, CGPointZero ) ) )
			{
				[UIView beginAnimations:nil context:NULL];
				[UIView animateWithDuration:animationDuration
					delay:0.f
					options:UIViewAnimationOptionBeginFromCurrentState
					animations:^(void)
					{
						v.center = fWindowCenterStart;
						fWindowCenterStart = CGPointZero;
					}
					completion:nil];
				[UIView commitAnimations];
			}
		}
		
		fKeyboardShown = NO;
	}
}

#endif

//
// ----------------------------------------------------------------------------
#pragma mark #


@end
