//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "SimulatorDeviceWindow.h"
#import "GLView.h"

#if Rtt_DEBUG
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#else
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#endif

@implementation SimulatorDeviceWindow

@synthesize saveFrameName;
@synthesize fScreenView;

- (void)dealloc
{
	[[NSApplication sharedApplication] removeWindowsItem:self];
	[fScreenView removeFromSuperview];
	// Fix bug #18412 (performs scheduled to move textfield focus were crashing on reload)
	[NSObject cancelPreviousPerformRequestsWithTarget:self];
	[fScreenView release];
	[saveFrameName release];
	[performCloseBlock release];
	[super dealloc];
}

// On 10.10, [NSWindow saveFrameUsingName:] saves the screen dimensions for neither [NSScreen frame]
// nor [NSScreen visibleFrame] (it's some other related but different size) making identifying the
// screen on reload difficult.  Here we just save [NSScreen frame] with the window coordinates thus
// overriding whatever the OS is doing and gaining consistency
- (void)saveFrameUsingName:(NSString *)frameName
{
    NSString *windowLocPrefName = [NSString stringWithFormat:@"NSWindow Frame %@", frameName];
    NSRect frame = [self frame];
    NSRect screenFrame = [[self screen] frame];

    // TODO: support remembering and restoring the device orientation; for now, make sure we remember the "upright" window
    if (fCurrentSkinOrientation != fOriginalSkinOrientation)
    {
        // Swap width and height
        NSInteger tmp = frame.size.height;
        frame.size.height = frame.size.width;
        frame.size.width = tmp;
    }

    NSString *windowLocPrefValue = [NSString stringWithFormat:@"%.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f ",
                                    frame.origin.x,frame.origin.y, frame.size.width, frame.size.height,
                                    screenFrame.origin.x, screenFrame.origin.y, screenFrame.size.width, screenFrame.size.height];

    [[NSUserDefaults standardUserDefaults] setObject:windowLocPrefValue forKey:windowLocPrefName];
}

// On 10.10 this [NSWindow orderWindow:] forces the window onto the same display as the app's main window regardless of
// the window's actual screen position.  Setting the window's position offscreen is enough to prevent this.
- (void)orderWindow:(NSWindowOrderingMode)orderingMode relativeTo:(NSInteger)otherWindowNumber
{
    NSDEBUG(@"orderWindow orderingMode %ld, otherWindowNumber %ld", (long)orderingMode, (long)otherWindowNumber);
    NSDEBUG(@"orderWindow: BEFORE frame %@", NSStringFromRect([self frame]));

    NSRect currFrame = [self frame];
    NSRect origFrame = currFrame;
    currFrame.origin.x = -5000;
    currFrame.origin.y = -5000;
    [super setFrame:currFrame display:NO];
    [super orderWindow:orderingMode relativeTo:otherWindowNumber];

    [super setFrame:origFrame display:NO];
    NSDEBUG(@"orderWindow: AFTER  frame %@", NSStringFromRect([self frame]));
}

