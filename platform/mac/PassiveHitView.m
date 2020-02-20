//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "PassiveHitView.h"

@interface MouseOverHighlightButtonCell : NSButtonCell
{
@private
    BOOL mouseOver;
    BOOL mouseDown;
	BOOL mouseOverDisabled;
}
@property(nonatomic, assign) BOOL mouseOver;
@property(nonatomic, assign) BOOL mouseDown;
@property(nonatomic, assign) BOOL mouseOverDisabled;

@end


@implementation MouseOverHighlightButtonCell

@synthesize mouseOver;
@synthesize mouseDown;
@synthesize mouseOverDisabled;

#define PATH(FRAME) [NSBezierPath bezierPathWithRoundedRect:FRAME xRadius:7.0 yRadius:7.0]


- (void) drawWithFrame:(NSRect)cell_frame inView:(NSView*)control_view
{
//    NSLog(@"%@", NSStringFromSelector(_cmd));
	
	if( NO == mouseOverDisabled)
	{
		if( YES == mouseOver )
		{
			if (mouseDown)
			{
				// Draw mouseDown button fill effect
				[NSGraphicsContext saveGraphicsState];
				
				[PATH(NSInsetRect(cell_frame, 0.0, 0.0)) setClip];
				[[NSColor colorWithCalibratedRed:191.0/255.0 green:191.0/255.0 blue:191.0/255.0 alpha:1.0] set];
				NSRectFill(cell_frame);
				
				[NSGraphicsContext restoreGraphicsState];
			}
			else
			{	
				// draw mouseOver button fill effect
				[NSGraphicsContext saveGraphicsState];
				[PATH(NSInsetRect(cell_frame, 0.0, 0.0)) setClip];
				
				[[NSColor colorWithCalibratedRed:217.0/255.0 green:217.0/255.0 blue:217.0/255.0 alpha:1.0] set];
				
				NSRect highlight_rect_bottom = NSMakeRect(cell_frame.origin.x, cell_frame.origin.y, cell_frame.size.width, cell_frame.size.height);
				NSRectFill(highlight_rect_bottom);
				[NSGraphicsContext restoreGraphicsState];
				

				// draw button outline
				[NSGraphicsContext saveGraphicsState];
				NSRect focus_ring_frame = NSInsetRect(cell_frame, 0.0, 0.0);
				[[NSColor colorWithCalibratedRed:184.0/255.0 green:184.0/255.0 blue:184.0/255.0 alpha:1.0] set];
				
				[PATH(focus_ring_frame) stroke];
				[NSGraphicsContext restoreGraphicsState];
				
			}
		}
	}
	
	// Draw a focus ring
	if ([self showsFirstResponder])
	{
        // This implementation is fragile, but trying to get an image mask from the actual bitmaps used to compose the button highlight was just way too complicated...
        [NSGraphicsContext saveGraphicsState];
        NSSetFocusRingStyle(NSFocusRingOnly);
        NSRect focus_ring_frame = NSInsetRect(cell_frame, 1.0, 0.0);
        [PATH(focus_ring_frame) fill];
        [NSGraphicsContext restoreGraphicsState];
    }
    
}

- (BOOL) startTrackingAt:(NSPoint)startPoint inView:(NSView *)control_view
{
	//NSLog(@"%@", NSStringFromSelector(_cmd));
	
    mouseDown = YES;
    [control_view setNeedsDisplay:YES];
    return YES;
}

- (BOOL) continueTracking:(NSPoint)last_point at:(NSPoint)current_point inView:(NSView *)control_view
{
	//NSLog(@"%@", NSStringFromSelector(_cmd));
	
    return YES;
}

- (void) stopTracking:(NSPoint)last_point at:(NSPoint)stop_point inView:(NSView *)control_view mouseIsUp:(BOOL)mouse_up_flag
{
	//NSLog(@"%@", NSStringFromSelector(_cmd));
	
    mouseDown = NO;
    [control_view setNeedsDisplay:YES];
}

@end


@implementation PassiveHitView


- (void) setMouseOverDisabled:(BOOL)set_disabled
{
	mouseOverHighlightButtonCell.mouseOverDisabled = set_disabled;
}

- (BOOL) mouseOverDisabled
{
	return mouseOverHighlightButtonCell.mouseOverDisabled;
}


- (void) updateTrackingAreas
{
//	NSLog(@"%@", NSStringFromSelector(_cmd));
	
    [self removeTrackingArea:trackingArea];
    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:(NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways) owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
}

- (void) myHighlightButtonCommonInit
{
    [self setBordered:NO];
    
    mouseOverHighlightButtonCell = [[MouseOverHighlightButtonCell alloc] init];
	
    [self setCell:mouseOverHighlightButtonCell];
    
    [self updateTrackingAreas];
}

- (id) initWithFrame:(NSRect)the_frame
{
//	NSLog(@"%@", NSStringFromSelector(_cmd));
	
    if (nil != (self = [super initWithFrame:the_frame]))
	{
		[self myHighlightButtonCommonInit];
	}
    return self;
}

- (id) initWithCoder:(NSCoder*)the_decoder
{
	//NSLog(@"%@", NSStringFromSelector(_cmd));
	
    if (nil != (self = [super initWithCoder:the_decoder]))
	{
		[self myHighlightButtonCommonInit];
	}
    return self;
}

- (void) dealloc
{
	[mouseOverHighlightButtonCell release];
	[super dealloc];
}

- (BOOL) isFlipped {
    
    return NO;
}

- (void) mouseEntered:(NSEvent*)the_event
{
	//NSLog(@"%@", NSStringFromSelector(_cmd));
    ((MouseOverHighlightButtonCell *)[self cell]).mouseOver = YES;
    [self setNeedsDisplay:YES];
}

- (void) mouseExited:(NSEvent*)the_event
{
	
    //NSLog(@"%@", NSStringFromSelector(_cmd));
    ((MouseOverHighlightButtonCell *)[self cell]).mouseOver = NO;
    [self setNeedsDisplay:YES];
}

/* // enable to debug accessibility without actually enabling accessibility
- (BOOL)canBecomeKeyView
{
	return YES;
}
*/

@end
