//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneMapViewObject_H__
#define _Rtt_IPhoneMapViewObject_H__

#include "Rtt_IPhoneDisplayObject.h"

#include <math.h>
#import <MapKit/MapKit.h>
#import <MapKit/MKAnnotation.h>


// ----------------------------------------------------------------------------

@class Rtt_MKMapView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneMapViewObject : public IPhoneDisplayObject
{
	public:
		typedef IPhoneMapViewObject Self;
		typedef IPhoneDisplayObject Super;

	public:
		IPhoneMapViewObject( const Rect& bounds );
		virtual ~IPhoneMapViewObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;
		
	public:
		static int getUserLocation( lua_State *L );
		static int setRegion( lua_State *L );
		static int setCenter( lua_State *L );
		static int requestLocation( lua_State *L ); // New. Asynchronous.
		static int getAddressLocation( lua_State *L ); // Old. Synchronous.
		static int addMarker( lua_State *L );
		static int removeMarker( lua_State *L );
		static int removeAllMarkers( lua_State *L );
		static int nearestAddress( lua_State *L );


	protected:
		// static int setTextColor( lua_State *L );
        int markerIdCounter;

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

@interface Rtt_MKMapView : MKMapView <CLLocationManagerDelegate>
{
	Rtt::IPhoneMapViewObject *owner;
}

@property(nonatomic, assign) BOOL trackUserLocationAfterAuthorization;
@property(nonatomic, assign) Rtt::IPhoneMapViewObject *owner;

@end

@interface AddressAnnotationWithCallout : NSObject<MKAnnotation> {
	CLLocationCoordinate2D coordinate;
	
	NSString *mTitle;
	NSString *mSubTitle;
}

@property(nonatomic, retain) MKAnnotationView* annotationView;
@property int annotationId;
@property(nonatomic, assign) Rtt::LuaResource* luaResourceForListener;

@end


// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneMapViewObject_H__