//
// On 10.9 (Mavericks) Apple added a Mission Control setting called "Displays have separate Spaces" which,
// aside from allocating separate Spaces to each display, implements various rules that it feels apps should
// follow.  One of these is window locality: all app windows should be on the same display unless moved to
// another by the user *in this session*.  The upshot of this is that newly created windows will be created
// on which ever display the OS feels is the principal one for the app.  In the Simulator this means that if
// you put the app window on a different display it will be moved back to the "principal" display whenever
// the window is created (which happens on application launch and relaunches of the project).  Luckily all
// we need to do to get the old, desired behavior of having the app window appear wherever it was last put is to
// position the window using the saved position from the user preferences but avoiding NSWindow's methods that
// are there to parse and set the preference value (i.e. setFrameUsingName: and setFrameFromString:).  To keep
// things as simple as possible, we revert to the default behavior unless the "Displays have separate Spaces"
// system preference is set (it's interesting that the capability to query this value exists as it's of little
// interest to an application except for cases like this).
//
// Sample window frame pref value:
// "NSWindow Frame iPhone5.png" = "2001 389 195 95 1920 -280 1440 900 ";
//
- (BOOL) setFrameUsingName:(NSString *)frameName
{
    // Remember this frame's name for later in case we need to resave when the window moves
    saveFrameName = [frameName copy];
    [self setDelegate:self];

    BOOL result = NO;
    NSString *windowLocPref = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithFormat:@"NSWindow Frame %@", frameName]];

    if (([windowLocPref length] > 0) &&
        ([NSScreen respondsToSelector:@selector(screensHaveSeparateSpaces)] && [NSScreen screensHaveSeparateSpaces]))
    {
        NSRect windowFrame;
        NSRect screenFrame;
        NSRect currFrame = [self frame];
        
        NSDEBUG(@"setFrameUsingName: screensHaveSeparateSpaces! %@: %@", frameName, windowLocPref);
        
        // Parse the preference value with a regular expression (it's a series of optionally-signed integers separated by white space)
        NSError *error = nil;
        NSRegularExpression *windowLocRegex = [NSRegularExpression regularExpressionWithPattern:@"([-]*\\d+)\\s+"
                                                                                        options:NSRegularExpressionCaseInsensitive
                                                                                          error:&error];
        NSArray *matches = [windowLocRegex matchesInString:windowLocPref
                                                   options:0
                                                     range:NSMakeRange(0, [windowLocPref length])];
        
        // There must be 8 captures for us to parse this pref correctly
        Rtt_ASSERT([matches count] == 8);
        if ([matches count] == 8)
        {
            // This is ugly but it merely continues the inherent knowledge of the format of the preference value
            windowFrame.origin.x = [[windowLocPref substringWithRange:[[matches objectAtIndex:0] range]] intValue];
            windowFrame.origin.y = [[windowLocPref substringWithRange:[[matches objectAtIndex:1] range]] intValue];
            windowFrame.size.width = currFrame.size.width; // don't restore the width and height as the new app
            windowFrame.size.height = currFrame.size.height; // may have a different orientation to the last
            
            screenFrame.origin.x = [[windowLocPref substringWithRange:[[matches objectAtIndex:4] range]] intValue];
            screenFrame.origin.y = [[windowLocPref substringWithRange:[[matches objectAtIndex:5] range]] intValue];
            screenFrame.size.width = [[windowLocPref substringWithRange:[[matches objectAtIndex:6] range]] intValue];
            screenFrame.size.height = [[windowLocPref substringWithRange:[[matches objectAtIndex:7] range]] intValue];

            if (NSEqualRects(screenFrame, NSZeroRect))
            {
                // Sometimes a zero screen rect gets saved to the preferences, ignore anything saved for this device
                [self center];
                result = YES;
            }
            else
            {
                // Only attempt to use this saved position if we have an existing screen that matches the one it was on
                // when the window's position was saved
                for (NSScreen *screen in [NSScreen screens])
                {
                    // NSDEBUG(@"screen: %@", [screen deviceDescription]);
                    NSDEBUG(@"setFrameUsingName: [screen frame] %@; [screen visibleFrame] %@; window's screenFrame %@",
                            NSStringFromRect([screen frame]), NSStringFromRect([screen visibleFrame]), NSStringFromRect(screenFrame));

                    if (NSEqualRects([screen frame], screenFrame))
                    {
                        // Set the window's size and position ourselves
                        NSDEBUG(@"BEFORE: self frame: %@", NSStringFromRect([self frame]));
                        [self setFrame:windowFrame display:YES];
                        NSDEBUG(@"AFTER:  self frame: %@", NSStringFromRect([self frame]));
                        result = YES;

                        break;
                    }
                }
            }
        }
    }

     // If we haven't successfully restored the frame yet and there's a saved preference, do it the old fashioned way
    if (! result && [windowLocPref length] > 0)
    {
        result = [super setFrameUsingName:frameName];
    }

    return result;
}

