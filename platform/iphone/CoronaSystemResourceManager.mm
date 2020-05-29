//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "CoronaSystemResourceManager.h"

#ifdef Rtt_ORIENTATION
#import "CoronaOrientationObserver.h"
#endif

#ifdef Rtt_CORE_MOTION
#import <CoreMotion/CoreMotion.h>
#import "CoronaGyroscopeObserver.h"
#endif

#import <UIKit/UIDevice.h>
#import <CoreLocation/CoreLocation.h>

@protocol UIAccelerometerDelegate;
@protocol CLLocationManagerDelegate;


// Resource Keys
// ----------------------------------------------------------------------------
#pragma mark # Resource Keys

NSString * const CoronaOrientationResourceKey()
{
	return @"Orientation";
}

NSString * const CoronaAccelerometerResourceKey()
{
	return @"Accelerometer";
}

NSString * const CoronaGyroscopeResourceKey()
{
	return @"Gyroscope";
}

NSString * const CoronaLocationResourceKey()
{
	return @"Location";
}

NSString * const CoronaHeadingResourceKey()
{
	return @"Heading";
}


// CoronaSystemResourceManager()
// ----------------------------------------------------------------------------
#pragma mark # CoronaSystemResourceManager()

@interface CoronaSystemResourceManager() <CLLocationManagerDelegate>

@property (nonatomic, retain) NSMutableDictionary *observersByKey;

// - (BOOL)validateObserver:(id)observer forKey:(NSString *)key;

- (NSMutableSet *)observersForKey:(NSString *)key;
- (BOOL)hasObserversForKey:(NSString *)key;

//- (void)removeObserver:(id)observer;
- (void)removeObservers;
- (void)removeObserversForKey:(NSString *)key;

#ifdef Rtt_ORIENTATION
- (void)addObserverOrientation:(id <CoronaOrientationObserver>)observer;
- (void)removeObserverOrientation:(id <CoronaOrientationObserver>)observer;
#endif

#ifdef Rtt_ACCELEROMETER
- (void)addObserverAccelerometer:(id <UIAccelerometerDelegate>)observer;
- (void)removeObserverAccelerometer:(id <UIAccelerometerDelegate>)observer;
#endif

#ifdef Rtt_CORE_MOTION
- (void)addObserverGyroscope:(id )observer;
- (void)removeObserverGyroscope:(id)observer;
#endif

#ifdef Rtt_CORE_LOCATION
- (void)addObserverLocation:(id <CLLocationManagerDelegate>)observer;
- (void)removeObserverLocation:(id <CLLocationManagerDelegate>)observer;

- (void)addObserverHeading:(id <CLLocationManagerDelegate>)observer;
- (void)removeObserverHeading:(id <CLLocationManagerDelegate>)observer;
#endif

@end


// CoronaSystemResourceManager
// ----------------------------------------------------------------------------
#pragma mark # CoronaSystemResourceManager

@implementation CoronaSystemResourceManager

+ (instancetype)sharedInstance
{
	static id sInstance = nil;
	static dispatch_once_t sOnce = 0;
	
	dispatch_once(&sOnce, ^{
		sInstance = [[self alloc] init];
	});

	return sInstance;
}

+ (NSSet *)validKeys
{
	static NSSet *sKeys = nil;

	if ( nil == sKeys )
	{
		NSArray *keys = [NSArray arrayWithObjects:
			CoronaAccelerometerResourceKey(),
			CoronaLocationResourceKey(),

#ifdef Rtt_ORIENTATION
			CoronaOrientationResourceKey(),
#endif

#ifdef Rtt_CORE_MOTION
			CoronaGyroscopeResourceKey(),
#endif

#ifdef Rtt_CORE_LOCATION
			CoronaHeadingResourceKey(),
#endif
			nil];
		sKeys = [[NSSet alloc] initWithArray:keys];
	}

	return sKeys;
}

- (instancetype)init
{
	self = [super init];
	if ( self )
	{
		_observersByKey = [NSMutableDictionary new];
	}
	return self;
}

- (void)dealloc
{
	[_locationManager release];

#ifdef Rtt_CORE_MOTION
	[_motionManager release];
#endif

	[self removeObservers];
	[_observersByKey release];

	[super dealloc];
}

- (CLLocationManager *)locationManager
{
	if ( nil == _locationManager )
	{
		_locationManager = [[CLLocationManager alloc] init];
	}
	
	return _locationManager;
}

#ifdef Rtt_CORE_MOTION
- (CMMotionManager *)motionManager
{
	if ( nil == _motionManager )
	{
		_motionManager = [[CMMotionManager alloc] init];
	}
	
	return _motionManager;
}
#endif

