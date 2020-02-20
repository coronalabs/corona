//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_IPhoneMapViewObject.h"

#import <UIKit/UIKit.h>
#include <math.h>
#import <MapKit/MapKit.h>
#import <MapKit/MKAnnotation.h>

#import "AppDelegate.h"
#import "CoronaSystemResourceManager.h"

#include "Rtt_Event.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaResource.h"

#ifdef Rtt_DEBUG
	// Used in asserts in Initialize()
	#include "Display/Rtt_Display.h"
	#include "Rtt_Runtime.h"
#endif
#include "Rtt_Runtime.h"
#include "Rtt_IPhoneDevice.h"

// ----------------------------------------------------------------------------

@implementation Rtt_MKMapView

@synthesize owner;
@synthesize trackUserLocationAfterAuthorization;


-(void)askForUserLocationAuthorization
{
	//removing self just avoid multiple instances of this map. If our map wasn't in list before this is no-op.
	[[CoronaSystemResourceManager sharedInstance] removeObserver:self forKey:CoronaLocationResourceKey()];
	//adding map to track location authorization changes
	[[CoronaSystemResourceManager sharedInstance] addObserver:self    forKey:CoronaLocationResourceKey()];
	
	[[CoronaSystemResourceManager sharedInstance] requestAuthorizationLocation];

}

-(void)locationManager:manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status{
	if(status!=kCLAuthorizationStatusNotDetermined
		&& status != kCLAuthorizationStatusDenied
		&& status != kCLAuthorizationStatusRestricted)
	{
		self.showsUserLocation = self.trackUserLocationAfterAuthorization;
	}
	if(status!=kCLAuthorizationStatusNotDetermined)
	{
		[[CoronaSystemResourceManager sharedInstance] removeObserver:self forKey:CoronaLocationResourceKey()];
	}
}


- (id)initWithFrame:(CGRect)rect
{
	self = [super initWithFrame:rect];
	if ( self )
	{
		owner = nil;
	}
	return self;
}

- (void)handleGesture:(UIGestureRecognizer *)gestureRecognizer
{
    if (gestureRecognizer.state != UIGestureRecognizerStateEnded)
    {
        return;
    }
    
	// When the map view is touched we have to deselect all the annotations because returning NO from
	// gestureRecognizerShouldBegin causes the annotations to not be automatically deselected.
	for( id annotation in [self annotations] )
	{
		[self deselectAnnotation:annotation animated:NO];
	}
	
    CGPoint touchPoint = [gestureRecognizer locationInView:self];
    CLLocationCoordinate2D touchMapCoordinate = [self convertPoint:touchPoint toCoordinateFromView:self];
    Rtt::MapLocationEvent e(touchMapCoordinate.latitude, touchMapCoordinate.longitude);
    owner->DispatchEventWithTarget( e );
}

- (void)handleGestureForMapMarker:(UIGestureRecognizer *)gestureRecognizer
{
    if (gestureRecognizer.state != UIGestureRecognizerStateEnded)
    {
        return;
    }
	
	UIView *uiView = gestureRecognizer.view;
	if (![uiView respondsToSelector:@selector(annotation)])
	{
		return;
	}
	MKAnnotationView* annotationView = (MKAnnotationView*)uiView;
	
	AddressAnnotationWithCallout *annotation = (annotationView).annotation;
	
	[self selectAnnotation:annotation animated:NO];
	
	using namespace Rtt;
	double latitude = annotation.coordinate.latitude;
	double longitude = annotation.coordinate.longitude;
    
    int markerId = annotation.annotationId;
    
	MapMarkerEvent e( markerId, latitude, longitude );
	
	Rtt:LuaResource * resource = annotation.luaResourceForListener;
	if ( resource )
	{
		resource->DispatchEvent( e );
	}
}

//Prevents the MKMapView gesture recognizer from activating which will cause our gesture handler to not activate
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    return NO;
}

@end

@implementation AddressAnnotationWithCallout

@synthesize coordinate;
@synthesize luaResourceForListener;
@synthesize annotationView;

- (NSString *)subtitle{
	return mSubTitle;
}
- (NSString *)title{
	return mTitle;
}

