//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "GLView.h"
#include <OpenGL/gl.h>

#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>

#ifdef Rtt_DEBUG
#define NSDEBUG(...) // NSLog(__VA_ARGS__)
#else
#define NSDEBUG(...)
#endif

#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
	// From <Foundation/NSRunLoop.h>
	@interface NSObject (NSDelayedPerforming)

	- (void)performSelector:(SEL)aSelector withObject:(id)anArgument afterDelay:(NSTimeInterval)delay inModes:(NSArray *)modes;
	- (void)performSelector:(SEL)aSelector withObject:(id)anArgument afterDelay:(NSTimeInterval)delay;
	+ (void)cancelPreviousPerformRequestsWithTarget:(id)aTarget selector:(SEL)aSelector object:(id)anArgument;
	#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
	+ (void)cancelPreviousPerformRequestsWithTarget:(id)aTarget;
	#endif

	@end
#endif

#include "Rtt_AppleBitmap.h"
#include "Rtt_Event.h"

#include "Rtt_Runtime.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_Display.h"
#include "Rtt_MacDisplayObject.h"
#include "Rtt_MacWebPopup.h"
#include "Rtt_MacVideoObject.h"
#include "Rtt_MacVideoPlayer.h"
#import "SPILDTopLayerView.h"

#include "Display/Rtt_Display.h"
#include "Renderer/Rtt_Renderer.h"
#include "Rtt_RenderingStream.h"

#include "Rtt_AppleKeyServices.h"

// So we can build with Xcode 8.0
#ifndef NSAppKitVersionNumber10_12
#define NSAppKitVersionNumber10_12 1504
#endif

// Container for cursor rects
@interface CursorRect : NSObject
{
}
@property (nonatomic, readwrite) NSRect rect;
@property (nonatomic, readwrite, retain) NSCursor *cursor;

- (id) initWithRect:(NSRect) rect cursor:(NSCursor *) cursor;

@end

@implementation CursorRect

- (id) initWithRect:(NSRect) aRect cursor:(NSCursor *) aCursor;
{
	self = [super init];
    
	if ( self )
	{
        _rect = aRect;
        _cursor = aCursor;
	}
    
	return self;
}

@end


@interface GLView ()
- (void)dispatchEvent:(Rtt::MEvent*)event;
@end


@interface TapEventWrapper : NSObject
{
	Rtt::TapEvent *event;
}

- (id)initWithTapEvent:(Rtt::TapEvent*)inEvent;

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
@property (nonatomic, readonly) Rtt::TapEvent *event;
#else
- (Rtt::TapEvent*)event;
#endif

@end


@implementation TapEventWrapper

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
@synthesize event;
#else
- (Rtt::TapEvent*)event
{
	return event;
}
#endif

- (id)initWithTapEvent:(Rtt::TapEvent*)inEvent
{
	self = [super init];
	if ( self )
	{
		event = inEvent;
	}

	return self;
}

- (void)dealloc
{
	delete event;

	[super dealloc];
}

@end



@implementation GLView

@synthesize fRuntime;
@synthesize fTapDelay;
@synthesize zoomLevel;
@synthesize scaleFactor;
@synthesize isReady;
@synthesize swapped;
@synthesize isSimulatorView;
@synthesize sendAllMouseEvents;
@synthesize inFullScreenTransition;
@synthesize isResizable;
@synthesize allowOverlay;
@synthesize cursorHidden;
@synthesize initialLocation;

// pixel format definition
+ (NSOpenGLPixelFormat*) basicPixelFormat
{

//#if 0


    NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFANoRecovery,
		NSOpenGLPFADoubleBuffer,	// double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
		NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
		NSOpenGLPFASamples,(NSOpenGLPixelFormatAttribute)4,
		
        (NSOpenGLPixelFormatAttribute)0
    };
    return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];

/*
#else

	CGLPixelFormatAttribute attributes[] =
	{
		kCGLPFADoubleBuffer,
		kCGLPFADepthSize, (CGLPixelFormatAttribute)16,
		kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
		kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
		kCGLPFAAccelerated,
		kCGLPFANoRecovery,

		// Need to explicitly request OpenGL 3.2 Core (otherwise, we use 2.1 legacy)
	//	kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
		// kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy,
		(CGLPixelFormatAttribute)0
	};
    return [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:attributes];

NSOpenGLPixelFormatAttribute attributes1 [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,	// double buffered
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16, // 16 bit depth buffer
		NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24,
		NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute)8,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		
		//NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
		//NSOpenGLPFASamples,(NSOpenGLPixelFormatAttribute)2,
		
        (NSOpenGLPixelFormatAttribute)nil
    };
    return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes1] autorelease];
	
//#endif
*/


}

- (void) setRuntime:(Rtt::Runtime *)runtime
{
	fRuntime = runtime;
}
- (void)initCommon
{
	fOrientation = Rtt::DeviceOrientation::kUpright;
	fFirstClickTime = 0.;
	fTapDelay = 0.;
}

