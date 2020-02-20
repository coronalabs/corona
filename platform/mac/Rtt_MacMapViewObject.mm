//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacMapViewObject.h"

#include "Rtt_Event.h"
#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacMapViewObject::MacMapViewObject( const Rect& bounds )
:	Super( bounds )
{
}

MacMapViewObject::~MacMapViewObject()
{
}

bool
MacMapViewObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	return true;
}

const LuaProxyVTable&
MacMapViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetMapViewObjectProxyVTable();
}

int
MacMapViewObject::getUserLocation( lua_State *L )
{
	lua_newtable( L );
	lua_pushnumber( L, 37.f );
	lua_setfield( L, -2, LocationEvent::kLatitudeKey );
	lua_pushnumber( L, -122.f );
	lua_setfield( L, -2, LocationEvent::kLongitudeKey );
	lua_pushnumber( L, 0.f );
	lua_setfield( L, -2, LocationEvent::kAltitudeKey );
	lua_pushnumber( L, 2000.f );
	lua_setfield( L, -2, LocationEvent::kAccuracyKey );
	lua_pushnumber( L, 0.f );
	lua_setfield( L, -2, LocationEvent::kSpeedKey );
	lua_pushnumber( L, time( NULL ) );
	lua_setfield( L, -2, LocationEvent::kTimeKey );
	lua_pushboolean( L, false );
	lua_setfield( L, -2, LocationEvent::kIsUpdatingKey );
	
	return 1;
}

int
MacMapViewObject::setRegion( lua_State *L )
{
	return 0;
}

int
MacMapViewObject::setCenter( lua_State *L )
{
	return 0;
}

// This function uses the native geocoding API to convert any address string to latitude, longitude (utility function in AppDelegate.mm)
int
MacMapViewObject::requestLocation( lua_State *L )
{
	return 0;
}

int
MacMapViewObject::getAddressLocation( lua_State *L )
{
	//! \todo Remove this function.
	Rtt_LogException("WARNING: getAddressLocation is deprecated. Use requestLocation instead.\n");

	return 0;
}

// myMap:addMarker( latitude, longitude [, params] )
// where params table specifies text for the optional popup on the map pin: { title="titleString", subtitle="subtitleString" }
int
MacMapViewObject::addMarker( lua_State *L )
{
	return 0;
}

int
MacMapViewObject::removeMarker( lua_State *L )
{
	return 0;
}

int
MacMapViewObject::removeAllMarkers( lua_State *L )
{
	return 0;
}


int
MacMapViewObject::nearestAddress( lua_State *L )
{
	return 0;
}

static const char kStandardMapType[] = "standard";
// NOT USED: static const char kSatelliteMapType[] = "satellite";
// NOT USED: static const char kHybridMapType[] = "hybrid";

int
MacMapViewObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "mapType", key ) == 0 )
	{
		lua_pushstring( L, kStandardMapType );
	}
	else if ( strcmp( "isZoomEnabled", key ) == 0 )
	{
		lua_pushboolean( L, false );
	}
	else if ( strcmp( "isScrollEnabled", key ) == 0 )
	{
		lua_pushboolean( L, false );
	}
	else if ( strcmp( "isLocationUpdating", key ) == 0 )
	{
		lua_pushboolean( L, false );
	}
	else if ( strcmp( "isLocationVisible", key ) == 0 )
	{
		lua_pushboolean( L, false ); // read-only
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
MacMapViewObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	/*
	Rtt_UITextView *t = (Rtt_UITextView*)GetView();

	if ( strcmp( "text", key ) == 0 )
	{
		const char *s = lua_tostring( L, valueIndex );
		if ( Rtt_VERIFY( s ) )
		{
			NSString *newValue = [[NSString alloc] initWithUTF8String:s];
			t.text = newValue;
			[newValue release];
		}
	}
	else if ( strcmp( "size", key ) == 0 )
	{
		if ( Rtt_VERIFY( lua_isnumber( L, valueIndex ) ) )
		{
			UIFont *newFont = [t.font fontWithSize:lua_tonumber( L, valueIndex )];
			t.font = newFont;
		}
	}
	else if ( strcmp( "font", key ) == 0 )
	{
		PlatformFont *font = LuaLibNative::ToFont( L, valueIndex );
		if ( font )
		{
			t.font = (UIFont*)font->NativeObject();
		}
	}
	*/	
	
	if ( strcmp( "mapType", key ) == 0 )
	{
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

