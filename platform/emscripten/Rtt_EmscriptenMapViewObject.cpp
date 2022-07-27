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
#include "Rtt_EmscriptenMapViewObject.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Constructors/Destructors
EmscriptenMapViewObject::EmscriptenMapViewObject(const Rect& bounds)
:	Super(bounds, "")
{
}

EmscriptenMapViewObject::~EmscriptenMapViewObject()
{
}

#pragma endregion


#pragma region Public Member Functions
bool EmscriptenMapViewObject::Initialize()
{
	Rect bounds;
	GetScreenBounds(bounds);
	return true;
}

const LuaProxyVTable& EmscriptenMapViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetMapViewObjectProxyVTable();
}

int EmscriptenMapViewObject::ValueForKey(lua_State *L, const char key[]) const
{
	Rtt_ASSERT(key);

	int result = 1;

	if (strcmp("mapType", key) == 0)
	{
	}
	else if (strcmp("isZoomEnabled", key) == 0)
	{
	}
	else if (strcmp("isScrollEnabled", key) == 0)
	{
	}
	else if (strcmp("isLocationUpdating", key) == 0)
	{
	}
	else if (strcmp("isLocationVisible", key) == 0)
	{
	}
	else if (strcmp("getUserLocation", key) == 0)
	{
		lua_pushcfunction(L, GetUserLocation);
	}
	else if (strcmp("setRegion", key) == 0)
	{
		lua_pushcfunction(L, SetRegion);
	}
	else if (strcmp("setCenter", key) == 0)
	{
		lua_pushcfunction(L, SetCenter);
	}
	else if (strcmp("requestLocation", key) == 0)
	{
		lua_pushcfunction(L, RequestLocation);
	}
	else if (strcmp("getAddressLocation", key) == 0)
	{
		lua_pushcfunction(L, GetAddressLocation);
	}
	else if (strcmp("addMarker", key) == 0)
	{
		lua_pushcfunction(L, AddMarker);
	}
	else if (strcmp("removeMarker", key) == 0)
	{
		lua_pushcfunction(L, RemoveMarker);
	}
	else if (strcmp("removeAllMarkers", key) == 0)
	{
		lua_pushcfunction(L, RemoveAllMarkers);
	}
	else if (strcmp("nearestAddress", key) == 0)
	{
		lua_pushcfunction(L, NearestAddress);
	}
	else
	{
		result = Super::ValueForKey(L, key);
	}

	return result;
}

bool EmscriptenMapViewObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	Rtt_ASSERT(key);

	bool result = true;

	if (strcmp("mapType", key) == 0)
	{
	}
	else if (strcmp("isZoomEnabled", key) == 0)
	{
	}
	else if (strcmp("isScrollEnabled", key) == 0)
	{
	}
	else
	{
		result = Super::SetValueForKey(L, key, valueIndex);
	}

	return result;
}

#pragma endregion