-(id)initWithCoordinate:(CLLocationCoordinate2D) c title:(NSString *) title subtitle:(NSString *) subtitle
{
	self = [super init];
	if ( self )
	{
		coordinate = c;
		mTitle = [title retain];
		mSubTitle = [subtitle retain];
	}
	return self;
}

-(void)dealloc
{
	[mSubTitle release];
	[mTitle release];

	if (luaResourceForListener) {
		Rtt_DELETE([self luaResourceForListener]);
	}

	[super dealloc];
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneMapViewObject::IPhoneMapViewObject( const Rect& bounds )
:	Super( bounds )
{
}

IPhoneMapViewObject::~IPhoneMapViewObject()
{
	Rtt_MKMapView *t = (Rtt_MKMapView*)GetView();
	t.owner = NULL;
}

bool
IPhoneMapViewObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
	UIViewController *controller = delegate.viewController;

	Rtt_ASSERT( delegate.runtime->GetDisplay().PointsWidth() == [UIScreen mainScreen].bounds.size.width );
	Rtt_ASSERT( delegate.runtime->GetDisplay().PointsHeight() == [UIScreen mainScreen].bounds.size.height );

	Rect screenBounds;
	GetScreenBounds( screenBounds );
	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
	Rtt_MKMapView *t = [[Rtt_MKMapView alloc] initWithFrame:r];

	t.delegate = delegate;
	t.owner = this;

	UITapGestureRecognizer *tgr = [[UITapGestureRecognizer alloc]initWithTarget:t action:@selector(handleGesture:)];
	[t addGestureRecognizer:tgr];
	[tgr release];

	UIView *parent = controller.view;
	[parent addSubview:t];

	Super::InitializeView( t );
	[t release];
	
	// Some reasonable default values
	MKCoordinateRegion region;
	MKCoordinateSpan span = { 0.01f, 0.01f };
	region.span = span;

	CLLocationCoordinate2D center = { 37.47f, 122.12f };
	region.center = center;

	t.region = region;
	if ( [CLLocationManager locationServicesEnabled] && !(  [CLLocationManager authorizationStatus] == kCLAuthorizationStatusDenied
														  ||[CLLocationManager authorizationStatus] == kCLAuthorizationStatusNotDetermined
														  ||[CLLocationManager authorizationStatus] == kCLAuthorizationStatusRestricted ))
	{
		t.showsUserLocation = YES;
	}
	t.trackUserLocationAfterAuthorization = YES;

    markerIdCounter = 1;
    
	return t;
}

const LuaProxyVTable&
IPhoneMapViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetMapViewObjectProxyVTable();
}

int
IPhoneMapViewObject::getUserLocation( lua_State *L )
{
	IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );
	
	Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();
	
	
	if(![CLLocationManager locationServicesEnabled]) {
		lua_newtable(L);
		lua_pushnumber(L, -1);
		lua_setfield(L, -2, "errorCode");
		lua_pushstring(L, "Location services are disabled.");
		lua_setfield(L, -2, "errorMessage");
		return 1;
	}
	CLAuthorizationStatus authStatus = [CLLocationManager authorizationStatus];
	if(authStatus == kCLAuthorizationStatusDenied) {
		lua_newtable(L);
		lua_pushnumber(L, authStatus);
		lua_setfield(L, -2, "errorCode");
		lua_pushstring(L, "Location is denied by user.");
		lua_setfield(L, -2, "errorMessage");
		return 1;
	}
	if(authStatus == kCLAuthorizationStatusNotDetermined
	   ||authStatus == kCLAuthorizationStatusRestricted ) {
		
		[t askForUserLocationAuthorization];
		
		lua_newtable(L);
		lua_pushnumber(L, authStatus);
		lua_setfield(L, -2, "errorCode");
		lua_pushstring(L, "Pending user authorization.");
		lua_setfield(L, -2, "errorMessage");
		return 1;
	}
	
	
	MKUserLocation *userLocation = t.userLocation;
	CLLocation *location = userLocation.location;	
	CLLocationCoordinate2D coordinate = location.coordinate;
	CLLocationDistance altitude = location.altitude;
	CLLocationAccuracy accuracy = location.horizontalAccuracy;
	CLLocationSpeed speed = location.speed;
	double time = [location.timestamp timeIntervalSince1970];
	BOOL updating = userLocation.updating;
	
	lua_newtable( L );
	lua_pushnumber( L, Rtt_FloatToReal( coordinate.latitude ) );
	lua_setfield( L, -2, LocationEvent::kLatitudeKey );
	lua_pushnumber( L, Rtt_FloatToReal( coordinate.longitude ) );
	lua_setfield( L, -2, LocationEvent::kLongitudeKey );
	lua_pushnumber( L, Rtt_FloatToReal( altitude ) );
	lua_setfield( L, -2, LocationEvent::kAltitudeKey );
	lua_pushnumber( L, Rtt_FloatToReal( accuracy ) );
	lua_setfield( L, -2, LocationEvent::kAccuracyKey );
	lua_pushnumber( L, Rtt_FloatToReal( speed ) );
	lua_setfield( L, -2, LocationEvent::kSpeedKey );
	lua_pushnumber( L, Rtt_FloatToReal( time ) );
	lua_setfield( L, -2, LocationEvent::kTimeKey );
	lua_pushboolean( L, updating );
	lua_setfield( L, -2, LocationEvent::kIsUpdatingKey );
	
	return 1;
}

