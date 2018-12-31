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

#import <Foundation/Foundation.h>

@class CLLocationManager;
@class CMMotionManager;

// ----------------------------------------------------------------------------

// @protocol CoronaSystemResourceObserver <NSObject>

// @end

// ----------------------------------------------------------------------------

// Keys for different system resources
FOUNDATION_EXPORT NSString * const CoronaOrientationResourceKey();
FOUNDATION_EXPORT NSString * const CoronaAccelerometerResourceKey();
FOUNDATION_EXPORT NSString * const CoronaGyroscopeResourceKey();
FOUNDATION_EXPORT NSString * const CoronaLocationResourceKey();
FOUNDATION_EXPORT NSString * const CoronaHeadingResourceKey();

// System Resource Manager
// Designed to be shared across multiple CoronaView instances
// TODO: Make this thread safe. Currently, must be invoked on main thread
@interface CoronaSystemResourceManager : NSObject

@property (nonatomic, retain) CLLocationManager *locationManager;
@property (nonatomic, retain) CMMotionManager *motionManager;

+ (instancetype)sharedInstance;

- (BOOL)resourceAvailableForKey:(NSString *)key;

- (void)addObserver:(id)observer forKey:(NSString *)key;
- (void)removeObserver:(id)observer forKey:(NSString *)key;

#ifdef Rtt_CORE_LOCATION
- (void)requestAuthorizationLocation;
#endif

@end

// ----------------------------------------------------------------------------