#pragma region Protected Member Functions
int EmscriptenMapViewObject::GetUserLocation(lua_State *L)
{
	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	EmscriptenMapViewObject *view = (EmscriptenMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Return current location data as a table to Lua.
	// Returns nil if current location has not been received.
	//	return NativeToJavaBridge::GetInstance()->MapViewPushCurrentLocationToLua(view->GetId(), L);
	return 0;
}

int EmscriptenMapViewObject::SetRegion(lua_State *L)
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
	EmscriptenMapViewObject *view = (EmscriptenMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Have the map view center at the given coordinates.
	//	NativeToJavaBridge::GetInstance()->MapViewSetRegion(
	//			view->GetId(), latitude, longitude, latitudeSpan, longitudeSpan, isAnimated);
	return 0;
}

int EmscriptenMapViewObject::SetCenter(lua_State *L)
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
	EmscriptenMapViewObject *view = (EmscriptenMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Have the map view center at the given coordinates.
	//	NativeToJavaBridge::GetInstance()->MapViewSetCenter(view->GetId(), latitude, longitude, isAnimated);
	return 0;
}

// This function uses the native geocoding API to convert any address string to latitude, longitude (utility function in AppDelegate.mm)
int EmscriptenMapViewObject::RequestLocation(lua_State *L)
{
	return 0;
}

int EmscriptenMapViewObject::GetAddressLocation(lua_State *L)
{
	Rtt_LogException("The GetAddressLocation function is in the process of being deprecated, please use RequestLocation instead");

	// Fetch the location name argument from Lua.
	const char *locationName = NULL;
	if (lua_type(L, 2) == LUA_TSTRING)
	{
		locationName = lua_tostring(L, 2);
	}
	else
	{
		luaL_error(L, "Function mapView:getAddressLocation() was not given a valid argument. Was expecting a string.");
	}

	// Fetch the given location string's coordinates and return them to Lua.
	//	return NativeToJavaBridge::GetInstance()->PushLocationNameCoordinatesToLua(locationName, L);
	return 0;
}

// myMap:addMarker( latitude, longitude [, params] )
// where params table specifies text for the optional popup on the map pin: { title="titleString", subtitle="subtitleString" }
int EmscriptenMapViewObject::AddMarker(lua_State *L)
{
	const char *title = NULL;
	const char *subtitle = NULL;
	//	String fileNameWithPath( NativeToJavaBridge::GetInstance()->GetRuntime()->Allocator() );
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
		if (Lua::IsListener(L, -1, "mapMarker"))
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
					lua_touserdata(L, -1),
					MPlatform::kNumDirs,
					MPlatform::kUnknownDir);
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

		if (fileName) {
#if 0
			String suffixedFilename(NativeToJavaBridge::GetInstance()->GetRuntime()->Allocator());

			Rtt::EmscriptenPlatform *platform = NativeToJavaBridge::GetInstance()->GetPlatform();

			// If it does not have a suffix then suffixedFilename isn't populated
			bool hasSuffix = NativeToJavaBridge::GetInstance()->GetRuntime()->GetImageFilename(fileName, baseDirectory, suffixedFilename);
			if (hasSuffix) {
				platform->PathForFile(suffixedFilename.GetString(), baseDirectory, MPlatform::kTestFileExists, fileNameWithPath);
			}
			else {
				platform->PathForFile(fileName, baseDirectory, MPlatform::kTestFileExists, fileNameWithPath);
			}

			// The file specified was not valid so return an error
			if (!fileNameWithPath.GetString()) {
				lua_pushnil(L);
				lua_pushstring(L, "The image file specified was not valid.");
				Rtt_LogException("The image file specified was not valid.");
				return 2;
			}
#endif
		}
	}

	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	EmscriptenMapViewObject *view = (EmscriptenMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		lua_pushnil(L);
		lua_pushstring(L, "The map view does not exist.");
		Rtt_LogException("The map view does not exist.");
		return 2;
	}

	// Add the marker to the map.
	//	int result = NativeToJavaBridge::GetInstance()->MapViewAddMarker(view->GetId(), latitude, longitude, title, subtitle, listener, fileNameWithPath.GetString());
	//	lua_pushnumber( L, result );
	//	return 1;
	return 0;
}

int EmscriptenMapViewObject::RemoveMarker(lua_State *L)
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
		markerId = lua_tointeger(L, 2);
	}
	else
	{
		luaL_error(L, "Function mapView:removeMarker() was given an invalid 'markerId' argument. Was expecting a number.");
	}

	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	EmscriptenMapViewObject *view = (EmscriptenMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Remove the marker to the map.
	//	NativeToJavaBridge::GetInstance()->MapViewRemoveMarker(view->GetId(), markerId);
	return 0;
}

int EmscriptenMapViewObject::RemoveAllMarkers(lua_State *L)
{
	// Fetch the map view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetMapViewObjectProxyVTable();
	EmscriptenMapViewObject *view = (EmscriptenMapViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Remove all markers from the map.
	//	NativeToJavaBridge::GetInstance()->MapViewRemoveAllMarkers(view->GetId());
	return 0;
}

int EmscriptenMapViewObject::NearestAddress(lua_State *L)
{
	// Fetch the coordinates nearest address asynchronously.
	// The results will be provided via an event.
	//	NativeToJavaBridge::GetInstance()->RequestNearestAddressFromCoordinates(L);
	return 0;
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
