//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <AppKit/NSOpenGL.h>

#include "Core/Rtt_Build.h"

#import "CoronaViewPrivate.h"

#include "Rtt_DeviceOrientation.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Rtt_MacViewCallback.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Display.h"
#include "CoronaViewRuntimeDelegate.h"

#include "Rtt_ProjectSettings.h"
#include "Rtt_NativeWindowMode.h"

#include "Rtt_AppleInputDeviceManager.h"
#include "Rtt_AppleInputHIDDeviceListener.h"
#include "Rtt_AppleInputMFiDeviceListener.h"

extern "C" {
#include "lua.h"
}

#ifdef Rtt_DEBUG
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#else
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#endif

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

@interface CoronaView()
{
@private
	int fSuspendCount;
}


@end

@implementation CoronaView

@synthesize _projectPath;
@synthesize _GLView;
@synthesize _viewDelegate;
@synthesize _runtime;
@synthesize _platform;
@synthesize _runtimeDelegate;
@synthesize _projectSettings;
@synthesize _locationManager;
@synthesize _currentLocation;
@synthesize _launchParams;


#if !defined( Rtt_AUTHORING_SIMULATOR )
Rtt_EXPORT const luaL_Reg* Rtt_GetCustomModulesList()
{
	return NULL;
}
#endif

- (NSInteger)run
{
    return [self runWithPath:_projectPath parameters:nil];
}

// Note: Should only be called in CoronaCards contexts
- (NSInteger)runWithPath:(NSString*)path parameters:(NSDictionary *)params
{
    NSDEBUG(@"CoronaView: runWithPath: %@ parameters: %@", path, params);
    using namespace Rtt;

	_launchParams = [params retain];

    NSInteger result = (NSInteger)Runtime::kSuccess;
    
    MacPlatform *platform = new MacPlatform(self);

	// Sanity check
    if (! [[NSFileManager defaultManager] isReadableFileAtPath:[path stringByAppendingPathComponent:@"main.lua"]] &&
        ! [[NSFileManager defaultManager] isReadableFileAtPath:[path stringByAppendingPathComponent:@"main.lu"]]&&
        ! [[NSFileManager defaultManager] isReadableFileAtPath:[path stringByAppendingPathComponent:@"resource.car"]])
	{
		NSTextView *accessory = [[NSTextView alloc] initWithFrame:NSMakeRect(0,0,400,15)];
		NSFont *msgFont = [NSFont userFontOfSize:[NSFont smallSystemFontSize]];
		NSFont *fixedFont = [NSFont userFixedPitchFontOfSize:[NSFont smallSystemFontSize]];
		NSDictionary *pathAttributes = [NSDictionary dictionaryWithObject:fixedFont forKey:NSFontAttributeName];
		NSString *link = @"http://docs.coronalabs.com/coronacards/osx/index.html";
		NSString *linkText = @"\n\nCoronaCards documentation";
		NSDictionary *linkAttributes = [NSDictionary dictionaryWithObject:link forKey:NSLinkAttributeName];
		NSString *msgText = @"\n\n(no main.lua found)";
		NSDictionary *msgAttributes = [NSDictionary dictionaryWithObject:msgFont forKey:NSFontAttributeName];
		[accessory insertText:[[NSAttributedString alloc] initWithString:path
															  attributes:pathAttributes]];
		[accessory insertText:[[NSAttributedString alloc] initWithString:linkText
															  attributes:linkAttributes]];
		[accessory insertText:[[NSAttributedString alloc] initWithString:msgText
															  attributes:msgAttributes]];
		[accessory setEditable:NO];
		[accessory setDrawsBackground:NO];

		NSAlert *alert = [[[NSAlert alloc] init] autorelease];
		[alert setMessageText:@"CoronaCards Error"];
		[alert setInformativeText:@"Cannot load Corona project:"];
		[alert setAccessoryView:accessory];
		[alert runModal];

		[NSApp terminate:nil];

		return Runtime::kGeneralFail;
	}

	_projectPath = [path retain];
	platform->SetResourcePath( [_projectPath UTF8String] );

	platform->Initialize( self.glView );

	_platform = platform;
	
	[self initializeRuntime];

	// Initialize Joystick Support:
	AppleInputDeviceManager& macDeviceManager = (AppleInputDeviceManager&)_runtime->Platform().GetDevice().GetInputDeviceManager();
	
    // MFI:
	_macMFiDeviceListener = [[AppleInputMFiDeviceListener alloc] initWithRuntime:_runtime andDeviceManager:&macDeviceManager];
	[_macMFiDeviceListener start];
	
	//HID
    _macHIDInputDeviceListener = new Rtt::AppleInputHIDDeviceListener;
    _macHIDInputDeviceListener->StartListening(_runtime, &macDeviceManager);
	
    return result;
}

