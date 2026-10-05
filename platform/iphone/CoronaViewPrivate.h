//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaCards/CoronaView.h"

#import <UIKit/UIKit.h>

#import "CoronaRuntime.h"
#include "Rtt_DeviceOrientation.h"
#include "Rtt_MetalAngleTypes.h"

// ----------------------------------------------------------------------------

@class CoronaViewController;
@protocol CoronaOrientationObserver;
@protocol CoronaGyroscopeObserver;
@protocol CLLocationManagerDelegate;

// C++ Forward Declarations
namespace Rtt
{

class Runtime;
class CoronaViewRuntimeDelegate;
class IPhonePlatformBase;

} // Rtt

// ----------------------------------------------------------------------------

// Used in SDK/Enterprise to enable custom initialization of the Rtt::Runtime
// and Rtt::IPhonePlatformBase during relaunches
@protocol CoronaViewLaunchDelegate

@required
- (NSInteger)runView:(CoronaView *)sender withPath:(NSString *)path parameters:(NSDictionary *) params;

@end

// ----------------------------------------------------------------------------

@interface CoronaView() <UITextFieldDelegate, UITextViewDelegate>

@property (nonatomic, readonly) Rtt::Runtime *runtime;
@property (nonatomic, readonly) Rtt::CoronaViewRuntimeDelegate *runtimeDelegate;
@property (nonatomic, readonly) Rtt_GLKViewController *viewController;
@property (nonatomic, readwrite, getter=inhibitCount, setter=setInhibitCount:) int fInhibitCount;
@property (nonatomic, readwrite, getter=tapDelay, setter=setTapDelay:) NSTimeInterval fTapDelay;
@property (nonatomic, assign) BOOL observeSuspendResume;
@property (nonatomic, assign) BOOL beginRunLoopManually;
@property (nonatomic, assign) id< CoronaRuntime > pluginContext; // Weak reference. CoronaViewController has the strong reference.
@property (nonatomic, assign) id< CoronaViewLaunchDelegate > launchDelegate;

@property (nonatomic, assign) id< CoronaOrientationObserver > orientationObserver;
@property (nonatomic, assign) id< CLLocationManagerDelegate > locationObserver;
@property (nonatomic, assign) id< CoronaGyroscopeObserver > gyroscopeObserver;

+ (Rtt::DeviceOrientation::Type)deviceOrientationForString:(NSString *)value;
+ (Rtt::DeviceOrientation::Type)defaultOrientation;

- (void)initializeRuntimeWithPlatform:(Rtt::IPhonePlatformBase *)platform runtimeDelegate:(Rtt::CoronaViewRuntimeDelegate *)runtimeDelegate;

- (NSInteger)runWithPath:(NSString*)path parameters:(NSDictionary *)params;

- (NSInteger)runWithPath:(NSString*)path parameters:(NSDictionary *)params orientation:(Rtt::DeviceOrientation::Type)orientation;

- (NSInteger)beginRunLoop;

- (BOOL)simulateCommand:(NSDictionary *)options;

- (void)terminate;

#ifdef Rtt_ORIENTATION
- (void)didOrientationChange:(id)sender;
- (void)notifyRuntimeAboutOrientationChange:(UIInterfaceOrientation)toInterfaceOrientation;
#endif

- (void)dismissKeyboard;

@end

// ----------------------------------------------------------------------------
