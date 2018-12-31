//
//  SPILDTopLayerView.m
//  SPILDemo
//
//  Copyright 2009 Kelan Champagne. All rights reserved.
//

/* Modified by Eric Wing with the following quick changes:
 - Removed Quartz Composer stuff
 - Made background black with a little transparency
 - Made spinner white with a little transparency
 - Fixed double release problem with _rootLayer which caused crash
 - Added mouse event handling to 'trap' events to make it seem modal
 */

#import "SPILDTopLayerView.h"

#import "YRKSpinningProgressIndicatorLayer.h"


@interface SPILDTopLayerView ()

- (void)setupLayers;
/*
- (void)usePlainBackground;
- (void)useQCBackground;
*/
@end


@implementation SPILDTopLayerView


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Init, Dealloc, etc
//------------------------------------------------------------------------------

- (id) initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	if(nil != self)
	{
		[self setupLayers];
	}
	return self;
}

- (void)dealloc
{
    [_progressIndicatorLayer removeFromSuperlayer];
    [_plainBackgroundLayer removeFromSuperlayer];
//    [_qcBackgroundLayer removeFromSuperlayer];

	[self setLayer:nil]; // should release the rootLayer
//    [_rootLayer release];

    [super dealloc];
}

- (void)awakeFromNib
{
    [self setupLayers];
}

- (void)setupLayers
{
    _rootLayer = [CALayer layer];
    [self setLayer:_rootLayer];
    [self setWantsLayer:YES];

    // Create the plain background layer
    _plainBackgroundLayer = [CALayer layer];
    _plainBackgroundLayer.name = @"plainBackgroundLayer";
    _plainBackgroundLayer.anchorPoint = CGPointMake(0.0, 0.0);
    _plainBackgroundLayer.position = CGPointMake(0, 0);
    _plainBackgroundLayer.bounds = [[self layer] bounds];
    _plainBackgroundLayer.autoresizingMask = (kCALayerWidthSizable|kCALayerHeightSizable);
    _plainBackgroundLayer.zPosition = 0;
    CGColorRef cgColor = CGColorCreateFromNSColor([NSColor blackColor]);
    _plainBackgroundLayer.backgroundColor = cgColor;
    _plainBackgroundLayer.opacity = 0.7;

    CGColorRelease(cgColor);
    [_rootLayer addSublayer:_plainBackgroundLayer];

    // Start with QC background
//    [self useQCBackground];

    // Put a SpinningProgressIndicatorLayer in front of everything
    _progressIndicatorLayer = [[YRKSpinningProgressIndicatorLayer alloc] init];
    _progressIndicatorLayer.name = @"progressIndicatorLayer";
    _progressIndicatorLayer.anchorPoint = CGPointMake(0.0, 0.0);
    _progressIndicatorLayer.position = CGPointMake(0, 0);
    _progressIndicatorLayer.bounds = [[self layer] bounds];
    _progressIndicatorLayer.autoresizingMask = (kCALayerWidthSizable|kCALayerHeightSizable);
    _progressIndicatorLayer.zPosition = 10; // make sure it goes in front of the background layer
    _progressIndicatorLayer.hidden = YES;
    _progressIndicatorLayer.color = [NSColor colorWithCalibratedRed:1.0 green:1.0 blue:1.0 alpha:0.8];

    [_rootLayer addSublayer:_progressIndicatorLayer];
}


// Need to handle mouse events to trap/block input

- (void) mouseDown:(NSEvent*)the_event
{
	//	NSLog(@"%@", NSStringFromSelector(_cmd));
}

- (void) mouseDragged:(NSEvent*)the_event
{
	//	NSLog(@"%@", NSStringFromSelector(_cmd));
}

- (void) mouseUp:(NSEvent*)the_event
{
	//	NSLog(@"%@", NSStringFromSelector(_cmd));
}

- (void)rightMouseDown:(NSEvent*)event
{
	//	NSLog(@"%@", NSStringFromSelector(_cmd));
}

- (void)rightMouseUp:(NSEvent*)event
{
	//	NSLog(@"%@", NSStringFromSelector(_cmd));
}

- (void)rightMouseDragged:(NSEvent*)event
{
	//	NSLog(@"%@", NSStringFromSelector(_cmd));
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Toggling Background
//------------------------------------------------------------------------------
/*
- (IBAction)toggleBackground:(id)sender
{
    if (nil == _qcBackgroundLayer) {
        [self useQCBackground];
    }
    else {
        [self usePlainBackground];
    }
}

- (void)usePlainBackground
{
    // Hide the QC background and show the plain one
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithBool:YES] forKey:kCATransactionDisableActions];
    _qcBackgroundLayer.hidden = YES;
    _plainBackgroundLayer.hidden = NO;
    [CATransaction commit];

    // destroy the QC background completely, so we can test the CPU usage of just the progress indicator itself
    [_qcBackgroundLayer removeFromSuperlayer];
    _qcBackgroundLayer = nil;
}

- (void)useQCBackground
{
    // Create the QC background layer
    _qcBackgroundLayer = [QCCompositionLayer compositionLayerWithFile:
               [[NSBundle mainBundle] pathForResource:@"Background" ofType:@"qtz"]];
    _qcBackgroundLayer.name = @"qcBackgroundLayer";
    _qcBackgroundLayer.anchorPoint = CGPointMake(0.0, 0.0);
    _qcBackgroundLayer.position = CGPointMake(0, 0);
    _qcBackgroundLayer.bounds = [[self layer] bounds];
    _qcBackgroundLayer.autoresizingMask = (kCALayerWidthSizable|kCALayerHeightSizable);
    _qcBackgroundLayer.zPosition = 0;
    [_rootLayer addSublayer:_qcBackgroundLayer];

    // Hide the plain background and show the QC one
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithBool:YES] forKey:kCATransactionDisableActions];
    _qcBackgroundLayer.hidden = NO;
    _plainBackgroundLayer.hidden = YES;
    [CATransaction commit];
}

- (void)setPlainBackgroundColor:(NSColor *)newColor
{
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithBool:YES] forKey:kCATransactionDisableActions];
    CGColorRef cgColor = CGColorCreateFromNSColor(newColor);
    _plainBackgroundLayer.backgroundColor = cgColor;
    CGColorRelease(cgColor);
    [CATransaction commit];
}
*/

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Properties
//------------------------------------------------------------------------------
@synthesize rootLayer = _rootLayer;
@synthesize progressIndicatorLayer = _progressIndicatorLayer;

@end