- (void)suspend
{
	if ( _runtime )
	{
		Rtt_ASSERT( fSuspendCount >= 0 );

		++fSuspendCount;

		if ( fSuspendCount == 1 )
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
			--fSuspendCount;

			if ( fSuspendCount == 0 )
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
	// Destroy runtime first
	delete _runtime;
	_runtime = NULL;

	delete _runtimeDelegate;
	_runtimeDelegate = NULL;

	delete _platform;
	_platform = NULL;

	[_GLView removeFromSuperview];
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
		Lua::DispatchRuntimeEvent( L, 1 );

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

- (void) initializeRuntime
{
	 // [self initializeRuntimeWithPlatform:NULL runtimeDelegate:NULL];
	 using namespace Rtt;

	Rtt_ASSERT( ! _runtime );

	if ( Rtt_VERIFY( [self _platform] ) )
	{
		// Launch
		Runtime *runtime = new Runtime( * [self _platform], _GLViewCallback );

		if ( Rtt_VERIFY( runtime ) )
		{
			_runtime = runtime;
			_GLView.runtime = runtime;

			_runtimeDelegate = ( _runtimeDelegate ? _runtimeDelegate : new CoronaViewRuntimeDelegate( self ) );
			_runtime->SetDelegate( _runtimeDelegate );

            if ([[NSFileManager defaultManager] isReadableFileAtPath:[_projectPath stringByAppendingPathComponent:@"resource.car"]])
            {
                _runtime->SetProperty( Runtime::kIsApplicationNotArchived, false );
            }
            else
            {
                _runtime->SetProperty( Runtime::kIsApplicationNotArchived, true );
            }
			
			_runtime->SetProperty(Runtime::kIsSimulatorExtension, true);
			_runtime->SetProperty(Runtime::kIsLuaParserAvailable, true);
			_runtime->SetProperty(Runtime::kRenderAsync, true);
            _runtime->SetProperty(Runtime::kIsCoronaKit, true);

#ifndef Rtt_AUTHORING_SIMULATOR
			_runtime->SetProperty(Runtime::kIsLuaParserAvailable, true);
#endif
			_GLViewCallback->Initialize( _runtime );

			id delegate = _viewDelegate;
			if ( [delegate respondsToSelector:@selector(willLoadApplication:)] )
			{
				[delegate willLoadApplication:self];
			}

			_platform->SetProjectResourceDirectory([_projectPath UTF8String]);

			// Load the project's "build.settings" and "config.lua" file first.
			// Used to fetch supported orientations, supported image suffix scales, and content width/height.
			_projectSettings->LoadFromDirectory([_projectPath UTF8String]);

			_GLView.isResizable = _projectSettings->IsWindowResizable();
			
			Rtt::DeviceOrientation::Type orientation = _projectSettings->GetDefaultOrientation();
			if (Rtt::DeviceOrientation::IsSideways( orientation ))
			{
				[self setFrameSize:NSMakeSize(CoronaViewPrivateDefaultHeight, CoronaViewPrivateDefaultWidth)];
			}
			else
			{
				[self setFrameSize:NSMakeSize(CoronaViewPrivateDefaultWidth, CoronaViewPrivateDefaultHeight)];
			}
		}
	}
}

- (void)didPrepareOpenGLContext:(id)sender
{
	NSDEBUG(@"CoronaView: didPrepareOpenGLContext: %@", NSStringFromRect([self frame]));

	if ([_coronaViewDelegate respondsToSelector:@selector(didPrepareOpenGLContext:)])
	{
		// Give the host a pointer to the GLView if it wants it
		[_coronaViewDelegate didPrepareOpenGLContext:_GLView];
	}


#ifdef Rtt_AUTHORING_SIMULATOR
	U32 launchOptions = Rtt::Runtime::kCoronaViewOption;
#else
	U32 launchOptions = Rtt::Runtime::kCoronaCardsOption;
#endif

    Rtt::DeviceOrientation::Type orientation = _projectSettings->GetDefaultOrientation();
    int width = _projectSettings->GetContentWidth();
    int height = _projectSettings->GetContentHeight();
	
	if (width == 0 || height == 0)
	{
		width = CoronaViewPrivateDefaultWidth;
		height = CoronaViewPrivateDefaultHeight;
	}
	
    if (orientation == Rtt::DeviceOrientation::kUnknown)
    {
        if (width > height)
        {
            orientation = Rtt::DeviceOrientation::kSidewaysLeft;
        }
        else
        {
            orientation = Rtt::DeviceOrientation::kUpright;
        }
    }
    
    // FIXME: Bad things happen if the view gets too small, arbitrarily restrict it for now
    if ([[self window] minSize].width == 0 || [[self window] minSize].height == 0)
    {
        [[self window] setMinSize:NSMakeSize(50, 50)];
    }
    
    [_GLView setOrientation:orientation];
	
	if (  Rtt::Runtime::kSuccess == _runtime->LoadApplication( launchOptions, orientation ) )
	{
        // Now that we've loaded the application, width and height may have changed (this has to do
        // with whether there are runtime dependent items, e.g. "display.pixelWidth", in the config.lua)
        width = _projectSettings->GetContentWidth();
        height = _projectSettings->GetContentHeight();

		// This has to happen after Runtime::LoadApplication() because that can reset kShowRuntimeErrorsSet
		if ([_coronaViewDelegate respondsToSelector:@selector(notifyRuntimeError:)])
		{
			// The delegate is setup to do something with errors, so send them
			_runtime->SetProperty(Rtt::Runtime::kShowRuntimeErrors, true);
			_runtime->SetProperty(Rtt::Runtime::kShowRuntimeErrorsSet, true);
		}
		
		[self willBeginRunLoop:_launchParams];

		_runtime->BeginRunLoop();

		id delegate = _viewDelegate;

		if ( [delegate respondsToSelector:@selector(didLoadApplication:)] )
		{
			[delegate didLoadApplication:self];
		}
        
        if (width == 0 || height == 0)
        {
            // If width and height aren't specified, default to window size
			width = CoronaViewPrivateDefaultWidth;
			height = CoronaViewPrivateDefaultHeight;
        }
        
        if (Rtt::DeviceOrientation::IsSideways( orientation ))
        {
            // Swap orientation for non-portrait layouts
            Rtt::Swap<int>(width, height);
        }
        
		_runtime->GetDisplay().WindowSizeChanged();
		//_runtime->GetDisplay().Restart(width, height);
	}
}

/*
- (void)initializeRuntimeWithPlatform:(Rtt::MacPlatform *)platform runtimeDelegate:(Rtt::CoronaViewRuntimeDelegate *)runtimeDelegate
{
    using namespace Rtt;

    if ( ! _runtime )
    {
        _platform = ( platform ? platform : new MacPlatform( ) );
        _runtime = new Runtime( * _platform );

		_runtimeDelegate = ( runtimeDelegate ? runtimeDelegate : new CoronaViewRuntimeDelegate( self ) );

        _runtime->SetDelegate( _runtimeDelegate );

		//bool isCoronaKit = Rtt::MacTemplate::IsProperty( MacTemplate::kIsCoronaKit );
		//_runtime->SetProperty( Rtt::Runtime::kIsCoronaKit, isCoronaKit );
    }
}
*/

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

- (id)init
{
    NSDEBUG(@"CoronaView: init");

    return [super init];
}

- (id)initWithPath:(NSString *)path frame:(NSRect)frame
{
    NSDEBUG(@"CoronaView: initWithPath: %@ frame: %@", path, NSStringFromRect(frame));

    self = [[CoronaView alloc] initWithFrame:frame];

    if ( self )
    {
		_projectPath = path;
    }

    return self;
}

- (id)initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if ( self )
	{
        NSDEBUG(@"CoronaView: initWithFrame: %@", NSStringFromRect([self frame]));

        [self initInternals];
	}
	return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];

    if ( self )
    {
        NSDEBUG(@"CoronaView: initWithCoder: %@", NSStringFromRect([self frame]));

        [self initInternals];
    }
    return self;
}