int
IPhoneMapViewObject::setRegion( lua_State *L )
{
	IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();
	
	// Specify center point of map
	Rtt_Real latitude = Rtt_FloatToReal( lua_tonumber( L, 2 ) );
	Rtt_Real longitude = Rtt_FloatToReal( lua_tonumber( L, 3 ) );
	
	CLLocationCoordinate2D center;
	center.latitude = latitude;
	center.longitude = longitude;

	// TODO: boundary-check the above and below values -- the maximum value for longitude is 180°, but for latitude it's only 90°
	
	// Specify visible map span (and implicitly the zoom level)
	Rtt_Real latitudeDelta = Rtt_FloatToReal( lua_tonumber( L, 4 ) );
	Rtt_Real longitudeDelta = Rtt_FloatToReal( lua_tonumber( L, 5 ) );

	MKCoordinateSpan span;
	span.latitudeDelta = latitudeDelta;
	span.longitudeDelta = longitudeDelta;

	MKCoordinateRegion region;
	region.center = center;
	region.span = span;
	
	BOOL isAnimated = lua_toboolean( L, 6 );

	// Move the map to the specified region, optionally with animation
	[t setRegion:region animated:isAnimated];
	
	// TODO: make sure the 5th parameter is optional (and it should default to false)

	return 0;
}

int
IPhoneMapViewObject::setCenter( lua_State *L )
{
	IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();
	
	// Specify center point of map
	Rtt_Real latitude = Rtt_FloatToReal( lua_tonumber( L, 2 ) );
	Rtt_Real longitude = Rtt_FloatToReal( lua_tonumber( L, 3 ) );
	
	CLLocationCoordinate2D center;
	center.latitude = latitude;
	center.longitude = longitude;

	BOOL isAnimated = lua_toboolean( L, 4 );

	// Move the map to the specified region, optionally with animation
	[t setCenterCoordinate:center animated:isAnimated];
	
	// TODO: make sure the 3rd parameter is optional (and it should default to false)

	return 0;
}

// This function uses the native geocoding API to convert any address string to latitude, longitude.
int
IPhoneMapViewObject::requestLocation( lua_State *L )
{
	if( ! Lua::IsListener( L, 3, MapLocationEvent::kName ) )
	{
		char msg[ 128 ];
		sprintf( msg,
					"A function, or an object able to respond to %s, is expected",
					MapLocationEvent::kName );
		luaL_argerror( L, 3, msg );
	}
	
	// Result callback.
	LuaResource *resource = Rtt_NEW( LuaContext::GetAllocator( L ),
										LuaResource( LuaContext::GetContext( L )->LuaState(),
														3 /*!< Callback index. */ ) );

	// Input location.
	const char* location_string = luaL_checkstring( L, 2 );

	CLGeocoder *geocoder = [ [ [ CLGeocoder alloc ] init ] autorelease ];

	// Submit the lookup.
	[ geocoder geocodeAddressString:[ NSString stringWithUTF8String:location_string ]
				completionHandler:
		^(NSArray *placemarks, NSError *error)
		{
			// There is no guarantee that the CLGeocodeCompletionHandler
			// will be invoked on the main thread. So we use a
			// dispatch_async(dispatch_get_main_queue(),^{}) call to
			// ensure that UI updates are always performed from the main
			// thread.
	        dispatch_async( dispatch_get_main_queue(),
				^{
					if( ( error ) ||
						( placemarks.count == 0 ) )
					{
						// We have no result to provide.
						MapLocationEvent e( error.localizedDescription.UTF8String, (S32)error.code, location_string, MapLocationEvent::kRequestType );

						resource->DispatchEvent( e );
					}
					else
					{
						CLPlacemark *placemark = [ placemarks objectAtIndex:0 ];
						/*
						printf( "Result: %s : %f %f",
								placemark.description.UTF8String,
								placemark.location.coordinate.latitude,
								placemark.location.coordinate.longitude );
						*/
						MapLocationEvent e( placemark.location.coordinate.latitude,
											placemark.location.coordinate.longitude,
										    location_string);
						resource->DispatchEvent( e );
					}

					Rtt_DELETE( resource );
		        } );
		}
	];

	return 0;
}

