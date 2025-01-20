//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "SkinlessSimulatorWindow.h"

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

@interface SkinlessSimulatorWindow()
- (void)scaleDidChange;
- (NSSize) updateSkinFrameSize;
@end

@implementation SkinlessSimulatorWindow

@synthesize windowTitle;

// Unlike the skinable window, we have both a performClose handler and close.
// performClose eventually calls close, so we don't want our callback to fire twice.
// While performClose simulates hitting the red close button on the window,
// when you hit the real red close button yourself, the performClose method never gets invoked.
// So if I put the callback in performClose, I will fail to hit the clean up case when the user hits the red button.
// And if I put the callback in the close method, I hit a multiple close cycle problem because the AppDelegate is calling on the window and controller.
// The workaround seems to be to take advantage of windowShouldClose: because it gets invoked by both performClose: and the red button, 
// but does not get invoked by calling close.
- (BOOL)windowShouldClose:(id)sender
{
	if ( nil != performCloseBlock )
	{
		performCloseBlock(sender);
	}	
	return YES;
}

- (id)initWithScreenView:(GLView*)screenView
				viewRect:(NSRect)screenRect
				   title:(NSString*)title
			 orientation:(Rtt::DeviceOrientation::Type)orientation
                   scale:(float)scale
		   isTransparent:(BOOL)isTransparent
{
	// Need to make window size larger than the view rect (e.g. large enough to hold it with the titlebar).
	// This is redudnant because the call to setOrientation does this again.
	NSRect framerectforcontentrect = [NSWindow frameRectForContentRect:screenRect styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask];
	
	NSWindowStyleMask styleMask = isTransparent ? NSBorderlessWindowMask : NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
	
	self = [super initWithContentRect:framerectforcontentrect
							styleMask:styleMask
							  backing:NSBackingStoreBuffered
							 	defer:NO];
	if ( self )
	{
		fScreenView = [screenView retain];
		fScreenRect = screenRect;
		fExponent = 0;
		
		// Tell OpenGL we want it to use the best resolution the current display
		// is capable of so that we take advantage of Retina screens
		[fScreenView setWantsBestResolutionOpenGLSurface:YES];
		[fScreenView setZoomLevel:1.0];

		if (isTransparent)
		{
			NSOpenGLContext* context = [fScreenView openGLContext];
			GLint opacity = 0;

			[context setValues: &opacity forParameter: NSOpenGLCPSurfaceOpacity];

			[self setBackgroundColor:[NSColor clearColor]];
		}
		else
		{
			[self setBackgroundColor:[NSColor blackColor]];
		}

		[self setTitle:title];
		windowTitle = [title copy];
		
		[self setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		
		NSView* contentView = [self contentView];

		[[NSApplication sharedApplication] addWindowsItem:self title:title filename:NO];
		[screenView setFrameOrigin:NSMakePoint(0, 0)];
		// We'd like to use a NSScrollView here but the OpenGL viewport is constantly reset to origin 0,0 which
		// makes that impossible without a lot of work
		[contentView addSubview:screenView];
		fCurrentSkinOrientation = orientation;
		
		[self setScale:scale];
		[self setOrientation:orientation];

		fRotationAngle = Rtt::DeviceOrientation::AngleForOrientation( orientation );
	}
	return self;
}

- (void)setOrientation:(Rtt::DeviceOrientation::Type)orientation
{
	using namespace Rtt;
	
	Rtt_ASSERT( Rtt::DeviceOrientation::IsInterfaceOrientation( orientation ) );
	
	fCurrentSkinOrientation = orientation;
	
	NSSize newskinsize = [self updateSkinFrameSize];
	[self updateGLViewFrameSize];
	[self updateGLViewFrameOrigin];
	
	NSRect framerectforcontentrect = [self frameRectForContentRect:NSMakeRect([self frame].origin.x, [self frame].origin.y, newskinsize.width, newskinsize.height)];
	
	[self setFrame:NSMakeRect([self frame].origin.x, [self frame].origin.y, framerectforcontentrect.size.width, framerectforcontentrect.size.height) display:YES];
	
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

- (NSSize) nativeSize
{
	return fScreenRect.size;
}

- (NSSize) updateSkinFrameSize
{
	NSSize new_skin_size = [self computeSkinFrameSize];
	return new_skin_size;
}

- (void)rotate:(BOOL)clockwise
{
	using namespace Rtt;
	
	NSSize new_skin_size;
	NSSize new_glview_size;
	NSPoint new_glview_origin;
	
	//	NSLog(@"rotating1 skinView frame    %@", NSStringFromRect([fSkinView frame]));
	
	fRotationAngle += (clockwise ? -90 : 90 );
	if ( fRotationAngle >= 360 )
	{
		fRotationAngle = fRotationAngle - 360;
	}
	if ( fRotationAngle < 0 )
	{
		fRotationAngle = fRotationAngle + 360;
	}
	S32 angle = fRotationAngle;
	
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
	
	NSSize nativeskinsize = fScreenRect.size;
	NSSize newskinsize = NSMakeSize(nativeskinsize.width * newValue, nativeskinsize.height * newValue);
	
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

// Toggling NSTitledWindowMask when entering and exiting fullscreen works around
// a problem where the window would grow by the height of the title bar on each
// transition (this process is completely vanilla and no custom code is involved)
- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
	NSUInteger styleMask = [self styleMask];
	styleMask &= ~NSTitledWindowMask;
	[self setStyleMask:styleMask];
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
	NSUInteger styleMask = [self styleMask];
	styleMask |= NSTitledWindowMask;
	[self setStyleMask:styleMask];
	[self setTitle:windowTitle];
}

@end