- (void) initInternals
{
    _GLView = [[GLView alloc] initWithFrame:[self frame]];
    [self addSubview:_GLView];
    [_GLView release];

    [_GLView setDelegate:self];

    [_GLView setOrientation:Rtt::DeviceOrientation::kUpright];  // default

	[_GLView setWantsBestResolutionOpenGLSurface:YES];

    _platform = NULL;
    _runtime = NULL;
	fSuspendCount = 0;
    _macHIDInputDeviceListener = NULL;
	_macMFiDeviceListener = nil;

    _GLViewCallback = new Rtt::MacViewCallback( _GLView ); // This is what is on the Timer loop

	_projectSettings = new Rtt::ProjectSettings();
}

/*
- (BOOL) isFlipped
{
	return YES;
}
*/

- (void)dealloc
{
    if (_macHIDInputDeviceListener != NULL)
    {
        _macHIDInputDeviceListener->StopListening();
		delete _macHIDInputDeviceListener;
    }
	
	if (_macMFiDeviceListener)
	{
		[_macMFiDeviceListener stop];
		[_macMFiDeviceListener release];
	}

	delete _GLViewCallback;
	delete _projectSettings;

	_runtime->SetDelegate(NULL);
	delete _runtime;

	Rtt_DELETE( _platform );
	[_GLView removeFromSuperview];

	[super dealloc];
}

