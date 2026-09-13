//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "CoronaWindowController.h"

#import "CoronaViewPrivate.h"

#import "CoronaLua.h"

// Player
#include "Rtt_MacPlatform.h"
#include "Rtt_MacSimulatorServices.h"

// Modules
#include "Rtt_LuaLibSimulator.h"

// Librtt
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "Rtt_RuntimeDelegate.h"
#include "Rtt_MacSimulatorServices.h"
#include "HomeScreenRuntimeDelegate.h"


// ----------------------------------------------------------------------------

#define ENABLE_WINDOW_FADE_ANIMATIONS 1
#define WELCOME_FADE_OUT_DURATION 0.20
#define WELCOME_FADE_IN_DURATION 0.20

@class CoronaWindowController;

namespace Rtt
{

// ----------------------------------------------------------------------------

static int
close( lua_State *L )
{
	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	const CoronaWindowController *controller =
		(const CoronaWindowController *)lua_touserdata( L, lua_upvalueindex( 1 ) );

	[controller.window close];

	return 0;
}

static int
stopModal( lua_State *L )
{
	[NSApp stopModal];

	return 0;
}

// ----------------------------------------------------------------------------

class LuaLibScreen
{
	public:
		typedef LuaLibScreen Self;

	public:
		static const char kName[];
		static int Open( lua_State *L );
};

const char LuaLibScreen::kName[] = "screen";

// ----------------------------------------------------------------------------

int
LuaLibScreen::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "close", close },
		{ "stopModal", stopModal },

		{ NULL, NULL }
	};

	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	void *context = lua_touserdata( L, lua_upvalueindex( 1 ) );
	lua_pushlightuserdata( L, context );
	luaL_openlib( L, kName, kVTable, 1 ); // leave "simulator" on top of stack

	return 1;
}

// ----------------------------------------------------------------------------

class RuntimeDelegateWrapper : public RuntimeDelegate
{
	public:
		RuntimeDelegateWrapper( CoronaWindowController *owner );
		~RuntimeDelegateWrapper();

	public:
		virtual void DidInitLuaLibraries( const Runtime& sender ) const;
		virtual void WillLoadMain( const Runtime& sender ) const;
		virtual void DidLoadMain( const Runtime& sender ) const;
		virtual void WillLoadConfig( const Runtime& sender, lua_State *L ) const;
		virtual void InitializeConfig( const Runtime& sender, lua_State *L ) const;
		virtual void DidLoadConfig( const Runtime& sender, lua_State *L ) const;

	public:
		void SetDelegate( RuntimeDelegate *delegate );
		RuntimeDelegate * GetDelegate() { return fDelegate; }
	
	private:
		CoronaWindowController *fOwner;
		RuntimeDelegate *fDelegate;
};

// ----------------------------------------------------------------------------

RuntimeDelegateWrapper::RuntimeDelegateWrapper( CoronaWindowController *owner )
:	fOwner( owner ),
	fDelegate( NULL )
{
}

RuntimeDelegateWrapper::~RuntimeDelegateWrapper()
{
	delete fDelegate;
}

void
RuntimeDelegateWrapper::DidInitLuaLibraries( const Runtime& sender ) const
{
	if ( fDelegate )
	{
		fDelegate->DidInitLuaLibraries( sender );
	}
}

void
RuntimeDelegateWrapper::WillLoadMain( const Runtime& sender ) const
{
	// TODO: Shouldn't this be in DidInitLuaLibraries?
	lua_State *L = sender.VMContext().L();
	lua_pushlightuserdata( L, fOwner );
	LuaContext::RegisterModuleLoader( L, LuaLibScreen::kName, LuaLibScreen::Open, 1 );

	if ( fDelegate )
	{
		fDelegate->WillLoadMain( sender );
	}
}

void
RuntimeDelegateWrapper::DidLoadMain( const Runtime& sender ) const
{
	if ( fDelegate )
	{
		fDelegate->DidLoadMain( sender );
	}
}

void
RuntimeDelegateWrapper::WillLoadConfig( const Runtime& sender, lua_State *L ) const
{
	if ( fDelegate )
	{
		fDelegate->WillLoadConfig( sender, L );
	}
}

