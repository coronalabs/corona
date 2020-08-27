//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaViewControllerPrivate.h"

#import "CoronaViewPrivate.h"
#import <MetalANGLE/angle_gl.h>
#import "CoronaRuntime.h"
#import "CoronaViewPluginContext.h"

// ----------------------------------------------------------------------------

@interface CoronaViewController()

@property (nonatomic, retain) id< CoronaRuntime > pluginContext;

@end

// ----------------------------------------------------------------------------

@implementation CoronaViewController

//// To support resizing windows we'll have to implement something here.
//-(void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
//{
//#ifdef Rtt_ORIENTATION
//	CoronaView *coronaView = (CoronaView *)self.view;
//	[coronaView notifyRuntimeAboutOrientationChange:[self interfaceOrientation]];
//#endif
//}

-(void)viewWillLayoutSubviews
{
#ifdef Rtt_ORIENTATION
	CoronaView *coronaView = (CoronaView *)self.view;
	[coronaView notifyRuntimeAboutOrientationChange:[self interfaceOrientation]];
#endif
}


- (void)dealloc
{
	if ( [MGLContext currentContext] == self.context )
	{
        [MGLContext setCurrentContext:nil];
    }
    
    [_context release];
	
	[super dealloc];
}

- (void)loadView
{
	if ( nil != self.nibName )
	{
		[super loadView];
	}
	else
	{
		// Default to full screen
		UIScreen *screen = [UIScreen mainScreen];
		CGRect screenBounds = screen.bounds; // includes status bar
		
		if(!self.context) self.context = [[[MGLContext alloc] initWithAPI:kMGLRenderingAPIOpenGLES2] autorelease];


		CoronaView *view = [[CoronaView alloc] initWithFrame:screenBounds context:self.context];
		self.view = view;
		[view release];
	}

	self.pluginContext = [[CoronaViewPluginContext alloc] initWithOwner:self];
	((CoronaView *)self.view).pluginContext = self.pluginContext;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    if(!self.context) self.context = [[[MGLContext alloc] initWithAPI:kMGLRenderingAPIOpenGLES2] autorelease];

    if ( ! self.context )
	{
        NSLog( @"Failed to create ES context" );
    }

	CoronaView *view = (CoronaView *)self.view;

	view.context = self.context;
	view.drawableDepthFormat = MGLDrawableDepthFormat24;
}

#if Rtt_DEBUG_VIEWCONTROLLER

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
}

// MGLKViewControllerDelegate
- (void)mglkViewControllerUpdate:(MGLKViewController *)controller
{
}

- (void)mglkViewController:(MGLKViewController *)controller willPause:(BOOL)pause
{
}

#endif // Rtt_DEBUG_VIEWCONTROLLER

@end

// ----------------------------------------------------------------------------

