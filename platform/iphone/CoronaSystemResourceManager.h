//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