void
RuntimeDelegateWrapper::InitializeConfig( const Runtime& sender, lua_State *L ) const
{
	if ( fDelegate )
	{
		fDelegate->InitializeConfig( sender, L );
	}
}

void
RuntimeDelegateWrapper::DidLoadConfig( const Runtime& sender, lua_State *L ) const
{
	if ( fDelegate )
	{
		fDelegate->DidLoadConfig( sender, L );
	}
}

void
RuntimeDelegateWrapper::SetDelegate( RuntimeDelegate *delegate )
{
	if ( delegate != fDelegate )
	{	
		delete fDelegate;
		fDelegate = delegate;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

@interface CoronaWindowController ()

@property (nonatomic, retain) NSString *fWindowTitle;

- (void) startWindowFadeInAnimation:(id)user_data;
- (void) startWindowFadeOutAnimation:(id)user_data;
@end

@implementation CoronaWindowController

@synthesize fView;
@synthesize windowGoingAway;
@synthesize fWindowTitle;

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)menuitem
{
    BOOL enable;
	
    if ([menuitem action] == @selector(close:))
    {
		enable = NO;
    }
	else
	{
		enable = [self respondsToSelector:[menuitem action]]; //handle general case.
	}
	
    return enable;
}

- (id)initWithPath:(NSString*)path
{
    // These are the default size of the Welcome window
    return [self initWithPath:path width:960 height:540 title:nil resizable:false showWindowTitle:true];
}

- (id)initWithPath:(NSString*)path width:(int)width height:(int)height title:(NSString *)windowTitle resizable:(bool) resizable showWindowTitle:(bool) showWindowTitle
{
	using namespace Rtt;

	// We use APIs that are available in 10.10 and above to hide the titlebar
	if (NSAppKitVersionNumber < NSAppKitVersionNumber10_10)
	{
		showWindowTitle = YES;
	}

	BOOL isDir = NO;
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    
    if (resizable)
    {
        styleMask |= NSResizableWindowMask;
    }

	if (! showWindowTitle)
	{
		styleMask |= NSFullSizeContentViewWindowMask;
	}

	if ( [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir]
		 && isDir )
	{			
		NSRect frame = NSMakeRect( 0, 0, width, height );
		NSWindow* fWindow = [[NSWindow alloc] initWithContentRect:frame
														styleMask:styleMask
														  backing:NSBackingStoreBuffered
															defer:NO];

		// Centering the window now before we attach the window controller.
		// This seems to center the window for first time launches (new users with no saved preferences).
		// But this gets overridden by the window controller's last saved position if it exists which is what we want
		// because we want to respect the user's last position (especially in multiple display situations).
		[fWindow center];

        if (resizable)
        {
            [fWindow setShowsResizeIndicator:YES];
            [fWindow setMinSize:[fWindow frame].size];  // min window size is original size
            [fWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary]; // turn on full-screen button
        }

		if (! showWindowTitle)
		{
			[fWindow setTitlebarAppearsTransparent:YES];
			[fWindow setTitleVisibility:NSWindowTitleHidden];
		}
        
		// Now create the WindowController with the window
		self = [super initWithWindow:fWindow];

		// WindowController should now own the window
		[fWindow release];

		if ( self )
		{
			[fWindow setReleasedWhenClosed:NO];
			[fWindow setDelegate:(id<NSWindowDelegate>)self];

            fView = [[CoronaView alloc] initWithPath:path frame:frame];
            
            [fView setViewDelegate:(id<CoronaViewDelegate>)self];  // DPC: ???
            [self.view.glView setIsResizable:resizable];

			fRuntimeDelegateWrapper = new RuntimeDelegateWrapper( self );

			NSView *contentView = [fWindow contentView];
			[contentView addSubview:fView];

			fIsInitialized = NO;
			fWindowTitle = windowTitle;
		}

		return self;
	}

	// Failed
	return nil;
}

- (void)didPrepare
{
    NSString* window_title = nil;

    [self.view run];
    
    if (fWindowTitle == nil || [fWindowTitle length] == 0)
    {
        [self setWindowFrameAutosaveName:@"CoronaWelcomeWindow"];

        window_title = [NSString stringWithFormat:@"%@ (Build %@)",
                        NSLocalizedString(@"Welcome to Solar2D", @"Welcome Window Title Bar Text Base"),
                        [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"]
                        ];
    }
    else
    {
        [self setWindowFrameAutosaveName:[NSString stringWithFormat:@"%@-Window", fWindowTitle]];

        window_title = fWindowTitle;
    }

    [[self window] setTitle:window_title];

    // This ensures any Lua window resize callback gets called with
    // the current window size as recovered from the user's preferences
    [self.view.glView setFrameSize:[self.view frame].size];
    [self windowWillResize:self.window toSize:[self.window frame].size];

#if ENABLE_WINDOW_FADE_ANIMATIONS
    [[self window] setAlphaValue:0.0];
    [self performSelector:@selector(startWindowFadeInAnimation:) withObject:nil afterDelay:0.0];
#endif
}

- (void)dealloc
{
	delete fRuntimeDelegateWrapper;
	[fView release];
	[windowShouldCloseBlock release];
    windowShouldCloseBlock = nil;
	[windowCloseCompletionBlock release];
	windowCloseCompletionBlock = nil;
    [fColorPanelCallbackBlock release];
    fColorPanelCallbackBlock = nil;

	[super dealloc];
}

- (void)setRuntimeDelegate:(Rtt::RuntimeDelegate *)delegate
{
	fRuntimeDelegateWrapper->SetDelegate( delegate );
}

- (void)willLoadApplication:(CoronaView*)sender
{
	using namespace Rtt;

	Runtime *runtime = self.view.runtime;
	runtime->SetDelegate( fRuntimeDelegateWrapper );
}

- (void)didLoadApplication:(CoronaView*)sender
{
	fIsInitialized = YES;
	[self show];
}

- (void)show
{
	if ( fIsInitialized )
	{
		[[self window] makeKeyAndOrderFront:nil];
	}
}

- (void)hide
{
	if ( fIsInitialized )
	{
		[[self window] close];
	}
}

- (void) newProject
{
	if ( NULL != fRuntimeDelegateWrapper )
	{
		 ((Rtt::HomeScreenRuntimeDelegate*)fRuntimeDelegateWrapper->GetDelegate())->NewProject();
	}
}
- (void) projectLoaded
{
	if ( NULL != fRuntimeDelegateWrapper )
	{
		Rtt::Runtime *runtime = self.view.runtime;
        
        if ( fIsInitialized && NULL != runtime && NULL != fRuntimeDelegateWrapper->GetDelegate() )
        {
            ((Rtt::HomeScreenRuntimeDelegate*)fRuntimeDelegateWrapper->GetDelegate())->ProjectLoaded(*runtime);
        }
	}
}

- (void) startWindowFadeInAnimation:(id)user_data
{
	CABasicAnimation* fade_animation = [CABasicAnimation animation];
	[fade_animation setValue:@"windowFadeIn" forKey:@"name"];
	if([user_data isKindOfClass:[NSNumber class]])
	{
		// Assuming elapsed time was from fade out that got interrupted part way through.
		// We want to start at the alpha value that we think is currently on screen
		NSNumber* elapsed_number = (NSNumber*)user_data;
		CFTimeInterval elapsed_time = [elapsed_number doubleValue];
		CFTimeInterval delta_time = elapsed_time / WELCOME_FADE_OUT_DURATION;
		// Linear interpolation formula
		// X = x0 + (x1-x0)t 
		CFTimeInterval current_alpha = 1.0 + (0.0 - 1.0) * delta_time;
		if(current_alpha < 0.0)
		{
			current_alpha = 0;
		}
		else if(current_alpha > 1.0)
		{
			current_alpha = 1.0;
		}
		//		NSLog(@"current alpha=%f", current_alpha);
		fade_animation.fromValue = [NSNumber numberWithDouble:current_alpha];	
	}
	else
	{
		fade_animation.fromValue = [NSNumber numberWithFloat:0.0f];
	}
	
	fade_animation.toValue = [NSNumber numberWithFloat:1.0f];		
	fade_animation.delegate = self;
	fade_animation.duration = WELCOME_FADE_IN_DURATION;
	fade_animation.removedOnCompletion = YES;
	[[self window] setAnimations:[NSDictionary dictionaryWithObject:fade_animation forKey:@"alphaValue"]];
	[[[self window] animator] setAlphaValue:1.0];
	windowFadeAnimationStartTime = CACurrentMediaTime();
	
}

- (void)animationDidStop:(CAAnimation*)the_animation finished:(BOOL)finished_naturally
{
	NSString* animation_name = [the_animation valueForKey:@"name"];
	if([animation_name isEqualToString:@"windowFadeOut"])
	{
		// Animation might be interrupted by resurrection so check if finished naturally
		if(YES == finished_naturally)
		{
			// There seems to be a retain cycle here. 
			// Clear the animations list to break the cycle so the WindowController can be released.
			[[self window] setAnimations:[NSDictionary dictionary]];
			// expect that windowWillClose: will get called, but windowShouldClose will be bypassed
			[self close];
		}
	}
	else if([animation_name isEqualToString:@"windowFadeIn"])
	{
		// There seems to be a retain cycle here. 
		// Clear the animations list to break the cycle so the WindowController can be released.
		[[self window] setAnimations:[NSDictionary dictionary]];
	}
}

- (void) startWindowFadeOutAnimation:(id)user_data
{
	CABasicAnimation* fade_animation = [CABasicAnimation animation];
	[fade_animation setValue:@"windowFadeOut" forKey:@"name"];
	fade_animation.toValue = [NSNumber numberWithFloat:0.0f];
	fade_animation.delegate = self;
	fade_animation.duration = WELCOME_FADE_OUT_DURATION;
	fade_animation.removedOnCompletion = YES;
	[[self window] setAnimations:[NSDictionary dictionaryWithObject:fade_animation forKey:@"alphaValue"]];
	[[[self window] animator] setAlphaValue:0.0];
	windowFadeAnimationStartTime = CACurrentMediaTime();
}


#pragma mark Window delegate methods

#if ENABLE_WINDOW_FADE_ANIMATIONS
// Override to prevent window from immediately closing so we can animate its departure.
- (BOOL) windowShouldClose:(id)the_sender
{
    BOOL shouldClose = YES;
    
	if ( nil != windowShouldCloseBlock )
	{
		shouldClose = windowShouldCloseBlock();
	}
    
    if (shouldClose)
    {
        // Suspend the Lua runtime because we sometimes have a timing issue where the owning view wont get
        // fully autoreleased but the window is gone which has caused crashes when timers fire.  This only
        // ever happens if an app is dragged to the Simulator's Dock icon.  When the window truly closes,
        // self.view is released and the runtime is deleted.
        Rtt::Runtime *runtime = self.view.runtime;
        runtime->Suspend();

        self.windowGoingAway = YES;
        [self startWindowFadeOutAnimation:nil];
    }
    
    // startWindowFadeOutAnimation closes the window when it's done fading so we always return NO
	return NO;
}
#endif

// Warning: Watch out for the control flow due to overriding windowShouldClose for fadeout
// NSWindow performClose will invoke windowShouldClose which will avoid calling this method if it returns NO.
// But NSWindowController close or NSWindow close will bypass windowShouldClose and come here directly.
- (void) windowWillClose:(NSNotification*)the_notification
{
    // If this window is showing the colorPanel, hide it
    [self hideColorPanel];
    [self setColorPanelCallbackBlock:nil];

	if ( nil != windowCloseCompletionBlock )
	{
		windowCloseCompletionBlock();
	}
}

- (void) resurrectWindow
{
	if(NO == self.windowGoingAway)
	{
		return;
	}
	/*
	 if(windowNeedsResurrection == needs_resurrection)
	 {
	 return;
	 }
	 windowNeedsResurrection = needs_resurrection;
	 */	
	
	NSNumber* elapsed_time = [NSNumber numberWithDouble:CACurrentMediaTime() - windowFadeAnimationStartTime];
	[[self window] setAnimations:[NSDictionary dictionary]];
	// NSLog(@"elapsed_time: %@", elapsed_time);
	
	[self startWindowFadeInAnimation:elapsed_time];
	
	//		windowNeedsResurrection = NO;
	self.windowGoingAway = NO;
    
}


- (void) setWindowShouldCloseBlock:(BOOL (^)(void))block
{
	[windowShouldCloseBlock release];
	windowShouldCloseBlock = [block copy];
}

- (void) setWindowDidCloseCompletionBlock:(void (^)(void))block
{
	[windowCloseCompletionBlock release];
	windowCloseCompletionBlock = [block copy];
}

- (void) setWindowWillResizeBlock:(BOOL (^)(int oldWidth, int oldHeight, int newWidth, int newHeight))block
{
	[windowWillResizeBlock release];
	windowWillResizeBlock = [block copy];
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
    NSRect windowFrame = [self.window frame];
    NSRect contentFrame = [[self.window contentView] frame];
    int titleBarHeight = 0;

	// If we've hidden the titlebar pretend its height is zero
	if (([self.window styleMask] & NSFullSizeContentViewWindowMask) != 0)
	{
		titleBarHeight = 0;
	}
	else
	{
		titleBarHeight = windowFrame.size.height - contentFrame.size.height;
	}

    // NSLog(@"CoronaWindowController:windowWillResize: old %@ - new %@", NSStringFromSize(windowFrame.size), NSStringFromSize(frameSize));
    
    // Corona wants to see the size of the content view
    frameSize.height -= titleBarHeight;
    
    BOOL doResize = YES;
    
    if ( nil != windowWillResizeBlock )
	{
        // the Lua listener can reject the resize attempt by returning false
        doResize = windowWillResizeBlock(windowFrame.size.width, contentFrame.size.height,
                                             frameSize.width, frameSize.height);
    }
    
    if (doResize)
    {
        [fView setFrameSize:frameSize];

		// Add titleBarHeight back so that the window ends up the correct size
		frameSize.height += titleBarHeight;

        return frameSize;
    }
    else
    {
        return windowFrame.size;
    }
}

// Call the resize callback for fullscreen events
- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    [fView.glView setInFullScreenTransition:YES];
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
    // NSLog(@"CoronaWindowController:windowDidEnterFullScreen: %@", notification);
    [fView.glView setInFullScreenTransition:NO];
    [self windowWillResize:self.window toSize:[self.window frame].size];
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
    [fView.glView setInFullScreenTransition:YES];
}
- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    // NSLog(@"CoronaWindowController:windowDidExitFullScreen: %@", notification);
    [fView.glView setInFullScreenTransition:NO];
    [self windowWillResize:self.window toSize:[self.window frame].size];
}