- (id)initWithFrame:(NSRect)frameRect
{
    NSDEBUG(@"GLView: initWithFrame: %@", NSStringFromRect(frameRect));
	NSOpenGLPixelFormat * pf = [GLView basicPixelFormat];

	self = [super initWithFrame: frameRect pixelFormat: pf];
	
	if ( self )
	{
		isReady = NO;

		fRuntime = NULL;
		fDelegate = nil;
		[self initCommon];
        fCursorRects = [[NSMutableArray alloc] initWithCapacity:18];

		sendAllMouseEvents = YES;
        inFullScreenTransition = NO;
        allowOverlay = YES; // this can be set to NO externally to disallow graphically showing the
                            // suspended state (e.g. when the Shift key is down)
		
		nativeFrameRect = frameRect;
        swapped = NO;
        isSimulatorView = NO;

		// It seems we need to set wantsLayer on macOS 10.12 or native display objects don't appear
		// (we avoid it on earlier versions because it has performance issues with OpenGL views)
		if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_12)
		{
			[self setWantsLayer:YES];
		}

		// This needs to be true or else we need to swap the width and height in nativeFrameRect
		// (see [self setOrientation:])
		Rtt_ASSERT(fOrientation == Rtt::DeviceOrientation::kUpright);

		zoomLevel = 1.0;
		scaleFactor = 1.0;

		// We're looking for a 10.9 API call to determine if we need to invalidate
		shouldInvalidate = [[NSApplication sharedApplication] respondsToSelector:@selector(occlusionState)];

		cursorHidden = NO;
		numCursorHides = 0;
#if Rtt_AUTHORING_SIMULATOR
		lastTouchPressure = Rtt::TouchEvent::kPressureInvalid;
#endif
	}
	
	return self;
}