int
IPhoneMapViewObject::getAddressLocation( lua_State *L )
{
	//! \todo Remove this deprecated, blocking, function.
	Rtt_LogException("WARNING: getAddressLocation is deprecated. Use requestLocation instead.\n");

	__block double latitude = 0.0;
	__block double longitude = 0.0;

	// CLGeocoder was introduced in iOS 5.0.
	if ( NSClassFromString( @"CLGeocoder" ) )
	{
		// Input location.
		const char* location_string = luaL_checkstring( L, 2 );

		CLGeocoder *geocoder = [ [ [ CLGeocoder alloc ] init ] autorelease ];

		__block bool we_have_a_result = false;

		// Submit the lookup.
		[ geocoder geocodeAddressString:[ NSString stringWithUTF8String:location_string ]
					completionHandler:
			^(NSArray *placemarks, NSError *error)
			{
				// There is no guarantee that the CLGeocodeCompletionHandler
				// will be invoked on the main thread. So we use a
				// dispatch_async(dispatch_get_main_queue(),^{}) call to
				// ensure that UI updates are always performed from the main
				// thread.
		        dispatch_async( dispatch_get_main_queue(),
					^{
						if( ( error ) ||
							( placemarks.count == 0 ) )
						{
							// We have no result to provide.
							// Nothing to do.
						}
						else
						{
							CLPlacemark *placemark = [ placemarks objectAtIndex:0 ];
							/*
							printf( "Result: %s : %f %f",
									placemark.description.UTF8String,
									placemark.location.coordinate.latitude,
									placemark.location.coordinate.longitude );
							*/
							latitude = placemark.location.coordinate.latitude;
							longitude = placemark.location.coordinate.longitude;
						}

						we_have_a_result = true;
			        } );
			}
		];

		// Wait until we have a result.
		while( 1 )
		{
			// This call has a lot of potential side-effects. It's ok because
			// we'll remove support for getAddressLocation soon.
			CFRunLoopRunInMode( kCFRunLoopDefaultMode,
								0.1f,
								false );

			if( we_have_a_result )
			{
				break;
			}
		}
	}
	else
	{
		Rtt_LogException("WARNING: getAddressLocation depends on CoreLocation, which requires iOS 5.0 or higher.\n");
	}

	lua_pushnumber( L, Rtt_FloatToReal( latitude ) );
	lua_pushnumber( L, Rtt_FloatToReal( longitude ) );

	return 2;
}

