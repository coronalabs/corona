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


#include "Core/Rtt_Build.h"
#include "Rtt_AndroidMapViewObject.h"

#include "Core/Rtt_String.h"
#include "Rtt_AndroidPlatform.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "NativeToJavaBridge.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidMapViewObject::AndroidMapViewObject(
		const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb)
:	Super(bounds, displayObjectRegistry, ntjb), fNativeToJavaBridge(ntjb)
{
}

AndroidMapViewObject::~AndroidMapViewObject()
{
}

bool
AndroidMapViewObject::Initialize()
{
	Rect bounds;
	GetScreenBounds(bounds);
	fNativeToJavaBridge->MapViewCreate(
			GetId(), bounds.xMin, bounds.yMin, bounds.Width(), bounds.Height());
	return true;
}

const LuaProxyVTable&
AndroidMapViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetMapViewObjectProxyVTable();
}

int
AndroidMapViewObject::GetUserLocation(lua_State *L)
{
	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	AndroidMapViewObject *view = (AndroidMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Return current location data as a table to Lua.
	// Returns nil if current location has not been received.
	return view->fNativeToJavaBridge->MapViewPushCurrentLocationToLua(view->GetId(), L);
}

int
AndroidMapViewObject::SetRegion( lua_State *L )
{
	double latitude = 0;
	double longitude = 0;
	double latitudeSpan = 0;
	double longitudeSpan = 0;
	bool isAnimated = false;

	// Do not continue if not given enough arguments.
	if (lua_gettop(L) < 5)
	{
		luaL_error(L, "Function mapView:setRegion() was not given enough arguments.");
	}

	// Fetch latitude and longitude arguments from Lua.
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		latitude = lua_tonumber(L, 2);
	}
	else
	{
		luaL_error(L, "Function mapView:setRegion() was given an invalid 'latitude' argument. Was expecting a number.");
	}
	if (lua_type(L, 3) == LUA_TNUMBER)
	{
		longitude = lua_tonumber(L, 3);
	}
	else
	{
		luaL_error(L, "Function mapView:setRegion() was given an invalid 'longitude' argument. Was expecting a number.");
	}

	// Fetch the zoom level arguments from Lua.
	if (lua_type(L, 4) == LUA_TNUMBER)
	{
		latitudeSpan = lua_tonumber(L, 4);
	}
	else
	{
		luaL_error(L, "Function mapView:setRegion() was given an invalid 'latitudeSpan' argument. Was expecting a number.");
	}
	if (lua_type(L, 5) == LUA_TNUMBER)
	{
		longitudeSpan = lua_tonumber(L, 5);
	}
	else
	{
		luaL_error(L, "Function mapView:setRegion() was given an invalid 'longitudeSpan' argument. Was expecting a number.");
	}

	// Attempt to fetch the optional "isAnimated" argument from Lua.
	if (lua_type(L, 4) == LUA_TBOOLEAN)
	{
		isAnimated = lua_toboolean(L, 4) ? true : false;
	}

	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	AndroidMapViewObject *view = (AndroidMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Have the map view center at the given coordinates.
	view->fNativeToJavaBridge->MapViewSetRegion(
			view->GetId(), latitude, longitude, latitudeSpan, longitudeSpan, isAnimated);
	return 0;
}

int
AndroidMapViewObject::SetCenter( lua_State *L )
{
	double latitude = 0;
	double longitude = 0;
	bool isAnimated = false;

	// Do not continue if not given enough arguments.
	if (lua_gettop(L) < 3)
	{
		luaL_error(L, "Function mapView:setCenter() was not given enough arguments.");
	}

	// Fetch latitude and longitude arguments from Lua.
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		latitude = lua_tonumber(L, 2);
	}
	else
	{
		luaL_error(L, "Function mapView:setCenter() was given an invalid 'latitude' argument. Was expecting a number.");
	}
	if (lua_type(L, 3) == LUA_TNUMBER)
	{
		longitude = lua_tonumber(L, 3);
	}
	else
	{
		luaL_error(L, "Function mapView:setCenter() was given an invalid 'longitude' argument. Was expecting a number.");
	}

	// Attempt to fetch the optional "isAnimated" argument from Lua.
	if (lua_type(L, 4) == LUA_TBOOLEAN)
	{
		isAnimated = lua_toboolean(L, 4) ? true : false;
	}

	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	AndroidMapViewObject *view = (AndroidMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Have the map view center at the given coordinates.
	view->fNativeToJavaBridge->MapViewSetCenter(view->GetId(), latitude, longitude, isAnimated);
	return 0;
}

// This function uses the native geocoding API to convert any address string to latitude, longitude (utility function in AppDelegate.mm)
int
AndroidMapViewObject::RequestLocation( lua_State *L )
{
	// Fetch the given location string's coordinates and return them to Lua.
	NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
	bridge->RequestLocationAsync(L);
	return 0;
}

int
AndroidMapViewObject::GetAddressLocation( lua_State *L )
{
	Rtt_LogException("The GetAddressLocation function is in the process of being deprecated, please use RequestLocation instead");

	// Fetch the location name argument from Lua.
	const char *locationName = NULL;
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		locationName = lua_tostring( L, 2 );
	}
	else
	{
		luaL_error(L, "Function mapView:getAddressLocation() was not given a valid argument. Was expecting a string.");
	}

	// Fetch the given location string's coordinates and return them to Lua.
	NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
	return bridge->PushLocationNameCoordinatesToLua(locationName, L);
}