- (void)dealloc
{
	[self setLayer:nil];
	
	fRuntime = NULL; // Don't delete. We do not own this pointer
    
    [fCursorRects release];

	[super dealloc];
}
- (void) reshape
{	
	[super reshape];
	
}
- (void) prepareOpenGL
{
    NSDEBUG(@"XXX: GLView: prepareOpenGL: fRuntime %p, self.isReady %s", fRuntime, (self.isReady ? "YES" : "NO"));
	//[super prepareOpenGL];

	[[self openGLContext] makeCurrentContext];

	Rtt::Display *display = NULL;

	if ( fRuntime != NULL && fRuntime->IsProperty(Rtt::Runtime::kIsApplicationLoaded) )
	{
		display = static_cast<Rtt::Display*>(&fRuntime->GetDisplay());
		// NSDEBUG(@"Deciding to call display->GetRenderer().ReleaseGPUResources(): display %p, GetRenderer() %p", display, (&display->GetRenderer()));
        if ( display != NULL && (&display->GetRenderer()) != NULL )
		{
			// NSDEBUG(@"Calling display->GetRenderer().ReleaseGPUResources(): display %p, GetRenderer() %p", display, (&display->GetRenderer()));
			// FIXME: this crashes in release builds because (&display->GetRenderer()) is NULL but the test above succeeds
			display->GetRenderer().ReleaseGPUResources();
		}
	}

	if (self.isReady == NO)
	{
		glClearColor( 0.0, 0.0, 0.0, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//        [[self openGLContext] flushBuffer];
		
		self.isReady = YES;
		
		[fDelegate didPrepareOpenGLContext:self];
	}

	if ( fRuntime != NULL && fRuntime->IsProperty(Rtt::Runtime::kIsApplicationLoaded) )
	{
		if ( display )
		{
			fRuntime->SetContentOrientation( fOrientation );
			display->GetRenderer().Initialize();
			[self invalidate];
		}
	}
}

- (void)drawRect:(NSRect)rect
{
    if ([self inLiveResize] || self.inFullScreenTransition)
    {
        // This fixes nasty OpenGL painting artifacts when live resizing
		[self invalidate];
    }
	else if (shouldInvalidate)
	{
		// This turns out to be lightweight b/c setNeedsDisplay is called by the timer *only*
		// when the Scene has already been invalidated. We invalidate here b/c drawRect
		// can also be called by the OS in situations like dragging between multiple monitors.
		[self invalidate];
	}

	[[self openGLContext] makeCurrentContext];

    
    
	// This should be called by the layer, not NSTimer!!!
	// That's b/c the OGL context is valid and ready for new OGL commands
	if ( isReady && fRuntime != NULL && fRuntime->IsProperty(Rtt::Runtime::kIsApplicationLoaded))
	{
		fRuntime->Render();
	}
    
    [[self openGLContext] flushBuffer];
}

- (void)setDelegate:(id< GLViewDelegate >)delegate
{
	fDelegate = delegate;
}

// This method will force the Corona OpenGL renderer to redraw everything.
- (void) invalidate
{
	if ( fRuntime )
	{
		fRuntime->GetDisplay().Invalidate();
	}

	[self update];
}

- (BOOL) isOpaque
{
	return YES;
}

// Provides a slight performance benefit
- (BOOL) wantsDefaultClipping
{
	return NO;
}

// Set origin at upper left corner
- (BOOL)isFlipped
{
	return YES;
}

- (void) dispatchMouseEvent:(Rtt::MouseEvent::MouseEventType) eventType event:(NSEvent *) event
{
	using namespace Rtt;

    NSPoint p = [self pointForEvent:event];

    NSUInteger modifierFlags = [event modifierFlags];
    NSUInteger mouseButtons = [NSEvent pressedMouseButtons];
    const NSUInteger kLeftMask = 1 << 0;
    const NSUInteger kRightMask = 1 << 1;
    const NSUInteger kMiddleMask = 1 << 2;

	int clickCount = 1;
	if(event.type != NSEventTypePressure)
	{
		clickCount = (int)event.clickCount;
	}

    // Create the Corona mouse event
    MouseEvent mouseEvent(eventType,
                          p.x, p.y,
                          0, 0, clickCount,
                          (mouseButtons & kLeftMask),
                          (mouseButtons & kRightMask),
                          (mouseButtons & kMiddleMask),
                          (modifierFlags & NSShiftKeyMask),
                          (modifierFlags & NSAlternateKeyMask),
                          (modifierFlags & NSControlKeyMask),
                          (modifierFlags & NSCommandKeyMask) );

    [self dispatchEvent: ( & mouseEvent )];
}

- (void)dispatchEvent:(Rtt::MEvent*)e
{
	using namespace Rtt;

	// Since we defer the loading of the application in [self prepareOpenGL] we shouldn't dispatch
	// any events until it really is loaded or when the runtime is suspended
	if ( fRuntime == NULL || ! fRuntime->IsProperty(Rtt::Runtime::kIsApplicationLoaded) ||
		 fRuntime->IsSuspended())
	{
		return;
	}

	Runtime* runtime = self.runtime;
	Rtt_ASSERT( runtime );

	if ( Rtt_VERIFY( e ) )
	{
		runtime->DispatchEvent( * e );
	}
}

- (void)dispatchTapEvent:(TapEventWrapper*)e
{
	[self dispatchEvent:e.event];
}

- (Rtt::DeviceOrientation::Type)orientation
{
	return (Rtt::DeviceOrientation::Type)fOrientation;
}

- (void)setOrientation:(Rtt::DeviceOrientation::Type)newValue
{
#ifndef Rtt_AUTHORING_SIMULATOR
    if ( !swapped && (Rtt::DeviceOrientation::IsSideways( newValue ) && Rtt::DeviceOrientation::IsUpright( fOrientation )) )
	{
        Rtt::Swap<CGFloat>(nativeFrameRect.size.width, nativeFrameRect.size.height);
        swapped = YES;
	}
#endif
    
    fOrientation = newValue;
}

// TODO: This function needs to be kept in sync with PlatformSimulator::AdjustPoint(),
// and should eventually call it directly.
- (void)adjustPoint:(NSPoint*)p
{
	using namespace Rtt;

	p->x = roundf( p->x );
	p->y = roundf( p->y );

	//Rtt_TRACE( ( "before(%g,%g)", p->x, p->y ) );

	Display& display = fRuntime->GetDisplay();
	Rtt_ASSERT( display.GetSurfaceOrientation() == fOrientation );

	const DeviceOrientation::Type orientation = display.GetRelativeOrientation(); // fOrientation;
	if ( DeviceOrientation::kUpright != orientation )
	{
		NSRect viewRect = [self frame];
		NSSize viewSize = viewRect.size;

//Rtt_TRACE( ( "view w,h(%g,%g)\n", viewSize.width, viewSize.height ) );
//		viewSize.width -= 1;

		if ( DeviceOrientation::kUpsideDown == orientation )
		{
			p->x = viewSize.width - p->x;
			p->y = viewSize.height - p->y;
		}
		else
		{
//			viewSize.height -= 1;
			float x = p->x;
			if ( DeviceOrientation::kSidewaysRight == orientation )
			{
				p->x = p->y;
				p->y = viewSize.width - x;
			}
			else
			{
				Rtt_ASSERT( DeviceOrientation::kSidewaysLeft == orientation );
				p->x = viewSize.height - p->y;
				p->y = x;
			}
		}
	}

	// If there is a zoom applied, scale the point to the original unzoomed window
	// coordinates b/c Corona expects the original coordinates
    p->x = p->x / zoomLevel;
    p->y = p->y / zoomLevel;

	//Rtt_TRACE( ( " after(%g,%g)\n", p->x, p->y ) );
}

- (NSPoint)pointForEvent:(NSEvent*)event
{
	NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];

	[self adjustPoint:&p];
	return p;
}

static const float kTapTolerance = 1.;
static U32 *sTouchId = (U32*)(& kTapTolerance); // any arbitrary pointer value will do

- (void)rightMouseDown:(NSEvent*)event
{
    using namespace Rtt;
    
    NSDEBUG( @"rightMouseDown: %@", event );
    
	[self dispatchMouseEvent:MouseEvent::kDown event:event];
}

- (void)rightMouseUp:(NSEvent*)event
{
    using namespace Rtt;
    
    NSDEBUG( @"rightMouseUp: %@", event );
    
	[self dispatchMouseEvent:MouseEvent::kUp event:event];
}

- (void)rightMouseDragged:(NSEvent*)event
{
    using namespace Rtt;
    
    // NSDEBUG( @"rightMouseDragged: %@", event );

	[self dispatchMouseEvent:MouseEvent::kDrag event:event];
}

#if Rtt_AUTHORING_SIMULATOR
-(void)pressureChangeWithEvent:(NSEvent *)event
{
	const float maxPressure = 20.f/3.f; // iOS Seems to have pressure values between 0 and 6.66(6)
	float oldPressure = lastTouchPressure;
	lastTouchPressure = event.pressure*maxPressure;
	if(event.stage >= 2)
	{ // When "Force Touch" activates, just kick to highest gear, or else pressure meter resets
		lastTouchPressure = maxPressure;
	}

	if(CGPointEqualToPoint([event locationInWindow], lastPressurePoint)
	   && oldPressure != Rtt::TouchEvent::kPressureInvalid
	   && oldPressure != lastTouchPressure
	   && lastTouchPressure > 0)
	{ // this is for when pressure applied without moving a cursor
		[self mouseDragged:event];
	}
}
#endif