// myMap:addMarker( latitude, longitude [, params] )
// where params table specifies text for the optional popup on the map pin: { title="titleString", subtitle="subtitleString" }
int
IPhoneMapViewObject::addMarker( lua_State *L )
{
	IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();
	
	// Specify location to drop pin
	Rtt_Real latitude = Rtt_FloatToReal( lua_tonumber( L, 2 ) );
	Rtt_Real longitude = Rtt_FloatToReal( lua_tonumber( L, 3 ) );

	CLLocationCoordinate2D coordinate;
	coordinate.latitude = latitude;
	coordinate.longitude = longitude;

    int markerId = o->markerIdCounter++;
    
	if ( lua_istable( L, 4 ) )
	{
		AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
		const char *markerTitle = NULL;
		const char *markerSubtitle = NULL;
		
		LuaResource* resource = NULL;

		lua_getfield( L, 4, "title" );
		if ( lua_isstring( L, -1 ) )
		{
			markerTitle = luaL_checkstring( L, -1 );
		}
		lua_pop( L, 1 );
		lua_getfield( L, 4, "subtitle" );
		if (lua_isstring( L, -1 ) )
		{
			markerSubtitle = luaL_checkstring( L, -1 );
		}
		lua_pop( L, 1 );
		lua_getfield( L, 4, "listener" );
		if (Lua::IsListener( L, -1, "mapMarker" ))
		{
			resource = Rtt_NEW( LuaContext::GetAllocator( L ),
										LuaResource( LuaContext::GetContext( L )->LuaState(), -1 ) );
		}
		lua_pop( L, 1 );

		const char* fileName = NULL;
		// display.newImage and all the other apis all default to this directory
		MPlatform::Directory baseDirectory = MPlatform::kResourceDir;

		String fileNameWithPath( LuaContext::GetAllocator( L ) );
		lua_getfield(L, 4, "imageFile");
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			//map.appMarker(123, 234, {imageFIle = "picture.png"})
			fileName = lua_tostring(L, -1);
		}
		else if (lua_type(L, -1) == LUA_TTABLE)
		{
			//This file table is used in other apis like native.showPopup
			//map.appMarker(123, 234, {imageFIle = {filename = "picture.png", baseDir = system.TemporaryDirectory,}})
			lua_getfield(L, -1, "filename");
			if (lua_type(L, -1) == LUA_TSTRING)
			{
				fileName = lua_tostring(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "baseDir");
			if (lua_islightuserdata(L, -1))
			{
				baseDirectory = (MPlatform::Directory)EnumForUserdata(
													LuaLibSystem::Directories(),
													lua_touserdata( L, -1 ),
													MPlatform::kNumDirs,
													MPlatform::kUnknownDir);
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		if (fileName) 
		{			
			String suffixedFilename( LuaContext::GetAllocator( L ) );
				
			const MPlatform& platform = LuaContext::GetPlatform( L );

			// If it does not have a suffix then the suffixedFilename isn't populated
			bool hasSuffix = delegate.runtime->GetImageFilename(fileName, baseDirectory, suffixedFilename);
			if (hasSuffix) 
			{
				platform.PathForFile(suffixedFilename.GetString(), baseDirectory, MPlatform::kTestFileExists, fileNameWithPath);
			} 
			else 
			{
				platform.PathForFile(fileName, baseDirectory, MPlatform::kTestFileExists, fileNameWithPath);
			}
			if (!fileNameWithPath.GetString()) 
			{
				lua_pushnil( L );
				lua_pushstring( L, "The image file specified was not valid." );
				Rtt_LogException( "The image file specified was not valid." );
				return 2;
			}
		}
		
		NSString *markerTitleString = nil;
		NSString *markerSubtitleString = nil;
		if ( markerTitle )
		{
			markerTitleString = [[NSString alloc] initWithCString:markerTitle encoding:NSUTF8StringEncoding];
		}
		
		if ( markerSubtitle )
		{
			markerSubtitleString = [[NSString alloc] initWithCString:markerSubtitle encoding:NSUTF8StringEncoding];
		}
		
		AddressAnnotationWithCallout *annotation = [[AddressAnnotationWithCallout alloc] initWithCoordinate:coordinate title:markerTitleString subtitle:markerSubtitleString];

		[markerTitleString release];
		[markerSubtitleString release];
		
		MKAnnotationView *annotationView;
		if (fileName && fileNameWithPath.GetString())
		{
			UIImage *image = [UIImage imageWithContentsOfFile:[NSString stringWithUTF8String:fileNameWithPath.GetString()]];
			annotationView = [[MKAnnotationView alloc] initWithAnnotation:annotation reuseIdentifier:[NSString stringWithUTF8String:fileNameWithPath.GetString()]];
			annotationView.image = image;
			
			// Sets it so the image markers bottom middle is the point that stays set at a place.
			CGPoint bottomCenter = {0.0f, -image.size.height/2};
			annotationView.centerOffset = bottomCenter;
		}
		else
		{
			annotationView = [[MKPinAnnotationView alloc] initWithAnnotation:annotation reuseIdentifier:@""];
		}

		// Lets the annotation show the title and subtitle dialog box
		annotationView.canShowCallout = true;

		// This is used instead of the mapviewdelegate because the map view delegate doesn't detect when the annotationview is tapped a second time
		UITapGestureRecognizer *tgr = [[UITapGestureRecognizer alloc]initWithTarget:t action:@selector(handleGestureForMapMarker:)];

		[annotationView addGestureRecognizer:tgr];
		[tgr release];

		[annotation setAnnotationView:annotationView];
		[annotationView release];

        [annotation setAnnotationId:markerId];
        [annotation setLuaResourceForListener:resource];
		[t addAnnotation:annotation];
		[annotation release];
	}
	else 
	{
		AddressAnnotationWithCallout *annotation = [[AddressAnnotationWithCallout alloc] initWithCoordinate:coordinate title:nil subtitle:nil];
        [annotation setAnnotationId:markerId];
		[t addAnnotation:annotation];
		[annotation release];
	}

    lua_pushnumber(L, markerId);
    
	return 1;
}

int
IPhoneMapViewObject::removeMarker( lua_State *L )
{
    IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );
    
	Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();

    if ( lua_type( L, -1 ) == LUA_TNUMBER )
    {
    	int markerId = 0;
        markerId = (int)lua_tonumber(L, -1);
        for (id annotation in t.annotations)
		{
	    	if ([annotation isMemberOfClass:[AddressAnnotationWithCallout class]] && [annotation annotationId] == markerId)
			{
				[t removeAnnotation:annotation];
	            break;
			}
		}
    }

	return 0;
}
    
int
IPhoneMapViewObject::removeAllMarkers( lua_State *L )
{
	IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );

	Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();

	NSMutableArray *annotationsToRemove = [NSMutableArray arrayWithCapacity:[t.annotations count]];
	for (id annotation in t.annotations)
	{
    	if (annotation != t.userLocation)
		{
	        [annotationsToRemove addObject:annotation];
		}
	}
	[t removeAnnotations:annotationsToRemove];
	
	// Note: this method removes ALL annotations, including the blue user marker.	
	// [t removeAnnotations:t.annotations];
		
	return 0;
}


