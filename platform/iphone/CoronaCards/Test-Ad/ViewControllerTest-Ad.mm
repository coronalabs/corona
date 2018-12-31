//
//  ViewControllerTest-Ad.mm
//
//  Copyright (c) 2013 Walter Luh. All rights reserved.
//

#import "ViewControllerTest-Ad.h"

#import "CoronaKit/CoronaKit.h"

@interface ViewController ()

@property (nonatomic, strong) CoronaViewController *bannerController;
@property (nonatomic, strong) CoronaViewController *interstitialController;

@end

@implementation ViewController

// ViewController overrides
// ----------------------------------------------------------------------------

- (void)viewDidLoad
{
	[super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.

	[self showBanner];
}

- (void)didReceiveMemoryWarning
{
	[super didReceiveMemoryWarning];
	// Dispose of any resources that can be recreated.
}

// Banner
// ----------------------------------------------------------------------------

- (void)showBanner
{
	self.bannerController = [[[CoronaViewController alloc] init] autorelease];
	[self addChildViewController:_bannerController];

	UIView *parent = self.view;
	CoronaView *bannerView = (CoronaView *)_bannerController.view;
	
	// Size Corona banner
	const CGFloat kBannerHeight = 50;
	const CGRect screenBounds = [[UIScreen mainScreen] bounds];

	CGRect bannerFrame = parent.frame;
	bannerFrame.size.height = kBannerHeight;
	bannerFrame.origin.y = screenBounds.size.height - kBannerHeight;
	bannerView.frame = bannerFrame;

	[parent addSubview:bannerView];
	
	// Tap triggers interstitial
	UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap:)];
	[bannerView addGestureRecognizer:tapRecognizer];

	// Get path for banner ad
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *path = [[bundle resourcePath] stringByAppendingPathComponent:@"BannerAd"];
	[bannerView runWithPath:path parameters:nil];
}

- (void)handleTap:(UIGestureRecognizer *)sender
{
	if ( sender.state == UIGestureRecognizerStateEnded )
	{
		[self showInterstitial];
	}
}

// Interstitial
// ----------------------------------------------------------------------------

- (void)showInterstitial
{
	_bannerController.paused = YES;

	self.interstitialController = [[[CoronaViewController alloc] init] autorelease];
	[self addChildViewController:_interstitialController];

	CoronaView *interstitialView = (CoronaView *)_interstitialController.view;
	interstitialView.coronaViewDelegate = self;
	[self.view addSubview:interstitialView];
	
	// Get path for interstitial ad
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *path = [[bundle resourcePath] stringByAppendingPathComponent:@"InterstitialAd"];
	[interstitialView runWithPath:path parameters:nil];
}

- (void)closeInterstitial
{
	_bannerController.paused = NO;

	_interstitialController.paused = YES;
	[_interstitialController removeFromParentViewController];

	CoronaView *interstitialView = (CoronaView *)_interstitialController.view;
	[interstitialView removeFromSuperview];
	self.interstitialController = nil;
}

// CoronaViewDelegate
// ----------------------------------------------------------------------------

- (id)coronaView:(CoronaView *)view receiveEvent:(NSDictionary *)event
{
	id result = nil;

	NSString *action = [event valueForKey:@"action"];
	if ( [action isEqualToString:@"close"] )
	{
		[self closeInterstitial];
	}

	return result;
}

@end
