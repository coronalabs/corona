//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacSimulator.h"

#include "Rtt_Lua.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_MacSurfaceVideoRecorder.h"
#include "Rtt_MacViewCallback.h"
#include "Rtt_PlatformPlayer.h"


#import "AppDelegate.h"
#import "GLView.h"
#import "SkinnableWindow.h"
#import "SkinlessSimulatorWindow.h"

#import "CoreAnimationUtilities.h" // for shake animation
#include "Display/Rtt_Display.h"
#include "Display/Rtt_Scene.h"
#include "Rtt_Runtime.h"

#import <AppKit/NSApplication.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSScreen.h>

#include "Rtt_AppleKeyServices.h"
#import <Carbon/Carbon.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
	//#import <Foundation/NSNumber.h>
	#import <Foundation/NSDictionary.h>
	#import <Foundation/NSKeyValueCoding.h>
	#import <Foundation/NSString.h>
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacSimulator::MacSimulator( bool headless )
:	Super( Super::Finalizer< MacGUIPlatform > ),
//	fPool( [[NSAutoreleasePool alloc] init] ),
	fWindow( nil ),
	fWindowController( nil ),
	fProperties( [[NSMutableDictionary alloc] init] ),
	fScreenWidth( 0. ),
	fScreenHeight( 0. ),
    fSupportsScreenRotation( true ),
	fHeadless( headless ),
	fDeviceSkinIdentifier( nil ),
	fViewCallback( NULL ),
	fMacMFIListener(nil),
	fMacHidDeviceListener(NULL)
{
}

MacSimulator::~MacSimulator()
{
	if (fMacMFIListener)
	{
		[fMacMFIListener stop];
		[fMacMFIListener release];
	}
	if (fMacHidDeviceListener)
	{
		fMacHidDeviceListener->StopListening();
		delete fMacHidDeviceListener;
	}

	// The NSView and CALayer have a pointer to the runtime which it uses to draw/
	// Since the drawing may be asynchonous and the view release may cause dealloc to happen
	// at the end of the runloop, we need to clear the runtime variable now.
	GLView* view = [(SkinnableWindow *)fWindow screenView];
//	Rtt_ASSERT(YES == [view isKindOfClass:GLView]);
    if (view != nil && [view runtime] != NULL)
    {
        [view runtime]->GetDisplay().Invalidate();
        [view setRuntime:NULL];
    }


    [fWindow saveFrameUsingName:fDeviceName];
	[fWindow close];
	// Fix case 40335: Simulator crashes sometime after an app with a native textfield runs (http://bugs.coronalabs.com/default.asp?40335)
	[fWindow makeFirstResponder:nil];

	[fWindowController close];
	delete fViewCallback;

	[fDeviceSkinIdentifier release];
	[fDeviceName release];
	[fProperties release];
	[fWindow release];
	// Fix case 20368: Setting focus on native keyboard crashes simulator (http://bugs.coronalabs.com/default.asp?20368)
	fWindow = nil;
	[fWindowController release];

//	[fPool release];
}

static bool
AddValueForKey( NSMutableDictionary *d, const char *value, NSString *key )
{
	bool didAdd = ( value && ( '\0' != *value ) );

	if ( didAdd )
	{
		NSString* s = [NSString stringWithExternalString:value];
		[d setValue:s forKey:key];
	}

	return didAdd;
}