int
IPhoneMapViewObject::nearestAddress( lua_State *L )
{
	
	// Result callback.
	LuaResource *resource = Rtt_NEW( LuaContext::GetAllocator( L ),
										LuaResource( LuaContext::GetContext( L )->LuaState(),
														4 /*!< Callback index. */ ) );

	// Input location.
	Rtt_Real latitude = Rtt_FloatToReal( lua_tonumber( L, 2 ) );
	Rtt_Real longitude = Rtt_FloatToReal( lua_tonumber( L, 3 ) );

	CLGeocoder *geocoder = [ [ [ CLGeocoder alloc ] init ] autorelease ];

	// Submit the lookup.
	CLLocation *location = [ [ [ CLLocation alloc ] initWithLatitude:latitude
													longitude:longitude ] autorelease ];

	[ geocoder reverseGeocodeLocation:location
				completionHandler:
		^(NSArray *placemarks, NSError *error)
		{
			if( error )
			{
				// We have no result to provide.
				MapAddressEvent e( error.localizedDescription.UTF8String, (S32)error.code );

				resource->DispatchEvent( e );
			}
			else
			{
				CLPlacemark *placemark = [ placemarks objectAtIndex:0 ];
				/*
				printf( "Result: %s : %f %f",
						placemark.description.UTF8String,
						placemark.location.coordinate.latitude,
						placemark.location.coordinate.longitude );
				*/
				MapAddressEvent e( placemark.thoroughfare.UTF8String,
									placemark.subThoroughfare.UTF8String,
									placemark.locality.UTF8String,
									placemark.subLocality.UTF8String,
									placemark.administrativeArea.UTF8String,
									placemark.subAdministrativeArea.UTF8String,
									placemark.postalCode.UTF8String,
									placemark.country.UTF8String,
									placemark.ISOcountryCode.UTF8String );

				resource->DispatchEvent( e );
			}

			Rtt_DELETE( resource );
		}
	];

	// The closest address is returned a bit later as a "mapAddress" event, since it's an asynchronous lookup using a native API.
	// Therefore, no value is returned here.
	return 0;
}

static const char kStandardMapType[] = "standard";
static const char kSatelliteMapType[] = "satellite";
static const char kHybridMapType[] = "hybrid";