- (void)mouseDown:(NSEvent*)event
{
    using namespace Rtt;
    
	// NSDEBUG( @"mouseDown: %@", event );

	if ([self startWindowMovement])
	{
		return;
	}

	NSPoint p = [self pointForEvent:event];
    
	// Send mouse event before the touch
	[self dispatchMouseEvent:MouseEvent::kDown event:event];
    
	fStartPosition = p;
	float tClick = fRuntime->GetElapsedMS();
	if ( tClick - fFirstClickTime > 250 )
	{
		fFirstClickTime = tClick;
		fNumTaps = 1;
	}
	else if ( fabs( fStartPosition.x - p.x ) <= kTapTolerance
		 && fabs( fStartPosition.y - p.y ) <= kTapTolerance )
	{
		fNumTaps++;
	}

#if Rtt_AUTHORING_SIMULATOR
	float pressure = lastTouchPressure;
	lastPressurePoint = [event locationInWindow];
#else
	const float pressure = TouchEvent::kPressureInvalid;
#endif

	TouchEvent t( p.x, p.y, p.x, p.y, TouchEvent::kBegan, pressure );
	t.SetId( sTouchId );
	if ( fRuntime->Platform().GetDevice().DoesNotify( MPlatformDevice::kMultitouchEvent ) )
	{
		MultitouchEvent t2( &t, 1 );
		[self dispatchEvent: (&t2)];
	}
	else
	{
		[self dispatchEvent: (&t)];
	}

	if ( fTapDelay > 0. && fNumTaps > 1 )
	{
		[NSObject cancelPreviousPerformRequestsWithTarget:self];
	}
}

- (void)mouseDragged:(NSEvent*)event
{
	using namespace Rtt;

	// NSDEBUG( @"mouseDragged: %@", event );

	if ([self handleWindowMovement])
	{
		return;
	}

	NSPoint p = [self pointForEvent:event];

	// Send mouse event before the touch
	[self dispatchMouseEvent:MouseEvent::kDrag event:event];

#if Rtt_AUTHORING_SIMULATOR
	float pressure = lastTouchPressure;
	lastPressurePoint = [event locationInWindow];
#else
	const float pressure = TouchEvent::kPressureInvalid;
#endif

	TouchEvent t( p.x, p.y, fStartPosition.x, fStartPosition.y, TouchEvent::kMoved, pressure );
	t.SetId( sTouchId );
	if ( fRuntime->Platform().GetDevice().DoesNotify( MPlatformDevice::kMultitouchEvent ) )
	{
		MultitouchEvent t2( &t, 1 );
		[self dispatchEvent: (&t2)];
	}
	else
	{
		[self dispatchEvent: (&t)];
	}

	DragEvent e( fStartPosition.x, fStartPosition.y, p.x, p.y );
	[self dispatchEvent: (&e)];
}

- (void)mouseUp:(NSEvent*)event
{
	using namespace Rtt;

	if ([self endWindowMovement])
	{
		return;
	}

	// NSDEBUG( @"mouseUp: %@", event );

	// Send mouse event before the touch
	[self dispatchMouseEvent:MouseEvent::kUp event:event];
    
	NSPoint p = [self pointForEvent:event];

#if Rtt_AUTHORING_SIMULATOR
	float pressure = lastTouchPressure;
	if(lastTouchPressure != TouchEvent::kPressureInvalid)
	{
		lastTouchPressure = TouchEvent::kPressureInvalid;
		pressure = 0.00001f; //some small value to mimic iOS
	}
#else
	const float pressure = TouchEvent::kPressureInvalid;
#endif

	TouchEvent t( p.x, p.y, fStartPosition.x, fStartPosition.y, TouchEvent::kEnded, pressure );
	t.SetId( sTouchId++ );
	if ( fRuntime->Platform().GetDevice().DoesNotify( MPlatformDevice::kMultitouchEvent ) )
	{
		MultitouchEvent t2( &t, 1 );
		[self dispatchEvent: (&t2)];
	}
	else
	{
		[self dispatchEvent: (&t)];
	}

	if ( fNumTaps > 0 )
	{
		if ( fabs( fStartPosition.x - p.x ) <= kTapTolerance
			 && fabs( fStartPosition.y - p.y ) <= kTapTolerance )
		{
			TapEvent* tapEvent = new TapEvent( p.x, p.y, fNumTaps );
			TapEventWrapper* e = [[TapEventWrapper alloc] initWithTapEvent:tapEvent];

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
		}
	}

//	NSDEBUG( @"mouseUp(%g,%g)", p.x, p.y );
}

// Another kludge. Because the activity indicator is not a display object and because it is a single, modal instance,
// and because it is in a C++ wrapper that has no built-in notion of redisplay or callbacks,
// it is easier to use a performWithDelay here to force an update.
- (void) recomputeSizeFromRotationOrScaleForActivityIndicator:(NSView*)view
{
	// Animate because the glview is also animating. 
	// Unfortunately, the animations may not be in sync, but there is at least a chance they will be in sync,
	// whereas they will not be in sync at all if we don't animate.
	[[view animator] setFrameSize:[self frame].size];
}

