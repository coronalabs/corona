//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "SkinnableWindow.h"

#include "Core/Rtt_Build.h"

#import <Cocoa/Cocoa.h>
#import <AvailabilityMacros.h>

#import "AppDelegate.h"
#import "SkinView.h"
#import "GLView.h"

#include "Rtt_Event.h"
#include "Rtt_Runtime.h"

#include "Rtt_MacSimulator.h"

#ifdef Rtt_DEBUG
	#include "Rtt_GPUStream.h"
#endif

// ----------------------------------------------------------------------------

@interface SkinnableWindow()
- (void)scaleDidChange;
- (void)setSkinImage:(NSString*)path;
- (NSSize) updateSkinFrameSize;
@end

@implementation SkinnableWindow

// Borderless windows don't handle performClose:
// Because I want to close them via the menu, I need to override performClose:
// Because our original implementation did not properly implement the close for the frontmost window, 
// this will redirect the message back to the appdelegate which does the management for both closing 
// the window/shuting down the simulator and bringing back the welcome window.
// I think a cleaner way to handle this is to just close the window and have a notification system handle the cleanup in the AppDeleage.
- (IBAction)performClose:(id)sender
{
	if ( nil != performCloseBlock )
	{
		performCloseBlock(sender);
	}	
}

// For performClose: to not be disabled in the menu for the borderless window, we must override the validation callback.
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    return ([menuItem action]==@selector(performClose:))?YES:[super validateMenuItem:menuItem];
}