void
MacSimulator::Initialize(
	const char deviceConfigFile[],
	const char resourcePath[] )
{
	using namespace Rtt;

	MacGUIPlatform* platform = new MacGUIPlatform( * this );
    NSString *skinDir = [[NSString stringWithExternalString:deviceConfigFile] stringByDeletingLastPathComponent];
	platform->SetResourcePath( resourcePath );
	Super::Config config( platform->GetAllocator() );
	Super::LoadConfig( deviceConfigFile, *platform, config );

	platform->SetAdaptiveWidth( config.GetAdaptiveWidth() );
	platform->SetAdaptiveHeight( config.GetAdaptiveHeight() );
    
    if (! config.configLoaded)
    {
        return;
    }
    
	LoadBuildSettings( *platform );

	fScreenWidth = config.screenWidth;
	fScreenHeight = config.screenHeight;
    fSupportsScreenRotation = config.supportsScreenRotation;

	platform->GetMacDevice().SetManufacturer( [NSString stringWithExternalString:config.displayManufacturer.GetString()] );
	platform->GetMacDevice().SetModel( [NSString stringWithExternalString:config.displayName.GetString()] );

	// -------------

	// Only add if default is available
	if ( AddValueForKey( fProperties, config.statusBarDefaultFile.GetString(), @"statusBarDefaultFile" ) )
	{
		Rtt_VERIFY( AddValueForKey( fProperties, config.statusBarTranslucentFile.GetString(), @"statusBarTranslucentFile" ) );
		Rtt_VERIFY( AddValueForKey( fProperties, config.statusBarBlackFile.GetString(), @"statusBarBlackFile" ) );
		Rtt_VERIFY( AddValueForKey( fProperties, config.statusBarLightTransparentFile.GetString(), @"statusBarLightTransparentFile" ) );
		Rtt_VERIFY( AddValueForKey( fProperties, config.statusBarDarkTransparentFile.GetString(), @"statusBarDarkTransparentFile" ) );
		[fProperties setValue:[NSNumber numberWithInt:MPlatform::kTranslucentStatusBar] forKey:@"statusBarCurrent"];
	}

	if (config.screenDressingFile.GetString() != NULL)
	{
		AddValueForKey(fProperties, config.screenDressingFile.GetString(), @"screenDressingFile");
	}

	AppDelegate *delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	[fProperties setValue:@"Solar2D" forKey:@"subscription"];
	
	// Store the simulated device's default font size.
	[fProperties setValue:[NSNumber numberWithFloat:config.defaultFontSize] forKey:@"defaultFontSize"];
	
	// Store whether or not the simulated device supports the following features.
	[fProperties setValue:[NSNumber numberWithBool:config.supportsExitRequests] forKey:@"supportsExitRequests"];
	[fProperties setValue:[NSNumber numberWithBool:config.supportsBackKey] forKey:@"supportsBackKey"];
	[fProperties setValue:[NSNumber numberWithBool:config.supportsKeyEvents] forKey:@"supportsKeyEvents"];
	[fProperties setValue:[NSNumber numberWithBool:config.supportsMouse] forKey:@"supportsMouse"];
	[fProperties setValue:[NSString stringWithExternalString:config.osName] forKey:@"osName"];

	// Store the simulated device's safe screen area
	[fProperties setValue:[NSNumber numberWithFloat:config.safeScreenInsetStatusBar] forKey:@"safeScreenInsetStatusBar"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeScreenInsetTop] forKey:@"safeScreenInsetTop"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeScreenInsetLeft] forKey:@"safeScreenInsetLeft"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeScreenInsetBottom] forKey:@"safeScreenInsetBottom"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeScreenInsetRight] forKey:@"safeScreenInsetRight"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeLandscapeScreenInsetStatusBar] forKey:@"safeLandscapeScreenInsetStatusBar"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeLandscapeScreenInsetTop] forKey:@"safeLandscapeScreenInsetTop"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeLandscapeScreenInsetLeft] forKey:@"safeLandscapeScreenInsetLeft"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeLandscapeScreenInsetBottom] forKey:@"safeLandscapeScreenInsetBottom"];
	[fProperties setValue:[NSNumber numberWithFloat:config.safeLandscapeScreenInsetRight] forKey:@"safeLandscapeScreenInsetRight"];

	// -------------

	NSPoint screenOrigin =
	{
		config.screenOriginX,
		config.screenOriginY
	};
	
	NSSize screenSize =
	{
		config.screenWidth,
		config.screenHeight
	};
	const char* device_image_file_cstr = config.deviceImageFile.GetString();
	const char* display_name_cstr = config.displayName.GetString();
	const char* window_titlebar_name_cstr = config.windowTitleBarName.GetString();
	
	NSString* deviceImageFile = nil;
	NSString* displayName = nil;
	NSString* windowTitleBarName = nil;
    
	if ( NULL != device_image_file_cstr )
	{
		deviceImageFile = [skinDir stringByAppendingPathComponent:[NSString stringWithExternalString:device_image_file_cstr]];
	}
	if ( NULL != display_name_cstr )
	{
		displayName = [NSString stringWithExternalString:display_name_cstr];
	}
	if ( NULL != window_titlebar_name_cstr )
	{
		//windowTitleBarName = [NSString stringWithExternalString:window_titlebar_name_cstr];
		windowTitleBarName = [NSString stringWithFormat:@"%s - %.0fx%.0f", window_titlebar_name_cstr, config.screenWidth, config.screenHeight];
	}
	
	// This used to be a user preference for whether the user wanted skinned windows or not, we now offer borderless windows
    // as a separate option in the Window/View As menu so we just default this to false (this allows us to continue to use
    // some other behavior associated with the setting)
	BOOL user_disabled_skins = NO;

    if (! [[NSFileManager defaultManager] fileExistsAtPath:deviceImageFile] )
    {
        // If the skin image file isn't available, act as if skins are disabled (important
        // because we allow user defined skins)
        user_disabled_skins = YES;
    }

	// If the user doesn't want a skin, we need to reset the origin offset or the view won't be centered
	// correctly in the window unless the description file already happens to have an offset of 0
	if(YES == user_disabled_skins)
	{
		screenOrigin.x = 0;
		screenOrigin.y = 0;
	}
	NSRect screenRect = { screenOrigin, screenSize };

	GLView* screenView = [[GLView alloc] initWithFrame:screenRect];
	[screenView autorelease];
	[screenView setOrientation:GetOrientation()];
	[screenView setDelegate:delegate];

	SimulatorDeviceWindow* instanceWindow = nil;
	void (^window_close_handler)(id) = ^(id sender)
	{
		// pass the action to the app delegate which will handle the close for us
		[NSApp sendAction:@selector(close:) to:[[NSApplication sharedApplication] delegate] from:sender];
	};	

	// Need to do this BEFORE window is set up b/c window triggers other prepareOpenGL
	platform->Initialize( screenView );
    platform->SetSkinResourceDirectory([skinDir UTF8String]);

	fViewCallback = new MacViewCallback( screenView ); // This is what is on the Timer loop
	Super::Initialize( platform, fViewCallback ); // Inside here, Runtime is instantiated

	// TODO: Set kDeferUpdate and kRenderSeparately properties here

	//Mac simulator needs to defer the initial update and render in separate pass
	GetPlayer()->GetRuntime().SetProperty(Runtime::kDeferUpdate, true);
	GetPlayer()->GetRuntime().SetProperty(Runtime::kRenderAsync, true);
	


	// Chicken and egg issue between fViewCallback and this->fRuntime
	// Runtime is not valid until Super::Initialize() but we need to pass fViewCallback to Super::Initialize.
	fViewCallback->Initialize( & GetPlayer()->GetRuntime() );

    // restore the user's last setting for this skin
	// We need a placeholder name for autosave
	if ( nil == deviceImageFile )
	{
		fDeviceName = [displayName copy];
	}
    else
    {
        fDeviceName = [[[deviceImageFile lastPathComponent] stringByDeletingPathExtension] copy];
    }
	
	// Use this string as our current selected skin identifier which will be used as a key to save the user's set scale factor to
	fDeviceSkinIdentifier = [windowTitleBarName copy];
	
    BOOL restoredScale = NO;
    NSNumber* scalefactor = [NSNumber numberWithFloat:0]; // Counterintuitively, 0 is full size
    NSString* skinname = fDeviceSkinIdentifier;
    if ( nil != skinname )
    {
        NSDictionary* savepref = [[NSUserDefaults standardUserDefaults] dictionaryForKey:kUserPreferenceScaleFactorForSkin];
        
        if(nil != savepref)
        {
            scalefactor = [savepref objectForKey:skinname];
            if(nil != scalefactor)
            {
                float scalefloat = [scalefactor floatValue];
                if( scalefloat > 0.0f )
                {
                    restoredScale = YES;
                }
            }
        }
    }
    
	// ---
    
	if ( ((nil == deviceImageFile) || (0 == [deviceImageFile length])) || user_disabled_skins)
	{
        instanceWindow = [[SkinlessSimulatorWindow alloc] initWithScreenView:screenView
                                                                    viewRect:screenRect
                                                                       title:windowTitleBarName
                                                                 orientation:GetOrientation()
                                                                       scale:[scalefactor floatValue]];
        
		[(SkinlessSimulatorWindow*)instanceWindow setPerformCloseBlock:window_close_handler];
	}
	else
	{
        instanceWindow = [[SkinnableWindow alloc] initWithScreenView:screenView
                                                            viewRect:screenRect
                                                               title:windowTitleBarName
                                                           skinImage:deviceImageFile
                                                         orientation:GetOrientation()
                                                               scale:[scalefactor floatValue]];
        
		[(SkinnableWindow*)instanceWindow setPerformCloseBlock:window_close_handler];
	}

	NSWindowController *windowController = [[NSWindowController alloc] initWithWindow:instanceWindow];
	
	[instanceWindow setDelegate:(id <NSWindowDelegate>)windowController];

    fWindow = instanceWindow;
	fWindowController = windowController;

	Runtime& runtime = GetPlayer()->GetRuntime();
	screenView.runtime = & runtime;
	runtime.SetProperty( Runtime::kIsOrientationLocked, Super::IsProperty( Super::kIsOrientationLocked ) );
	
	//Initialize Joystick support
	AppleInputDeviceManager& macDeviceManager = (AppleInputDeviceManager&)runtime.Platform().GetDevice().GetInputDeviceManager();
	
	if (fMacMFIListener)
	{
		[fMacMFIListener stop];
		[fMacMFIListener release];
	}
	fMacMFIListener = [[AppleInputMFiDeviceListener alloc] initWithRuntime:&runtime andDeviceManager:&macDeviceManager];
	[fMacMFIListener start];
	
	if (fMacHidDeviceListener)
	{
		fMacHidDeviceListener->StopListening();
		delete fMacHidDeviceListener;
	}
	fMacHidDeviceListener = new AppleInputHIDDeviceListener();
	fMacHidDeviceListener->StartListening(&runtime, &macDeviceManager);
	
	// -------------

    if (! restoredScale)
    {
        // Nothing remembered for this skin, scale it to fit screen if necessary and center the first time
        NSScreen *windowsScreen = [instanceWindow screen];
        
        if (windowsScreen == nil)
        {
            windowsScreen = [NSScreen mainScreen];
        }
        
        int screenHeight = [windowsScreen frame].size.height;
        int windowHeight = [instanceWindow frame].size.height;
        
        if (windowHeight > screenHeight)
        {
            if ((windowHeight * 0.5) > screenHeight)
            {
                // Assumes no screen is so small 2 zoom outs wont fit it
                [(SkinnableWindow *)instanceWindow setScale:0.25];
            }
            else
            {
                [(SkinnableWindow *)instanceWindow setScale:0.5];
            }
        }
        else
        {
            // This makes the framework record that we've opened this skin before
            [(SkinnableWindow *)instanceWindow setScale:1.0];
        }
        
        [instanceWindow center];
    }

	if ( ! fHeadless )
	{
        [instanceWindow setFrameUsingName:fDeviceName];
        [instanceWindow makeKeyAndOrderFront:nil];
	}

	Rtt_TRACE_SIM( ( "Loading project from:   %s\n", [[[NSString stringWithExternalString:resourcePath] stringByAbbreviatingWithTildeInPath] UTF8String] ) );
	Rtt_TRACE_SIM( ( "Project sandbox folder: %s\n", [[platform->GetSandboxPath() stringByAbbreviatingWithTildeInPath] UTF8String] ) );
}