//
// We want certain menu items (specifically "Zoom in" and "Zoom out") and their associated
// accelerator keys to work correctly for CoronaView windows (specifically Composer) so we
// define handlers that Cocoa will call via the Main Menu here and translate them into
// key events the Lua app can understand.
//
- (void)dispatchEvent:(Rtt::MEvent*)e
{
	using namespace Rtt;
    
	Runtime* runtime = self.view.runtime;
	Rtt_ASSERT( runtime );
    
	if ( Rtt_VERIFY( e ) )
	{
		runtime->DispatchEvent( * e );
	}
}

- (void)zoomIn:(id)sender
{
    using namespace Rtt;
    
    // Fake a Cmd + keystroke
	KeyEvent e(
               NULL,
               KeyEvent::kDown,
               "+",
               24,
               false,  // (modifierFlags & NSShiftKeyMask) || (modifierFlags & NSAlphaShiftKeyMask),
               false,  // (modifierFlags & NSAlternateKeyMask),
               false,  // (modifierFlags & NSControlKeyMask),
               true ); // (modifierFlags & NSCommandKeyMask) );
	[self dispatchEvent: ( & e )];
}

- (void)zoomOut:(id)sender
{
    using namespace Rtt;
    
    // Fake a Cmd - keystroke
	KeyEvent e(
               NULL,
               KeyEvent::kDown,
               "-",
               27,
               false,  // (modifierFlags & NSShiftKeyMask) || (modifierFlags & NSAlphaShiftKeyMask),
               false,  // (modifierFlags & NSAlternateKeyMask),
               false,  // (modifierFlags & NSControlKeyMask),
               true ); // (modifierFlags & NSCommandKeyMask) );
	[self dispatchEvent: ( & e )];
}