// ----------------------------------------------------------------------------

//#if 0
- (void) setFrameSize:(NSSize)newSize
{
	NSDEBUG(@"CoronaViewPrivate: setFrameSize: frame %@", NSStringFromRect([self frame]));
	NSDEBUG(@"CoronaViewPrivate: setFrameSize: old %@, new %@", NSStringFromSize([self frame].size), NSStringFromSize(newSize));

	/*
	if (_GLView.isResizable && _runtime)
	{
		if (_runtime->IsDisplayValid() && _runtime->GetDisplay().HasWindowSizeChanged())
		{
			//_runtime->WindowSizeChanged();

			_runtime->RestartRenderer([_GLView orientation]);
			_runtime->GetDisplay().Invalidate();
		}
	}
	 */
	
	[_GLView setFrameSize:newSize];
	[super setFrameSize:newSize];
}

- (void) setFrameOrigin:(NSPoint)newOrigin
{
    NSDEBUG(@"CoronaViewPrivate: setFrameOrigin: %@", NSStringFromPoint(newOrigin));

    [super setFrameOrigin:newOrigin];
}

- (void) setFrame:(NSRect)newFrame
{
	NSDEBUG(@"CoronaViewPrivate: setFrame: from %@ to %@", NSStringFromRect([self frame]), NSStringFromRect(newFrame));

	[super setFrame:newFrame];
}

- (NSRect) frame
{
	NSRect frame = [super frame];
	// NSDEBUG(@"CoronaViewPrivate: frame: %@", NSStringFromRect(frame));

	return frame;
}
//#endif // 0

- (void) startLocationUpdating
{
	if (_locationManager == nil)
	{
		// Initialize CoreLocation
		_locationManager = [[CLLocationManager alloc] init];
		_locationManager.delegate = self;
		_locationManager.desiredAccuracy = kCLLocationAccuracyBest;
	}
	
	[self requestAuthorizationLocation];
	
	[_locationManager startUpdatingLocation];
}

- (void) endLocationUpdating
{
	if (_locationManager != nil)
	{
		[_locationManager stopUpdatingLocation];
		[_locationManager release];
		_locationManager = nil;
	}
}

// > The user prompt contains the text from the NSLocationWhenInUseUsageDescription key
// > in your app’s Info.plist file, and the presence of that key is required when calling
// > this method.
- (void) requestAuthorizationLocation
{
#if 0
	// Calling requestWhenInUseAuthorization only does something if the authorization
	// status is kCLAuthorizationStatusNotDetermined so if its something else we can
	// skip this block
	if ( [CLLocationManager authorizationStatus] != kCLAuthorizationStatusNotDetermined )
	{
		return;
	}
	
	CLLocationManager *locationManager = _locationManager;
	
	if ( [locationManager respondsToSelector:@selector(requestWhenInUseAuthorization)] )
	{
		[locationManager requestWhenInUseAuthorization];
	}
#endif // 0
}

-(void) sendLocationEvent
{
	if (_currentLocation == nil)
	{
		return;
	}
	
	NSDictionary *event = @{ @"name" : @"location",
							 @"latitude" : [NSNumber numberWithDouble:_currentLocation.coordinate.latitude],
							 @"longitude" : [NSNumber numberWithDouble:_currentLocation.coordinate.longitude],
							 @"altitude" : [NSNumber numberWithDouble:_currentLocation.altitude],
							 @"accuracy" : [NSNumber numberWithDouble:_currentLocation.horizontalAccuracy],
							 @"speed" : [NSNumber numberWithDouble:_currentLocation.speed],
							 @"direction" : [NSNumber numberWithDouble:_currentLocation.course],
							 @"time" : [NSNumber numberWithDouble:[_currentLocation.timestamp timeIntervalSince1970]] };
	
	[self sendEvent:event];
}

- (void)locationManager:(CLLocationManager *)manager
	didUpdateToLocation:(CLLocation *)newLocation
		   fromLocation:(CLLocation *)oldLocation
{
	[_currentLocation release];
	_currentLocation = [newLocation retain];
	
	[self sendLocationEvent];
}

