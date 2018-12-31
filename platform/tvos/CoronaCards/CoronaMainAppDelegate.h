//
//  CoronaMainAppDelegate.h
//  ratatouille
//
//  Copyright © 2015 Corona Labs. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol CoronaDelegate;

@interface CoronaMainAppDelegate : UIResponder <UIApplicationDelegate>

@property (retain, nonatomic) UIWindow *window;

- (void)initialize;
- (void)run;

@end