// Pin the window's top-left corner during resizing (scaling)
- (void) setFrame:(NSRect)frameRect display:(BOOL)flag
{
	NSRect oldRect = [self frame];
    NSRect screenRect = [[self screen] frame];

    // Happens if they somehow move the window offscreen and on first display
    if (NSEqualRects(screenRect, NSZeroRect))
    {
        // Figure out which screen the window will be on
        for (NSScreen *screen in [NSScreen screens])
        {
            if (NSPointInRect(frameRect.origin, [screen frame]))
            {
                screenRect = [screen frame];
                break;
            }
        }
        
        // Last resort
        if (NSEqualRects(screenRect, NSZeroRect))
        {
            screenRect = [[NSScreen mainScreen] frame];
        }
    }
	
    NSDEBUG(@"setFrame: frame old %@, new %@;\n\tscreen %@ (fScaleDidChange %s)", NSStringFromRect(oldRect), NSStringFromRect(frameRect), NSStringFromRect(screenRect), (fScaleDidChange?"YES":"NO"));
	//NSDEBUG(@"setFrame: %@",[NSThread callStackSymbols]);
    
    if (fScaleDidChange)
    {
        // If we don't have an old size, use the new one (this covers a degenerate case where we have a
        // scaleFactor for the skin but no saved position)
        if (oldRect.size.height <= 0)
        {
            oldRect.size.height = frameRect.size.height;
        }
        
        // Adjust frame's Y so that top left corner of window doesn't appear to move
        frameRect.origin.y = oldRect.origin.y + oldRect.size.height - frameRect.size.height;
        
        // Stop the window leaving the screen (you can drag it off if you want to but
        // we wont ever leave it off screen due to resizing)
        int menubarHeight = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];
        
        if (frameRect.origin.y > (screenRect.size.height - [[[NSApplication sharedApplication] mainMenu] menuBarHeight]))
        {
            frameRect.origin.y = screenRect.size.height - frameRect.size.height - menubarHeight;
        }
        
        if (frameRect.origin.x < screenRect.origin.x)
        {
            frameRect.origin.x = screenRect.origin.x;
        }

		// OS X flips a window to a different screen if more than ~50% of it is positioned off the current
		// screen (by dragging or programmatically).  This is an issue when we have very large Simulator
		// windows and small screens like the default size of a Retina laptop screen.  Unfortunately this
		// behavior is hardwired so we have to try to move the window to somewhere that wont trigger it.
		// When the size of the Simulator is lot larger than the size of the screen (e.g. a 1920x1080
		// device on a default MacBookPro Retina screen, this is a kludge at best).

		if (screenRect.origin.x < 0 &&
			(frameRect.origin.x + (frameRect.size.width/2)) > (screenRect.origin.x + (screenRect.size.width/2)))
		{
			// Place it slightly off center
			frameRect.origin.x = screenRect.origin.x - ((frameRect.size.width / 2) + 200);
		}

		if (screenRect.origin.y < 0 &&
			(frameRect.origin.y + (frameRect.size.height/2)) < (screenRect.origin.y + (screenRect.size.height/2)))
		{
			// Pin the top of the window to the top of the screen
			frameRect.origin.y = screenRect.origin.y - (frameRect.size.height - screenRect.size.height);
		}

		NSDEBUG(@"setFrame:new frame %@ (occluded %ld) %ld", NSStringFromRect(frameRect), [self occlusionState], NSWindowOcclusionStateVisible);
        
        fScaleDidChange = NO;
    }

	[super setFrame:frameRect display:flag];
}

- (void)constrainFrameRectToScreen
{
	// use message syntax instead of dot syntax because of clang crash in Xcode 4 preview.
	// visibleFrame isn't a property, so this is better anyway.
	NSRect screenRect = [[self screen] visibleFrame];
	NSSize screenSize = screenRect.size;
	NSSize frameSize = [self frame].size;
	CGFloat frameHeight = frameSize.height;
	CGFloat frameWidth = frameSize.width;
	CGFloat screenHeight = screenSize.height;
	CGFloat screenWidth = screenSize.width;
	if ( frameHeight > screenHeight || frameWidth > screenWidth )
	{
		// aspect ratio is width/height
		CGFloat screenAspect = screenWidth / screenHeight;
		CGFloat frameAspect = frameWidth / frameHeight;

		// Map ratios into ranges [1,2) [2,4) [4,8) and so on.
		// Take log base 2 of the ratios and then ceil that number to get exponent.
		CGFloat ratio = ( frameAspect < screenAspect ? frameHeight / screenHeight : frameWidth / screenWidth );
		const float kInvLog2 = log2( 2 );
		S32 exponent = (S32)( 1. + log2( ratio ) * kInvLog2 ); // Add 1 to achieve ceil
		fExponent = -exponent; // minus sign b/c zoom out
		[self scaleDidChange];
	}
}