const char *
MacSimulator::GetPlatformName() const
{
	static const char kName[] = "mac-sim";
	return kName;
}

const char *
MacSimulator::GetPlatform() const
{
	static const char kName[] = "macos";
	return kName;
}

void
MacSimulator::DidRotate( bool clockwise, DeviceOrientation::Type start, DeviceOrientation::Type end )
{
//	S32 absoluteAngle = -DeviceOrientation::CalculateRotation( DeviceOrientation::kUpright, end );

	[(SkinnableWindow *)fWindow rotate:clockwise];
}



void
MacSimulator::DidChangeScale(float scalefactor)
{
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
	
	// Watch out for ordering issues. Is it possible the current selected skin is saved to defaults after the scale change?
	NSString* skinname = fDeviceSkinIdentifier;
	if ( nil == skinname )
	{
		// Not expecting nil, but due to special behaviors for things like the Demo skin, this is a safe guard.
		return;
	}
	
	NSMutableDictionary* savepref = [[[defaults dictionaryForKey:kUserPreferenceScaleFactorForSkin] mutableCopy] autorelease];
	if(nil == savepref)
	{
		savepref = [NSMutableDictionary dictionary];
	}
	
	[savepref setObject:[NSNumber numberWithFloat:scalefactor] forKey:skinname];
	[defaults setObject:savepref forKey:kUserPreferenceScaleFactorForSkin];
}