// Interface for hosts to send "open URL" AppleScript events
- (void) handleOpenURL:(NSString *)urlStr
{
	if ( _runtime != NULL )
	{
		Rtt::SystemOpenEvent e( [urlStr UTF8String] );

		_runtime->DispatchEvent( e );
	}
}

#pragma mark GLView Helpers

- (void) setScaleFactor:(CGFloat)scaleFactor
{
	_GLView.scaleFactor = scaleFactor;
}

- (void) restoreWindowProperties
{
	[_GLView restoreWindowProperties];
}

#pragma mark ProjectSettings Helpers

- (BOOL) settingsIsWindowResizable
{
	return (BOOL) _projectSettings->IsWindowResizable();
}

- (BOOL) settingsIsWindowCloseButtonEnabled
{
	return (BOOL) _projectSettings->IsWindowCloseButtonEnabled();
}

- (BOOL) settingsIsWindowMinimizeButtonEnabled
{
	return (BOOL) _projectSettings->IsWindowMinimizeButtonEnabled();
}

- (BOOL) settingsIsWindowMaximizeButtonEnabled
{
	return (BOOL) _projectSettings->IsWindowMaximizeButtonEnabled();
}

- (BOOL) settingsSuspendWhenMinimized
{
    return (BOOL) _projectSettings->SuspendWhenMinimized();
}

- (int) settingsContentWidth
{
	if (! Rtt::DeviceOrientation::IsSideways(_projectSettings->GetDefaultOrientation()))
	{
		return (int) _projectSettings->GetContentWidth();
	}
	else
	{
		return (int) _projectSettings->GetContentHeight();
	}
}

- (int) settingsContentHeight
{
	if (! Rtt::DeviceOrientation::IsSideways(_projectSettings->GetDefaultOrientation()))
	{
		return (int) _projectSettings->GetContentHeight();
	}
	else
	{
		return (int) _projectSettings->GetContentWidth();
	}
}

- (NSString *) settingsWindowTitle
{
	NSString * language = [[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode];
	NSString *countryCode = [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode];
	const char *title = _projectSettings->GetWindowTitleTextForLocale([language UTF8String], [countryCode UTF8String]);

	if (title != NULL)
	{
		return [NSString stringWithExternalString:title];
	}
	else
	{
		return nil;
	}
}

- (const CoronaViewWindowMode) settingsDefaultWindowMode
{
    CoronaViewWindowMode coronaViewWindowMode = kNormal;
    const Rtt::NativeWindowMode *nativeWindowMode = _projectSettings->GetDefaultWindowMode();
    
    if (*nativeWindowMode == Rtt::NativeWindowMode::kNormal)
    {
        coronaViewWindowMode = kNormal;
    }
    else if (*nativeWindowMode == Rtt::NativeWindowMode::kMinimized)
    {
        coronaViewWindowMode = kMinimized;
    }
    else if (*nativeWindowMode == Rtt::NativeWindowMode::kMaximized)
    {
        coronaViewWindowMode = kMaximized;
    }
    else if (*nativeWindowMode == Rtt::NativeWindowMode::kFullscreen)
    {
        coronaViewWindowMode = kFullscreen;
    }

	return coronaViewWindowMode;
}

- (int) settingsMinWindowViewWidth
{
	return _projectSettings->GetMinWindowViewWidth();
}

- (int) settingsMinWindowViewHeight
{
	return _projectSettings->GetMinWindowViewHeight();
}

- (int) settingsDefaultWindowViewWidth
{
	return _projectSettings->GetDefaultWindowViewWidth();
}

- (int) settingsDefaultWindowViewHeight
{
	return _projectSettings->GetDefaultWindowViewHeight();
}

- (int) settingsImageSuffixScaleCount
{
	return _projectSettings->GetImageSuffixScaleCount();
}

- (double) settingsImageSuffixScaleByIndex:(int) index
{
	return _projectSettings->GetImageSuffixScaleByIndex(index);
}

- (BOOL) settingsIsWindowTitleShown
{
	// We use APIs that are available in 10.10 and above to hide the titlebar
	// so pretend it can't be hidden on lower OS X versions 
	if (NSAppKitVersionNumber < NSAppKitVersionNumber10_10)
	{
		return YES;
	}
	else
	{
		return (BOOL) _projectSettings->IsWindowTitleShown();
	}
}

- (BOOL) settingsIsTransparent
{
	return (BOOL) _projectSettings->IsWindowTransparent();
}

// ----------------------------------------------------------------------------

@end
