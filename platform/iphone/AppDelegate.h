//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <UIKit/UIKit.h>
#include <math.h>

//Must include this for isinf usage in mapkit
using namespace std;
#import <MapKit/MapKit.h>
#import "Rtt_IPhoneMapViewObject.h"

#import "CoronaRuntime.h"
#import "CoronaViewPrivate.h"
#import "CoronaViewControllerPrivate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	class IPhonePlatform;
	class Runtime;
}

// ----------------------------------------------------------------------------

@class AddressAnnotationWithCallout;
@class CMAccelerometerData;
@class CoronaView;
@class UIViewController;
@protocol CoronaDelegate;
@protocol CoronaViewLaunchDelegate;

// ----------------------------------------------------------------------------

@interface AppViewController : CoronaViewController
{
	UIResponder *fNextResponder;

    // NEW: Internal flags for gesture deferral
    BOOL scDeferSystemGestures;   // Enable/disable flag
    UIRectEdge scDeferEdges;      // Which edges to defer
}

- (void)setNextResponder:(UIResponder *)responder;
- (BOOL)prefersStatusBarHidden;
- (UIStatusBarStyle)preferredStatusBarStyle;

- (void)setSystemGestureDeferralEnabled:(BOOL)enabled;
- (void)setSystemGestureDeferEdges:(UIRectEdge)edges;
- (BOOL)isSystemGestureDeferralEnabled;
- (UIRectEdge)systemGestureDeferEdges;

// Override this method instead of using @property
- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures;

@property (nonatomic, assign) BOOL prefersHomeIndicatorAutoHidden;
@property (nonatomic, assign) bool prefersStatusBarhidden;
@property (nonatomic, assign) UIStatusBarStyle preferredStatusBarStyle;

@end

// ----------------------------------------------------------------------------

@interface AppDelegate : NSObject<
							CoronaRuntime,
							UIApplicationDelegate,
							MKMapViewDelegate,
							CoronaViewLaunchDelegate >
{
	UIWindow *window;
	CoronaView *view;
	AppViewController *viewController;
	id<CoronaDelegate> fCoronaDelegate;
	int fSuspendCount;
	double fGravityAccel[3];
	double fInstantAccel[3];
	CFTimeInterval fPreviousShakeTime;
	BOOL fIsAppStarted;
	BOOL appEnteredBackground;
	NSTimeInterval lastAccelerometerTimeStamp;
	UIView *splashView;
	NSTimeInterval fAppLaunchTime;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, readonly) UIView *view;
@property (nonatomic, retain) IBOutlet UIViewController *viewController;

@property (nonatomic, assign) NSTimeInterval lastAccelerometerTimeStamp; // Needed a public variable to initialize the variable from the caller to 0 when the accelerometer starts.

// Called by Rtt::IPhoneDevice with CoreMotion samples (UIAccelerometer was removed from the iOS 27 SDK)
- (void)accelerometerDidUpdate:(CMAccelerometerData *)data;

- (Rtt::Runtime *)runtime;

- (id<CoronaDelegate>)coronaDelegate;

- (void)didLoadMain:(id<CoronaRuntime>)runtime;

@end

// ----------------------------------------------------------------------------

// Scene-based life cycle (mandatory for apps built with the iOS 27 SDK). Named by UIApplicationSceneManifest in
// Info.plist: it attaches the app's window to the window scene, starts the runtime on the first connection and
// forwards the scene callbacks to the application-level AppDelegate methods, so CoronaDelegate plugins keep
// receiving the same callbacks as before.
API_AVAILABLE(ios(13.0))
@interface CoronaSceneDelegate : UIResponder< UIWindowSceneDelegate >
@end

