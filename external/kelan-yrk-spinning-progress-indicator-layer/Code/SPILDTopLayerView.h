//
//  SPILDTopLayerView.h
//  SPILDemo
//
//  Copyright 2009 Kelan Champagne. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CoreAnimation.h>
#import <Quartz/Quartz.h>
#import "YRKSpinningProgressIndicatorLayer.h"

// the MenuView class is the view subclass that is inserted into
// the window.  It hosts the rootLayer, and responds to events
@interface SPILDTopLayerView : NSView {
    CALayer *_rootLayer;
    CALayer *_plainBackgroundLayer;
    CALayer *_qcBackgroundLayer;
    YRKSpinningProgressIndicatorLayer *_progressIndicatorLayer;
}

// IB Actions
//- (IBAction)toggleBackground:(id)sender;
//- (void)setPlainBackgroundColor:(NSColor *)newColor;

// Properties
@property (readonly) CALayer *rootLayer;
@property (readonly) YRKSpinningProgressIndicatorLayer *progressIndicatorLayer;

@end