- (void) setZoomLevel:(CGFloat) newZoomLevel
{
    isSimulatorView = YES;
	zoomLevel = newZoomLevel;

	[self resizeNativeDisplayObjects];
}

- (void) resizeNativeDisplayObjects
{
    // All the native display objects (which are NSView subviews) need to be resized to fit.
	NSArray* subviews = [self subviews];

	for( NSView* displayview in subviews )
	{
		if ( [displayview isKindOfClass:[MacWebView class]] )
		{
			Rtt::MacWebPopup* displayobject = (Rtt::MacWebPopup*)[(MacWebView *)displayview owner];
			displayobject->SetNeedsRecomputeFromRotationOrScale();				
		}
		else if ( [displayview isKindOfClass:[Rtt_VideoPlayerView class]] )
		{
			Rtt::MacVideoPlayer* displayobject = (Rtt::MacVideoPlayer*)[(Rtt_VideoPlayerView *)displayview owner];
			displayobject->SetNeedsRecomputeFromRotationOrScale();
		}
		else if ( [displayview isKindOfClass:[SPILDTopLayerView class]] )
		{
			// Another kludge. Because the activity indicator is not a display object and because it is a single, modal instance,
			// and because it is in a C++ wrapper that has no built-in notion of redisplay or callbacks,
			// it is easier to use a performWithDelay here to force an update.
			[self performSelector:@selector(recomputeSizeFromRotationOrScaleForActivityIndicator:) withObject:displayview afterDelay:0.0];
		}
	}
}

- (NSRect) frame
{
    return [super frame];
}

- (void) setFrameSize:(NSSize)new_size
{
	BOOL sizeChanged = ! NSEqualSizes(new_size, nativeFrameRect.size);

	NSDEBUG(@"GLView:setFrameSize: newSize %@, frame %@ (isResizable %s, sizeChanged %s, isReady %s, fRuntime %s)",
		  NSStringFromSize(new_size), NSStringFromRect([self frame]),
		  (self.isResizable ? "YES" : "NO"),
		  (sizeChanged ? "YES" : "NO"),
		  (self.isReady ? "YES" : "NO"),
		  (fRuntime != NULL ? "YES" : "NO") );
	//NSDEBUG(@"GLView:setFrameSize: old %@, new %@", NSStringFromSize([self frame].size), NSStringFromSize(new_size));

	if (! isSimulatorView ) // if (sizeChanged)
	{
		nativeFrameRect.size = new_size;
        swapped = NO;

		if ( ! swapped && Rtt::DeviceOrientation::IsSideways( fOrientation ) )
		{
            Rtt::Swap<CGFloat>(nativeFrameRect.size.width, nativeFrameRect.size.height);
            swapped = YES;
		}
	}

	[super setFrameSize:new_size];

	[self resizeNativeDisplayObjects];

	// Update rectangle used for mouseMoved: events
	if(trackingRectTag) {
		[self removeTrackingRect:trackingRectTag];
	}
	trackingRectTag = [self addTrackingRect:[self bounds] owner:self userData:nil assumeInside:NO];

	if (! isSimulatorView && sizeChanged && self.isReady && self.runtime != NULL )
	{
		Rtt::Display *display = NULL;
		display = static_cast<Rtt::Display*>(&fRuntime->GetDisplay());

		if ( display )
		{
//            if (display->GetScaleMode() == Rtt::Display::kAdaptive)
//            {
//                display->Restart(-1, -1);
//            }
//            else
            {
                //display->Restart(nativeFrameRect.size.width, nativeFrameRect.size.height);
                display->WindowSizeChanged();
                display->Restart(fOrientation);
            }
		}

		// The Simulator sends its "resize" event in PlatformSimulator::Rotate() but that
		// doesn't exist if this isn't a Simulator view so we need to do it here
		self.runtime->DispatchEvent( Rtt::ResizeEvent() );
	}

	// Prevents occasional GL buffer flashes
	[self invalidate];
}

- (CGFloat) viewportWidth
{
    if ( Rtt::DeviceOrientation::IsSideways( fOrientation ) )
	{
		// Rtt_TRACE(("viewportWidth: %g\n", nativeFrameRect.size.height * (zoomLevel * scaleFactor)));
        return nativeFrameRect.size.height * (zoomLevel * scaleFactor);
	}
	else
	{
		// Rtt_TRACE(("viewportWidth: %g\n", nativeFrameRect.size.width * (zoomLevel * scaleFactor)));
        return nativeFrameRect.size.width * (zoomLevel * scaleFactor);
	}
}

- (CGFloat) viewportHeight
{
    if ( Rtt::DeviceOrientation::IsSideways( fOrientation ) )
	{
		// Rtt_TRACE(("viewportHeight: %g\n", nativeFrameRect.size.width * (zoomLevel * scaleFactor)));
		return nativeFrameRect.size.width * (zoomLevel * scaleFactor);
	}
	else
	{
		// Rtt_TRACE(("viewportHeight: %g\n", nativeFrameRect.size.height * (zoomLevel * scaleFactor)));
		return nativeFrameRect.size.height * (zoomLevel * scaleFactor);
	}
}