// myMap:addMarker( latitude, longitude [, params] )
// where params table specifies text for the optional popup on the map pin: { title="titleString", subtitle="subtitleString" }
int
AndroidMapViewObject::AddMarker( lua_State *L )
{
	const char *title = NULL;
	const char *subtitle = NULL;

	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	AndroidMapViewObject *view = (AndroidMapViewObject *)luaL_todisplayobject(L, 1, table);

	if (!view)
	{
		lua_pushnil( L );
		lua_pushstring( L, "The map view does not exist." );
		Rtt_LogException( "The map view does not exist." );
		return 2;
	}

	String fileNameWithPath( view->fNativeToJavaBridge->GetRuntime()->Allocator() );

	double latitude = 0;
	double longitude = 0;

	int listener = LUA_NOREF;

	// Do not continue if not given enough arguments.
	if (lua_gettop(L) < 3)
	{
		luaL_error(L, "Function mapView:addMarker() was not given enough arguments.");
	}

	// Fetch latitude and longitude arguments from Lua.
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		latitude = lua_tonumber(L, 2);
	}
	else
	{
		luaL_error(L, "Function mapView:addMarker() was given an invalid 'latitude' argument. Was expecting a number.");
	}
	if (lua_type(L, 3) == LUA_TNUMBER)
	{
		longitude = lua_tonumber(L, 3);
	}
	else
	{
		luaL_error(L, "Function mapView:addMarker() was given an invalid 'longitude' argument. Was expecting a number.");
	}

	// Fetch optional table argument from Lua.
	if (lua_istable(L, 4))
	{
		lua_getfield(L, 4, "title");
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			title = lua_tostring(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "subtitle");
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			subtitle = lua_tostring(L, -1);
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "listener");
		if (Lua::IsListener( L, -1, "mapMarker")) 
		{
			// This pops the object off the stack
			listener = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		else
		{
			lua_pop(L, 1);
		}
		
		const char* fileName = NULL;
		// display.newImage and all the other apis all default to this directory
		MPlatform::Directory baseDirectory = MPlatform::kResourceDir;

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

		if (fileName) {
			String suffixedFilename( view->fNativeToJavaBridge->GetRuntime()->Allocator() );
				
			Rtt::AndroidPlatform *platform = view->fNativeToJavaBridge->GetPlatform();

			// If it does not have a suffix then suffixedFilename isn't populated
			bool hasSuffix = view->fNativeToJavaBridge->GetRuntime()->GetImageFilename(fileName, baseDirectory, suffixedFilename);
			if (hasSuffix) {
				platform->PathForFile(suffixedFilename.GetString(), baseDirectory, MPlatform::kTestFileExists, fileNameWithPath);
			} else {
				platform->PathForFile(fileName, baseDirectory, MPlatform::kTestFileExists, fileNameWithPath);
			}

			// The file specified was not valid so return an error
			if (!fileNameWithPath.GetString()) {
				lua_pushnil( L );
				lua_pushstring( L, "The image file specified was not valid." );
				Rtt_LogException( "The image file specified was not valid." );
				return 2;
			}
		}	
	}

	// Add the marker to the map.
	int result = view->fNativeToJavaBridge->MapViewAddMarker(view->GetId(), latitude, longitude, title, subtitle, listener, fileNameWithPath.GetString());
	lua_pushnumber( L, result );
	return 1;
}