- (id)initWithScreenView:(GLView*)screenView
				viewRect:(NSRect)screenRect
                   title:(NSString*)title
               skinImage:(NSString*)path
             orientation:(Rtt::DeviceOrientation::Type)orientation
                   scale:(float)scale
		   isTransparent:(BOOL)isTransparent
{
	self = [super initWithContentRect:NSZeroRect
							styleMask:NSBorderlessWindowMask
							  backing:NSBackingStoreBuffered
							 	defer:NO];
	if ( self )
	{
		fSkinView = nil;
		fScreenView = [screenView retain];
		fScreenRect = screenRect;
		fExponent = 0;
		fCurrentSkinOrientation = orientation;
        fOriginalSkinOrientation = orientation;
		// Tell OpenGL we want it to use the best resolution the current display
		// is capable of so that we take advantage of Retina screens
		[fScreenView setWantsBestResolutionOpenGLSurface:YES];
		[fScreenView setZoomLevel:1.0];
		
		fIsTransparent = isTransparent;

		if (isTransparent)
		{
			NSOpenGLContext* context = [fScreenView openGLContext];
			GLint opacity = 0;

			[context setValues: &opacity forParameter: NSOpenGLCPSurfaceOpacity];
		}

		// Set the background color to clear so that (along with the setOpaque 
		// call below) we can see through the window.
		[self setBackgroundColor:[NSColor clearColor]];
		[self setOpaque:NO];

		[self setTitle:title];

		[self setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

		[[NSApplication sharedApplication] addWindowsItem:self title:title filename:NO];

		[self setSkinImage:path];
		[self setScale:scale];
 		[self setOrientation:orientation];
	}
	return self;
}

- (void)setOrientation:(Rtt::DeviceOrientation::Type)orientation
{
	using namespace Rtt;

	Rtt_ASSERT( Rtt::DeviceOrientation::IsInterfaceOrientation( orientation ) );

	fCurrentSkinOrientation = orientation;
	
	[fSkinView setOrientation:orientation];

	NSSize newskinsize = [self updateSkinFrameSize];
	[self updateGLViewFrameSize];
	[self updateGLViewFrameOrigin];
	
	[self setFrame:NSMakeRect([self frame].origin.x, [self frame].origin.y, newskinsize.width, newskinsize.height) display:YES];
	
	AppDelegate *delegate = (AppDelegate *)[NSApp delegate];
	MacSimulator *simulator = delegate.simulator;
	
	Runtime* runtime = [fScreenView runtime];
	if( NULL != runtime)
	{
		runtime->WindowDidRotate( orientation, simulator->IsOrientationSupported( orientation ) );
		runtime->WindowSizeChanged();
	}
	[fScreenView invalidate];

}

- (void) setSkinImage:(NSString*)path
{
 	NSImage* image = [[NSImage alloc] initByReferencingFile:path];
	NSView* contentView = [self contentView];

	if ( image )
	{
		Rtt_ASSERT( fScreenView );
		if ( ! fSkinView )
		{
			SkinView* skinView = [[SkinView alloc] initWithFrame:NSZeroRect isTransparent:fIsTransparent];
			fSkinView = skinView;
			[skinView autorelease];
		}

		// Add check for absolute path
		NSURL* urlpath = [NSURL fileURLWithPath:path];
		if (! [fSkinView setImageWithURL:urlpath])
        {
            Rtt_TRACE_SIM(("Error: could not load skin image '%s'\n", [path UTF8String]));
            
            // return;
        }

		[fSkinView setFrameSize:[fSkinView imageSize]];

		[fScreenView removeFromSuperview];
		[contentView addSubview:fSkinView];
		[contentView addSubview:fScreenView];

        // Setting frame size can trigger things like rerenders downstream, so don't do it unless necessary
        if (! NSEqualRects([fScreenView frame], fScreenRect))
        {
            [fScreenView setFrame:fScreenRect];
            [fScreenView update];
        }

		[fSkinView setNextResponder:self];
		[fScreenView setNextResponder:self];

		// TODO: Currently the window gets destroyed when the skin changes. If we don't want to destroy the window, we need to recompute things.
		// The 3 update methods will probably cover this.
	}
    else
    {
        Rtt_TRACE_SIM(("Error: failed to load skin image '%s'\n", [path UTF8String]));
    }
}

- (NSSize) nativeSize
{
	return [fSkinView nativeSize];
}

- (NSSize) updateSkinFrameSize
{
	NSSize new_skin_size = [self computeSkinFrameSize];

	[fSkinView setFrameSize:new_skin_size];

	return new_skin_size;
}

- (void)rotate:(BOOL)clockwise
{
	using namespace Rtt;

	NSSize new_skin_size;
	NSSize new_glview_size;
	NSPoint new_glview_origin;

//	NSLog(@"rotating1 skinView frame    %@", NSStringFromRect([fSkinView frame]));

	// Rotate simulator skin image
	S32 angle = [fSkinView rotate:clockwise];
	
	DeviceOrientation::Type orientation = DeviceOrientation::OrientationForAngle( angle );

	fCurrentSkinOrientation = orientation;
	[fScreenView setOrientation:orientation];
	
	new_skin_size = [self updateSkinFrameSize];
	new_glview_size = [self updateGLViewFrameSize];
	new_glview_origin = [self updateGLViewFrameOrigin];
	
	// Updates the invisible window frame to fit the new dimensions
	// TODO: To support rotation animations, the window needs to be slightly larger (Pythagorean theorm) so the edges don't get clipped.
	// We should make the window bigger and have a callback to fit on animation completion.

	// Need to account for the titlebar height and other window thingys
	NSRect framerectforcontentrect = [self frameRectForContentRect:NSMakeRect([self frame].origin.x, [self frame].origin.y, new_skin_size.width, new_skin_size.height)];

	[self setFrame:framerectforcontentrect display:YES];
	
//	NSLog(@"rotating1 contentView frame    %@", NSStringFromRect([[self contentView] frame]));
	AppDelegate *delegate = (AppDelegate *)[NSApp delegate];
	MacSimulator *simulator = delegate.simulator;
	
	Runtime* runtime = [fScreenView runtime];
	
	runtime->WindowDidRotate( orientation, simulator->IsOrientationSupported( orientation ) );
}

- (void)scaleDidChange
{
	CGFloat newValue = [self scale];

	// This method will record a scale factor in the GLView and also trigger special handling for native display objects.
	[fScreenView setZoomLevel:newValue];

	// Note that the scale changed as [setFrame:display:] needs to know to adjust the window's y correctly
	fScaleDidChange = YES;

    /*
	NSLog(@"old frame             %@", NSStringFromRect([self frame]));
	NSLog(@"old contentView frame %@", NSStringFromRect([[self contentView] frame]));
	NSLog(@"old screenView frame  %@", NSStringFromRect([fScreenView frame]));
	NSLog(@"old screenRect frame  %@", NSStringFromRect(fScreenRect));
	NSLog(@"old skinView frame    %@", NSStringFromRect([fSkinView frame]));
	NSLog(@"topLeftAdjustment     %d", topLeftAdjustment);
     */
	
	NSSize nativeskinsize = [fSkinView nativeSize];
	NSSize newskinsize = NSMakeSize(nativeskinsize.width * newValue, nativeskinsize.height * newValue);
									
	[fSkinView setFrameSize:newskinsize];

	newskinsize = [self updateSkinFrameSize];
	[self updateGLViewFrameSize];
	[self updateGLViewFrameOrigin];

	// Resize the window
	NSRect framerectforcontentrect = [self frameRectForContentRect:NSMakeRect([self frame].origin.x,
                                                                              [self frame].origin.y,
                                                                              newskinsize.width,
                                                                              newskinsize.height)];
    //framerectforcontentrect.origin.y -= topLeftAdjustment;
    //topLeftAdjustment = [self frame].size.height - newskinsize.height;
	// NSLog(@"new frame             %@", NSStringFromRect(framerectforcontentrect));
	[self setFrame:framerectforcontentrect display:NO];

	Rtt::Runtime* runtime = [fScreenView runtime];
	if( NULL != runtime )
	{
		runtime->WindowSizeChanged();
	}
	
	// Simulator's DidChangeScale will save scale value to the user preference
	AppDelegate* delegate = (AppDelegate*)[NSApp delegate];
	Rtt::MacSimulator* simulator = delegate.simulator;
	simulator->DidChangeScale(newValue);
}

- (void)mouseDown:(NSEvent*)e
{
	//NSLog( @"mouseDown: %@", e );

	NSRect windowFrame = [self frame];
	NSPoint locationPoint = [NSEvent mouseLocation];

	fMouseDownLocation = locationPoint;

	fMouseDownLocation.x -= windowFrame.origin.x;
	fMouseDownLocation.y -= windowFrame.origin.y;
}

- (void)mouseDragged:(NSEvent*)e
{
	// NSLog( @"mouseDragged: %@", e );
	NSPoint currentLocation;
	NSPoint newOrigin;
	NSPoint locationPoint = [NSEvent mouseLocation];

	currentLocation = locationPoint;

	newOrigin.x = currentLocation.x - fMouseDownLocation.x;
	newOrigin.y = currentLocation.y - fMouseDownLocation.y;

	// Move the window to the new location
	// Don't move the window in fullscreen mode	or when changing screens
	if ( ! ([NSApp presentationOptions] & NSApplicationPresentationFullScreen) )
	{
		[self setFrameOrigin:newOrigin];
	}
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

@end