- (CGFloat)deviceWidth
{
	// Rtt_TRACE(("deviceWidth: %g\n", nativeFrameRect.size.width ));
    return nativeFrameRect.size.width;
}

- (CGFloat)deviceHeight
{
	// Rtt_TRACE(("deviceHeight: %g\n", nativeFrameRect.size.height ));
    return nativeFrameRect.size.height;
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

- (void) suspendNativeDisplayObjects:(bool) showOverlay
{
#if Rtt_AUTHORING_SIMULATOR
	// TODO: Figure out what 'suspend' really means for native display objects.
	// Right now, we know we need to suspend video.
	NSArray* subviews = [self subviews];

	for( NSView* displayview in subviews )
	{
		if ( [displayview isKindOfClass:[MacWebView class]] )
		{
		}
		else if ( [displayview isKindOfClass:[Rtt_VideoObjectView class]] )
		{
			[(Rtt_VideoObjectView *)displayview setSuspended:YES];
		}
		/*
		else if ( [displayview isKindOfClass:[Rtt_VideoPlayerView class]] )
		{
			[(Rtt_VideoPlayerView *)displayview setSuspended:YES];
		}
		 */
		else if ( [displayview isKindOfClass:[SPILDTopLayerView class]] )
		{
			// Another kludge. Because the activity indicator is not a display object and because it is a single, modal instance,
			// and because it is in a C++ wrapper that has no built-in notion of redisplay or callbacks,
			// it is easier to use a performWithDelay here to force an update.
		}
		else if( [displayview respondsToSelector:@selector(owner)] )
		{
		}
	}
    
    if (showOverlay && self.allowOverlay)
    {
        if ( nil == suspendedOverlay )
        {
            NSSize size = [self frame].size;
            suspendedOverlay = [[SPILDTopLayerView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
            [[suspendedOverlay progressIndicatorLayer] setAnimationTimeScaleFactor:8.0];
            [[suspendedOverlay progressIndicatorLayer] setColor:[NSColor orangeColor]];
            [self addSubview:suspendedOverlay];
			[[self window] makeFirstResponder:suspendedOverlay];
            [[suspendedOverlay progressIndicatorLayer] startProgressAnimation];
        }
    }
#endif // Rtt_AUTHORING_SIMULATOR
}

- (void) resumeNativeDisplayObjects
{
#if Rtt_AUTHORING_SIMULATOR
    if ( nil != suspendedOverlay )
    {
        [[suspendedOverlay progressIndicatorLayer] stopProgressAnimation];
        [suspendedOverlay removeFromSuperview];
        [suspendedOverlay release];
        suspendedOverlay = nil;
    }

    // TODO: Figure out what 'suspend' really means for native display objects.
	// Right now, we know we need to suspend video.
	NSArray* subviews = [self subviews];
	
	for( NSView* displayview in subviews )
	{
		if ( [displayview isKindOfClass:[MacWebView class]] )
		{
		}
		else if ( [displayview isKindOfClass:[Rtt_VideoObjectView class]] )
		{
			[(Rtt_VideoObjectView *)displayview setSuspended:NO];
		}
		/*
		else if ( [displayview isKindOfClass:[Rtt_VideoPlayerView class]] )
		{
			[(Rtt_VideoPlayerView *)displayview setSuspended:NO];
		}
		 */
		else if ( [displayview isKindOfClass:[SPILDTopLayerView class]] )
		{
			// Another kludge. Because the activity indicator is not a display object and because it is a single, modal instance,
			// and because it is in a C++ wrapper that has no built-in notion of redisplay or callbacks,
			// it is easier to use a performWithDelay here to force an update.
		}
		else if( [displayview respondsToSelector:@selector(owner)] )
		{
		}
	}
#endif // Rtt_AUTHORING_SIMULATOR
}

// keyDown and keyUp do not trigger modifier key events (shift, control, etc.)
- (void)flagsChanged:(NSEvent *)event
{
    unsigned long mask = [AppleKeyServices getModifierMaskForKey:[event keyCode]];

    // After certain actions, like using the screenshot tool, MacOS apparently triggers the "a" key event. Can't imagine anyone would like this event.
    if ( [event keyCode] != kVK_ANSI_A )
    {
        // The mask contains a few bits set. All must be set to consider the key down.
        if ( ( [event modifierFlags] & mask ) == mask)
        {
            [self dispatchKeyEvent:event withPhase:Rtt::KeyEvent::kDown];
        }
        else
        {
            [self dispatchKeyEvent:event withPhase:Rtt::KeyEvent::kUp];
        }
    }
}

- (void)keyDown:(NSEvent *)event
{
	[self dispatchKeyEvent:event withPhase:Rtt::KeyEvent::kDown];
	const char* characters = [[event characters] UTF8String];
	if (strlen(characters) > 1 || isprint(characters[0])) {
		Rtt::CharacterEvent e(NULL, characters);
		[self dispatchEvent: ( & e )];
	}
}

- (void)keyUp:(NSEvent *)event
{
	[self dispatchKeyEvent:event withPhase:Rtt::KeyEvent::kUp];
}

- (void)dispatchKeyEvent:(NSEvent *)event withPhase:(Rtt::KeyEvent::Phase)phase
{
	using namespace Rtt;
	
	NSUInteger modifierFlags = [event modifierFlags];
	unsigned short keyCode = [event keyCode];
	NSString *keyName = [AppleKeyServices getNameForKey:[NSNumber numberWithInt:keyCode]];
	
	KeyEvent e(
			   NULL,
			   phase,
			   [keyName UTF8String],
			   keyCode,
			   (modifierFlags & NSShiftKeyMask) || (modifierFlags & NSAlphaShiftKeyMask),
			   (modifierFlags & NSAlternateKeyMask),
			   (modifierFlags & NSControlKeyMask),
			   (modifierFlags & NSCommandKeyMask) );
	[self dispatchEvent: ( & e )];
}

- (void)viewDidMoveToWindow
{
	// We may have called addTrackingRect: in a setFrame: call before we get here
	if(trackingRectTag) {
		[self removeTrackingRect:trackingRectTag];
	}

	// Limit mouse events to the view's bounds
	NSRect r = [self bounds];
	trackingRectTag = [self addTrackingRect:r owner:self userData:nil assumeInside:NO];

	// This is needed for systems that _only_ have Retina screens and may not get all the
	// notifications multi-display systems do
	scaleFactor = [[self window] backingScaleFactor];
}

- (void)mouseMoved:(NSEvent *)event
{
	using namespace Rtt;

	if ( sendAllMouseEvents )
	{
        // NSDEBUG( @"mouseMoved: %@", event );
		NSPoint p = [self pointForEvent:event];
        NSUInteger modifierFlags = [event modifierFlags];

		// Raise the mouse event.
		// When button is down, mouseDragged: is called instead,
		// so pass false for button states
		MouseEvent e(MouseEvent::kMove, p.x, p.y, 0, 0, 0, false, false, false,
                     (modifierFlags & NSShiftKeyMask),
                     (modifierFlags & NSAlternateKeyMask),
                     (modifierFlags & NSControlKeyMask),
                     (modifierFlags & NSCommandKeyMask) );
		[self dispatchEvent: ( & e )];
	}
}

- (void)mouseEntered:(NSEvent *)event
{
	NSDEBUG(@"mouseEntered: cursorHidden %s (%d)", (cursorHidden ? "YES" : "NO"), numCursorHides);
	// Start dispatching mouseMoved: events
	// NSDEBUG( @"mouseEntered: %@", event );
	[[self window] setAcceptsMouseMovedEvents:YES];

	if (cursorHidden)
	{
		NSDEBUG(@"mouseEntered: actually hiding");
		[NSCursor hide];
		++numCursorHides;
	}
}

- (void)mouseExited:(NSEvent *)event
{
	NSDEBUG(@"mouseExited: cursorHidden %s (%d)", (cursorHidden ? "YES" : "NO"), numCursorHides);
	// Stop dispatching mouseMoved: events
	// NSDEBUG( @"mouseExited: %@", event );
	[[self window] setAcceptsMouseMovedEvents:NO];

	if (cursorHidden)
	{
		for (int i = 0; i < numCursorHides; i++)
		{
			[NSCursor unhide];
		}
		numCursorHides = 0;
		[NSCursor unhide];  // TODO: figure out why one more unhide is necessary for reliability
	}
}

- (void)scrollWheel:(NSEvent *)event
{
    using namespace Rtt;
    
	// NSDEBUG( @"scrollWheel: %@", event );
	NSPoint p = [self pointForEvent:event];
	NSUInteger modifierFlags = [event modifierFlags];

	// Raise the mouse event
	// The sign of the deltas is the opposite of what is expected so they are swapped
	MouseEvent e(MouseEvent::kScroll, p.x, p.y, -([event deltaX]), -([event deltaY]), 0, false, false, false,
				 (modifierFlags & NSShiftKeyMask),
				 (modifierFlags & NSAlternateKeyMask),
				 (modifierFlags & NSControlKeyMask),
				 (modifierFlags & NSCommandKeyMask) );

	[self dispatchEvent: ( & e )];
}

-(void)resetCursorRects
{
    // NSDEBUG(@"resetCursorRects: %@", fCursorRects);
    
    for (CursorRect *cr in fCursorRects)
    {
        [self addCursorRect:cr.rect cursor:cr.cursor];
    }
}

-(void) setCursor:(const char *) cursorName forRect:(NSRect) bounds
{
    // NSDEBUG(@"GLView:setCursor: %@", NSStringFromRect(bounds));

    NSCursor *cursor = [NSCursor currentSystemCursor];

    if (strcasecmp(cursorName, "arrow") == 0)
    {
        cursor = [NSCursor arrowCursor];
    }
    else if (strcasecmp(cursorName, "closedHand") == 0)
    {
        cursor = [NSCursor closedHandCursor];
    }
    else if (strcasecmp(cursorName, "openHand") == 0)
    {
        cursor = [NSCursor openHandCursor];
    }
	else if (strcasecmp(cursorName, "pointingHand") == 0)
    {
        cursor = [NSCursor pointingHandCursor];
    }
    else if (strcasecmp(cursorName, "crosshair") == 0)
    {
        cursor = [NSCursor crosshairCursor];
    }
    else if (strcasecmp(cursorName, "notAllowed") == 0)
    {
        cursor = [NSCursor operationNotAllowedCursor];
    }
	else if (strcasecmp(cursorName, "beam") == 0)
    {
        cursor = [NSCursor IBeamCursor];
    }
	else if (strcasecmp(cursorName, "resizeRight") == 0)
    {
        cursor = [NSCursor resizeRightCursor];
    }
	else if (strcasecmp(cursorName, "resizeLeft") == 0)
    {
        cursor = [NSCursor resizeLeftCursor];
    }
	else if (strcasecmp(cursorName, "resizeLeftRight") == 0)
    {
        cursor = [NSCursor resizeLeftRightCursor];
    }
	else if (strcasecmp(cursorName, "resizeUp") == 0)
    {
        cursor = [NSCursor resizeUpCursor];
    }
	else if (strcasecmp(cursorName, "resizeDown") == 0)
    {
        cursor = [NSCursor resizeDownCursor];
    }
	else if (strcasecmp(cursorName, "resizeUpDown") == 0)
    {
        cursor = [NSCursor resizeUpDownCursor];
    }
	else if (strcasecmp(cursorName, "disappearingItem") == 0)
    {
        cursor = [NSCursor disappearingItemCursor];
    }
	else if (strcasecmp(cursorName, "beamHorizontal") == 0)
    {
        cursor = [NSCursor IBeamCursorForVerticalLayout];
    }
	else if (strcasecmp(cursorName, "dragLink") == 0)
    {
        cursor = [NSCursor dragLinkCursor];
    }
	else if (strcasecmp(cursorName, "dragCopy") == 0)
    {
        cursor = [NSCursor dragCopyCursor];
    }
	else if (strcasecmp(cursorName, "contextMenu") == 0)
    {
        cursor = [NSCursor contextualMenuCursor];
    }
    else
    {
		// Remove any rect with these bounds
		int currIdx = 0;
		for (CursorRect *cr in fCursorRects)
		{
			if (NSEqualRects(cr.rect, bounds))
			{
				[fCursorRects removeObjectAtIndex:currIdx];
				[self.window invalidateCursorRectsForView:self];
				break;
			}
			++currIdx;
		}
		return;
	}

    [fCursorRects addObject:[[[CursorRect alloc] initWithRect:bounds cursor:cursor] autorelease]];
	[self.window invalidateCursorRectsForView:self];
}

- (void) hideCursor
{
	NSDEBUG(@"hideCursor: cursorHidden %s (%d)", (cursorHidden ? "YES" : "NO"), numCursorHides);

	if (! cursorHidden)
	{
		// We need to deal with whether the cursor is inside our window when
		// the API is called because we only want the cursor to not be visible
		// when inside the Corona window (its state when going in and out of
		// the window is handled by the mouseEntered/Exited handlers above)
		NSPoint screenPoint = [NSEvent mouseLocation];
		NSRect screenRect = NSMakeRect(screenPoint.x, screenPoint.y, 0, 0);
		NSRect baseRect = [self.window convertRectFromScreen:screenRect];
		NSPoint point = [self convertPoint:baseRect.origin fromView:nil];

		if ([self mouse:point inRect:[self bounds]])
		{
			NSDEBUG(@"hideCursor: actually hiding");

			[NSCursor hide];

			++numCursorHides;
		}

		cursorHidden = YES;
	}
}

- (void) showCursor
{
	NSDEBUG(@"showCursor: cursorHidden %s (%d)", (cursorHidden ? "YES" : "NO"), numCursorHides);

	// Various combinations of things (like whether the mouse cursor happens to be on
	// the area of screen where an app's window appears on startup) can make the matching
	// of NSCursor hides and unhides problematic so we track how many hides we do and
	// make sure to do that many unhides
	for (int i = 0; i < numCursorHides; i++)
	{
		[NSCursor unhide];
	}
	numCursorHides = 0;
	[NSCursor unhide];  // TODO: figure out why one more unhide is necessary for reliability

	cursorHidden = NO;
}

// Fix the view layering when the app is hidden or minaturized
// Fixes bug http://bugs.coronalabs.com/default.asp?44953
- (void) restoreWindowProperties
{
	NSArray* subviews = [self subviews];

	for (NSView* displayview in subviews)
	{
		if ([displayview respondsToSelector:@selector(setWantsLayer:)])
		{
			// Toggle wantsLayer off and on again
			[displayview setWantsLayer:NO];
			[displayview setWantsLayer:YES];
		}
	}
}

//
// These window movement methods handle dragging in the titlebar area of windows
// that don't have a titlebar
//
- (BOOL) startWindowMovement
{
	NSRect  windowFrame = [[self window] frame];

	initialLocation = [NSEvent mouseLocation];

	if (initialLocation.y < ((windowFrame.origin.y + windowFrame.size.height) - 22))
	{
		initialLocation.x = -1;
		initialLocation.y = -1;

		return NO;
	}

	initialLocation.x -= windowFrame.origin.x;
	initialLocation.y -= windowFrame.origin.y;

	return YES;
}

- (BOOL) handleWindowMovement
{
	if (initialLocation.x == -1)
	{
		return NO;
	}

	NSPoint currentLocation;
	NSPoint newOrigin;

	currentLocation = [NSEvent mouseLocation];
	newOrigin.x = currentLocation.x - initialLocation.x;
	newOrigin.y = currentLocation.y - initialLocation.y;

	[[self window] setFrameOrigin:newOrigin];

	return YES;
}

- (BOOL) endWindowMovement
{
	if (initialLocation.x == -1)
	{
		return NO;
	}

	initialLocation.x = -1;
	initialLocation.y = -1;

	return YES;
}

@end