void
MacSimulator::SetStatusBarMode( MPlatform::StatusBarMode newValue )
{
	[fProperties setValue:[NSNumber numberWithInt:newValue] forKey:@"statusBarCurrent"];
}

MPlatform::StatusBarMode
MacSimulator::GetStatusBarMode() const
{
	NSNumber* number = (NSNumber*)[fProperties valueForKey:@"statusBarCurrent"];
	return (MPlatform::StatusBarMode)[number intValue];
}

GLView*
MacSimulator::GetScreenView() const
{
	return [(SkinnableWindow *)fWindow screenView];
}

void
MacSimulator::Shake()
{
	PlatformSimulator::Shake();
	CoreAnimationUtilities_ShakeViewWithAnimation( fWindow, [fWindow frame], CoreAnimationUtilities_PasswordShakeAnimationWithRect( NSRectToCGRect( [fWindow frame] ) ) );
	
}

bool
MacSimulator::SupportsBackKey()
{
	return [(NSNumber*)[fProperties valueForKey:@"supportsBackKey"] boolValue];
}

const char *
MacSimulator::GetOSName() const
{
	// return the name of the OS this simulator is simulating
	return [[fProperties valueForKey:@"osName"] UTF8String];
}

bool
MacSimulator::Back()
{
	Runtime& runtime = GetPlayer()->GetRuntime();
	BOOL skinSupportsBackKey = SupportsBackKey();

	if ( skinSupportsBackKey && ! runtime.IsSuspended() )
	{
		// Simulate the pressing of a virtual "back" button
		short keyCode = kVK_Back;
		NSString *keyName = [AppleKeyServices getNameForKey:[NSNumber numberWithInt:keyCode]];

		KeyEvent eDown(NULL,
					   KeyEvent::kDown,
					   [keyName UTF8String],
					   keyCode,
					   false,  // (modifierFlags & NSShiftKeyMask) || (modifierFlags & NSAlphaShiftKeyMask),
					   false,  // (modifierFlags & NSAlternateKeyMask),
					   false,  // (modifierFlags & NSControlKeyMask),
					   false ); // (modifierFlags & NSCommandKeyMask) );

		runtime.DispatchEvent( eDown );

		KeyEvent eUp(NULL,
					 KeyEvent::kUp,
					 [keyName UTF8String],
					 keyCode,
					 false,  // (modifierFlags & NSShiftKeyMask) || (modifierFlags & NSAlphaShiftKeyMask),
					 false,  // (modifierFlags & NSAlternateKeyMask),
					 false,  // (modifierFlags & NSControlKeyMask),
					 false ); // (modifierFlags & NSCommandKeyMask) );

		runtime.DispatchEvent( eUp );

		// Let our caller decide what to do (e.g. if the handler for the "back" button returns false, exit the simulation)
		return eUp.GetResult();
	}

	return true;
}

void
MacSimulator::WillSuspend()
{
    GLView* layerhostview = GetScreenView();

    [layerhostview suspendNativeDisplayObjects:YES];
}

void
MacSimulator::DidResume()
{
	GLView* layerhostview = GetScreenView();

	[layerhostview resumeNativeDisplayObjects];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