int
IPhoneMapViewObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;
	
	Rtt_MKMapView *t = (Rtt_MKMapView*)GetView();

	if ( strcmp( "mapType", key ) == 0 )
	{
		IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );
		Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();

		switch ( t.mapType ) {
			case MKMapTypeSatellite:
				lua_pushstring( L, kSatelliteMapType );
				break;
			case MKMapTypeHybrid:
				lua_pushstring(L, kHybridMapType );
				break;
			default:
				lua_pushstring( L, kStandardMapType );
				break;
		}
	}
	else if ( strcmp( "isZoomEnabled", key ) == 0 )
	{
		lua_pushboolean( L, t.zoomEnabled );
	}
	else if ( strcmp( "isScrollEnabled", key ) == 0 )
	{
		lua_pushboolean( L, t.scrollEnabled );
	}
	else if ( strcmp( "isLocationUpdating", key ) == 0 )
	{
		lua_pushboolean( L, t.showsUserLocation );
	}
	else if ( strcmp( "isLocationVisible", key ) == 0 )
	{
		lua_pushboolean( L, t.userLocationVisible ); // read-only
	}
	else if ( strcmp( "getUserLocation", key ) == 0 )
	{
		lua_pushcfunction( L, getUserLocation );
	}
	else if ( strcmp( "setRegion", key ) == 0 )
	{
		lua_pushcfunction( L, setRegion );
	}
	else if ( strcmp( "setCenter", key ) == 0 )
	{
		lua_pushcfunction( L, setCenter );
	}
	else if ( strcmp( "requestLocation", key ) == 0 )
	{
		lua_pushcfunction( L, requestLocation );
	}
	else if ( strcmp( "getAddressLocation", key ) == 0 )
	{
		lua_pushcfunction( L, getAddressLocation );
	}
	else if ( strcmp( "addMarker", key ) == 0 )
	{
		lua_pushcfunction( L, addMarker );
	}
    else if ( strcmp( "removeMarker", key ) == 0 )
	{
		lua_pushcfunction( L, removeMarker );
	}
	else if ( strcmp( "removeAllMarkers", key ) == 0 )
	{
		lua_pushcfunction( L, removeAllMarkers );
	}
	else if ( strcmp( "nearestAddress", key ) == 0 )
	{
		lua_pushcfunction( L, nearestAddress );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
IPhoneMapViewObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	IPhoneMapViewObject* o = (IPhoneMapViewObject*)LuaProxy::GetProxyableObject( L, 1 );
	Rtt_MKMapView *t = (Rtt_MKMapView*)o->GetView();

	bool result = true;
	
	if ( strcmp( "mapType", key ) == 0 )
	{
		const char *mapType = lua_tostring( L, valueIndex );
		
		if ( mapType ) 
		{
			if ( strcmp( kSatelliteMapType, mapType ) == 0 )
			{
				t.mapType = MKMapTypeSatellite;
			}
			else if ( strcmp( kHybridMapType, mapType ) == 0 )
			{
				t.mapType = MKMapTypeHybrid;
			}
			else
			{
				t.mapType = MKMapTypeStandard;
			}
		}
	}
	else if ( strcmp( "isZoomEnabled", key ) == 0 )
	{
		t.zoomEnabled = lua_toboolean( L, valueIndex ) ? YES : NO;
	}
	else if ( strcmp( "isScrollEnabled", key ) == 0 )
	{
		t.scrollEnabled = lua_toboolean( L, valueIndex ) ? YES : NO;
	}
	else if ( strcmp( "isLocationUpdating", key ) == 0 )
	{
		BOOL set = lua_toboolean( L, valueIndex ) ? YES : NO;
		if ( set ) {
			[t askForUserLocationAuthorization];
		}
		
		if ( [CLLocationManager locationServicesEnabled] && !(  [CLLocationManager authorizationStatus] == kCLAuthorizationStatusDenied
															  ||[CLLocationManager authorizationStatus] == kCLAuthorizationStatusNotDetermined
															  ||[CLLocationManager authorizationStatus] == kCLAuthorizationStatusRestricted ))
		{
			t.showsUserLocation = set;
		}
		t.trackUserLocationAfterAuthorization = set;
	}
	else
	{
		result = Super::SetValueForKey( L, key, valueIndex );
	}
		
	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

