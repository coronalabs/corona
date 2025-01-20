//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_WinRTDisplayObject.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_MPlatform.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTMapViewObject : public WinRTDisplayObject
{
	public:
		typedef WinRTMapViewObject Self;
		typedef WinRTDisplayObject Super;

		WinRTMapViewObject(const Rect& bounds);
		virtual ~WinRTMapViewObject();

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

} // namespace Rtt