// Handle Color Panel functionality of Simulator Extension windows

- (void) setColorPanelCallbackBlock:(void (^)(double r, double g, double b, double a))block
{
	[fColorPanelCallbackBlock release];
	fColorPanelCallbackBlock = [block copy];
}

- (void) colorPanelAction:(id) sender
{
    NSColorPanel *colorPanel = (NSColorPanel *) sender;
    NSColor *color = [[colorPanel color] colorUsingColorSpace:[NSColorSpace deviceRGBColorSpace]]; // Ensure color is RGB

    // NSLog(@"colorPanelAction: %@", color);

    if (fColorPanelCallbackBlock != nil)
    {
        fColorPanelCallbackBlock([color redComponent], [color greenComponent], [color blueComponent], [color alphaComponent]);
    }
}

- (void) hideColorPanel
{
	// Cocoa doesn't provide an API to hide the color panel
	// so we have to do it by hand
    NSArray *windows = [NSApp windows];

    for( NSWindow *w in windows)
    {
        if( [w isKindOfClass:[NSColorPanel class]] )
        {
            [w orderOut:nil];
            break;
        }
    }
}

// Called when the window moves to a screen with different "backing properties" (i.e. retina to non-retina and vice versa)
- (void)windowDidChangeBackingProperties:(NSNotification *)notification
{
	fView.glView.scaleFactor = [[self window] backingScaleFactor];
	[fView.glView setNeedsDisplay:YES];
}

