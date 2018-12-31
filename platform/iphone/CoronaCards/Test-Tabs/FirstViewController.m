//
//  FirstViewController.m
//  CoronaKitTabs
//
//  Created by Walter Luh on 12/10/13.
//
//

#import "FirstViewController.h"

#import "CoronaCards/CoronaCards.h"

@interface FirstViewController ()

@property (nonatomic, strong) CoronaViewController *coronaController;

@end

@implementation FirstViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.

	_coronaController = [[[CoronaViewController alloc] init] autorelease];
	[self addChildViewController:_coronaController];

	CoronaView *coronaView = (CoronaView *)_coronaController.view;

	// Same size as parent view
	coronaView.frame = self.view.frame;

	[self.view addSubview:coronaView];

	// Transparent background
	coronaView.backgroundColor = [UIColor clearColor];
	coronaView.opaque = NO;

	[coronaView run];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