- (NSSize) nativeSize
{
	// to be overriden by subclasses

	return NSZeroSize;
}

- (void)scaleDidChange
{
	// to be overriden by subclasses
}

- (NSSize) computeSkinFrameSize
{
	using namespace Rtt;

	NSSize native_skin_size = [self nativeSize];
	float scale_factor = [self scale];

	NSSize new_skin_size;

	DeviceOrientation::Type orientation = fCurrentSkinOrientation;

	if( DeviceOrientation::IsUpright( orientation ) )
	{
		new_skin_size = NSMakeSize(native_skin_size.width * scale_factor, native_skin_size.height * scale_factor);
	}
	else
	{
		new_skin_size = NSMakeSize(native_skin_size.height * scale_factor, native_skin_size.width * scale_factor);
	}

	return new_skin_size;
}

- (NSPoint) updateGLViewFrameOrigin
{
	using namespace Rtt;

	NSSize native_skin_size = [self nativeSize];
	float scale_factor = [self scale];

	NSPoint new_glview_origin;

	DeviceOrientation::Type orientation = fCurrentSkinOrientation;

	if( DeviceOrientation::kUpsideDown == orientation )
	{
		// When the skin is upside-down, remember that we don't actually change the GLView and its coordinates are still right-side-up.
		// Also remember that the skin might be asymmetrical, so we need to adjust the x,y offset of the GLView to adjust for this.
		// Also don't forget to account for the scale.
		// new_offset = (skin_size - glview_size) - glview_offset

		// The 'new_sizes' are already scaled, but the fScreenRect is not.
		new_glview_origin.x = (native_skin_size.width * scale_factor - fScreenRect.size.width * scale_factor) - (fScreenRect.origin.x * scale_factor);
		new_glview_origin.y = (native_skin_size.height * scale_factor - fScreenRect.size.height * scale_factor) - (fScreenRect.origin.y * scale_factor);
	}
	else if( DeviceOrientation::kSidewaysLeft == orientation )
	{
		new_glview_origin.x = fScreenRect.origin.y * scale_factor;
		new_glview_origin.y = (native_skin_size.width * scale_factor - fScreenRect.size.width * scale_factor) - (fScreenRect.origin.x * scale_factor);
	}
	else if( DeviceOrientation::kSidewaysRight == orientation )
	{
		// When the skin is sideways-right (home button on the right), the offset coordinates are swapped for x,y and also
		// the coordinate system is relative to the top-left corner of the skin.
		// Also don't forget to account for the scale.

		// new_offset_x = (skin_size.height - glview_size.height) - glview_old_origin.y
		// new_offset_y = (skin_size.width - glview_size.width) - glview_old_origin.x

		// The 'new_sizes' are already scaled, but the fScreenRect is not.
		new_glview_origin.x = (native_skin_size.height  * scale_factor - fScreenRect.size.height  * scale_factor) - (fScreenRect.origin.y * scale_factor);
		//		new_glview_origin.y = (new_skin_size.width - fScreenRect.size.width) - (fScreenRect.origin.x * scale_factor);
		new_glview_origin.y = fScreenRect.origin.x * scale_factor;
	}
	else
	{
		new_glview_origin.x = fScreenRect.origin.x * scale_factor;
		new_glview_origin.y = fScreenRect.origin.y * scale_factor;
	}

	// Don't allow fractional origin coordinates (it causes blurriness in the GL view)
	new_glview_origin.x = floor(new_glview_origin.x);
	new_glview_origin.y = floor(new_glview_origin.y);

	[fScreenView setFrameOrigin:new_glview_origin];

	return new_glview_origin;
}