int
AndroidMapViewObject::RemoveMarker( lua_State *L )
{
	int markerId = 0;

	// Do not continue if not given enough arguments.
	if (lua_gettop(L) < 1)
	{
		luaL_error(L, "Function mapView:addMarker() was not given enough arguments.");
	}

	// Fetch latitude and longitude arguments from Lua.
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		markerId = lua_tonumber(L, 2);
	}
	else
	{
		luaL_error(L, "Function mapView:removeMarker() was given an invalid 'markerId' argument. Was expecting a number.");
	}
	
	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	AndroidMapViewObject *view = (AndroidMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Remove the marker to the map.
	view->fNativeToJavaBridge->MapViewRemoveMarker(view->GetId(), markerId);
	
	return 0;
}

int
AndroidMapViewObject::RemoveAllMarkers( lua_State *L )
{
	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	AndroidMapViewObject *view = (AndroidMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Remove all markers from the map.
	view->fNativeToJavaBridge->MapViewRemoveAllMarkers(view->GetId());
	return 0;
}

int
AndroidMapViewObject::NearestAddress( lua_State *L )
{
	// Fetch the coordinates nearest address asynchronously.
	// The results will be provided via an event.
	NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
	bridge->RequestNearestAddressFromCoordinates(L);
	return 0;
}

int
AndroidMapViewObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;
	
	if ( strcmp( "mapType", key ) == 0 )
	{
		Rtt_Allocator& allocator = fNativeToJavaBridge->GetPlatform()->GetAllocator();
		Rtt::String mapTypeName(&allocator);
		fNativeToJavaBridge->MapViewGetType(GetId(), &mapTypeName);
		lua_pushstring( L, mapTypeName.GetString() );
	}
	else if ( strcmp( "isZoomEnabled", key ) == 0 )
	{
		bool isEnabled = fNativeToJavaBridge->MapViewIsZoomEnabled(GetId());
		lua_pushboolean( L, isEnabled ? 1 : 0 );
	}
	else if ( strcmp( "isScrollEnabled", key ) == 0 )
	{
		bool isEnabled = fNativeToJavaBridge->MapViewIsScrollEnabled(GetId());
		lua_pushboolean( L, isEnabled ? 1 : 0 );
	}
	else if ( strcmp( "isLocationUpdating", key ) == 0 )
	{
		lua_pushboolean( L, true ? 1 : 0 );
	}
	else if ( strcmp( "isLocationVisible", key ) == 0 )
	{
		bool isVisible = fNativeToJavaBridge->MapViewIsCurrentLocationVisible(GetId());
		lua_pushboolean( L, isVisible ? 1 : 0 );
	}
	else if ( strcmp( "getUserLocation", key ) == 0 )
	{
		lua_pushcfunction( L, GetUserLocation );
	}
	else if ( strcmp( "setRegion", key ) == 0 )
	{
		lua_pushcfunction( L, SetRegion );
	}
	else if ( strcmp( "setCenter", key ) == 0 )
	{
		lua_pushcfunction( L, SetCenter );
	}
	else if ( strcmp( "requestLocation", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, RequestLocation, 1 );
	}
	else if ( strcmp( "getAddressLocation", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, GetAddressLocation, 1 );
	}
	else if ( strcmp( "addMarker", key ) == 0 )
	{
		lua_pushcfunction( L, AddMarker );
	}
	else if ( strcmp( "removeMarker", key ) == 0 )
	{
		lua_pushcfunction( L, RemoveMarker );
	}
	else if ( strcmp( "removeAllMarkers", key ) == 0 )
	{
		lua_pushcfunction( L, RemoveAllMarkers );
	}
	else if ( strcmp( "nearestAddress", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, NearestAddress, 1 );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
AndroidMapViewObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "mapType", key ) == 0 )
	{
		const char *mapTypeName = lua_tostring(L, valueIndex);
		fNativeToJavaBridge->MapViewSetType(GetId(), mapTypeName);
	}
	else if ( strcmp( "isZoomEnabled", key ) == 0 )
	{
		bool enabled = lua_toboolean(L, valueIndex) ? true : false;
		fNativeToJavaBridge->MapViewSetZoomEnabled(GetId(), enabled);
	}
	else if ( strcmp( "isScrollEnabled", key ) == 0 )
	{
		bool enabled = lua_toboolean(L, valueIndex) ? true : false;
		fNativeToJavaBridge->MapViewSetScrollEnabled(GetId(), enabled);
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

