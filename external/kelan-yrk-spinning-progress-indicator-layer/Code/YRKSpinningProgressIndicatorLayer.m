//
//  YRKSpinningProgressIndicatorLayer.m
//  SPILDemo
//
//  Copyright 2009 Kelan Champagne. All rights reserved.
//

#import "YRKSpinningProgressIndicatorLayer.h"


@interface YRKSpinningProgressIndicatorLayer ()

// Animation
- (void)advancePosition;

// Helper Methods
- (void)removeFinLayers;
- (void)createFinLayers;

- (CGRect)finBoundsForCurrentBounds;
- (CGPoint)finAnchorPointForCurrentBounds;

- (void)setupAnimTimer;
- (void)disposeAnimTimer;

@end


@implementation YRKSpinningProgressIndicatorLayer

@synthesize animationTimeScaleFactor;

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Init, Dealloc, etc
//------------------------------------------------------------------------------

- (id)init
{
    self = [super init];
    if (self) {
        _position = 0;
        _numFins = 12;
        _fadeDownOpacity = 0.0f;
        _isRunning = NO;
        self.color = [NSColor blackColor];
        [self setBounds:CGRectMake(0.0f, 0.0f, 10.0f, 10.0f)];
        [self createFinLayers];
		animationTimeScaleFactor = 1.0;
    }
    return self;
}

- (void)dealloc
{
    self.color = nil;
    [self removeFinLayers];

    [super dealloc];
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Overrides
//------------------------------------------------------------------------------

- (void)setBounds:(CGRect)newBounds
{
    [super setBounds:newBounds];

    // Resize the fins
    CGRect finBounds = [self finBoundsForCurrentBounds];
    CGPoint finAnchorPoint = [self finAnchorPointForCurrentBounds];
    CGPoint finPosition = CGPointMake([self bounds].size.width/2, [self bounds].size.height/2);
    CGFloat finCornerRadius = finBounds.size.width/2;

    // do the resizing all at once, immediately
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithBool:YES] forKey:kCATransactionDisableActions];
    for (CALayer *fin in _finLayers) {
        fin.bounds = finBounds;
        fin.anchorPoint = finAnchorPoint;
        fin.position = finPosition;
        fin.cornerRadius = finCornerRadius;
    }
    [CATransaction commit];
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Animation
//------------------------------------------------------------------------------

- (void)advancePosition
{
    _position++;
    if (_position >= _numFins) {
        _position = 0;
    }

    CALayer *fin = (CALayer *)[_finLayers objectAtIndex:_position];

    // Set the next fin to full opacity, but do it immediately, without any animation
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithBool:YES] forKey:kCATransactionDisableActions];
    fin.opacity = 1.0;
    [CATransaction commit];

    [CATransaction begin];
	[CATransaction setValue:[NSNumber numberWithFloat:0.5*animationTimeScaleFactor] forKey:kCATransactionAnimationDuration];    // Tell that fin to animate its opacity to transparent.
    fin.opacity = _fadeDownOpacity;
	[CATransaction commit];
	
    [self setNeedsDisplay];
}

- (void)setupAnimTimer
{
    // Just to be safe kill any existing timer.
    [self disposeAnimTimer];

    // Why animate if not visible?  viewDidMoveToWindow will re-call this method when needed.
    _animationTimer = [[NSTimer timerWithTimeInterval:0.05 * animationTimeScaleFactor
                                               target:self
                                             selector:@selector(advancePosition)
                                             userInfo:nil
                                              repeats:YES] retain];

    [_animationTimer setFireDate:[NSDate date]];
    [[NSRunLoop currentRunLoop] addTimer:_animationTimer forMode:NSRunLoopCommonModes];
    [[NSRunLoop currentRunLoop] addTimer:_animationTimer forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:_animationTimer forMode:NSEventTrackingRunLoopMode];
}

- (void)disposeAnimTimer
{
    [_animationTimer invalidate];
    [_animationTimer release];
    _animationTimer = nil;
}

- (void)startProgressAnimation
{
    self.hidden = NO;
    _isRunning = YES;
    _position = _numFins - 1;
    
    [self setNeedsDisplay];

    [self setupAnimTimer];
}