- (BOOL)resourceAvailableForKey:(NSString *)key
{
	BOOL result = [[[self class] validKeys] containsObject:key];
	
	if ( result )
	{
#ifdef Rtt_CORE_MOTION
		// Additional checks
		if ( [key isEqualToString:CoronaGyroscopeResourceKey()] )
		{
			result = [self motionManager].gyroAvailable;
		}
#endif
	}
	
	return result;
}

- (NSMutableSet *)observersForKey:(NSString *)key
{
	NSMutableSet *result = [self.observersByKey valueForKey:key];
	
	// Initialize set if none exists
	if ( ! result
			&& [[CoronaSystemResourceManager validKeys] containsObject:key] )
	{
		result = [NSMutableSet set];
		[self.observersByKey setValue:result forKey:key];
	}
	
	return result;
}

- (BOOL)hasObserversForKey:(NSString *)key
{
	return ( [[self observersForKey:key] count] > 0 );
}

- (void)addObserver:(id)observer forKey:(NSString *)key
{
	SEL sel = NSSelectorFromString( [NSString stringWithFormat:@"addObserver%@:", key] );
	if ( [self respondsToSelector:sel] )
	{
		[self performSelector:sel withObject:observer];

		// Update observer bookkeeping *after* system-specific operation
		[[self observersForKey:key] addObject:observer];
	}
}

- (void)removeObserver:(id)observer forKey:(NSString *)key
{
	SEL sel = NSSelectorFromString( [NSString stringWithFormat:@"removeObserver%@:", key] );
	if ( [self respondsToSelector:sel] )
	{
		// Update observer bookkeeping *before* system-specific operation
		[[self observersForKey:key] removeObject:observer];

		[self performSelector:sel withObject:observer];
	}
}

// Convenience function if you don't actually know what resource type
/*
- (void)removeObserver:(id)observer
{
	for ( NSString *key in [CoronaSystemResourceManager validKeys] )
	{
		[self removeObserver:observer forKey:key];
	}
}
*/

- (void)removeObservers
{
	NSSet *validKeys = [[self class] validKeys];
	for ( NSString *key in validKeys )
	{
		[self removeObserversForKey:key];
	}
}

- (void)removeObserversForKey:(NSString *)key
{
	SEL sel = NSSelectorFromString( [NSString stringWithFormat:@"removeObserver%@:", key] );
	if ( [self respondsToSelector:sel] )
	{
		NSMutableSet *observersSrc = [self observersForKey:key];

		// Update observer bookkeeping *before* system-specific operation
		NSSet *observersCopy = [[observersSrc copy] autorelease]; // Copy before removing
		[observersSrc removeAllObjects];

		for ( id observer in observersCopy )
		{
			[self performSelector:sel withObject:observer];
		}
	}
}

// -----------------------------------------------------------------------------

#ifdef Rtt_ORIENTATION