- (NSSize) updateGLViewFrameSize
{
	using namespace Rtt;

	float scale_factor = [self scale];

	NSSize new_glview_size;

	DeviceOrientation::Type orientation = fCurrentSkinOrientation;

	// This fixes a display glitch when the OpenGL canvas width is an uneven number
	int widthAdj = 0;
	int heightAdj = 0;

	if ((lrint(fScreenRect.size.width) % 2) != 0)
	{
		widthAdj = -1;
	}

	if ((lrint(fScreenRect.size.height) % 2) != 0)
	{
		heightAdj = -1;
	}

	if( DeviceOrientation::IsUpright( orientation ) )
	{
		new_glview_size = NSMakeSize((fScreenRect.size.width + widthAdj) * scale_factor, (fScreenRect.size.height + heightAdj) * scale_factor);
	}
	else
	{
		new_glview_size = NSMakeSize((fScreenRect.size.height + heightAdj) * scale_factor, (fScreenRect.size.width + widthAdj) * scale_factor);
	}

	[fScreenView setZoomLevel:scale_factor];

	[fScreenView setFrameSize:new_glview_size];

	return new_glview_size;
}

- (BOOL)canZoomIn
{
	return fExponent < 0;
}

- (BOOL)canZoomOut
{
	return fExponent > -3;
}

- (void)zoomIn:(id)sender
{
	if ( Rtt_VERIFY( [self canZoomIn] ) )
	{
		++fExponent;
		[self scaleDidChange];
	}
}

- (void)zoomOut:(id)sender
{
	if ( Rtt_VERIFY( [self canZoomOut] ) )
	{
		--fExponent;
		[self scaleDidChange];
	}
}

// Scale by (2^exponent):
// * Set to 100% by passing in 0 for exponent
// * Zoom in by using exponent > 0.
// * Zoom out by using exponent < 0
- (float)scale
{
	int absExponent = Rtt::Abs( fExponent );
	float result = 1 << absExponent;
	if ( fExponent < 1 )
	{
		result = 1. / result;
	}
	return result;
}

- (void)setScale:(float)newValue
{
	const double kInvLog2 = 1.0 / log2(2);
	fExponent = ( log2( newValue ) * kInvLog2 );

	[self scaleDidChange];
}

// Override b/c NSBorderlessWindowMask windows can't become key by default
// and thus controls in such windows won't ever be enabled by default.
- (BOOL) canBecomeKeyWindow
{
	return YES;
}

- (BOOL) canBecomeMainWindow
{
	return YES;
}

- (void) setPerformCloseBlock:(void (^)(id sender))block
{
	[performCloseBlock release];
	performCloseBlock = [block copy];
}

// Called when the window moves to a screen with different "backing properties" (i.e. retina to non-retina and vice versa)
- (void)windowDidChangeBackingProperties:(NSNotification *)notification
{
	NSDEBUG(@"===  windowDidChangeBackingProperties: %@; screen %@: %g", NSStringFromRect([self frame]), [[[self screen] deviceDescription] objectForKey:@"NSDeviceSize"], [self backingScaleFactor]);

	fScreenView.scaleFactor = [self backingScaleFactor];
}

// This notification serves as a way to tell that the window is on a screen and
// that we can reliably query the screen's backingScaleFactor (the system doesn't
// send windowDidChangeBackingProperties: when the window is first displayed)
- (void)windowDidChangeOcclusionState:(NSNotification *)notification
{
	if (self.occlusionState & NSWindowOcclusionStateVisible)
	{
		NSDEBUG(@"windowDidChangeOcclusionState: %@", notification);

		fScreenView.scaleFactor = [self backingScaleFactor];

		[fScreenView restoreWindowProperties];
	}
}

- (void)windowDidMove:(NSNotification *)notification
{
	//NSDEBUG(@"windowDidMove: frameRect %@: %g", NSStringFromPoint([self frame].origin), [self backingScaleFactor]);

    // We moved the window, save the new position so that it's remembered if we are suddenly terminated
    // (this is necessary because we override the default window position restoration process to get the
    // behavior we want, see saveFrameUsingName:)
    if (saveFrameName != nil)
    {
        [self saveFrameUsingName:saveFrameName];
    }
}

@end
