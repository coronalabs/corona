//
//  CoronaMainViewController.h
//  ratatouille
//
//  Copyright © 2015 Corona Labs. All rights reserved.
//

#import <GameController/GCController.h>
#import <GameController/GCEventViewController.h>

@class CoronaView;
@class CoronaViewController;

// "On tvOS, when you want your game to process controller input directly,
// you need to use GCEventViewController (or a subclass) to display your
// game content."
// [Source](https://developer.apple.com/library/prerelease/tvos/documentation/General/Conceptual/AppleTV_PG/WorkingwithGameControllers.html)
@interface CoronaMainViewController : GCEventViewController

@property (retain, nonatomic) CoronaViewController *coronaViewController;

@end
