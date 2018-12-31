//
//  CoronaMainViewController.mm
//  ratatouille
//
//  Copyright © 2015 Corona Labs. All rights reserved.
//

#include "Core/Rtt_Build.h"

#import "CoronaCards/CoronaMainViewController.h"

#import "CoronaViewPrivate.h"
#import "CoronaViewController.h"

#include "Rtt_TVOSPlatform.h"
#include "Rtt_IPhoneRuntimeDelegate.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"

#import "CoronaCards/CoronaMainAppDelegate.h"

// TODO
// * GCMicroGamepad
//     + [GCEventViewController controllerUserInteractionEnabled]
//     + https://developer.apple.com/library/prerelease/tvos/documentation/General/Conceptual/AppleTV_PG/WorkingwithGameControllers.html
//     + https://developer.apple.com/library/prerelease/tvos/documentation/GameController/Reference/GCEventViewController_Ref/index.html
// * Focus (controllerUserInteractionEnabled == YES)
//     + https://developer.apple.com/library/prerelease/tvos/documentation/General/Conceptual/AppleTV_PG/WorkingwiththeAppleTVRemote.html
//     + https://developer.apple.com/library/prerelease/tvos/documentation/UIKit/Reference/UIFocusEnvironment_Protocol/index.html
//     + https://developer.apple.com/library/prerelease/tvos/documentation/UIKit/Reference/UIFocusUpdateContext_Class/index.html
// * Widget
//     + Focus ring
//     + MicroGamepad's D-pad
// * Plugin (?)
//     + UIButton
//     + UIProgressView
@interface CoronaMainViewController()

@end


@implementation CoronaMainViewController

- (void)loadView
{
	[super loadView];

	CoronaViewController *controller = [[[CoronaViewController alloc] init] autorelease];
	self.coronaViewController = controller;
	[self addChildViewController:controller];

	CoronaView *coronaView = (CoronaView *)controller.view;
	Rtt_ASSERT( [coronaView isKindOfClass:[CoronaView class]] );

	coronaView.frame = self.view.frame;
	[self.view addSubview:coronaView];

	[GCController startWirelessControllerDiscoveryWithCompletionHandler:nil];
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	CoronaMainAppDelegate *appDelegate = (CoronaMainAppDelegate *)[UIApplication sharedApplication].delegate;
	[appDelegate initialize];
	[appDelegate run];
}

- (void)viewDidAppear:(BOOL)animated
{
//	NSLog( @"[DID] ---------" );
//	NSLog( @"self.view: %@", self.view );
//	NSLog( @"self.view.window: %@", self.view.window );
//	NSLog( @"self.view.coronaView: %@", self.coronaViewController.view );
//	NSLog( @"self.view.coronaView.window: %@", self.coronaViewController.view.window );

	// TODO: Revisit this in later tvOS beta's.
	// By viewDidAppear is invoked, CoronaView has been properly added to the
	// window hierarchy so it's safe to execute main.lua
	CoronaView *view = (CoronaView *)self.coronaViewController.view;
	[view beginRunLoop];
}

@end