- (void)stopProgressAnimation
{
    _isRunning = NO;

    [self disposeAnimTimer];

    [self setNeedsDisplay];
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Properties and Accessors
//------------------------------------------------------------------------------

@synthesize isRunning = _isRunning;

// Can't use @synthesize because we need to convert NSColor <-> CGColor
- (NSColor *)color
{
    // Need to convert from CGColor to NSColor
    return  NSColorFromCGColorRef(_foreColor);
}
- (void)setColor:(NSColor *)newColor
{
    // Need to convert from NSColor to CGColor
    CGColorRef cgColor = CGColorCreateFromNSColor(newColor);

    if (nil != _foreColor) {
        CGColorRelease(_foreColor);
    }
    _foreColor = cgColor;

    // Update do all of the fins to this new color, at once, immediately
    [CATransaction begin];
    [CATransaction setValue:[NSNumber numberWithBool:YES] forKey:kCATransactionDisableActions];
    for (CALayer *fin in _finLayers) {
        fin.backgroundColor = cgColor;
    }
    [CATransaction commit];
}

- (void)toggleProgressAnimation
{
    if (_isRunning) {
        [self stopProgressAnimation];
    }
    else {
        [self startProgressAnimation];
    }
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Helper Methods
//------------------------------------------------------------------------------

- (void)createFinLayers
{
    [self removeFinLayers];
    [CATransaction begin];
	[CATransaction setValue:[NSNumber numberWithFloat:0.5*animationTimeScaleFactor] forKey:kCATransactionAnimationDuration];
	
    // Create new fin layers
    _finLayers = [[NSMutableArray alloc] initWithCapacity:_numFins];

    CGRect finBounds = [self finBoundsForCurrentBounds];
    CGPoint finAnchorPoint = [self finAnchorPointForCurrentBounds];
    CGPoint finPosition = CGPointMake([self bounds].size.width/2, [self bounds].size.height/2);
    CGFloat finCornerRadius = finBounds.size.width/2;

    for (NSUInteger i=0; i<_numFins; i++) {
        CALayer *newFin = [CALayer layer];

        newFin.bounds = finBounds;
        newFin.anchorPoint = finAnchorPoint;
        newFin.position = finPosition;
        newFin.transform = CATransform3DMakeRotation(i*(-6.282185/_numFins), 0.0, 0.0, 1.0);
        newFin.cornerRadius = finCornerRadius;
        newFin.backgroundColor = _foreColor;

        // Set the fin's initial opacity
        [CATransaction begin];
        [CATransaction setValue:[NSNumber numberWithBool:YES] forKey:kCATransactionDisableActions];
        newFin.opacity = _fadeDownOpacity;
        [CATransaction commit];

        // set the fin's fade-out time (for when it's animating)
        CABasicAnimation *anim = [CABasicAnimation animation];
        anim.duration = 0.7 * animationTimeScaleFactor;
        NSDictionary* actions = [NSDictionary dictionaryWithObjectsAndKeys:
                                 anim, @"opacity",
                                 nil];
        [newFin setActions:actions];

        [self addSublayer: newFin];
        [_finLayers addObject:newFin];
    }
	[CATransaction commit];
}

- (void)removeFinLayers
{
    for (CALayer *finLayer in _finLayers) {
        [finLayer removeFromSuperlayer];
    }
    [_finLayers release];
}

- (CGRect)finBoundsForCurrentBounds
{
    CGSize size = [self bounds].size;
    CGFloat minSide = size.width > size.height ? size.height : size.width;
    CGFloat width = minSide * 0.095f * 0.5;
    CGFloat height = minSide * 0.30f * 0.5;
    return CGRectMake(0,0,width,height);
}

- (CGPoint)finAnchorPointForCurrentBounds
{
    CGSize size = [self bounds].size;
    CGFloat minSide = size.width > size.height ? size.height : size.width;
    CGFloat height = minSide * 0.30f;
    return CGPointMake(0.5, -0.9*(minSide-height)/minSide);
}

@end


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Helper Functions
//------------------------------------------------------------------------------

/// Note: This returns a CGColorRef that the caller needs to release.
CGColorRef CGColorCreateFromNSColor(NSColor *nscolor)
{
// make this work with both 10.5 and 10.6 SDKs, based on a trick used
// by Cairo, and recommened to me by Eloy Duran (via email)
// http://lists.cairographics.org/archives/cairo-bugs/2009-December/003385.html
#ifdef CGFLOAT_DEFINED
#define yrkspil_float_t CGFloat
#else
#define yrkspil_float_t float
#endif
    yrkspil_float_t components[4];
    NSColor *deviceColor = [nscolor colorUsingColorSpaceName: NSDeviceRGBColorSpace];
    [deviceColor getRed: &components[0] green: &components[1] blue: &components[2] alpha: &components[3]];

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGColorRef cgcolor = CGColorCreate(colorSpace, components);
    CGColorSpaceRelease(colorSpace);

    return cgcolor;
#undef yrkspil_float_t
}


NSColor *NSColorFromCGColorRef(CGColorRef cgcolor)
{
    NSColorSpace *colorSpace = [NSColorSpace deviceRGBColorSpace];
    return [NSColor colorWithColorSpace:colorSpace
                             components:CGColorGetComponents(cgcolor)
                                  count:CGColorGetNumberOfComponents(cgcolor)];
}
