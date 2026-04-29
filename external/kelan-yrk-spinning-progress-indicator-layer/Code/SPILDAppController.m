//
//  SPILDAppController.m
//  SPILDemo
//
//  Copyright 2009 Kelan Champagne. All rights reserved.
//

#import "SPILDAppController.h"

#import "SPILDTopLayerView.h"
#import "YRKSpinningProgressIndicatorLayer.h"

@implementation SPILDAppController

//------------------------------------------------------------------------------
#pragma mark -
#pragma mark Init, Dealloc, etc
//------------------------------------------------------------------------------

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // start with a nice green
    NSColor *niceGreenColor = [NSColor colorWithCalibratedRed:0.40f green:0.69f blue:0.45f alpha:1.0f];
    _fgColorWell.color = niceGreenColor;
    [self pickNewForeColor:_fgColorWell];

    _bgColorWell.color = [NSColor blueColor];
    [self pickNewBackColor:_bgColorWell];
}


//------------------------------------------------------------------------------
#pragma mark -
#pragma mark IB Actions
//------------------------------------------------------------------------------

- (IBAction)pickNewForeColor:(id)sender
{
    [_mainView progressIndicatorLayer].color = [sender color];
}

- (IBAction)pickNewBackColor:(id)sender
{
    [_mainView setPlainBackgroundColor:[sender color]];
}

- (IBAction)startStopProgressIndicator:(id)sender
{
    if ([[_mainView progressIndicatorLayer] isRunning]) {
        // it is running, so stop it
        [[_mainView progressIndicatorLayer] stopProgressAnimation];
        [_startStopButton setTitle:@"Start"];
    }
    else {
        // it is stopped, so start it
        [[_mainView progressIndicatorLayer] startProgressAnimation];
        [_startStopButton setTitle:@"Stop"];
    }
}


@end
