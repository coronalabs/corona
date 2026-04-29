//
//  SPILDAppController.h
//  SPILDemo
//
//  Copyright 2009 Kelan Champagne. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SPILDTopLayerView;
@class SpinningProgressIndicatorLayer;


@interface SPILDAppController : NSObject {
    IBOutlet NSWindow *_window;
    IBOutlet SPILDTopLayerView *_mainView;

    IBOutlet NSButton *_startStopButton;
    IBOutlet NSColorWell *_fgColorWell;
    IBOutlet NSColorWell *_bgColorWell;
}

// IB Actions
- (IBAction)pickNewForeColor:(id)sender;
- (IBAction)pickNewBackColor:(id)sender;
- (IBAction)startStopProgressIndicator:(id)sender;

@end