// This notification serves as a way to tell that the window is on a screen and
// that we can reliably query the screen's backingScaleFactor (the system doesn't
// send windowDidChangeBackingProperties: when the window is first displayed)
- (void)windowDidChangeOcclusionState:(NSNotification *)notification
{
	if ([self window].occlusionState & NSWindowOcclusionStateVisible)
	{
		fView.glView.scaleFactor = [[self window] backingScaleFactor];

		[fView.glView restoreWindowProperties];
	}
}


//Support Dragging main.lua for Sim Only projects only at the moment
#if Rtt_AUTHORING_SIMULATOR
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
    NSPasteboard *pboard;
    pboard = [sender draggingPasteboard];
    if (@available(macOS 10.13, *)) {
        if ( [[pboard types] containsObject:NSPasteboardTypeFileURL] ) {
            NSString *fileURL = [[NSURL URLFromPasteboard:pboard] path];
            NSArray *splitPath = [fileURL componentsSeparatedByString:@"/"];
            if([splitPath.lastObject isEqualToString:@"main.lua"] ){
                return NSDragOperationLink;
            }
            
        }
    }
    return NSDragOperationNone;
}
//Support for Drag a main.lua on to Welcome Screen
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
    NSPasteboard *pboard;
    pboard = [sender draggingPasteboard];
    if (@available(macOS 10.13, *)) {
        if ( [[pboard types] containsObject:NSPasteboardTypeFileURL] ) {
            NSString *fileURL = [[NSURL URLFromPasteboard:pboard] path];
            NSArray *splitPath = [fileURL componentsSeparatedByString:@"/"];
            
                if([splitPath.lastObject isEqualToString:@"main.lua"] ){
                    AppDelegate * appDelegate = (AppDelegate*)[NSApp delegate];
                    Rtt::MacSimulatorServices * simulatorServices = new Rtt::MacSimulatorServices(appDelegate, (CoronaWindowController *)self, nil);
                    simulatorServices->OpenProject( [[fileURL stringByReplacingOccurrencesOfString:@"main.lua" withString:@""] UTF8String] );
                }
        }
    }
    return YES;
}
#endif
@end

// ----------------------------------------------------------------------------
