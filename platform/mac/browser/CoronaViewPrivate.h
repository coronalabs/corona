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

#import <AppKit/NSView.h>
#import <CoreLocation/CoreLocation.h>

#import "CoronaCards/CoronaView.h"

#import "GLView.h"
#import "GLViewDelegate.h"
#import "CoronaViewRuntimeDelegate.h"

// ----------------------------------------------------------------------------

const int CoronaViewPrivateDefaultWidth = 320;
const int CoronaViewPrivateDefaultHeight = 480;

namespace Rtt
{
	class MacPlatform;
	class MacViewCallback;
	class Runtime;
	class ProjectSettings;
    class AppleInputHIDDeviceListener;
} // Rtt

@class CoronaView;
@class AppleInputMFiDeviceListener;

// ----------------------------------------------------------------------------

@protocol CoronaViewControllerDelegate
- (void)willLoadApplication:(CoronaView*)sender;
- (void)didLoadApplication:(CoronaView*)sender;
@end

// ----------------------------------------------------------------------------

@interface CoronaView() <GLViewDelegate>
{
	@private
		NSString *_projectPath;
		GLView *_GLView;
		Rtt::MacPlatform *_platform;
		Rtt::Runtime *_runtime;
		id< CoronaViewDelegate > _viewDelegate;
		Rtt::MacViewCallback *_GLViewCallback;
		Rtt::ProjectSettings *_projectSettings;
        Rtt::AppleInputHIDDeviceListener *_macHIDInputDeviceListener;
		AppleInputMFiDeviceListener *_macMFiDeviceListener;
}

@property (nonatomic, readwrite, getter=projectPath) NSString *_projectPath;
@property (nonatomic, readonly, getter=glView) GLView *_GLView;
@property (nonatomic, readwrite, getter=runtime, setter=setRuntime:) Rtt::Runtime *_runtime;
@property (nonatomic, readwrite, assign, getter=viewDelegate, setter=setViewDelegate:) id< CoronaViewDelegate > _viewDelegate;
@property (nonatomic, readonly) Rtt::MacPlatform *_platform;
@property (nonatomic, readonly) Rtt::CoronaViewRuntimeDelegate *_runtimeDelegate;
@property (nonatomic, readonly, getter=projectSettings) Rtt::ProjectSettings *_projectSettings;
@property (nonatomic, readwrite, copy) CLLocationManager *_locationManager;
@property (nonatomic, readwrite, copy) CLLocation *_currentLocation;
@property (nonatomic, readwrite, copy) NSDictionary *_launchParams;

- (id)initWithPath:(NSString *)path frame:(NSRect)frame;
- (void) startLocationUpdating;
- (void) endLocationUpdating;

@end

// ----------------------------------------------------------------------------