- (void)addObserverOrientation:(id <CoronaOrientationObserver>)observer
{
	if ( ! [self hasObserversForKey:CoronaOrientationResourceKey()] )
	{
		[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	}
}

- (void)removeObserverOrientation:(id <CoronaOrientationObserver>)observer
{
	if ( ! [self hasObserversForKey:CoronaOrientationResourceKey()] )
	{
		[[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
	}
}

#endif // Rtt_ORIENTATION

// -----------------------------------------------------------------------------

#ifdef Rtt_ACCELEROMETER

- (void)addObserverAccelerometer:(id <UIAccelerometerDelegate>)observer
{
	;
}

- (void)removeObserverAccelerometer:(id <UIAccelerometerDelegate>)observer
{
	;
}

#endif // Rtt_ACCELEROMETER

// -----------------------------------------------------------------------------

#ifdef Rtt_CORE_MOTION

- (void)addObserverGyroscope:(id)observer
{
	NSString *key = CoronaGyroscopeResourceKey();
	if ( [self resourceAvailableForKey:key] )
	{
		if ( ! [self hasObserversForKey:key] )
		{
			// For games/best performance, Apple recommends polling periodically
			// instead of registering a block/callback handler
			[self.motionManager startGyroUpdates];
		}

		if ( [observer conformsToProtocol:@protocol(CoronaGyroscopeObserver)] )
		{
			[observer startPolling];
		}
	}
}

- (void)removeObserverGyroscope:(id)observer
{
	NSString *key = CoronaGyroscopeResourceKey();
	if ( [self resourceAvailableForKey:key] )
	{
		if ( ! [self hasObserversForKey:key] )
		{
			[self.motionManager stopGyroUpdates];
		}

		if ( [observer conformsToProtocol:@protocol(CoronaGyroscopeObserver)] )
		{
			[observer stopPolling];
		}
	}
}

#endif // Rtt_CORE_MOTION

// -----------------------------------------------------------------------------

#ifdef Rtt_CORE_LOCATION

- (void)addObserverLocation:(id <CLLocationManagerDelegate>)observer
{
	if ( ! [self hasObserversForKey:CoronaLocationResourceKey()] )
	{
		[self requestAuthorizationLocation];
		CLLocationManager *locationManager = [self locationManager];
		locationManager.delegate = self;

		locationManager.desiredAccuracy = kCLLocationAccuracyBest;
		[locationManager startUpdatingLocation];
	}
}

- (void)removeObserverLocation:(id <CLLocationManagerDelegate>)observer
{
	if ( ! [self hasObserversForKey:CoronaLocationResourceKey()] )
	{
		CLLocationManager *locationManager = [self locationManager];
		[locationManager stopUpdatingLocation];
	}
}

// From (https://developer.apple.com/library/prerelease/ios/documentation/CoreLocation/Reference/CLLocationManager_Class/index.html)
// 
// > The user prompt contains the text from the NSLocationWhenInUseUsageDescription key 
// > in your app’s Info.plist file, and the presence of that key is required when calling
// > this method.
- (void)requestAuthorizationLocation
{
    CLLocationManager *locationManager = [self locationManager];
    
    // Set the background property if the background property is set in build.settings
    //
    // plist: UIBackgroundModes is an array containing keys. We want to find the key 'location'.
    NSArray* modes = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"UIBackgroundModes"];
    if ( [modes containsObject:@"location"] &&
        [locationManager respondsToSelector:@selector(setAllowsBackgroundLocationUpdates:)] )
    {
        // This property is only available in iOS 9.0 and above
        [self.locationManager setAllowsBackgroundLocationUpdates:YES];
    }
    
	// Calling requestWhenInUseAuthorization only does something if the authorization
	// status is kCLAuthorizationStatusNotDetermined so if its something else we can
	// skip this block
	if ( [CLLocationManager authorizationStatus] != kCLAuthorizationStatusNotDetermined )
	{
		return;
	}
	
    // Requesting location authorization for the first time
	if ( [locationManager respondsToSelector:@selector(requestWhenInUseAuthorization)] )
	{
		[locationManager requestWhenInUseAuthorization];
	}
}

// ----------------------------------------------------------------------------

- (void)addObserverHeading:(id <CLLocationManagerDelegate>)observer
{
	if ( ! [self hasObserversForKey:CoronaHeadingResourceKey()] )
	{
		CLLocationManager *locationManager = [self locationManager];
		locationManager.delegate = self;

		if ([CLLocationManager headingAvailable])
		{
			[locationManager startUpdatingHeading];
		}
	}
}

- (void)removeObserverHeading:(id <CLLocationManagerDelegate>)observer
{
	if ( ! [self hasObserversForKey:CoronaHeadingResourceKey()] )
	{
		CLLocationManager *locationManager = [self locationManager];

		if ([CLLocationManager headingAvailable])
		{
			[locationManager stopUpdatingHeading];
		}
	}
}

#endif // Rtt_CORE_LOCATION


// Accelerometer
// ----------------------------------------------------------------------------
#pragma mark # Accelerometer

// Gyroscope
// ----------------------------------------------------------------------------
#pragma mark # Gyroscope

#ifdef Rtt_CORE_LOCATION

// CLLocationManager (This multiplexes calls to observers)
// ----------------------------------------------------------------------------
#pragma mark # CLLocationManager

- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{
	NSSet *observers = [self observersForKey:CoronaLocationResourceKey()];
	for ( id <CLLocationManagerDelegate> o in observers )
	{
		if ( [o respondsToSelector:@selector(locationManager:didUpdateToLocation:fromLocation:)] )
		{
			[o locationManager:manager didUpdateToLocation:newLocation fromLocation:oldLocation];
		}
	}
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
	NSSet *observers = [self observersForKey:CoronaLocationResourceKey()];
	for ( id <CLLocationManagerDelegate> o in observers )
	{
		if ( [o respondsToSelector:@selector(locationManager:didFailWithError:)] )
		{
			[o locationManager:manager didFailWithError:error];
		}
	}
}

- (void)locationManager:(CLLocationManager *)manager didUpdateHeading:(CLHeading *)newHeading
{	
	NSSet *observers = [self observersForKey:CoronaHeadingResourceKey()];
	for ( id <CLLocationManagerDelegate> o in observers )
	{
		if ( [o respondsToSelector:@selector(locationManager:didUpdateHeading:)] )
		{
			[o locationManager:manager didUpdateHeading:newHeading];
		}
	}
}

- (BOOL)locationManagerShouldDisplayHeadingCalibration:(CLLocationManager *)manager
{
	return YES;
}

- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
	NSSet *observers = [self observersForKey:CoronaLocationResourceKey()];
	for ( id <CLLocationManagerDelegate> o in observers )
	{
		if ( [o respondsToSelector:@selector(locationManager:didChangeAuthorizationStatus:)] )
		{
			[o locationManager:manager didChangeAuthorizationStatus:status];
		}
	}
}
#endif // Rtt_CORE_LOCATION

@end

// ----------------------------------------------------------------------------
