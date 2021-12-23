//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_EmscriptenDisplayObject.h"
#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EmscriptenMapViewObject : public EmscriptenDisplayObject
{
	public:
		typedef EmscriptenMapViewObject Self;
		typedef EmscriptenDisplayObject Super;

		EmscriptenMapViewObject(const Rect& bounds);
		virtual ~EmscriptenMapViewObject();

		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);

	protected:
		static int GetUserLocation(lua_State *L);
		static int SetRegion(lua_State *L);
		static int SetCenter(lua_State *L);
		static int RequestLocation(lua_State *L); // New. Asynchronous.
		static int GetAddressLocation(lua_State *L); // Old. Synchronous.
		static int AddMarker(lua_State *L);
		static int RemoveMarker(lua_State *L);
		static int RemoveAllMarkers(lua_State *L);
		static int NearestAddress(lua_State *L);
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
