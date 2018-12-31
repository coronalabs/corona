//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaCards/CoronaView.h"

#import <UIKit/UIKit.h>

#import "CoronaRuntime.h"
#include "Rtt_DeviceOrientation.h"


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
@property (nonatomic, readonly) GLKViewController *viewController;
@property (nonatomic, readwrite, getter=inhibitCount, setter=setInhibitCount:) int fInhibitCount;
@property (nonatomic, readwrite, getter=tapDelay, setter=setTapDelay:) NSTimeInterval fTapDelay;
@property (nonatomic, readwrite, getter=getForceTouchSupport, setter=setForceTouchSupport:) BOOL fSupportsForceTouch;
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
